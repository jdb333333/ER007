/**
  ******************************************************************************
  * @file    stm32_eval_spi_sd.c
  * @author  MCD Application Team
  * @version V4.5.0
  * @date    07-March-2011  
  * Modefied
  * @date    03-April-2012
  * @author  Armjishu.com Application Team
  * @version V2.1
  * @brief   This file provides a set of functions needed to manage the SPI SD 
  *          Card memory mounted on STM32xx-EVAL board (refer to stm32_eval.h
  *          to know about the boards supporting this memory). 
  *          It implements a high level communication layer for read and write 
  *          from/to this memory. The needed STM32 hardware resources (SPI and 
  *          GPIO) are defined in stm32xx_eval.h file, and the initialization is 
  *          performed in SD_LowLevel_Init() function declared in stm32xx_eval.c 
  *          file.
  *          You can easily tailor this driver to any other development board, 
  *          by just adapting the defines for hardware resources and 
  *          SD_LowLevel_Init() function.
  *
  *          ===================================================================
  *          Note: 
  *           - This driver does support SD High Capacity cards.
  *          ===================================================================
  *
  *          +-------------------------------------------------------+
  *          |                     Pin assignment                    |
  *          +-------------------------+---------------+-------------+
  *          |  STM32 SPI Pins         |     SD        |    Pin      |
  *          +-------------------------+---------------+-------------+
  *          | SD_SPI_CS_PIN           |   ChipSelect  |    1        |
  *          | SD_SPI_MOSI_PIN / MOSI  |   DataIn      |    2        |
  *          |                         |   GND         |    3 (0 V)  |
  *          |                         |   VDD         |    4 (3.3 V)|
  *          | SD_SPI_SCK_PIN / SCLK   |   Clock       |    5        |
  *          |                         |   GND         |    6 (0 V)  |
  *          | SD_SPI_MISO_PIN / MISO  |   DataOut     |    7        |
  *          +-------------------------+---------------+-------------+
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************  
  */

/* Includes ------------------------------------------------------------------*/
#include "spi_sd.h"
#include <stdio.h>


static u8 flag_SDHC = 0;

/** @addtogroup Utilities
  * @{
  */
  
/** @addtogroup STM32_EVAL
  * @{
  */ 

/** @addtogroup Common
  * @{
  */
  
/** @addtogroup STM32_EVAL_SPI_SD
  * @brief      This file includes the SD card driver of STM32-EVAL boards.
  * @{
  */ 

/** @defgroup STM32_EVAL_SPI_SD_Private_Types
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup STM32_EVAL_SPI_SD_Private_Defines
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup STM32_EVAL_SPI_SD_Private_Macros
  * @{
  */
/**
  * @}
  */ 
  

/** @defgroup STM32_EVAL_SPI_SD_Private_Variables
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup STM32_EVAL_SPI_SD_Private_Function_Prototypes
  * @{
  */
/**
  * @}
  */ 


/** @defgroup STM32_EVAL_SPI_SD_Private_Functions
  * @{
  */ 

/**
  * @brief  DeInitializes the SD/SD communication.
  * @param  None
  * @retval None
  */
void SD_DeInit(void)
{
  SD_LowLevel_DeInit();
}

/**
  * @brief  Initializes the SD/SD communication.
  * @param  None
  * @retval The SD Response: 
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_Init(void)
{
  uint32_t TimeOut, i = 0;
  SD_Error Status = SD_RESPONSE_NO_ERROR;

  /*!< Initialize SD_SPI */
  SD_LowLevel_Init(); 
  SD_SPI_SetSpeedLow();

  /*------------Put SD in SPI mode--------------*/
  /*!< SD initialized and set to SPI mode properly */
  TimeOut = 0;
  do
  {
    /*!< SD chip select high */
    SD_CS_HIGH();
  
    /*!< Send dummy byte 0xFF, 10 times with CS high */
    /*!< Rise CS and MOSI for 80 clocks cycles */
    for (i = 0; i <= 9; i++)
    {
      /*!< Send dummy byte 0xFF */
      SD_WriteByte(SD_DUMMY_BYTE);
    }    
    
	Status = SD_GoIdleState();
    if(TimeOut > 6)
    {
      break;
    };
    TimeOut++;
  }while(Status);

  SD_SPI_SetSpeedHigh();
  
  return (Status);
}

/**
 * @brief  Detect if SD card is correctly plugged in the memory slot.
 * @param  None
 * @retval Return if SD is detected or not
 */
uint8_t SD_Detect(void)
{
  __IO uint8_t status = SD_PRESENT;

  /*!< Check GPIO to detect SD */
  if (GPIO_ReadInputData(SD_DETECT_GPIO_PORT_1ST) & SD_DETECT_PIN_1ST)
  {
    status = SD_NOT_PRESENT;
  }
  return status;
}

/**
  * @brief  Returns information about specific card.
  * @param  cardinfo: pointer to a SD_CardInfo structure that contains all SD 
  *         card information.
  * @retval The SD Response:
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
uint32_t CardCapacity=0;
SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo)
{
  SD_Error status = SD_RESPONSE_FAILURE;

  status = SD_GetCSDRegister(&(cardinfo->SD_csd));
  if(status == SD_RESPONSE_FAILURE)
  {
  	status = SD_GetCSDRegister(&(cardinfo->SD_csd));
    if(status == SD_RESPONSE_FAILURE)
		return status;
  }
  
  status = SD_GetCIDRegister(&(cardinfo->SD_cid));
  if(status == SD_RESPONSE_FAILURE)
  {
  	status = SD_GetCIDRegister(&(cardinfo->SD_cid));
	if(status == SD_RESPONSE_FAILURE)
    	return status;
  } 
  
  cardinfo->CardCapacity = (cardinfo->SD_csd.DeviceSize + 1) ;
  cardinfo->CardCapacity *= (1 << (cardinfo->SD_csd.DeviceSizeMul + 2));
  cardinfo->CardBlockSize = 1 << (cardinfo->SD_csd.RdBlockLen);
  cardinfo->CardCapacity *= cardinfo->CardBlockSize;

	//ouhs
	CardCapacity = cardinfo->CardCapacity;
/*
	xprintf("\n\r SD CardBlockSize %d CardCapacity %d M", cardinfo->CardBlockSize, cardinfo->CardCapacity / 1024 / 1024);
	xprintf("\n\r SD Product name: %c%c%c%c%c", (cardinfo->SD_cid.ProdName1 & 0xff000000)>>24, 
							(cardinfo->SD_cid.ProdName1 & 0xff0000)>>16, (cardinfo->SD_cid.ProdName1 & 0xff00)>>8,
							(cardinfo->SD_cid.ProdName1 & 0xff),  cardinfo->SD_cid.ProdName2);
	xprintf("\n\r SD ManufactDate: 20%02d.%d \n", (cardinfo->SD_cid.ManufactDate & 0x0ff0)>>4, 
							(cardinfo->SD_cid.ManufactDate & 0x0f));	
*/	
	
  /*!< Returns the reponse */
  return status;
}

