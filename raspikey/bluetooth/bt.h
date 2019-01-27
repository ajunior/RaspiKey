//
// RaspiKey Copyright (c) 2019 George Samartzidis <samartzidis@gmail.com>. All rights reserved.
// You are not allowed to redistribute, modify or sell any part of this file in either 
// compiled or non-compiled form without the author's written permission.
//

#pragma once

#include <string>
#include <vector>

namespace bluetooth
{
	typedef struct BtDevice
	{
		std::string Name;
		std::string Alias;
		std::string Modalias;
		std::string Address;
		std::string Icon;
		bool Paired;
		bool Connected;
		bool Trusted;	
	} tagBtDevice;

	typedef struct BtDeviceInfo
	{
		std::string Name;
		std::string Address;		
		std::string Alias;
		std::string Class;
		std::string Appearance;
		std::string Icon;
		std::string Paired;
		std::string Trusted;
		std::string Blocked;
		std::string Connected;
		std::string LegacyPairing;
		std::string Modalias;
	} tagBtDeviceInfo;

	enum class ReplyMessageStatus { Fail, Success, PinCode };
	typedef struct ReplyMessage
	{
		ReplyMessageStatus Status;
		std::string Data;

		ReplyMessage(const ReplyMessageStatus status = ReplyMessageStatus::Fail, const std::string& data = "")
		{
			Status = status;
			Data = data;
		}
	} tagReplyMessage;

	void OnAgentNotify(const std::string& data);

	void Start();
	void Stop();
	bool IsStarted();
	bool GetDiscovery();
	bool SetPower(bool on);
	void SetDiscovery(bool on);
	void TrustDevice(const char* address);
	std::vector<BtDevice> GetDevices();

	void GetDeviceInfo(const char *arg, BtDeviceInfo& dd);

	ReplyMessage BeginPairDevice(const char* address);
	ReplyMessage EndPairDevice();
	void ConnectDevice(const char* address);
	void DisconnectDevice(const char* address);
	void RemoveDevice(const char* address);
	void NotifyCommandCompletion(const ReplyMessage& msg);
}


