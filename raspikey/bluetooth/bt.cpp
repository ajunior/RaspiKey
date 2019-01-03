//
// RaspiKey Copyright (c) 2019 George Samartzidis <samartzidis@gmail.com>. All rights reserved.
// You are not allowed to redistribute, modify or sell any part of this file in either 
// compiled or non-compiled form without the author's written permission.
//

#include "bt.h"

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <glib.h>
#include <iostream>
#include <vector>
#include "agent.h" 
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <sstream>
#include <queue>
#include "../Globals.h"
#include "../Logger.h"

using namespace std;

namespace bluetooth
{
	// Global vars
	//
	GMainLoop* g_mainLoop = nullptr;
	DBusConnection* g_dbusConn = nullptr;
	GDBusProxy* g_agentManager = nullptr;
	GDBusProxy* g_defaultCtrl = nullptr;
	GList* g_ctrlList = nullptr;
	GList* g_devList = nullptr;
	thread* g_pMainThread = nullptr;
	mutex g_mutex;
	ReplyMessage g_pairingReplyMessage;
	bool g_bHasPairingReplyMessage = false;
	bool g_bIsPairing = false;

	void ThrowRuntimeError(const string& msg)
	{
		ErrorMsg(msg.c_str());
		throw runtime_error(msg);
	}

	void CheckDefaultCtrlOrThrow()
	{
		if (!g_defaultCtrl)
			ThrowRuntimeError("No default controller available");
	}
	
	GDBusProxy* FindProxyByAddress(GList* source, const char* address)
	{
		for (GList* list = g_list_first(source); list; list = g_list_next(list))
		{
			GDBusProxy *proxy = (GDBusProxy*)list->data;
			DBusMessageIter iter;
			const char *str;

			if (g_dbus_proxy_get_property(proxy, "Address", &iter) == FALSE)
				continue;

			dbus_message_iter_get_basic(&iter, &str);

			if (!strcmp(str, address))
				return proxy;
		}

		return nullptr;
	}

	string GetPropertyValue(GDBusProxy* proxy, const char* name)
	{
		DBusMessageIter iter;

		if (g_dbus_proxy_get_property(proxy, name, &iter) == FALSE)
			return "";

		char val[256] = { 0 };

		dbus_bool_t valbool;
		dbus_uint32_t valu32;
		dbus_uint16_t valu16;
		dbus_int16_t vals16;
		const char *valstr;

		switch (dbus_message_iter_get_arg_type(&iter))
		{
		case DBUS_TYPE_INVALID:
			sprintf(val, "%s is invalid\n", name);
			break;
		case DBUS_TYPE_STRING:
		case DBUS_TYPE_OBJECT_PATH:
			dbus_message_iter_get_basic(&iter, &valstr);
			sprintf(val, "%s", valstr);
			break;
		case DBUS_TYPE_BOOLEAN:
			dbus_message_iter_get_basic(&iter, &valbool);
			sprintf(val, "%s", valbool == TRUE ? "yes" : "no");
			break;
		case DBUS_TYPE_UINT32:
			dbus_message_iter_get_basic(&iter, &valu32);
			sprintf(val, "0x%06x", valu32);
			break;
		case DBUS_TYPE_UINT16:
			dbus_message_iter_get_basic(&iter, &valu16);
			sprintf(val, "0x%04x", valu16);
			break;
		case DBUS_TYPE_INT16:
			dbus_message_iter_get_basic(&iter, &vals16);
			sprintf(val, "%d", vals16);
			break;
		default:
			sprintf(val, "has unsupported type");
			break;
		}

		return val;
	}

	void GetDeviceInfo(const char* const address, BtDeviceInfo& dd)
	{
		DbgMsg("GetDeviceInfo()");

		if (!address || !strlen(address))
			ThrowRuntimeError("Missing required device address argument");

		GDBusProxy* proxy = FindProxyByAddress(g_devList, address);
		if (!proxy)
		{
			string msg = Globals::FormatString("Device %s not available", address);
			ThrowRuntimeError(msg);
		}

		DBusMessageIter iter;
		if (g_dbus_proxy_get_property(proxy, "Address", &iter) == FALSE)
			ThrowRuntimeError("Could not retrieve device address");

		const char* address_ = nullptr;
		dbus_message_iter_get_basic(&iter, &address_);

		dd.Address = address_;
		dd.Name = GetPropertyValue(proxy, "Name");
		dd.Alias = GetPropertyValue(proxy, "Alias");
		dd.Class = GetPropertyValue(proxy, "Class");
		dd.Appearance = GetPropertyValue(proxy, "Appearance");
		dd.Icon = GetPropertyValue(proxy, "Icon");
		dd.Paired = GetPropertyValue(proxy, "Paired");
		dd.Trusted = GetPropertyValue(proxy, "Trusted");
		dd.Blocked = GetPropertyValue(proxy, "Blocked");
		dd.Connected = GetPropertyValue(proxy, "Connected");
		dd.LegacyPairing = GetPropertyValue(proxy, "LegacyPairing");
		dd.Modalias = GetPropertyValue(proxy, "Modalias");
	}

