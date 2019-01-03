//
// RaspiKey Copyright (c) 2019 George Samartzidis <samartzidis@gmail.com>. All rights reserved.
// You are not allowed to redistribute, modify or sell any part of this file in either 
// compiled or non-compiled form without the author's written permission.
//

#include <stdio.h>
#include <stdlib.h>
#include <mutex>
#include "agent.h"
#include <unistd.h>
#include "bt.h"
#include "../Globals.h"
#include "../Logger.h"

#define AGENT_PATH "/org/bluez/agent"
#define AGENT_CAPABILITY ""
#define AGENT_INTERFACE "org.bluez.Agent1"

using namespace std;

namespace bluetooth
{
	gboolean g_AgentRegistered = FALSE;
	DBusMessage* g_PendingMessage = nullptr;
	
	const GDBusMethodTable g_AgentMethods[] =
	{

		{ "DisplayPinCode" , OnDisplayPinCode, (GDBusMethodFlags)0, 0, GDBUS_ARGS({ "device", "o" },{ "pincode", "s" }),  nullptr },

		{ "DisplayPasskey", OnDisplayPasskey, (GDBusMethodFlags)0, 0, GDBUS_ARGS({ "device", "o" },{ "passkey", "u" }, { "entered", "q" }), nullptr },

		{ "RequestConfirmation" , OnRequestConfirmation, G_DBUS_METHOD_FLAG_ASYNC, 0, GDBUS_ARGS({ "device", "o" },{ "passkey", "u" }),  nullptr },
		{}
	};

	dbus_bool_t AgentCompletion()
	{
		if (!g_PendingMessage)
			return FALSE;

		return TRUE;
	}

	void AgentConfirmResponse(DBusConnection* conn, const char* input)
	{
		if (!strcmp(input, "yes"))
			g_dbus_send_reply(conn, g_PendingMessage, DBUS_TYPE_INVALID);
		else if (!strcmp(input, "no"))
			g_dbus_send_error(conn, g_PendingMessage, "org.bluez.Error.Rejected", nullptr);
		else
			g_dbus_send_error(conn, g_PendingMessage, "org.bluez.Error.Canceled", nullptr);
	}

	DBusMessage* OnDisplayPasskey(DBusConnection* conn, DBusMessage* msg, void* userdata)
	{
		DbgMsg("OnDisplayPasskey()");

		const char *device;
		dbus_uint32_t passkey;
		dbus_uint16_t entered;
		

		dbus_message_get_args(msg, nullptr, DBUS_TYPE_OBJECT_PATH, &device,
			DBUS_TYPE_UINT32, &passkey, DBUS_TYPE_UINT16, &entered,
			DBUS_TYPE_INVALID);
		
		char passkey_full[7];
		snprintf(passkey_full, sizeof(passkey_full), "%.6u", passkey);
		passkey_full[6] = '\0';

		InfoMsg("Passkey: %s", passkey_full);
		InfoMsg("Entered: %d", entered);

		//if (entered > strlen(passkey_full))
		//	entered = strlen(passkey_full);

		//rl_printf(AGENT_PROMPT "Passkey: "
		//	COLOR_BOLDGRAY "%.*s" COLOR_BOLDWHITE "%s\n" COLOR_OFF,
		//	entered, passkey_full, passkey_full + entered);

		if(entered == 0)
			bluetooth::OnAgentNotify(passkey_full);
		

		return dbus_message_new_method_return(msg);
	}

	DBusMessage* OnDisplayPinCode(DBusConnection* conn, DBusMessage* msg, void* userdata)
	{
		DbgMsg("OnDisplayPinCode()");

		const char* device;
		const char* pincode;

		dbus_message_get_args(msg, nullptr, DBUS_TYPE_OBJECT_PATH, &device, DBUS_TYPE_STRING, &pincode, DBUS_TYPE_INVALID);

		InfoMsg("PIN code: %s", pincode);

		bluetooth::OnAgentNotify(pincode);

		return dbus_message_new_method_return(msg);
	}

