/*
*********************************************************************************************************
*
*	ģ������ : SIM800��������
*	�ļ����� : bsp_SIM800.c
*	��    �� : V1.0
*	˵    �� : ��װSIM800ģ����ص�AT����
*********************************************************************************************************
*/
#include <stdio.h>
#include <string.h>

#include "sim800.h"
#include "usart.h"
#include "SysTick.h"

/*
	SIM800_TERM_ON   �� PG6
	SIM800_RESET   	 �� PG7
*/

/*
	AT+CIPSTATUS ��ѯ��ǰTCP����״̬
	
	AT+CIPSEND TCP��������
	
	AT+CIPSTART ����TCP����
	
	AT+CIPCLOSE �ر�TCP����
	
	AT+CIFSR ��ѯģ��IP

	AT+CREG?  ��ѯ��ǰ����״̬

	AT+CSQ ��ѯ�ź���������

	AT+CIMI ��ѯSIM ����IMSI �š�
	
	ATI ��ѯģ���Ӳ����Ϣ
	
	AT+CIND? ��ȡ��ǰ��ָʾ״̬

	ATA ��������
	ATH �Ҷ���������
	
	ATI ��ʾSIM800ģ���Ӳ����Ϣ
	
	ATD10086; ����10086

	AT^SWSPATH=<n>  �л���Ƶͨ��
*/


/* STM32��GSM��TERM_ON���ż���1��NPN�����ܣ������Ҫ���� */
#define TERM_ON_1()		GPIOG->BSRRH =  GPIO_Pin_6//GPRS_PWR_LOW()
#define TERM_ON_0()		GPIOG->BSRRL =  GPIO_Pin_6//GPRS_PWR_HIGH()

/* STM32��MG323��RESET���ż���1��NPN�����ܣ������Ҫ����. MG323��RESET���ǵ����帴λ */
//#define MG_RESET_1()	GPRS_RST_LOW()
//#define MG_RESET_0()	GPRS_RST_HIGH()

uint16_t GPRS_msTimeOut;
//char CmdBuf[24 * 1024];
/*
*********************************************************************************************************
*	�� �� ��: InitSIM800
*	����˵��: ��������ģ����ص�GPIO
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void InitSIM800(void)
{
//�����ź�TERM_ON�͸�λ�ź�RESET������interface.c�ļ����HW_GPIOInit()ʵ��
//���ڲ�����ʼ����usart.c�ļ���ʵ��
}


/*
*********************************************************************************************************
*	�� �� ��: SIM800_PrintRxData
*	����˵��: ��ӡSTM32��SIM800�յ������ݵ�COM1���ڣ���Ҫ���ڸ��ٵ���
*	��    ��: _ch : �յ�������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SIM800_PrintRxData(uint8_t _ch)
{
#ifdef SIM800_TO_COM1_EN
	UARTSend(PORT_DEBUG, _ch);		/* �����յ����ݴ�ӡ�����Դ���1 */
#endif
}



/*
*********************************************************************************************************
*	�� �� ��: MG323_PowerOn
*	����˵��: ��MG323ģ���ϵ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SIM800_PowerOn(void)
{
	/* ���㴮�ڽ��ջ����� */
	if (Bios_PortRead(COM_SIM800, NULL, 0, 0, NULL))
		Bios_PortRead(COM_SIM800, NULL, -1, 0, NULL);	

	TERM_ON_0();
	GPRS_msTimeOut = GetSystemTimer();
	while((GetSystemTimer()-GPRS_msTimeOut)<1200);	//�����ź�1.2S
	TERM_ON_1();
	
	/* �ȴ�ģ������ϵ磬�ж��Ƿ���յ� ^SYSSTART */
	//SIM800_WaitResponse("RDY", 5000);
}


