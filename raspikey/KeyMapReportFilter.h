#pragma once
#include "ReportFilter.h"
#include <string>
#include <vector>

typedef struct KeyMapping
{
	uint8_t imod;
	uint8_t ikey;
	uint8_t omod;
	uint8_t okey;
} tagKeyMapping;

const int NullKeyMapping = 0xff;

class KeyMapReportFilter : public IReportFilter
{
protected:
	std::vector<KeyMapping> m_keyMap;

public:
	KeyMapReportFilter();
	~KeyMapReportFilter();

	size_t ProcessInputReport(uint8_t* buf, size_t len) override;
	size_t ProcessOutputReport(uint8_t* buf, size_t len) override;

	void SetSettings(std::string settings) override;
	std::string GetSettings() override;
};

