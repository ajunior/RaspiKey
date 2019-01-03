//
// RaspiKey Copyright (c) 2019 George Samartzidis <samartzidis@gmail.com>. All rights reserved.
// You are not allowed to redistribute, modify or sell any part of this file in either 
// compiled or non-compiled form without the author's written permission.
//

#pragma once

#include <string> 
#include "ReportFilter.h"

typedef struct DevFileDescriptors
{
	int hidgKbFd = 0;
	int appleKbInputEventFd = 0;
	int appleKbHidrawFd = 0;
	std::string appleKbInputEventDevName;
} tagDevFileDescriptors;

int ForwardingLoop(IReportFilter* prp, int appleKbHidrawFd, int hidgKbFd);
int OpenAppleKbDevice(std::string& strDevName, int& appleKbInputEventFd, int& appleKbHidrawFd);
int OpenHidgKbDevice(int& hidgKbFd);
bool OpenDevices(DevFileDescriptors& fds);
void CloseDevices(DevFileDescriptors& fds);
void SignalHandler(int signo);
void StopAllServices();
void PollDevicesLoop();
bool StartServices();
