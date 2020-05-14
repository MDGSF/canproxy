#ifdef _WIN32
#include <WinSock2.h>
#endif
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "log.h"

//extern EDebugLogLevel g_eDebugLogLevel = EError;
EDebugLogLevel g_eDebugLogLevel = EError;

void SetLogLevel(EDebugLogLevel level)
{
	g_eDebugLogLevel = level;
}

void debug_printf(
	int eLogLevel,
	const char* pcLogLevel,
	const char* pcComponent,
	const char* pcFormat,
	const char* pcFile,
	int iLine,
	...)
{
	if (eLogLevel > g_eDebugLogLevel) {
		return;
	}

	time_t now = time(0);
	tm stTime;

#ifdef _WIN32
	localtime_s(&stTime, &now);
#else
	localtime_r(&now, &stTime);
#endif

	va_list pcArg;
	va_start(pcArg, iLine);

	const char* pcTrimedFile = strrchr(pcFile, PATH_SEP);
	char acBuffer[MAX_LOG_BUFFER_SIZE] = { 0 };
#ifdef _WIN32
	int iOffset = sprintf_s(acBuffer, "[%d/%02d/%02d %02d:%02d:%02d]<%s>(%s)%s(%d):",
		stTime.tm_year + 1900,
		stTime.tm_mon + 1,
		stTime.tm_mday,
		stTime.tm_hour,
		stTime.tm_min,
		stTime.tm_sec,
		pcComponent, pcLogLevel, pcTrimedFile ? pcTrimedFile + 1 : pcFile, iLine);
#else
	int iOffset = sprintf(acBuffer, "[%d/%02d/%02d %02d:%02d:%02d]<%s>(%s)%s(%d):",
		stTime.tm_year + 1900,
		stTime.tm_mon + 1,
		stTime.tm_mday,
		stTime.tm_hour,
		stTime.tm_min,
		stTime.tm_sec,
		pcComponent, pcLogLevel, pcTrimedFile ? pcTrimedFile + 1 : pcFile, iLine);
#endif
	iOffset += vsnprintf(acBuffer + iOffset, MAX_LOG_BUFFER_SIZE - iOffset - 1, pcFormat, pcArg);
	if (iOffset >= MAX_LOG_BUFFER_SIZE) {
		iOffset = MAX_LOG_BUFFER_SIZE - 1;
	}
	acBuffer[iOffset] = 0;

	fputs(acBuffer, stdout);

	va_end(pcArg);
}
