//
// RaspiKey Copyright (c) 2019 George Samartzidis <samartzidis@gmail.com>. All rights reserved.
// You are not allowed to redistribute, modify or sell any part of this file in either 
// compiled or non-compiled form without the author's written permission.
//

#include "GenericReportFilter.h"
#include "Logger.h"
#include <fstream>
#include <sstream>
#include "JsonTypes.h"

using namespace std;

GenericReportFilter::GenericReportFilter()
{
	
}


GenericReportFilter::~GenericReportFilter()
{
}

size_t GenericReportFilter::ProcessInputReport(uint8_t* buf, size_t len)
{
	return len;
}

size_t GenericReportFilter::ProcessOutputReport(uint8_t* buf, size_t len)
{
	return len;
}
