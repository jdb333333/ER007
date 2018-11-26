
#include "at45db321.h"
unsigned char DF_buffer[528];
#define SPI_FLASH_PageSize	528
#define SPI_FLASH_PageNum		8192
#define SPI_FLASH_SIZE			0x400000  //4MByte
#define ddd                	111
#define Dummy_Byte1  0xA5
/*******************************************************************************
* Function Name  : SPI_FLASH_Init
* Description    : Initializes the peripherals used by the SPI FLASH driver.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_Init(void)
{
  SPI_InitTypeDef    SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStruct;  	
   
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOA, ENABLE);
  
  	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_25MHz;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;  

   GPIO_InitStruct.GPIO_Pin=GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;
   GPIO_Init(GPIOB,&GPIO_InitStruct);
   	  
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);  
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);	
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);	 
												   
  SPI_I2S_DeInit(SPI1);	    
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; 
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master; 
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; 
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;//;Low//; 
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge; 
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;   //SPI_NSS_Hard
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2; 
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; 
  SPI_InitStructure.SPI_CRCPolynomial = 7; 
  SPI_Init(SPI1,&SPI_InitStructure); 
  SPI_Cmd(SPI1,ENABLE); 

  //CS
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_25MHz;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;  

   GPIO_InitStruct.GPIO_Pin=GPIO_Pin_4;
   GPIO_Init(GPIOA,&GPIO_InitStruct);     	

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();
 
}
/*******************************************************************************
* Function Name  : SPI_FLASH_ReadByte
* Description    : Reads a byte from the SPI Flash.
*                  This function must be used only if the Start_Read_Sequence
*                  function has been previously called.
* Input          : None
* Output         : None
* Return         : Byte Read from the SPI Flash.
*******************************************************************************/
u8 SPI_FLASH_ReadByte(u8 Dummy_Byte)
{
  return (SPI_FLASH_SendByte(Dummy_Byte));
}

/*******************************************************************************
* Function Name  : SPI_FLASH_SendByte
* Description    : Sends a byte through the SPI interface and return the byte 
*                  received from the SPI bus.
* Input          : byte : byte to send.
* Output         : None
* Return         : The value of the received byte.
*******************************************************************************/
u8 SPI_FLASH_SendByte(u8 byte)
{
#if 0
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI1, byte);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
  return SPI_I2S_ReceiveData(SPI1);
#else
	while ((SPI1->SR & SPI_I2S_FLAG_TXE) == RESET);
	SPI1->DR = byte;
	while ((SPI1->SR & SPI_I2S_FLAG_RXNE) == RESET);
	return SPI1->DR;
#endif
}

uint8_t SPI_FLASH_BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
	u32 i=0,j=0;
	u32 PageAddr=0;	

	if ((WriteAddr + NumByteToWrite > SPI_FLASH_SIZE)//Out of the max size of flash
      ||(NumByteToWrite == 0))
    return (ERROR);	
	
	PageAddr=WriteAddr/SPI_FLASH_PageSize;
	if((WriteAddr%SPI_FLASH_PageSize)==0)
	{
		i=NumByteToWrite/SPI_FLASH_PageSize;
		if(i)
		{
			for(j=0;j<i;j++)
			{
				DF_write_page(pBuffer,PageAddr);
				PageAddr++;
				pBuffer+=SPI_FLASH_PageSize;	
			 }
		}
		i=(NumByteToWrite%SPI_FLASH_PageSize);
		if(i)
		{
			DF_mm_to_buf(1,PageAddr);
			DF_write_buf(1,0,i,pBuffer);
			DF_buf_to_mm(1,PageAddr);
		}	
	}
	else 
	{
		DF_mm_to_buf(1,PageAddr);
		i = WriteAddr%SPI_FLASH_PageSize;
		j = i+NumByteToWrite;
		j = j/SPI_FLASH_PageSize;	
		if(j)
		{
			DF_write_buf(1,i,(SPI_FLASH_PageSize-i),pBuffer);
			DF_buf_to_mm(1,PageAddr);
			PageAddr++;
			pBuffer+=(SPI_FLASH_PageSize-i);
			for(;j!=0;j--)
			{
				if(j!=1)
				{
					DF_write_page(pBuffer,PageAddr);
					PageAddr++;
					pBuffer+=SPI_FLASH_PageSize;			
				}
				else 
				{
					DF_mm_to_buf(1,PageAddr);
					DF_write_buf(1,0,(i+NumByteToWrite)%SPI_FLASH_PageSize,pBuffer);
					DF_buf_to_mm(1,PageAddr);
				}	
			}
		}
		else 
		{
			DF_write_buf(1,i,NumByteToWrite,pBuffer);
			DF_buf_to_mm(1,PageAddr);	
		}
	};
	
	return (SUCCESS);
	
}