/**
  * @brief  Reads a block of data from the SD.
  * @param  pBuffer: pointer to the buffer that receives the data read from the 
  *                  SD.
  * @param  ReadAddr: SD's internal address to read from.
  * @param  BlockSize: the SD card Data block size.
  * @retval The SD Response:
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_ReadBlock(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t BlockSize)
{
  uint32_t i = 0;
  SD_Error rvalue = SD_RESPONSE_FAILURE;
  SD_Error status = SD_RESPONSE_FAILURE;
	
  /*!< SD chip select low */
  SD_CS_LOW();
  
if(flag_SDHC == 1)
{
	ReadAddr = ReadAddr/512;
}
  /*!< Send CMD17 (SD_CMD_READ_SINGLE_BLOCK) to read one block */
  SD_SendCmd(SD_CMD_READ_SINGLE_BLOCK, ReadAddr, 0xFF);


//ouhs>>>>>>>>>>>>>>>>>>>>>>
//使用fatfs有时对cmd17命令没有响应，需要重发一次命令。但当作flash操作时没有发现该问题.
	status = SD_GetResponse(SD_RESPONSE_NO_ERROR);
  if (status != SD_RESPONSE_NO_ERROR)
	{
			SD_SendCmd(SD_CMD_READ_SINGLE_BLOCK, ReadAddr, 0xFF);
			status = SD_GetResponse(SD_RESPONSE_NO_ERROR);
	}
//ouhs<<<<<<<<<<<<<<<<<<<<<<  

  /*!< Check if the SD acknowledged the read block command: R1 response (0x00: no errors) */
  //ouhs if (!SD_GetResponse(SD_RESPONSE_NO_ERROR))
	if (status == SD_RESPONSE_NO_ERROR)
  {
    /*!< Now look for the data token to signify the start of the data */
    if (!SD_GetResponse(SD_START_DATA_SINGLE_BLOCK_READ))
    {
#if (SPI_DMA==0)		
      /*!< Read the SD block data : read NumByteToRead data */
      for (i = 0; i < BlockSize; i++)
      {
        /*!< Save the received data */
        *pBuffer = SD_ReadByte();
       
        /*!< Point to the next location where the byte read will be saved */
        pBuffer++;
      }	
#else			
			DMA1_Start_SPI_RX(pBuffer,BlockSize);			
#endif	
			
      /*!< Get CRC bytes (not really needed by us, but required by SD) */
      SD_ReadByte();
      SD_ReadByte();
					
      /*!< Set response value to success */
      rvalue = SD_RESPONSE_NO_ERROR;
    }
  }
  /*!< SD chip select high */
  SD_CS_HIGH();
  
  /*!< Send dummy byte: 8 Clock pulses of delay */
  SD_WriteByte(SD_DUMMY_BYTE);
  
  /*!< Returns the reponse */
  return rvalue;
}

/**
  * @brief  Reads multiple block of data from the SD.
  * @param  pBuffer: pointer to the buffer that receives the data read from the 
  *                  SD.
  * @param  ReadAddr: SD's internal address to read from.
  * @param  BlockSize: the SD card Data block size.
  * @param  NumberOfBlocks: number of blocks to be read.
  * @retval The SD Response:
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_ReadMultiBlocks(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
  uint32_t i = 0, Offset = 0;
  SD_Error rvalue = SD_RESPONSE_FAILURE;
  
  /*!< SD chip select low */
  SD_CS_LOW();

  /*!< Data transfer */
  while (NumberOfBlocks--)
  {
    if(flag_SDHC == 1)
    {
        /*!< Send CMD17 (SD_CMD_READ_SINGLE_BLOCK) to read one block */
        SD_SendCmd (SD_CMD_READ_SINGLE_BLOCK,(ReadAddr + Offset)/512, 0xFF);
    }
    else
    {
        /*!< Send CMD17 (SD_CMD_READ_SINGLE_BLOCK) to read one block */
        SD_SendCmd (SD_CMD_READ_SINGLE_BLOCK, ReadAddr + Offset, 0xFF);
    }
    /*!< Check if the SD acknowledged the read block command: R1 response (0x00: no errors) */
    if (SD_GetResponse(SD_RESPONSE_NO_ERROR))
    {
      return  SD_RESPONSE_FAILURE;
    }
    /*!< Now look for the data token to signify the start of the data */
    if (!SD_GetResponse(SD_START_DATA_SINGLE_BLOCK_READ))
    {
#if (SPI_DMA==0)				
      /*!< Read the SD block data : read NumByteToRead data */
      for (i = 0; i < BlockSize; i++)
      {
        /*!< Read the pointed data */
        *pBuffer = SD_ReadByte();
        /*!< Point to the next location where the byte read will be saved */
        pBuffer++;
      }
#else
			DMA1_Start_SPI_RX(pBuffer,BlockSize);
			pBuffer += 512;			
#endif
      /*!< Set next read address*/
      Offset += 512;
      /*!< get CRC bytes (not really needed by us, but required by SD) */
      SD_ReadByte();
      SD_ReadByte();
      /*!< Set response value to success */
      rvalue = SD_RESPONSE_NO_ERROR;
    }
    else
    {
      /*!< Set response value to failure */
      rvalue = SD_RESPONSE_FAILURE;
    }
  }
  /*!< SD chip select high */
  SD_CS_HIGH();
  /*!< Send dummy byte: 8 Clock pulses of delay */
  SD_WriteByte(SD_DUMMY_BYTE);
  /*!< Returns the reponse */
  return rvalue;
}

