#ifndef __FTL_H
#define __FTL_H
#include "stm32f2xx.h"


//������������
//�������Ϊ1,����FTL_Format��ʱ��,��Ѱ����,��ʱ��(512M,3��������),�һᵼ��RGB������
#define FTL_USE_BAD_BLOCK_SEARCH		0		//�����Ƿ�ʹ�û�������



u8 FTL_Init(void); 
void FTL_BadBlockMark(u32 blocknum);
u8 FTL_CheckBadBlock(u32 blocknum); 
u8 FTL_UsedBlockMark(u32 blocknum);
u32 FTL_FindUnusedBlock(u32 sblock,u8 flag);
u32 FTL_FindSamePlaneUnusedBlock(u32 sblock);
u8 FTL_CopyAndWriteToBlock(u32 Source_PageNum,u16 ColNum,u8 *pBuffer,u32 NumByteToWrite);
u16 FTL_LBNToPBN(u32 LBNNum); 
u8 FTL_WriteSectors(u8 *pBuffer,u32 SectorNo,u16 SectorSize,u32 SectorCount);
u8 FTL_ReadSectors(u8 *pBuffer,u32 SectorNo,u16 SectorSize,u32 SectorCount);
u8 FTL_CreateLUT(u8 mode);
u8 FTL_BlockCompare(u32 blockx,u32 cmpval);
u32 FTL_SearchBadBlock(void);
u8 FTL_Format(void); 
#endif