/*
*********************************************************************************************************
*	�� �� ��: SIM800_PowerOff
*	����˵��: ����SIM800ģ��ػ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SIM800_PowerOff(void)
{
	/* Ӳ���ػ� */
	

	/* Ҳ��������ػ� */
	SIM800_SendAT("AT+CPOWD=1");
}
/*
*********************************************************************************************************
*	�� �� ��: MG323_PowerOn
*	����˵��: ��MG323ģ���ϵ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SIM800_Reset(void)
{
	/*
		����MG323�ֲᣬ
		RESET �ܽ�����ʵ��ģ��Ӳ����λ����ģ�����������������ʱ��ͨ������
		RESET �ܽ� �� 10 ms ��ģ�����Ӳ����λ��
	*/

}
/*
*********************************************************************************************************
*	�� �� ��: SIM800_WaitResponse
*	����˵��: �ȴ�SIM800����ָ����Ӧ���ַ���. ����ȴ� OK
*	��    ��: _pAckStr : Ӧ����ַ����� ���Ȳ��ó���255
*			 _usTimeOut : ����ִ�г�ʱ��0��ʾһֱ�ȴ�. >����ʾ��ʱʱ�䣬��λ1ms
*	�� �� ֵ: 1 ��ʾ�ɹ�  0 ��ʾʧ��
*********************************************************************************************************
*/
uint8_t SIM800_WaitResponse(char *_pAckStr, uint16_t _usTimeOut)
{
	uint8_t ucData;
	uint8_t ucRxBuf[256];
	uint16_t pos = 0;
	uint32_t len;
	uint8_t ret;

	len = strlen(_pAckStr);
	if (len > 255)
	{
		return 0;
	}
	
	GPRS_msTimeOut = GetSystemTimer();	//��0���¼�ʱ
	
	while (1)
	{

		if (_usTimeOut > 0)
		{
			if ((GetSystemTimer()-GPRS_msTimeOut) > _usTimeOut)
			{
				ret = 0;	/* ��ʱ */
				break;
			}
		}

		if (UARTGet(COM_SIM800, &ucData, 0))
		{
			SIM800_PrintRxData(ucData);		/* �����յ����ݴ�ӡ�����Դ���1 */

			if (ucData == '\n')
			{
				if (pos > 0)	/* ��2���յ��س����� */
				{
					if (memcmp(ucRxBuf, _pAckStr,  len) == 0)
					{
						ret = 1;	/* �յ�ָ����Ӧ�����ݣ����سɹ� */
						break;
					}
					else
					{
						pos = 0;
					}
				}
				else
				{
					pos = 0;
				}
			}
			else
			{
				if (pos < sizeof(ucRxBuf))
				{
					/* ֻ����ɼ��ַ� */
					if (ucData >= ' ')
					{
						ucRxBuf[pos++] = ucData;
					}
				}
			}
		}
	}
	return ret;
}

/*
*********************************************************************************************************
*	�� �� ��: SIM800_ReadResponse
*	����˵��: ��ȡSIM800����Ӧ���ַ������ú��������ַ��䳬ʱ�жϽ����� ��������Ҫ����AT����ͺ�����
*	��    ��: _pBuf : ���ģ�鷵�ص������ַ���
*			  _usBufSize : ��������󳤶�
*			 _usTimeOut : ����ִ�г�ʱ��0��ʾһֱ�ȴ�. >0 ��ʾ��ʱʱ�䣬��λ1ms
*	�� �� ֵ: 0 ��ʾ���󣨳�ʱ��  > 0 ��ʾӦ������ݳ���
*********************************************************************************************************
*/
uint16_t SIM800_ReadResponse(char *_pBuf, uint16_t _usBufSize, uint16_t _usTimeOut)
{
	uint8_t ucData;
	uint16_t pos = 0;
	uint8_t ret;
	uint8_t status = 0;		/* ����״̬ */

	GPRS_msTimeOut = GetSystemTimer();	//��0���¼�ʱ
	
	while (1)
	{
		if (status == 2)		/* ���ڽ�����ЧӦ��׶Σ�ͨ���ַ��䳬ʱ�ж����ݽ������ */
		{
			if ((GetSystemTimer()-GPRS_msTimeOut) > 5)	//5ms
			{
				_pBuf[pos]	 = 0;	/* ��β��0�� ���ں���������ʶ���ַ������� */
				ret = pos;		/* �ɹ��� �������ݳ��� */
				break;
			}
		}
		else
		{
			if (_usTimeOut > 0)
			{
				if ((GetSystemTimer()-GPRS_msTimeOut) > _usTimeOut)
				{
					ret = 0;	/* ��ʱ */
					break;
				}
			}
		}
		
		if (UARTGet(COM_SIM800, &ucData, 0))
		{			
			SIM800_PrintRxData(ucData);		/* �����յ����ݴ�ӡ�����Դ���1 */

			switch (status)
			{
				case 0:			/* ���ַ� */
					if (ucData == AT_CR)		/* ������ַ��ǻس�����ʾ AT������� */
					{
						_pBuf[pos++] = ucData;		/* ������յ������� */
						status = 2;	 /* ��Ϊ�յ�ģ��Ӧ���� */
					}
					else	/* ���ַ��� A ��ʾ AT������� */
					{
						status = 1;	 /* �����������͵�AT�����ַ�����������Ӧ�����ݣ�ֱ������ CR�ַ� */
					}
					break;
					
				case 1:			/* AT������Խ׶�, ����������. �����ȴ� */
					if (ucData == AT_CR)
					{
						status = 2;
					}
					break;
					
				case 2:			/* ��ʼ����ģ��Ӧ���� */
					/* ֻҪ�յ�ģ���Ӧ���ַ���������ַ��䳬ʱ�жϽ�������ʱ�����ܳ�ʱ�������� */
					GPRS_msTimeOut = GetSystemTimer();	//��0���¼�ʱ
					if (pos < _usBufSize - 1)
					{
						_pBuf[pos++] = ucData;		/* ������յ������� */
					}
					break;
			}
		}
	}
	return ret;
}

