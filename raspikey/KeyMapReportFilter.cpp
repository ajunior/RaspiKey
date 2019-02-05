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

static void JsonArrayPushKeyMappingValue(nlohmann::json& j, uint8_t val)
{
	if (val == NullKeyMapping)
		j.push_back(nlohmann::detail::value_t::null);
	else
		j.push_back(Globals::FormatString("0x%02x", val));
}

static void from_json(const nlohmann::json& j, KeyMapping& p)
{
	p.imod = KeyMappingStrToNum(j.at(0));
	p.ikey = KeyMappingStrToNum(j.at(1));
	p.omod = KeyMappingStrToNum(j.at(2));
	p.okey = KeyMappingStrToNum(j.at(3));
}

static void to_json(nlohmann::json& j, const KeyMapping& p)
{
	JsonArrayPushKeyMappingValue(j, p.imod);
	JsonArrayPushKeyMappingValue(j, p.ikey);
	JsonArrayPushKeyMappingValue(j, p.omod);
	JsonArrayPushKeyMappingValue(j, p.okey);
}

KeyMapReportFilter::KeyMapReportFilter()
{

}

KeyMapReportFilter::~KeyMapReportFilter()
{

}

size_t KeyMapReportFilter::ProcessInputReport(uint8_t* buf, size_t len)
{
	if (buf[0] != 1 || len < 9 || m_keyMap.empty())
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

	if (!settings.empty())
	{
		auto jobj = nlohmann::json::parse(settings);
		if (!jobj.is_array())
			throw runtime_error("A json array was expected");

		for (const auto& it : jobj)
		{
			auto keyMapping = it.get<KeyMapping>();
			m_keyMap.push_back(keyMapping);
		}
	}
}

std::string KeyMapReportFilter::GetSettings()
{
	auto jobj = nlohmann::json::array();
	for (const auto& it : m_keyMap)
	{
		jobj.push_back(it);
	}

	return jobj.dump();
}