	bool GetDeviceInfo(GDBusProxy* proxy, BtDevice& dev)
	{
		DBusMessageIter iter;

		if (g_dbus_proxy_get_property(proxy, "Address", &iter) == FALSE)
			return false;

		const char* address;
		dbus_message_iter_get_basic(&iter, &address);
		dev.Address = address;

		const char* name;
		if (g_dbus_proxy_get_property(proxy, "Alias", &iter) == TRUE)
			dbus_message_iter_get_basic(&iter, &name);
		else
			name = "<unknown>";
		dev.Name = name;

		const char* icon;
		if (g_dbus_proxy_get_property(proxy, "Icon", &iter) == TRUE)
			dbus_message_iter_get_basic(&iter, &icon);
		else
			icon = "";
		dev.Icon = icon;

		dev.Paired = false;
		if (g_dbus_proxy_get_property(proxy, "Paired", &iter) == TRUE)
		{
			dbus_bool_t paired;
			dbus_message_iter_get_basic(&iter, &paired);
			dev.Paired = paired;
		}

		dev.Connected = false;
		if (g_dbus_proxy_get_property(proxy, "Connected", &iter) == TRUE)
		{
			dbus_bool_t connected;
			dbus_message_iter_get_basic(&iter, &connected);
			dev.Connected = connected;
		}

		dev.Trusted = false;
		if (g_dbus_proxy_get_property(proxy, "Trusted", &iter) == TRUE)
		{
			dbus_bool_t trusted;
			dbus_message_iter_get_basic(&iter, &trusted);
			dev.Trusted = trusted;
		}

		return true;
	}

	vector<BtDevice> GetDevices()
	{
		vector<BtDevice> reply;
		for (auto list = g_list_first(g_devList); list; list = g_list_next(list))
		{
			GDBusProxy* proxy = (GDBusProxy*)list->data;

			BtDevice dev;
			GetDeviceInfo(proxy, dev);

			reply.push_back(dev);
		}

		return reply;
	}

	void GenericCallback(const DBusError* error, void* userdata)
	{
		char* str = (char*)userdata;

		if (dbus_error_is_set(error))
		{
			ErrorMsg("Failed to set %s: %s", str, error->name);
		}
		else
		{
			InfoMsg("Changing %s succeeded", str);
		}
	}

	bool SetPower(bool on)
	{
		InfoMsg("SetPower()");

		CheckDefaultCtrlOrThrow();

		char* str = g_strdup_printf("power %s", on ? "on" : "off");
		dbus_bool_t data = (dbus_bool_t)on;
		bool retval = false;
		if (g_dbus_proxy_set_property_basic(g_defaultCtrl, "Powered", DBUS_TYPE_BOOLEAN, &data, GenericCallback, str, g_free) == TRUE)					
			retval = true;
		
		g_free(str);
		return retval;
	}	

	void SetAgent(bool on)
	{
		InfoMsg("SetAgent()");

		if (on)
		{
			if (g_agentManager)
				bluetooth::AgentRegister(g_dbusConn, g_agentManager);
			else
				InfoMsg("Agent registration enabled\n");
		}
		else
		{
			if (g_agentManager)
				bluetooth::AgentUnregister(g_dbusConn, g_agentManager);
			else
				InfoMsg("Agent registration disabled");
		}
	}

	bool GetDiscovery()
	{
		InfoMsg("GetDiscovery()");

		CheckDefaultCtrlOrThrow();

		DBusMessageIter iter;
		if (g_dbus_proxy_get_property(g_defaultCtrl, "Discovering", &iter) == FALSE)
			ThrowRuntimeError("Could not query discovering mode");

		dbus_bool_t valbool;
		dbus_message_iter_get_basic(&iter, &valbool);

		return valbool;
	}

