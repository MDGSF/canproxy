#include <stdio.h>
#include "tap.h"
#include "tap-windows.h"
#include "log.h"

int main()
{
	SetLogLevel(EInfo);
	LOG(EInfo, TAPDEMO, "tap demo\n");
	vShowAllAdapterInfo();
	vShowAllRegConnectionInfo();
	TAPHandle handle = iTapOpen("192.168.20.100", "255.255.255.0");
	if (handle == NULL)
	{
		LOG(EError, TAPDEMO, "tap open failed, invalid handle\n");
		system("pause");
		return -1;
	}

	while (true)
	{
		char acBuf[1024] = { 0 };
		int iReaded = iTapRead(handle, acBuf, 1024);
		LOG(EInfo, TAPDEMO, "tap read len = %d\n", iReaded);
		for (int i = 0; i < iReaded; i++)
		{
			printf("%02X ", acBuf[i]);
		}
		printf("\n");
		Sleep(1000);
	}

	system("pause");
	return 0;
}