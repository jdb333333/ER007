/****************************************Copyright (c)**************************************************
**                               Hunan EUTRON information Equipment Co.,LTD.
**
**                                 http://www.eutron.com
**
**--------------File Info-------------------------------------------------------------------------------
** File Name:            interface.h
** Last modified Date:   2011-12-22
** Last Version:         1.0
** Descriptions:         接口函数
**
**------------------------------------------------------------------------------------------------------
** Created By:           Ruby
** Created date:         2011-12-22
** Version:              1.0
** Descriptions:         First version
**
**------------------------------------------------------------------------------------------------------
** Modified by:         CCRong
** Modified date:       2012-01-12
** Version:
** Descriptions:
**
********************************************************************************************************/

#ifndef __INTERFACE_H
#define __INTERFACE_H

#include "stm32f2xx.h"

#define BEEP_PIN		GPIO_Pin_10
#define BEEP_ON()		GPIOG->BSRRH = BEEP_PIN	  		// 清位
#define BEEP_OFF()		GPIOG->BSRRL = BEEP_PIN	  		// 置位

#define DRAWER_PIN		GPIO_Pin_15
#define DRAWER_PORT		GPIOG
#define MAC_PIN			GPIO_Pin_13
#define MAC_PORT		GPIOC

#define DRAWER_OPEN()  	DRAWER_PORT->BSRRL = DRAWER_PIN	  	// 置位, 开钱箱
#define DRAWER_OFF()	DRAWER_PORT->BSRRH = DRAWER_PIN	  	// 清位

#define UART0_CTS		GPIO_Pin_12			// 对应外部串口RS232-I
#define UART0_RTS		GPIO_Pin_10

#define UART0_RTS_SET()		GPIOF->BSRRL = UART0_RTS
#define UART0_RTS_CLR()		GPIOF->BSRRH = UART0_RTS
#define UART0_CTS_GET()		(GPIOG->IDR & UART0_CTS)


#ifdef CASE_GPRS
//UART2连接的是设备是GPRS模块,RTS和CTS两个I/O做模块的开机关和复位使用.
#define GPRS_PWR 		GPIO_Pin_6
#define GPRS_RST 		GPIO_Pin_7

#define GPRS_PWR_HIGH()		GPIOG->BSRRL = GPRS_PWR
#define GPRS_PWR_LOW()		GPIOG->BSRRH = GPRS_PWR
#define GPRS_RST_HIGH()		GPIOG->BSRRL = GPRS_RST
#define GPRS_RST_LOW()		GPIOG->BSRRH = GPRS_RST
#endif


extern volatile uint16_t beep_len;
extern volatile uint8_t DrawerOpen_time;

extern uint8_t FMDetect(void);

extern void EnableBeep(uint8_t on_off);
extern uint8_t CheckKeyboard(void);

void HW_GPIOInit(void);

void BeepInit(void);
void DrawerInit(void);

void Beep(uint8_t blen);
void DrawerOpen(void);
void SetRTS(BYTE port);
void ClrRTS(BYTE port);
BYTE GetCTS(BYTE port);

extern void KeyBoardInit(void);

#define YEAR 2000

extern UnLong RamOffSet;

void ReadRam(BYTE *buf,WORD len);
void WriteRam(BYTE *buf, WORD len);

void DrawerOpen(void);

WORD Bios(WORD cmd, void  *ptr, WORD par1, WORD par2);
WORD Bios_1(WORD cmd);
WORD Bios_2(WORD cmd, void  *ptr);

WORD Bios_SetKeyboard(BYTE Code, WORD NumKeys, CONSTBYTE *TabPnt, BYTE TabRecLen);
WORD Bios_SetCutter(BYTE Stato);

void SetRTS(BYTE port);
void ClrRTS(BYTE port);
WORD Bios_SetTic(WORD Interval);
WORD Bios_SetPortBusy(BYTE NumCom, BYTE Satato, BYTE ChrRx, BYTE ChrTx);
WORD Bios_TestMAC(void);
#if defined(DEBUGBYPC)
WORD Bios_FM_Write(UnLong DestAdr, void *Src, WORD Num);
WORD Bios_FM_Read(void *Dest, UnLong SrcAdr, WORD Num);
void FM_AllErase(void);
#endif
WORD Bios_FM_BlankCheck(UnLong SrcAdr, WORD Num);
BYTE Bios_FM_CheckPresence(void);

WORD CheckSum(void);

long TestRam(void);

void ClsXRam(void);
short Bell(short len);

WORD CC_Insert(void);

unsigned short Bios_PortWrite(BYTE NumCom, BYTE  *TxData, unsigned short NumBytes, unsigned char Flags);
short Bios_PortRead(BYTE NumCom, void *Dest, short NumBytes,ULONG TimeOut,unsigned  short *Status);

void PutsC( const char *str);
void PutsO( const char *str);
void Puts1( const char *str);

void InitMonitor(int outTo);

//jdb2018-09-02
//void OutPrintEx(WORD Cmd, CONSTBYTE *Line, WORD DotLinesBlank);
void CutRPaper(short lines);
short  RFeed(short line);

BYTE Getch(void);
void TIM6_Init(void);	     // --针对TIM5使用相同的配置，却不能成功？--
void TIM5_Init(void);         // --针对TIM5使用相同的配置，却不能成功？--
void usDelay(__IO uint32_t nTime);
void EXTILine_Init(void);

void USB_Start(void);

extern BYTE MMC_ReadBlock(char *buf,DWORD addr);
extern BYTE MMC_WriteBlock(char *buf,DWORD addr);
extern BYTE MMC_Read2Blocks(char *buf,DWORD addr);
extern BYTE MMC_Write2Blocks(char *buf,DWORD addr);

extern void BKPSRAM_Init(void);
extern void BKPSRAM_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint32_t NumHalfwordToWrite);
extern void BKPSRAM_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumHalfwordToRead);

void RNG_Config(void);
uint32_t GetRandomNumber(void);
#define random(rMAX) GetRandomNumber()%rMAX

#endif

/********************************************************************************************************
** End Of File
*********************************************************************************************************/

