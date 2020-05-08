#include <WinSock2.h>
#include <iphlpapi.h>
#include <winioctl.h>
#include <shellapi.h>
#include <Iphlpapi.h>
#include <Mprapi.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tap-windows.h"
#include "tap.h"

#pragma comment(lib,"WS2_32.LIB") 
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment( lib, "Mprapi.lib" )

TAPHandle iTapOpen(const char* pcIP, const char* pcMask)
{
	char AdapterGUID[MAX_PATH] = { 0 };
	if (0 != iGetAdapterGUID(ADAPTER_NAME, AdapterGUID, sizeof(AdapterGUID)))
	{
		return INVALID_VLNTAP_HANDLE;
	}

	return 0;
}

void vShowAllAdapterInfo()
{
	ULONG ulOutBufLen = 0;
	if (GetAdaptersInfo(NULL, &ulOutBufLen) != ERROR_BUFFER_OVERFLOW)
	{
		return;
	}

	PIP_ADAPTER_INFO pAdapterInfo = (PIP_ADAPTER_INFO)malloc(ulOutBufLen);
	if (pAdapterInfo == NULL)
	{
		return;
	}

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == NO_ERROR)
	{
		PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
		while (pAdapter)
		{
			printf("ComboIndex = %d\n", pAdapter->ComboIndex);
			printf("AdapterName = %s\n", pAdapter->AdapterName);
			printf("Description = %s\n", pAdapter->Description);
			printf("\n");

			pAdapter = pAdapter->Next;
		}
	}

	free(pAdapterInfo);
}

/*
GetAdapterGUID 通过适配器名获取 GUID
@param pcAdapterName[in]: 适配器名称
@param pcBuf[out]: 返回的 GUID
@param iBufLen[in]: pcBuf 的大小
@return  0: 成功
		-1: 失败
*/
int iGetAdapterGUID(const char* pcAdapterName, char* pcBuf, int iBufLen)
{
	int iRet = -1;
	ULONG ulOutBufLen = 0;
	PIP_ADAPTER_INFO pAdapterInfo = NULL;
	PIP_ADAPTER_INFO pAdapter = NULL;

	//获取网络适配器信息
	if (GetAdaptersInfo(NULL, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
	{
		pAdapterInfo = (PIP_ADAPTER_INFO)malloc(ulOutBufLen);
		if (pAdapterInfo)
		{
			if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == NO_ERROR)
			{
				pAdapter = pAdapterInfo;
				while (pAdapter)
				{
					if (0 == strncmp(pcAdapterName, pAdapter->Description, strlen(pcAdapterName)))
					{
						strncpy_s(pcBuf, iBufLen, pAdapter->AdapterName, iBufLen - 1);
						iRet = 0;
						break;
					}
					pAdapter = pAdapter->Next;
				}
			}
			free(pAdapterInfo);
		}
	}

	return iRet;
}