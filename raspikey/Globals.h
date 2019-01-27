//
// RaspiKey Copyright (c) 2019 George Samartzidis <samartzidis@gmail.com>. All rights reserved.
// You are not allowed to redistribute, modify or sell any part of this file in either 
// compiled or non-compiled form without the author's written permission.
//

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <memory.h>
#include <sys/syslog.h>
#include <string>
#include <memory>
#include <vector>
#include <linux/limits.h>

// Global definitions
//
#define EVENT_DEV_PATH "/dev/input/event0"
#define HIDRAW_DEV_PATH "/dev/hidraw0"
#define HIDG_DEV_PATH "/dev/hidg0"

//#define A1314_DEV_NAME "Apple Wireless Keyboard"
//#define A1644_DEV_NAME "Magic Keyboard"



#define LOG_FILE_PATH "/tmp/raspikey.log"
#define VERSION "1.0.8"
//
//

namespace Globals
{	
	// Types
	//
	enum HidCodes : uint8_t
	{
		HidKeyB = 0x5,
		HidKeyP = 0x13,
		HidKeyS = 0x16,
		HidF1 = 0x3a,
		HidF2 = 0x3b,
		HidF3 = 0x3c,
		HidF4 = 0x3d,
		HidF5 = 0x3e,
		HidF6 = 0x3f,
		HidF7 = 0x40,
		HidF8 = 0x41,
		HidF9 = 0x42,
		HidF10 = 0x43,
		HidF11 = 0x44,
		HidF12 = 0x45,
		HidF13 = 0x68,
		HidF14 = 0x69,
		HidF15 = 0x6a,
		HidF16 = 0x6b,
		HidF17 = 0x6c,
		HidF18 = 0x6d,
		HidF19 = 0x6e,
		HidF20 = 0x6f,
		HidF21 = 0x70,
		HidF22 = 0x71,
		HidF23 = 0x72,
		HidF24 = 0x73,
		HidDel = 0x4c,
		HidLeft = 0x50,
		HidHome = 0x4a,
		HidRight = 0x4f,
		HidEnd = 0x4d,
		HidUp = 0x52,
		HidPgUp = 0x4b,
		HidDown = 0x51,
		HidPgDown = 0x4e,
		HidEnter = 0x28,
		HidPrtScr = 0x46,
		HidScrLck = 0x47,
		HidPauseBreak = 0x48,
		HidInsert = 0x49,
		HidLCtrlMask = 0x1,
		HidRCtrlMask = 0x10,
		HidLAltMask = 0x4,
		HidRAltMask = 0x40,
		HidLCmdMask = 0x8,
		HidRCmdMask = 0x80
	};

	enum class ModelId: uint8_t
	{
		Unknown = 0, A1314, A1644
	};

	typedef struct DevDesc {
		int Vid;
		int Pid;
		ModelId Model;
	} tagDevDesc;

	typedef struct HidgOutputReport
	{
		uint8_t ReportId;
		uint8_t Key;
	} tagHidgOutputReport;

	//
	//

	// Global variables
	//
	extern bool g_dwSwapAltCmd;
	extern bool g_dwSwapFnCtrl;
	extern std::string g_strSerial;	
	extern char g_szModuleDir[PATH_MAX];
	extern DevDesc g_devDesc[13];

	//
	//

	// Global Functions
	//
	std::string FormatBuffer(const uint8_t* const buf, size_t len);

	inline int IsZeroBuffer(uint8_t* buf, size_t size)
	{
		return buf[0] == 0 && !memcmp(buf, buf + 1, size - 1);
	}

	template<typename ... Args> std::string FormatString(const char* const format, Args ... args)
	{
		size_t size = snprintf(nullptr, 0, format, args ...) + 1; // Extra space for '\0'
		std::unique_ptr<char[]> buf(new char[size]);

		snprintf(buf.get(), size, format, args ...);
		std::string res = std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside

		return res;
	}

	std::vector<std::string> Split(const std::string& s, char seperator);
	std::string& Ltrim(std::string &s);
	std::string& Rtrim(std::string &s);
	std::string& Trim(std::string &s);

	int GetBtHidBatteryCapacity(const std::string& btaddress);
	long GetUptime();
	bool SetPiLedState(bool state, const char* const led = "0");

	bool GetInputDeviceInfo(const char* const szDeviceName, int& vid, int& pid, ModelId& modelId);


	//
	//
}

