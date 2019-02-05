//
// RaspiKey Copyright (c) 2019 George Samartzidis <samartzidis@gmail.com>. All rights reserved.
// You are not allowed to redistribute, modify or sell any part of this file in either 
// compiled or non-compiled form without the author's written permission.
//

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <linux/input.h>
#include <string>      
#include <iostream> 
#include <sstream>
#include <stdint.h>
#include <stddef.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>
#include <signal.h>
#include <tuple>
#include <libgen.h>
#include <sys/time.h>
#include "Globals.h"
#include "ReportFilter.h"
#include "A1644.h"
#include "A1314.h"
#include "WebApiServer.h"
#include "bluetooth/bt.h"
#include "Logger.h"
#include "GenericReportFilter.h"
#include "Main.h"
#include "KeyMapReportFilter.h"

using namespace std;

WebApiServer g_WebApiServer;
bool g_ExitRequested = false;
IReportFilter* g_pReportFilters[3] = { nullptr };

int main(int argc, char** argv)
{
	InfoMsg("RaspiKey " VERSION ".");

	// Get RaspiKey binary "working" dir
	readlink("/proc/self/exe", Globals::g_szModuleDir, sizeof(Globals::g_szModuleDir));
	dirname(Globals::g_szModuleDir); //strip last component from path

	InfoMsg(Globals::FormatString("System uptime: %lu", Globals::GetUptime()).c_str());

	struct stat info;
	if (stat(DATA_DIR, &info) != 0)
	{
		ErrorMsg("cannot access data directory %s", DATA_DIR);
		return -1;
	}
	if (!(info.st_mode & S_IFDIR))
	{
		ErrorMsg("data directory %s is not a directory", DATA_DIR);
		return -1;
	}

	if (!StartServices())
	{
		ErrorMsg("StartServices() failed.");
		return -1;
	}

	OpenDevicesLoop();

	return 0;
}

void SignalHandler(int signo)
{
	if (signo == SIGINT || signo == SIGTERM)
	{
		InfoMsg("Termination request received");		

		StopAllServices();
	}
	else if (signo == SIGALRM) // Timer
	{
		InfoMsg("Timer elapsed signal received");
	}
}

bool StartServices()
{
	// Start the Web Api server
	g_WebApiServer.Start();

	// Start the bluetooth
	try
	{
		bluetooth::Start();
	}
	catch (exception& ex)
	{
		ErrorMsg("Fatal error: could not initialize the bluetooth device: %s", ex.what());
		return false;
	}

	// Set signal handlers
	signal(SIGINT, SignalHandler);
	signal(SIGTERM, SignalHandler);
	signal(SIGALRM, SignalHandler);

	return true;
}

void OpenDevicesLoop()
{
	while (!g_ExitRequested)
	{
		DeviceDescriptors fds = { };

		// Open devices
		if (OpenKbDevice(fds) < 0)
		{
			sleep(1);
			continue;
		}
		
		if (OpenHidgDevice(fds.hidgFd) < 0)
		{
			close(fds.inputEventFd);
			close(fds.hidRawFd);

			sleep(1);
			continue;
		}

		// Get the unique identifier (bluetooth address) for opened device
		string devId = fds.inputEventDevUniq;
		std::transform(devId.begin(), devId.end(), devId.begin(), ::toupper);
		InfoMsg("Device unique identifier: %s", devId.c_str());

		// Create the correct keyboard device IReportFilter		
		if (fds.inputEventDevName == A1644_DEV_NAME)
		{
			InfoMsg("Using A1644 report filter");
			g_pReportFilters[0] = new A1644();
		}
		else if (fds.inputEventDevName == A1314_DEV_NAME)
		{
			InfoMsg("Using A1314 report filter");
			g_pReportFilters[0] = new A1314();
		}
		else
		{
			InfoMsg("Using Generic report filter");
			g_pReportFilters[0] = new GenericReportFilter(); 
		}
		
		// Set keyboard filter settings if file exists
		string path = GetSettingsFilePath(devId);
		string settings;
		if (Globals::FileReadAllText(path, settings))
		{
			try
			{
				g_pReportFilters[0]->SetSettings(settings);
			}
			catch (const exception& m)
			{
				ErrorMsg(m.what());
			}
		}

		// Create KeyMapReportFilter IReportFilter
		g_pReportFilters[1] = new KeyMapReportFilter();

		// Set keymap filter settings if file exists
		path = GetKeymapFilePath(devId);
		if (Globals::FileReadAllText(path, settings))
		{
			try
			{
				g_pReportFilters[1]->SetSettings(settings);
			}
			catch (const exception& m)
			{
				ErrorMsg(m.what());
			}
		}

		// Start forwarding loop with the established devices
		ForwardingLoop(g_pReportFilters, fds.hidRawFd, fds.hidgFd);

		// This is to prevent the frozen "C" key situation if the user presses Ctrl-C in the raspikey process console.
		if (g_ExitRequested)
		{
			DbgMsg("Sending break scancode before exiting");
			uint8_t buf[16] = { 0 };
			buf[0] = 1;
			write(fds.hidgFd, buf, 9);
		}
		
		// Close devices
		close(fds.inputEventFd);
		close(fds.hidRawFd);
		close(fds.hidgFd);

		// Delete all previously created IReportFilter instances
		for (int k = 0; g_pReportFilters[k] != nullptr; k++)
			delete g_pReportFilters[k];

		sleep(1);
	}
}