/**
  * @brief  Writes a block on the SD
  * @param  pBuffer: pointer to the buffer containing the data to be written on 
  *                  the SD.
  * @param  WriteAddr: address to write on.
  * @param  BlockSize: the SD card Data block size.
  * @retval The SD Response: 
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_WriteBlock(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t BlockSize)
{
  uint32_t i = 0;
  SD_Error rvalue = SD_RESPONSE_FAILURE;
  SD_Error status = SD_RESPONSE_FAILURE;

  /*!< SD chip select low */
  SD_CS_LOW();

  if(flag_SDHC == 1)
  {
  	WriteAddr = WriteAddr/512;
  }
  /*!< Send CMD24 (SD_CMD_WRITE_SINGLE_BLOCK) to write multiple block */
  SD_SendCmd(SD_CMD_WRITE_SINGLE_BLOCK, WriteAddr, 0xFF);
	
//ouhs>>>>>>>>>>>>>>>>>>>>>>
//使用fatfs有时对cmd24命令没有响应，需要重发一次命令。但当作flash操作时没有发现该问题.
	status = SD_GetResponse(SD_RESPONSE_NO_ERROR);
  if (status != SD_RESPONSE_NO_ERROR)
	{		
		SD_SendCmd(SD_CMD_WRITE_SINGLE_BLOCK, WriteAddr, 0xFF);
		status = SD_GetResponse(SD_RESPONSE_NO_ERROR);
	}
//ouhs<<<<<<<<<<<<<<<<<<<<<<  
	
  /*!< Check if the SD acknowledged the write block command: R1 response (0x00: no errors) */
  //if (!SD_GetResponse(SD_RESPONSE_NO_ERROR))
	if (status == SD_RESPONSE_NO_ERROR)
  {
    /*!< Send a dummy byte */
    SD_WriteByte(SD_DUMMY_BYTE);

    /*!< Send the data token to signify the start of the data */
    SD_WriteByte(0xFE);
#if (SPI_DMA==0)
    /*!< Write the block data to SD : write count data by block */
    for (i = 0; i < BlockSize; i++)
    {
      /*!< Send the pointed byte */
      SD_WriteByte(*pBuffer);
      /*!< Point to the next location where the byte read will be saved */
      pBuffer++;
    }
#else
		DMA1_Start_SPI_TX(pBuffer);
		
#endif
		
    /*!< Put CRC bytes (not really needed by us, but required by SD) */
    SD_ReadByte();
    SD_ReadByte();

    /*!< Read data response */
    if (SD_GetDataResponse() == SD_DATA_OK)
    {
      rvalue = SD_RESPONSE_NO_ERROR;
    }
  }
  /*!< SD chip select high */
  SD_CS_HIGH();
  /*!< Send dummy byte: 8 Clock pulses of delay */
  SD_WriteByte(SD_DUMMY_BYTE);

  /*!< Returns the reponse */
  return rvalue;
}

/**
  * @brief  Writes many blocks on the SD
  * @param  pBuffer: pointer to the buffer containing the data to be written on 
  *                  the SD.
  * @param  WriteAddr: address to write on.
  * @param  BlockSize: the SD card Data block size.
  * @param  NumberOfBlocks: number of blocks to be written.
  * @retval The SD Response: 
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_WriteMultiBlocks(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
  uint32_t i = 0, Offset = 0;
  SD_Error rvalue = SD_RESPONSE_FAILURE;

  /*!< SD chip select low */
  SD_CS_LOW();
  /*!< Data transfer */
  while (NumberOfBlocks--)
  {
    if(flag_SDHC == 1)
    {
        /* Send CMD24 (MSD_WRITE_BLOCK) to write blocks */
        SD_SendCmd(SD_CMD_WRITE_SINGLE_BLOCK, (WriteAddr + Offset)/512, 0xFF);
    }
    else
    {
        /*!< Send CMD24 (SD_CMD_WRITE_SINGLE_BLOCK) to write blocks */
        SD_SendCmd(SD_CMD_WRITE_SINGLE_BLOCK, WriteAddr + Offset, 0xFF);
    }
    /*!< Check if the SD acknowledged the write block command: R1 response (0x00: no errors) */
    if (SD_GetResponse(SD_RESPONSE_NO_ERROR))
    {
      return SD_RESPONSE_FAILURE;
    }
    /*!< Send dummy byte */
    SD_WriteByte(SD_DUMMY_BYTE);
    /*!< Send the data token to signify the start of the data */
    SD_WriteByte(SD_START_DATA_SINGLE_BLOCK_WRITE);
    /*!< Write the block data to SD : write count data by block */
#if (SPI_DMA==0)		
    for (i = 0; i < BlockSize; i++)
    {
      /*!< Send the pointed byte */
      SD_WriteByte(*pBuffer);
      /*!< Point to the next location where the byte read will be saved */
      pBuffer++;
    }
#else
		DMA1_Start_SPI_TX(pBuffer);
		pBuffer += 512;
#endif		
    /*!< Set next write address */
    Offset += 512;
    /*!< Put CRC bytes (not really needed by us, but required by SD) */
    SD_ReadByte();
    SD_ReadByte();
    /*!< Read data response */
    if (SD_GetDataResponse() == SD_DATA_OK)
    {
      /*!< Set response value to success */
      rvalue = SD_RESPONSE_NO_ERROR;
    }
    else
    {
      /*!< Set response value to failure */
      rvalue = SD_RESPONSE_FAILURE;
    }
  }
  /*!< SD chip select high */
  SD_CS_HIGH();
  /*!< Send dummy byte: 8 Clock pulses of delay */
  SD_WriteByte(SD_DUMMY_BYTE);
  /*!< Returns the reponse */
  return rvalue;
}

/**
  * @brief  Read the CSD card register.
  *         Reading the contents of the CSD register in SPI mode is a simple 
  *         read-block transaction.
  * @param  SD_csd: pointer on an SCD register structure
  * @retval The SD Response: 
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_GetCSDRegister(SD_CSD* SD_csd)
{
  uint32_t i = 0;
  SD_Error rvalue = SD_RESPONSE_FAILURE;
  uint8_t CSD_Tab[16];
//ouhs	uint32_t CardCapacity;
//ouhs  static uint32_t times = 0;

  /*!< SD chip select low */
  SD_CS_LOW();
  /*!< Send CMD9 (CSD register) or CMD10(CSD register) */
  SD_SendCmd(SD_CMD_SEND_CSD, 0, 0xFF);
  /*!< Wait for response in the R1 format (0x00 is no errors) */
  if (!SD_GetResponse(SD_RESPONSE_NO_ERROR))
  {
    if (!SD_GetResponse(SD_START_DATA_SINGLE_BLOCK_READ))
    {
      for (i = 0; i < 16; i++)
      {
        /*!< Store CSD register value on CSD_Tab */
        CSD_Tab[i] = SD_ReadByte();
/* ouhs       if(0 == times)
        {
          USART_printf("\n\r CSD_Tab[%d] 0x%X", i, CSD_Tab[i]);
        }
*/
      }
    }
    /*!< Get CRC bytes (not really needed by us, but required by SD) */
    SD_WriteByte(SD_DUMMY_BYTE);
    SD_WriteByte(SD_DUMMY_BYTE);
    /*!< Set response value to success */
    rvalue = SD_RESPONSE_NO_ERROR;
  }
  else
  {
     return rvalue;
  }
  /*!< SD chip select high */
  SD_CS_HIGH();
  /*!< Send dummy byte: 8 Clock pulses of delay */
  SD_WriteByte(SD_DUMMY_BYTE);

  /*!< Byte 0 */
  SD_csd->CSDStruct = (CSD_Tab[0] & 0xC0) >> 6;
  SD_csd->SysSpecVersion = (CSD_Tab[0] & 0x3C) >> 2;
  SD_csd->Reserved1 = CSD_Tab[0] & 0x03;

