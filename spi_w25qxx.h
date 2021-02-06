
#ifndef _BSP_SPI_FLASH_H
#define _BSP_SPI_FLASH_H

#include "stm32f2xx.h"


/* 定义串行Flash ID */
enum
{
	SST25VF016B_ID = 0xBF2541,
	MX25L1606E_ID  = 0xC22015,
	W25Q16DV_ID    = 0xEF4015,
	W25Q64BV_ID    = 0xEF4017,
	W25Q128FV_ID   = 0xEF4018
};

typedef struct
{
	uint32_t ChipID;		/* 芯片ID */
	char ChipName[16];		/* 芯片型号字符串，主要用于显示 */
	uint32_t TotalSize;		/* 总容量 */
	uint16_t PageSize;		/* 页面大小 */
}SFLASH_T;

#define SFLASH_PAGESIZE     		0x100		//W25Q16DV page大小256B
#define SFLASH_SECTORSIZE     	0x1000	//W25Q16DV sector大小4KB
#define SFLASH_SECTOR_MAXNUM    0x200		//W25Q16DV sector数量512
#define SFLASH_SIZE							0x200000//W25Q16DV 容量2MB

#define SFLASH_TIMEOUT  				120000000	//超时

void sf_Init(void);
uint32_t sf_ReadID(void);
uint8_t sf_EraseChip(void);
uint8_t sf_EraseSector(uint32_t _uiSectorAddr);
void sf_PageWrite(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize);
uint8_t sf_WriteBuffer(uint8_t* _pBuf, uint32_t _uiWriteAddr, uint16_t _usWriteSize);
void sf_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize);

extern SFLASH_T g_tSF;

//>>>>>>>>>>>>>>>>>>ouhs 20190304 
uint8_t bFlashMBlockErase(uint32_t ucBlock, uint32_t ucNumBlocks);
uint8_t bFlashWrite( uint32_t ulOff, uint8_t ucVal );
uint8_t bFlashProgram( uint32_t ulOff, uint32_t NumBytes, uint8_t *Array );

uint8_t bFlashReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumBytesToRead);
uint8_t FlashReadByte(uint32_t Addr);

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#endif


