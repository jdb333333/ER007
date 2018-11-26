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

#define BUFFER_1_WRITE 0x84					// д���һ������
#define BUFFER_2_WRITE 0x87					// д��ڶ�������
#define BUFFER_1_READ 0xD4					// ��ȡ��һ������
#define BUFFER_2_READ 0xD6					// ��ȡ�ڶ�������
#define B1_TO_MM_PAGE_PROG_WITH_ERASE 0x83	// ����һ������������д�����洢��������ģʽ��
#define B2_TO_MM_PAGE_PROG_WITH_ERASE 0x86	// ���ڶ�������������д�����洢��������ģʽ��
	 
#define B1_TO_MM_PAGE_PROG_WITHOUT_ERASE 0x88	// ����һ������������д�����洢��
#define B2_TO_MM_PAGE_PROG_WITHOUT_ERASE 0x89	// ���ڶ�������������д�����洢��
	 
#define MM_PAGE_TO_B1_XFER 0x53				// �����洢����ָ��ҳ���ݼ��ص���һ������
#define MM_PAGE_TO_B2_XFER 0x55				// �����洢����ָ��ҳ���ݼ��ص��ڶ�������
#define PAGE_ERASE 0x81						// ҳɾ����ÿҳ512/528�ֽڣ�
#define SECTOR_ERASE 0x7C					// ����������ÿ����128K�ֽڣ�
#define READ_STATE_REGISTER 0xD7			// ��ȡ״̬�Ĵ���
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



