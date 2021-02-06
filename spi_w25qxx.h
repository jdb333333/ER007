
#ifndef _BSP_SPI_FLASH_H
#define _BSP_SPI_FLASH_H

#include "stm32f2xx.h"


/* ���崮��Flash ID */
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
	uint32_t ChipID;		/* оƬID */
	char ChipName[16];		/* оƬ�ͺ��ַ�������Ҫ������ʾ */
	uint32_t TotalSize;		/* ������ */
	uint16_t PageSize;		/* ҳ���С */
}SFLASH_T;

#define SFLASH_PAGESIZE     		0x100		//W25Q16DV page��С256B
#define SFLASH_SECTORSIZE     	0x1000	//W25Q16DV sector��С4KB
#define SFLASH_SECTOR_MAXNUM    0x200		//W25Q16DV sector����512
#define SFLASH_SIZE							0x200000//W25Q16DV ����2MB

#define SFLASH_TIMEOUT  				120000000	//��ʱ

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


