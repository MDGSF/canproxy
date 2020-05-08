#pragma once

#define ADAPTER_NAME "TAP-Windows Adapter V9"

#define INVALID_VLNTAP_HANDLE -1
typedef int TAPHandle;

TAPHandle iTapOpen(const char * pcIP, const char * pcMask);

void vShowAllAdapterInfo();
int iGetAdapterGUID(const char* pcAdapterName, char* pcBuf, int iBufLen);
