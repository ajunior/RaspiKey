#include "A1314.h"
#include <iostream> 
#include "Globals.h"
#include "A1644.h"
#include "Logger.h"

using namespace std;

A1314::A1314()
{
}


A1314::~A1314()
{
}

size_t A1314::ProcessInputReport(uint8_t* buf, size_t len)
{	
	if (len == sizeof(A1314HidReport2))
	{
		auto& inRpt = *reinterpret_cast<A1314HidReport2*>(buf);
		
		m_a1644rep.Special = 0;
		if (inRpt.Special & 0x08) //Eject physical key
			m_a1644rep.Special |= 0x1;
		if (inRpt.Special & 0x10) //Fn physical key
			m_a1644rep.Special |= 0x2;
	}
	else if (len == sizeof(A1314HidReport))
	{
		memcpy(&m_a1644rep, buf, sizeof(A1314HidReport));		
	}

	DbgMsg("[in] +> %s", Globals::FormatBuffer(reinterpret_cast<uint8_t*>(&m_a1644rep), sizeof(A1644HidReport)).c_str());
	
	memcpy(buf, &m_a1644rep, sizeof(A1644HidReport));

	return m_a1644.ProcessInputReport(buf, sizeof(A1644HidReport));
}

size_t A1314::ProcessOutputReport(uint8_t* buf, size_t len)
{	
	if (len != sizeof(Globals::HidgOutputReport))
		return 0;

	return len;
}
