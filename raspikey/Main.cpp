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

using namespace std;

IReportFilter* g_prp;
WebApiServer g_WebApiServer;
bool g_ExitRequested = false;

int main(int argc, char** argv)
{
	InfoMsg("RaspiKey " VERSION ".");

	// Get RaspiKey binary "working" dir
	readlink("/proc/self/exe", Globals::g_szModuleDir, sizeof(Globals::g_szModuleDir));
	dirname(Globals::g_szModuleDir); //strip last component from path

	InfoMsg(Globals::FormatString("System uptime: %lu", Globals::GetUptime()).c_str());

	if (!StartServices())
	{
		ErrorMsg("StartServices() failed.");
		return -1;
	}

	PollDevicesLoop();

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

void PollDevicesLoop()
{
	while (!g_ExitRequested)
	{
		FileDescriptors fds;
		memset(&fds, 0, sizeof(fds));

		if (!OpenDevices(fds))
		{
			sleep(1);
			continue;
		}

		int vid, pid = 0;
		Globals::ModelId modelId;
		if (!Globals::GetInputDeviceInfo(fds.inputEventDevName.c_str(), vid, pid, modelId))
		{
			ErrorMsg("GetInputDeviceInfo() failed");
			sleep(1);
			continue;
		}

		InfoMsg("Got device info Vid=0x%04x, Pid=0x%04x, ModelId=%d", vid, pid, modelId);

		if (modelId == Globals::ModelId::A1644)
		{
			InfoMsg("Using A1644 report filter");
			g_prp = new A1644();
		}
		else if (modelId == Globals::ModelId::A1314)
		{
			InfoMsg("Using A1314 report filter");
			g_prp = new A1314();
		}
		else
		{
			InfoMsg("Using Generic report filter");
			g_prp = new GenericReportFilter();
		}

		// Start forwarding loop with the established devices
		ForwardingLoop(g_prp, fds.hidRawFd, fds.hidgFd);
		if (g_ExitRequested)
		{
			// This is to prevent the frozen "C" key situation if the user presses Ctrl-C in the raspikey process console.
			DbgMsg("Sending break scancode before exiting");
			uint8_t buf[16] = { 0 };
			buf[0] = 1;
			write(fds.hidgFd, buf, 9);
		}

		CloseDevices(fds);
		delete g_prp;
		g_prp = nullptr;

		sleep(1);
	}
}

void StopAllServices()
{	
	g_ExitRequested = true;

	bluetooth::Stop();
	g_WebApiServer.Stop();	
}

void CloseDevices(FileDescriptors& fds)
{
	close(fds.inputEventFd);
	close(fds.hidRawFd);
	close(fds.hidgFd);
}

bool OpenDevices(FileDescriptors& fds)
{
	int res = OpenKbDevice(fds.inputEventDevName, fds.inputEventFd, fds.hidRawFd);
	if (res < 0)
		return false;

	res = OpenHidgDevice(fds.hidgFd);

	return res >= 0;
}

int ForwardingLoop(IReportFilter* prp, int hidRawFd, int hidgFd)
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

		// Kbd -> PC
		if (FD_ISSET(hidRawFd, &fds))
		{
			ssize_t len = read(hidRawFd, buf, sizeof(buf));
			if (len < 0)
			{
				ErrorMsg("Error read: %s", strerror(errno));
				return -1;
			}
					
			DbgMsg("[in] -> %s", Globals::FormatBuffer(buf, len).c_str());

			len = prp->ProcessInputReport(buf, len);
			if (len > 0)
			{
				DbgMsg("[in] ~> %s", Globals::FormatBuffer(buf, len).c_str());

				len = write(hidgFd, buf, len);
				if (len < 0)
				{
					ErrorMsg("Error write: %s", strerror(errno));
					return -1;
				}
			}
		}

		// PC -> Kbd
		if (FD_ISSET(hidgFd, &fds))
		{
			ssize_t len = read(hidgFd, buf, sizeof(buf));
			if (len < 0)
			{
				ErrorMsg("Error read: %s", strerror(errno));
				return -1;
			}

			DbgMsg("[out] -> %s", Globals::FormatBuffer(buf, len).c_str());

			len = prp->ProcessOutputReport(buf, len);
			if (len > 0)
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

int OpenKbDevice(string& strDevName, int& inputEventFd, int& hidRawFd)
{
	inputEventFd = open(EVENT_DEV_PATH, O_RDWR);
	if (inputEventFd < 0)
	{
		InfoMsg("Failed to open() " EVENT_DEV_PATH ": %s", strerror(errno));
		
		return -1;
	}

	char szDevName[256] = "";
	ioctl(inputEventFd, EVIOCGNAME(sizeof(szDevName)), szDevName);
	strDevName = szDevName;

	int res = ioctl(inputEventFd, EVIOCGRAB, 1);
	if (res < 0)
	{
		InfoMsg("Failed to get exclusive access on " EVENT_DEV_PATH ": %s", strerror(errno));
		
		close(inputEventFd);
		inputEventFd = -1;

		return -1;
	}

	hidRawFd = open(HIDRAW_DEV_PATH, O_RDWR);
	if (hidRawFd < 0)
	{
		InfoMsg("Failed to open() " HIDRAW_DEV_PATH ": %s", strerror(errno));

		close(inputEventFd);
		inputEventFd = -1;

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





