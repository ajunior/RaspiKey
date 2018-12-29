#pragma once
#include <mutex>
#include <fstream>

class CLogger
{
public:
	static CLogger& Instance();
	
	void Log(const int priority, const char* const file, const int line, const char* const msg);

protected:
	CLogger();
	~CLogger();

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
	CLogger::Instance().Log(LOG_DEBUG, __FILE__, __LINE__, Globals::FormatString(__VA_ARGS__).c_str() );
#else // RELEASE
#define DbgMsg(...)	
#endif

#define InfoMsg(...) \
	CLogger::Instance().Log(LOG_INFO, __FILE__, __LINE__, Globals::FormatString(__VA_ARGS__).c_str() );

#define ErrorMsg(...) \
	CLogger::Instance().Log(LOG_ERR, __FILE__, __LINE__, Globals::FormatString(__VA_ARGS__).c_str() );
//
//

