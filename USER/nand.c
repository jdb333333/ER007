#include "fsmc_sram.h"
#include "nand.h"
#include "malloc.h"


nand_attriute nand_dev;             //nand重要参数结构体

//初始化NAND FLASH
u8 NAND_Init(void)
{
	
	FSMC_Init();

	NAND_Reset();       		        //复位NAND
	//delay_ms(100);
	nand_dev.id=NAND_ReadID();	    //读取ID
	//NAND_ModeSet(4);			        //设置为MODE4,高速模式
	//xprintf("NAND ID = 0x%x",nand_dev.id);
	if((nand_dev.id==K9F1G08U0D) || (nand_dev.id==MX30LF1G18AC)|| (nand_dev.id == JS27HU1G08SC))    
	{
		nand_dev.page_totalsize = NAND_PAGE_TOTAL_SIZE;	//nand一个page的总大小（包括spare区）
		nand_dev.page_mainsize = NAND_PAGE_SIZE; 	//nand一个page的有效数据区大小
		nand_dev.page_sparesize = NAND_SPARE_AREA_SIZE;		//nand一个page的spare区大小
		nand_dev.block_pagenum = NAND_BLOCK_SIZE;		//nand一个block所包含的page数目
		nand_dev.plane_blocknum = NAND_ZONE_SIZE;	//nand一个plane所包含的block数目
		nand_dev.block_totalnum = NAND_ZONE_SIZE; 	//nand的总block数目
	}else
		return 1;	//错误，返回
  return 0;
}

