#include "A1644.h"
#include <iostream> 
#include "Globals.h"

using namespace std;

A1644::A1644()
{
}


A1644::~A1644()
{
}


size_t A1644::ProcessInputReport(uint8_t* buf, size_t len)
{
	if (len != sizeof(A1644HidReport))
		return 0; // Ignore report

	A1644HidReport& inRpt = *reinterpret_cast<A1644HidReport*>(buf);

	if (Globals::g_dwSwapFnCtrl)
	{
		//Process LCtrl modifier and translate to FakeFn key

		bool fakeFnActiveBefore = m_FakeFnActive;
		m_FakeFnActive = inRpt.Modifier & Globals::HidLCtrlMask;
		if (fakeFnActiveBefore != m_FakeFnActive) //FakeFn state changed?
			return 0; //Do not send scancode for this

		inRpt.Modifier &= (uint8_t)~Globals::HidLCtrlMask; //Clear LCtrl modifier
	}

	//Process special key input
	if (inRpt.Special)
	{
		if (inRpt.Special & 0x1) //Eject (translate to Del)
			inRpt.Key1 = Globals::HidDel; //Set Del key

		if (inRpt.Special & 0x2) //Fn (translate to LCtrl)
		{
			//If we swap Fn and Ctrl
			if (Globals::g_dwSwapFnCtrl)
				inRpt.Modifier |= (uint8_t)Globals::HidLCtrlMask; //Set LCtrl modifier
			else
				m_FakeFnActive = true;
		}
	}

	//Process optional Alt-Cmd swap
	if (Globals::g_dwSwapAltCmd)
	{
		if (inRpt.Modifier & Globals::HidLAltMask)
		{
			inRpt.Modifier &= (uint8_t)~Globals::HidLAltMask;
			inRpt.Modifier |= (uint8_t)Globals::HidLCmdMask;
		}
		else if (inRpt.Modifier & Globals::HidLCmdMask)
		{
			inRpt.Modifier &= (uint8_t)~Globals::HidLCmdMask;
			inRpt.Modifier |= (uint8_t)Globals::HidLAltMask;
		}

		if (inRpt.Modifier & (uint8_t)Globals::HidRAltMask)
		{
			inRpt.Modifier &= (uint8_t)~Globals::HidRAltMask;
			inRpt.Modifier |= (uint8_t)Globals::HidRCmdMask;
		}
		else if (inRpt.Modifier & Globals::HidRCmdMask)
		{
			inRpt.Modifier &= (uint8_t)~Globals::HidRCmdMask;
			inRpt.Modifier |= (uint8_t)Globals::HidRAltMask;
		}
	}

	//Swap key below ESC (0x64) and key to the right of LShift (0x35)
	uint8_t swaps[] = { 
		0x64, 0x35, 
		0x35, 0x64 
	};
	for(int k = 0; k < 6; k++)
	{
		uint8_t* srcKeys = &inRpt.Key1;
		uint8_t& srcKey = srcKeys[k];
		for (size_t j = 0; j <= sizeof(swaps) / 2; j += 2)
		{
			if (srcKey == swaps[j])
			{
				srcKey = swaps[j + 1];
				break;
			}
		}
	}

	//Is this a break code for a previously pressed multimedia key?
	if (m_MultimediaKeyActive && !inRpt.Key1)
	{
		m_MultimediaKeyActive = false;

		inRpt.ReportId = 0x2;
		inRpt.Modifier = 0x0;

		return 2;
	}

	//Process Fn+[key] combination 
	if (m_FakeFnActive && (inRpt.Key1 || inRpt.Modifier))
	{
		switch (inRpt.Key1)
		{
		case Globals::HidLeft: inRpt.Key1 = Globals::HidHome; 
			break;
		case Globals::HidRight: inRpt.Key1 = Globals::HidEnd; 
			break;
		case Globals::HidUp: inRpt.Key1 = Globals::HidPgUp; 
			break;
		case Globals::HidDown: inRpt.Key1 = Globals::HidPgDown; 
			break;
		case Globals::HidEnter: inRpt.Key1 = Globals::HidInsert; 
			break;
		case Globals::HidF1: inRpt.Key1 = Globals::HidF13; 
			break;
		case Globals::HidF2: inRpt.Key1 = Globals::HidF14; 
			break;
		case Globals::HidF3: inRpt.Key1 = Globals::HidF15; 
			break;
		case Globals::HidF4: inRpt.Key1 = Globals::HidF16; 
			break;
		case Globals::HidF5: inRpt.Key1 = Globals::HidF17; 
			break;
		case Globals::HidF6: inRpt.Key1 = Globals::HidF18; 
			break;
		case Globals::HidF7:
			m_MultimediaKeyActive = true;
			inRpt.ReportId = 0x2; inRpt.Modifier = 0x02; //Prev Track
			return 2;
			break;
		case Globals::HidF8:
			m_MultimediaKeyActive = true;
			inRpt.ReportId = 0x2; inRpt.Modifier = 0x08; //Play/Pause
			return 2;
			break;
		case Globals::HidF9:
			m_MultimediaKeyActive = true;
			inRpt.ReportId = 0x2; inRpt.Modifier = 0x01; //Next Track
			return 2;
			break;
		case Globals::HidF10:
			m_MultimediaKeyActive = true;
			inRpt.ReportId = 0x2; inRpt.Modifier = 0x10; //Mute
			return 2;
			break;
		case Globals::HidF11:
			m_MultimediaKeyActive = true;
			inRpt.ReportId = 0x2; inRpt.Modifier = 0x40; //Vol Down
			return 2;
			break;
		case Globals::HidF12:
			m_MultimediaKeyActive = true;
			inRpt.ReportId = 0x2; inRpt.Modifier = 0x20; //Vol Up
			return 2;
			break;
		case Globals::HidKeyP: inRpt.Key1 = Globals::HidPrtScr; 
			break;
		case Globals::HidKeyB: inRpt.Key1 = Globals::HidPauseBreak; 
			break;
		case Globals::HidKeyS: inRpt.Key1 = Globals::HidScrLck; 
			break;
		default:
			if (inRpt.Modifier & Globals::HidLCtrlMask) //Map Fn+LCtrl to RCtrl
			{
				inRpt.Modifier &= (uint8_t)~Globals::HidLCtrlMask; //Clear LCtrl
				inRpt.Modifier |= (uint8_t)Globals::HidRCtrlMask; //Set RCtrl
			}
			else
				return 0; //Ignore all other Fn+[key] combinations
			break;
		}
	}

	return 9;
}

size_t A1644::ProcessOutputReport(uint8_t* buf, size_t len)
{	
	if (len != sizeof(Globals::HidgOutputReport))
		return 0;

	return len;
}


