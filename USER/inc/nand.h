#ifndef _NAND_H
#define _NAND_H
#include "fsmc_sram.h"

 
#define NAND_MAX_PAGE_SIZE			4096		//����NAND FLASH������PAGE��С��������SPARE������Ĭ��4096�ֽ�
#define NAND_ECC_SECTOR_SIZE		512			//ִ��ECC����ĵ�Ԫ��С��Ĭ��512�ֽ�

//NAND���Խṹ��
typedef struct
{
    u16 page_totalsize;     	//ÿҳ�ܴ�С��main����spare���ܺ�
    u16 page_mainsize;      	//ÿҳ��main����С
    u16 page_sparesize;     	//ÿҳ��spare����С
    u8  block_pagenum;      	//ÿ���������ҳ����
    u16 plane_blocknum;     	//ÿ��plane�����Ŀ�����
    u16 block_totalnum;     	//�ܵĿ�����
    u16 good_blocknum;      	//�ÿ�����    
    u16 valid_blocknum;     	//��Ч������(���ļ�ϵͳʹ�õĺÿ�����)
    u32 id;             		//NAND FLASH ID
    u16 *lut;      			   	//LUT�������߼���-�����ת��
	u32 ecc_hard;				//Ӳ�����������ECCֵ
	u32 ecc_hdbuf[NAND_MAX_PAGE_SIZE/NAND_ECC_SECTOR_SIZE];//ECCӲ������ֵ������  	
	u32 ecc_rdbuf[NAND_MAX_PAGE_SIZE/NAND_ECC_SECTOR_SIZE];//ECC��ȡ��ֵ������
}nand_attriute;      

extern nand_attriute nand_dev;				//nand��Ҫ�����ṹ�� 

#define NAND_ADDRESS		Bank_NAND_ADDR
#define NAND_CMD				1<<16		//��������
#define NAND_ADDR				1<<17		//���͵�ַ

//NAND FLASH����
#define NAND_READID         	0X90    	//��IDָ��
#define NAND_FEATURE			0XEF    	//��������ָ��
#define NAND_RESET          	0XFF    	//��λNAND
#define NAND_READSTA        	0X70   	 	//��״̬
#define NAND_AREA_A         	0X00   
#define NAND_AREA_TRUE1     	0X30  
#define NAND_WRITE0        	 	0X80
#define NAND_WRITE_TURE1    	0X10
#define NAND_ERASE0        	 	0X60
#define NAND_ERASE1         	0XD0
#define NAND_MOVEDATA_CMD0  	0X00
#define NAND_MOVEDATA_CMD1  	0X35
#define NAND_MOVEDATA_CMD2  	0X85
#define NAND_MOVEDATA_CMD3  	0X10

//NAND FLASH״̬
#define NSTA_READY       	   	0X40		//nand�Ѿ�׼����
#define NSTA_ERROR				0X01		//nand����
#define NSTA_TIMEOUT        	0X02		//��ʱ
#define NSTA_ECC1BITERR       	0X03		//ECC 1bit����
#define NSTA_ECC2BITERR       	0X04		//ECC 2bit���ϴ���


//NAND FLASH�ͺźͶ�Ӧ��ID��
#define MT29F4G08ABADA			0XDC909556	//MT29F4G08ABADA
#define MT29F16G08ABABA			0X48002689	//MT29F16G08ABABA

#define HY27UF081G2A	0xADF1801D
#define K9F1G08U0A		0xECF18015
#define K9F1G08U0B		0xECF10095
#define K9F1G08U0D		0xECF10095
#define H27U1G8F2BTR	0xADF1001D
#define H27U4G8F2DTR	0xADDC9095
#define MX30LF1G18AC  0xC2F18095
#define JS27HU1G08SC	0xADF1801D

#define NAND_UNKNOW		0xFFFFFFFF
 
#define NAND_TYPE	JS27HU1G08SC 
 
/* FSMC NAND memory parameters */
/* ����HY27UF081G2A    K9F1G08 */
#if 1//(NAND_TYPE == K9F1G08U0D) || (NAND_TYPE == MX30LF1G18AC) || (NAND_TYPE == JS27HU1G08SC)
	#define NAND_PAGE_SIZE             ((uint16_t)0x0800) /* 2 * 1024 bytes per page w/o Spare Area */
	#define NAND_BLOCK_SIZE            ((uint16_t)0x0040) /* 64 pages per block */
	#define NAND_ZONE_SIZE             ((uint16_t)0x0400) /* 1024 Block per zone */
	#define NAND_SPARE_AREA_SIZE       ((uint16_t)0x0040) /* last 64 bytes as spare area */
	#define NAND_MAX_ZONE              ((uint16_t)0x0001) /* 1 zones of 1024 block */
	#define NAND_ADDR_5					0			/* 0��ʾֻ�÷���4���ֽڵĵ�ַ��1��ʾ5�� */
	
	/* ������붨�� */
	#define NAND_CMD_COPYBACK_A			((uint8_t)0x00)		/* PAGE COPY-BACK �������� */
	#define NAND_CMD_COPYBACK_B			((uint8_t)0x35)
	#define NAND_CMD_COPYBACK_C			((uint8_t)0x85)
	#define NAND_CMD_COPYBACK_D			((uint8_t)0x10)

	#define NAND_CMD_STATUS				((uint8_t)0x70)		/* ��NAND Flash��״̬�� */

	#define MAX_PHY_BLOCKS_PER_ZONE  1024	/* ÿ������������� */
	#define MAX_LOG_BLOCKS_PER_ZONE  1000	/* ÿ��������߼���� */

	#define NAND_BLOCK_COUNT			1024 /* ����� */
	#define NAND_PAGE_TOTAL_SIZE		(NAND_PAGE_SIZE + NAND_SPARE_AREA_SIZE)	/* ҳ���ܴ�С */