//读取NAND FLASH的ID
//返回值:0,成功;
//    其他,失败
/*
u8 NAND_ModeSet(u8 mode)
{   
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_FEATURE;//发送设置特性命令
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=0X01;		//地址为0X01,设置mode
 	*(vu8*)NAND_ADDRESS=mode;					//P1参数,设置mode
	*(vu8*)NAND_ADDRESS=0;
	*(vu8*)NAND_ADDRESS=0;
	*(vu8*)NAND_ADDRESS=0; 
    if(NAND_WaitForReady()==NSTA_READY)return 0;//成功
    else return 1;								//失败
}
*/
//读取NAND FLASH的ID
//不同的NAND略有不同，请根据自己所使用的NAND FALSH数据手册来编写函数
//返回值:NAND FLASH的ID值
u32 NAND_ReadID(void)
{
    u8 deviceid[5]; 
    u32 id;  
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_READID; //发送读取ID命令
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=0X00;
	
		/* 顺序读取NAND Flash的ID */
		id = *(__IO uint32_t *)(NAND_ADDRESS);
		id =  ((id << 24) & 0xFF000000) |
				((id << 8 ) & 0x00FF0000) |
				((id >> 8 ) & 0x0000FF00) |
				((id >> 24) & 0x000000FF) ;
		return id;
}  
//读NAND状态
//返回值:NAND状态值
//bit0:0,成功;1,错误(编程/擦除/READ)
//bit6:0,Busy;1,Ready
u8 NAND_ReadStatus(void)
{
	vu8 data=0; 
	*(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_READSTA;//发送读状态命令
	data++;data++;data++;data++;data++;	//加延时,防止-O2优化,导致的错误.
	data=*(vu8*)NAND_ADDRESS;			//读取状态值
	return data;
}
//等待NAND准备好
//返回值:NSTA_TIMEOUT 等待超时了
//      NSTA_READY    已经准备好
u8 NAND_WaitForReady(void)
{
	u8 status=0;
	vu32 time=0; 
	while(1)						//等待ready
	{
		status=NAND_ReadStatus();	//获取状态值
		if(status&NSTA_READY)break;
		time++;
		if(time>=0X1FFFF)return NSTA_TIMEOUT;//超时
	}  
    return NSTA_READY;//准备好
}  
//复位NAND
//返回值:0,成功;
//    其他,失败
u8 NAND_Reset(void)
{ 
	*(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_RESET;	//复位NAND
	if(NAND_WaitForReady()==NSTA_READY)return 0;//复位成功
	else return 1;								//复位失败
} 
//等待RB信号为某个电平
//rb:0,等待RB==0
//   1,等待RB==1
//返回值:0,成功
//       1,超时
/*
u8 NAND_WaitRB(vu8 rb)
{
    vu16 time=0;  
	while(time<10000)
	{
		time++;
		if(NAND_RB==rb)return 0;
	}
	return 1;
}
*/
//NAND延时
void NAND_Delay(vu32 i)
{
	while(i>0)i--;
}
//读取NAND Flash的指定页指定列的数据(main区和spare区都可以使用此函数)
//PageNum:要读取的页地址,范围:0~(block_pagenum*block_totalnum-1)
//ColNum:要读取的列开始地址(也就是页内地址),范围:0~(page_totalsize-1)
//*pBuffer:指向数据存储区
//NumByteToRead:读取字节数(不能跨页读)
//返回值:0,成功 
//    其他,错误代码
u8 NAND_ReadPage(u32 PageNum,u16 ColNum,u8 *pBuffer,u16 NumByteToRead)
{
    vu16 i=0;
	u8 res=0;
	u8 eccnum=0;		//需要计算的ECC个数，每NAND_ECC_SECTOR_SIZE字节计算一个ecc
	u8 eccstart=0;		//第一个ECC值所属的地址范围
	u8 errsta=0;
	u8 *p;
	
	 *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_AREA_A;
	
	//发送地址
	*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)ColNum;
	*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(ColNum>>8);
	*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)PageNum;
	*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(PageNum>>8);
	
#if NAND_ADDR_5 == 1	
	*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(PageNum>>16);
#endif
	*(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_AREA_TRUE1;
	
	//下面两行代码是等待R/B引脚变为低电平，其实主要起延时作用的，等待NAND操作R/B引脚。因为我们是通过
	//将STM32的NWAIT引脚(NAND的R/B引脚)配置为普通IO，代码中通过读取NWAIT引脚的电平来判断NAND是否准备
	//就绪的。这个也就是模拟的方法，所以在速度很快的时候有可能NAND还没来得及操作R/B引脚来表示NAND的忙
	//闲状态，结果我们就读取了R/B引脚,这个时候肯定会出错的，事实上确实是会出错!大家也可以将下面两行
	//代码换成延时函数,只不过这里我们为了效率所以没有用延时函数。
#if 0	
	res=NAND_WaitRB(0);			//等待RB=0 
    if(res)return NSTA_TIMEOUT;	//超时退出
    //下面2行代码是真正判断NAND是否准备好的
	res=NAND_WaitRB(1);			//等待RB=1 
    if(res)return NSTA_TIMEOUT;	//超时退出
#else		
	 /* 必须等待，否则读出数据异常, 此处应该判断超时 */
	NAND_Delay(30);
	
/*	ouhs 20180321 使用FSMC_NWAIT信号	
	while( GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_6) == 0);
*/
#endif

	if(NumByteToRead%NAND_ECC_SECTOR_SIZE)//不是NAND_ECC_SECTOR_SIZE的整数倍，不进行ECC校验
	{ 
		//读取NAND FLASH中的值
		for(i=0;i<NumByteToRead;i++)
		{
			*(vu8*)pBuffer++ = *(vu8*)NAND_ADDRESS;
		}
	}else
	{
		eccnum=NumByteToRead/NAND_ECC_SECTOR_SIZE;			//得到ecc计算次数
		eccstart=ColNum/NAND_ECC_SECTOR_SIZE;
		p=pBuffer;
		for(res=0;res<eccnum;res++)
		{
			FSMC_NANDECCCmd(FSMC_Bank2_NAND, ENABLE);//FMC_Bank2_3->PCR3|=1<<6;						//使能ECC校验 
			for(i=0;i<NAND_ECC_SECTOR_SIZE;i++)				//读取NAND_ECC_SECTOR_SIZE个数据
			{
				*(vu8*)pBuffer++ = *(vu8*)NAND_ADDRESS;
			}		
			while(FSMC_GetFlagStatus(FSMC_Bank2_NAND,FSMC_FLAG_FEMPT)!= SET);//while(!(FMC_Bank2_3->SR3&(1<<6)));				//等待FIFO空	
			nand_dev.ecc_hdbuf[res+eccstart]=FSMC_Bank2->ECCR2;//FMC_Bank2_3->ECCR3;//读取硬件计算后的ECC值
			FSMC_NANDECCCmd(FSMC_Bank2_NAND, DISABLE);//FMC_Bank2_3->PCR3&=~(1<<6);						//禁止ECC校验
		} 
		i=nand_dev.page_mainsize+0X10+eccstart*4;			//从spare区的0X10位置开始读取之前存储的ecc值
		NAND_Delay(30);//等待tADL 
		*(vu8*)(NAND_ADDRESS|NAND_CMD)=0X05;				//随机读指令
		//发送地址
		*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)i;
		*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(i>>8);
		*(vu8*)(NAND_ADDRESS|NAND_CMD)=0XE0;				//开始读数据
		NAND_Delay(30);//等待tADL 
		pBuffer=(u8*)&nand_dev.ecc_rdbuf[eccstart];
		for(i=0;i<4*eccnum;i++)								//读取保存的ECC值
		{
			*(vu8*)pBuffer++= *(vu8*)NAND_ADDRESS;
		}			
		for(i=0;i<eccnum;i++)								//检验ECC
		{
			if(nand_dev.ecc_rdbuf[i+eccstart]!=nand_dev.ecc_hdbuf[i+eccstart])//不相等,需要校正
			{
				xprintf("err hd,rd:0x%x,0x%x\r\n",nand_dev.ecc_hdbuf[i+eccstart],nand_dev.ecc_rdbuf[i+eccstart]); 
 				xprintf("eccnum,eccstart:%d,%d\r\n",eccnum,eccstart);	
				xprintf("PageNum,ColNum:%d,%d\r\n",PageNum,ColNum);	
				res=NAND_ECC_Correction(p+NAND_ECC_SECTOR_SIZE*i,nand_dev.ecc_rdbuf[i+eccstart],nand_dev.ecc_hdbuf[i+eccstart]);//ECC校验
				if(res)errsta=NSTA_ECC2BITERR;				//标记2BIT及以上ECC错误
				else errsta=NSTA_ECC1BITERR;				//标记1BIT ECC错误
			} 
		} 		
	}
    if(NAND_WaitForReady()!=NSTA_READY)errsta=NSTA_ERROR;	//失败
    return errsta;	//成功   
} 
//读取NAND Flash的指定页指定列的数据(main区和spare区都可以使用此函数),并对比(FTL管理时需要)
//PageNum:要读取的页地址,范围:0~(block_pagenum*block_totalnum-1)
//ColNum:要读取的列开始地址(也就是页内地址),范围:0~(page_totalsize-1)
//CmpVal:要对比的值,以u32为单位
//NumByteToRead:读取字数(以4字节为单位,不能跨页读)
//NumByteEqual:从初始位置持续与CmpVal值相同的数据个数
//返回值:0,成功
//    其他,错误代码
u8 NAND_ReadPageComp(u32 PageNum,u16 ColNum,u32 CmpVal,u16 NumByteToRead,u16 *NumByteEqual)
{
	u16 i=0;
	u8 res=0;
	*(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_AREA_A;
	//发送地址
	*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)ColNum;
	*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(ColNum>>8);
	*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)PageNum;
	*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(PageNum>>8);
