/*
    形式如“a:b::cd:“，分别表示程序支持的命令行短选项有-a、-b、-c、-d，冒号含义如下：
    (1)只有一个字符，不带冒号——只表示选项， 如-c 
    (2)一个字符，后接一个冒号——表示选项后面带一个参数，如-a 100
    (3)一个字符，后接两个冒号——表示选项后面带一个可选参数，即参数可有可无，如果带参数，则选项与参数直接不能有空格
        形式应该如-b200

*/

#include <iostream>
#include <stdlib.h>
#include "getopt.h"

void vPrintHelpMessage()
{
	printf("\
getoptdemo usage:\n \
    -h --help      Show help message.\n \
    -v --version   Show version number.\n \
    -i --ip=xxx    Specify ip address.\n \
    -p --port=xxx  Specify ip port.\n");
	exit(0);
}

int iParseArguments(int argc, char** argv)
{
	const struct option long_options[] = {
		{"help", no_argument, NULL, 'h'},
		{"version", no_argument, NULL, 'v'},
		{"ip", required_argument, NULL, 'i'},
		{"port", required_argument, NULL, 'p'},
		{NULL, 0, NULL, 0}
	};

	while (true)
	{
		int digit_optind = 0;
		int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		int c = getopt_long(argc, argv, "hvabc:i:p:", long_options, &option_index);
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
			printf("option a\n");
			break;
		case 'b':
			printf("option b\n");
			break;
		case 'c':
			printf("option c with value: %s\n", optarg);
			break;
		case 'i':
			printf("option i with value: %s\n", optarg);
			break;
		case 'p':
			printf("option p with value: %s\n", optarg);
			break;
		case 'h':
			vPrintHelpMessage();
			break;
		case 'v':
			printf("option v\n");
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
	return 0;
}

int main(int argc, char** argv) 
{
	iParseArguments(argc, argv);
	return 0;
}