	void SetDiscovery(bool on)
	{		
		InfoMsg("SetDiscovery()");

		CheckDefaultCtrlOrThrow();

		const char* method = nullptr;
		if (on)
			method = "StartDiscovery";
		else
			method = "StopDiscovery";

		DBusError error;
		dbus_error_init(&error);
		dbus_bool_t data = (dbus_bool_t)on;
		DBusMessage* message = g_dbus_proxy_method_call_blocking(g_defaultCtrl, method, NULL, GUINT_TO_POINTER(data), &error);
		if (message == NULL)
		{
			string msg = Globals::FormatString("Failed to %s discovery: %s", data == TRUE ? "start" : "stop", error.name);
			dbus_error_free(&error);

			ErrorMsg(msg.c_str());

			ThrowRuntimeError(msg);
		}				
	}

	void OnPairDeviceReply(DBusMessage* message, void* userdata)
	{
		InfoMsg("OnPairDeviceReply()");

		{
			lock_guard<mutex> guard(g_mutex);

			if (!g_bIsPairing)
				return;
			g_bIsPairing = false;
		}

		DBusError error;
		dbus_error_init(&error);
		if (dbus_set_error_from_message(&error, message) == TRUE)
		{
			string msg = Globals::FormatString("Pairing failed: %s", error.name);
			dbus_error_free(&error);

			ErrorMsg(msg.c_str());

			g_pairingReplyMessage = ReplyMessage(ReplyMessageStatus::Fail, msg);
			g_bHasPairingReplyMessage = true;
		}
		else
		{
			string msg = "Pairing successful";
			InfoMsg(msg.c_str());

			g_pairingReplyMessage = ReplyMessage(ReplyMessageStatus::Success, msg);
			g_bHasPairingReplyMessage = true;
		}
	}

	ReplyMessage BeginPairDevice(const char* address)
	{	
		InfoMsg("BeginPairDevice()");
		
		{
			lock_guard<mutex> guard(g_mutex);

			if (g_bIsPairing)
				ThrowRuntimeError("Already pairing with a device");
			g_bIsPairing = true;
		}

		if (!address || !strlen(address))
		{
			string msg = "Missing required device address argument";
			ThrowRuntimeError(msg);
		}

		GDBusProxy* proxy = FindProxyByAddress(g_devList, address);
		if (!proxy)
			ThrowRuntimeError("Device not available");

		g_bHasPairingReplyMessage = false;

		if (g_dbus_proxy_method_call(proxy, "Pair", NULL, OnPairDeviceReply, NULL, NULL) == FALSE)
			ThrowRuntimeError("Failed to pair");

		InfoMsg("Started pairing with %s, now waiting for agent notification.", address);

		int count = 0;
		while (!g_bHasPairingReplyMessage && ++count < 100) // Wait 10 sec for agent reply
			usleep(100 * 1000); // 100msec

		if (g_bHasPairingReplyMessage)
		{
			g_bHasPairingReplyMessage = false;
			return g_pairingReplyMessage;
		}

		g_bHasPairingReplyMessage = false;
		return ReplyMessage(ReplyMessageStatus::Fail, "Pairing operation Timed-out");
	}
	
	void OnAgentNotify(const string& data)
	{
		InfoMsg("OnAgentNotify()");

		if (!g_bIsPairing)
			return;

		if(data != "")
			g_pairingReplyMessage = ReplyMessage(ReplyMessageStatus::PinCode, data);
		else
			g_pairingReplyMessage = ReplyMessage(ReplyMessageStatus::Success);

		g_bHasPairingReplyMessage = true;
	}

	ReplyMessage EndPairDevice()
	{
		InfoMsg("EndPairDevice()");

		ReplyMessage msg;
		int count = 0;

		while (!g_bHasPairingReplyMessage && ++count < 300) // 30sec in total
			usleep(100 * 1000); // 100msec
		if (g_bHasPairingReplyMessage)
			return g_pairingReplyMessage;

		return ReplyMessage(ReplyMessageStatus::Fail, "Pairing operation Timed-out");

		//return ReplyMessage(ReplyMessageStatus::Success, "Success");
	}

	void TrustDevice(const char* address)
	{
		InfoMsg("TrustDevice()");

		if (!address || !strlen(address))
			ThrowRuntimeError("Missing device address argument");

		GDBusProxy* proxy = FindProxyByAddress(g_devList, address);
		if (!proxy)
		{
			string msg = Globals::FormatString("Device %s not available", address);
			ThrowRuntimeError(msg);
		}

		dbus_bool_t trusted = TRUE;
		char* str = g_strdup_printf("%s trust", address);
		if (g_dbus_proxy_set_property_basic(proxy, "Trusted", DBUS_TYPE_BOOLEAN, &trusted, GenericCallback, str, g_free) == TRUE)
			return;
		g_free(str);
	}

