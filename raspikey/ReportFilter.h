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