#if NAND_ADDR_5 == 1		
	*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(PageNum>>16);
#endif	
	*(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_AREA_TRUE1;
	//下面两行代码是等待R/B引脚变为低电平，其实主要起延时作用的，等待NAND操作R/B引脚。因为我们是通过
	//将STM32的NWAIT引脚(NAND的R/B引脚)配置为普通IO，代码中通过读取NWAIT引脚的电平来判断NAND是否准备
	//就绪的。这个也就是模拟的方法，所以在速度很快的时候有可能NAND还没来得及操作R/B引脚来表示NAND的忙
	//闲状态，结果我们就读取了R/B引脚,这个时候肯定会出错的，事实上确实是会出错!大家也可以将下面两行
	//代码换成延时函数,只不过这里我们为了效率所以没有用延时函数。
#if 0
	res=NAND_WaitRB(0);			//等待RB=0 
	if(res)return NSTA_TIMEOUT;	//超时退出
    //下面2行代码是真正判断NAND是否准备好的
	res=NAND_WaitRB(1);			//等待RB=1 
    if(res)return NSTA_TIMEOUT;	//超时退出  
#else		
	 /* 必须等待，否则读出数据异常, 此处应该判断超时 */
	NAND_Delay(30);
	
/*	ouhs 20180321 使用FSMC_NWAIT信号	
	while( GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_6) == 0);
*/
#endif

	for(i=0;i<NumByteToRead;i++)//读取数据,每次读4字节
	{
		if(*(vu32*)NAND_ADDRESS!=CmpVal)break;	//如果有任何一个值,与CmpVal不相等,则退出.
	}
	*NumByteEqual=i;					//与CmpVal值相同的个数
	if(NAND_WaitForReady()!=NSTA_READY)return NSTA_ERROR;//失败
	return 0;	//成功   
} 
//在NAND一页中写入指定个字节的数据(main区和spare区都可以使用此函数)
//PageNum:要写入的页地址,范围:0~(block_pagenum*block_totalnum-1)
//ColNum:要写入的列开始地址(也就是页内地址),范围:0~(page_totalsize-1)
//pBbuffer:指向数据存储区
//NumByteToWrite:要写入的字节数，该值不能超过该页剩余字节数！！！
//返回值:0,成功 
//    其他,错误代码
u8 NAND_WritePage(u32 PageNum,u16 ColNum,u8 *pBuffer,u16 NumByteToWrite)
{
	vu16 i=0;  
	u8 res=0;
	u8 eccnum=0;		//需要计算的ECC个数，每NAND_ECC_SECTOR_SIZE字节计算一个ecc
	u8 eccstart=0;		//第一个ECC值所属的地址范围
	
	*(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_WRITE0;
	//发送地址
	*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)ColNum;
	*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(ColNum>>8);
	*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)PageNum;
	*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(PageNum>>8);
