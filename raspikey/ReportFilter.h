//
// RaspiKey Copyright (c) 2019 George Samartzidis <samartzidis@gmail.com>. All rights reserved.
// You are not allowed to redistribute, modify or sell any part of this file in either 
// compiled or non-compiled form without the author's written permission.
//

#pragma once

#include <stdint.h>
#include <stddef.h>

class IReportFilter
{
public:
	virtual ~IReportFilter() {}

	virtual size_t ProcessInputReport(uint8_t* buf, size_t len) = 0;
	virtual size_t ProcessOutputReport(uint8_t* buf, size_t len) = 0;
};