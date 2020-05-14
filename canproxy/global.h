#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

extern char g_acProgramName[BUFSIZ];
extern char g_acVersionNumber[BUFSIZ];

extern char g_acTunIPAddr[BUFSIZ];
extern char g_acTunIPMask[BUFSIZ];
extern char g_acTunAdapterDescription[BUFSIZ];

extern int g_iDevType;
extern int g_iDevIndex;
extern int g_iCanIndex;
extern unsigned int g_uiCanID;


extern void vShowAllGloablVariable();
