
/* Includes ------------------------------------------------------------------*/
#include "king.h"                               /* Kingtron library and I/O routines */
#include "appl.h"
#include "include.h"
#include "monitor.h"
#include "nor_flash.h"
#include "test.h"
#include "spi_m25p80.h"
#include "SDDisk_Utilities.h"
#include "nand.h"
#include "ftl.h"

#include "sim800.h"

#include <stdio.h>  //ouhs 20140818
#include <string.h>  //ouhs 20140818
#if defined(CASE_ETHERNET)
#include "netconf.h"

#endif

#if defined(CASE_ASCIIDISPLAY)
#include "lcd_160x2.h"
#endif

#if _USE_LFN == 3
#include "malloc.h"	
#endif





//#include "znfat.h"

//*******************************SRAM_DEBUG**************************************//

#if defined   (__CC_ARM)      /* ARM Compiler */
  uint8_t TxBuffer[BUFFER_SIZE] __attribute__ ((aligned (4)));
  uint8_t RxBuffer[BUFFER_SIZE] __attribute__ ((aligned (4)));
#elif defined (__ICCARM__)    /* IAR Compiler */
  #pragma data_alignment=4
  uint8_t TxBuffer[BUFFER_SIZE], RxBuffer[BUFFER_SIZE];
#endif

#ifdef 	SRAM_DEBUG

#define WRITE_READ_ADDR    (0x80000-0x1fff)		 // SRAM:0~0x80000
#define MAX_ADDR    (FiscalMem_SPI_MAXADDR+1)


unsigned long RamOffSet;
BYTE    SysBuf[128];            /*    System buffer max 128 positions     */
APPLICATION_SAVE    ApplVar;


/**************************************************************************/
/**************************************************************************/
BYTE ApplVar_Restored=0x5a;   //=0x5a,ApplVar中的变量数据被恢复

void Test_Poweroff(void);
void Save_ApplRam()
{
	USART_printf("Power off...");
	Test_Poweroff();
}

BYTE SummerTimeEnabled()
{
	return true;
}

void Save_Config()
{

}
void ClsState1()
{}
void DispStrXY(CONSTBYTE *str ,BYTE x,BYTE y)
{}

void DisplayChar(unsigned char ch)
{}
BYTE GotoXY(BYTE x,BYTE y)
{
	return 0;  //ouhs 20140815
}

short     KbHit(void)
{
}

const char MsgSRAMNOTFOUND[];

/**************************************************************************/
/**************************************************************************/

/* Private function prototypes -----------------------------------------------*/
void Fill_Buffer(uint8_t *pBuffer, uint16_t BufferLenght, uint32_t Offset);

void Save_ConfigVar(void) //保护参数
{
    xputs("Power off!");
}

extern void ReadRam( char *buf,DWORD len);
extern void WriteRam( char *buf, DWORD len);
void TIM6_Init_Count(void)
{//by ouhs test 用于测试程序执行时间
    uint16_t PrescalerValue = 0;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	/* Clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	/* Compute the prescaler value */
	// 定时器时钟为:60MHz/PrescalerValue  //1MHz
    PrescalerValue = (uint16_t) ((SystemCoreClock / 2) / 1000000) - 1; 
	/* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = 99999; //100ms一次

    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
	/* Prescaler configuration */
    TIM_PrescalerConfig(TIM6, PrescalerValue, TIM_PSCReloadMode_Immediate);

	/* Enable counter */
	//TIM_Cmd(TIM6, ENABLE);
}

uint8_t Test_SRAM(void)
{
	uint32_t i;
	uint32_t sram_size, buf_size;
	
	xprintf("Test_SRAM...\r\n");
#if defined(CASE_ECR99) ||defined(CASE_MCR30)
	sram_size = 0x100000;//1MB
#else
	sram_size = 0x80000;//512KB
#endif	
	buf_size = 4096;	
	RamOffSet = 0;
	
	DisplayTime(0);
	for(i=0; i<(sram_size/buf_size);i++)
	{
		Fill_Buffer(TxBuffer, buf_size, i);
		WriteRam(TxBuffer, buf_size);
		RamOffSet -= buf_size;
		ReadRam(RxBuffer, buf_size);
		if(Buffercmp(TxBuffer, RxBuffer, buf_size))
		{
			xprintf("SRAM ERROR: 0x%x", i*buf_size);
			return;
		}
		xprintf(".");
	}
	xprintf("Test OK!\r\n");
	DisplayTime(1);
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Fills buffer with user predefined data.
  * @param  pBuffer: pointer on the buffer to fill
  * @param  BufferLenght: size of the buffer to fill
  * @param  Offset: first value to fill on the buffer
  * @retval None
  */
void Fill_Buffer(uint8_t *pBuffer, uint16_t BufferLenght, uint32_t Offset)
{
  uint16_t IndexTmp = 0;

  /* Put in global buffer different values */
  for (IndexTmp = 0; IndexTmp < BufferLenght; IndexTmp++ )
  {
    pBuffer[IndexTmp] = IndexTmp + Offset;
  }
}
//return:0 or
//       >0 error at returen-1
uint32_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint32_t BufferLength)
{
	__IO uint32_t WriteReadStatus = 0;
	uint32_t Index;

	for (Index = 0x00; (Index < BufferLength) && (WriteReadStatus == 0); Index++)
	{
		//USART_printf("\r\nTxBuffer[%d]=0x%x---RxBuffer[%d]=0x%x \r\n",Index,pBuffer1[Index],Index, pBuffer2[Index]);
		if (pBuffer1[Index] != pBuffer2[Index])
		{
			WriteReadStatus++;
            USART_printf("\r\n ERROR:TxBuffer[%d]=0x%x---RxBuffer[%d]=0x%x \r\n",Index,pBuffer1[Index],Index, pBuffer2[Index]);
            return Index+1;
		}
	}

	if (WriteReadStatus == 0)
	{
		/* OK */
		//USART_printf("\r\n Write & Read OK:TxBuffer[%d]=0x%x---RxBuffer[%d]=0x%x \r\n",BufferLength-1,pBuffer1[BufferLength-1],BufferLength-1, pBuffer2[BufferLength-1]);
		//USART_printf(" Test OK \r\n");
    return 0;
	}

	return 0;
}


#endif

//*******************************SPIFLASH_DEBUG**********************************//
/************************** PRIVATE DEFINITIONS *********************/
#define TEST_SIZE 64


uint32_t    ChipID = 0;

