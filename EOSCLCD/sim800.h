
#ifndef __BSP_SIM800_H
#define __BSP_SIM800_H

#include <stm32f2xx.h>

#define COM_SIM800	LOGIC_COM3		/* 选择串口 */
#define SIM800_TX_SIZE 1460

/* 定义下面这句话, 将把收到的字符发送到调试串口1 */
#define SIM800_TO_COM1_EN

/* 本模块部分函数用到了软件定时器最后1个ID。 因此主程序调用本模块的函数时，请注意回避定时器 TMR_COUNT - 1。
  bsp_StartTimer(3, _usTimeOut);
  
  TMR_COUNT 在 bsp_timer.h 文件定义
*/
//#define SIM800_TMR_ID	(TMR_COUNT - 1)

#define AT_CR		'\r'
#define AT_LF		'\n'

/* MIC增益范围 */
#define	MIC_GAIN_MIN		-12
#define	MIC_GAIN_MAX		12
#define	MIC_GAIN_MUTE		13
#define	MIC_GAIN_DEFAULT	0

/* 耳机音量范围 */
#define	EAR_VOL_MIN			0
#define	EAR_VOL_MAX			100
#define	EAR_VOL_DEFAULT		50
#define	MAIN_AUDIO_CHANNEL	0

/* AT+CREG? 命令应答中的网络状态定义 	当前网络注册状态  SIM800_GetNetStatus() */
enum
{
	CREG_NO_REG = 0,  	/* 0：没有注册，ME现在并没有在搜寻要注册的新的运营商 */
	CREG_LOCAL_OK = 1,	/* 1：注册了本地网络 */
	CREG_SEARCH = 2,	/* 2：没有注册，但MS正在搜寻要注册的新的运营商 */
	CREG_REJECT = 3,	/* 3：注册被拒绝 */
	CREG_UNKNOW = 4,	/* 4：未知原因 */
	CREG_REMOTE_OK = 5, /* 5：注册了漫游网络 */
};

/* 通过 ATI 指令，可以查询模块的硬件信息 
ATI
Manufacture: HUAWEI
Model: SIM800
Revision: 12.210.10.05.00
IMEI: 351869045435933
+GCAP: +CGSM

OK
*/
typedef struct
{
	char Manfacture[12];	/* 厂商 */
	char Model[12];			/* 型号 */
	char Revision[15 + 1];	/* 固件版本 */
	char IMEI[15 + 1];		/* IMEI 码 */
}MG_HARD_INFO_T;

/* 供外部调用的函数声明 */
void InitSIM800(void);
void SIM800_Reset(void);
void SIM800_PowerOn(void);
void SIM800_PowerOff(void);
void SIM800_SendAT(char *_Cmd);
void SIM800_SendAT2(char *_Cmd);
void SIM800_SetAutoReport(void);

uint8_t SIM800_WaitResponse(char *_pAckStr, uint16_t _usTimeOut);

void SIM800_SwitchPath(uint8_t ch);
void SIM800_SetMicGain(uint16_t _Channel, uint16_t _iGain);
void SIM800_SetEarVolume(uint8_t _ucVolume);

void SIM800_PrintRxData(uint8_t _ch);

uint16_t SIM800_ReadResponse(char *_pBuf, uint16_t _usBufSize, uint16_t _usTimeOut);

uint8_t SIM800_GetHardInfo(MG_HARD_INFO_T *_pInfo);
uint8_t SIM800_GetNetStatus(void);
void SIM800_Reset(void);

void SIM800_DialTel(char *_pTel);
void SIM800_Hangup(void);
void SIM800_GetLocalIP(void);
void SIM800_TCPSend(char *buf);
void SIM800_TCPClose(void);
void SIM800_TCPStart(char *_ip, uint16_t _port);

#endif


