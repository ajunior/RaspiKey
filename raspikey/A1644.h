#pragma once

#include <stdint.h>
#include <stddef.h>
#include "ReportFilter.h"

typedef struct A1644HidReport
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
	uint8_t Special;
} tagA1644HidReport;

class A1644 : public IReportFilter
{
public:
	A1644();
	virtual ~A1644();

private:
	bool m_FakeFnActive = false;
	bool m_MultimediaKeyActive = false;

public:	
	size_t ProcessInputReport(uint8_t* buf, size_t len) override;
	size_t ProcessOutputReport(uint8_t* buf, size_t len) override;
};