/*-------------------------MAIN FUNCTION------------------------------*/
/* CCR 20110824                                                         */
/* Display help messages                                                */
/*-----------------------------------------------------------------------*/
void DisplyFlashHelp()
{
    xprintf("\nTest serial flash SST25VF016B(SIZE:%d Bytes). \n",MAX_ADDR);
    xputs(" ? - Help for Flash. \n");
    xputs(" q - Quit.\n");
    xputs(" fi - Initialize SPI for Flash.\n");
    xprintf(" fw <addr:%d> <string:120> - Auto-Erase & then write the string. \n",MAX_ADDR-1);
    xprintf(" fc <addr:%d> <string:120> - Check the data of write by fW. \n",MAX_ADDR-1);
    xprintf(" fr <addr:%d> <len> - read data from Flash.\n",MAX_ADDR-1);
    xprintf(" fe <secF:%d> <secT> - Erase sections  from secF to secT. \n",FiscalMem_SEC_MAX);
    xputs(" fE - Erase chip.\n");

    xprintf(" fW <addr:%d> <len> <val> - write data to Flash. \n",MAX_ADDR-1);
    xprintf(" fC <addr:%d> <len> <val> - Check the data of write by fw. \n",MAX_ADDR-1);
    xputs(" fl - Load Chinese into flash.\n");
//    xputs(" ff - Send Flash data to host.\n");

}


/* With ARM and GHS toolsets, the entry point is main() - this will
   allow the linker to generate wrapper code to setup stacks, allocate
   heap area, and initialize and copy code and data segments. For GNU
   toolsets, the entry point is through __start() in the crt0_gnu.asm
   file, and that startup code will setup stacks and data */
void Test_SPIFlash(void)
{
    uint8_t rc;
    char *ptr;
    uint32_t  numMax, blkSize,p1,p2,p3,i,timeRD;

		uint8_t cnt;
	
	if(FMDetect()==SUCCESS)
	{	

    DisplyFlashHelp();



    for (;;)
    {
       // xputs("FLASH>");
        ptr = ConsoleLine;
        ConsoleLine[0]=0;
        get_line(ptr, sizeof(ConsoleLine));


        switch (*ptr++)
        {
        case '?':
            DisplyFlashHelp();
            break;
        case 'q':
            return ;

        case 'f' :
            switch (*ptr++)
            {
            case 'i' :  /* fi  - Initialize USB HOST device */

                ChipID = FiscalMem_ReadID();//ChipID = SPI_FLASH_ReadID();//

                xprintf("Manufactory_ID is:%x\n",ChipID);

								ChipID = FiscalMem_ReadRES();//sFlash_ReadRES();//
								xprintf("RES is:%x\n",ChipID);

                break;


            case 'r' :  /* fr <addr> <len> - read file */
                if (!xatoi(&ptr, &p1) || p1>=MAX_ADDR)
                {
                    xprintf("Input the address  for read.\n");
                    break;
                }
                if (!xatoi(&ptr, &p2) || (p1+p2)>MAX_ADDR)
                {
                    xprintf("Input the length for read.\n");
                    break;
                }
                numMax = blkSize = p3 = p2;
                if (blkSize>MAX_BUFFER_SIZE)
                {
                    blkSize = MAX_BUFFER_SIZE;
                }
                timeRD = 0;
                while (p2 >0 )
                {
									//if (SPI_FLASH_BufferRead(UserBuffer,p1, blkSize)==ERROR)
										if (FiscalMem_ReadBuffer(UserBuffer,p1, blkSize)==ERROR)
                    {
                        xprintf("Error at 0x%x when read.\n",p1);
                        break;
                    }

                    for (i=0;i<blkSize;i++)
                    {
                        xputc(UserBuffer[i]);
                    }
                    p1 += blkSize;//Totals of the read
                    p2 -= blkSize;
                    p3 -= blkSize;

                    if (blkSize>p3)
                    {
                        blkSize = p3;
                    }
                 }
               //  xprintf("\n<---%lu bytes read in  %lu m.seconds.\n", numMax, timeRD );

                break;

            case 'W' :  /* fw <addr> <len> <val> - write file */
								DisplayTime(0); //ouhs20170918
                if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)
                    || p1>=MAX_ADDR ||(p1+p2)>MAX_ADDR
                    )
                {
                    xprintf("Input the address,length , val for write.\n");
                    break;
                }
                xmemset(UserBuffer, p3, MAX_BUFFER_SIZE);

                numMax = blkSize = p3 = p2;

                if (blkSize>MAX_BUFFER_SIZE)
                {
                    blkSize = MAX_BUFFER_SIZE;
                }
                p2 += p1;
                timeRD = 0;

								cnt=1;
                while (p1<p2)
                {
                    if (FiscalMem_WriteBuffer(UserBuffer, p1, blkSize)==ERROR)
										//if (SPI_FLASH_BufferWrite(UserBuffer, p1, blkSize)==ERROR)
                        break;

                    p1+=blkSize;
                    p3-=blkSize;
                    if (blkSize>p3)
                    {
                        blkSize = p3;
                    }
                    xputc('.');
                }
                xprintf("\n<---%lu bytes written in %lu m.seconds.\n", numMax, timeRD );
								DisplayTime(1); //ouhs20170918
                break;

            case 'C' :  /* fc <addr> <len> <val> - Check the data of write by fw */
                if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)
                    || p1>=MAX_ADDR ||(p1+p2)>MAX_ADDR
                    )
                {
                    xprintf("Input the address,length , val for check.\n");
                    break;
                }
                xmemset(UserBuffer, 0, MAX_BUFFER_SIZE);
                rc = p3;

                numMax = blkSize = p3 = p2;

                if (blkSize>MAX_BUFFER_SIZE)
                {
                    blkSize = MAX_BUFFER_SIZE;
                }
                p2 += p1;
                while (p1 < p2 )
                {

                    if (FiscalMem_ReadBuffer(UserBuffer,p1, blkSize)==ERROR)
                    {
                        xprintf("Error at 0x%x when read.\n",p1);
                        break;
                    }
                    for (i=0;i<blkSize;i++)
                    {
                        if (UserBuffer[i]!=rc)
                            break;
                    }
                    if (i<blkSize)
                    {
                        xprintf("Error write at 0x%x .\n",p1+i);
                        break;
                    }
                    p1 += blkSize;//Totals of the read
                    p3 -= blkSize;

                    if (blkSize>p3)
                    {
                        blkSize = p3;
                    }
                }
                break;
            case 'w' :  /* fW <addr> <string> - write a string to flash,erase the section auto*/
                if (!xatoi(&ptr, &p1) || p1>=MAX_ADDR)//get the address for write
                {
                    xprintf("Input the address,string for write.\n");
                    break;
                }
                if (*ptr==' ') ptr++;

                for (p2=0;*ptr;p2++)//get the string for write
                {
                    UserBuffer[p2] = *ptr++	;
                }
                if ((p1+p2)>=MAX_ADDR)//out of range
                {
                    p2 = MAX_ADDR-p1;
                }

                timeRD = 0;
                if (1)//(WriteBlock(p1,UserBuffer,p2)==ERROR)
                    xputs("Error found for fw command.\n");
                else
                {
                    xprintf("\n<---%lu bytes written in %lu m.seconds.\n", p2, timeRD );
                }
                break;
            case 'c' :  /* fC <addr> <string> - check the write by fW */
                if (!xatoi(&ptr, &p1) || p1>=MAX_ADDR)//get the address for check
                {
                    xprintf("Input the address,string for check.\n");
                    break;
                }
                if (*ptr==' ') ptr++;
                for (p2=0;*ptr;p2++)//get the string for write
                {
                    UserBuffer[p2] = *ptr++	 ;
                }
                if ((p1+p2)>=MAX_ADDR)//out of range
                {
                    p2 = MAX_ADDR-p1;
                }
                if (FiscalMem_ReadBuffer(UserBuffer+p2,p1, p2)==ERROR)
                {
                    xputs("Error when read for fc command.\n");
                    break;
                }
                for (i=0;i<p2;i++)
                {
                    if (UserBuffer[i]!=UserBuffer[i+p2])
                    {
                        xprintf("Error found for fc command at:0x%x.\n",p1+i);
                        break;
                    }
                }
                if (i==p2)
                    xputs("OK for fW command.\n");
                break;

            case 'E':
                if (FiscalMem_EraseBulk()==ERROR)
						//if (sFlash_EraseBulk()==ERROR)
                    xputs("Error found when erase Chip\n");
                else
                    xputs("All of the Chip erased");

                break;
            case 'e' :  /* fe <secFrom> <secTo> - erase blocks */
                if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2) || p1>p2 || p1>FiscalMem_SEC_MAX || p2>FiscalMem_SEC_MAX)
                {
                    xputs("Input the section.from and section.to for erase.\n");
                    break;
                }
                if (FiscalMem_Erase(p1, p2)==ERROR)
                    xputs("Error found when erase flash\n");
                else
                    xprintf("Chip erased from %d to %d (Section)\n",p1,p2);

                break;
            case 'l':
                xputs("Start to Loading Chinese. \n");
                //DowmLoadBINCLIB();
                xputs("Finish of Loading Chinese. \n");
                break;
            }
            break;

        }//switch
    }//for
	}
	else
		xputs("FM is not present. \r\n");
}


