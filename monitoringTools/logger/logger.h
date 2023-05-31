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
	Off,
	Fatal,
	Error,
	Warning,
	Info,
	Debug,
};

//
// 1. �ϴܷ����� ���� ���� Ȯ���ؾ���, �׷��� �α׸� ������ ���� ���� �Ǵܰ���
//		- level
// 2. �״��� ��带 Ȯ���ؾ���, �׷��� ������� ���ڿ��� ������ ������
//		- writeMode
// 3. �״��� ���ڿ��� ��������, CMD �ϰ�� �׳� message��, text, (CSV, DBS) �ϰ�쿡�� �׿� �°�
//		-���ڿ� ����µ� �ʿ��� ����
//			- logCppName(CPP ���ϸ�)
//			- line
//			- time
//			- threadID
//			- �޽���
// 
// 4. ���������� file path �� ��������, ���Ͽ� ���� ����ϰ�쿡�� �ش�
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
	//										DB ������ ���� �ĺ���
};


class logger {
public:
	logger(logLevel _loggingLevel = logLevel::Debug);
	~logger();

	bool			writeLog(const logMessage& msg);

	bool			setLoggingDir(const std::string& logFileDir);
	std::string		getLoggingDir();

	//bool			setLoggingDBA(const std::string logDBA);
	//std::string	getLoggingDBA();

	bool			setLoggingLevel(const logLevel _loggingLevel);
	logLevel		getLoggingLevel();

	void loggingThreadProc();

public:
	bool runFlag;
	unsigned int days;

	logLevel loggingLevel;
	std::string logFileDir;

	std::thread* loggingThread;
	HANDLE hThread;
	HANDLE loggingEvent;

	std::queue<logMessage*> logMessageQueue;
	std::stack<logMessage*> logMessagePool;
	CRITICAL_SECTION queueCS;
	CRITICAL_SECTION poolCS;
};
