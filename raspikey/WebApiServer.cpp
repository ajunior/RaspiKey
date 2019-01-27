//
// RaspiKey Copyright (c) 2019 George Samartzidis <samartzidis@gmail.com>. All rights reserved.
// You are not allowed to redistribute, modify or sell any part of this file in either 
// compiled or non-compiled form without the author's written permission.
//

#include "WebApiServer.h"
#include <thread>
#include <chrono>
#include "bluetooth/bt.h"
#include "Globals.h"
#include "Logger.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#include "json.hpp"
#pragma GCC diagnostic pop

using namespace std;

WebApiServer::WebApiServer()
{
	BuildRoutes();
}

WebApiServer::~WebApiServer()
{
}

void WebApiServer::MainThread()
{
	m_crowApp.port(80).run();

	m_pMainThread = nullptr;
}

void WebApiServer::Start()
{
	if (m_pMainThread)
		return;

	m_pMainThread = new thread(&WebApiServer::MainThread, this);
}

void WebApiServer::Stop()
{
	m_crowApp.stop();

	m_pMainThread->join();
	delete m_pMainThread;
	m_pMainThread = nullptr;
}

void WebApiServer::BuildRoutes()
{
	CROW_ROUTE(m_crowApp, "/") ([](const crow::request& req, crow::response& res)
	{
		res.code = 302;
		res.set_header("Location", "html/index.html");
		res.end();
	});

	CROW_ROUTE(m_crowApp, "/html/<path>")
		([](string path)
	{
		string fullPath = Globals::FormatString("%s/html/%s", Globals::g_szModuleDir, path.c_str());		
		DbgMsg("Local full file path: %s", fullPath.c_str());

		ifstream ifs(fullPath);
		if (ifs.good())
		{
			stringstream buffer;
			buffer << ifs.rdbuf();
			const string& content = buffer.str();

			crow::response resp(content);

			string ext = fullPath.substr(fullPath.find_last_of(".") + 1);
			if(ext == "css")
				resp.add_header("Content-Type", "text/css");
			if (ext == "js")
				resp.add_header("Content-Type", "application/javascript");
			if (ext == "png")
				resp.add_header("Content-Type", "image/png");
			else if (ext == "html")
				resp.add_header("Content-Type", "text/html");

			return resp;
		}
		else 
		{
			return crow::response(404);
		}
	});

	CROW_ROUTE(m_crowApp, "/save-data")
		([]()
	{
		int retval = system("/bin/mount -o remount,rw /boot && /bin/tar cpjf /boot/data.tar.bz2 -C /data . && /bin/mount -o remount,ro /boot");
		if(retval)
			return crow::response(500, Globals::FormatString("System command failed with exit code %d", retval));

		return crow::response(200, "{}");
	});

	CROW_ROUTE(m_crowApp, "/delete-data")
		([]()
	{

		int retval = system(
			"/bin/mount -o remount,rw /boot && "\
			"rm -f /boot/data.tar.bz2 && "\
			"/bin/mount -o remount,ro /boot && "\
			"service bluetooth stop &&"\
			"/bin/rm -fr /data/*"); 

		if (retval)
			return crow::response(500, Globals::FormatString("System command failed with exit code %d", retval));

		system("/sbin/reboot &");

		return crow::response(200, "{}");
	});

	CROW_ROUTE(m_crowApp, "/info")
		([]()
	{
		nlohmann::json jobj;
		jobj["uptime"] = Globals::FormatString("%ld", Globals::GetUptime());
		jobj["version"] = VERSION;

		return crow::response(200, jobj.dump());
	});

	CROW_ROUTE(m_crowApp, "/log")
		([]()
	{
		string strLog;
		ifstream logfile;
		logfile.open(LOG_FILE_PATH);
		if (logfile.is_open())
		{
			stringstream buffer;
			buffer << logfile.rdbuf();
			strLog = buffer.str();
			logfile.close();
		}

		return crow::response(200, strLog);
	});

	CROW_ROUTE(m_crowApp, "/bt")
		([](const crow::request& req)
	{
		char* val = req.url_params.get("v");
		try
		{
			if (val == nullptr)
			{
				nlohmann::json jobj;
				jobj["v"] = bluetooth::IsStarted() ? "on" : "off";
				return crow::response(200, jobj.dump());
			}
			else if (strcmp(val, "on") == 0)
				bluetooth::Start();
			else if (strcmp(val, "off") == 0)
				bluetooth::Stop();
		}
		catch (exception& ex)
		{
			return crow::response(500, ex.what());
		}

		return crow::response(200, "{}");
	});

	CROW_ROUTE(m_crowApp, "/bt/discovery")
		([](const crow::request& req)
	{
		char* val = req.url_params.get("v");
		try
		{			
			if (val == nullptr)
			{
				nlohmann::json jobj;
				jobj["v"] = bluetooth::GetDiscovery() ? "on" : "off";
				return crow::response(200, jobj.dump());
			}
			else
			{
				bluetooth::ReplyMessage m;
				if (strcasecmp(val, "on") == 0)
					bluetooth::SetDiscovery(true);
				else if (strcasecmp(val, "off") == 0)
					bluetooth::SetDiscovery(false);
			}
		}
		catch (exception& ex)
		{
			return crow::response(500, ex.what());
		}

		return crow::response(200, "{}");
	});

	CROW_ROUTE(m_crowApp, "/bt/devices")
		([]()
	{
		auto jarray = nlohmann::json::array();
		auto devs = bluetooth::GetDevices();
		for (auto& dev : devs)
		{
			nlohmann::json jobj;
			jobj["name"] = dev.Name;
			jobj["address"] = dev.Address;
			jobj["icon"] = dev.Icon;
			jobj["paired"] = dev.Paired;
			jobj["connected"] = dev.Connected;
			jobj["trusted"] = dev.Trusted;

			if (dev.Connected)
			{
				int bcap = Globals::GetBtHidBatteryCapacity(dev.Address);
				if (bcap >= 0)
					jobj["battery"] = bcap;
			}

			jarray.push_back(jobj);
		}

		return jarray.dump();
	});

	//CROW_ROUTE(m_app, "/bt/power")
	//	([](const crow::request& req)
	//{
	//	char* val = req.url_params.get("v");
	//	string msg = "Expected a query parameter ...?v=[bluetooth device address] e.g. ...?v=0C:D7:46:E4:FF:29";
	//	if (val == nullptr)
	//		return crow::response(400, msg);
	//	return crow::response(200, "{}");
	//}

	CROW_ROUTE(m_crowApp, "/bt/info")
		([](const crow::request& req)
	{
		try
		{
			char* val = req.url_params.get("v");
			string msg = "Expected a query parameter ...?v=[bluetooth device address] e.g. ...?v=0C:D7:46:E4:FF:29";
			if (val == nullptr)
				return crow::response(400, msg);

			bluetooth::BtDeviceInfo di;
			bluetooth::GetDeviceInfo(val, di);

			nlohmann::json jobj;
			jobj["name"] = di.Name;
			jobj["address"] = di.Address;
			jobj["alias"] = di.Alias;
			jobj["class"] = di.Class;
			jobj["appearance"] = di.Appearance;
			jobj["icon"] = di.Icon;
			jobj["paired"] = di.Paired;
			jobj["trusted"] = di.Trusted;
			jobj["blocked"] = di.Blocked;
			jobj["connected"] = di.Connected;
			jobj["legacyPairing"] = di.LegacyPairing;
			jobj["modalias"] = di.Modalias;

			return crow::response(200, jobj.dump());
		}
		catch (exception& ex)
		{
			return crow::response(500, ex.what());
		}
	});

	CROW_ROUTE(m_crowApp, "/bt/begin-pair")
		([](const crow::request& req)
	{
		try
		{
			char* val = req.url_params.get("v");
			string msg = "Expected a query parameter ...?v=[bluetooth device address] e.g. ...?v=0C:D7:46:E4:FF:29";
			if (val == nullptr)
				return crow::response(400, msg);

			bluetooth::ReplyMessage res = bluetooth::BeginPairDevice(val);
			if (res.Status == bluetooth::ReplyMessageStatus::Success)
				return crow::response(200, "{}");
			else if (res.Status == bluetooth::ReplyMessageStatus::PinCode)
				return crow::response(200, Globals::FormatString("{\"pinCode\":\"%s\"}", res.Data.c_str()));

			return crow::response(500, res.Data.c_str());
		}
		catch (exception& ex)
		{
			return crow::response(500, ex.what());
		}
	});

	CROW_ROUTE(m_crowApp, "/bt/end-pair")
		([](const crow::request& req)
	{
		try
		{
			bluetooth::ReplyMessage res = bluetooth::EndPairDevice();
			if(res.Status == bluetooth::ReplyMessageStatus::Success)
				return crow::response(200, "{}");

			return crow::response(500, res.Data.c_str());
		}
		catch (exception& ex)
		{
			return crow::response(500, ex.what());
		}
	});

	CROW_ROUTE(m_crowApp, "/bt/remove")
		([](const crow::request& req)
	{
		char* val = req.url_params.get("v");
		try
		{
			string msg = "Expected a query parameter ...?v=[bluetooth device address] e.g. ...?v=0C:D7:46:E4:FF:29";
			if (val == nullptr)
				return crow::response(400, msg);

			bluetooth::RemoveDevice(val);
		}
		catch (exception& ex)
		{
			return crow::response(500, ex.what());
		}

		return crow::response(200, "{}");
	});

	CROW_ROUTE(m_crowApp, "/bt/connect")
		([](const crow::request& req)
	{
		char* val = req.url_params.get("v");
		try
		{		
			string msg = "Expected a query parameter ...?v=[bluetooth device address] e.g. ...?v=0C:D7:46:E4:FF:29";
			if (val == nullptr)
				return crow::response(400, msg);

			bluetooth::ConnectDevice(val);
		}
		catch (exception& ex)
		{
			return crow::response(500, ex.what());
		}

		return crow::response(200, "{}");
	});

	CROW_ROUTE(m_crowApp, "/bt/disconnect")
		([](const crow::request& req)
	{
		char* val = req.url_params.get("v");
		try
		{			
			string msg = "Expected a query parameter ...?v=[bluetooth device address] e.g. ...?v=0C:D7:46:E4:FF:29";
			if (val == nullptr)
				return crow::response(400, msg);

			bluetooth::DisconnectDevice(val);
		}
		catch (exception& ex)
		{
			return crow::response(500, ex.what());
		}

		return crow::response(200, "{}");
	});

	CROW_ROUTE(m_crowApp, "/bt/trust")
		([](const crow::request& req)
	{
		char* val = req.url_params.get("v");
		try
		{			
			string msg = "Expected a query parameter ...?v=[bluetooth device address] e.g. ...?v=0C:D7:46:E4:FF:29";
			if (val == nullptr)
				return crow::response(400, msg);

			bluetooth::TrustDevice(val);
		}
		catch (exception& ex)
		{
			return crow::response(500, ex.what());
		}

		return crow::response(200, "{}");
	});
}