//ouhs  if(0 == times)
  {
    if(SD_csd->CSDStruct == 1)
    {
      //USART_printf("\n\r SDHC CSD Version 2.0 Acess AS SD High Capacity.");
      flag_SDHC = 1;
    }
	else
	{
      //USART_printf("\n\r SDSC CSD Version 1.0 Acess AS SD Standard Capacity.");	
      flag_SDHC = 0;
	}
  }

  /*!< Byte 1 */
  SD_csd->TAAC = CSD_Tab[1];

  /*!< Byte 2 */
  SD_csd->NSAC = CSD_Tab[2];

  /*!< Byte 3 */
  SD_csd->MaxBusClkFrec = CSD_Tab[3];

  /*!< Byte 4 */
  SD_csd->CardComdClasses = CSD_Tab[4] << 4;

  /*!< Byte 5 */
  SD_csd->CardComdClasses |= (CSD_Tab[5] & 0xF0) >> 4;
  SD_csd->RdBlockLen = CSD_Tab[5] & 0x0F;

  /*!< Byte 6 */
  SD_csd->PartBlockRead = (CSD_Tab[6] & 0x80) >> 7;
  SD_csd->WrBlockMisalign = (CSD_Tab[6] & 0x40) >> 6;
  SD_csd->RdBlockMisalign = (CSD_Tab[6] & 0x20) >> 5;
  SD_csd->DSRImpl = (CSD_Tab[6] & 0x10) >> 4;
  SD_csd->Reserved2 = 0; /*!< Reserved */

  if(flag_SDHC == 0)
  {
    SD_csd->DeviceSize = (CSD_Tab[6] & 0x03) << 10;
  
    /*!< Byte 7 */
    SD_csd->DeviceSize |= (CSD_Tab[7]) << 2;
  
    /*!< Byte 8 */
    SD_csd->DeviceSize |= (CSD_Tab[8] & 0xC0) >> 6;
  }
  else
  {
    SD_csd->DeviceSize = (CSD_Tab[7] & 0x3F) << 16;
    /* Byte 7 */
    SD_csd->DeviceSize += (CSD_Tab[8]) << 8;
    /* Byte 8 */
    SD_csd->DeviceSize += CSD_Tab[9] ;
  }    

  SD_csd->MaxRdCurrentVDDMin = (CSD_Tab[8] & 0x38) >> 3;
  SD_csd->MaxRdCurrentVDDMax = (CSD_Tab[8] & 0x07);

  /*!< Byte 9 */
  SD_csd->MaxWrCurrentVDDMin = (CSD_Tab[9] & 0xE0) >> 5;
  SD_csd->MaxWrCurrentVDDMax = (CSD_Tab[9] & 0x1C) >> 2;
  SD_csd->DeviceSizeMul = (CSD_Tab[9] & 0x03) << 1;
  /*!< Byte 10 */
  SD_csd->DeviceSizeMul |= (CSD_Tab[10] & 0x80) >> 7;
    
  SD_csd->EraseGrSize = (CSD_Tab[10] & 0x40) >> 6;
  SD_csd->EraseGrMul = (CSD_Tab[10] & 0x3F) << 1;

  /*!< Byte 11 */
  SD_csd->EraseGrMul |= (CSD_Tab[11] & 0x80) >> 7;
  SD_csd->WrProtectGrSize = (CSD_Tab[11] & 0x7F);

  /*!< Byte 12 */
  SD_csd->WrProtectGrEnable = (CSD_Tab[12] & 0x80) >> 7;
  SD_csd->ManDeflECC = (CSD_Tab[12] & 0x60) >> 5;
  SD_csd->WrSpeedFact = (CSD_Tab[12] & 0x1C) >> 2;
  SD_csd->MaxWrBlockLen = (CSD_Tab[12] & 0x03) << 2;

  /*!< Byte 13 */
  SD_csd->MaxWrBlockLen |= (CSD_Tab[13] & 0xC0) >> 6;
  SD_csd->WriteBlockPaPartial = (CSD_Tab[13] & 0x20) >> 5;
  SD_csd->Reserved3 = 0;
  SD_csd->ContentProtectAppli = (CSD_Tab[13] & 0x01);

  /*!< Byte 14 */
  SD_csd->FileFormatGrouop = (CSD_Tab[14] & 0x80) >> 7;
  SD_csd->CopyFlag = (CSD_Tab[14] & 0x40) >> 6;
  SD_csd->PermWrProtect = (CSD_Tab[14] & 0x20) >> 5;
  SD_csd->TempWrProtect = (CSD_Tab[14] & 0x10) >> 4;
  SD_csd->FileFormat = (CSD_Tab[14] & 0x0C) >> 2;
  SD_csd->ECC = (CSD_Tab[14] & 0x03);

  /*!< Byte 15 */
  SD_csd->CSD_CRC = (CSD_Tab[15] & 0xFE) >> 1;
  SD_csd->Reserved4 = 1;

//ouhs  if(times < 3)
  {
    if(flag_SDHC ==1)
    {
	  /* SDHC memory capacity = (C_SIZE+1) * 512K byte */
      SD_csd->DeviceSizeMul = 8 ;
    }
/*ouhs
    CardCapacity = (SD_csd->DeviceSize + 1);
    CardCapacity *= (1 << (SD_csd->DeviceSizeMul + 2));
    CardCapacity *= (1<<SD_csd->RdBlockLen);
    USART_printf("\n\r SD CardBlockSize %d CardCapacity %d M.", (1<<SD_csd->RdBlockLen), CardCapacity / 1024 / 1024);

	if(times == 0)
  {
		uint8_t str[20];
		sprintf((char *)&str[0], " SD Card Size %dM. ", CardCapacity / 1024 / 1024);
		//LCD_DisplayStringLine(LCD_LINE_0, str);
	}
	times++;
*/
  }
  /*!< Return the reponse */
  return rvalue;
}

