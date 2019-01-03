//
// RaspiKey Copyright (c) 2019 George Samartzidis <samartzidis@gmail.com>. All rights reserved.
// You are not allowed to redistribute, modify or sell any part of this file in either 
// compiled or non-compiled form without the author's written permission.
//

#pragma once

#include <string>
#include "../gdbus/gdbus.h"

namespace bluetooth
{	
	dbus_bool_t AgentCompletion();
	void AgentConfirmResponse(DBusConnection* conn, const char* input);
	DBusMessage* OnDisplayPasskey(DBusConnection* conn, DBusMessage* msg, void* userdata);
	DBusMessage* OnDisplayPinCode(DBusConnection* conn, DBusMessage* msg, void* userdata);
	DBusMessage* OnRequestConfirmation(DBusConnection* conn, DBusMessage* msg, void* userdata);
	void RegisterAgentSetup(DBusMessageIter* iter, void* userdata);
	void RegisterAgentReply(DBusMessage* message, void* userdata);
	void AgentRegister(DBusConnection* conn, GDBusProxy* manager);
	void UnregisterAgentSetup(DBusMessageIter* iter, void* userdata);
	void UnregisterAgentReply(DBusMessage* message, void* userdata);
	void AgentUnregister(DBusConnection* conn, GDBusProxy* manager);
	void RequestDefaultSetup(DBusMessageIter* iter, void* userdata);
	void RequestDefaultReply(DBusMessage* message, void* userdata);
	void AgentDefault(DBusConnection* conn, GDBusProxy* manager);
}
