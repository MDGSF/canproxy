#pragma once

class CProxy
{
public:
	CProxy();
	~CProxy();

	int m_Start();

	void ReceiveCanThread();

	// m_iDevType ȡֵΪ: VCI_USBCAN1��VCI_USBCAN2��VCI_USBCAN2A
	int m_iDevType;

	// m_iDevIndex �豸���������統ֻ��һ��USB-CAN������ʱ��������Ϊ0����ʱ�ٲ���һ��USB-CAN��
	// ������ô������������豸�����ž���1���Դ����ơ�
	int m_iDevIndex;

	// m_iCanIndex CAN ͨ�������� �ڼ�· CAN������Ӧ����CANͨ���ţ� CAN1Ϊ0�� CAN2Ϊ1��
	int m_iCanIndex;
};
