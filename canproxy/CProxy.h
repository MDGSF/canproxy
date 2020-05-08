#pragma once

class CProxy
{
public:
	CProxy();
	~CProxy();

	int m_Start();

	void ReceiveCanThread();

	// m_iDevType 取值为: VCI_USBCAN1，VCI_USBCAN2，VCI_USBCAN2A
	int m_iDevType;

	// m_iDevIndex 设备索引，比如当只有一个USB-CAN适配器时，索引号为0，这时再插入一个USB-CAN适
	// 配器那么后面插入的这个设备索引号就是1，以此类推。
	int m_iDevIndex;

	// m_iCanIndex CAN 通道索引。 第几路 CAN。即对应卡的CAN通道号， CAN1为0， CAN2为1。
	int m_iCanIndex;
};
