#include "GenericReportFilter.h"



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