#include <WinSock2.h>
#include <iphlpapi.h>
#include <winioctl.h>
#include <shellapi.h>
#include <Iphlpapi.h>
#include <Mprapi.h>
#include <windows.h>
#include <Ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tap-windows.h"
#include "tap.h"
#include "log.h"

#pragma comment(lib,"WS2_32.LIB") 
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment( lib, "Mprapi.lib" )

static OVERLAPPED overlap_read, overlap_write;

/*
iTapOpen ���������� IP ��ַ���������С�
@param pcIP[in]: Ҫ���������õ� IP ��ַ��
@param pcMask[in]: Ҫ���������õ��������롣
@return �ɹ������������������ļ���д�����
		ʧ�ܣ����� NULL��
*/
TAPHandle iTapOpen(const char* pcIP, const char* pcMask)
{
	char acAdapterID[MAX_PATH] = { 0 };
	if (0 != iGetAdapterGUID(ADAPTER_DESCRIPTION, acAdapterID, sizeof(acAdapterID)))
	{
		LOG(EError, TAPDEMO, "get adapter GUID failed\n");
		return NULL;
	}

	LOG(EInfo, TAPDEMO, "AdapterDescription = %s\n", ADAPTER_DESCRIPTION);
	LOG(EInfo, TAPDEMO, "acAdapterID = %s\n", acAdapterID);

	char acDeviceName[MAX_PATH] = { 0 };
	sprintf_s(acDeviceName, sizeof(acDeviceName), USERMODEDEVICEDIR "%s" TAP_WIN_SUFFIX, acAdapterID);

	LOG(EInfo, TAPDEMO, "acDeviceName = %s\n", acDeviceName);

	//���豸
	HANDLE rtHandle = CreateFileA(acDeviceName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_OVERLAPPED,
		0);
	if (rtHandle == INVALID_HANDLE_VALUE)
	{
		LOG(EError, TAPDEMO, "CreateFileA failed\n");
		return NULL;
	}

	DWORD len = 0;

	//ep����tun������IP��ַ������
	ULONG ep[3] = {};
	inet_pton(AF_INET, pcIP, &ep[0]);
	inet_pton(AF_INET, pcMask, &ep[2]);
	ep[1] = ep[0] & ep[2];

	//����Ϊ Tun ģʽ
	if (!DeviceIoControl(rtHandle, TAP_WIN_IOCTL_CONFIG_TUN, ep, sizeof(ep), ep, sizeof ep, &len, NULL))
	{
		LOG(EError, TAPDEMO, "set tun failed\n");
		CloseHandle(rtHandle);
		return NULL;
	}

	LONG ret;
	HKEY key;
	char acRegpath[MAX_PATH] = { 0 };
	char acAdapterName[MAX_PATH] = { 0 };
	DWORD lLen = MAX_PATH;

	_snprintf_s(acRegpath, MAX_PATH, "%s\\%s\\Connection", NETWORK_CONNECTIONS_KEY, acAdapterID);
	LOG(EInfo, TAPDEMO, "acRegpath = %s\n", acRegpath);

	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, acRegpath, 0, KEY_READ, &key))
	{
		LOG(EError, TAPDEMO, "RegOpenKeyExA()\n");
		return NULL;
	}

	ret = RegQueryValueExA(key, "Name", 0, 0, (LPBYTE)acAdapterName, &lLen);
	lLen = MAX_PATH;
	RegCloseKey(key);

	if (ret)
	{
		LOG(EError, TAPDEMO, "iTapOpen(): RegQueryValueExA()\n");
		return NULL;
	}

	LOG(EInfo, TAPDEMO, "acAdapterName = %s\n", acAdapterName);

	char acCmd[MAX_PATH] = { 0 };
	if (pcIP == NULL || strlen(pcIP) == 0)
	{
		_snprintf_s(acCmd, sizeof(acCmd), "netsh interface ip set address \"%s\" dhcp", acAdapterName);
	}
	else
	{
		_snprintf_s(acCmd, sizeof(acCmd), "netsh interface ip set address \"%s\" static %s %s",
			acAdapterName, pcIP, pcMask);
	}

	system(acCmd);

	/*
	* Initialize overlapped structures
	*/
	overlap_read.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	overlap_write.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!overlap_read.hEvent || !overlap_write.hEvent)
	{
		CloseHandle(rtHandle);
		return NULL;
	}

	//������������״̬
	ULONG MediaStatus = TRUE;
	if (!DeviceIoControl(rtHandle, TAP_WIN_IOCTL_SET_MEDIA_STATUS, &MediaStatus, sizeof(MediaStatus), &MediaStatus, sizeof(MediaStatus), &len, NULL))
	{
		//������������
		LOG(EError, TAPDEMO, "start network interface failed\n");
		CloseHandle(rtHandle);
		return NULL;
	}

	return rtHandle;
}

/*
iTapRead ������������ȡ���������ݣ���Щ���ݶ���ϵͳ�ϲ�Э��ջ���������ġ�
@param handle[in]: �����������ļ���д�����
@param pcBuf[in]: ���������ȡ�������ݵ��ڴ档
@param iLen[in]: pcBuf �ĳ��ȡ�
@return �ɹ������سɹ���ȡ�������ݳ��ȡ�
		ʧ�ܣ�-1��
*/
int iTapRead(TAPHandle handle, char* pcBuf, int iLen)
{
	DWORD iSize, last_err;

	ResetEvent(overlap_read.hEvent);
	if (ReadFile(handle, pcBuf, iLen, &iSize, &overlap_read))
	{
		return iSize;
	}
	switch (last_err = GetLastError())
	{
	case ERROR_IO_PENDING:
		WaitForSingleObject(overlap_read.hEvent, INFINITE);
		GetOverlappedResult(handle, &overlap_read, &iSize, FALSE);
		return iSize;
		break;
	default:
		LOG(EError, TAPDEMO, "GetLastError() returned %d\n", last_err);
		break;
	}

	return -1;
}

