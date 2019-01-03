//
// RaspiKey Copyright (c) 2019 George Samartzidis <samartzidis@gmail.com>. All rights reserved.
// You are not allowed to redistribute, modify or sell any part of this file in either 
// compiled or non-compiled form without the author's written permission.
//

#include "Logger.h"
#include <mutex>
#include <fstream>
#include <sys/stat.h>
#include "Globals.h"
#include <iomanip>
#include <iostream>

using namespace std;

CLogger& CLogger::Instance()
{
	static CLogger inst;
	return inst;
}

CLogger::CLogger()
{
	m_filePath = LOG_FILE_PATH;
	OpenLogFile();

	m_logCount = GetFileSize(m_filePath);
}

CLogger::~CLogger()
{
	CloseLogFile();
}

void CLogger::OpenLogFile()
{
	m_logStream.open(m_filePath, fstream::out | fstream::app);
}

void CLogger::CloseLogFile()
{
	if (m_logStream.is_open())
		m_logStream.close();
}

long CLogger::GetFileSize(const std::string& filename) const
{
	struct stat stat_buf;
	int rc = stat(filename.c_str(), &stat_buf);
	return rc == 0 ? stat_buf.st_size : -1;
}

void CLogger::FileLog(const string& msg)
{
	lock_guard<mutex> lck(m_logMutex);

	//cout << m_logCount <<  " > " << m_maxFileSizeBytes << endl;

	if (m_logCount > m_maxFileSizeBytes)
	{
		CloseLogFile();
		if (remove(m_filePath.c_str()) != 0)
			return;
		m_logCount = 0;
		OpenLogFile();
	}

	m_logStream << msg << flush;
	
	m_logCount += msg.length();
}

void CLogger::Log(const int priority, const char* const file, const int line, const char* const msg)
{
	string strPriority = "LOG";
	switch (priority)
	{
	case LOG_DEBUG:
		strPriority = "LOG_DEBUG"; break;
	case LOG_INFO:
		strPriority = "LOG_INFO"; break;
	case LOG_ERR:
		strPriority = "LOG_ERR"; break;
	default:
		break;
	}

	long totalSecs = Globals::GetUptime();
	long days = totalSecs / (24 * 3600);
	totalSecs %= (24 * 3600);
	long hours = totalSecs / 3600;
	totalSecs %= 3600;
	long minutes = totalSecs / 60;
	totalSecs %= 60;
	long seconds = totalSecs;

	string msgFmt = Globals::FormatString("%d.%02d:%02d:%02d|%s|(%s:%d)|%s\n", 
		days, hours, minutes, seconds, 
		strPriority.c_str(), file, line, msg);
	
#if !defined(NDEBUG) // DEBUG
	fprintf(stdout, msgFmt.c_str());
	FileLog(msgFmt);
#else // RELEASE 
	if (priority < LOG_DEBUG)
		FileLog(msgFmt);
#endif		
}

