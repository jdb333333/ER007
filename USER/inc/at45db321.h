#ifndef __AT45DB161_H
#define __AT45DB161_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"


/* Select SPI FLASH: ChipSelect pin low  */
#define SPI_FLASH_CS_LOW()     GPIO_ResetBits(GPIOA, GPIO_Pin_4)
/* Deselect SPI FLASH: ChipSelect pin high */
#define SPI_FLASH_CS_HIGH()    GPIO_SetBits(GPIOA, GPIO_Pin_4)

#define BUFFER_1_WRITE 0x84					// 写入第一缓冲区
#define BUFFER_2_WRITE 0x87					// 写入第二缓冲区
#define BUFFER_1_READ 0xD4					// 读取第一缓冲区
#define BUFFER_2_READ 0xD6					// 读取第二缓冲区
#define B1_TO_MM_PAGE_PROG_WITH_ERASE 0x83	// 将第一缓冲区的数据写入主存储器（擦除模式）
#define B2_TO_MM_PAGE_PROG_WITH_ERASE 0x86	// 将第二缓冲区的数据写入主存储器（擦除模式）
	 
#define B1_TO_MM_PAGE_PROG_WITHOUT_ERASE 0x88	// 将第一缓冲区的数据写入主存储器
#define B2_TO_MM_PAGE_PROG_WITHOUT_ERASE 0x89	// 将第二缓冲区的数据写入主存储器
	 
#define MM_PAGE_TO_B1_XFER 0x53				// 将主存储器的指定页数据加载到第一缓冲区
#define MM_PAGE_TO_B2_XFER 0x55				// 将主存储器的指定页数据加载到第二缓冲区
#define PAGE_ERASE 0x81						// 页删除（每页512/528字节）
#define SECTOR_ERASE 0x7C					// 扇区擦除（每扇区128K字节）
#define READ_STATE_REGISTER 0xD7			// 读取状态寄存器
#define READ_SEQUENCE 0xE8

extern unsigned char DF_buffer[528];

void SPI_FLASH_Init(void); 
u32 SPI_FLASH_ReadID(void);
u8 SPI_FLASH_SendByte(u8 byte);
u8 SPI_FLASH_ReadByte(u8 Dummy_Byte);
uint8_t SPI_FLASH_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
uint8_t SPI_FLASH_BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void SPI_FLASH_StartReadSequence(u32 ReadAddr);
u16 SPI_FLASH_SendHalfWord(u16 HalfWord);

u32 SPI_FLASH_ChipErase(void);

void write_SPI				(unsigned char data);
void DF_format				(void);
void DF_page_earse			(unsigned int page);

void DF_write_page			(u8* pBuffer,u16 page);
void DF_read_page			(u8* pBuffer,u16 page);
void DF_mm_read_page		(unsigned int page);
unsigned char DF_read_reg	(void);
void DF_wait_busy			(void);
void DF_mm_to_buf			(unsigned char buffer,unsigned int page);

u8 DF_read_buf(u8 buffer,u16 start_address,u16 length,u8* pBuffer);
u8 DF_write_buf(u8 buffer,u16 start_address,u16 length,u8* pBuffer);

void DF_buf_to_mm			(unsigned char buffer,unsigned int page);



#ifdef __cplusplus
}
#endif

#endif 