#if	(1)//SDIO_SD_DEBUG

/* Private typedef -----------------------------------------------------------*/
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

/* Private define ------------------------------------------------------------*/

#define NUMBER_OF_BLOCKS     (BUFFER_SIZE/1024)  /* For Multi Blocks operation (Read/Write) */
#define MULTI_BUFFER_SIZE    BUFFER_SIZE //(1024 * NUMBER_OF_BLOCKS)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//uint8_t Buffer_Block_Tx[SDBLOCKSIZE], Buffer_Block_Rx[SDBLOCKSIZE];

//ouhs 20140818 4个字节对齐，防止不同编译器编译优化时导致SD卡读写数据最后两个字节丢失。

#define Buffer_MultiBlock_Tx TxBuffer
#define Buffer_MultiBlock_Rx RxBuffer
#define Buffer_Block_Tx TxBuffer
#define Buffer_Block_Rx RxBuffer


volatile TestStatus EraseStatus = FAILED, TransferStatus1 = FAILED, TransferStatus2 = FAILED;
volatile SD_Error SD_Status = SD_OK;

SD_Error Status = SD_OK;

SD_CardInfo SDCardInfo;

#ifdef SPI_SD
extern uint32_t CardCapacity;
extern uint32_t CardCapacity_2nd;
#else
uint32_t CardCapacity;
uint32_t CardCapacity_2nd;
#endif



/* Private function prototypes -----------------------------------------------*/
void SD_EraseTest(void);

uint32_t SD_SingleBlockTest(uint32_t SDAddr);
uint32_t SD_MultiBlockTest(uint32_t SDAddr);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
void Test_SD(char mode)
{
	uint32_t ret,i,j;
			
	//NVIC_SDConfiguration();

	//SD_Init(); //ouhs test 20140620
	if (1)//(SD_Detect()== SD_PRESENT)
	{
		xputs(" SD is present. \n");
		//SD_PWR_ON();
		SD_Status = SD_Init();
		//SD_Init();

		if (SD_Status == SD_OK)
		{
			USART_printf("\r\n SD Init success ");
#ifdef SPI_SD		
			//SD_Status = SD_GetCardInfo_2nd(&SDCardInfo);
			SD_Status =SD_GetCardInfo(&SDCardInfo);
#else
CardCapacity = SDCardInfo.CardCapacity;
#endif			
			if (SD_Status == SD_OK)
			{
				//CardCapacity = SDCardInfo.CardCapacity;
				xprintf("\r\n SD_GetCardInfo is OK");
				xprintf("\r\n CardBlockSize is ..%d ", SDCardInfo.CardBlockSize);
				xprintf("\r\n CardCapacity is ..%d M \r\n", SDCardInfo.CardCapacity/1024 /1024);
				xprintf("\r\n CardCapacity is ..%d M \r\n", CardCapacity/1024 /1024);
				//xprintf( " \r\n CardType is ..%d ", SDCardInfo.CardType );
				//xprintf( " \r\n RCA is ..%d ", SDCardInfo.RCA);
				//xprintf( " \r\n ManufacturerID is ..%d ", SDCardInfo.SD_cid.ManufacturerID );

			}
			else
			{
				USART_printf( " \r\n SD_GetCardInfo failed ");
			}
			//SD_EraseTest();

			/*if (mode=='S')
			{//
					for (i=0;i<CardCapacity;i+=SDBLOCKSIZE*NUMBER_OF_BLOCKS)
					{
							sprintf(TxBuffer,"%d",i);
							//PutsO(TxBuffer);
							ret = SD_MultiBlockTest(i);
							if (ret)
							{
									USART_printf( " \r\n SD_MultiBlockTest Error at:%d \n", i+ret-1);
									break;
							}
					}
			}
			else 
			*/if (mode=='s')
			{
					for (i=0;i<CardCapacity;i+=SDBLOCKSIZE)
					{
							//sprintf(Buffer_Block_Rx,"Addr=%lX",i);
							//PutsO(Buffer_Block_Rx);
							xprintf("\n\rAddr 0x%X ", i);
							ret = SD_SingleBlockTest(i);
							if (ret)
							{
									USART_printf( " \r\n SD_SingleBlockTest Error at:%d \n", i+ret-1);
									break;
							}
					}
			}
			else if (mode=='S') //按读写特定数量的字节测试
			{			
				for (i=0;i<CardCapacity;i+=1024)
        {
					for (j=0; j<1024; j++) //读写512字节
					  Buffer_Block_Tx[j] = i+j;
					ret = SD_ReadMultiBlocks((BYTE *)Buffer_Block_Rx, i, SDBLOCKSIZE, 2); //使用SD_ReadMultiBlocks先读后写再读容易出错
					memset(Buffer_Block_Rx, 0x5A, 1024);
					//写入
					if (ret == SD_OK)	
					{
						ret = SD_WriteMultiBlocks((BYTE *)Buffer_Block_Tx, i, SDBLOCKSIZE, 2);
					}
					else
					{
						xprintf("\r\n 1 SD_ReadMultiBlocks Status=%d ", ret);
					}
					//读取
					if (ret == SD_OK)	
					{
						ret = SD_ReadMultiBlocks((BYTE *)Buffer_Block_Rx, i, SDBLOCKSIZE, 2);
					}
					else
					{
						xprintf("\r\n SD_WriteMultiBlocks Status=%d ", ret);
					}
					//校验
					if (ret == SD_OK)
					{
						ret = Buffercmp(Buffer_Block_Tx, Buffer_Block_Rx, 1024);
					} 
					else
					{
						xprintf("\r\n 2 SD_ReadMultiBlocks Status=%d ", ret);
					}
					
					if (ret)
					{
							USART_printf( " \r\n SD Write&Read  Error at:%d \n", i+ret-1);
							break;
					}
					//sprintf(Buffer_Block_Rx,"Addr=%lX",i);
					//PutsO(Buffer_Block_Rx);
					xprintf("\n\rAddr 0x%X ", i);
        }
				USART_printf( " \r\n SD Write&Read  END ");					
			}
			
		}
		else
		{
			USART_printf("\r\n ERROR:SD Init failed! ");
			xprintf("\r\n SD_Status=%d \r\n", SD_Status);
		}
	}
	else
	{
		xputs(" SD is not present. \n");
	}
}


