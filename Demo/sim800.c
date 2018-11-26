/*
*********************************************************************************************************
*
*	模块名称 : SIM800驱动程序
*	文件名称 : bsp_SIM800.c
*	版    本 : V1.0
*	说    明 : 封装SIM800模块相关的AT命令
*********************************************************************************************************
*/
#include <stdio.h>
#include <string.h>

#include "sim800.h"
#include "usart.h"
#include "SysTick.h"

/*
	SIM800_TERM_ON   ： PG6
	SIM800_RESET   	 ： PG7
*/

/*
	AT+CIPSTATUS 查询当前TCP连接状态
	
	AT+CIPSEND TCP发送数据
	
	AT+CIPSTART 建立TCP连接
	
	AT+CIPCLOSE 关闭TCP连接
	
	AT+CIFSR 查询模块IP

	AT+CREG?  查询当前网络状态

	AT+CSQ 查询信号质量命令

	AT+CIMI 查询SIM 卡的IMSI 号。
	
	ATI 查询模块的硬件信息
	
	AT+CIND? 读取当前的指示状态

	ATA 接听命令
	ATH 挂断连接命令
	
	ATI 显示SIM800模块的硬件信息
	
	ATD10086; 拨打10086

	AT^SWSPATH=<n>  切换音频通道
*/


/* STM32和GSM的TERM_ON引脚间有1个NPN三极管，因此需要反相 */
#define TERM_ON_1()		GPIOG->BSRRH =  GPIO_Pin_6//GPRS_PWR_LOW()
#define TERM_ON_0()		GPIOG->BSRRL =  GPIO_Pin_6//GPRS_PWR_HIGH()

/* STM32和MG323的RESET引脚间有1个NPN三极管，因此需要反相. MG323的RESET脚是低脉冲复位 */
//#define MG_RESET_1()	GPRS_RST_LOW()
//#define MG_RESET_0()	GPRS_RST_HIGH()

uint16_t GPRS_msTimeOut;
//char CmdBuf[24 * 1024];
/*
*********************************************************************************************************
*	函 数 名: InitSIM800
*	功能说明: 配置无线模块相关的GPIO
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void InitSIM800(void)
{
//开机信号TERM_ON和复位信号RESET设置在interface.c文件里的HW_GPIOInit()实现
//串口参数初始化在usart.c文件里实现
}


/*
*********************************************************************************************************
*	函 数 名: SIM800_PrintRxData
*	功能说明: 打印STM32从SIM800收到的数据到COM1串口，主要用于跟踪调试
*	形    参: _ch : 收到的数据
*	返 回 值: 无
*********************************************************************************************************
*/
void SIM800_PrintRxData(uint8_t _ch)
{
#ifdef SIM800_TO_COM1_EN
	UARTSend(PORT_DEBUG, _ch);		/* 将接收到数据打印到调试串口1 */
#endif
}



/*
*********************************************************************************************************
*	函 数 名: MG323_PowerOn
*	功能说明: 给MG323模块上电
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void SIM800_PowerOn(void)
{
	/* 清零串口接收缓冲区 */
	if (Bios_PortRead(COM_SIM800, NULL, 0, 0, NULL))
		Bios_PortRead(COM_SIM800, NULL, -1, 0, NULL);	

	TERM_ON_0();
	GPRS_msTimeOut = GetSystemTimer();
	while((GetSystemTimer()-GPRS_msTimeOut)<1200);	//开机信号1.2S
	TERM_ON_1();
	
	/* 等待模块完成上电，判断是否接收到 ^SYSSTART */
	//SIM800_WaitResponse("RDY", 5000);
}


