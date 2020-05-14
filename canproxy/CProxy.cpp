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
	m_pstCtrlThread = NULL;
}

CProxy::~CProxy()
{

}

void ThreadNodeProxy(CProxy* pstProxy, CNode* pstSrc, CNode* pstDst)
{
	LOG(EInfo, CANPROXY, "ThreadNodeProxy start, %s --> %s\n",
		pstSrc->m_pcName(), pstDst->m_pcName());
	char acBuf[DEFAULT_BUFSIZE] = { 0 };
	int iReaded = 0;
	int iWrited = 0;
	while (!pstProxy->m_bGetTerminate())
	{
		iReaded = pstSrc->m_iRead(acBuf, DEFAULT_BUFSIZE);
		if (iReaded < 0)
		{
			LOG(EError, CANPROXY, "[%s] read failed, iReaded = %d\n",
				pstSrc->m_pcName(), iReaded);
			break;
		}
		if (iReaded == 0)
		{
			continue;
		}

		LOG(ETrace, CANPROXY, "[%s] read success, iReaded = %d\n", pstSrc->m_pcName(), iReaded);

		iWrited = pstDst->m_iWrite(acBuf, iReaded);
		if (iWrited < 0)
		{
			LOG(EError, CANPROXY, "[%s] write failed, iWrited = %d\n", pstDst->m_pcName(), iWrited);
			break;
		}
	}
	LOG(EInfo, CANPROXY, "ThreadNodeProxy end, %s --> %s\n",
		pstSrc->m_pcName(), pstDst->m_pcName());
}

void ThreadCtrl(CProxy* pstProxy)
{
	LOG(EInfo, CANPROXY, "ThreadCtrl start\n");
	int iRet = 0;
	CNode* pstNode1 = pstProxy->m_pstNode1;
	CNode* pstNode2 = pstProxy->m_pstNode2;
	while (!pstProxy->m_bGetTerminate())
	{
		LOG(EInfo, CANPROXY, "ThreadCtrl Trying to start proxy between [%s] and [%s]\n",
			pstNode1->m_pcName(), pstNode2->m_pcName());
		iRet = pstNode1->m_iStart();
		if (iRet != 0)
		{
			LOG(EError, CANPROXY, "node1(%s) start failed, iRet = %d\n", pstNode1->m_pcName(), iRet);
			Sleep(5000);
			continue;
		}

		iRet = pstNode2->m_iStart();
		if (iRet != 0)
		{
			LOG(EError, CANPROXY, "node2(%s) start failed, iRet = %d\n", pstNode1->m_pcName(), iRet);
			Sleep(5000);
			continue;
		}

		std::thread t1(ThreadNodeProxy, pstProxy, pstNode1, pstNode2);
		std::thread t2(ThreadNodeProxy, pstProxy, pstNode2, pstNode1);

		t1.join();
		t2.join();

		pstNode1->m_iStop();
		pstNode2->m_iStop();
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