/*
*********************************************************************************************************
*	�� �� ��: SIM800_SendAT
*	����˵��: ��GSMģ�鷢��AT��� �������Զ���AT�ַ���������<CR>�ַ�
*	��    ��: _Str : AT�����ַ�����������ĩβ�Ļس�<CR>. ���ַ�0����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SIM800_SendAT(char *_Cmd)
{
	UARTSendStr(COM_SIM800, (uint8_t *)_Cmd, strlen(_Cmd));
	UARTSendStr(COM_SIM800, "\r", 1);
}

/*
*********************************************************************************************************
*	�� �� ��: SIM800_SendAT
*	����˵��: ��GSMģ�鷢��AT��� �������Զ���AT�ַ���������CTRL+Z
*	��    ��: _Str : AT�����ַ�����������ĩβ�Ļس�CTRL+Z. ���ַ�0����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SIM800_SendAT2(char *_Cmd)
{
	UARTSendStr(COM_SIM800, (uint8_t *)_Cmd, strlen(_Cmd));
	
	UARTSend(COM_SIM800, 26);
}

/*
*********************************************************************************************************
*	�� �� ��: SIM800_HardInfo
*	����˵��: ��ʾSIM800ģ���Ӳ����Ϣ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SIM800_HardInfo(void)
{
	SIM800_SendAT("ATI");
}



/*
*********************************************************************************************************
*	�� �� ��: SIM800_SetEarVolume
*	����˵��: ���ö�������
*	��    ��: _ucVolume : ������ 0 ��ʾ������0-9 ��ʾ������С��5���4��ȱʡ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SIM800_SetEarVolume(uint8_t _ucVolume)
{
	char CmdBuf[32];

	sprintf(CmdBuf, "AT+CLVL=%d", _ucVolume);
	SIM800_SendAT(CmdBuf);
}

/*
*********************************************************************************************************
*	�� �� ��: SIM800_SetMicGain
*	����˵��: ����MIC ���� .    ���ú����·ͨ���������ã���ֻ�����м���绰ǰʹ�á�
*	��    ��: _iGain : ���档 -12 ��С��12���13��ʾ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SIM800_SetMicGain(uint16_t _Channel, uint16_t _iGain)
{
	char CmdBuf[32];

	sprintf(CmdBuf, "AT+CMIC=%d,%d", _Channel, _iGain);
	SIM800_SendAT(CmdBuf);
}


/*
*********************************************************************************************************
*	�� �� ��: SIM800_DialTel
*	����˵��: ����绰
*	��    ��: _pTel �绰�����ַ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SIM800_DialTel(char *_pTel)
{
	char CmdBuf[64];

	sprintf(CmdBuf, "ATD%s;", _pTel);
	SIM800_SendAT(CmdBuf);
}

/*
*********************************************************************************************************
*	�� �� ��: SIM800_Hangup
*	����˵��: �Ҷϵ绰
*	��    ��: _pTel �绰�����ַ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SIM800_Hangup(void)
{
	SIM800_SendAT("ATH");
}

/*
*********************************************************************************************************
*	�� �� ��: SIM800_GetHardInfo
*	����˵��: ��ȡģ���Ӳ����Ϣ. ���� ATI ����Ӧ������
*	��    ��: ��Ž���Ľṹ��ָ��
*	�� �� ֵ: 1 ��ʾ�ɹ��� 0 ��ʾʧ��
*********************************************************************************************************
*/
uint8_t SIM800_GetHardInfo(MG_HARD_INFO_T *_pInfo)
{
	/*
		ATI
		Manufacture: SIMCOM
		Model: SIM800
		Revision: 12.210.10.05.00
		IMEI: 351869045435933
		+GCAP: +CGSM
		
		OK	
	*/	
	char buf[255];
	uint16_t len, i, begin, num;
	uint8_t status = 0;	
	
	/* ���㴮�ڽ��ջ����� */	
	if (Bios_PortRead(COM_SIM800, NULL, 0, 0, NULL))
		Bios_PortRead(COM_SIM800, NULL, -1, 0, NULL);
	
	SIM800_SendAT("ATI");		/* ���� ATI ���� */
	
	len = SIM800_ReadResponse(buf, sizeof(buf), 300);	/* ��ʱ 300ms */
	if (len == 0)
	{
		return 0;		
	}
	
	_pInfo->Manfacture[0] = 0;
	_pInfo->Model[0] = 0;
	_pInfo->Revision[0] = 0;
	_pInfo->IMEI[0] = 0;
	
	for (i = 2; i < len; i++)
	{
		if (buf[i] == ':')
		{
			i += 2;
			begin = i;
		}
		if (buf[i] == AT_CR)
		{
			num = i - begin;
			
			if (status == 0)
			{
				if (num >= sizeof(_pInfo->Manfacture))
				{
					num = sizeof(_pInfo->Manfacture) - 1;
				}
				memcpy(_pInfo->Manfacture, &buf[begin], num);
				_pInfo->Manfacture[num] = 0;
			}
			else if (status == 1)
			{
				if (num >= sizeof(_pInfo->Model))
				{
					num = sizeof(_pInfo->Model) - 1;
				}				
				memcpy(_pInfo->Model, &buf[begin], num);
				_pInfo->Model[num] = 0;
			}
			else if (status == 2)
			{
				if (num >= sizeof(_pInfo->Revision))
				{
					num = sizeof(_pInfo->Revision) - 1;
				}				
				memcpy(_pInfo->Revision, &buf[begin], num);
				_pInfo->Revision[num] = 0;
			}
			else if (status == 3)
			{
				if (num >= sizeof(_pInfo->IMEI))
				{
					num = sizeof(_pInfo->IMEI) - 1;
				}					
				memcpy(_pInfo->IMEI, &buf[begin], num);
				_pInfo->IMEI[num] = 0;
				break;
			}
			status++;	
		}
	}
	return 1;
}

