#pragma once

#include <mutex>
#include <thread>
#include "CNode.h"

class CProxy
{
public:
	CProxy(CNode* pstNode1, CNode* pstNode2);
	~CProxy();

	void m_vStart();
	void m_vStop();
	void m_vSetTerminate(bool val);
	bool m_bGetTerminate();

	CNode* m_pstNode1;
	CNode* m_pstNode2;

private:
	bool m_bTerminate;
	std::mutex m_stMutex;
	std::thread* m_pstCtrlThread;
};