/**
  * @brief  Read the CID card register.
  *         Reading the contents of the CID register in SPI mode is a simple 
  *         read-block transaction.
  * @param  SD_cid: pointer on an CID register structure
  * @retval The SD Response: 
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_GetCIDRegister(SD_CID* SD_cid)
{
  uint32_t i = 0;
  SD_Error rvalue = SD_RESPONSE_FAILURE;
  uint8_t CID_Tab[16];
  
  /*!< SD chip select low */
  SD_CS_LOW();
  
  /*!< Send CMD10 (CID register) */
  SD_SendCmd(SD_CMD_SEND_CID, 0, 0xFF);
  
  /*!< Wait for response in the R1 format (0x00 is no errors) */
  if (!SD_GetResponse(SD_RESPONSE_NO_ERROR))
  {
    if (!SD_GetResponse(SD_START_DATA_SINGLE_BLOCK_READ))
    {
      /*!< Store CID register value on CID_Tab */
      for (i = 0; i < 16; i++)
      {
        CID_Tab[i] = SD_ReadByte();
      }
    }
    /*!< Get CRC bytes (not really needed by us, but required by SD) */
    SD_WriteByte(SD_DUMMY_BYTE);
    SD_WriteByte(SD_DUMMY_BYTE);
    /*!< Set response value to success */
    rvalue = SD_RESPONSE_NO_ERROR;
  }
  /*!< SD chip select high */
  SD_CS_HIGH();
  /*!< Send dummy byte: 8 Clock pulses of delay */
  SD_WriteByte(SD_DUMMY_BYTE);

  /*!< Byte 0 */
  SD_cid->ManufacturerID = CID_Tab[0];

  /*!< Byte 1 */
  SD_cid->OEM_AppliID = CID_Tab[1] << 8;

  /*!< Byte 2 */
  SD_cid->OEM_AppliID |= CID_Tab[2];

  /*!< Byte 3 */
  SD_cid->ProdName1 = CID_Tab[3] << 24;

  /*!< Byte 4 */
  SD_cid->ProdName1 |= CID_Tab[4] << 16;

  /*!< Byte 5 */
  SD_cid->ProdName1 |= CID_Tab[5] << 8;

  /*!< Byte 6 */
  SD_cid->ProdName1 |= CID_Tab[6];

  /*!< Byte 7 */
  SD_cid->ProdName2 = CID_Tab[7];

  /*!< Byte 8 */
  SD_cid->ProdRev = CID_Tab[8];

  /*!< Byte 9 */
  SD_cid->ProdSN = CID_Tab[9] << 24;

  /*!< Byte 10 */
  SD_cid->ProdSN |= CID_Tab[10] << 16;

  /*!< Byte 11 */
  SD_cid->ProdSN |= CID_Tab[11] << 8;

  /*!< Byte 12 */
  SD_cid->ProdSN |= CID_Tab[12];

  /*!< Byte 13 */
  SD_cid->Reserved1 |= (CID_Tab[13] & 0xF0) >> 4;
  SD_cid->ManufactDate = (CID_Tab[13] & 0x0F) << 8;

  /*!< Byte 14 */
  SD_cid->ManufactDate |= CID_Tab[14];

  /*!< Byte 15 */
  SD_cid->CID_CRC = (CID_Tab[15] & 0xFE) >> 1;
  SD_cid->Reserved2 = 1;

  /*!< Return the reponse */
  return rvalue;
}

/**
  * @brief  Send 5 bytes command to the SD card.
  * @param  Cmd: The user expected command to send to SD card.
  * @param  Arg: The command argument.
  * @param  Crc: The CRC.
  * @retval None
  */
void SD_SendCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc)
{
  uint32_t i = 0x00;
  
  uint8_t Frame[6];

  Frame[0] = (Cmd | 0x40); /*!< Construct byte 1 */
  
  Frame[1] = (uint8_t)(Arg >> 24); /*!< Construct byte 2 */
  
  Frame[2] = (uint8_t)(Arg >> 16); /*!< Construct byte 3 */
  
  Frame[3] = (uint8_t)(Arg >> 8); /*!< Construct byte 4 */
  
  Frame[4] = (uint8_t)(Arg); /*!< Construct byte 5 */
  
  Frame[5] = (Crc); /*!< Construct CRC: byte 6 */
  
  for (i = 0; i < 6; i++)
  {
    SD_WriteByte(Frame[i]); /*!< Send the Cmd bytes */
  }
}

/**
  * @brief  Get SD card data response.
  * @param  None
  * @retval The SD status: Read data response xxx0<status>1
  *         - status 010: Data accecpted
  *         - status 101: Data rejected due to a crc error
  *         - status 110: Data rejected due to a Write error.
  *         - status 111: Data rejected due to other error.
  */
uint8_t SD_GetDataResponse(void)
{
  uint32_t i = 0;
  uint8_t response, rvalue;

  while (i <= 64)
  {
    /*!< Read resonse */
    response = SD_ReadByte();
    /*!< Mask unused bits */
    response &= 0x1F;
    switch (response)
    {
      case SD_DATA_OK:
      {
        rvalue = SD_DATA_OK;
        break;
      }
      case SD_DATA_CRC_ERROR:
        return SD_DATA_CRC_ERROR;
      case SD_DATA_WRITE_ERROR:
        return SD_DATA_WRITE_ERROR;
      default:
      {
        rvalue = SD_DATA_OTHER_ERROR;
        break;
      }
    }
    /*!< Exit loop in case of data ok */
    if (rvalue == SD_DATA_OK)
      break;
    /*!< Increment loop counter */
    i++;
  }

  /*!< Wait null data */
  while (SD_ReadByte() == 0);

  /*!< Return response */
  return response;
}

/**
  * @brief  Returns the SD response.
  * @param  None
  * @retval The SD Response: 
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_GetResponse(uint8_t Response)
{
  uint32_t Count = 0x8000;  //ouhs 0xFFF

  /*!< Check if response is got or a timeout is happen */
  while ((SD_ReadByte() != Response) && Count)
  {
    Count--;
  }
  if (Count == 0)
  {
    /*!< After time out */
    return SD_RESPONSE_FAILURE;
  }
  else
  {
    /*!< Right response got */
    return SD_RESPONSE_NO_ERROR;
  }
}