/**
  * @brief  Tests the SD card erase operation.
  * @param  None
  * @retval None
  */
void SD_EraseTest(void)
{
  /*------------------- Block Erase ------------------------------------------*/
  if (Status == SD_OK)
  {
    /* Erase NumberOfBlocks Blocks of WRITE_BL_LEN(512 Bytes) */
    //Status = SD_Erase(0x00, (SDBLOCKSIZE * NUMBER_OF_BLOCKS));
	xprintf("\r\n SD_Erase Status=%d ", Status);
  }

  if (Status == SD_OK)
  {
    Status = SD_ReadMultiBlocks(Buffer_MultiBlock_Rx, 0x00, SDBLOCKSIZE, NUMBER_OF_BLOCKS);

		xprintf("\r\n SD_ReadMultiBlocks Status=%d ", Status);

    /* Check if the Transfer is finished */
		//Status = SD_WaitReadOperation();

		xprintf("\r\n SD_WaitReadOperation Status=%d ", Status);

    /* Wait until end of DMA transfer */
    while(SD_GetStatus() != SD_OK);
  }

  /* Check the correctness of erased blocks */
  if (Status == SD_OK)
  {
  	USART_printf("\r\n SD_EraseTest success");
  }
  else
  {
  	USART_printf("\r\n SD_EraseTest failed");
  }
}

/**
  * @brief  Tests the SD card Single Blocks operations.
  * @param  None
  * @retval None
  */
uint32_t SD_SingleBlockTest(uint32_t SDAddr)
{
  /*------------------- Block Read/Write --------------------------*/
  /* Fill the buffer to send */

  uint32_t i;
  /*--------------- Multiple Block Read/Write ---------------------*/
  /* Fill the buffer to send */
	for (i=0; i<SDBLOCKSIZE; i++) //读写512字节
		Buffer_Block_Tx[i] = i;

	Status = SD_OK;//Status = SD_ReadBlock((BYTE *)Buffer_Block_Rx, SDAddr, SDBLOCKSIZE);
	memset(Buffer_Block_Rx, 0x5A, SDBLOCKSIZE);
  if (Status == SD_OK)
  {
    /* Write block of 512 bytes on address 0 */
          Status = SD_WriteBlock(Buffer_Block_Tx, SDAddr, SDBLOCKSIZE);		
  }
	else
	{
		xprintf("\r\n 1 SD_ReadBlock Status=%d ", Status);
		return 1;
	}
	
  if (Status == SD_OK)
  {
    /* Read block of 512 bytes from address 0 */
    Status = SD_ReadBlock(Buffer_Block_Rx, SDAddr, SDBLOCKSIZE);
  }
	else
	{
		xprintf("\r\n SD_WriteBlock Status=%d ", Status);
		return 1;
	}
	
  if (Status == SD_OK)
	{
    return  Buffercmp(Buffer_Block_Tx, Buffer_Block_Rx, SDBLOCKSIZE);
  }
	else
	{
		xprintf("\r\n 2 SD_ReadBlock Status=%d ", Status);
    return 1;
	}
}

/**
  * @brief  Tests the SD card Multiple Blocks operations.
  * @param  None
  * @retval None
  */
uint32_t SD_MultiBlockTest(uint32_t SDAddr)
{
  uint32_t i;
  /*--------------- Multiple Block Read/Write ---------------------*/
  /* Fill the buffer to send */
  //Fill_Buffer(Buffer_MultiBlock_Tx, MULTI_BUFFER_SIZE, 0x0);
  for (i=0;i<MULTI_BUFFER_SIZE/2;i++)
  {
      Buffer_MultiBlock_Tx[2*i]= i & 0xff;
      Buffer_MultiBlock_Tx[2*i+1]= SDAddr & 0xff;
  }

  if (Status == SD_OK)
  {
    /* Write multiple block of many bytes on address 0 */
    Status = SD_WriteMultiBlocks(Buffer_MultiBlock_Tx, SDAddr, SDBLOCKSIZE, NUMBER_OF_BLOCKS);
  }

  if (Status == SD_OK)
  {
    /* Read block of many bytes from address 0 */
    Status = SD_ReadMultiBlocks(Buffer_MultiBlock_Rx, SDAddr, SDBLOCKSIZE, NUMBER_OF_BLOCKS);

  }

  /* Check the correctness of written data */
  if (Status == SD_OK)
  {
      return Buffercmp(Buffer_MultiBlock_Tx, Buffer_MultiBlock_Rx, SDBLOCKSIZE*NUMBER_OF_BLOCKS);
  }
  return 1;
}

