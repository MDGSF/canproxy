#include <iostream>
#include <stdlib.h>
#include "getopt.h"
#include <string>
#include <stdio.h>
#include <Windows.h>
#include "log.h"
#include "CProxy.h"
#include "CNodeTun.h"
#include "CNodeCan.h"
#include "global.h"

void vPrintHelpMessage()
{
	printf("\
getoptdemo usage:\n \
    -h --help             Show help message.\n \
    -v --version          Show version number.\n \
    -a --tunaddr=xxx      Specify tun/tap ip address.(Default: 192.168.20.100)\n \
    -m --tunmask=xxx      Specify tun/tap ip mask.(Default: 255.255.255.0)\n \
    -d --tunad=xxx        Specify tun/tap adapter description.(Default: TAP-Windows Adapter V9)\n \
    -t --candevtype=xxx   Specify can device type.(Default: 4)\n \
    -i --candevindex=xxx  Specify can device index.(Default: 0)\n \
    -g --canindex=xxx     Specify can index.(Default: 0)\n \
    -k --canid=xxx        Specify can id.(Default: 520[0x208])\n");
	system("pause");
	exit(0);
}

void vPrintVersionNumber()
{
	printf("%s %s (%s)\n", g_acProgramName, g_acVersionNumber, __DATE__);
	system("pause");
	exit(0);
}

int iParseArguments(int argc, char** argv)
{
	const struct option long_options[] = {
		{"help", no_argument, NULL, 'h'},
		{"version", no_argument, NULL, 'v'},
		{"tunaddr", required_argument, NULL, 'a'},
		{"tunmask", required_argument, NULL, 'm'},
		{"tunad", required_argument, NULL, 'd'},
		{NULL, 0, NULL, 0}
	};

	while (true)
	{
		int option_index = 0;
		int c = getopt_long(argc, argv, "hva:m:d:", long_options, &option_index);
		if (c == -1)
		{
			break;
		}
		switch (c)
		{
		case 0:
			printf("long option %s", long_options[option_index].name);
			if (optarg) {
				printf(" with arg %s", optarg);
			}
			printf("\n");
			break;
		case 'a':
			LOG(EInfo, CANPROXY, "option a with value: %s\n", optarg);
			strcpy_s(g_acTunIPAddr, optarg);
			break;
		case 'm':
			LOG(EInfo, CANPROXY, "option m with value: %s\n", optarg);
			strcpy_s(g_acTunIPMask, optarg);
			break;
		case 'd':
			LOG(EInfo, CANPROXY, "option d with value: %s\n", optarg);
			strcpy_s(g_acTunAdapterDescription, optarg);
			break;
		case 'h':
			vPrintHelpMessage();
			break;
		case 'v':
			vPrintVersionNumber();
			break;
		case '?':
			break;
		default:
			printf("?? getopt returned character code 0%o ??\n", c);
		}
	}
	if (optind < argc)
	{
		printf("non-option argv-elements: ");
		while (optind < argc)
		{
			printf("%s ", argv[optind++]);
		}
		printf("\n");
	}
	vShowAllGloablVariable();
	return 0;
}

int main(int argc, char** argv)
{
	SetLogLevel(EInfo);
	SetLogLevel(ETrace);
	LOG(EInfo, CANPROXY, "can proxy\n");

	iParseArguments(argc, argv);

	CNodeTun stNode1;
	CNodeCan stNode2;
	CProxy* proxy = new CProxy(&stNode1, &stNode2);

	proxy->m_vStart();

	/*while (true)
	{
		Sleep(30000000000);
	}

	proxy->m_vStop();*/

	system("pause");
	return 0;
}