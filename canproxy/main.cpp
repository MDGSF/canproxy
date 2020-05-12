#include <stdio.h>
#include <Windows.h>
#include "log.h"
#include "CProxy.h"
#include "CNodeTun.h"
#include "CNodeCan.h"

int main()
{
	SetLogLevel(EInfo);
	SetLogLevel(ETrace);
	LOG(EInfo, CANPROXY, "can proxy\n");

	CNodeTun stNode1;
	CNodeCan stNode2;
	CProxy* proxy = new CProxy(&stNode1, &stNode2);
	
	proxy->m_vStart();

	while (true)
	{
		Sleep(30000000000);
	}

	proxy->m_vStop();

	system("pause");
	return 0;
}