#endif	 // TEST_SD

#if TEST_SD_FS

BYTE writeTextBuff1[] = "STM32 Demo application";
void Test_SD_FS(uint8_t pdrv)
{
  FRESULT res;
  FILINFO finfo;
  DIR dirs;
	FATFS fs, fs_2nd;
	FIL file;
	
	WORD bytesWritten, bytesToWrite,i=0;
	
	DWORD free_clust, tot_sect, fre_sect,total, free;

	FATFS *fs1;
	
  char path[100]={""};  
	char TestFileName[100];
	char DirName1[50],DirName2[50],DirName3[50],DirName4[50];
	uint32_t rng_data,Filenum=0;

	extern TSDISK *SDiskP;
	
	rng_data = GetRandomNumber();
	
#if _USE_LFN == 3
	my_mem_init(SRAMIN);
	//exfuns_init();	
#endif
	
  //if (SD_Detect()== SD_PRESENT)
  {
		
		//xputs("\r\n SD is present.");
    //if(SD_Init()==SD_OK)
    {
		//	xputs("\r\n SD Init success.");
		
			disk_initialize(pdrv);						     			//fatfs可以管理多个介质的分区， 所以把物理存储介质SD卡标示为0区，相当于磁盘编号  
	
			if(f_mount(pdrv, &fs) == FR_OK)	
				xprintf("\r\n %d:f_mount OK", pdrv);
			else
				xprintf("\r\n %d:f_mount err", pdrv);
			//ff_MountSDisk(FS_SD);
		}

	
		//读取SD卡容量及剩余容量
		DisplayTime(0);
		if(f_getfree("1:/", &free_clust, &fs1)==FR_OK)
		{
			tot_sect=(fs1->n_fatent-2)*fs1->csize; 
			fre_sect=free_clust*fs1->csize;   

			total=tot_sect>>1; 
			free=fre_sect>>1; 
			xprintf("\r\n Capacity is ..%d M /%d M \r\n", free/1024, total/1024);
		}
		else
		{
			xputs("\r\n f_getfree err");
		}	
		DisplayTime(1);
/*		
		disk_initialize(FS_SD_2ND);
		f_mount(FS_SD_2ND, &fs_2nd);
		
		DisplayTime(0);
		if(f_getfree("2:/", &free_clust, &fs1)==FR_OK)
		{
			tot_sect=(fs1->n_fatent-2)*fs1->csize; 
			fre_sect=free_clust*fs1->csize;   

			total=tot_sect>>1; 
			free=fre_sect>>1; 
			xprintf("\r\n INTR Capacity is ..%d M / %d M \r\n", free/1024, total/1024);
		}
		else
		{
			xputs("\r\n INTR f_getfree err");
		}	
		DisplayTime(1);
*/
/*		
		if(f_mkfs(pdrv,1,4096)== FR_OK)	//4K
			xprintf("\r\n %d:f_mkfs OK", pdrv);
		else
			xprintf("\r\n %d:f_mkfs err", pdrv);
*/
		dirs.fs = &fs;

		if (f_opendir(&dirs, "1:/") == FR_OK)        	//读取该磁盘的根目录   
		{
			while(Filenum++<20000)
			{			
				sprintf(DirName1, "1:dir%d-%02d", (uint8_t)rng_data,Filenum);
				sprintf(DirName2, "1:dir%d-%02d/dir2-%02d", (uint8_t)rng_data,Filenum,Filenum);
				sprintf(DirName3, "1:dir%d-%02d/dir2-%02d/dir3-%02d", (uint8_t)rng_data,Filenum,Filenum,Filenum);
				sprintf(DirName4, "1:dir%d-%02d/dir2-%02d/dir3-%02d/dir4-%02d", (uint8_t)rng_data,Filenum,Filenum,Filenum,Filenum);
				sprintf(TestFileName, "1:/dir%d-%02d/dir2-%02d/dir3-%02d/dir4-%02d/test%02d.txt", (uint8_t)rng_data,Filenum,Filenum,Filenum,Filenum,Filenum);      /* 每写1次，序号递增 */
								
				xprintf("Create dir...\r\n");
				DisplayTime(0);	f_mkdir(DirName1);DisplayTime(1);
				DisplayTime(0);	f_mkdir(DirName2);DisplayTime(1);
				DisplayTime(0);	f_mkdir(DirName3);DisplayTime(1);
				DisplayTime(0);	res = f_mkdir(DirName4);DisplayTime(1);

				if(res != FR_OK)
				{
						xprintf("Fail to create dir, Err Code:%d\n",res);
						//break;				
				}
				xprintf("Open file\r\n");				
				DisplayTime(0);
				res = f_open(&file, TestFileName, FA_CREATE_ALWAYS | FA_WRITE);		
				//res = ff_OpenFile(TestFileName, FA_CREATE_ALWAYS | FA_WRITE);				
				DisplayTime(1);
				
				if(res == FR_OK)
				{ 
					//xprintf("Write to file\r\n");
					bytesToWrite = sizeof(writeTextBuff1);
					//DisplayTime(0);
					res= f_write (&file, writeTextBuff1, bytesToWrite, (void *)&bytesWritten);
					//res= f_write (&SDiskP->File, writeTextBuff1, bytesToWrite, (void *)&bytesWritten);					
					//DisplayTime(1);
					if((bytesWritten == 0) || (res != FR_OK)) 
					{
						xprintf("\r\n%s CANNOT be writen", TestFileName);
						break;
					}
					else
					{
						xprintf("\r\n%s file created", TestFileName);
					}	
				}
				else
				{
					xprintf("\r\n f_open:%d", res);
					break;
				}
				
				res = f_close(&file);				
				//res = f_close(&SDiskP->File);					
				if(res != FR_OK)
				{
					xprintf("\r\n f_close:%d", res);
					break;
				}
			}
			/*
			while (f_readdir(&dirs, &finfo) == FR_OK)  	//循环依次读取文件名
			{
				//if (finfo.fattrib & AM_ARC) 							//判断文件属性是否为存档型
				if (finfo.fattrib & AM_ARC || finfo.fattrib & AM_DIR)
				{
					if(!finfo.fname[0]) break;         			//如果是文件名为空表示到目录的末尾。退出	 
					xprintf("\r\n file name is: %s",finfo.fname);
					xprintf("\n\r file size is: %d ", finfo.fsize); 
				}
				else
				{
					xprintf("\n\r Path name is: %s", finfo.fname); 
					break; //continue;//
				}
			}	
			*/
		}	
		else
		{
			xprintf("\n\r err: f_opendir"); 
		}		
	}
	//else
	{
		//xputs("\r\n SD is not present.");
	}
}

