//
// RaspiKey Copyright (c) 2019 George Samartzidis <samartzidis@gmail.com>. All rights reserved.
// You are not allowed to redistribute, modify or sell any part of this file in either 
// compiled or non-compiled form without the author's written permission.
//

#pragma once

#include <thread>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wshift-count-overflow"
#include "crow_all.h"
#pragma GCC diagnostic pop   

class WebApiServer
{
public:
	WebApiServer();
	~WebApiServer();

private:
	std::thread* m_pMainThread;
	crow::SimpleApp m_app;

private:
	void MainThread();
	void BuildRoutes();

public:
	void Start();
	void Stop();
};