uint8_t SPI_FLASH_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	u32 i=0,j=0;
	u32 PageAddr=0;	

	if ((ReadAddr + NumByteToRead > SPI_FLASH_SIZE)//Out of the max size of flash
		||(NumByteToRead == 0))
	return (ERROR);	
	
	PageAddr=ReadAddr/SPI_FLASH_PageSize;
	if((ReadAddr%SPI_FLASH_PageSize)==0)
	{
		i=NumByteToRead/SPI_FLASH_PageSize;
		if(i)
		{
			for(j=0;j<i;j++)
			{
				DF_read_page(pBuffer,PageAddr);
				PageAddr++;
				pBuffer+=SPI_FLASH_PageSize;	
			 }
		}
		i=(NumByteToRead%SPI_FLASH_PageSize);

		if(i)
		{
			DF_mm_to_buf(2,PageAddr);
			DF_read_buf(2,0,i,pBuffer);
		}	
	}
	else 
	{
		DF_mm_to_buf(2,PageAddr);
		i = ReadAddr%SPI_FLASH_PageSize;
		j = i+NumByteToRead;
		j = j/SPI_FLASH_PageSize;

		if(j)
		{
			
			DF_read_buf(2,i,(SPI_FLASH_PageSize-i),pBuffer);
			PageAddr++;
			pBuffer+=(SPI_FLASH_PageSize-i);
			for(;j!=0;j--)
			{
				if(j!=1)
				{
					DF_read_page(pBuffer,PageAddr);
					PageAddr++;
					pBuffer+=SPI_FLASH_PageSize;			
				}
				else 
				{
					DF_mm_to_buf(2,PageAddr);
					DF_read_buf(2,0,(i+NumByteToRead)%SPI_FLASH_PageSize,pBuffer);
				}	
			}  
		}
		else 
		{
			DF_read_buf(2,i,NumByteToRead,pBuffer);	
		}  
	};
	
	return (SUCCESS);
}

void SPI_FLASH_StartReadSequence(u32 ReadAddr)
{
	#if SPI_FLASH_PageSize == 528
    u32 PAddr =0 ,BAddr=0;
    PAddr = (ReadAddr/SPI_FLASH_PageSize);
	BAddr = (ReadAddr%SPI_FLASH_PageSize);
	#endif
	DF_wait_busy();
	/* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();
  
  /* Send "Read from Memory " instruction */
  SPI_FLASH_SendByte(READ_SEQUENCE);
 #if SPI_FLASH_PageSize == 528
/* Send the 24-bit address of the address to read from -----------------------*/  
  /* Send ReadAddr high nibble address byte */
 SPI_FLASH_SendByte(( PAddr & 0x0fc0)  >> 6);
    /* Send ReadAddr medium nibble address byte */
  SPI_FLASH_SendByte(((PAddr &0x003f) << 2)| ((BAddr&0x0030)>>8));
   /* Send ReadAddr low nibble address byte */
  SPI_FLASH_SendByte(BAddr&0x000f);
  #else
   SPI_FLASH_SendByte((ReadAddr&0x003f0000)>>16);
   SPI_FLASH_SendByte((ReadAddr&0x0000ff00)>>8);
   SPI_FLASH_SendByte(ReadAddr&0x000000ff);	
   #endif
} 

