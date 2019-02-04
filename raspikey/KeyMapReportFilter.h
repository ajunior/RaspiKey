#pragma once
#include "JsonTypes.h"
#include "ReportFilter.h"

class KeyMapReportFilter : public IReportFilter
{
protected:
	std::vector<JsonTypes::KeyMapping> m_keyMap;

public:
	KeyMapReportFilter();
	~KeyMapReportFilter();

	void LoadKeyMap(const std::string& strJson);
	void LoadKeyMapFile(const std::string& path);

	size_t ProcessInputReport(uint8_t* buf, size_t len) override;
	size_t ProcessOutputReport(uint8_t* buf, size_t len) override;
};