/**
  * @brief  Returns the SD status.
  * @param  None
  * @retval The SD status.
  */
uint16_t SD_GetStatus(void)
{
  uint16_t Status = 0;

  /*!< SD chip select low */
  SD_CS_LOW();

  /*!< Send CMD13 (SD_SEND_STATUS) to get SD status */
  SD_SendCmd(SD_CMD_SEND_STATUS, 0, 0xFF);

  Status = SD_ReadByte();
  Status |= (uint16_t)(SD_ReadByte() << 8);

  /*!< SD chip select high */
  SD_CS_HIGH();

  /*!< Send dummy byte 0xFF */
  SD_WriteByte(SD_DUMMY_BYTE);

  return Status;
}

/**
  * @brief  Put SD in Idle state.
  * @param  None
  * @retval The SD Response: 
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_GoIdleState(void)
{
  uint16_t TimeOut;
  uint8_t r1;
  SD_Error Status = SD_RESPONSE_NO_ERROR;
  uint16_t n2,n;
  /*!< SD chip select low */
  SD_CS_LOW();
  
  /*!< Send CMD0 (SD_CMD_GO_IDLE_STATE) to put SD in SPI mode */
  SD_SendCmd(SD_CMD_GO_IDLE_STATE, 0, 0x95);
  
  /*!< Wait for In Idle State Response (R1 Format) equal to 0x01 */
  if (SD_GetResponse(SD_IN_IDLE_STATE))
  {
    /*!< No Idle State Response: return response failue */
		//USART_printf("\n\r SD_RESPONSE_FAILURE: Send CMD0 (SD_CMD_GO_IDLE_STATE) to put SD in SPI mode");
		//USART_printf("\n\r No SD card detect.");
    return SD_RESPONSE_FAILURE;
  }

  
  SD_SendCmd(8, 0x1AA, 0x87); /*check version*/
  /*!< Check if response is got or a timeout is happen */
  TimeOut = 200;
  while (((r1 = SD_ReadByte()) == 0xFF) && TimeOut)
  {
    TimeOut--;
  }

  if(r1 == 0x05) // not SDHC
  {
    //flag_SDHC = 0;
    //USART_printf("\r\n  this is STD_CAPACITY_SD_CARD!\n");
    
    TimeOut = 0;
    /*----------Activates the card initialization process-----------*/
    do
    {
      /*!< SD chip select high */
      SD_CS_HIGH();
      
      /*!< Send Dummy byte 0xFF */
      SD_WriteByte(SD_DUMMY_BYTE);
      
      /*!< SD chip select low */
      SD_CS_LOW();
      
      /*!< Send CMD1 (Activates the card process) until response equal to 0x0 */
      SD_SendCmd(SD_CMD_SEND_OP_COND, 0, 0xFF);
      /*!< Wait for no error Response (R1 Format) equal to 0x00 */
      TimeOut++;
    
      if(TimeOut == 0x00F0)
      {
        break;
      }
  	
  	Status = SD_GetResponse(SD_RESPONSE_NO_ERROR);	
    }
    while (Status);
    
    /*!< SD chip select high */
    SD_CS_HIGH();
    
    /*!< Send dummy byte 0xFF */
    SD_WriteByte(SD_DUMMY_BYTE);    
  }
  else
  {
    //if(r1 != 1)
    //{
    //    USART_printf("\n\r SD card r1 is %X.\n\r", r1);
    //}
    //USART_printf("\r\n this is SDHC: HIGH_CAPACITY_SD_CARD!\n");
    r1 = 1;
    /* Send Dummy byte 0xFF */
    for(n=0; n<5; n++)
    {
      SD_ReadByte();
    }
  
    /* MSD chip select high */
    SD_CS_HIGH();
    SD_WriteByte(SD_DUMMY_BYTE);
    /* MSD chip select low */
    SD_CS_LOW();
    SD_WriteByte(SD_DUMMY_BYTE);
    SD_WriteByte(SD_DUMMY_BYTE);
    n=0xff;
  
    do
    {
      SD_SendCmd(55, 0, 0xFF);
      for(n2=0; n2<0x08;n2++)
      {
       r1= SD_ReadByte();
       if(r1 !=1)n=0;
      }
      SD_SendCmd(41, 0x40000000, 0);
  
      for(n2=0; n2<0xff;n2++)
      {
       r1= SD_ReadByte();
       if(r1 ==0)break;
      }
      n--;
    }while((r1!=0)&&(n>0));
    if(n==0)
    {
      Status =  SD_RESPONSE_FAILURE;
      //USART_printf("\r\n Activates the SDHC card falled!\n");
    }
    else
    {
#if 0	//ouhs20171103	会导致有些SD卡对CMD命令无响应 如ADATA的4G卡
      SD_SendCmd(58, 0, 0);
      for(n=0;n<5;n++)
      {
				r1 = SD_ReadByte();
				//USART_printf("\n\r 58 SD card r1 is %X.\n\r", r1);
      }
#endif			
			flag_SDHC = 1;
      //USART_printf("\r\n Activates the SDHC card sucessed!\n");
      Status =  SD_RESPONSE_NO_ERROR;
     }
  }
  
  
  /*
  if((Status != SD_RESPONSE_NO_ERROR))
  {
    //flag_SDHC = 1;
    USART_printf("\n\r Maybe this is a SDHC(SD High Capacity cards).");
    //break;
  }
  else
  {
    USART_printf("\n\r SD cards inint Done.");
  }  
  */
    
  return Status;//SD_RESPONSE_NO_ERROR;
}

/**
  * @brief  Write a byte on the SD.
  * @param  Data: byte to send.
  * @retval None
  */
uint8_t SD_WriteByte(uint8_t Data)
{
#if 0	
  /*!< Wait until the transmit buffer is empty */
  while(SPI_I2S_GetFlagStatus(SD_SPI, SPI_I2S_FLAG_TXE) == RESET)
  {
  }
  
  /*!< Send the byte */
  SPI_I2S_SendData(SD_SPI, Data);
  
  /*!< Wait to receive a byte*/
  while(SPI_I2S_GetFlagStatus(SD_SPI, SPI_I2S_FLAG_RXNE) == RESET)
  {
  }
  
  /*!< Return the byte read from the SPI bus */ 
  return SPI_I2S_ReceiveData(SD_SPI);
#else
	
	while ((SD_SPI->SR & SPI_I2S_FLAG_TXE) == RESET);
	SD_SPI->DR = Data;
	while ((SD_SPI->SR & SPI_I2S_FLAG_RXNE) == RESET);
	return SD_SPI->DR;
	
#endif	
}

