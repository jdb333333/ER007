
#ifndef __TEST_H
#define __TEST_H


#include "include.h"

#define BUFFER_SIZE        4096

extern uint8_t TxBuffer[BUFFER_SIZE], RxBuffer[BUFFER_SIZE];

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SRAM_DEBUG		1
#define	SPIFLASH_DEBUG		0
#define NOR_FLASH_TEST 0
#define TEST_LED 0
#define TEST_SD_FS	1
#define TEST_RLFLASHFS 0
#define TEST_ZNFAT 0
#define TEST_NANDFLASH	0
#define TEST_NANDFLASH_FATFS	0

uint32_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint32_t BufferLength);

void TIM6_Init_Count(void);
uint8_t Test_SRAM(void);
void Test_BKPSRAM(void);
void Test_SPIFlash(void);
void Test_DataFlash(void);
uint8_t Test_COM(uint8_t port);
void Test_SD_FS(uint8_t pdrv);
void Test_PWR(void);
void Test_Ethernet(void);
void Test_OTPFlash_MAC(void);
void Test_CustomerDis(void);
void Test_OPDisplay(void);
void Test_Printer(void);
void Test_LowPowerMode(void);
void Test_Drawer(void);
void Test_Alarm(void);
void Test_Keyboard(void);
void Test_SIM800(void);

void NOR_Flash_Test( void );

void DisplayTime(BYTE t);
void TimeToStrHHMM(char *HHMM);
void DateToStrYYMMDD(char *YYMMDD);

#endif   //__TEST_H


