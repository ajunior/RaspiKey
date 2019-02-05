#include "KeyMapReportFilter.h"
#include "Globals.h"
#include "Logger.h"
#include <sstream>
#include "json.hpp"

using namespace std;

uint8_t KeyMappingStrToNum(const nlohmann::basic_json<>& ref)
{
	if (ref.is_null())
		return NullKeyMapping;

	string str = ref.get<string>();
	if (str.empty())
		return NullKeyMapping;

	return (uint8_t)std::stoi(str, nullptr, 16);
}

static void from_json(const nlohmann::json& j, KeyMapping& p)
{
	p.imod = KeyMappingStrToNum(j.at(0));
	p.ikey = KeyMappingStrToNum(j.at(1));
	p.omod = KeyMappingStrToNum(j.at(2));
	p.okey = KeyMappingStrToNum(j.at(3));
}

KeyMapReportFilter::KeyMapReportFilter()
{

}


KeyMapReportFilter::~KeyMapReportFilter()
{

}

size_t KeyMapReportFilter::ProcessInputReport(uint8_t* buf, size_t len)
{
	if (buf[0] != 1 || len < 9)
		return len;

	uint8_t& mod = buf[1];
	for (const auto& mapping : m_keyMap)
	{		
		if (mapping.imod == NullKeyMapping || (mod & mapping.imod) == mapping.imod) // Do we care about the mod OR do the imod flags provided match with the mod?
		{
			for (int k = 3; k < 9; k++) // Look at all the input keys pressed
			{
				uint8_t& key = buf[k];
				if (mapping.ikey == key) // Found one matching the ikey
				{
					if(mapping.omod != NullKeyMapping) // Is omod defined in this mapping?
						mod = mapping.omod; // Only modify omod if defined

					key = mapping.okey; // Modify key with okey valye

					return len; // Do not process further mappings
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

void KeyMapReportFilter::SetSettings(std::string settings)
{
	m_keyMap.clear();
	try
	{
		auto jsondoc = nlohmann::json::parse(settings);

		for (auto it = jsondoc.begin(); it != jsondoc.end(); ++it)
		{
			auto keyMapping = it.value().get<KeyMapping>();
			m_keyMap.push_back(keyMapping);
		}
	}
	catch (const exception& m)
	{
		ErrorMsg("Failed to parse keymap data: %s", m.what());
		throw;
	}

	m_strKeymap = settings;
	InfoMsg("Successfully loaded keymap");
}

std::string KeyMapReportFilter::GetSettings()
{
	return m_strKeymap;
}