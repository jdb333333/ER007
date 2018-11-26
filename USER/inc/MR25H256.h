#ifndef __MR25H256_H__
#define __MR25H256_H__

#include "stm32f2xx.h"

#define Protected_Area_None             0
#define Protected_Area_Upper_Quarter    1
#define Protected_Area_Upper_Half       2
#define Protected_Area_All              3

#define MRAM_SIZE			0x20000		//128KB
#define MRAM_MAXADDR 	0x1FFFF


#define MRAM_SET_CS()	GPIO_SetBits(GPIOA, GPIO_Pin_4)
#define MRAM_CLR_CS()	GPIO_ResetBits(GPIOA, GPIO_Pin_4)

#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned int

void MR25H256_Init(void);
uint8_t SPI_MRAM_WR(uint8_t data);
uint8_t MRAM_RDSR(void);
void MRAM_WREN(void);
void MRAM_WRDI(void);
void MRAM_WRSR(uint8_t cmd);
uint8_t MRAM_READ(uint8_t * Recv, uint32_t Addr, uint16_t Len);
uint8_t MRAM_WRITE(uint8_t * Send, uint32_t Addr, uint16_t Len);
void MRAM_SLEEP(void);
void MRAM_WAKE(void);

#endif
