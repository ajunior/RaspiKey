//
// RaspiKey Copyright (c) 2019 George Samartzidis <samartzidis@gmail.com>. All rights reserved.
// You are not allowed to redistribute, modify or sell any part of this file in either 
// compiled or non-compiled form without the author's written permission.
//

#pragma once

#include <string> 
#include "ReportFilter.h"

typedef struct FileDescriptors
{
	int inputEventFd = 0;
	int hidgFd = 0;
	int hidRawFd = 0;
	std::string inputEventDevName;
} tagFileDescriptors;

int ForwardingLoop(IReportFilter* prp, int hidRawFd, int hidgFd);

int OpenKbDevice(std::string& strDevName, int& inputEventFd, int& hidRawFd);
int OpenHidgDevice(int& hidgFd);

bool OpenDevices(FileDescriptors& fds);
void CloseDevices(FileDescriptors& fds);
void SignalHandler(int signo);
void StopAllServices();
void PollDevicesLoop();
bool StartServices();