//! This function checks the file extension
//! @param     FileName   ASCII string of a File Name (ex: "abc.wav" )
//!
//! @param     filter      extension filter is a ASCII string (ex: "wav" )
//!
//! @return    TRUE, the file name have a good extension
//! @return    FALSE, otherwise
/*
bool  File_type_Check( u8 * FileName, u8 * filter )
{
  u8 * FileNameLocal;
  u32 DotPos = 0;
  bool RetVal = FALSE;
  

  while(1)
  {
    if((*(FileName + DotPos) == '.') || (DotPos >= 8+3))
    {
      break;
    }
    DotPos++;
  }
  
  if(DotPos >= 8+3)
  {
    RetVal = FALSE;
  }
  else
  {
    FileNameLocal = (FileName + DotPos + 1);
    DotPos = 0;
    for(DotPos=0; DotPos < 3; DotPos++)
    {
      if((*FileNameLocal == *filter) || ((*FileNameLocal+('a'-'A')) == *filter))
      {
        RetVal = TRUE;
        if(*(filter + 1) == '\0')
          break;
      }
      else
      {
        RetVal = FALSE;
        break;
      }
    }
  }

  return RetVal;
}
*/

#endif	//TEST_SD_FS


#if NOR_FLASH_TEST

uint16_t hBuff[128];
uint16_t wrBuff[25]={0x1230,0x1231,0x1232,0x1233,0x1234,0x1235,0x1236,0x1237,0x1238,0x1239,0x123a,0x123b,0x123c,0x123d,0x123e,0x123f};
uint16_t rdBuff[25];
void NOR_Flash_Test(void)
{
	uint16_t s_ID, d_ID;
	uint8_t Index;

	Read_SiliconID_OP(&s_ID);
	Read_DeviceID_OP(0, &d_ID);

	xprintf("\r\n SiliconID=0x%x, DeviceID=0x%x\r\n", s_ID, d_ID);
	/*
	NorFlash_WReadBuffer(hBuff, 0x1000, 128);

	for(Index = 0x00; Index < 128; Index++)
	{
		xprintf("\r\n hBuff[%d]=0x%x", Index, hBuff[Index]);
	}
	*/
	xprintf("---------------------------------------------");
	Sector_Erase_OP(NOR_FLASH_DATA_ADDR, 5);
	NorFlash_WriteBuffer(wrBuff, NOR_FLASH_DATA_ADDR, 23);
	NorFlash_WReadBuffer(rdBuff, NOR_FLASH_DATA_ADDR, 23);
	for(Index = 0x00; Index < 23; Index++)
	{
		xprintf("\r\n hBuff[%d]=0x%x", Index, rdBuff[Index]);
	}

}

#endif  //	NOR_FLASH_TEST


#if !defined(CASE_ASCIIDISPLAY)
void Test_OPDisplay(void)
{
	uint8_t cnt = 0;
	uint16_t i,j=1000;

	xprintf("Test_OPDisplay...\r\n");
#if 0  //by ouhs
	short i,j;
    BYTE ch=32;

    Cursor.x = 0;
    Cursor.y = 0;

    Bios1_ByPTR(BiosCmd_OPDisp_Cursor,&Cursor);

	for (j=32;j<=128;j+=SCREENWD)
	{
		if (j>32)
	        Bios1_ByVAL(BiosCmd_OPDisp_Scroll,1);
        for (i=0;i<SCREENWD;i++)
        {
            TxBuffer[i] = ch++;
            if (ch>129)
                ch =32;
        }
        TxBuffer[SCREENWD]=0;
        Bios_ByPTR(BiosCmd_OPDisp_StringXY,TxBuffer,0xff,0xff);
		Bios1_ByVAL(BiosCmd_msDelay,400);
	}
#else

//主显  128064
#if defined(CASE_ECR100S)
	//mSetInvAttr();
	memcpy(ApplVar.ScreenMap[2],"ABCDEFGHIJKLMNOPQRSTUVWXYZ...",SCREENWD);
	mRefreshLine(2);

	mClearInvAttr();
	memcpy(ApplVar.ScreenMap[3],"  1  1  1  1  1  1  1  1  1 ",SCREENWD);
	mRefreshLine(3);
	
	mSetInvAttr();
	memcpy(ApplVar.ScreenMap[4],"abcdefghijklmnopqrstuvwxyz",SCREENWD);
	mRefreshLine(4);

	mEnCursor(ENABLE);
	mMoveCursor(16, 3);	
#elif defined(CASE_MCR30)

	//mSetState(ENABLE);	// 状态行显示分隔线

	mSetInvAttr();	//反显
	memcpy(ApplVar.ScreenMap[0],"1.DEPT/PLU...         ",SCREENWD);
	mRefreshLine(0);
	
	mClearInvAttr();
	memcpy(ApplVar.ScreenMap[1],"2.GENERAL SET         ",SCREENWD);
	mRefreshLine(1);
#if defined(OD_4ROW)	 
	//mSetInvAttr();
	memcpy(ApplVar.ScreenMap[2],"3.KEYBOARD            ",SCREENWD);
	mRefreshLine(2);

	mClearInvAttr();
	memcpy(ApplVar.ScreenMap[3],"SET                    ",SCREENWD);
	mRefreshLine(3);
		
	//mSetInvAttr();
	//memcpy(ApplVar.ScreenMap[4],"abcdefghijklmnopqrstuvwxyz",SCREENWD);
	//mRefreshLine(4);

	//mEnCursor(ENABLE);
	//mMoveCursor(16, 3);	
#endif	
#endif	


#if 0	//defined(CASE_MCR30)
	//while(cnt+0x19 < 0x3F) //对比度调节 
	{
		mSetContrast(cnt++);
		msDelay(500);
	}
#endif	

#endif	
}
#endif



//测试客显
void Test_CustomerDis(void)
{
	uint8_t cnt;
	uint8_t i;

	uint8_t buf0[] = {"Test......"};//{0x80,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf};
	uint8_t buf1[] = {0x80,0x81,0x82,0x83,0x84/*,0x85,0x86,0x87*/};//{"0123456789123456"};//{"0123456789:;<=>?"};//
	xprintf("Test_CustomerDis...\r\n");

#if defined(CASE_ER260) || defined(CASE_ECR100S)
	cnt = 2;
	while(cnt--)
	{
		msDelay(1000);
		PutsO("888888888888");
		msDelay(1000);
		PutsO("01230123.4567");
	}	
/*	
#elif defined(CASE_MCR30)		//128032
	
	memcpy(ApplVar.ScreenMap[0],"     MCR30      ",16);
	cRefreshLine(0);
	memcpy(ApplVar.ScreenMap[1],"LCD128032       ",16);
	cRefreshLine(1);	
	
	cnt = 0;
	//while(cnt+0x2C < 0x3F) //对比度调节 
	{
	//	cSetContrast(cnt++);
	//	msDelay(500);
	}
*/	
#elif	defined(CASE_ECR100F) || defined(CASE_ECR99) || defined(CASE_MCR30)
	
	PutsO(buf0);//PutsC0(buf0);
	Puts1(buf1);//PutsC1(buf1);

#endif	
	
}

