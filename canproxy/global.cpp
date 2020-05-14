#include "global.h"

char g_acProgramName[BUFSIZ] = "canproxy";
char g_acVersionNumber[BUFSIZ] = "1.0.0";

char g_acTunIPAddr[BUFSIZ] = "192.168.20.100";
char g_acTunIPMask[BUFSIZ] = "255.255.255.0";
char g_acTunAdapterDescription[BUFSIZ] = "TAP-Windows Adapter V9";

// VCI_USBCAN2=4
int g_iDevType = 4;
int g_iDevIndex = 0;
int g_iCanIndex = 0;
unsigned int g_uiCanID = 520;


void vShowAllGloablVariable()
{
	printf("g_acProgramName = %s\n", g_acProgramName);
	printf("g_acVersionNumber = %s\n", g_acVersionNumber);

	printf("g_acTunIPAddr = %s\n", g_acTunIPAddr);
	printf("g_acTunIPMask = %s\n", g_acTunIPMask);
	printf("g_acTunAdapterDescription = %s\n", g_acTunAdapterDescription);

	printf("g_iDevType = %d\n", g_iDevType);
	printf("g_iDevIndex = %d\n", g_iDevIndex);
	printf("g_iCanIndex = %d\n", g_iCanIndex);
	printf("g_uiCanID = %d\n", g_uiCanID);
}