void StopAllServices()
{	
	g_ExitRequested = true;

	bluetooth::Stop();
	g_WebApiServer.Stop();	
}

int ForwardingLoop(IReportFilter** prps, int hidRawFd, int hidgFd)
{
	uint8_t buf[16] = { 0 };

	while(!g_ExitRequested)
	{
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(hidRawFd, &fds);
		FD_SET(hidgFd, &fds);
		const int fdsmax = std::max(hidRawFd, hidgFd);

		struct timeval tv{}; 
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		const int ret = select(fdsmax + 1, &fds, nullptr, nullptr, &tv);
		if (ret == -1)
		{
			ErrorMsg("Error select(): %s", strerror(errno));
			if (errno == EINTR) // Interrupted system call due to external signal (we should retry)
				continue;
			return -1; // Any other error, exit the loop
		}

		// Direction: hidRawFd -> hidgFd
		if (FD_ISSET(hidRawFd, &fds))
		{
			ssize_t len = read(hidRawFd, buf, sizeof(buf));
			if (len < 0)
			{
				ErrorMsg("Error read: %s", strerror(errno));
				return -1;
			}
					
			DbgMsg("[in] -> %s", Globals::FormatBuffer(buf, len).c_str());

			// Process input report with each IReportFilter
			for (int k = 0; prps[k] != nullptr; k++)
			{
				len = prps[k]->ProcessInputReport(buf, len); // Process report with filter
			}			

			if (len > 0) // Should we suppress this report?
			{
				DbgMsg("[in] ~> %s", Globals::FormatBuffer(buf, len).c_str());

				len = write(hidgFd, buf, len); // Transfer report to hidg device
				if (len < 0)
				{
					ErrorMsg("Error write: %s", strerror(errno));
					return -1;
				}
			}
		}

		// Direction: hidgFd -> hidRawFd
		if (FD_ISSET(hidgFd, &fds))
		{
			ssize_t len = read(hidgFd, buf, sizeof(buf));
			if (len < 0)
			{
				ErrorMsg("Error read: %s", strerror(errno));
				return -1;
			}

			DbgMsg("[out] -> %s", Globals::FormatBuffer(buf, len).c_str());

			// Process output report with each IReportFilter
			for (int k = 0; prps[k] != nullptr; k++)
			{
				len = prps[k]->ProcessOutputReport(buf, len); // Process report with filter
			}

			if (len > 0) // Should we suppress this report?
			{
				DbgMsg("[out] ~> %s", Globals::FormatBuffer(buf, len).c_str());

				len = write(hidRawFd, buf, len);
				if (len < 0)
				{
					ErrorMsg("Error write: %s", strerror(errno));
					return -1;
				}
			}
		}		
	}

	return 0;
}