/*
*********************************************************************************************************
*	�� �� ��: MG323_GetNetStatus
*	����˵��: ��ѯ��ǰ����״̬
*	��    ��: ��
*	�� �� ֵ: ����״̬, CREG_NO_REG, CREG_LOCAL_OK �ȡ�
*********************************************************************************************************
*/
uint8_t SIM800_GetNetStatus(void)
{
	/*
		AT+CREG?
		+CREG: 0,1
		
		OK				
	*/	
	char buf[128];
	uint16_t len, i;
	uint8_t status;
	
	/* ���㴮�ڽ��ջ����� */	
	if (Bios_PortRead(COM_SIM800, NULL, 0, 0, NULL))
		Bios_PortRead(COM_SIM800, NULL, -1, 0, NULL);
	
	SIM800_SendAT("AT+CREG?");	/* ���� AT ���� */
	
	len = SIM800_ReadResponse(buf, sizeof(buf), 200);	/* ��ʱ 200ms */
	if (len == 0)
	{
		return 0;		
	}
	
	for (i = 2; i < len; i++)
	{
		if (buf[i] == ',')
		{
			i++;
			
			status = buf[i] - '0';
		}
	}
	return status;
}


/*
*********************************************************************************************************
*	�� �� ��: SIM800_TCPStart
*	����˵��: ����TCP����
*	��    ��: ��
*	�� �� ֵ: ����״̬, CREG_NO_REG, CREG_LOCAL_OK �ȡ�
*********************************************************************************************************
*/
void SIM800_TCPStart(char *_ip, uint16_t _port)
{
		
	char Cmdbuf[128];
	
	/* ���㴮�ڽ��ջ����� */	
	if (Bios_PortRead(COM_SIM800, NULL, 0, 0, NULL))
		Bios_PortRead(COM_SIM800, NULL, -1, 0, NULL);
	
	sprintf(Cmdbuf, "AT+CIPSTART=\"TCP\", \"%s\", %d", _ip, _port);
	
	SIM800_SendAT(Cmdbuf);	/* ���� AT ���� */
}

