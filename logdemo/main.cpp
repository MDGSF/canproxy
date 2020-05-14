#include <stdio.h>
#include "log.h"

int main()
{
	SetLogLevel(ETrace);
	LOG(EFatal, LOGDEMO, "EFatal log\n");
	LOG(EError, LOGDEMO, "EError log\n");
	LOG(EWarn, LOGDEMO, "EWarn log\n");
	LOG(EInfo, LOGDEMO, "EInfo log\n");
	LOG(EDebug, LOGDEMO, "EDebug log\n");
	LOG(ETrace, LOGDEMO, "ETrace log\n");
	return 0;
}