/*
iTapWrite �����������������д���ݣ������յ����ݺ�����Ϸ��͸�ϵͳ��Э��ջ��
@param handle[in]: �����������ļ���д�����
@param pcBuf[in]: Ҫ���͸����������ݡ�
@param iLen[in]: pcBuf �ĳ��ȡ�
@return �ɹ������سɹ����͵����ݳ��ȡ�
		ʧ�ܣ�-1��
*/
int iTapWrite(TAPHandle handle, const char* pcBuf, int iLen)
{
	DWORD iSize;

	ResetEvent(overlap_write.hEvent);
	if (WriteFile(handle, pcBuf, iLen, &iSize, &overlap_write))
	{
		return iSize;
	}
	switch (GetLastError())
	{
	case ERROR_IO_PENDING:
		WaitForSingleObject(overlap_write.hEvent, INFINITE);
		GetOverlappedResult(handle, &overlap_write, &iSize, FALSE);
		return iSize;
		break;
	default:
		break;
	}
	return -1;
}

/*
vTapClose �ر�����������
*/
void vTapClose(TAPHandle handle)
{
	CloseHandle(handle);
}

/*
vShowAllRegConnectionInfo ��ӡ��ע��������е�����������Ϣ
*/
void vShowAllRegConnectionInfo() {
	HKEY key;
	HKEY key2;
	LONG ret;
	int iIndex = 0;
	char acRegpath[MAX_PATH] = { 0 };
	char acAdapterID[MAX_PATH] = { 0 };
	char acAdapterName[MAX_PATH] = { 0 };
	DWORD lLen = MAX_PATH;

	/* Open registry and look for network adapters */
	if ((ret = RegOpenKeyExA(HKEY_LOCAL_MACHINE, NETWORK_CONNECTIONS_KEY, 0, KEY_READ, &key)) != ERROR_SUCCESS)
	{
		LOG(EError, TAPDEMO, "Unable to read registry, ret = %d\n", ret);
		return;
	}

	while (RegEnumKeyExA(key, iIndex++, acAdapterID, (LPDWORD)&lLen, 0, 0, 0, NULL) == ERROR_SUCCESS)
	{
		lLen = MAX_PATH;

		_snprintf_s(acRegpath, MAX_PATH, "%s\\%s\\Connection", NETWORK_CONNECTIONS_KEY, acAdapterID);
		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, acRegpath, 0, KEY_READ, &key2))
		{
			LOG(EDebug, TAPDEMO, "RegOpenKeyExA()\n");
			continue;
		}

		ret = RegQueryValueExA(key2, "Name", 0, 0, (LPBYTE)acAdapterName, &lLen);
		lLen = MAX_PATH;
		RegCloseKey(key2);

		if (ret)
		{
			LOG(EDebug, TAPDEMO, "iTapOpen(): RegQueryValueExA()\n");
			continue;
		}

		LOG(EInfo, TAPDEMO, "acAdapterID = %s\n", acAdapterID);
		LOG(EInfo, TAPDEMO, "acAdapterName = %s\n\n", acAdapterName);
	}
}

/*
vShowAllAdapterInfo ��ӡ��������������Ϣ

���ӣ�
ComboIndex = 15
AdapterName = {C2E362C6-EF62-4968-AAEB-3C33B4CA6554}
Description = Intel(R) Ethernet Connection (5) I219-V

ComboIndex = 14
AdapterName = {BCE2E997-3B8F-4811-AB6F-63E73154CF2A}
Description = Bluetooth Device (Personal Area Network)
...
*/
void vShowAllAdapterInfo()
{
	ULONG ulOutBufLen = 0;
	ULONG ulRet = GetAdaptersInfo(NULL, &ulOutBufLen);
	if (ulRet != ERROR_BUFFER_OVERFLOW)
	{
		LOG(EError, TAPDEMO, "GetAdaptersInfo failed, ulRet = %ul\n", ulRet);
		return;
	}

	PIP_ADAPTER_INFO pAdapterInfo = (PIP_ADAPTER_INFO)malloc(ulOutBufLen);
	if (pAdapterInfo == NULL)
	{
		LOG(EError, TAPDEMO, "malloc failed\n");
		return;
	}

	ulRet = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
	if (ulRet != NO_ERROR)
	{
		LOG(EError, TAPDEMO, "GetAdaptersInfo failed, ulRet = %ul\n", ulRet);
		free(pAdapterInfo);
		return;
	}

	PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
	while (pAdapter)
	{
		printf("ComboIndex = %d\n", pAdapter->ComboIndex);
		printf("AdapterName = %s\n", pAdapter->AdapterName);
		printf("Description = %s\n", pAdapter->Description);
		printf("\n");

		pAdapter = pAdapter->Next;
	}

	free(pAdapterInfo);
}

/*
iGetAdapterGUID ͨ������������ȡ GUID
@param pcAdapterDscription[in]: ����������
@param pcBuf[out]: ���ص� GUID
@param iBufLen[in]: pcBuf �Ĵ�С
@return  0: �ɹ�
		-1: ʧ��
*/
int iGetAdapterGUID(const char* pcAdapterDscription, char* pcBuf, int iBufLen)
{
	int iRet = -1;
	ULONG ulOutBufLen = 0;
	PIP_ADAPTER_INFO pAdapterInfo = NULL;
	PIP_ADAPTER_INFO pAdapter = NULL;

	//��ȡ������������Ϣ
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
					if (0 == strncmp(pcAdapterDscription, pAdapter->Description, strlen(pcAdapterDscription)))
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