/*******************************************************************************
* Function Name  : SPI_FLASH_SendHalfWord
* Description    : Sends a Half Word through the SPI interface and return the  
*                  Half Word received from the SPI bus.
* Input          : Half Word : Half Word to send.
* Output         : None
* Return         : The value of the received Half Word.
*******************************************************************************/
u16 SPI_FLASH_SendHalfWord(u16 HalfWord)
{
	//SPI_FLASH_CS_LOW();
  /* Loop while DR register in not emplty */
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

  /* Send Half Word through the SPI1 peripheral */
  SPI_I2S_SendData(SPI1, HalfWord);

  /* Wait to receive a Half Word */
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
 // SPI_FLASH_CS_HIGH();
  /* Return the Half Word read from the SPI bus */
  return SPI_I2S_ReceiveData(SPI1);
  
}

/*******************************************************************************
* Function Name  : SPI_FLASH_SectorErase
* Description    : Erases the specified FLASH sector.
* Input          : SectorAddr: address of the sector to erase.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_SectorErase(u32 SectorAddr)
{
  /* Send write enable instruction */
 // SPI_FLASH_WriteEnable();

  /* Sector Erase */
  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();
  /* Send Sector Erase instruction */
 // SPI_FLASH_SendByte(SE);
  /* Send SectorAddr high nibble address byte */
  SPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
  /* Send SectorAddr medium nibble address byte */
  SPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);
  /* Send SectorAddr low nibble address byte */
  SPI_FLASH_SendByte(SectorAddr & 0xFF);
  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();

  /* Wait the end of Flash writing */
  //SPI_FLASH_WaitForWriteEnd();
}


//擦除指定的主存储器页（地址范围0-4095）
void DF_page_earse(unsigned int page)
{

	DF_wait_busy();
	
	SPI_FLASH_CS_LOW();
	SPI_FLASH_SendByte(PAGE_ERASE);
	SPI_FLASH_SendByte((unsigned char)(page >> 6));
	SPI_FLASH_SendByte((unsigned char)(page << 2));
	SPI_FLASH_SendByte(0x00);
	SPI_FLASH_CS_HIGH();

}


//将保存在数组DF_buffer[]中的一页数据写入第二缓冲区后送入主存储区
//（先擦除后写入模式，页地址范围0-4095）
void DF_write_page(u8* pBuffer,u16 page)
{
	unsigned int i;
	
	DF_wait_busy();
	SPI_FLASH_CS_LOW();
	SPI_FLASH_SendByte(BUFFER_2_WRITE);
	SPI_FLASH_SendByte(0x00);
	SPI_FLASH_SendByte(0x00);
	SPI_FLASH_SendByte(0x00);
	for (i=0;i<SPI_FLASH_PageSize;i++)
	{
		SPI_FLASH_SendByte(*pBuffer);
		pBuffer++;
	}
	SPI_FLASH_CS_HIGH();
	
	if (page<SPI_FLASH_PageNum)
	{
		SPI_FLASH_CS_LOW();
		SPI_FLASH_SendByte(B2_TO_MM_PAGE_PROG_WITH_ERASE);//SPI_FLASH_SendByte(B2_TO_MM_PAGE_PROG_WITHOUT_ERASE);
		SPI_FLASH_SendByte((unsigned char)(page>>6));
		SPI_FLASH_SendByte((unsigned char)(page<<2));
		SPI_FLASH_SendByte(0x00);
		SPI_FLASH_CS_HIGH();
		DF_wait_busy();
	}

}