	void RemoveDeviceSetup(DBusMessageIter* iter, void* userdata)
	{
		const char *path = (const char*)userdata;

		dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &path);
	}

	void RemoveDevice(const char* arg)
	{
		InfoMsg("RemoveDevice()");

		if (!arg || !strlen(arg))
			ThrowRuntimeError("Missing device address argument");

		CheckDefaultCtrlOrThrow();

		GDBusProxy* proxy = FindProxyByAddress(g_devList, arg);
		if (!proxy)
		{
			string msg = Globals::FormatString("Device %s not available", arg);
			ThrowRuntimeError(msg);
		}

		DBusError error;
		dbus_error_init(&error);
		char* path = g_strdup(g_dbus_proxy_get_path(proxy));


		//DBusMessage* message = g_dbus_proxy_method_call_blocking(g_defaultCtrl, "RemoveDevice", RemoveDeviceSetup, path, &error);
		//if (message == NULL)
		//{
		//	g_free(path);
		//	string msg = Globals::FormatString("Failed to remove device: %s", error.name);
		//	dbus_error_free(&error);
		//	ErrorMsg(msg.c_str());
		//	ThrowRuntimeError(msg);
		//}

		// TODO: Fix to avoid occasional deadlock
		//
		if (g_dbus_proxy_method_call(g_defaultCtrl, "RemoveDevice", RemoveDeviceSetup, nullptr, path, g_free) == 0)
		{
			g_free(path);

			string msg = Globals::FormatString("Failed to remove device: %s", error.name);
			dbus_error_free(&error);

			ErrorMsg(msg.c_str());
			ThrowRuntimeError(msg);
		}
	}

	void ConnectDevice(const char* address)
	{
		InfoMsg("ConnectDevice()");

		if (!address || !strlen(address))
			ThrowRuntimeError("Missing device address argument");

		GDBusProxy* proxy = FindProxyByAddress(g_devList, address);
		if (!proxy)
		{
			string msg = Globals::FormatString("Device %s not available", address);
			ThrowRuntimeError(msg);
		}

		DBusError error;
		dbus_error_init(&error);
		DBusMessage* message = g_dbus_proxy_method_call_blocking(proxy, "Connect", NULL, NULL, &error);
		if (message == NULL)
		{
			string msg = Globals::FormatString("Failed to connect: %s", error.name);
			dbus_error_free(&error);

			ErrorMsg(msg.c_str());

			ThrowRuntimeError(msg);
		}
	}

	void DisconnectDevice(const char* address)
	{
		DbgMsg("DisconnectDevice()");

		if (!address || !strlen(address))
			ThrowRuntimeError("Missing device address argument");

		GDBusProxy* proxy = FindProxyByAddress(g_devList, address);
		if (!proxy)
		{
			string msg = Globals::FormatString("Device %s not available", address);
			ThrowRuntimeError(msg);
		}

		DBusError error;
		dbus_error_init(&error);
		DBusMessage* message = g_dbus_proxy_method_call_blocking(proxy, "Disconnect", NULL, NULL, &error);
		if (message == NULL)
		{
			string msg = Globals::FormatString("Failed to disconnect: %s", error.name);
			dbus_error_free(&error);

			ErrorMsg(msg.c_str());

			ThrowRuntimeError(msg);
		}
	}

	void OnProxyLeak(gpointer data)
	{
		ErrorMsg("Leaking proxy %p\n", data);
	}

	void OnConnect(DBusConnection* connection, void* userdata)
	{
		InfoMsg("OnConnect()");
	}

	void OnDisconnect(DBusConnection* connection, void* userdata)
	{
		InfoMsg("OnDisconnect()");

		g_list_free(g_ctrlList);
		g_ctrlList = nullptr;

		g_defaultCtrl = nullptr;

		g_list_free(g_devList);
		g_devList = nullptr;
	}

	void PrintAdapter(GDBusProxy* proxy, const char* status)
	{
		DBusMessageIter iter;
		const char *address, *name;

		if (g_dbus_proxy_get_property(proxy, "Address", &iter) == FALSE)
			return;

		dbus_message_iter_get_basic(&iter, &address);

		if (g_dbus_proxy_get_property(proxy, "Alias", &iter) == TRUE)
			dbus_message_iter_get_basic(&iter, &name);
		else
			name = "<unknown>";

		DbgMsg("%s adapter: address %s, name %s, default_ctrl: %s", status, address, name, g_defaultCtrl == proxy ? "yes" : "no");
	}

	void PrintDevice(GDBusProxy* proxy, const char* status)
	{
		DBusMessageIter iter;
		const char *address, *name;

		if (g_dbus_proxy_get_property(proxy, "Address", &iter) == FALSE)
			return;

		dbus_message_iter_get_basic(&iter, &address);

		if (g_dbus_proxy_get_property(proxy, "Alias", &iter) == TRUE)
			dbus_message_iter_get_basic(&iter, &name);
		else
			name = "<unknown>";

		DbgMsg("%s device: address %s, name %s", status, address, name);
	}

	gboolean DeviceIsChild(GDBusProxy* device, GDBusProxy* master)
	{
		DBusMessageIter iter;
		const char *adapter, *path;

		if (!master)
			return FALSE;

		if (g_dbus_proxy_get_property(device, "Adapter", &iter) == FALSE)
			return FALSE;

		dbus_message_iter_get_basic(&iter, &adapter);
		path = g_dbus_proxy_get_path(master);

		if (!strcmp(path, adapter))
			return TRUE;

		return FALSE;
	}

	void OnProxyAdded(GDBusProxy* proxy, void* userdata)
	{
		const char* intf = g_dbus_proxy_get_interface(proxy);

		if (!strcmp(intf, "org.bluez.Device1"))
		{
			DbgMsg("OnProxyAdded(): org.bluez.Device1");

			if (DeviceIsChild(proxy, g_defaultCtrl) == TRUE)
			{
				g_devList = g_list_append(g_devList, proxy);

				PrintDevice(proxy, "NEW");
			}
		}
		else if (!strcmp(intf, "org.bluez.Adapter1"))
		{
			DbgMsg("OnProxyAdded(): org.bluez.Adapter1");

			g_ctrlList = g_list_append(g_ctrlList, proxy);

			if (!g_defaultCtrl)
			{
				// Default ctrl established
				g_defaultCtrl = proxy;		

				// Auto-register agent agent
				SetAgent(true);
			}

			PrintAdapter(proxy, "NEW");
		}
		else if (!strcmp(intf, "org.bluez.AgentManager1"))
		{
			DbgMsg("OnProxyAdded(): org.bluez.AgentManager1");

			if (!g_agentManager)
				g_agentManager = proxy;
		}
	}

	void OnProxyRemoved(GDBusProxy* proxy, void* userdata)
	{
		const char* intf = g_dbus_proxy_get_interface(proxy);

		if (!strcmp(intf, "org.bluez.Device1"))
		{
			if (DeviceIsChild(proxy, g_defaultCtrl) == TRUE)
			{
				g_devList = g_list_remove(g_devList, proxy);

				PrintDevice(proxy, "DEL");
			}
		}
		else if (!strcmp(intf, "org.bluez.Adapter1"))
		{
			g_ctrlList = g_list_remove(g_ctrlList, proxy);

			PrintAdapter(proxy, "DEL");

			if (g_defaultCtrl == proxy) {
				g_defaultCtrl = nullptr;

				g_list_free(g_devList);
				g_devList = nullptr;
			}
		}
		else if (!strcmp(intf, "org.bluez.AgentManager1"))
		{
			if (g_agentManager == proxy)
				g_agentManager = nullptr;
		}
	}

	void MainThread()
	{
		g_mainLoop = g_main_loop_new(NULL, FALSE);
		g_dbusConn = g_dbus_setup_bus(DBUS_BUS_SYSTEM, NULL, NULL);

		GDBusClient* client = g_dbus_client_new(g_dbusConn, "org.bluez", "/org/bluez");

		g_dbus_client_set_connect_watch(client, OnConnect, NULL);
		g_dbus_client_set_disconnect_watch(client, OnDisconnect, NULL);
		g_dbus_client_set_proxy_handlers(client, OnProxyAdded, OnProxyRemoved, NULL, NULL);

		g_main_loop_run(g_mainLoop);

		g_dbus_client_unref(client);

		dbus_connection_unref(g_dbusConn);
		g_main_loop_unref(g_mainLoop);

		g_list_free_full(g_ctrlList, OnProxyLeak);
		g_list_free_full(g_devList, OnProxyLeak);

		g_pMainThread = nullptr;
	}

	bool IsStarted()
	{
		return g_pMainThread != nullptr;
	}

	void Stop()
	{
		if (!g_pMainThread)
			return;

		g_main_loop_quit(g_mainLoop);

		g_pMainThread->join();
		delete g_pMainThread;
		g_pMainThread = nullptr;
	}
	 
	void Start()
	{
		if (g_pMainThread)
			return;

		// Start MainThread
		g_pMainThread = new thread(&MainThread);
	}
}


