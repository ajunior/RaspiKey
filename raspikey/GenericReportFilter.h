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

