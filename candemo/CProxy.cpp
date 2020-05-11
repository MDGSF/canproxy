#include <stdio.h>
#include <Windows.h>
#include <thread>
#include <iostream>
#include <mutex>
#include "CProxy.h"
#include "ControlCAN.h"

using namespace std;

void ReceiveTunThread(CProxy* proxy)
{
	printf("ReceiveTunThread start");
	while (true)
	{
		const int iCanObjBufLen = 1;
		VCI_CAN_OBJ pCanObj[iCanObjBufLen];
		for (int i = 0; i < iCanObjBufLen; i++)
		{
			pCanObj[i].ID = i;
			pCanObj[i].RemoteFlag = 0;
			pCanObj[i].ExternFlag = 0;
			pCanObj[i].DataLen = 8;
			for (int j = 0; j < 8; j++)
			{
				pCanObj[i].Data[j] = j;
			}
		}
		int ret = VCI_Transmit(proxy->m_iDevType, proxy->m_iDevIndex, proxy->m_iCanIndex, pCanObj, iCanObjBufLen);
		if (ret == -1) 
		{
			// USB-CAN设备不存在或USB掉线
			return;
		}
		printf("send ret = %d\n", ret);
		Sleep(3000);
	}
}

CProxy::CProxy()
{
	printf("CPoxy constructor\n");
	m_iDevType = VCI_USBCAN2;
	m_iDevIndex = 0;
	m_iCanIndex = 0;
}

CProxy::~CProxy()
{
}

int CProxy::m_Start()
{
	printf("proxy start\n");

	int ret = VCI_OpenDevice(m_iDevType, m_iDevIndex, 0);
	if (ret != 1)
	{
		printf("open device failed, ret = %d\n", ret);
		return -1;
	}

	VCI_INIT_CONFIG InitInfo[1];
	InitInfo->Timing0 = 0x00;
	InitInfo->Timing1 = 0x1C;
	InitInfo->Filter = 0;
	InitInfo->AccCode = 0x80000008;
	InitInfo->AccMask = 0xFFFFFFFF;
	InitInfo->Mode = 2;
	//初始化通道0
	if (VCI_InitCAN(m_iDevType, m_iDevIndex, m_iCanIndex, InitInfo) != 1)
	{
		printf("Init-CAN failed!");
		return -1;
	}
	Sleep(100);
	//初始化通道0
	if (VCI_StartCAN(m_iDevType, m_iDevIndex, m_iCanIndex) != 1)
	{
		printf("Start-CAN failed!");
		return -1;
	}

	printf("proxy start success\n");

	std::thread t1(ReceiveTunThread, this);

	ReceiveCanThread();

	t1.join();

	return 0;
}

void CProxy::ReceiveCanThread()
{
	while (true)
	{
		const int iCanObjBufLen = 200;
		VCI_CAN_OBJ pCanObj[iCanObjBufLen];
		int iReceived = VCI_Receive(m_iDevType, m_iDevIndex, m_iCanIndex, pCanObj, iCanObjBufLen, 0);
		if (iReceived == -1)
		{
			// USB-CAN设备不存在或USB掉线
			return;
		}

		for (int i = 0; i < iReceived; i++)
		{
			printf("%08X ", pCanObj[i].ID);

			if (pCanObj[i].RemoteFlag == 1)
			{
				printf("Remote ");
			}
			else
			{
				printf("Data ");
			}

			if (pCanObj[i].ExternFlag == 1)
			{
				printf("Extended ");
			}
			else
			{
				printf("Standard ");
			}

			printf("len = %d, [", pCanObj[i].DataLen);
			for (int j = 0; j < pCanObj[i].DataLen; j++)
			{
				printf("%02X ", pCanObj[i].Data[j]);
			}

			printf("]\n");
		}

		Sleep(10);
	}
}