#elif NAND_TYPE == H27U4G8F2DTR
	#define NAND_PAGE_SIZE             ((uint16_t)0x0800) /* 2 * 1024 bytes per page w/o Spare Area */
	#define NAND_BLOCK_SIZE            ((uint16_t)0x0040) /* 64 pages per block */
	#define NAND_ZONE_SIZE             ((uint16_t)0x1000) /* 4096 Block per zone */
	#define NAND_SPARE_AREA_SIZE       ((uint16_t)0x0040) /* last 64 bytes as spare area */
	#define NAND_MAX_ZONE              ((uint16_t)0x0001) /* 1 zones of 4096 block */
	#define NAND_ADDR_5					1			/* 1��ʾֻ����4���ֽڵĵ�ַ��1��ʾ5�� */

	/* ������붨�� */
	#define NAND_CMD_COPYBACK_A			((uint8_t)0x00)		/* PAGE COPY-BACK �������� */
	#define NAND_CMD_COPYBACK_B			((uint8_t)0x35)
	#define NAND_CMD_COPYBACK_C			((uint8_t)0x85)
	#define NAND_CMD_COPYBACK_D			((uint8_t)0x10)

//	#define NAND_CMD_STATUS				((uint8_t)0x70)		/* ��NAND Flash��״̬�� */

	#define MAX_PHY_BLOCKS_PER_ZONE     4096	/* ÿ������������� */
	#define MAX_LOG_BLOCKS_PER_ZONE     4000	/* ÿ��������߼���� */

	#define NAND_BLOCK_COUNT			4096 /* ����� */
	#define NAND_PAGE_TOTAL_SIZE		(NAND_PAGE_SIZE + NAND_SPARE_AREA_SIZE)	/* ҳ���ܴ�С */

#else
	#define NAND_PAGE_SIZE             ((uint16_t)0x0200) /* 512 bytes per page w/o Spare Area */
	#define NAND_BLOCK_SIZE            ((uint16_t)0x0020) /* 32x512 bytes pages per block */
	#define NAND_ZONE_SIZE             ((uint16_t)0x0400) /* 1024 Block per zone */
	#define NAND_SPARE_AREA_SIZE       ((uint16_t)0x0010) /* last 16 bytes as spare area */
	#define NAND_MAX_ZONE              ((uint16_t)0x0004) /* 4 zones of 1024 block */
#endif 
 

u8 NAND_Init(void);
u8 NAND_ModeSet(u8 mode);
u32 NAND_ReadID(void);
u8 NAND_ReadStatus(void);
u8 NAND_WaitForReady(void);
u8 NAND_Reset(void);
u8 NAND_WaitRB(vu8 rb);
void NAND_Delay(vu32 i);
u8 NAND_ReadPage(u32 PageNum,u16 ColNum,u8 *pBuffer,u16 NumByteToRead);
u8 NAND_ReadPageComp(u32 PageNum,u16 ColNum,u32 CmpVal,u16 NumByteToRead,u16 *NumByteEqual);
u8 NAND_WritePage(u32 PageNum,u16 ColNum,u8 *pBuffer,u16 NumByteToWrite);
u8 NAND_WritePageConst(u32 PageNum,u16 ColNum,u32 cval,u16 NumByteToWrite);
u8 NAND_CopyPageWithoutWrite(u32 Source_PageNum,u32 Dest_PageNum);
u8 NAND_CopyPageWithWrite(u32 Source_PageNum,u32 Dest_PageNum,u16 ColNum,u8 *pBuffer,u16 NumByteToWrite);
u8 NAND_ReadSpare(u32 PageNum,u16 ColNum,u8 *pBuffer,u16 NumByteToRead);
u8 NAND_WriteSpare(u32 PageNum,u16 ColNum,u8 *pBuffer,u16 NumByteToRead);
u8 NAND_EraseBlock(u32 BlockNum);
void NAND_EraseChip(void);

u16 NAND_ECC_Get_OE(u8 oe,u32 eccval);
u8 NAND_ECC_Correction(u8* data_buf,u32 eccrd,u32 ecccl);
#endif



