//将指定主存储器页的数据转入第一缓冲区后读出，保存在DF_buffer[]数组中
//（页地址范围0-4095）
void DF_read_page	(u8* pBuffer,u16 page)
{
	unsigned int i;
	
//	while(!(DF_STA_PORT & (1<< DF_STATE)));
	DF_wait_busy();
	SPI_FLASH_CS_LOW();
	SPI_FLASH_SendByte(MM_PAGE_TO_B1_XFER);
	SPI_FLASH_SendByte((unsigned char)(page >> 6));
    SPI_FLASH_SendByte((unsigned char)(page << 2));
    SPI_FLASH_SendByte(0x00);
	SPI_FLASH_CS_HIGH();
	
	DF_wait_busy();
	SPI_FLASH_CS_LOW();
	SPI_FLASH_SendByte(BUFFER_1_READ); 
	SPI_FLASH_SendByte(0x00);                        
	SPI_FLASH_SendByte(0x00);   
	SPI_FLASH_SendByte(0x00);        
	SPI_FLASH_SendByte(0x00);
	for (i=0;i<SPI_FLASH_PageSize;i++)
	{
		
		*pBuffer = 	SPI_FLASH_ReadByte(0xFF);
		pBuffer++;
	}
	SPI_FLASH_CS_HIGH();
	
}


//以直接读取方式读取指定的主存储器页数据（页地址范围0-4095）


//读取状态寄存器
//	bit7		bit6	bit6	bit6	bit6	bit6	bit6		bit6
//RDY/BUSY		COMP	1		0		1		1		PROTECT		PAGE SIZE	
unsigned char DF_read_reg(void)
{
	unsigned char temp;

	SPI_FLASH_CS_LOW();
	SPI_FLASH_SendByte(READ_STATE_REGISTER);
	SPI_FLASH_SendByte(0x00);
	SPI_FLASH_SendByte(0x00);                        
	SPI_FLASH_SendByte(0x00);   
	
	temp= SPI_FLASH_ReadByte(0x00);
	SPI_FLASH_CS_HIGH();

	return temp;
}


//检查状态寄存器最高位是否为忙，并等待空闲
void DF_wait_busy(void)
{
	unsigned char state_reg=0x00;
	SPI_FLASH_CS_LOW();
	SPI_FLASH_SendByte(READ_STATE_REGISTER);
	SPI_FLASH_SendByte(0x00);
	SPI_FLASH_SendByte(0x00);                        
	SPI_FLASH_SendByte(0x00);
	while((state_reg&0x80) == 0)
	{
		
		state_reg =  SPI_FLASH_ReadByte(0x00);
	}
	SPI_FLASH_CS_HIGH();
}


//将指定主存储器页的数据转入指定缓冲区
void DF_mm_to_buf(unsigned char buffer,unsigned int page)
{

	DF_wait_busy();
	SPI_FLASH_CS_LOW();
	if (buffer==1)
		SPI_FLASH_SendByte(MM_PAGE_TO_B1_XFER);
	else
		SPI_FLASH_SendByte(MM_PAGE_TO_B2_XFER);
	SPI_FLASH_SendByte((unsigned char)(page >> 6));
    SPI_FLASH_SendByte((unsigned char)(page << 2));
    SPI_FLASH_SendByte(0x00);
	SPI_FLASH_CS_HIGH();

}


//读取指定缓冲区指定单元的数据，保存在DF_buffer[]数组中
u8 DF_read_buf(u8 buffer,u16 start_address,u16 length,u8* pBuffer)
{
	unsigned int i;
	if ((SPI_FLASH_PageSize-start_address)>=length)
	{
		DF_wait_busy();
		SPI_FLASH_CS_LOW();
		if (buffer==1)
			SPI_FLASH_SendByte(BUFFER_1_READ); 
		else
			SPI_FLASH_SendByte(BUFFER_2_READ);
		SPI_FLASH_SendByte(0x00);                        
		SPI_FLASH_SendByte((unsigned char)(start_address >> 8));   
		SPI_FLASH_SendByte((unsigned char)start_address);        
		SPI_FLASH_SendByte(0x00);
		for (i=0;i<length;i++)
		{
			*pBuffer = 	SPI_FLASH_ReadByte(0xFF);
			pBuffer++;
		}
		SPI_FLASH_CS_HIGH();
		return 1;
	}
	else
		return 0;
}


