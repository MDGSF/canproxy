#pragma once

#include <WinSock2.h>

#define PATH_SEP '\\'
#define MAX_LOG_BUFFER_SIZE 1400

typedef enum {
	ENOLOG,
	EOUT,	//for other people
	EFatal,
	EError,
	EWarn,
	EInfo,
	EDebug,
	ETrace
}EDebugLogLevel;


//log level default is EError
//extern EDebugLogLevel g_eDebugLogLevel;

void SetLogLevel(EDebugLogLevel level);

void debug_printf(
	int eLogLevel,
	const char* pcLogLevel,
	const char* pcComponent,
	const char* pcFormat,
	const char* pcFile,
	int iLine,
	...);



#define __DEBUGLOG__
#ifdef __DEBUGLOG__
#define LOG(eLogLevel, pcComponent, pcFormat, ...) \
	debug_printf(eLogLevel, #eLogLevel, #pcComponent, pcFormat, __FILE__, __LINE__, ##__VA_ARGS__);
#else
#define LOG(eLogLevel, pcFormat, ...)
#endif