#if NAND_ADDR_5 == 1			
	*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(PageNum>>16);
#endif	
	NAND_Delay(30);//等待tADL 
	if(NumByteToWrite%NAND_ECC_SECTOR_SIZE)//不是NAND_ECC_SECTOR_SIZE的整数倍，不进行ECC校验
	{  
		for(i=0;i<NumByteToWrite;i++)		//写入数据
		{
			*(vu8*)NAND_ADDRESS=*(vu8*)pBuffer++;
		}
	}else
	{
		eccnum=NumByteToWrite/NAND_ECC_SECTOR_SIZE;			//得到ecc计算次数
		eccstart=ColNum/NAND_ECC_SECTOR_SIZE; 
 		for(res=0;res<eccnum;res++)
		{
			FSMC_NANDECCCmd(FSMC_Bank2_NAND, ENABLE);//FMC_Bank2_3->PCR3|=1<<6;						//使能ECC校验 
			for(i=0;i<NAND_ECC_SECTOR_SIZE;i++)				//写入NAND_ECC_SECTOR_SIZE个数据
			{
				*(vu8*)NAND_ADDRESS=*(vu8*)pBuffer++;
			}		
			while(FSMC_GetFlagStatus(FSMC_Bank2_NAND,FSMC_FLAG_FEMPT)!= SET);//while(!(FMC_Bank2_3->SR3&(1<<6)));				//等待FIFO空	
			nand_dev.ecc_hdbuf[res+eccstart]=FSMC_Bank2->ECCR2;//FMC_Bank2_3->ECCR3;	//读取硬件计算后的ECC值
  		FSMC_NANDECCCmd(FSMC_Bank2_NAND, DISABLE);//FMC_Bank2_3->PCR3&=~(1<<6);						//禁止ECC校验
		}  
		i=nand_dev.page_mainsize+0X10+eccstart*4;			//计算写入ECC的spare区地址
		NAND_Delay(30);//等待 
		*(vu8*)(NAND_ADDRESS|NAND_CMD)=0X85;				//随机写指令
		//发送地址
		*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)i;
		*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(i>>8);
		NAND_Delay(30);//等待tADL 
		pBuffer=(u8*)&nand_dev.ecc_hdbuf[eccstart];
		for(i=0;i<eccnum;i++)					//写入ECC
		{ 
			for(res=0;res<4;res++)				 
			{
				*(vu8*)NAND_ADDRESS=*(vu8*)pBuffer++;
			}
		} 		
	}
	*(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_WRITE_TURE1; 
	if(NAND_WaitForReady()!=NSTA_READY)return NSTA_ERROR;//失败
	return 0;//成功   
}
//在NAND一页中的指定地址开始,写入指定长度的恒定数字
//PageNum:要写入的页地址,范围:0~(block_pagenum*block_totalnum-1)
//ColNum:要写入的列开始地址(也就是页内地址),范围:0~(page_totalsize-1)
//cval:要写入的指定常数
//NumByteToWrite:要写入的字数(以4字节为单位)
//返回值:0,成功 
//    其他,错误代码
u8 NAND_WritePageConst(u32 PageNum,u16 ColNum,u32 cval,u16 NumByteToWrite)
{
	u16 i=0;  
	*(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_WRITE0;
	//发送地址
	*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)ColNum;
	*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(ColNum>>8);
	*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)PageNum;
	*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(PageNum>>8);