/**
  * @brief  Read a byte from the SD.
  * @param  None
  * @retval The received byte.
  */
uint8_t SD_ReadByte(void)
{
#if 0	
  uint8_t Data = 0;
  
  /*!< Wait until the transmit buffer is empty */
  while (SPI_I2S_GetFlagStatus(SD_SPI, SPI_I2S_FLAG_TXE) == RESET)
  {
  }
  /*!< Send the byte */
  SPI_I2S_SendData(SD_SPI, SD_DUMMY_BYTE);

  /*!< Wait until a data is received */
  while (SPI_I2S_GetFlagStatus(SD_SPI, SPI_I2S_FLAG_RXNE) == RESET)
  {
  }
  /*!< Get the received data */
  Data = SPI_I2S_ReceiveData(SD_SPI);

  /*!< Return the shifted data */
  return Data;
	
#else
	return (SD_WriteByte(SD_DUMMY_BYTE));	
#endif	
}

/**
  * @}
  */


/**
  * @brief  DeInitializes the SD/SD communication.
  * @param  None
  * @retval None
  */
void SD_LowLevel_DeInit(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  SPI_Cmd(SD_SPI, DISABLE); /*!< SD_SPI disable */
  SPI_I2S_DeInit(SD_SPI);   /*!< DeInitializes the SD_SPI */
  
  /*!< SD_SPI Periph clock disable */
  RCC_APB1PeriphClockCmd(SD_SPI_CLK, DISABLE);
  
  /*!< Configure SD_SPI pins: SCK */
  GPIO_InitStructure.GPIO_Pin = SD_SPI_SCK_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(SD_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure SD_SPI pins: MISO */
  GPIO_InitStructure.GPIO_Pin = SD_SPI_MISO_PIN;
  GPIO_Init(SD_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure SD_SPI pins: MOSI */
  GPIO_InitStructure.GPIO_Pin = SD_SPI_MOSI_PIN;
  GPIO_Init(SD_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure SD_SPI_CS_PIN pin: SD Card CS pin */
  GPIO_InitStructure.GPIO_Pin = SD_CS_PIN_1ST;
  GPIO_Init(SD_CS_GPIO_PORT_1ST, &GPIO_InitStructure);

  /*!< Configure SD_SPI_CS_PIN pin: SD Card CS pin */
  GPIO_InitStructure.GPIO_Pin = SD_CS_PIN_2ND;
  GPIO_Init(SD_CS_GPIO_PORT_2ND, &GPIO_InitStructure);

  /*!< Configure SD_SPI_DETECT_PIN pin: SD Card detect pin */
  GPIO_InitStructure.GPIO_Pin = SD_DETECT_PIN_1ST;
  GPIO_Init(SD_DETECT_GPIO_PORT_1ST, &GPIO_InitStructure);
}


/**
  * @brief  Initializes the SD_SPI and CS pins.
  * @param  None
  * @retval None
  */
void SD_LowLevel_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  SPI_InitTypeDef   SPI_InitStructure;

  /*!< SD_SPI_CS_GPIO, SD_SPI_MOSI_GPIO, SD_SPI_MISO_GPIO, SD_SPI_DETECT_GPIO 
       and SD_SPI_SCK_GPIO Periph clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOG, ENABLE);

  /*!< SD_SPI Periph clock enable */
  RCC_APB1PeriphClockCmd(SD_SPI_CLK, ENABLE);
	
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SPI3);	  // SPI3_SCK
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_SPI3);	  // SPI3_MISO
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SPI3);	  // SPI3_MOSI
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;//GPIO_PuPd_DOWN;
	
  /*!< Configure SD_SPI pins: SCK */
  GPIO_InitStructure.GPIO_Pin = SD_SPI_SCK_PIN;
  GPIO_Init(SD_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure SD_SPI pins: MISO */
  GPIO_InitStructure.GPIO_Pin = SD_SPI_MISO_PIN;
  GPIO_Init(SD_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure SD_SPI pins: MOSI */
  GPIO_InitStructure.GPIO_Pin = SD_SPI_MOSI_PIN;
  GPIO_Init(SD_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);
  
  /*!< Configure sFLASH Card CS pin in output pushpull mode ********************/
  GPIO_InitStructure.GPIO_Pin = SD_CS_PIN_1ST;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(SD_CS_GPIO_PORT_1ST, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = SD_CS_PIN_2ND; 
  GPIO_Init(SD_CS_GPIO_PORT_2ND, &GPIO_InitStructure);
  
  /*!< Configure SD_SPI_DETECT_PIN pin: SD Card detect pin */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = SD_DETECT_PIN_1ST;
  GPIO_Init(SD_DETECT_GPIO_PORT_1ST, &GPIO_InitStructure);

#ifdef CASE_MCR30
  GPIO_InitStructure.GPIO_Pin = SD_DETECT_PIN_2ND;
  GPIO_Init(SD_DETECT_GPIO_PORT_2ND, &GPIO_InitStructure);
#endif

  SD_CS_HIGH();
  SD_CS_HIGH_2ND();
	

  /*!< SD_SPI Config */
	// SPI1 配置:全双工、主机、8位帧、SCK上升沿采样(SCK极性为1、相位为1)、CS由软件控制、
	//			 SCK最大速度APB2/2=15MHz、高位在前
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SD_SPI, &SPI_InitStructure);
#if SPI_DMA==1 
	DMA1_Init();
	NVIC_SPIDMAonfiguration();
#endif 
	SPI_Cmd(SD_SPI, ENABLE); /*!< SD_SPI enable */
}

//设置SPI速度 
void SD_SPI_SetSpeedLow(void)
{
	SPI_InitTypeDef   SPI_InitStructure;
	
	SPI_Cmd(SD_SPI, DISABLE);
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;//SPI_BaudRatePrescaler_8;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SD_SPI, &SPI_InitStructure);
  
  SPI_Cmd(SD_SPI, ENABLE); /*!< SD_SPI enable */
}

void SD_SPI_SetSpeedHigh(void)
{
	SPI_InitTypeDef   SPI_InitStructure;
	
	SPI_Cmd(SD_SPI, DISABLE);
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;//SPI_BaudRatePrescaler_4;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SD_SPI, &SPI_InitStructure);
  
  SPI_Cmd(SD_SPI, ENABLE); /*!< SD_SPI enable */
}

void NVIC_SPIDMAonfiguration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure the NVIC Preemption Priority Bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  NVIC_InitStructure.NVIC_IRQChannel = SPIx_DMA_TX_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = SPIx_DMA_RX_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; 
  NVIC_Init(&NVIC_InitStructure);
}