	DBusMessage* OnRequestConfirmation(DBusConnection* conn, DBusMessage* msg, void* userdata)
	{
		DbgMsg("OnRequestConfirmation()");

		const char *device;
		dbus_uint32_t passkey;
		char *str;

		dbus_message_get_args(msg, nullptr, DBUS_TYPE_OBJECT_PATH, &device, DBUS_TYPE_UINT32, &passkey, DBUS_TYPE_INVALID);

		str = g_strdup_printf("Confirm passkey %06u (yes/no): ", passkey);
		InfoMsg(str);
		g_free(str);

		g_PendingMessage = dbus_message_ref(msg);

		// Autoconfirm
		g_dbus_send_reply(conn, g_PendingMessage, DBUS_TYPE_INVALID);

		bluetooth::OnAgentNotify("");

		return nullptr;
	}

	void RegisterAgentSetup(DBusMessageIter* iter, void* userdata)
	{
		const char *path = AGENT_PATH;
		const char *capability = AGENT_CAPABILITY;

		dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &path);
		dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &capability);
	}

	void RegisterAgentReply(DBusMessage* message, void* userdata)
	{
		DBusConnection *conn = (DBusConnection *)userdata;
		DBusError error;

		dbus_error_init(&error);

		if (dbus_set_error_from_message(&error, message) == FALSE)
		{
			g_AgentRegistered = TRUE;
			InfoMsg("Agent registered");
		}
		else
		{
			ErrorMsg("Failed to register agent: %s", error.name);
			dbus_error_free(&error);

			if (g_dbus_unregister_interface(conn, AGENT_PATH, AGENT_INTERFACE) == FALSE)
				ErrorMsg("Failed to unregister agent object");
		}
	}

	void AgentRegister(DBusConnection* conn, GDBusProxy* manager)
	{
		if (g_AgentRegistered == TRUE)
		{
			ErrorMsg("Agent is already registered");
			return;
		}

		if (g_dbus_register_interface(conn, AGENT_PATH, AGENT_INTERFACE, g_AgentMethods, nullptr, nullptr, nullptr, nullptr) == FALSE)
		{
			ErrorMsg("Failed to register agent object");
			return;
		}

		if (g_dbus_proxy_method_call(
			manager, 
			"RegisterAgent",
			RegisterAgentSetup,
			RegisterAgentReply,
			conn, nullptr) == FALSE)
		{
			ErrorMsg("Failed to call register agent method");
			return;
		}
	}

	void UnregisterAgentSetup(DBusMessageIter* iter, void* userdata)
	{
		const char *path = AGENT_PATH;

		dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &path);
	}

	void UnregisterAgentReply(DBusMessage* message, void* userdata)
	{
		DBusConnection *conn = (DBusConnection *)userdata;
		DBusError error;

		dbus_error_init(&error);

		if (dbus_set_error_from_message(&error, message) == FALSE)
		{
			g_AgentRegistered = FALSE;
			InfoMsg("Agent unregistered");

			if (g_dbus_unregister_interface(conn, AGENT_PATH, AGENT_INTERFACE) == FALSE)
				ErrorMsg("Failed to unregister agent object");
		}
		else
		{
			ErrorMsg("Failed to unregister agent: %s", error.name);
			dbus_error_free(&error);
		}
	}

	void AgentUnregister(DBusConnection* conn, GDBusProxy* manager)
	{
		if (g_AgentRegistered == FALSE) {
			InfoMsg("No agent is registered");
			return;
		}

		if (g_dbus_proxy_method_call(
			manager, 
			"UnregisterAgent",
			UnregisterAgentSetup,
			UnregisterAgentReply,
			conn, 
			nullptr) == FALSE)
		{
			ErrorMsg("Failed to call unregister agent method");
			return;
		}
	}

	void RequestDefaultSetup(DBusMessageIter* iter, void* userdata)
	{
		const char *path = AGENT_PATH;

		dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &path);
	}

	void RequestDefaultReply(DBusMessage* message, void* userdata)
	{
		DBusError error;

		dbus_error_init(&error);

		if (dbus_set_error_from_message(&error, message) == TRUE) {
			ErrorMsg("Failed to request default agent: %s", error.name);
			dbus_error_free(&error);
			return;
		}

		InfoMsg("Default agent request successful");
	}

	void AgentDefault(DBusConnection* conn, GDBusProxy* manager)
	{
		if (g_AgentRegistered == FALSE)
		{
			InfoMsg("No agent is registered");
			return;
		}

		if (g_dbus_proxy_method_call(manager, "RequestDefaultAgent",
			RequestDefaultSetup,
			RequestDefaultReply,
			nullptr, nullptr) == FALSE)
		{
			ErrorMsg("Failed to call request default agent method");
			return;
		}
	}
}