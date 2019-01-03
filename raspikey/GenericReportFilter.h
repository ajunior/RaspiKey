//
// RaspiKey Copyright (c) 2019 George Samartzidis <samartzidis@gmail.com>. All rights reserved.
// You are not allowed to redistribute, modify or sell any part of this file in either 
// compiled or non-compiled form without the author's written permission.
//

#pragma once

#include "ReportFilter.h"

class GenericReportFilter :
	public IReportFilter
{
public:
	GenericReportFilter();
	virtual ~GenericReportFilter();

	size_t ProcessInputReport(uint8_t* buf, size_t len) override;
	size_t ProcessOutputReport(uint8_t* buf, size_t len) override;
};

