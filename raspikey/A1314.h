#pragma once

#include <stdint.h>
#include "ReportFilter.h"
#include "A1644.h"

typedef struct A1314HidReport
{
	uint8_t ReportId;
	uint8_t Modifier;
	uint8_t Reserved;
	uint8_t Key1;
	uint8_t Key2;
	uint8_t Key3;
	uint8_t Key4;
	uint8_t Key5;
	uint8_t Key6;
} tagA1314HidReport;

typedef struct A1314HidReport2
{
	uint8_t ReportId;
	uint8_t Special;
} tagA1314HidReport2;

class A1314 : public IReportFilter
{
public:
	A1314();
	virtual ~A1314();

private:
	A1644HidReport m_a1644rep = { 0 };
	A1644 m_a1644;

public:
	size_t ProcessInputReport(uint8_t* buf, size_t len) override;
	size_t ProcessOutputReport(uint8_t* buf, size_t len) override;
};