// 测试看门狗
void TEST_IWDG(void)
{
	uint16_t i, j;
	while (j--)  for(i=0; i<20000; i++); j=500;	//500ms
	USART_printf("\r\nStarting IWDG test...\r\n");
	*(__IO uint32_t *) 0xA0001000 = 0x5A;
	USART_printf("\r\nIWDG test Failed!\r\n");
}

// 测试Backup SRAM
extern void BKPSRAM_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint32_t NumHalfwordToWrite);
extern void BKPSRAM_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumHalfwordToRead);
void Test_BKPSRAM(void)
{
	DWORD i;

	//测试程序执行时间需要先设置TIM6
	/* Write data to FSMC SRAM memory */
	/* Fill the buffer to send */
	Fill_Buffer(TxBuffer, 4096, 0xf321);	 // 0xf321
	
	TIM6_Init_Count();//用于测试读写时间
	TIM_Cmd(TIM6, ENABLE);
	BKPSRAM_WriteBuffer(TxBuffer, 0, 4096);
	i = TIM_GetCounter(TIM6);
	USART_printf("\r\n Backup SRAM test: 4KB write time:%dus\r\n",i);
	TIM_Cmd(TIM6, DISABLE);
	//测试SRAM写入速度

	BKPSRAM_ReadBuffer(RxBuffer, 0, 4096);

	/* Read back SRAM memory and check content correctness */
	Buffercmp(TxBuffer, RxBuffer, 4096);
}

//网络测试
#if defined(CASE_ETHERNET)
extern __IO uint32_t LocalTime;
void Test_Ethernet(void)
{	
	xputs("Ethernet_Test...\n");

#if 1
	tcp_echoclient_connect();
	
#else	
  while (1)
  {
    /* check if any packet received */
    if (ETH_CheckFrameReceived())
    {
      /* process received ethernet packet */
      LwIP_Pkt_Handle();
    }
    /* handle periodic timers for LwIP */
    LwIP_Periodic_Handle(LocalTime);
  }
#endif
}
#endif

//测试锂电池和纽扣电池电路
void Test_PWR(void)
{
	uint8_t stat;
	uint8_t i, cnt=5,j=1000;
	uint8_t DispBattBuff[16];

	while(cnt--)
	{
		//while (j--)  for(i=0; i<20000; i++); j=1000;	//1s
		//if(DC_DET_GET() == 0)  //test by ouhs
		{
			//if(DC_DET_GET() == 0)
			{//系统由电池供电时检测电池电量
				stat = pwrGetStatus(GET_VIN_STATUS);
				switch (stat)
				{
					case PWR_STAT_OK: USART_printf("Battery OK"); break;
					case PWR_WARNING1: USART_printf("Warning:Low Battery!"); break;
					case PWR_WARNING2: USART_printf("Fatal Warning:Low Battery!"); break;
					default: break;
				}
			}
		}

		// 在系统开机时检测RTC电池电量一次
		stat = pwrGetStatus(GET_VRTC_STATUS);
		switch (stat)
		{
			case PWR_STAT_OK: USART_printf("RTC Battery OK\r\n"); break;
			case PWR_WARNING1: USART_printf("Warning:Low RTC Battery!\r\n"); break;
			case PWR_WARNING2: USART_printf("Fatal Warning:Low RTC Battery!\r\n"); break;
			default: break;
		}
	}

	stat = pwrBATStatus();
	if(stat==STAT_CHARGING)
		USART_printf("Battery charging...\r\n");
	else if (stat==STAT_DONE_IDLE)
		USART_printf("Battery charged or idle...\r\n");
	else if (stat==STAT_NOBATT_ERR)
		USART_printf("ERR:NO Battery...\r\n");
	
	i = BattaryCheck(true);
	xprintf("BATTERY: %d%%",i);
	sprintf(DispBattBuff, "BATTERY: %d%%", (uint8_t)i);
#if !defined(OD_4ROW)	
	PutsO(DispBattBuff);
#else
	//
#endif	

}


//网卡MAC地址设置
extern uint8_t OTPFlash_WriteMAC(uint8_t* pBuffer);
extern uint8_t OTPFlash_ReadMAC(uint8_t* pBuffer);

/*
00:01:6C:A3:7E:0A  15    已用

44:87:FC:C3:B2:28  31    已用

00:01:25:86:74:E8  42    已用     

00:01:6C:8C:01:FF  69 		已用    

00:26:55:B2:24:11  76			已用  
*/

void Test_OTPFlash_MAC(void)
{
	
	uint8_t status,i;
	uint8_t MAC_Addr[6]={0x00, 0x12, 0x15, 0xC2, 0x68, 0x2A};
	uint8_t Readbuffer[6];

	status=ERROR;
	
	status = OTPFlash_WriteMAC(MAC_Addr);

	if (status==SUCCESS)
	{
		OTPFlash_ReadMAC(Readbuffer);
		xputs("MAC_Addr:");
		for (i=0;i<6;i++)
		{
				xputc(Readbuffer[i]);
		}
	}
	else
	{
		xputs("OTPFlash Failed...");
	}

}

#if 0
//测试SPI Data Flash
void Test_DataFlash(void)
{
	uint32_t i, at45_ID;

	Fill_Buffer(TxBuffer, BUFFER_SIZE, 0);

	at45_ID = SPI_FLASH_ReadID();
	xprintf("Manufactory_ID is:%x\n",at45_ID);
	
	//xprintf("SPI_FLASH_ChipErase...\n");
	//DisplayTime(0);
	//SPI_FLASH_ChipErase();
	//DisplayTime(1);
	
	xprintf("SPI_FLASH_BufferWrite...\n");
	DisplayTime(0);
	RamOffSet = 5;
	WriteRam(TxBuffer, BUFFER_SIZE);
	DisplayTime(1);

	memset(RxBuffer, '\0', sizeof(RxBuffer));

	xprintf("SPI_FLASH_BufferRead...\n");
	DisplayTime(0);
  RamOffSet = 5;
	ReadRam(RxBuffer, BUFFER_SIZE);
	DisplayTime(1);

	Buffercmp(TxBuffer, RxBuffer, BUFFER_SIZE);
}
#endif


