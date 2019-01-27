//
// RaspiKey Copyright (c) 2019 George Samartzidis <samartzidis@gmail.com>. All rights reserved.
// You are not allowed to redistribute, modify or sell any part of this file in either 
// compiled or non-compiled form without the author's written permission.
//

#include "Globals.h"
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <sys/sysinfo.h>
#include <algorithm>
#include "Logger.h"
#include <regex>

using namespace std;

namespace Globals
{
	bool g_dwSwapAltCmd = false;
	bool g_dwSwapFnCtrl = true;
	char g_szModuleDir[PATH_MAX];
	//DevDesc g_devDesc[13] = { 
	//	{0x05ac, 0x0208, ModelId::A1314},
	//	{0x05ac, 0x0209, ModelId::A1314},
	//	{0x05ac, 0x020a, ModelId::A1314},
	//	{0x05ac, 0x022c, ModelId::A1314},
	//	{0x05ac, 0x022d, ModelId::A1314},
	//	{0x05ac, 0x022e, ModelId::A1314},
	//	{0x05ac, 0x0239, ModelId::A1314},
	//	{0x05ac, 0x023A, ModelId::A1314},
	//	{0x05ac, 0x023B, ModelId::A1314},
	//	{0x05ac, 0x0255, ModelId::A1314},
	//	{0x05ac, 0x0256, ModelId::A1314},
	//	{0x05ac, 0x0257, ModelId::A1314},
	//	{0x004c, 0x0267, ModelId::A1644}
	//};

	string FormatBuffer(const uint8_t* const buf, size_t len)
	{
		ostringstream ss;
		for (size_t i = 0; i < len; i++)
			ss << std::hex << (short)buf[i] << " ";
		ss << std::flush;

		return ss.str();
	}

	vector<string> Split(const string& s, char seperator)
	{
		vector<string> output;
		string::size_type prev_pos = 0, pos = 0;

		while ((pos = s.find(seperator, pos)) != string::npos)
		{
			string substring(s.substr(prev_pos, pos - prev_pos));
			output.push_back(substring);
			prev_pos = ++pos;
		}
		output.push_back(s.substr(prev_pos, pos - prev_pos)); //Last token

		return output;
	}

	string& Ltrim(string &s)
	{
		s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
		return s;
	}

	string& Rtrim(string &s)
	{
		s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
		return s;
	}

	string& Trim(string &s)
	{
		return Ltrim(Rtrim(s));
	}

	long GetUptime()
	{
		struct sysinfo s_info;
		int error = sysinfo(&s_info);
		if (error != 0)
			ErrorMsg("sysinfo error: %d", error);
		
		return s_info.uptime;
	}

	int GetBtHidBatteryCapacity(const string& btaddress)
	{
		string btaddresslower = btaddress;
		transform(btaddresslower.begin(), btaddresslower.end(), btaddresslower.begin(), ::tolower);

		char pfad[128];
		snprintf(pfad, 128, "/sys/class/power_supply/hid-%s-battery/capacity", btaddresslower.c_str());
		FILE* f = fopen(pfad, "r");
		if (f == nullptr)
			return -1;
		int ret = 0;
		fscanf(f, "%d", &ret);
		fclose(f);

		return ret;
	}

	bool SetPiLedState(bool state, const char* const led)
	{
		char pfad[128];
		snprintf(pfad, 128, "/sys/class/leds/%s/brightness", led);
		FILE* f = fopen(pfad, "w");
		if (f == nullptr) 
			return false;
		fprintf(f, "%i", state ? 1 : 0);
		fclose(f);

		return true;
	}

	/*
	bool GetInputDeviceInfo(const char* const szDeviceName, int& vid, int& pid, ModelId& modelId)
	{
		ifstream ifs;
		string strLine;

		const char* const szDevsPath = "/proc/bus/input/devices";
		ifs.open(szDevsPath);
		if (!ifs.is_open())
		{
			ErrorMsg("Failed to open %s", szDevsPath);
			return false;
		}

		const regex regexVidPid("^I:.*Vendor=([0-9a-fA-F]+)\\s+Product=([0-9a-fA-F]+).*", regex::ECMAScript | regex::icase);
		const regex regexName("^N:.*Name=\"([^\"]+)\"*", regex::ECMAScript | regex::icase);

		while (getline(ifs, strLine))
		{
			smatch matches;
			if (std::regex_search(strLine, matches, regexVidPid))
			{
				try
				{
					vid = stoi(matches[1].str(), nullptr, 16);
					pid = stoi(matches[2].str(), nullptr, 16);					
				}
				catch (...)
				{
					vid = pid = 0;
				}

				continue;
			}

			smatch matches2;
			if (std::regex_search(strLine, matches2, regexName))
			{
				string name = matches2[1].str();
				if (name == szDeviceName)
				{
					modelId = Globals::ModelId::Unknown;
					for (uint k = 0; k < sizeof(Globals::g_devDesc) / sizeof(Globals::DevDesc); k++)
					{
						if (Globals::g_devDesc[k].Vid == vid && Globals::g_devDesc[k].Pid == pid)
						{
							modelId = Globals::g_devDesc[k].Model;
							break;
						}
					}

					return true;
				}
			}
		}

		return false;
	}
	*/
}