#if NAND_ADDR_5 == 1		
	*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(PageNum>>16);
#endif	
	NAND_Delay(30);//等待tADL 
	for(i=0;i<NumByteToWrite;i++)		//写入数据,每次写4字节
	{
		*(vu32*)NAND_ADDRESS=cval;
	} 
	*(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_WRITE_TURE1; 
	if(NAND_WaitForReady()!=NSTA_READY)return NSTA_ERROR;//失败
	return 0;//成功   
}
//将一页数据拷贝到另一页,不写入新数据
//注意:源页和目的页要在同一个Plane内！
//Source_PageNo:源页地址,范围:0~(block_pagenum*block_totalnum-1)
//Dest_PageNo:目的页地址,范围:0~(block_pagenum*block_totalnum-1)  
//返回值:0,成功
//    其他,错误代码
u8 NAND_CopyPageWithoutWrite(u32 Source_PageNum,u32 Dest_PageNum)
{
	u8 res=0;
    u16 source_block=0,dest_block=0;  
    //判断源页和目的页是否在同一个plane中
    source_block=Source_PageNum/nand_dev.block_pagenum;
    dest_block=Dest_PageNum/nand_dev.block_pagenum;
    if((source_block%2)!=(dest_block%2))return NSTA_ERROR;	//不在同一个plane内 
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD0;	//发送命令0X00
    //发送源页地址
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)0;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)0;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)Source_PageNum;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(Source_PageNum>>8);
#if NAND_ADDR_5 == 1		
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(Source_PageNum>>16);
#endif    
		*(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD1;//发送命令0X35 
    //下面两行代码是等待R/B引脚变为低电平，其实主要起延时作用的，等待NAND操作R/B引脚。因为我们是通过
    //将STM32的NWAIT引脚(NAND的R/B引脚)配置为普通IO，代码中通过读取NWAIT引脚的电平来判断NAND是否准备
    //就绪的。这个也就是模拟的方法，所以在速度很快的时候有可能NAND还没来得及操作R/B引脚来表示NAND的忙
    //闲状态，结果我们就读取了R/B引脚,这个时候肯定会出错的，事实上确实是会出错!大家也可以将下面两行
    //代码换成延时函数,只不过这里我们为了效率所以没有用延时函数。
#if 0	
	res=NAND_WaitRB(0);			//等待RB=0 
    if(res)return NSTA_TIMEOUT;	//超时退出
    //下面2行代码是真正判断NAND是否准备好的
	res=NAND_WaitRB(1);			//等待RB=1 
    if(res)return NSTA_TIMEOUT;	//超时退出
#else		
	 /* 必须等待，否则读出数据异常, 此处应该判断超时 */
	NAND_Delay(30);
	
/*	ouhs 20180321 使用FSMC_NWAIT信号	
	while( GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_6) == 0);
*/
#endif
		
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD2;  //发送命令0X85
    //发送目的页地址
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)0;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)0;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)Dest_PageNum;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(Dest_PageNum>>8);
#if NAND_ADDR_5 == 1			
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(Dest_PageNum>>16);
#endif		
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD3;	//发送命令0X10 
    if(NAND_WaitForReady()!=NSTA_READY)return NSTA_ERROR;	//NAND未准备好 
    return 0;//成功   
}