/**
  * @brief  This function handles DMA Rx interrupt request.
  * @param  None
  * @retval None
  */
void SPIx_DMA_RX_IRQHandler(void)
{
  if(DMA_GetITStatus(SPIx_RX_DMA_STREAM, SPIx_RX_DMA_IT_TEIF) != RESET)
	{
		//xputs("\r\nSD SPI DMA RX Error......");
		DMA_ClearITPendingBit(SPIx_RX_DMA_STREAM, SPIx_RX_DMA_IT_TEIF);
	}
}

/**
  * @brief  This function handles DMA Tx interrupt request.
  * @param  None
  * @retval None
  */
void SPIx_DMA_TX_IRQHandler(void)
{
  if(DMA_GetITStatus(SPIx_TX_DMA_STREAM, SPIx_TX_DMA_IT_TEIF) != RESET)
	{
		//xputs("\r\nSD SPI DMA TX Error......");
		DMA_ClearITPendingBit(SPIx_TX_DMA_STREAM, SPIx_TX_DMA_IT_TEIF);
	}
}
/**
  * @}
  */

void DMA1_Init(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	
  /* Enable DMA clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);	
	
	/* DMA configuration -------------------------------------------------------*/
  /* Deinitialize DMA Streams */
  DMA_DeInit(SPIx_TX_DMA_STREAM);
  DMA_DeInit(SPIx_RX_DMA_STREAM);
  
  /* Configure DMA Initialization Structure */

  DMA_InitStructure.DMA_BufferSize = SD_BLOCK_SIZE ;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable ;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull ;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single ;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t) (&(SD_SPI->DR)) ;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;

  /* Configure TX DMA */  
  DMA_InitStructure.DMA_Channel = SPIx_TX_DMA_CHANNEL ;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral ;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;//DMA_Priority_Low;//
  DMA_Init(SPIx_TX_DMA_STREAM, &DMA_InitStructure);
	
  /* Configure RX DMA */
  DMA_InitStructure.DMA_Channel = SPIx_RX_DMA_CHANNEL ;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory ;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_Init(SPIx_RX_DMA_STREAM, &DMA_InitStructure);	

	DMA_ITConfig(SPIx_TX_DMA_STREAM, DMA_IT_TE, ENABLE);
	DMA_ITConfig(SPIx_RX_DMA_STREAM, DMA_IT_TE, ENABLE);

  /* Enable DMA SPI TX Stream */
  //DMA_Cmd(SPIx_TX_DMA_STREAM,ENABLE);

  /* Enable DMA SPI RX Stream */
  //DMA_Cmd(SPIx_RX_DMA_STREAM,ENABLE);

  /* Enable SPI DMA TX Requsts */
  SPI_I2S_DMACmd(SD_SPI, SPI_I2S_DMAReq_Tx, ENABLE);

  /* Enable SPI DMA RX Requsts */
  SPI_I2S_DMACmd(SD_SPI, SPI_I2S_DMAReq_Rx, ENABLE);	
	
}



//从SD卡读取一个数据包的内容时候启动DMA传输
uint8_t unused_data;
uint8_t DMA1_Start_SPI_RX(uint8_t *buffer,uint16_t len)
{
	uint8_t temp=0xff;
	
	SPIx_RX_DMA_STREAM->NDTR = len; 
	SPIx_RX_DMA_STREAM->M0AR = (uint32_t)buffer; 

	/*SPI作为主机进行数据接收时必须要主动产生时钟，因此此处必须有TX DMA通道的配合*/
	SPIx_TX_DMA_STREAM->NDTR = len; 
	SPIx_TX_DMA_STREAM->M0AR = (uint32_t)&temp; 
	SPIx_TX_DMA_STREAM->CR &= ~DMA_MemoryInc_Enable; 

	DMA_Cmd(SPIx_RX_DMA_STREAM,ENABLE); 
	DMA_Cmd(SPIx_TX_DMA_STREAM,ENABLE); 
	
	while(DMA_GetFlagStatus(SPIx_RX_DMA_STREAM, SPIx_RX_DMA_FLAG_TCIF)==RESET); 
	while(/*((SD_SPI)->SR & (SPI_SR_TXE | SPI_SR_RXNE)) == 0 ||*/ ((SD_SPI)->SR & SPI_SR_BSY));
	unused_data = SD_SPI->DR;

	DMA_ClearFlag(SPIx_RX_DMA_STREAM,SPIx_RX_DMA_FLAG_TCIF);  
	DMA_ClearFlag(SPIx_TX_DMA_STREAM,SPIx_TX_DMA_FLAG_TCIF);


	DMA_Cmd(SPIx_RX_DMA_STREAM,DISABLE);
	DMA_Cmd(SPIx_TX_DMA_STREAM,DISABLE);

	SPIx_TX_DMA_STREAM->CR |= DMA_MemoryInc_Enable; //将DMA通道恢复为内存地址自增方式
	return 0;
}

//向SD卡写入一个数据包的内容 512字节 启动DMA传输
uint8_t DMA1_Start_SPI_TX(uint8_t *buffer)
{	
	SPIx_TX_DMA_STREAM->NDTR = 512; 
	SPIx_TX_DMA_STREAM->M0AR = (uint32_t)buffer; 

	DMA_Cmd(SPIx_TX_DMA_STREAM,ENABLE); 

	while(DMA_GetFlagStatus(SPIx_TX_DMA_STREAM, SPIx_TX_DMA_FLAG_TCIF)==RESET); 
	while(/*((SD_SPI)->SR & (SPI_SR_TXE | SPI_SR_RXNE)) == 0 ||*/ ((SD_SPI)->SR & SPI_SR_BSY));
	unused_data = SD_SPI->DR;
	
	DMA_ClearFlag(SPIx_TX_DMA_STREAM,SPIx_TX_DMA_FLAG_TCIF); 
	
	DMA_Cmd(SPIx_TX_DMA_STREAM,DISABLE); 
	
	return 0;
}


/**
  * @}
  */

/**
  * @}
  */  

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