/*
*********************************************************************************************************
*	�� �� ��: SIM800_TCPClose
*	����˵��: �ر�TCP����
*	��    ��: ��
*	�� �� ֵ: ����״̬, CREG_NO_REG, CREG_LOCAL_OK �ȡ�
*********************************************************************************************************
*/
void SIM800_TCPClose(void)
{	
	SIM800_SendAT("AT+CIPCLOSE");	/* ���� AT ���� */
}

/*
*********************************************************************************************************
*	�� �� ��: SIM800_TCPSend
*	����˵��: TCP��������
*	��    ��: ��
*	�� �� ֵ: ����״̬, CREG_NO_REG, CREG_LOCAL_OK �ȡ�
*********************************************************************************************************
*/
void SIM800_TCPSend(char *buf)
{	
	uint16_t i, j, m, size; 
	uint8_t CmdBuf[255]; 
	size = strlen(buf);
	if (size < SIM800_TX_SIZE)
	{
		
		SIM800_SendAT("AT+CIPSEND");
		
		msDelay(100);
		
		for(j = 0; j < size; j++)
		{
			
			CmdBuf[j] = *(buf++);
			
		}
		SIM800_SendAT2(CmdBuf);
		
		//msDelay(200);
	}
	else
	{
		if(m % size == 0)
		{
			m = size / SIM800_TX_SIZE;
			
			for(i = 0; i < m; i++)
			{
				SIM800_SendAT("AT+CIPSEND");
				
				msDelay(100);
				
				for(j = 0; j < SIM800_TX_SIZE; j++)
				{
					
					CmdBuf[j] = *(buf++);
					
				}
				SIM800_SendAT2(CmdBuf);
				
				msDelay(500);
				
			}
		}
		else
		{
			m = size / SIM800_TX_SIZE;
			
			for(i = 0; i < m; i++)
			{
				
				SIM800_SendAT("AT+CIPSEND");
				
				msDelay(100);
				
				for(j = 0; j < SIM800_TX_SIZE; j++)
				{
					
					CmdBuf[j] = *(buf++);
					
				}
				
				SIM800_SendAT2(CmdBuf);
				
				msDelay(500);
			}
			SIM800_SendAT("AT+CIPSEND");
			
			msDelay(100);
			
			for(j = 0; j < (size - i * SIM800_TX_SIZE); j++)
			{
				
					CmdBuf[j] = *(buf++);
				
			}
			
			SIM800_SendAT2(CmdBuf);
			
			msDelay(500);
		}
	}
}



/*
*********************************************************************************************************
*	�� �� ��: SIM800_GetLocalIP
*	����˵��: ��ѯ����IP
*	��    ��: ��
*	�� �� ֵ: ����״̬, CREG_NO_REG, CREG_LOCAL_OK �ȡ�
*********************************************************************************************************
*/
void SIM800_GetLocalIP(void)
{	
	SIM800_SendAT("AT+CIFSR");	/* ���� AT ���� */
}

/*
*********************************************************************************************************
*	�� �� ��: SIM800_GetTCPStatus
*	����˵��: ��ѯTCP����״̬
*	��    ��: ��
*	�� �� ֵ: ����״̬, CREG_NO_REG, CREG_LOCAL_OK �ȡ�
*********************************************************************************************************
*/
void SIM800_GetTCPStatus(void)
{	
	SIM800_SendAT("AT+CIPSTATUS");	/* ���� AT ���� */
}





/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
