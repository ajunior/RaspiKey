//
// RaspiKey Copyright (c) 2019 George Samartzidis <samartzidis@gmail.com>. All rights reserved.
// You are not allowed to redistribute, modify or sell any part of this file in either 
// compiled or non-compiled form without the author's written permission.
//

#pragma once

#include <string> 
#include "ReportFilter.h"

typedef struct DeviceDescriptors
{
	int inputEventFd = 0; // device: /dev/input/eventX
	int hidRawFd = 0; // device: /dev/hidrawX

	int hidgFd = 0; // device: /dev/hidgX
	
	std::string inputEventDevName;
	std::string inputEventDevPhys;
	std::string inputEventDevUniq;
} tagDeviceDescriptors;

int ForwardingLoop(IReportFilter** prps, int hidRawFd, int hidgFd);

int OpenKbDevice(DeviceDescriptors& fds);
int OpenHidgDevice(int& hidgFd);

void SignalHandler(int signo);
void StopAllServices();
void OpenDevicesLoop();
bool StartServices();

std::string GetKeymapFilePath(const std::string& addr);
bool HasKeyMap(const std::string& addr);
bool DeleteKeyMap(const std::string& addr);
void SetKeyMap(const std::string& addr, const std::string& json);
std::string GetKeyMap(const std::string& addr);

