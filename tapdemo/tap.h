#pragma once

#define ADAPTER_DESCRIPTION "TAP-Windows Adapter V9"

typedef void* TAPHandle;

TAPHandle iTapOpen(const char * pcIP, const char * pcMask);
int iTapRead(TAPHandle handle, char* pcBuf, int iLen);
int iTapWrite(TAPHandle handle, char* pcBuf, int iLen);
void vTapClose(TAPHandle handle);

void vShowAllRegConnectionInfo();
void vShowAllAdapterInfo();
int iGetAdapterGUID(const char* pcAdapterName, char* pcBuf, int iBufLen);
