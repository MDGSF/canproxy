#include "unitcodec.h"

int32_t
iEncode8(uint8_t ucValue, char ** ppcBuf, uint16_t * pusEncodeLen)
{
	if(NULL == ppcBuf || NULL == *ppcBuf || NULL == pusEncodeLen) {
		return EXIT_FAILURE;
	}
	(*ppcBuf)[0] = ucValue;
	*ppcBuf += sizeof(uint8_t);
	*pusEncodeLen += sizeof(uint8_t);
	return EXIT_SUCCESS;
}

int32_t
iDecode8(uint8_t * pucValue, char ** ppcMsg, uint16_t * pusMsgLen)
{
	if(NULL == pucValue || NULL == ppcMsg || NULL == *ppcMsg ||
	   NULL == pusMsgLen || *pusMsgLen < sizeof(uint8_t))
	{
		return EXIT_FAILURE;
	}
	pucValue[0] = (*ppcMsg)[0];
	*ppcMsg += sizeof(uint8_t);
	*pusMsgLen -= sizeof(uint8_t);
	return EXIT_SUCCESS;
}


int32_t
iEncode16(uint16_t usValue, char ** ppcBuf, uint16_t * pusEncodeLen)
{
	if(NULL == ppcBuf || NULL == *ppcBuf || NULL == pusEncodeLen) {
		return EXIT_FAILURE;
	}

	char * pcValue = (char*)&usValue;
#if IS_BIG_ENDIAN
	(*ppcBuf)[0] = pcValue[0];
	(*ppcBuf)[1] = pcValue[1];
#else
	(*ppcBuf)[1] = pcValue[0];
	(*ppcBuf)[0] = pcValue[1];
#endif

	*ppcBuf += sizeof(uint16_t);
	*pusEncodeLen += sizeof(uint16_t);
	return EXIT_SUCCESS;
}

int32_t
iDecode16(uint16_t * pusValue, char ** ppcMsg, uint16_t * pusMsgLen)
{
	if(NULL == pusValue || NULL == ppcMsg || NULL == *ppcMsg ||
	   NULL == pusMsgLen || *pusMsgLen < sizeof(uint16_t))
	{
		return EXIT_FAILURE;
	}

	char * pcValue = (char*)pusValue;
#if IS_BIG_ENDIAN
	pcValue[0] = (*ppcMsg)[0];
	pcValue[1] = (*ppcMsg)[1];
#else
	pcValue[1] = (*ppcMsg)[0];
	pcValue[0] = (*ppcMsg)[1];
#endif

	*ppcMsg += sizeof(uint16_t);
	*pusMsgLen -= sizeof(uint16_t);
	return EXIT_SUCCESS;
}


int32_t
iEncode32(uint32_t uiValue, char ** ppcBuf, uint16_t * pusEncodeLen)
{
	if(NULL == ppcBuf || NULL == *ppcBuf || NULL == pusEncodeLen) {
		return EXIT_FAILURE;
	}

	char * pcValue = (char*)&uiValue;
#if IS_BIG_ENDIAN
	(*ppcBuf)[0] = pcValue[0];
	(*ppcBuf)[1] = pcValue[1];
	(*ppcBuf)[2] = pcValue[2];
	(*ppcBuf)[3] = pcValue[3];
#else
	(*ppcBuf)[3] = pcValue[0];
	(*ppcBuf)[2] = pcValue[1];
	(*ppcBuf)[1] = pcValue[2];
	(*ppcBuf)[0] = pcValue[3];
#endif

	*ppcBuf += sizeof(uint32_t);
	*pusEncodeLen += sizeof(uint32_t);
	return EXIT_SUCCESS;
}

int32_t
iDecode32(uint32_t * puiValue, char ** ppcMsg, uint16_t * pusMsgLen)
{
	if(NULL == puiValue || NULL == ppcMsg || NULL == *ppcMsg ||
	   NULL == pusMsgLen || *pusMsgLen < sizeof(uint32_t))
	{
		return EXIT_FAILURE;
	}

	char * pcValue = (char*)puiValue;
#if IS_BIG_ENDIAN
	pcValue[0] = (*ppcMsg)[0];
	pcValue[1] = (*ppcMsg)[1];
	pcValue[2] = (*ppcMsg)[2];
	pcValue[3] = (*ppcMsg)[3];
#else
	pcValue[3] = (*ppcMsg)[0];
	pcValue[2] = (*ppcMsg)[1];
	pcValue[1] = (*ppcMsg)[2];
	pcValue[0] = (*ppcMsg)[3];
#endif

	*ppcMsg += sizeof(uint32_t);
	*pusMsgLen -= sizeof(uint32_t);
	return EXIT_SUCCESS;
}

int32_t
iEncode64(uint64_t ullValue, char ** ppcBuf, uint16_t * pusEncodeLen)
{
	if(NULL == ppcBuf || NULL == *ppcBuf || NULL == pusEncodeLen) {
		return EXIT_FAILURE;
	}

	char * pcValue = (char*)&ullValue;
#if IS_BIG_ENDIAN
	memcpy(*ppcBuf, pcValue, sizeof(uint64_t));
#else
	(*ppcBuf)[7] = pcValue[0];
	(*ppcBuf)[6] = pcValue[1];
	(*ppcBuf)[5] = pcValue[2];
	(*ppcBuf)[4] = pcValue[3];
	(*ppcBuf)[3] = pcValue[4];
	(*ppcBuf)[2] = pcValue[5];
	(*ppcBuf)[1] = pcValue[6];
	(*ppcBuf)[0] = pcValue[7];
#endif

	*ppcBuf += sizeof(uint64_t);
	*pusEncodeLen += sizeof(uint64_t);
	return EXIT_SUCCESS;
}

int32_t
iDecode64(uint64_t * pullValue, char ** ppcMsg, uint16_t * pusMsgLen)
{
	if(NULL == pullValue || NULL == ppcMsg || NULL == *ppcMsg ||
	   NULL == pusMsgLen || *pusMsgLen < sizeof(uint64_t))
	{
		return EXIT_FAILURE;
	}

	char * pcValue = (char*)pullValue;
#if IS_BIG_ENDIAN
	memcpy(pcValue, *ppcMsg, sizeof(uint64_t));
#else
	pcValue[7] = (*ppcMsg)[0];
	pcValue[6] = (*ppcMsg)[1];
	pcValue[5] = (*ppcMsg)[2];
	pcValue[4] = (*ppcMsg)[3];
	pcValue[3] = (*ppcMsg)[4];
	pcValue[2] = (*ppcMsg)[5];
	pcValue[1] = (*ppcMsg)[6];
	pcValue[0] = (*ppcMsg)[7];
#endif

	*ppcMsg += sizeof(uint64_t);
	*pusMsgLen -= sizeof(uint64_t);
	return EXIT_SUCCESS;
}