//将一页数据拷贝到另一页,并且可以写入数据
//注意:源页和目的页要在同一个Plane内！
//Source_PageNo:源页地址,范围:0~(block_pagenum*block_totalnum-1)
//Dest_PageNo:目的页地址,范围:0~(block_pagenum*block_totalnum-1)  
//ColNo:页内列地址,范围:0~(page_totalsize-1)
//pBuffer:要写入的数据
//NumByteToWrite:要写入的数据个数
//返回值:0,成功 
//    其他,错误代码
u8 NAND_CopyPageWithWrite(u32 Source_PageNum,u32 Dest_PageNum,u16 ColNum,u8 *pBuffer,u16 NumByteToWrite)
{
	u8 res=0;
    vu16 i=0;
	u16 source_block=0,dest_block=0;  
	u8 eccnum=0;		//需要计算的ECC个数，每NAND_ECC_SECTOR_SIZE字节计算一个ecc
	u8 eccstart=0;		//第一个ECC值所属的地址范围
    //判断源页和目的页是否在同一个plane中
    source_block=Source_PageNum/nand_dev.block_pagenum;
    dest_block=Dest_PageNum/nand_dev.block_pagenum;
    if((source_block%2)!=(dest_block%2))return NSTA_ERROR;//不在同一个plane内
	*(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD0;  //发送命令0X00
    //发送源页地址
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)0;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)0;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)Source_PageNum;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(Source_PageNum>>8);
#if NAND_ADDR_5 == 1		
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(Source_PageNum>>16);
#endif    
		*(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD1;  //发送命令0X35
    
    //下面两行代码是等待R/B引脚变为低电平，其实主要起延时作用的，等待NAND操作R/B引脚。因为我们是通过
    //将STM32的NWAIT引脚(NAND的R/B引脚)配置为普通IO，代码中通过读取NWAIT引脚的电平来判断NAND是否准备
    //就绪的。这个也就是模拟的方法，所以在速度很快的时候有可能NAND还没来得及操作R/B引脚来表示NAND的忙
    //闲状态，结果我们就读取了R/B引脚,这个时候肯定会出错的，事实上确实是会出错!大家也可以将下面两行
    //代码换成延时函数,只不过这里我们为了效率所以没有用延时函数。
#if 0	
	res=NAND_WaitRB(0);			//等待RB=0 
    if(res)return NSTA_TIMEOUT;	//超时退出
    //下面2行代码是真正判断NAND是否准备好的
	res=NAND_WaitRB(1);			//等待RB=1 
    if(res)return NSTA_TIMEOUT;	//超时退出
#else		
	 /* 必须等待，否则读出数据异常, 此处应该判断超时 */
	NAND_Delay(30);
	
/*	ouhs 20180321 使用FSMC_NWAIT信号	
	while( GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_6) == 0);
*/
#endif
		
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD2;  //发送命令0X85
    //发送目的页地址
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)ColNum;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(ColNum>>8);
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)Dest_PageNum;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(Dest_PageNum>>8);
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(Dest_PageNum>>16); 
    //发送页内列地址
	NAND_Delay(30);//等待tADL 
	if(NumByteToWrite%NAND_ECC_SECTOR_SIZE)//不是NAND_ECC_SECTOR_SIZE的整数倍，不进行ECC校验
	{  
		for(i=0;i<NumByteToWrite;i++)		//写入数据
		{
			*(vu8*)NAND_ADDRESS=*(vu8*)pBuffer++;
		}
	}else
	{
		eccnum=NumByteToWrite/NAND_ECC_SECTOR_SIZE;			//得到ecc计算次数
		eccstart=ColNum/NAND_ECC_SECTOR_SIZE; 
 		for(res=0;res<eccnum;res++)
		{
			FSMC_NANDECCCmd(FSMC_Bank2_NAND, ENABLE);//FMC_Bank2_3->PCR3|=1<<6;						//使能ECC校验 
			for(i=0;i<NAND_ECC_SECTOR_SIZE;i++)				//写入NAND_ECC_SECTOR_SIZE个数据
			{
				*(vu8*)NAND_ADDRESS=*(vu8*)pBuffer++;
			}		
			while(FSMC_GetFlagStatus(FSMC_Bank2_NAND,FSMC_FLAG_FEMPT)!= SET);//while(!(FMC_Bank2_3->SR3&(1<<6)));				//等待FIFO空	
			nand_dev.ecc_hdbuf[res+eccstart]=FSMC_Bank2->ECCR2;//FMC_Bank2_3->ECCR3;	//读取硬件计算后的ECC值
 			FSMC_NANDECCCmd(FSMC_Bank2_NAND, DISABLE);//FMC_Bank2_3->PCR3&=~(1<<6);						//禁止ECC校验
		}  
		i=nand_dev.page_mainsize+0X10+eccstart*4;			//计算写入ECC的spare区地址
		NAND_Delay(30);//等待 
		*(vu8*)(NAND_ADDRESS|NAND_CMD)=0X85;				//随机写指令
		//发送地址
		*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)i;
		*(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(i>>8);
		NAND_Delay(30);//等待tADL 
		pBuffer=(u8*)&nand_dev.ecc_hdbuf[eccstart];
		for(i=0;i<eccnum;i++)					//写入ECC
		{ 
			for(res=0;res<4;res++)				 
			{
				*(vu8*)NAND_ADDRESS=*(vu8*)pBuffer++;
			}
		} 		
	}
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD3;	//发送命令0X10 
    if(NAND_WaitForReady()!=NSTA_READY)return NSTA_ERROR;	//失败
    return 0;	//成功   
} 
//读取spare区中的数据
//PageNum:要写入的页地址,范围:0~(block_pagenum*block_totalnum-1)
//ColNum:要写入的spare区地址(spare区中哪个地址),范围:0~(page_sparesize-1) 
//pBuffer:接收数据缓冲区 
//NumByteToRead:要读取的字节数(不大于page_sparesize)
//返回值:0,成功
//    其他,错误代码
u8 NAND_ReadSpare(u32 PageNum,u16 ColNum,u8 *pBuffer,u16 NumByteToRead)
{
    u8 temp=0;
    u8 remainbyte=0;
    remainbyte=nand_dev.page_sparesize-ColNum;
    if(NumByteToRead>remainbyte) NumByteToRead=remainbyte;  //确保要写入的字节数不大于spare剩余的大小
    temp=NAND_ReadPage(PageNum,ColNum+nand_dev.page_mainsize,pBuffer,NumByteToRead);//读取数据
    return temp;
} 
//向spare区中写数据
//PageNum:要写入的页地址,范围:0~(block_pagenum*block_totalnum-1)
//ColNum:要写入的spare区地址(spare区中哪个地址),范围:0~(page_sparesize-1)  
//pBuffer:要写入的数据首地址 
//NumByteToWrite:要写入的字节数(不大于page_sparesize)
//返回值:0,成功
//    其他,失败
u8 NAND_WriteSpare(u32 PageNum,u16 ColNum,u8 *pBuffer,u16 NumByteToWrite)
{
    u8 temp=0;
    u8 remainbyte=0;
    remainbyte=nand_dev.page_sparesize-ColNum;
    if(NumByteToWrite>remainbyte) NumByteToWrite=remainbyte;  //确保要读取的字节数不大于spare剩余的大小
    temp=NAND_WritePage(PageNum,ColNum+nand_dev.page_mainsize,pBuffer,NumByteToWrite);//读取
    return temp;
} 
//擦除一个块
//BlockNum:要擦除的BLOCK编号,范围:0-(block_totalnum-1)
//返回值:0,擦除成功
//    其他,擦除失败
u8 NAND_EraseBlock(u32 BlockNum)
{
		//if(nand_dev.id==MT29F16G08ABABA)BlockNum<<=7;  	//将块地址转换为页地址
    //else if(nand_dev.id==MT29F4G08ABADA)
			BlockNum<<=6;
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_ERASE0;
    //发送块地址
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)BlockNum;
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(BlockNum>>8);
#if NAND_ADDR_5 == 1		
    *(vu8*)(NAND_ADDRESS|NAND_ADDR)=(u8)(BlockNum>>16);
