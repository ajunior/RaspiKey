#include "KeyMap.h"
#include "Globals.h"
#include "Logger.h"
#include <sstream>

using namespace std;

KeyMap::KeyMap()
{

}


KeyMap::~KeyMap()
{

}

void KeyMap::LoadKeyMapFile(const char* path)
{	
	ifstream ifs;
	ifs.open(path);
	if (!ifs.is_open())
	{
		auto str = Globals::FormatString("Failed to read keymap file %s", path);
		ErrorMsg(str.c_str());

		throw runtime_error(str);
	}

	stringstream ss;
	ss << ifs.rdbuf();
	const string strJson = ss.str();
	ifs.close();

	LoadKeyMap(strJson);
}

void KeyMap::LoadKeyMap(const std::string& strJson)
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
	catch (exception& m)
	{
		ErrorMsg("Failed to parse keymap data: %s", m.what());
		throw m;
	}

	InfoMsg("Successfully loaded keymap");
}

size_t KeyMap::ProcessInputReport(uint8_t* buf, size_t len)
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
					break;
				}
			}
		}
	}

	return len;
}

size_t KeyMap::ProcessOutputReport(uint8_t* buf, size_t len)
{

	return len;
}