#ifndef UNIT_CODEC_H
#define UNIT_CODEC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define IS_BIG_ENDIAN 0


int32_t iEncode8(uint8_t ucValue, char ** ppcBuf, uint16_t * pusEncodeLen);

int32_t iDecode8(uint8_t * pucValue, char ** ppcMsg, uint16_t * pusMsgLen);


int32_t iEncode16(uint16_t usValue, char ** ppcBuf, uint16_t * pusEncodeLen);

int32_t iDecode16(uint16_t * pusValue, char ** ppcMsg, uint16_t * pusMsgLen);


int32_t iEncode32(uint32_t uiValue, char ** ppcBuf, uint16_t * pusEncodeLen);

int32_t iDecode32(uint32_t * puiValue, char ** ppcMsg, uint16_t * pusMsgLen);


int32_t iEncode64(uint64_t ullValue, char ** ppcBuf, uint16_t * pusEncodeLen);

int32_t iDecode64(uint64_t * pullValue, char ** ppcMsg, uint16_t * pusMsgLen);


#endif
