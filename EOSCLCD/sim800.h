
#ifndef __BSP_SIM800_H
#define __BSP_SIM800_H

#include <stm32f2xx.h>

#define COM_SIM800	LOGIC_COM3		/* ѡ�񴮿� */
#define SIM800_TX_SIZE 1460

/* ����������仰, �����յ����ַ����͵����Դ���1 */
#define SIM800_TO_COM1_EN

/* ��ģ�鲿�ֺ����õ��������ʱ�����1��ID�� �����������ñ�ģ��ĺ���ʱ����ע��رܶ�ʱ�� TMR_COUNT - 1��
  bsp_StartTimer(3, _usTimeOut);
  
  TMR_COUNT �� bsp_timer.h �ļ�����
*/
//#define SIM800_TMR_ID	(TMR_COUNT - 1)

#define AT_CR		'\r'
#define AT_LF		'\n'

/* MIC���淶Χ */
#define	MIC_GAIN_MIN		-12
#define	MIC_GAIN_MAX		12
#define	MIC_GAIN_MUTE		13
#define	MIC_GAIN_DEFAULT	0

/* ����������Χ */
#define	EAR_VOL_MIN			0
#define	EAR_VOL_MAX			100
#define	EAR_VOL_DEFAULT		50
#define	MAIN_AUDIO_CHANNEL	0

/* AT+CREG? ����Ӧ���е�����״̬���� 	��ǰ����ע��״̬  SIM800_GetNetStatus() */
enum
{
	CREG_NO_REG = 0,  	/* 0��û��ע�ᣬME���ڲ�û������ѰҪע����µ���Ӫ�� */
	CREG_LOCAL_OK = 1,	/* 1��ע���˱������� */
	CREG_SEARCH = 2,	/* 2��û��ע�ᣬ��MS������ѰҪע����µ���Ӫ�� */
	CREG_REJECT = 3,	/* 3��ע�ᱻ�ܾ� */
	CREG_UNKNOW = 4,	/* 4��δ֪ԭ�� */
	CREG_REMOTE_OK = 5, /* 5��ע������������ */
};

/* ͨ�� ATI ָ����Բ�ѯģ���Ӳ����Ϣ 
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
	char Manfacture[12];	/* ���� */
	char Model[12];			/* �ͺ� */
	char Revision[15 + 1];	/* �̼��汾 */
	char IMEI[15 + 1];		/* IMEI �� */
}MG_HARD_INFO_T;

/* ���ⲿ���õĺ������� */
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