/*
*********************************************************************************************************
*	函 数 名: SIM800_PowerOff
*	功能说明: 控制SIM800模块关机
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void SIM800_PowerOff(void)
{
	/* 硬件关机 */
	

	/* 也可以软件关机 */
	SIM800_SendAT("AT+CPOWD=1");
}
/*
*********************************************************************************************************
*	函 数 名: MG323_PowerOn
*	功能说明: 给MG323模块上电
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void SIM800_Reset(void)
{
	/*
		根据MG323手册，
		RESET 管脚用于实现模块硬件复位。当模块出现软件死机的情况时，通过拉低
		RESET 管脚 ≥ 10 ms 后，模块进行硬件复位。
	*/

}
/*
*********************************************************************************************************
*	函 数 名: SIM800_WaitResponse
*	功能说明: 等待SIM800返回指定的应答字符串. 比如等待 OK
*	形    参: _pAckStr : 应答的字符串， 长度不得超过255
*			 _usTimeOut : 命令执行超时，0表示一直等待. >０表示超时时间，单位1ms
*	返 回 值: 1 表示成功  0 表示失败
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
	
	GPRS_msTimeOut = GetSystemTimer();	//清0重新计时
	
	while (1)
	{

		if (_usTimeOut > 0)
		{
			if ((GetSystemTimer()-GPRS_msTimeOut) > _usTimeOut)
			{
				ret = 0;	/* 超时 */
				break;
			}
		}

		if (UARTGet(COM_SIM800, &ucData, 0))
		{
			SIM800_PrintRxData(ucData);		/* 将接收到数据打印到调试串口1 */

			if (ucData == '\n')
			{
				if (pos > 0)	/* 第2次收到回车换行 */
				{
					if (memcmp(ucRxBuf, _pAckStr,  len) == 0)
					{
						ret = 1;	/* 收到指定的应答数据，返回成功 */
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
					/* 只保存可见字符 */
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
*	函 数 名: SIM800_ReadResponse
*	功能说明: 读取SIM800返回应答字符串。该函数根据字符间超时判断结束。 本函数需要紧跟AT命令发送函数。
*	形    参: _pBuf : 存放模块返回的完整字符串
*			  _usBufSize : 缓冲区最大长度
*			 _usTimeOut : 命令执行超时，0表示一直等待. >0 表示超时时间，单位1ms
*	返 回 值: 0 表示错误（超时）  > 0 表示应答的数据长度
*********************************************************************************************************
*/
uint16_t SIM800_ReadResponse(char *_pBuf, uint16_t _usBufSize, uint16_t _usTimeOut)
{
	uint8_t ucData;
	uint16_t pos = 0;
	uint8_t ret;
	uint8_t status = 0;		/* 接收状态 */

	GPRS_msTimeOut = GetSystemTimer();	//清0重新计时
	
	while (1)
	{
		if (status == 2)		/* 正在接收有效应答阶段，通过字符间超时判断数据接收完毕 */
		{
			if ((GetSystemTimer()-GPRS_msTimeOut) > 5)	//5ms
			{
				_pBuf[pos]	 = 0;	/* 结尾加0， 便于函数调用者识别字符串结束 */
				ret = pos;		/* 成功。 返回数据长度 */
				break;
			}
		}
		else
		{
			if (_usTimeOut > 0)
			{
				if ((GetSystemTimer()-GPRS_msTimeOut) > _usTimeOut)
				{
					ret = 0;	/* 超时 */
					break;
				}
			}
		}
		
		if (UARTGet(COM_SIM800, &ucData, 0))
		{			
			SIM800_PrintRxData(ucData);		/* 将接收到数据打印到调试串口1 */

			switch (status)
			{
				case 0:			/* 首字符 */
					if (ucData == AT_CR)		/* 如果首字符是回车，表示 AT命令不会显 */
					{
						_pBuf[pos++] = ucData;		/* 保存接收到的数据 */
						status = 2;	 /* 认为收到模块应答结果 */
					}
					else	/* 首字符是 A 表示 AT命令回显 */
					{
						status = 1;	 /* 这是主机发送的AT命令字符串，不保存应答数据，直到遇到 CR字符 */
					}
					break;
					
				case 1:			/* AT命令回显阶段, 不保存数据. 继续等待 */
					if (ucData == AT_CR)
					{
						status = 2;
					}
					break;
					
				case 2:			/* 开始接收模块应答结果 */
					/* 只要收到模块的应答字符，则采用字符间超时判断结束，此时命令总超时不起作用 */
					GPRS_msTimeOut = GetSystemTimer();	//清0重新计时
					if (pos < _usBufSize - 1)
					{
						_pBuf[pos++] = ucData;		/* 保存接收到的数据 */
					}
					break;
			}
		}
	}
	return ret;
}

/*
*********************************************************************************************************
*	函 数 名: SIM800_SendAT
*	功能说明: 向GSM模块发送AT命令。 本函数自动在AT字符串口增加<CR>字符
*	形    参: _Str : AT命令字符串，不包括末尾的回车<CR>. 以字符0结束
*	返 回 值: 无
*********************************************************************************************************
*/
void SIM800_SendAT(char *_Cmd)
{
	UARTSendStr(COM_SIM800, (uint8_t *)_Cmd, strlen(_Cmd));
	UARTSendStr(COM_SIM800, "\r", 1);
}

/*
*********************************************************************************************************
*	函 数 名: SIM800_SendAT
*	功能说明: 向GSM模块发送AT命令。 本函数自动在AT字符串口增加CTRL+Z
*	形    参: _Str : AT命令字符串，不包括末尾的回车CTRL+Z. 以字符0结束
*	返 回 值: 无
*********************************************************************************************************
*/
void SIM800_SendAT2(char *_Cmd)
{
	UARTSendStr(COM_SIM800, (uint8_t *)_Cmd, strlen(_Cmd));
	
	UARTSend(COM_SIM800, 26);
}

/*
*********************************************************************************************************
*	函 数 名: SIM800_HardInfo
*	功能说明: 显示SIM800模块的硬件信息
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void SIM800_HardInfo(void)
{
	SIM800_SendAT("ATI");
}



/*
*********************************************************************************************************
*	函 数 名: SIM800_SetEarVolume
*	功能说明: 设置耳机音量
*	形    参: _ucVolume : 音量。 0 表示静音，0-9 表示音量大小。5最大。4是缺省。
*	返 回 值: 无
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
*	函 数 名: SIM800_SetMicGain
*	功能说明: 设置MIC 增益 .    设置后对两路通道都起作用，但只能在有激活电话前使用。
*	形    参: _iGain : 增益。 -12 最小，12最大，13表示静音
*	返 回 值: 无
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
*	函 数 名: SIM800_DialTel
*	功能说明: 拨打电话
*	形    参: _pTel 电话号码字符串
*	返 回 值: 无
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
*	函 数 名: SIM800_Hangup
*	功能说明: 挂断电话
*	形    参: _pTel 电话号码字符串
*	返 回 值: 无
*********************************************************************************************************
*/
void SIM800_Hangup(void)
{
	SIM800_SendAT("ATH");
}

/*
*********************************************************************************************************
*	函 数 名: SIM800_GetHardInfo
*	功能说明: 读取模块的硬件信息. 分析 ATI 命令应答结果。
*	形    参: 存放结果的结构体指针
*	返 回 值: 1 表示成功， 0 表示失败
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
	
	/* 清零串口接收缓冲区 */	
	if (Bios_PortRead(COM_SIM800, NULL, 0, 0, NULL))
		Bios_PortRead(COM_SIM800, NULL, -1, 0, NULL);
	
	SIM800_SendAT("ATI");		/* 发送 ATI 命令 */
	
	len = SIM800_ReadResponse(buf, sizeof(buf), 300);	/* 超时 300ms */
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
*	函 数 名: MG323_GetNetStatus
*	功能说明: 查询当前网络状态
*	形    参: 无
*	返 回 值: 网络状态, CREG_NO_REG, CREG_LOCAL_OK 等。
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
	
	/* 清零串口接收缓冲区 */	
	if (Bios_PortRead(COM_SIM800, NULL, 0, 0, NULL))
		Bios_PortRead(COM_SIM800, NULL, -1, 0, NULL);
	
	SIM800_SendAT("AT+CREG?");	/* 发送 AT 命令 */
	
	len = SIM800_ReadResponse(buf, sizeof(buf), 200);	/* 超时 200ms */
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
*	函 数 名: SIM800_TCPStart
*	功能说明: 建立TCP连接
*	形    参: 无
*	返 回 值: 网络状态, CREG_NO_REG, CREG_LOCAL_OK 等。
*********************************************************************************************************
*/
void SIM800_TCPStart(char *_ip, uint16_t _port)
{
		
	char Cmdbuf[128];
	
	/* 清零串口接收缓冲区 */	
	if (Bios_PortRead(COM_SIM800, NULL, 0, 0, NULL))
		Bios_PortRead(COM_SIM800, NULL, -1, 0, NULL);
	
	sprintf(Cmdbuf, "AT+CIPSTART=\"TCP\", \"%s\", %d", _ip, _port);
	
	SIM800_SendAT(Cmdbuf);	/* 发送 AT 命令 */
}

/*
*********************************************************************************************************
*	函 数 名: SIM800_TCPClose
*	功能说明: 关闭TCP连接
*	形    参: 无
*	返 回 值: 网络状态, CREG_NO_REG, CREG_LOCAL_OK 等。
*********************************************************************************************************
*/
void SIM800_TCPClose(void)
{	
	SIM800_SendAT("AT+CIPCLOSE");	/* 发送 AT 命令 */
}

/*
*********************************************************************************************************
*	函 数 名: SIM800_TCPSend
*	功能说明: TCP发送数据
*	形    参: 无
*	返 回 值: 网络状态, CREG_NO_REG, CREG_LOCAL_OK 等。
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
*	函 数 名: SIM800_GetLocalIP
*	功能说明: 查询本地IP
*	形    参: 无
*	返 回 值: 网络状态, CREG_NO_REG, CREG_LOCAL_OK 等。
*********************************************************************************************************
*/
void SIM800_GetLocalIP(void)
{	
	SIM800_SendAT("AT+CIFSR");	/* 发送 AT 命令 */
}

/*
*********************************************************************************************************
*	函 数 名: SIM800_GetTCPStatus
*	功能说明: 查询TCP连接状态
*	形    参: 无
*	返 回 值: 网络状态, CREG_NO_REG, CREG_LOCAL_OK 等。
*********************************************************************************************************
*/
void SIM800_GetTCPStatus(void)
{	
	SIM800_SendAT("AT+CIPSTATUS");	/* 发送 AT 命令 */
}





/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
