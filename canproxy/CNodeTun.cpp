#include <stdio.h>
#include "global.h"
#include "CNodeTun.h"

const char* CNodeTun::m_pcName()
{
	return "Tun";
}

int CNodeTun::m_iStart()
{
	m_handle = iTapOpen(g_acTunIPAddr, g_acTunIPMask, g_acTunAdapterDescription);
	if (m_handle == NULL)
	{
		return -1;
	}
	return 0;
}

int CNodeTun::m_iRead(char* pcBuf, int iLen)
{
	return iTapRead(m_handle, pcBuf, iLen);
}

int CNodeTun::m_iWrite(const char* pcBuf, int iLen)
{
	return iTapWrite(m_handle, pcBuf, iLen);
}

int CNodeTun::m_iStop()
{
	vTapClose(m_handle);
	return 0;
}