//粗略计算产生掉电信号到系统复位的时间
void Test_Poweroff(void)
{
	DWORD i;

	TIM6_Init_Count();//用于测试读写时间

	TIM_Cmd(TIM6, ENABLE);
	while(1)
	{
		i = TIM_GetCounter(TIM6);	//us
		USART_printf("\r\nPWROFF:%dus\r\n",i);
	}
}


//测试低功耗模式
void Test_LowPowerMode(void)
{
	uint8_t i=0;
	while(i++<2)
	{
			
#if !defined(CASE_ASCIIDISPLAY)
			msDelay(5000);
			mClearScreen();
		 	memcpy(ApplVar.ScreenMap[0],"Low power mode...",22);
			mRefreshLine(0);
			mLightLCDOff();
			EnterSTOP();
			memcpy(ApplVar.ScreenMap[0],"running mode...",22);
			mRefreshLine(0);//mDispLogo();
			mLightLCDOn();
#else		 
			msDelay(5000);
			PutsO_Only("Low power mode.");
			LCD_BL_OFF();
			EnterSTOP();
			PutsO_Only("running mode.");
			LCD_BL_ON();
#endif
	 }
}

//钱箱测试
void Test_Drawer(void)
{
	
}

//串口测试 port = LOGIC_COM1 2 3
uint8_t Test_COM(uint8_t port)
{
	uint8_t ch1=1,ch2=1;

	//清空
	if (Bios_PortRead(port, NULL, 0, 0, NULL))
		Bios_PortRead(port, NULL, -1, 0, NULL);
	
	//LCDClearLine(0, 0);LCDClearLine(0, 1);
	
	while(ch1<0x80)
	{
		UARTSend(port,ch1);
		UARTGet(port,&ch2,500);
		if(ch1 != ch2)
		{
#if defined(CASE_ASCIIDISPLAY)		
			PutsO_Only("error");
#else			
			memcpy(ApplVar.ScreenMap[0]," error          ",16);
			mRefreshLine(0);
#endif
			return 1;
		}
		ch1++;
	}
#if defined(CASE_ASCIIDISPLAY)		
	PutsO_Only("ok");
#else	
	memcpy(ApplVar.ScreenMap[0]," ok             ",16);
	mRefreshLine(0);
#endif	
	return 0;
	
}

//打印测试
void Test_Printer(void)
{
	uint8_t prt_buf1[] = {"abcdefghijklmnopqrstuvwxyz"};
	uint8_t prt_buf2[] = {"abc123456789012345678901234567890"};
	uint8_t prt_buf3[] = {""};
	uint8_t prt_buf4[] = {0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf};

	uint8_t cnt=5;	
#if 0
	while(1)
	{
		msDelay(500);
		if(PrintDetect()==0)
			xputs("Printer is not connected...\n");
	}
#endif
	
	if (!TESTPAPER())
	{
			xputs("Paper out.\n");
	}
	else
	{
		while(cnt--)
		{
			RSPrint(prt_buf1, 30, CMDP_RJ);RSPrint(prt_buf1, 30, CMDP_RJ);RSPrint(prt_buf1, 30, CMDP_RJ);RSPrint(prt_buf1, 30, CMDP_RJ);
			RSPrint(prt_buf2, 30, CMDP_RJ);RSPrint(prt_buf2, 30, CMDP_RJ);RSPrint(prt_buf2, 30, CMDP_RJ);RSPrint(prt_buf2, 30, CMDP_RJ);
			RSPrint(prt_buf4, 30, CMDP_RJ);
			if(PrinterDetect()==0)
				xputs("Printer is not connected...\n");			
			//msDelay(1000);
		}
	}	
}

//闹钟测试
void Test_Alarm(void)
{
	RTC_AlarmSet();
	DisplayTime(0);
}

//键盘测试
void Test_Keyboard(void)
{
	uint8_t keyval=255,i=0;
	uint8_t buff[20];
	
	xprintf("Test_Keyboard...\r\n");
#if !defined(CASE_ASCIIDISPLAY)			
	mClearScreen();
#endif
	while(1)
	{
		msDelay(10);
		if(CheckKeyboard())
		{
			
			keyval = Getch();
			sprintf(buff,"Key    %d\r\n",keyval);
			xprintf(buff);

			if(i==SCREENLN)
			{
				i = 0;	
#if !defined(CASE_ASCIIDISPLAY)					
				mClearScreen();
#endif		
			}
#if !defined(CASE_ASCIIDISPLAY)	
			memcpy(ApplVar.ScreenMap[i],buff,20);
			mRefreshLine(i++);
#endif


#if CASE_MCR30			
			if(keyval==29)
#elif CASE_ECR99
			if(keyval==34)
#endif			
				break;
		}

	}
}

//GSM测试
void Test_SIM800(void)
{
	char *ptr;
	uint8_t ucValue;
	
	InitSIM800();

	xprintf("正在给SIM800模块上电...\r\n");
	
	SIM800_PowerOn();

	xprintf("上电完成\r\n");
	
	xprintf("【1 - 模块AT指令测试】\r\n");
	xprintf("【2 - 模块TCPIP测试】\r\n");

	/* 进入主程序循环体 */
	while (1)
	{
		ptr = ConsoleLine;
		ConsoleLine[0]=0;
		get_line(ptr, sizeof(ConsoleLine));
		switch (*ptr++)
		{
				case '1':
					while(1)
					{
						if (UARTGet(COM_SIM800, &ucValue, 0))
						{
							UARTSend(PORT_DEBUG, ucValue);
						}

						if (UARTGet(PORT_DEBUG, &ucValue, 0))
						{
							UARTSend(COM_SIM800, ucValue);
						}
					}
					break;
					
				case '2':
					ptr = ConsoleLine;
					ConsoleLine[0]=0;
					get_line(ptr, sizeof(ConsoleLine));
					switch (*ptr++)
					{
						case '1':
							SIM800_GetLocalIP();		/*获取模块本机IP地址 */
							break;
						case '2':
							SIM800_GetTCPStatus();		/*获取IP连接状态 */
							break;
						case '3':
							SIM800_TCPStart("www.xxx.com", 29155);	/* 建立与服务器的TCP连接 */
							break;
						case '4':
							SIM800_TCPSend("hello...");	/* 向服务器发送数据 */
							break;
						case '5':
							SIM800_TCPClose();			/* 关闭TCP连接 */
							break;
					}
					break;
			}
	}
}

