#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include "CProxy.h"
#include "ControlCAN.h"
#include "log.h"

using namespace std;

#define DEFAULT_BUFSIZE 4096

CProxy::CProxy(CNode* pstNode1, CNode* pstNode2)
{
	m_pstNode1 = pstNode1;
	m_pstNode2 = pstNode2;
	m_bTerminate = true;
}

CProxy::~CProxy()
{

}

void ThreadNode1ToNode2(CProxy* proxy)
{
	LOG(EInfo, CANPROXY, "ThreadNode1ToNode2 start\n");
	char acBuf[DEFAULT_BUFSIZE] = { 0 };
	int iReaded = 0;
	int iWrited = 0;
	while (!proxy->m_bGetTerminate())
	{
		iReaded = proxy->m_pstNode1->m_iRead(acBuf, DEFAULT_BUFSIZE);
		if (iReaded < 0)
		{
			LOG(EError, CANPROXY, "node1 read failed, iReaded = %d\n", iReaded);
			break;
		}
		if (iReaded == 0)
		{
			continue;
		}

		LOG(ETrace, CANPROXY, "node1 read success, iReaded = %d\n", iReaded);

		iWrited = proxy->m_pstNode2->m_iWrite(acBuf, iReaded);
		if (iWrited < 0)
		{
			LOG(EError, CANPROXY, "node2 write failed, iWrited = %d\n", iWrited);
			break;
		}
	}
	LOG(EInfo, CANPROXY, "ThreadNode1ToNode2 end\n");
}

void ThreadNode2ToNode1(CProxy* proxy)
{
	LOG(EInfo, CANPROXY, "ThreadNode2ToNode1 start\n");
	char acBuf[DEFAULT_BUFSIZE] = { 0 };
	int iReaded = 0;
	int iWrited = 0;
	while (!proxy->m_bGetTerminate())
	{
		iReaded = proxy->m_pstNode2->m_iRead(acBuf, DEFAULT_BUFSIZE);
		if (iReaded < 0)
		{
			LOG(EError, CANPROXY, "node2 read failed, iReaded = %d\n", iReaded);
			break;
		}
		if (iReaded == 0)
		{
			continue;
		}

		LOG(ETrace, CANPROXY, "node2 read success, iReaded = %d\n", iReaded);

		iWrited = proxy->m_pstNode1->m_iWrite(acBuf, iReaded);
		if (iWrited < 0)
		{
			LOG(EError, CANPROXY, "node1 write failed, iWrited = %d\n", iWrited);
			break;
		}
	}
	LOG(EInfo, CANPROXY, "ThreadNode2ToNode1 end\n");
}

void ThreadCtrl(CProxy* proxy)
{
	LOG(EInfo, CANPROXY, "ThreadCtrl start\n");
	int iRet = 0;
	while (!proxy->m_bGetTerminate())
	{
		iRet = proxy->m_pstNode1->m_iStart();
		if (iRet != 0)
		{
			LOG(EError, CANPROXY, "node1 start failed, iRet = %d\n", iRet);
			Sleep(5000);
			continue;
		}

		iRet = proxy->m_pstNode2->m_iStart();
		if (iRet != 0)
		{
			LOG(EError, CANPROXY, "node2 start failed, iRet = %d\n", iRet);
			Sleep(5000);
			continue;
		}

		std::thread t1(ThreadNode1ToNode2, proxy);
		std::thread t2(ThreadNode2ToNode1, proxy);

		t1.join();
		t2.join();

		proxy->m_pstNode1->m_iStop();
		proxy->m_pstNode2->m_iStop();
		Sleep(5000);
	}
	LOG(EInfo, CANPROXY, "ThreadCtrl end\n");
}

void CProxy::m_vStart()
{
	m_vSetTerminate(false);
	m_pstCtrlThread = new std::thread(ThreadCtrl, this);
}

void CProxy::m_vStop()
{
	m_vSetTerminate(true);
	m_pstCtrlThread->join();
}

void CProxy::m_vSetTerminate(bool val)
{
	m_stMutex.lock();
	m_bTerminate = val;
	m_stMutex.unlock();
}

bool CProxy::m_bGetTerminate()
{
	bool val;
	m_stMutex.lock();
	val = m_bTerminate;
	m_stMutex.unlock();
	return val;
}