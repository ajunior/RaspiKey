#include "KeyMapReportFilter.h"
#include "Globals.h"
#include "Logger.h"
#include <sstream>

using namespace std;

KeyMapReportFilter::KeyMapReportFilter()
{

}


KeyMapReportFilter::~KeyMapReportFilter()
{

}

void KeyMapReportFilter::LoadKeyMapFile(const std::string& strPath)
{	
	ifstream ifs;
	ifs.open(strPath);
	if (!ifs.is_open())
	{
		auto str = Globals::FormatString("Failed to read keymap file %s", strPath.c_str());
		ErrorMsg(str.c_str());

		throw runtime_error(str);
	}

	stringstream ss;
	ss << ifs.rdbuf();
	const string strJson = ss.str();
	ifs.close();

	LoadKeyMap(strJson);
}

void KeyMapReportFilter::LoadKeyMap(const std::string& strJson)
{
	m_keyMap.clear();
	try
	{
		auto jsondoc = nlohmann::json::parse(strJson);

		for (auto it = jsondoc.begin(); it != jsondoc.end(); ++it)
		{
			auto keyMapping = it.value().get<JsonTypes::KeyMapping>();
			m_keyMap.push_back(keyMapping);
		}
	}
	catch (const exception& m)
	{
		ErrorMsg("Failed to parse keymap data: %s", m.what());
		throw;
	}

	InfoMsg("Successfully loaded keymap");
}

size_t KeyMapReportFilter::ProcessInputReport(uint8_t* buf, size_t len)
{
	if (buf[0] != 1 || len < 9)
		return len;

	uint8_t& mod = buf[1];
	for (const auto& mapping : m_keyMap)
	{		
		if (mapping.imod == mod)
		{
			for (int k = 3; k < 9; k++)
			{
				uint8_t& key = buf[k];
				if (mapping.ikey == key)
				{
					mod = mapping.omod;
					key = mapping.okey;

					return len;
				}
			}
		}
	}

	return len;
}

size_t KeyMapReportFilter::ProcessOutputReport(uint8_t* buf, size_t len)
{

	return len;
}