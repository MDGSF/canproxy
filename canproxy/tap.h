#pragma once

#define ADAPTER_DESCRIPTION "TAP-Windows Adapter V9"

typedef void* TAPHandle;

TAPHandle iTapOpen(const char * pcIP, const char * pcMask, const char * pcAdapterDescription);
int iTapRead(TAPHandle handle, char* pcBuf, int iLen);
int iTapWrite(TAPHandle handle, const char* pcBuf, int iLen);
void vTapClose(TAPHandle handle);


/*
Debug api.
*/
void vShowAllRegConnectionInfo();
void vShowAllAdapterInfo();
int iGetAdapterGUID(const char* pcAdapterName, char* pcBuf, int iBufLen);