#endif	
    *(vu8*)(NAND_ADDRESS|NAND_CMD)=NAND_ERASE1;
	if(NAND_WaitForReady()!=NSTA_READY)return NSTA_ERROR;//失败
    return 0;	//成功   
} 
//全片擦除NAND FLASH
void NAND_EraseChip(void)
{
    u8 status;
    u16 i=0;
    for(i=0;i<nand_dev.block_totalnum;i++) //循环擦除所有的块
    {
        status=NAND_EraseBlock(i);
        if(status)
					xprintf("Erase %d block fail!!，错误码为%d\r\n",i,status);//擦除失败
    }
}

//获取ECC的奇数位/偶数位
//oe:0,偶数位
//   1,奇数位
//eccval:输入的ecc值
//返回值:计算后的ecc值(最多16位)
u16 NAND_ECC_Get_OE(u8 oe,u32 eccval)
{
	u8 i;
	u16 ecctemp=0;
	for(i=0;i<24;i++)
	{
		if((i%2)==oe)
		{
			if((eccval>>i)&0X01)ecctemp+=1<<(i>>1); 
		}
	}
	return ecctemp;
} 
//ECC校正函数
//eccrd:读取出来,原来保存的ECC值
//ecccl:读取数据时,硬件计算的ECC只
//返回值:0,错误已修正
//    其他,ECC错误(有大于2个bit的错误,无法恢复)
u8 NAND_ECC_Correction(u8* data_buf,u32 eccrd,u32 ecccl)
{
	u16 eccrdo,eccrde,eccclo,ecccle;
	u16 eccchk=0;
	u16 errorpos=0; 
	u32 bytepos=0;  
	eccrdo=NAND_ECC_Get_OE(1,eccrd);	//获取eccrd的奇数位
	eccrde=NAND_ECC_Get_OE(0,eccrd);	//获取eccrd的偶数位
	eccclo=NAND_ECC_Get_OE(1,ecccl);	//获取ecccl的奇数位
	ecccle=NAND_ECC_Get_OE(0,ecccl); 	//获取ecccl的偶数位
	eccchk=eccrdo^eccrde^eccclo^ecccle;
	if(eccchk==0XFFF)	//全1,说明只有1bit ECC错误
	{
		errorpos=eccrdo^eccclo; 
		xprintf("errorpos:%d\r\n",errorpos); 
		bytepos=errorpos/8; 
		data_buf[bytepos]^=1<<(errorpos%8);
	}else				//不是全1,说明至少有2bit ECC错误,无法修复
	{
		xprintf("2bit ecc error or more\r\n");
		return 1;
	} 
	return 0;
}
 






