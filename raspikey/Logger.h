//
// RaspiKey Copyright (c) 2019 George Samartzidis <samartzidis@gmail.com>. All rights reserved.
// You are not allowed to redistribute, modify or sell any part of this file in either 
// compiled or non-compiled form without the author's written permission.
//

#pragma once
#include <mutex>
#include <fstream>

class Logger
{
public:
	static Logger& Instance();
	
	void Log(const int priority, const char* const file, const int line, const char* const msg);

protected:
	Logger();
	~Logger();

private:
	long GetFileSize(const std::string& filename) const;
	void OpenLogFile();
	void CloseLogFile();
	void FileLog(const std::string& msg);

	size_t m_logCount = 0;
	size_t m_maxFileSizeBytes = 5 * 1024 * 1024; // 5 MB
	std::ofstream m_logStream;
	std::mutex m_logMutex;
	std::string m_filePath;
};

// Logging macros
//
#if !defined(NDEBUG) // DEBUG
#define DbgMsg(...) \
	Logger::Instance().Log(LOG_DEBUG, __FILE__, __LINE__, Globals::FormatString(__VA_ARGS__).c_str() );
#else // RELEASE
#define DbgMsg(...)	
#endif

#define InfoMsg(...) \
	Logger::Instance().Log(LOG_INFO, __FILE__, __LINE__, Globals::FormatString(__VA_ARGS__).c_str() );

#define ErrorMsg(...) \
	Logger::Instance().Log(LOG_ERR, __FILE__, __LINE__, Globals::FormatString(__VA_ARGS__).c_str() );
//
//

