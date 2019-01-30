#pragma once

#include "json.hpp"

namespace JsonTypes 
{
	typedef struct KeyMapping 
	{
		uint8_t imod;
		uint8_t omod;
		uint8_t ikey;
		uint8_t okey;
	} tagKeyMapping;

	inline void to_json(nlohmann::json& j, const KeyMapping& p) {
		j = nlohmann::json{ p.imod, p.ikey, p.omod, p.okey };
	}

	inline void from_json(const nlohmann::json& j, KeyMapping& p) {
		
		p.imod = j.at(0).get<uint8_t>();
		p.ikey = j.at(1).get<uint8_t>();
		p.omod = j.at(2).get<uint8_t>();
		p.okey = j.at(3).get<uint8_t>();
	}
}