//将DF_buffer[]数组中指定长度的数据写入指定缓冲区
u8 DF_write_buf(u8 buffer,u16 start_address,u16 length,u8* pBuffer)
{
	unsigned int i;
	if  ((SPI_FLASH_PageSize-start_address)>=length)
	{
		DF_wait_busy();
		SPI_FLASH_CS_LOW();
		if (buffer==1)
			SPI_FLASH_SendByte(BUFFER_1_WRITE);
		else
			SPI_FLASH_SendByte(BUFFER_2_WRITE);
		SPI_FLASH_SendByte(0x00);
		SPI_FLASH_SendByte((unsigned char)(start_address >> 8));   
		SPI_FLASH_SendByte((unsigned char)start_address);
		for (i=0;i<length;i++)
			{SPI_FLASH_SendByte(*pBuffer);pBuffer++;}
		SPI_FLASH_CS_HIGH();
		return 1;
	}
	else
		return 0;
}


//将指定缓冲区中的数据写入主存储区的指定页
void DF_buf_to_mm(unsigned char buffer,unsigned int page)
{
	DF_wait_busy();
	if (page<SPI_FLASH_PageNum)
	{
		SPI_FLASH_CS_LOW();
		if (buffer==1)
			SPI_FLASH_SendByte(B1_TO_MM_PAGE_PROG_WITH_ERASE);//SPI_FLASH_SendByte(B1_TO_MM_PAGE_PROG_WITHOUT_ERASE);
		else
			SPI_FLASH_SendByte(B2_TO_MM_PAGE_PROG_WITH_ERASE);//SPI_FLASH_SendByte(B2_TO_MM_PAGE_PROG_WITHOUT_ERASE);
		SPI_FLASH_SendByte((unsigned char)(page>>6));
		SPI_FLASH_SendByte((unsigned char)(page<<2));
		SPI_FLASH_SendByte(0x00);
		SPI_FLASH_CS_HIGH();
	}
}


/*******************************************************************************
* Function Name  : SPI_FLASH_ReadID
* Description    : Reads FLASH identification.
* Input          : None
* Output         : None
* Return         : FLASH identification
*******************************************************************************/
u32 SPI_FLASH_ReadID(void)
{
  u32 Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;

  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();

  /* Send "RDID " instruction */
  SPI_FLASH_SendByte(0x9F);

  /* Read a byte from the FLASH */
  Temp0 = SPI_FLASH_SendByte(Dummy_Byte1);
  // printf("\nTemp0  %x",Temp0);
  /* Read a byte from the FLASH */
  Temp1 = SPI_FLASH_SendByte(Dummy_Byte1);
   // printf("\nTemp1  %x",Temp1);
  /* Read a byte from the FLASH */
  Temp2 = SPI_FLASH_SendByte(Dummy_Byte1);
   // printf("\nTemp2  %x",Temp2);
  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();

  Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;

  return Temp;
}

/*******************************************************************************
SPI_FLASH_ChipErase
*******************************************************************************/
u32 SPI_FLASH_ChipErase(void)
{
	SPI_FLASH_CS_LOW();
	
	SPI_FLASH_SendByte(0xC7);
	SPI_FLASH_SendByte(0x94);
	SPI_FLASH_SendByte(0x80);
	SPI_FLASH_SendByte(0x9A);
	SPI_FLASH_CS_HIGH();
	
	DF_wait_busy();
	
	return (SUCCESS);
}
