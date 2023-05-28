#pragma once

#include <queue>
#include <stack>
#include <string>
#include <thread>
#include <Windows.h>

#define LO_NONE          0x00
#define LO_TXT           0x01
#define LO_CSV           0x02
#define LO_DBS           0x04
#define LO_CMD           0x08

enum class logLevel {
	Fatal,
	Error,
	Warning,
	Info,
	Debug,
};

//
// 1. 일단레벨을 가장 먼저 확인해야함, 그래야 로그를 적을지 말지 조차 판단가능
//		- level
// 2. 그다음 모드를 확인해야함, 그래야 어떤식으로 문자열을 만들지 결정함
//		- writeMode
// 3. 그다음 문자열을 만들어야함, CMD 일경우 그냥 message만, text, (CSV, DBS) 일경우에도 그에 맞게
//		-문자열 만드는데 필요한 정보
//			- logCppName(CPP 파일명)
//			- line
//			- time
//			- threadID
//			- 메시지
// 
// 4. 마지막으로 file path 을 만들어야함, 파일에 쓰기 모드일경우에만 해당
//
struct logMessage {
	logLevel								level;				//	4
	unsigned char							writeMode;			//	1

	unsigned int							line;				//	4
	unsigned int							threadID;			//	4
	std::chrono::system_clock::time_point	time;				//	8
	std::string								logCppName;			//	32
	std::string								message;			//	32
	std::string								logWriteFileName;	//	32
	//										DB 연결을 위한 식별자
};


class logger {
public:
	logger();
	~logger();

	bool			writeLog(const logMessage& msg);

	bool			setLoggingDir(const std::string& logFileDir);
	std::string		getLoggingDir();

	//bool			setLoggingDBA(const std::string logDBA);
	//std::string	getLoggingDBA();

	bool			setLoggingLevel(const logLevel _levelThreshold);
	logLevel		getLoggingLevel();

	void loggingThreadProc();

public:
	bool runFlag;
	unsigned int days;

	logLevel levelThreshold;
	std::string logFileDir;

	std::thread* loggingThread;
	HANDLE hThread;
	HANDLE loggingEvent;

	std::queue<logMessage*> logMessageQueue;
	std::stack<logMessage*> logMessagePool;
	CRITICAL_SECTION queueCS;
	CRITICAL_SECTION poolCS;
};