int OpenKbDevice(DeviceDescriptors& fds)
{
	fds.inputEventFd = open(EVENT_DEV_PATH, O_RDWR);
	if (fds.inputEventFd < 0)
	{
		InfoMsg("Failed to open() " EVENT_DEV_PATH ": %s", strerror(errno));
		
		return -1;
	}

	char szDevName[256] = "";
	ioctl(fds.inputEventFd, EVIOCGNAME(sizeof(szDevName)), szDevName);
	fds.inputEventDevName = szDevName;

	char szDevPhys[256] = "";
	ioctl(fds.inputEventFd, EVIOCGPHYS(sizeof(szDevPhys)), szDevPhys); //BT ctrl address
	fds.inputEventDevPhys = szDevPhys;

	char szDevUniq[256] = ""; 
	ioctl(fds.inputEventFd, EVIOCGUNIQ(sizeof(szDevUniq)), szDevUniq); //Dev BT address
	fds.inputEventDevUniq = szDevUniq;

	int res = ioctl(fds.inputEventFd, EVIOCGRAB, 1);
	if (res < 0)
	{
		InfoMsg("Failed to get exclusive access on " EVENT_DEV_PATH ": %s", strerror(errno));
		
		close(fds.inputEventFd);
		fds.inputEventFd = -1;

		return -1;
	}

	fds.hidRawFd = open(HIDRAW_DEV_PATH, O_RDWR);
	if (fds.hidRawFd < 0)
	{
		InfoMsg("Failed to open() " HIDRAW_DEV_PATH ": %s", strerror(errno));

		close(fds.inputEventFd);
		fds.inputEventFd = -1;

		return -1;
	}

	return 0;
}

int OpenHidgDevice(int& hidgFd)
{
	hidgFd = open(HIDG_DEV_PATH, O_RDWR);
	if (hidgFd == -1)
	{
		InfoMsg("Failed to open() " HIDG_DEV_PATH ": %s", strerror(errno));

		return -1;
	}

	return 0;
}

std::string GetKeymapFilePath(const std::string& devId)
{
	string addrUpper = devId;
	std::transform(addrUpper.begin(), addrUpper.end(), addrUpper.begin(), ::toupper);

	return Globals::FormatString(DATA_DIR "/%s.keymap", addrUpper.c_str());
}

std::string GetSettingsFilePath(const std::string& devId)
{
	string addrUpper = devId;
	std::transform(addrUpper.begin(), addrUpper.end(), addrUpper.begin(), ::toupper);

	return Globals::FormatString(DATA_DIR "/%s.settings", addrUpper.c_str());
}

void DeleteKeyMap(const std::string& devId)
{
	const string path = GetKeymapFilePath(devId);

	Globals::DeleteFile(path);
	g_pReportFilters[1]->SetSettings("");	
}

bool HasKeyMap(const std::string& devId)
{	
	const string path = GetKeymapFilePath(devId);

	return access(path.c_str(), F_OK) != -1;
}

string GetKeyMap(const std::string& devId)
{
	return g_pReportFilters[1]->GetSettings();
}

void SetKeyMap(const std::string& devId, const std::string& json)
{
	// Will fail if the data is invalid
	try
	{
		g_pReportFilters[1]->SetSettings(json);
	}
	catch (const exception& m)
	{
		ErrorMsg("Failed to set keymap: %s", m.what());
		throw;
	}

	// Write validated keymap file
	const string path = GetKeymapFilePath(devId);
	if (!Globals::FileWriteAllText(path, json))
		throw runtime_error("Failed to write the keymap file");
}

std::string GetSettings(const std::string& devId)
{
	return g_pReportFilters[0]->GetSettings();
}

void SetSettings(const std::string& devId, const std::string& json)
{
	// Will fail if the data is invalid
	try
	{
		g_pReportFilters[0]->SetSettings(json);
	}
	catch (const exception& m)
	{
		ErrorMsg("Failed to set device settings: %s", m.what());
		throw;
	}

	// Write validated settings file
	const string path = GetSettingsFilePath(devId);
	if (!Globals::FileWriteAllText(path, json))
		throw runtime_error("Failed to write device settings file");
}






