#include <stdio.h>
#include <windows.h>
#include "can.h"
#include "ControlCAN.h"
#include "log.h"
#include "unitcodec.h"
#include "global.h"

void vPrintBuf(const char* pcBuf, int iLen)
{
	printf("iLen = %d, [", iLen);
	for (int i = 0; i < iLen; i++)
	{
		printf("%02X ", pcBuf[i]);
	}
	printf("]\n");
}

/*
CAN������    Timing0(BTR0)    Timing1(BTR1)
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

/*
=0��ʾ����ģʽ���൱�������ڵ㣩
=1��ʾֻ��ģʽ��ֻ���գ���Ӱ�����ߣ�
=2��ʾ�Է�����ģʽ������ģʽ��
*/
unsigned char g_ucMode = 0;

/*
iCanStart ���� can �豸��
@return �ɹ������� 0��
		ʧ�ܣ����� -1��
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
	//��ʼ��ͨ��0
	if (VCI_InitCAN(g_iDevType, g_iDevIndex, g_iCanIndex, InitInfo) != 1)
	{
		LOG(EError, CANPROXY, "Init-CAN failed!\n");
		return -1;
	}
	Sleep(100);
	//��ʼ��ͨ��0
	if (VCI_StartCAN(g_iDevType, g_iDevIndex, g_iCanIndex) != 1)
	{
		LOG(EError, CANPROXY, "Start-CAN failed!\n");
		return -1;
	}

	LOG(EInfo, CANPROXY, "can start success\n");
	return 0;
}

/*
iCanRead �� can �豸��ȡ���ݡ�
@param pcBuf[out]: �����������ݵ��ڴ档
@param iLen[in]: pcBuf �Ĵ�С��
@return �ɹ������سɹ���ȡ�����ֽ�����
		ʧ�ܣ����� -1��
*/
int iCanRead(char* pcBuf, int iLen)
{
	VCI_CAN_OBJ stCanHeader;
	int iReceived = VCI_Receive(g_iDevType, g_iDevIndex, g_iCanIndex, &stCanHeader, 1, 0);
	if (iReceived == -1)
	{
		LOG(EError, CANPROXY, "USB-CAN�豸�����ڻ�USB����\n");
		return -1;
	}
	if (iReceived == 0 || iReceived != 1 || stCanHeader.DataLen != 8)
	{
		return 0;
	}
	if (stCanHeader.ID != g_uiCanID)
	{
		// LOG(ETrace, CANPROXY, "can receive frame id = %d[%X], expect %d[%X]\n", pCanObj[i].ID, pCanObj[i].ID, g_uiCanID, g_uiCanID);
		return 0;
	}

	uint64_t u64PayloadCanFrameNum = 0;
	char* pcData = (char*)stCanHeader.Data;
	uint16_t usDataLen = stCanHeader.DataLen;
	if (EXIT_SUCCESS != iDecode64(&u64PayloadCanFrameNum, &pcData, &usDataLen))
	{
		return -1;
	}

	PVCI_CAN_OBJ pCanObj = (PVCI_CAN_OBJ)malloc(u64PayloadCanFrameNum * sizeof(VCI_CAN_OBJ));
	if (NULL == pCanObj)
	{
		LOG(EError, CANPROXY, "malloc failed, u64PayloadCanFrameNum = %d\n", u64PayloadCanFrameNum);
		return -1;
	}

	LOG(EInfo, CANPROXY, "can read header success, u64PayloadCanFrameNum = %d\n", u64PayloadCanFrameNum);

	char* pcBufOffset = pcBuf;
	int iTotalReceived = 0;
	while (iTotalReceived < u64PayloadCanFrameNum)
	{
		iReceived = VCI_Receive(g_iDevType, g_iDevIndex, g_iCanIndex, pCanObj, u64PayloadCanFrameNum - iTotalReceived, 0);
		if (iReceived == -1)
		{
			LOG(EError, CANPROXY, "USB-CAN�豸�����ڻ�USB����\n");
			free(pCanObj);
			return -1;
		}

		for (int i = 0; i < iReceived; i++)
		{
			if (pCanObj[i].ID != g_uiCanID)
			{
				// LOG(ETrace, CANPROXY, "can receive frame id = %d[%X], expect %d[%X]\n", pCanObj[i].ID, pCanObj[i].ID, g_uiCanID, g_uiCanID);
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

			iTotalReceived += 1;

			memcpy(pcBufOffset, pCanObj[i].Data, pCanObj[i].DataLen);
			pcBufOffset += pCanObj[i].DataLen;
		}
	}

	LOG(EInfo, CANPROXY, "can read pay load success, len = %d\n", pcBufOffset - pcBuf);

	free(pCanObj);
	Sleep(10);
	return int(pcBufOffset - pcBuf);
}

int iCanWrite(const char* pcBuf, int iLen)
{
	LOG(ETrace, CANPROXY, "iCanWrite start, iLen = %d\n", iLen);

	// send header
	uint64_t u64PayloadCanFrameNum = iLen / 8;
	if (iLen % 8 != 0)
	{
		u64PayloadCanFrameNum++;
	}

	VCI_CAN_OBJ stCanHeader;
	stCanHeader.ID = g_uiCanID;
	stCanHeader.RemoteFlag = 0;
	stCanHeader.ExternFlag = 0;
	stCanHeader.DataLen = DEFAULT_CAN_DATA_SIZE;

	char* pcData = (char*)stCanHeader.Data;
	uint16_t usDataLen = DEFAULT_CAN_DATA_SIZE;
	if (EXIT_SUCCESS != iEncode64(u64PayloadCanFrameNum, &pcData, &usDataLen))
	{
		LOG(EError, CANPROXY, "encode can header failed\n");
		return -1;
	}

	int ret = VCI_Transmit(g_iDevType, g_iDevIndex, g_iCanIndex, &stCanHeader, 1);
	if (ret == -1)
	{
		LOG(EError, CANPROXY, "USB-CAN�豸�����ڻ�USB����\n");
		return -1;
	}

	LOG(ETrace, CANPROXY, "iCanWrite send header success, u64PayloadCanFrameNum = %d\n", u64PayloadCanFrameNum);
	vPrintBuf((const char *)stCanHeader.Data, 8);

	// send pay load
	const char* pcBufOffset = pcBuf;
	int iLeftLen = iLen;
	while (iLeftLen > 0)
	{
		const int iCanObjBufLen = 1;
		VCI_CAN_OBJ pCanObj[iCanObjBufLen];
		for (int i = 0; i < iCanObjBufLen; i++)
		{
			pCanObj[i].ID = g_uiCanID;
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

		/*LOG(ETrace, CANPROXY, "iCanWrite send payload, iLeftLen = %d\n", iLeftLen);
		vPrintBuf((const char*)pCanObj[0].Data, pCanObj[0].DataLen);*/

		/*printf("[");
		for (int i = 0; i < pCanObj[0].DataLen; i++)
		{
			printf("%02X ", pCanObj[0].Data[i]);
		}
		printf("]\n");*/

		int ret = VCI_Transmit(g_iDevType, g_iDevIndex, g_iCanIndex, pCanObj, iCanObjBufLen);
		if (ret == -1)
		{
			LOG(EError, CANPROXY, "USB-CAN�豸�����ڻ�USB����\n");
			return -1;
		}
	}

	LOG(ETrace, CANPROXY, "iCanWrite end\n");
	return iLen - iLeftLen;
}

/*
vCanStop �ر� can �豸��
*/
void vCanStop()
{
	DWORD ulRet = VCI_CloseDevice(g_iDevType, g_iDevIndex);
	if (ulRet == 1)
	{
		// �ɹ�
	}
	else if (ulRet == 0)
	{
		LOG(EError, CANPROXY, "VCI_CloseDevice failed\n");
	}
	else if (ulRet == -1)
	{
		LOG(EError, CANPROXY, "USB-CAN�豸�����ڻ�USB����\n");
	}
	else
	{
		// unknown
	}
}
