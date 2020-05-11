#include <stdio.h>
#include <windows.h>
#include "can.h"
#include "ControlCAN.h"
#include "log.h"

/*
CAN波特率    Timing0(BTR0)    Timing1(BTR1)
10 Kbps         0x31             0x1C
20 Kbps         0x18             0x1C
40 Kbps         0x87             0xFF
50 Kbps         0x09             0x1C
80 Kbps         0x83             0xFF
100 Kbps        0x04             0x1C
125 Kbps        0x03             0x1C
200 Kbps        0x81             0xFA
250 Kbps        0x01             0x1C
400 Kbps        0x80             0xFA
500 Kbps        0x00             0x1C
666 Kbps        0x80             0xB6
800 Kbps        0x00             0x16
1000 Kbps       0x00             0x14
33.33 Kbps      0x09             0x6F
66.66 Kbps      0x04             0x6F
83.33 Kbps      0x03             0x6F
*/

#define DEFAULT_CAN_DATA_SIZE 8

int g_iDevType = VCI_USBCAN2;
int g_iDevIndex = 0;
int g_iCanIndex = 0;

/*
=0表示正常模式（相当于正常节点）
=1表示只听模式（只接收，不影响总线）
=2表示自发自收模式（环回模式）
*/
unsigned char g_ucMode = 0;

unsigned int g_uiID = 520;

/*
iCanStart 启动 can 设备。
@return 成功：返回 0。
		失败：返回 -1。
*/
int iCanStart()
{
	LOG(EInfo, CANPROXY, "can start\n");

	int ret = VCI_OpenDevice(g_iDevType, g_iDevIndex, 0);
	if (ret != 1)
	{
		LOG(EError, CANPROXY, "open device failed, ret = %d\n", ret);
		return -1;
	}

	VCI_INIT_CONFIG InitInfo[1];
	InitInfo->Timing0 = 0x00;
	InitInfo->Timing1 = 0x1C;
	InitInfo->Filter = 0;
	InitInfo->AccCode = 0x80000008;
	InitInfo->AccMask = 0xFFFFFFFF;
	InitInfo->Mode = g_ucMode;
	//初始化通道0
	if (VCI_InitCAN(g_iDevType, g_iDevIndex, g_iCanIndex, InitInfo) != 1)
	{
		LOG(EError, CANPROXY, "Init-CAN failed!\n");
		return -1;
	}
	Sleep(100);
	//初始化通道0
	if (VCI_StartCAN(g_iDevType, g_iDevIndex, g_iCanIndex) != 1)
	{
		LOG(EError, CANPROXY, "Start-CAN failed!\n");
		return -1;
	}

	LOG(EInfo, CANPROXY, "can start success\n");
	return 0;
}

/*
iCanRead 从 can 设备读取数据。
@param pcBuf[out]: 用来保存数据的内存。
@param iLen[in]: pcBuf 的大小。
@return 成功：返回成功读取到的字节数。
		失败：返回 -1。
*/
int iCanRead(char* pcBuf, int iLen)
{
	char* pcBufOffset = pcBuf;
	const int iCanObjBufLen = 200;
	VCI_CAN_OBJ pCanObj[iCanObjBufLen];
	int iReceived = VCI_Receive(g_iDevType, g_iDevIndex, g_iCanIndex, pCanObj, iCanObjBufLen, 0);
	if (iReceived == -1)
	{
		LOG(EError, CANPROXY, "USB-CAN设备不存在或USB掉线\n");
		return -1;
	}

	if (iReceived * 8 > iLen)
	{
		LOG(EError, CANPROXY, "can receive buffer too small\n");
		return -1;
	}

	for (int i = 0; i < iReceived; i++)
	{
		if (pCanObj[i].ID != g_uiID)
		{
			LOG(ETrace, CANPROXY, "can receive frame id = %d[%X], expect %d[%X]\n", pCanObj[i].ID, pCanObj[i].ID, g_uiID, g_uiID);
			continue;
		}

		if (pCanObj[i].RemoteFlag == 1)
		{
			LOG(EError, CANPROXY, "can receive remote frame, expect data frame\n");
			continue;
		}

		if (pCanObj[i].ExternFlag == 1)
		{
			LOG(EError, CANPROXY, "can receive extended frame, expect standard frame\n");
			continue;
		}

		memcpy(pcBufOffset, pCanObj[i].Data, pCanObj[i].DataLen);
		pcBufOffset += pCanObj[i].DataLen;
	}

	Sleep(10);
	return int(pcBufOffset - pcBuf);
}

int iCanWrite(const char* pcBuf, int iLen)
{
	const char* pcBufOffset = pcBuf;
	int iLeftLen = iLen;
	while (iLeftLen > 0)
	{
		const int iCanObjBufLen = 1;
		VCI_CAN_OBJ pCanObj[iCanObjBufLen];
		for (int i = 0; i < iCanObjBufLen; i++)
		{
			pCanObj[i].ID = g_uiID;
			pCanObj[i].RemoteFlag = 0;
			pCanObj[i].ExternFlag = 0;

			if (iLeftLen >= DEFAULT_CAN_DATA_SIZE)
			{
				pCanObj[i].DataLen = DEFAULT_CAN_DATA_SIZE;
				memcpy(pCanObj[i].Data, pcBufOffset, DEFAULT_CAN_DATA_SIZE);
				pcBufOffset += DEFAULT_CAN_DATA_SIZE;
				iLeftLen -= DEFAULT_CAN_DATA_SIZE;
			}
			else
			{
				pCanObj[i].DataLen = iLeftLen;
				memcpy(pCanObj[i].Data, pcBufOffset, iLeftLen);
				pcBufOffset += iLeftLen;
				iLeftLen = 0;
			}
		}
		int ret = VCI_Transmit(g_iDevType, g_iDevIndex, g_iCanIndex, pCanObj, iCanObjBufLen);
		if (ret == -1)
		{
			LOG(EError, CANPROXY, "USB-CAN设备不存在或USB掉线\n");
			return -1;
		}
	}
	return iLen - iLeftLen;
}

/*
vCanStop 关闭 can 设备。
*/
void vCanStop()
{
	DWORD ulRet = VCI_CloseDevice(g_iDevType, g_iDevIndex);
	if (ulRet == 1)
	{
		// 成功
	}
	else if (ulRet == 0)
	{
		LOG(EError, CANPROXY, "VCI_CloseDevice failed\n");
	}
	else if (ulRet == -1)
	{
		LOG(EError, CANPROXY, "USB-CAN设备不存在或USB掉线\n");
	}
	else
	{
		// unknown
	}
}
