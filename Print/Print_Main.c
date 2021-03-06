/****************************************Copyright (c)**************************************************
*        EUTRON Human Information Equipment Co.,Ltd.
*                    www.eutron.com.cn
* File name:		Print_Main.c
* Descriptions:     打印机驱动程序
*------------------------------------------------------------------------------------------------------
*  Created by:			chen churong
* Created date:        2011-10-01
* Version: 1.0
********************************************************************************************************/

#include "king.h"                               /* Kingtron library and I/O routines */
#include "exthead.h"
#include "Font12x24cy.h"

#define DELAY_CONST 5   //ccr2014-09-12 检测纸信号的延迟计数常量 //

#if DEBUGONPC
    #include <stdio.h>
    #include <mem.h>
#endif


void ReadHZKU(ULONG addr,BYTE *hzBuf);
void ChangeGRBToMyself(void);

extern APPLICATION_SAVE  ApplVar;

/**************************** modules for printer ******************/
#if (CASE_FTP628MCL401) //HJ
#include "Print_FTP628MCL401.h"
#elif (CASE_PT723F_RJ)
#include "Print_PT723F.h"
#else //(CASE_PT486F)
#include "Print_PT486F.h" //ccr2018-03-22当前使用的打印机驱动 CURRENT for use
#endif

///////////////////////////////////////////////////////////////////
//ouhs 20140814
#if defined (__CC_ARM)      /* ARM Compiler */
  struct GRB_DEFINE GRB_Define __attribute__((at(BKPSRAM_BASE)));//=BKPSRAM_BASE;用户下载的GRB图片数据存储区
#elif defined (__ICCARM__)    /* IAR Compiler */
  __no_init struct GRB_DEFINE GRB_Define @BKPSRAM_BASE;
#endif
/*********************************************************************************************************
** Function name:      PrnTIMER_Config
** Descriptions:      定时器2用于打印任务控制,定时器3用于加热控制
*********************************************************************************************************/
/**
  * @brief  Configures the used Timers.
  * @param  None
  * @retval None
  */
void PRNTIMER_Init(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  NVIC_InitTypeDef  NVIC_InitStructure;

  /* Enable TIM2, TIM3 clocks */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3, ENABLE);

  /* .............................................*/

  /* TIM2 configuration */

  TIM_TimeBaseStructure.TIM_Period = uSECOND * 5000; //5ms
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
  TIM_ARRPreloadConfig(TIM2,DISABLE);// 不使用影子寄存器 //
  TIM_OCStructInit(&TIM_OCInitStructure);

  /* Output CompareBCDValue Timing Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
  TIM_OCInitStructure.TIM_Pulse = 0x0;
  TIM_OC1Init(TIM2, &TIM_OCInitStructure);

  /* Immediate load of TIM2 Precaler values */
  //ccr TIM_PrescalerConfig(TIM2, ((SystemCoreClock/1200) - 1), TIM_PSCReloadMode_Immediate);
  /* Clear TIM2 update pending flags */
  TIM_ClearFlag(TIM2, TIM_FLAG_Update);

  /* Enable the TIM2 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* .............................................*/

  /* TIM3 configuration */
  //ccr TIM_TimeBaseStructure.TIM_Period = 0x95F;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  /* Output CompareBCDValue Timing Mode configuration: Channel1 */
  TIM_OC1Init(TIM3, &TIM_OCInitStructure);

  /* Immediate load of TIM3 Precaler values */
  //ccr TIM_PrescalerConfig(TIM3, ((SystemCoreClock/1200) - 1), TIM_PSCReloadMode_Immediate);

  /* Clear TIM3 update pending flags */
  TIM_ClearFlag(TIM3, TIM_FLAG_Update);

  /* Configure two bits for preemption priority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);


  /* Enable the TIM3 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStructure);

  /* .............................................*/

  /* Enable TIM2  Update interrupts */
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
  /* TIM2 enable counters */
  TIM_Cmd(TIM2, ENABLE);

  /* Enable TIM3  Update interrupts */
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

  /* TIM3 enable counters */
  TIM_Cmd(TIM3, ENABLE);
}


/********* ADC1  Channel 10 用于检测温度 ***************************************
  * @brief  ADC1 Channel 10 configuration (DMA, ADC, CLK)
  * @param  None
  * @retval None
********************************************************************************/
void PRNTempPort_Init(void)
{

  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  ADC_InitTypeDef ADC_InitStructure;

  /* Enable peripheral clocks *************************************************/
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
  /* ADC Channel 10 -> PC0  */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

/* ADC Common Init */
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;//ADC_DMAAccessMode_1;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_15Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);

  /* ADC1 regular channels 10 configuration*/

  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 1;
  ADC_Init(ADC2, &ADC_InitStructure);

  /* ADC2 regular channels 10, 11 configuration */
  ADC_RegularChannelConfig(ADC2, ADC_Channel_10, 1, ADC_SampleTime_15Cycles);

  /* Enable ADC2 */
  ADC_Cmd(ADC2, ENABLE);

  /* Start ADC2 Software Conversion */
  ADC_SoftwareStartConv(ADC2);


}

/*******************************************************************************************
** 函数名称：PRNData_Init
** 功能说明：初始化用于数据端口的SPI。
** 入口参数：无
** 出口参数：无
*******************************************************************************************/
/**
  * @brief  Initializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
void PRNDataPort_Init(void)
{
#if CASE_ER260F
  SPI_InitTypeDef  SPI_InitStructure;

  GPIO_InitTypeDef GPIO_InitStructure;

  /*!< Enable the SPI clock */
  PRNData_SPI_CLK_INIT(PRNData_SPI_CLK, ENABLE);

  /*!< Enable GPIO clocks */
  RCC_AHB1PeriphClockCmd(PRNData_SPI_SCK_GPIO_CLK | PRNData_SPI_MOSI_GPIO_CLK | PRNData_LATCH_GPIO_CLK, ENABLE);

  /*!< SPI pins configuration *************************************************/

  /*!< Connect SPI pins to AF5 */
  GPIO_PinAFConfig(PRNData_SPI_SCK_GPIO_PORT, PRNData_SPI_SCK_SOURCE, PRNData_SPI_SCK_AF);
  GPIO_PinAFConfig(PRNData_SPI_MOSI_GPIO_PORT, PRNData_SPI_MOSI_SOURCE, PRNData_SPI_MOSI_AF);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

  /*!< SPI SCK pin configuration */
  /*!< SPI MOSI pin configuration */
  GPIO_InitStructure.GPIO_Pin = PRNData_SPI_SCK_PIN | PRNData_SPI_MOSI_PIN;
  GPIO_Init(PRNData_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure LATCH pin in output pushpull mode ********************/
  GPIO_InitStructure.GPIO_Pin = PRNData_LATCH_PIN;// LATCHBIT
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(PRNData_LATCH_GPIO_PORT, &GPIO_InitStructure);

  /*!< Deselect the FLASH: Chip Select high */
  PRNData_LATCH_HIGH();

  /*!< SPI configuration for Printer data port*/
  SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
#if WORDorBYTE
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
#else
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
#endif
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2; // ouhs SPI_BaudRatePrescaler_4;

  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(PRNData_SPI, &SPI_InitStructure);

  /*!< Enable the PRNData_SPI  */
  SPI_Cmd(PRNData_SPI, ENABLE);

#elif defined(CASE_ECR100F) || defined(CASE_ECR100S) || defined(CASE_MCR30) || defined(CASE_ECR99)
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	//GPIOC: PCLK--GPIO_Pin_3, PDATA--GPIO_Pin_5, PLATCH--GPIO_Pin_4
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

#endif
}
/* 初始化打印机走纸电机控制端口 */
void PRNFeedPort_Init(void)
{
#if (!CASE_FTP628MCL401)


#endif
}

/* 初始化打印机检测纸,电源控制端口 */
void PRNPePwPort_Init(void)
{
  GPIO_InitTypeDef      GPIO_InitStructure;
#if (!CASE_FTP628MCL401)

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG, ENABLE);

    /* 加热控制 */
	GPIO_InitStructure.GPIO_Pin = HOTBIT123 | HOTBIT456;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* 走纸*/
	GPIO_InitStructure.GPIO_Pin = RPH1 | RPH2 | RPH3 | RPH4;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

    /* 打印机电源控制 */
	GPIO_InitStructure.GPIO_Pin = POWERBIT;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
#if (CASE_PT723F_RJ)
    /* 卷纸电机J 电源控制 */
	GPIO_InitStructure.GPIO_Pin = POWER_J_BIT;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
#endif

 	/* 无纸检测 */
#if (CASE_PT723F_RJ)
	GPIO_InitStructure.GPIO_Pin = PAPERBIT | PAPER_J_BIT;
#else
	GPIO_InitStructure.GPIO_Pin = PAPERBIT;
#endif
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(PAPER_PORT, &GPIO_InitStructure);

#else

    /* GPIOB Pin0,1,6,7,8 */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  GPIO_InitStructure.GPIO_Pin = PHOEBIT | POWERBIT | HOTBIT | RMCL | CMCL;//
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif

}

//初始化打印机相关的控制端口
void  SPI_For_Printer(void)
{
    PRNTempPort_Init();
    PRNDataPort_Init();
    PRNFeedPort_Init();
    PRNPePwPort_Init();
    PRNTIMER_Init();

}

/************************************************************************
** 函数名称: Send_Byte
** 函数功能：通过硬件SPI发送一个字节到SST25VF016B
** 入口参数:data
** 出口参数:无
************************************************************************/
void Send_Byte(uint8_t data)
{
#if (!CASE_FTP628MCL401) //HJ

#ifdef CASE_ER260F
  /*!< Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(PRNData_SPI, SPI_I2S_FLAG_TXE) == RESET);

  /*!< Send byte through the SPI1 peripheral */
  SPI_I2S_SendData(PRNData_SPI, data);

  /*!< Wait to receive a byte */
  //ccr testonly while (SPI_I2S_GetFlagStatus(PRNData_SPI, SPI_I2S_FLAG_RXNE) == RESET);

  /*!< Return the byte read from the SPI bus */
  //ccr return SPI_I2S_ReceiveData(PRNData_SPI);

#else //MCR30 ECR100F
	uint8_t i;

	PRNData_LATCH_LOW();
	PRNData_PCLK_LOW();
	for (i=0; i<8; i++)
	{
		if ((data<<i) & 0x80) //高低在先
			PRNData_PDATA_HIGH();
		else
			PRNData_PDATA_LOW();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		PRNData_PCLK_HIGH();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		PRNData_PCLK_LOW();
	}

	PRNData_LATCH_HIGH();

#endif


#else

	/*!< Send byte through the SPI1 peripheral */
	SPI_I2S_SendData(PRNData_SPI, data); //HJ 先发送数据

	/*!< Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus(PRNData_SPI, SPI_I2S_FLAG_TXE) == RESET); //HJ 再查询发送结束
#endif
}
/************************************************************************
** 函数名称: Send_Word
** 函数功能：通过硬件SPI发送一个字节到SST25VF016B
** 入口参数:data
** 出口参数:无
************************************************************************/
void Send_Word(uint16_t data)
{
#ifdef CASE_ER260F
  /*!< Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(PRNData_SPI, SPI_I2S_FLAG_TXE) == RESET);

  /*!< Send Half Word through the sFLASH peripheral */
  SPI_I2S_SendData(PRNData_SPI, data);

  /*!< Wait to receive a Half Word */
  while (SPI_I2S_GetFlagStatus(PRNData_SPI, SPI_I2S_FLAG_RXNE) == RESET);

  /*!< Return the Half Word read from the SPI bus */
  //ccr return SPI_I2S_ReceiveData(PRNData_SPI);
#else //MCR30
	//
	//
#endif
}

void Print_Initial( void )
{
//    memset(&InfoSelf_Graph,0,sizeof(InfoSelf_Graph));//ccr2014-03-25

    InfoSelf_Graph.cGrapCols = 0;
    InfoSelf_Graph.cGrapSize = 0;

    CLONG(Prn_Status) = 0;

#if ENCUTTER
    CWORD(Cut_Status) = 0;
    Cut_Status.g_bFirst = 1;
#endif

    Prn_Status.g_bPrintOver = true;//not any data in POOL, POOL is empty
    Prn_Status.g_bGraphicOver = true;  //  A task for print graphic finished
#if defined(CASE_FORHANZI)
    Prn_HZTime = 0;
#endif
    memset(&DataFromHost,0,sizeof(DataFromHost));
    memset(&InfoSelf_Graph,0,sizeof(InfoSelf_Graph));
#if DEBUGONPC
#else
    SPI_For_Printer(); //initial for printer

    PrnHotOFF();

    Drive_Off();
    PrnPowerOFF();

    Reset_printer();

#endif
	SetPrintDensity(2);

//    uSECOND = SystemCoreClock / 4000000;
}

/****************************************************************
  Name: TESTPAPER  检测是否有纸;
 return: true-有纸; false-无纸
*****************************************************************/
BYTE TESTPAPER(void)
{
    ULONG t1,t2;
#if (!CASE_FTP628MCL401)

#if (CASE_PT486F)
    do {
        t1 = (PAPER_PORT->IDR & PAPERBIT);//ouhs 20180323 (GPIOD->IDR & PAPERBIT);
        for (t2=10;t2>0;t2--);//延迟
        t2 = (PAPER_PORT->IDR & PAPERBIT);//(GPIOD->IDR & PAPERBIT);
    } while (t1!=t2);

   	if (t1)
        return true;  // 无纸
   	else
        return false;  // 有纸

#elif (CASE_PT723F_RJ)
    do {
        t1 = (GPIOG->IDR & (PAPERBIT | PAPER_J_BIT));
        for (t2=10;t2>0;t2--);//延迟
        t2 = (GPIOG->IDR & (PAPERBIT | PAPER_J_BIT));
    } while (t1!=t2);

	if (t1 == (PAPERBIT | PAPER_J_BIT))
        return true;  // 无纸
   	else
        return false;  // 有纸
#endif

#else
    do {
        t1 = *(uint8_t *)PRT_PH_READ_ADDR & PER;
        t2 = t1;
        t2 = *(uint8_t *)PRT_PH_READ_ADDR & PER;
    } while (t1!=t2);

   	if (t1)
        return true;  // 无纸
   	else
        return false;  // 有纸

#endif

}

#if CASE_HANDUP
/****************************************************************
  Name: TESTHAND  检测压杆状态
 return: true-压杆正常; false-压杆异常
*****************************************************************/
BYTE TESTHAND(void)
{
    ULONG t1,t2;
#if (!CASE_FTP628MCL401) //OHS
        return true;  //hand down
//    do {
//        t1 = (LPC_GPIO0->FIOPIN & CASE_HANDUP);????
//        t2 = t1;
//        t2 = (LPC_GPIO0->FIOPIN & CASE_HANDUP);????
//    } while (t1!=t2);

#else

    do {
        t1 = *(uint8_t *)PRT_PH_READ_ADDR & HUPR; //(LPC_GPIO0->FIOPIN & CASE_HANDUP);????
        for (t2=10;t2>0;t2--);//延迟
        t2 = *(uint8_t *)PRT_PH_READ_ADDR & HUPR; //(LPC_GPIO0->FIOPIN & CASE_HANDUP);????
    } while (t1!=t2);

   	if (t1)
        return false;  // 压杆抬起
   	else
        return true;  //hand down
#endif

}
#endif
// read status of the print
void Check_Print()
{

    Prn_Status.g_bHANDUP = 0;

    if (!TESTPAPER())//paper out
    {
        Prn_FeedCount = 0;
        Prn_Status.g_bNOPAPER = true;
        Prn_Status.g_bPaperIn = false;
    }
    else
    {
        if (Prn_Status.g_bNOPAPER)
        {
            Prn_FeedCount = 0;
            Prn_Status.g_bPaperIn = true;
            Prn_Status.g_bWaitPaper = false;
        }
        Prn_Status.g_bNOPAPER = false;
    }

    if (Prn_FeedCount>FEEDMAX)
        Prn_Status.g_bTOOHOT = true;

#if CASE_HANDUP
    if (!TESTHAND())//paper out
    {
        Prn_Status.g_bHANDUP = true;
        Prn_Status.g_bHANDDN = false;
    }
    else
    {
        if (Prn_Status.g_bHANDUP)
        {
            Prn_Status.g_bWaitPaper = false;
            Prn_Status.g_bHANDDN = true;
        }
        Prn_Status.g_bHANDUP = false;
    }
#endif
}
/***********************************************************************
  Combine a BYTE with full byte if needed
  comFor:XXXX|....| >>>>> XXXXnnnn
  comWith:   |nnnn|mmmm   >>>>> mmmm....
************************************************************************/
void CombineWith8(BYTE *comFor,BYTE comWith,BYTE comFlag)
{
    if (comFlag)
    {// must combine withe the last byte
        comFor[0] = comFor[0] | (comWith>>4);
        comFor[1] = comWith<<4;
    }
    else
    {// don't combine with the last byte
        comFor[0] = comWith;
    }
}
/***********************************************************************
  Combine a BYTE with half byte if needed
  comFor:XXXX|....| >>>>> XXXXnnnn
  comWith:   |nnnn|....
************************************************************************/
BYTE CombineWith4(BYTE *comFor,BYTE comWith,BYTE *comFlag)
{
    if (*comFlag)
    {// must combine withe the last byte
        *comFor |=  (comWith>>4);
        *comFlag = false;// don't combine withe the last byte next time
        return 1;
    }
    else
    {// don't combine with the last byte
        *comFor = comWith;
        *comFlag = true;// must combine withe the last byte next ime
        return 0;
    }
}
/***********************************************************************
  Create dots for  the characters
************************************************************************/
void Create384Dots(void)
{
#if (defined(CASE_FORHANZI) )
    BYTE HanZiKu_Data[3];
#endif
    BYTE temp[2];
	BYTE * fontLib = NULL;

    int i,j;
    ULONG idx;
    BYTE bCombine=false;//must Combine with the last BYTE if it is true

    //Prn_LineDots = 0;
	if (FontsOffset[0] & BODFLAG)
		fontLib=(BYTE *)ASCII12x24BOLD;
	else
		fontLib=(BYTE *)ASCII12x24;

    memset(DotsOfCurrent,0,sizeof(DotsOfCurrent));
    if (Prn_LineIndex<TEXT_HIGH)
    {
        for (i=j=0;i<Prn_LineChars && j<BYTES_OF_PAPER_LINE;i++)
        {
            idx = FontsOffset[i];//get the FontsOffset of the fonts
#if (defined(CASE_FORHANZI) )
            if ((idx & HANZFLAG))//it is a chinese
            {
                idx += Prn_LineIndex*3;
                ReadHZKU(idx & 0x03fffff,HanZiKu_Data);
                if (idx & DBWFLAG)//double with print mode
                {
                    i+=3;
//                    idx &= 0x00ffffff;
                    CWORD(temp[0]) = DoubleByte[HanZiKu_Data[0]];
                    CombineWith8(&DotsOfCurrent[j],temp[1],bCombine);
                    if (++j>=BYTES_OF_PAPER_LINE) break;
                    CombineWith8(&DotsOfCurrent[j],temp[0],bCombine);
                    if (++j>=BYTES_OF_PAPER_LINE) break;
                    CWORD(temp[0]) = DoubleByte[HanZiKu_Data[1]];
                    CombineWith8(&DotsOfCurrent[j],temp[1],bCombine);
                    if (++j>=BYTES_OF_PAPER_LINE) break;
                    CombineWith8(&DotsOfCurrent[j],temp[0],bCombine);
                    if (++j>=BYTES_OF_PAPER_LINE) break;
                    CWORD(temp[0]) = DoubleByte[HanZiKu_Data[2]];
                    CombineWith8(&DotsOfCurrent[j],temp[1],bCombine);
                    if (++j>=BYTES_OF_PAPER_LINE) break;
                    CombineWith8(&DotsOfCurrent[j],temp[0],bCombine);
                    ++j;
                }
                else
                {
                    i++;
//                    idx &= 0x00ffffff;

                    CombineWith8(&DotsOfCurrent[j],HanZiKu_Data[0],bCombine);
                    if (++j>=BYTES_OF_PAPER_LINE) break;
                    CombineWith8(&DotsOfCurrent[j],HanZiKu_Data[1],bCombine);
                    if (++j>=BYTES_OF_PAPER_LINE) break;
                    CombineWith8(&DotsOfCurrent[j],HanZiKu_Data[2],bCombine);
                    ++j;
                }
            }
            else//It is a ASCII
#endif
            {
                idx += Prn_LineIndex*ASCIINUM;
                if (idx & DBWFLAG)//double with print mode
                {
                    i++;
                    idx &= 0x00ffffff;
                    CWORD(temp[0]) = DoubleByte[fontLib[idx++]];
                    CombineWith8(&DotsOfCurrent[j],temp[1],bCombine);
                    if (++j>=BYTES_OF_PAPER_LINE) break;
                    CombineWith8(&DotsOfCurrent[j],temp[0],bCombine);
                    if (++j>=BYTES_OF_PAPER_LINE) break;

                    CWORD(temp[0]) = DoubleByte[fontLib[idx]];
                    CombineWith8(&DotsOfCurrent[j],temp[1],bCombine);
                    ++j;
                }
                else//normal width print mode
                {
                    //|******** xxxx....|(765432107654....)
                    idx &= 0x00ffffff;
                    temp[0] = fontLib[idx++];//get the 8 dots for the left
                    CombineWith8(&DotsOfCurrent[j],temp[0],bCombine);
                    if (++j>=BYTES_OF_PAPER_LINE) break;
                    temp[0] = fontLib[idx];// get the 4 dots for the right
                    j+=CombineWith4(&DotsOfCurrent[j],temp[0],&bCombine);
                }
            }
        }
#if CASE_PT723F_RJ

#define DotsOfCurrent_Stubs ((BYTE *)(DotsOfCurrent+ (BYTES_OF_HEAT_LINE-BYTES_OF_PAPER_LINE) ) )

		//HJ 选择票据打印机//
		if(BIT(ApplVar.Prn_Command, PRN_RECEIPT_Mode))
		{
		    memcpy(DotsOfCurrent_Stubs, DotsOfCurrent, BYTES_OF_PAPER_LINE);
		}

		//HJ 不选择存根打印机//
		if(!BIT(ApplVar.Prn_Command, PRN_STUB_Mode))
		{
		    memset(DotsOfCurrent, 0, BYTES_OF_PAPER_LINE);
		}
#endif
    }
}

/***********************************************************************
  Create font offset of all of the characters current
************************************************************************/
void SetFontOffset_char(BYTE count)
{
    WORD qh,ql;
    BYTE *lineDots = ApplVar.PoolForPrint[Prn_PoolCurrent].data;
    WORD pCmd = ApplVar.PoolForPrint[Prn_PoolCurrent].Prn_Command;//ccr2018-07-31
    int i,j=0;

    BYTE dbW=false;   //double wodth falg for '~'

#if defined(CASE_FORHANZI)
    Prn_HZTime = PDATATIME;
#endif

    for (i=0;i<count && j<PRNWIDTH;i++)
    {
        qh = lineDots[0];

        if (qh=='~' && !dbW && !BIT(pCmd,PRN_DBLW_Mode))//ccr2018-07-31
            dbW = true;
        else
        {
#if defined(CASE_FORHANZI)
            if (qh >= 0xa0)//IT is a HANZI
            {//0xb0+1+4;//"啊":0xa1b0
                Prn_HZTime += HZTIMES;

                lineDots++;
                ql =lineDots[0];

                ql -= 0xa1;
                qh -= 0xa1;
                if (qh>3) qh -= (15-4);

                qh= qh * 94 + ql;

                FontsOffset[j]=(128*ASCII_SIZE + ((ULONG)qh*HZ_SIZE)+ FLASH_HZK24_ADDR) | HANZFLAG;

                if (dbW || BIT(pCmd,PRN_DBLW_Mode))//ccr2018-07-31
                {//double width for hanzi
                    FontsOffset[j] |= DBWFLAG;
                    j+=2;//skip 2 characters
                }
                j++;
                i++;
            }
            else
#endif
            {
#if defined(CASE_FORHANZI)
                Prn_HZTime += ASCTIMES;
#endif
                if (qh>=ASCIIFROM)
                    qh -= ASCIIFROM;
                else
                    qh = 0;//ccr2018-01-22
                FontsOffset[j] = qh*2;
                if (dbW || BIT(pCmd,PRN_DBLW_Mode))//ccr2018-07-31
                {
                    FontsOffset[j] |= DBWFLAG;
                    j++;
                }
            }
            j++;
            dbW= false;
        }
        lineDots++;
    }
    if (j>=PRNWIDTH)
        j = PRNWIDTH;
    if (BIT(ApplVar.Prn_Command,PRN_DBLH_Mode))
    {//set double high flag
        SETBIT(FontsOffset[0],DBHFLAG);
    }

	if(BIT(ApplVar.Prn_Command, PRN_BOLD_MODE))
    {
        //set BOLD flag
        SETBIT(FontsOffset[0], BODFLAG);
    }
	
    Prn_LineChars = j;
}
#if CASE_PT723F_RJ
#define BYTES_OF_SHUANG_ZHAN_GAP (BYTES_OF_HEAT_LINE - BYTES_OF_PAPER_LINE - BYTES_OF_PAPER_LINE)
void Send_blank_dot (BYTE dotNum )
{
	BYTE i;
#if (WORDorBYTE)
    for(i=0; i<dotNum; i++)
    {
    	Set_Hot_DataW( (WORD)0, i++);
	}
#else
    for(i=0; i<dotNum; i++)
    {
    	Set_Hot_Data( (BYTE)0, i);
	}
#endif
}
#endif
/**********************************************************************
  Get dots from the graphic
**********************************************************************/
void CreateGraphicDots()
{
    int i,j;
#if CASE_PT723F_RJ
    BYTE f_repeatOnce = 1;
#endif

#if defined(CASE_FORHANZI)
    Prn_HZTime = PDATATIME;
#endif
    //Prn_LineDots = 0;
    if (Graph_DotsLineMax || Graph_Remain)
    {
        j = Graph_LineCurrent*Graph_Width-Graph_PosX+Graph_Offset;
        if (Graph_DotsLineMax==0)
        {
            Graph_Width = Graph_Remain;
            Graph_Remain = 0;
        }
        else
        {
            Graph_LineCurrent++;
            Graph_DotsLineMax--;
        }
        //开始送点阵之前,进行必要的初始化
        Prn_LineDots = 0;
#if GROUPMODE==2
        memset(Prn_GroupDots,0,sizeof(Prn_GroupDots));
#endif

#if CASE_PT723F_RJ
		//HJ 如果不打印左侧，则传送空白点行数据//
		if(!BIT(ApplVar.Prn_Command, PRN_STUB_Mode))
		{
			Send_blank_dot(BYTES_OF_PAPER_LINE + BYTES_OF_SHUANG_ZHAN_GAP);
			f_repeatOnce = 0;
		}
		else
		{
			f_repeatOnce = 1;
		}
#endif

		//HJ 传送可见点行数据//
#if WORDorBYTE
CreateGraphicDots_1:
        for (i=0;i<BYTES_OF_PAPER_LINE;i+=2,j+=2)
        {
            if ((i>=Graph_PosX)&&(i<(Graph_Width+Graph_PosX)))
            {
                if ((i+1-Graph_PosX)<Graph_Width)
                    Set_Hot_DataW( ((WORD)pGraBuf[j]<<8)+pGraBuf[j+1] , i);
                else
                    Set_Hot_DataW( ((WORD)pGraBuf[j]<<8) , i);
            }
            else
            {
                Set_Hot_DataW( 0 , i );
            }
        }

#if CASE_PT723F_RJ
		//HJ 打印相同的内容//
		if(f_repeatOnce == 1)
		{
			f_repeatOnce = 0;

			Send_blank_dot(BYTES_OF_SHUANG_ZHAN_GAP);
			j -= BYTES_OF_PAPER_LINE;
			goto CreateGraphicDots_1;
		}
#endif

#else //HJ 字节SPI方式//
CreateGraphicDots_2:

        for (i=0;i<BYTES_OF_PAPER_LINE;i++,j++)
        {
            if ((i>=Graph_PosX)&&(i<(Graph_Width+Graph_PosX)))
            {
                Set_Hot_Data( pGraBuf[j] , i);
            }
            else
            {
                Set_Hot_Data( 0 , i );
            }
        }

#if CASE_PT723F_RJ
		//HJ 打印相同的内容//
		if(f_repeatOnce == 1)
		{
			f_repeatOnce = 0;

			Send_blank_dot(BYTES_OF_SHUANG_ZHAN_GAP);
			j -= BYTES_OF_PAPER_LINE;
			goto CreateGraphicDots_2;

		}
#endif

#endif


#if GROUPMODE==2
        //送完最后一个字节时,设定加热组属性数据
        Set_Hot_Group();
#endif

		//HJ 锁存、加热并计时
        Start_Hot();
    }
    if ((!Graph_DotsLineMax)&&(!Graph_Remain))
    {
        Graph_LineCurrent = 0;
        Prn_Status.g_bGraphicOver = true;//  A task for print graphic finished
    }
}

/**********************************************************************
  Attch text dots on the graphic HJ 在图形上,附加文本点
**********************************************************************/
void CreatGraphicWithText()
{
    int i,j;
#if WORDorBYTE
    WORD char_temp = 0;
#endif

    BYTE char_flag = 0;
    BYTE *linep=DotsOfCurrent;

#if CASE_PT723F_RJ
    BYTE f_repeatOnce = 1;
#endif

    if ((!Prn_LineIndex)&&(!Feed_BlankLines))
    {
        if (TextIdx_ForGraph < Prn_GraphTextMax)
        {
//			if(ApplVar.PoolPushIn != ApplVar.PoolPopOut)
            {
                if (++Prn_PoolCurrent>=MAX_PRNPOOL) Prn_PoolCurrent = 0;
                SetFontOffset_char(ApplVar.PoolForPrint[Prn_PoolCurrent].Length);
                TextIdx_ForGraph++;
                Prn_LineIndex=0;
            }
        }
    }
    if (TextIdx_ForGraph <= Prn_GraphTextMax)
    {
        if (Prn_LineIndex<TEXT_HIGH)
        {
            Create384Dots();
            Prn_LineIndex++;
            char_flag = 1;
        }
        else
        {
            memset(DotsOfCurrent,0,sizeof(DotsOfCurrent));

            if (Feed_BlankLines<LINE_BLANK)
                Feed_BlankLines++;
            else
            {
                Feed_BlankLines = 0;
                Prn_LineIndex = 0;
            }
            if (TextIdx_ForGraph == Prn_GraphTextMax)
            {//需要叠加的字符行叠加完毕
                TextIdx_ForGraph = Prn_GraphTextMax = 0;
            }
            char_flag = 0;
        }
    }
    //Prn_LineDots = 0;
    if (Graph_DotsLineMax || Graph_Remain)
    {
        j = Graph_LineCurrent*Graph_Width-Graph_PosX+Graph_Offset;
        if (Graph_DotsLineMax==0)
        {
            Graph_Width = Graph_Remain;
            Graph_Remain = 0;
        }
        else
        {
            Graph_LineCurrent++;
            Graph_DotsLineMax--;
        }

        //开始送点阵之前,进行必要的初始化
        Prn_LineDots = 0;
#if GROUPMODE==2
        memset(Prn_GroupDots,0,sizeof(Prn_GroupDots));
#endif

#if CASE_PT723F_RJ
		//HJ 如果不打印左侧，则传送空白点行数据//
		if(!BIT(ApplVar.Prn_Command, PRN_STUB_Mode))
		{
			Send_blank_dot(BYTES_OF_PAPER_LINE + BYTES_OF_SHUANG_ZHAN_GAP);
			f_repeatOnce = 0;
		}
		else
		{
			f_repeatOnce = 1;
		}
#endif

#if WORDorBYTE
CreatGraphicWithText_1:
        for (i=0;i<BYTES_OF_PAPER_LINE;i+=2,j+=2)
        {
            if (char_flag)
                char_temp = ((WORD)linep[0]<<8)+linep[1];
            if ((i>=Graph_PosX)&&(i<(Graph_Width+Graph_PosX)))
            {
                if (((i+1)-Graph_PosX)<Graph_Width)
                    Set_Hot_DataW( ((WORD)pGraBuf[j]<<8)+pGraBuf[j+1] | char_temp, i);
                else
                    Set_Hot_DataW( ((WORD)pGraBuf[j]<<8) | char_temp , i);
            }
            else
            {
                Set_Hot_DataW( char_temp , i );
            }
            linep+=2;
        }

#if CASE_PT723F_RJ
		//HJ 打印相同的内容//
		if(f_repeatOnce == 1)
		{
			f_repeatOnce = 0;

			Send_blank_dot(BYTES_OF_SHUANG_ZHAN_GAP);
			j -= BYTES_OF_PAPER_LINE;
			linep -= BYTES_OF_PAPER_LINE;
			goto CreatGraphicWithText_1;

		}
#endif

#else //HJ SPI基于字节发送//
CreatGraphicWithText_2:
		//HJ 传送一点行数据
        for (i=0;i<BYTES_OF_PAPER_LINE;i++,j++)
        {
            if ((i>=Graph_PosX)&&(i<(Graph_Width+Graph_PosX)))
            {
                Set_Hot_Data( *linep | pGraBuf[j] , i );
            }
            else
            {
                Set_Hot_Data( *linep , i );
            }
            linep++;
        }

#if CASE_PT723F_RJ
		//HJ 打印相同的内容//
		if(f_repeatOnce == 1)
		{
			f_repeatOnce = 0;

			Send_blank_dot(BYTES_OF_SHUANG_ZHAN_GAP);
			j -= BYTES_OF_PAPER_LINE;
			linep -= BYTES_OF_PAPER_LINE;
			goto CreatGraphicWithText_2;

		}
#endif

#endif


#if GROUPMODE==2
        //送完最后一个字节时,设定加热组属性数据
        Set_Hot_Group();
#endif

    }
    else
    {

        //开始送点阵之前,进行必要的初始化
        Prn_LineDots = 0;
#if GROUPMODE==2
        memset(Prn_GroupDots,0,sizeof(Prn_GroupDots));
#endif

        if (char_flag)
        {

#if CASE_PT723F_RJ
			//HJ 如果不打印左侧，则传送空白点行数据//
			if(!BIT(ApplVar.Prn_Command, PRN_STUB_Mode))
			{
				Send_blank_dot(BYTES_OF_PAPER_LINE + BYTES_OF_SHUANG_ZHAN_GAP);
				f_repeatOnce = 0;
			}
			else
			{
				f_repeatOnce = 1;
			}
#endif

#if WORDorBYTE
CreatGraphicWithText_3:
			for (i=0;i<BYTES_OF_PAPER_LINE;i+=2)
            {
                Set_Hot_DataW( ((WORD)linep[0]<<8)+ linep[1], i );
                linep+=2;
            }

#if CASE_PT723F_RJ
			//HJ 打印相同的内容//
			if(f_repeatOnce == 1)
			{
				f_repeatOnce = 0;

				Send_blank_dot(BYTES_OF_SHUANG_ZHAN_GAP);
				linep -= BYTES_OF_PAPER_LINE;
				goto CreatGraphicWithText_3;
			}
#endif

#else //HJ 字节SPI方式//
CreatGraphicWithText_4:

            for (i=0;i<BYTES_OF_PAPER_LINE;i++)
            {
                Set_Hot_Data( *linep , i );
                linep++;
            }

#if CASE_PT723F_RJ
			//HJ 打印相同的内容//
			if(f_repeatOnce == 1)
			{
				f_repeatOnce = 0;

				Send_blank_dot(BYTES_OF_SHUANG_ZHAN_GAP);
				linep -= BYTES_OF_PAPER_LINE;
				goto CreatGraphicWithText_4;
			}
#endif

#endif
        }


        else
        {
#if WORDorBYTE
            for (i=0;i<BYTES_OF_HEAT_LINE;i+=2)
            {
                Set_Hot_DataW( 0 , i );
            }
#else
            for (i=0;i<BYTES_OF_HEAT_LINE;i++)
            {
                Set_Hot_Data( 0 , i );
            }
#endif
        }


#if GROUPMODE==2
        //送完最后一个字节时,设定加热组属性数据
        Set_Hot_Group();
#endif

        if (TextIdx_ForGraph >= Prn_GraphTextMax && !Prn_LineIndex && !Feed_BlankLines)
        {
            TextIdx_ForGraph = 0;
            Graph_LineCurrent = 0;
            Prn_GraphTextMax = 0;
            Prn_Status.g_bGraphicOver = true;//  A task for print graphic finished
        }
//		TextIdx_ForGraph = 0;
//		Graph_LineCurrent = 0;
    }

	//HJ 锁存、加热并计时
    Start_Hot();
}
/*************************************************
 * 设置清除打印任务标志,用于结束所有打印
 *
 * @author EutronSoftware (2018-01-26)
 *
 ************************************************/
void ClearAllOfPrintTask()
{
    Prn_Status.g_bClean=0;
    ApplVar.PoolPopOut=ApplVar.PoolPushIn;
}
/************************************************
 * 从打印队列中获取一条打印任务
 *
 * @author EutronSoftware (2018-01-26)
 ***********************************************/
void GetTaskFromPool()
{
    BYTE sGraph_idx,char_cnt,sGrphPos;
		struct GRAPH_DEFINE * sGraInfo;

#if POWERCTRL
    pwrGetStatus(GET_VIN_STATUS);//ccr20131015
#endif

    Prn_LineIndex = 0;///////////////

    //ccr2018-01-26>>>>>>>>>>>>>>>>>>>>>>>>
    if (Prn_Status.g_bClean)
    {
         ApplVar.PoolPopOut = ApplVar.PoolPushIn;
         Prn_PoolCurrent=ApplVar.PoolPushIn;
         Prn_Status.g_bClean=0;
    }
    //ccr2018-01-26<<<<<<<<<<<<<<<<<<<<<<<<
    if (ApplVar.PoolPushIn == ApplVar.PoolPopOut)
    {//无打印任务
        Prn_Status.g_bPrintOver = true;
        Prn_FeedCount = 0;
    }
    else
    {
        Prn_PoolCurrent = (ApplVar.PoolPopOut + 1);
        if (Prn_PoolCurrent>=MAX_PRNPOOL) Prn_PoolCurrent = 0;

        sGraph_idx = ApplVar.PoolForPrint[Prn_PoolCurrent].GraphicNo;
        char_cnt = ApplVar.PoolForPrint[Prn_PoolCurrent].Length;
        ApplVar.Prn_Command = ApplVar.PoolForPrint[Prn_PoolCurrent].Prn_Command;

        if (sGraph_idx)// && BIT(ApplVar.Prn_Command,PRN_GRAPHIC_Mode) //print graphic
        {
#if BARCODESIZE
            if (sGraph_idx==BARCODE_GRP_ID)// && InfoBarCode_Graph.cGrapCols)
            {//=BARCODE_GRP_ID时，为条码图片,数据将占用Self_Graph
                pGraBuf = BarCode_Graph;
                sGraInfo =&InfoBarCode_Graph;
                CreateBarcodeImage(ApplVar.PoolForPrint[Prn_PoolCurrent].data+2,ApplVar.PoolForPrint[Prn_PoolCurrent].data[1]);
                SelfGrpReady=0;
            }
            else
#endif
            if (sGraph_idx>1 && sGraph_idx<=GRAPHICMAX)
            {
                pGraBuf = (BYTE*)Graphic_Data[sGraph_idx-1];
                sGraInfo =(struct GRAPH_DEFINE *)gra_info[sGraph_idx-1];
            }
            else if (GRB_Define.cGraSize>0 && GRB_Define.cTextFlag==0x80)//ccr2018-02-01InfoSelf_Graph.cGrapCols)
            {//=1时，为用户自定义图片而且有用户自定义图片数据
                if (!SelfGrpReady) //图片数据未转换
                    ChangeGRBToMyself();//先转换图片数据

                pGraBuf = Self_Graph;
                sGraInfo =&InfoSelf_Graph;
            }
            else//无自定义图片,默认打印第一幅图片
            {
                pGraBuf = (BYTE*)Graphic_Data[0];
                sGraInfo =(struct GRAPH_DEFINE *)gra_info[0];
            }

            if (sGraInfo->cGrapCols)
            {//为有效图片数据
                Prn_Status.g_bPrintGraph = true;
                Prn_Status.g_bGraphicOver = false;
                sGrphPos = ApplVar.PoolForPrint[Prn_PoolCurrent].data[0];//图片打印位置

                Prn_GraphTextMax = char_cnt;//text lines for graphic

                Graph_Offset = (sGraInfo->cLabels) * (sGraInfo->cByteALine + 1);//根据每行字符数计算图像点阵的窜方位置
                Graph_SizeInBytes = sGraInfo->cGrapSize-Graph_Offset;
                Graph_Width = sGraInfo->cGrapCols*sGraInfo->cWidthAChar/8;//为以字节为单位的图片宽度
                if (sGrphPos)
                    Graph_PosX = sGrphPos*sGraInfo->cWidthAChar/8;//使用指定打印位置(字节位置)
                else
                    Graph_PosX = sGraInfo->cPrintAt*sGraInfo->cWidthAChar/8;//使用默认打印位置
                Graph_DotsLineMax = Graph_SizeInBytes / Graph_Width;
                Graph_Remain = Graph_SizeInBytes % Graph_Width;
                Graph_LineCurrent = 0;
                TextIdx_ForGraph = 0;
                Feed_BlankLines = 0;

                if (Prn_GraphTextMax)
                    CreatGraphicWithText();
                else
                    CreateGraphicDots();
            }
            //为无效图片，Prn_LineIndex=0, 走纸空行
/*
            else
            {//为无效图片，打印一个空行
                ApplVar.PoolForPrint[Prn_PoolCurrent].data[0]=' ';
                ApplVar.PoolForPrint[Prn_PoolCurrent].Length = 1;
                ApplVar.PoolForPrint[Prn_PoolCurrent].Prn_Command = CMDP_R;
                ApplVar.Prn_Command = CMDP_R;
                SetFontOffset_char(1);
            }
*/
        }
#if ENCUTTER
        else if (BIT(ApplVar.Prn_Command,PRN_CUTRESET_Mode | PRN_CUTF_Mode ))
        {//reset cutter,cut paper
            Start_Motor(ApplVar.Prn_Command);
        }
#endif
        else
        {
            SetFontOffset_char(char_cnt);
        }
    }
}
//检测打印纸并显示错误提示
void Check_Paper(void)
{
    static BYTE beep=0;
    //缺纸检测和处理
    Get_Printer_status();
	  #if 0 //jdb2018-09-02
    if (Printer_Status.g_bDisconnect)
    {//断开//ccr2018-04-02
        SETBIT(beep,BIT0);//Bell(0);
        if ((ApplVar.ErrorNumber&0x7fff)!=ERROR_ID(CWXXI126))
        {
           ApplVar.ErrorNumber=ERROR_ID(CWXXI126);
           Fiscal_AddDisconnect(FISxPRINTERCOUNT);
        }
#if !defined(CASE_ASCIIDISPLAY)
        DispSt1C(Msg[CWXXI126].str);
#else
#if DISP2LINES
        Puts1_Only(Msg[CWXXI126].str);
#else
        PutsO_Only(Msg[CWXXI126].str);
#endif
#endif
#if defined(CASE_GREECE)//ccr2018-07-27>>>>>>
        Stop_Motor();
        if (BIT(ApplVar.Fiscal_PrintFlag,(BIT0 | BIT2)))
        {//打印Z报表时如果缺纸,需要重新打印Z报表
            SETBIT(ApplVar.Fiscal_PrintFlag,BIT7);
        }
        while (true)
        {
            CheckError(0);
        }
#endif//ccr2018-07-27<<<<<
	}
    else 
			#endif
		if (Printer_Status.g_bNOPAPER)
    {//无纸
        SETBIT(beep,BIT0);//Bell(0);
#if !defined(CASE_ASCIIDISPLAY)
        DispSt1C(Msg[CWXXI40].str);
#else
#if DISP2LINES
        Puts1_Only(Msg[CWXXI40].str);
#else
        PutsO_Only(Msg[CWXXI40].str);
#endif
#endif

#if defined(FISCAL)
        if (BIT(ApplVar.Fiscal_PrintFlag,(BIT0 | BIT2)))
        {//打印Z报表时如果缺纸,需要重新打印Z报表
            SETBIT(ApplVar.Fiscal_PrintFlag,BIT7);
        }
#endif

    }
    else if (Printer_Status.g_bPaperIn)
    {//重新装入打印纸
#if defined(CASE_ASCIIDISPLAY)
        PutsO_Saved();
#else
        ClsState1();
#endif
        Printer_Status.g_bPaperIn = 0;
    }
#if CASE_HANDUP
    else if (Printer_Status.g_bHANDUP)
    {//压杆抬起
        SETBIT(beep,BIT0);//Bell(0);
#if defined(CASE_ASCIIDISPLAY)
        PutsO_Only(Msg[CWXXI39].str);
#else
        DispSt1C(Msg[CWXXI39].str);
#endif
    }
    else if (Prn_Status.g_bHANDDN)
    {//压杆抬起后压下
#if defined(CASE_ASCIIDISPLAY)
        PutsO_Saved();
#else
        ClsState1();
#endif
        Printer_Status.g_bHANDDN = 0;
    }
#endif
    //ccr2018-05-31>>>>>>>避免连续报警
    if (beep==BIT0)
    {
        Bell(0);
        beep=BIT1;
    }
    else if (beep==BIT1)
    {
        beep=0;
    }
    else
        CLRBIT(beep,BIT0);
    //ccr2018-05-31<<<<<<<<<<<<<
}
/***********************************************************************************
// push data into the POOL
// INPUT:
//      *buf:   Text data for print
//      count:  Bytes in *buf must be print out
//      prnCMD: Command for TEXT ,must be one of:
                -PRN_DBLW_Mode        //double with print mode for text
                -PRN_DBLH_Mode        //double high print mode for text
                -PRN_RECEIPT_Mode     //print on receipt printer
                -PRN_STUB_Mode        //print on stub printer
                -PRN_GRAPHIC_Mode     //print a graphic
                -PRN_CUTF_Mode        //Cut paper full
                -PRN_RECEIPT_LINE     //ccr2018-01-31 打印收据行
                -PRN_CUTRESET_Mode    //Cutter reset


// return:<0;POOL is full & the data don't push in
//        >=0;data has pushed into the pool and the value is the
//        length of the data

//REMARK:
//    if prnCMD is the GRAPHIC command:
//        buf[0] must be the number of the graphic.(1,2,3....)
//               buf[0]=0,stop attach the text;
//        buf[1] must be '0'(don't attache text)
//              or '1' (8 lines of Text must attache for the graphic)
//              or '2' (copy text from graphic)
//       buf[2] Postion by character of the graphic on paper(0,1,2,..,32), It's not the must,
//              =0,Print at the default position defind by the graphic
//    if prnCMD is the CUT PAPER command:
//        buf[0] must be the FULL(1) cut or HALF cut(0).
//
********************************************************************************/
#if 0 //jdb2018-09-02
int RSPrint(BYTE *buf, int count, BYTE prnCMD)
{
    struct GRAPH_DEFINE *sGraInfo;
    BYTE *sGraph;
    struct TPRN_POOL *sPrnPool;

    BYTE i;
    int  retval; /* value used in "goto out" statements */

#if (0)	//ccr2014-10-23testonly>>>>>>>>>>>>>>>
    PrnPowerOFF();
	if (buf) {
        xputs(buf);	xputc('\n');
	}
	return 1;
#else  //<<<<<<<<<<<<<<<<ccr2014-10-23testonly

    Check_Paper();
    if ((ApplVar.PoolPushIn + Graph_TextCount + 1)%MAX_PRNPOOL == ApplVar.PoolPopOut)
    {
        do
        {
//            rtc_Delayms(10);
//			msDelay(1000);

            i = (ApplVar.PoolPushIn + Graph_TextCount + 1)%MAX_PRNPOOL;
            if (i>ApplVar.PoolPopOut)
                i -= ApplVar.PoolPopOut;
            else
                i = ApplVar.PoolPopOut - i;
            Check_Paper();
        } while (i<3);//等待队列空余3个空间
    }

    if (count > PRNWIDTH+1)//include double high flag '@'
        count = PRNWIDTH+1;

    retval = 0;//set the flag: does not copy to pool

#if ENCUTTER
    if (BIT(prnCMD,PRN_CUTF_Mode | PRN_CUTH_Mode | PRN_CUTRESET_Mode))//Print graphic
    {
        if (++ApplVar.PoolPushIn>=MAX_PRNPOOL) ApplVar.PoolPushIn = 0;
        sPrnPool = &ApplVar.PoolForPrint[ApplVar.PoolPushIn];

        sPrnPool->Prn_Command = prnCMD;
        sPrnPool->GraphicNo = 0;
        sPrnPool->Length = 0;

        Start_Motor(prnCMD);
        return retval;

    }
#endif
    if (BIT(prnCMD,PRN_GRAPHIC_Mode) && !Prn_Status.g_bTextForGraph)//Print graphic
    {
        {//为打印图片命令
            if (count<=2)
                buf[2] = 0;// Use the default position
            if (count>=2 && buf[0]>0)
            {
                if (buf[1] == '0')//Print graphic  without textor
                {
                    Prn_Status.g_bTextForGraph = false;//no text fot over-print
                }
                else if (buf[1] == '1' || buf[1] == '2')//Print graphic with text
                {
                    Prn_Status.g_bTextForGraph = true;//set the flag for over-print text
                }
                memset(&GraphForPrint,0,sizeof(GraphForPrint));
#if (DOWN_GR_SIZE)
                if (buf[0]==1 //=1时，为用户自定义图片
                    && GRB_Define.cGraSize>0 && GRB_Define.cTextFlag==0x80 //有用户自定义图片数据
                    && InfoSelf_Graph.cGrapCols==0) //图片数据未转换
                    ChangeGRBToMyself();//先转换图片数据
#endif
                if (Prn_Status.g_bTextForGraph && buf[0]<=GRAPHICMAX)//=1时，为用户自定义图片
                {// 当打印的图像需要叠加文字时,把打印数据暂时存储在GraphForPrint中
                 // GraphForPrint[0]保存要打印的图像命令,其它GraphForPrint保存叠加的文字
                    Graph_TextCount = 0;
                    GraphForPrint[0].Length = 1;    //flag-1: for some text must attached with the graphic
                    GraphForPrint[0].GraphicNo =  buf[0];//index of graphic
                    GraphForPrint[0].Prn_Command = prnCMD;
                    GraphForPrint[0].data[0] = buf[2];//Position of graphic


                    if (buf[0]==1 && InfoSelf_Graph.cGrapCols>0)//=1时，为用户自定义图片
                        sGraInfo = &InfoSelf_Graph;
                    else
                        sGraInfo =(struct GRAPH_DEFINE *) gra_info[GraphForPrint[0].GraphicNo-1];

                    if ( buf[1] == '2' && (sGraInfo->cLabels>0))
                    {//copy text from graphic area
                        //读取图像自身携带的字符数据
                        Graph_TextLineMax = sGraInfo->cLabels;
                        if (Graph_TextLineMax>TEXTSOFGRAPH)
                            Graph_TextLineMax=TEXTSOFGRAPH;
                        sGraph = (BYTE*)Graphic_Data[GraphForPrint[0].GraphicNo-1];
                        retval = sGraInfo->cByteALine;
                        if (retval>PRNWIDTH)//GraphForPrint[].data[]
                            retval = PRNWIDTH;
                        for (i=1;i<(Graph_TextLineMax+1);i++)//*(sGraInfo->cByteALine+1))
                        {
                            memcpy(GraphForPrint[i].data,sGraph,retval);
                            sGraph+=sGraInfo->cByteALine;
                            GraphForPrint[i].Length = *sGraph++;
                            GraphForPrint[i].GraphicNo = 0;
                        }
                        Graph_TextCount=Graph_TextLineMax;
                        retval = 1;//exit after copy to pool
                    }
                    else
                    {
                        //计算图像高度
                        retval = ((sGraInfo->cGrapSize-((sGraInfo->cLabels) * (sGraInfo->cByteALine + 1))) / (sGraInfo->cGrapCols*sGraInfo->cWidthAChar/8));
                        //计算图像允许叠加的字符行数
                        Graph_TextLineMax = retval / (LINE_BLANK+TEXT_HIGH);

                        if (retval % (LINE_BLANK+TEXT_HIGH))	 //图像高度超过字符整数行时
                            Graph_TextLineMax++;

                        if (Graph_TextLineMax>TEXTSOFGRAPH)
                            Graph_TextLineMax=TEXTSOFGRAPH;
                        return count;
                    }
                }
                else
                {
                    if (++ApplVar.PoolPushIn>=MAX_PRNPOOL) ApplVar.PoolPushIn = 0;
                    ApplVar.PoolForPrint[ApplVar.PoolPushIn].Length = 0;//flag-0: none text for graphic
                    ApplVar.PoolForPrint[ApplVar.PoolPushIn].GraphicNo = buf[0];
                    ApplVar.PoolForPrint[ApplVar.PoolPushIn].Prn_Command = prnCMD;
                    ApplVar.PoolForPrint[ApplVar.PoolPushIn].data[0] = buf[2];//Position of graphic
                    Start_Motor(prnCMD);
                    return count;
                }
            }
        }
    }

    if (Prn_Status.g_bTextForGraph)
    {//为图像打印接受叠加的文字

        if (count==1 && buf[0]==0)
        {// 终止接受叠加文字 //

/*
            for (;Graph_TextCount<(Graph_TextLineMax);)//(sGraInfo->cByteALine+1))
            {
                Graph_TextCount++;
                GraphForPrint[Graph_TextCount].Length = 1;
                GraphForPrint[Graph_TextCount].GraphicNo = 0;
                GraphForPrint[Graph_TextCount].data[0] = ' ';
            }
*/
            Graph_TextLineMax = Graph_TextCount;
            retval = 1;//exit after copy to pool
        }

        if (Graph_TextCount>=Graph_TextLineMax)
        {//The maxim Count of the text for graphic is 7
            Prn_Status.g_bTextForGraph = false;//clean the over-print text flag

            GraphForPrint[0].Length = Graph_TextCount;//lines of Text for graphic

            for (i=0;i<=Graph_TextCount;i++)
            {
                if (++ApplVar.PoolPushIn>=MAX_PRNPOOL) ApplVar.PoolPushIn = 0;
                while (ApplVar.PoolPushIn==ApplVar.PoolPopOut)
                {//wait for a empty
                    Check_Paper();
                }
                memcpy(&ApplVar.PoolForPrint[ApplVar.PoolPushIn],&GraphForPrint[i],sizeof(GraphForPrint[i]));
            }

            Graph_TextCount = 0;

            if (retval)//exit after copy to pool
            {
                Start_Motor(prnCMD);
                return 1;
            }
        }
        else
            Graph_TextCount++;
    }
    if (Prn_Status.g_bTextForGraph)
    {// The text recived used for over-print on graphic
        sPrnPool = &GraphForPrint[Graph_TextCount];
    }
    else
    {
        if (++ApplVar.PoolPushIn>=MAX_PRNPOOL) ApplVar.PoolPushIn = 0;
        sPrnPool = &ApplVar.PoolForPrint[ApplVar.PoolPushIn];
    }
    // copy data to pool
#if FOR_POS

    (count<=PRNWIDTH)?(retval=count):(retval=PRNWIDTH);
    memcpy(sPrnPool->data,buf,retval);
    sPrnPool->Prn_Command = prnCMD;
    sPrnPool->GraphicNo = 0;

#else//for ECR>>>>>>>>>>>>>>

    sPrnPool->Prn_Command = prnCMD;
    sPrnPool->GraphicNo = 0;
    for (i=retval=0;i<count && buf[i] == ' ' && retval<PRNWIDTH;i++)
    {

        sPrnPool->data[retval] = buf[i];
        retval++;
    }
    if (i<count)
    {
        if (buf[i] == '@')//double high flag
        {
            SETBIT(sPrnPool->Prn_Command,PRN_DBLH_Mode);
            i++;
        }
//        else
//        {
//            CLRBIT(sPrnPool->Prn_Command,PRN_DBLH_Mode);
//        }


        for (;i<count && retval<PRNWIDTH;i++)
        {
            sPrnPool->data[retval] = buf[i];
            retval++;
        }
    }

#endif//FOR_POS

    sPrnPool->Length = retval;

    if (!Prn_Status.g_bTextForGraph)
    {
        Start_Motor(prnCMD);
    }

    return retval;
#endif
}
#else
int RSPrint(BYTE *buf, int count, BYTE prnCMD)//jdb2018-09-03
//int RSPrint(BYTE *buf, int count, WORD prnCMD)//ccr2018-07-30
{
    struct GRAPH_DEFINE *sGraInfo;
    BYTE *sGraph;
    struct TPRN_POOL *sPrnPool;

    BYTE i;
    int  retval; /* value used in "goto out" statements */


	#if 0//jdb2018-09-03
    //ccr2018-01-30>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    if (MyFlags(OPENRECEIPT))
       SETBIT(prnCMD,PRN_RECEIPT_LINE);
    else
       RESETBIT(prnCMD,PRN_RECEIPT_LINE);
    //ccr2018-01-30<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    #endif
    Check_Paper();
    if ((ApplVar.PoolPushIn + Graph_TextCount + 1)%MAX_PRNPOOL == ApplVar.PoolPopOut)
    {
        do
        {
            if (!TestPrintGoingOn())//ccr2018-01-26
				Start_When_Ready(NULL);//ccr2018-01-26
//            rtc_Delayms(10);
//			msDelay(1000);

            i = (ApplVar.PoolPushIn + Graph_TextCount + 1)%MAX_PRNPOOL;
            if (i>ApplVar.PoolPopOut)
                i -= ApplVar.PoolPopOut;
            else
                i = ApplVar.PoolPopOut - i;
            Check_Paper();
        } while (i<3 && ApplVar.PoolPushIn !=ApplVar.PoolPopOut);//等待队列空余3个空间
    }

    if (count > PRNWIDTH+1)//include double high flag '@'
        count = PRNWIDTH+1;

    retval = 0;//set the flag: does not copy to pool

#if ENCUTTER
    if (BIT(prnCMD,PRN_CUTF_Mode  | PRN_CUTRESET_Mode))//Print graphic
    {
        if (++ApplVar.PoolPushIn>=MAX_PRNPOOL) ApplVar.PoolPushIn = 0;
        sPrnPool = &ApplVar.PoolForPrint[ApplVar.PoolPushIn];

        sPrnPool->Prn_Command = prnCMD;
        sPrnPool->GraphicNo = 0;
        sPrnPool->Length = 0;

        if (!Prn_Status.g_bHold)//ccr2018-03-01
           Start_Motor(prnCMD);
        return retval;

    }
#endif
    if (BIT(prnCMD,PRN_GRAPHIC_Mode) && !Prn_Status.g_bTextForGraph)//Print graphic
    {
        {//为打印图片命令
            if (count<=2)
                buf[2] = 0;// Use the default position
            if (count>=2 && buf[0]>0)
            {
                if (buf[1] == '0')//Print graphic  without textor
                {
                    Prn_Status.g_bTextForGraph = false;//no text fot over-print
                }
                else if (buf[1] == '1' || buf[1] == '2')//Print graphic with text
                {
                    Prn_Status.g_bTextForGraph = true;//set the flag for over-print text
                }
                memset(&GraphForPrint,0,sizeof(GraphForPrint));

                if (Prn_Status.g_bTextForGraph && buf[0]<=GRAPHICMAX)//=1时，为用户自定义图片
                {// 当打印的图像需要叠加文字时,把打印数据暂时存储在GraphForPrint中
                 // GraphForPrint[0]保存要打印的图像命令,其它GraphForPrint保存叠加的文字
                    Graph_TextCount = 0;
                    GraphForPrint[0].Length = 1;    //flag-1: for some text must attached with the graphic
                    GraphForPrint[0].GraphicNo =  buf[0];//index of graphic
                    GraphForPrint[0].Prn_Command = prnCMD;
                    GraphForPrint[0].data[0] = buf[2];//Position of graphic


                    if (buf[0]==1 && InfoSelf_Graph.cGrapCols>0)//=1时，为用户自定义图片
                        sGraInfo = &InfoSelf_Graph;
                    else
                        sGraInfo =(struct GRAPH_DEFINE *) gra_info[GraphForPrint[0].GraphicNo-1];

                    if ( buf[1] == '2' && (sGraInfo->cLabels>0))
                    {//copy text from graphic area
                        //读取图像自身携带的字符数据
                        Graph_TextLineMax = sGraInfo->cLabels;
                        if (Graph_TextLineMax>TEXTSOFGRAPH)
                            Graph_TextLineMax=TEXTSOFGRAPH;
                        sGraph = (BYTE*)Graphic_Data[GraphForPrint[0].GraphicNo-1];
                        retval = sGraInfo->cByteALine;
                        if (retval>PRNWIDTH)//GraphForPrint[].data[]
                            retval = PRNWIDTH;
                        for (i=1;i<(Graph_TextLineMax+1);i++)//*(sGraInfo->cByteALine+1))
                        {
                            memcpy(GraphForPrint[i].data,sGraph,retval);
                            sGraph+=sGraInfo->cByteALine;
                            GraphForPrint[i].Length = *sGraph++;
                            GraphForPrint[i].GraphicNo = 0;
                        }
                        Graph_TextCount=Graph_TextLineMax;
                        retval = 1;//exit after copy to pool
                    }
                    else
                    {
                        //计算图像高度
                        retval = ((sGraInfo->cGrapSize-((sGraInfo->cLabels) * (sGraInfo->cByteALine + 1))) / (sGraInfo->cGrapCols*sGraInfo->cWidthAChar/8));
                        //计算图像允许叠加的字符行数
                        Graph_TextLineMax = retval / (LINE_BLANK+TEXT_HIGH);

                        if (retval % (LINE_BLANK+TEXT_HIGH))	 //图像高度超过字符整数行时
                            Graph_TextLineMax++;

                        if (Graph_TextLineMax>TEXTSOFGRAPH)
                            Graph_TextLineMax=TEXTSOFGRAPH;
                        return count;
                    }
                }
                else
                {
                    if (++ApplVar.PoolPushIn>=MAX_PRNPOOL) ApplVar.PoolPushIn = 0;
                    ApplVar.PoolForPrint[ApplVar.PoolPushIn].Length = 0;//flag-0: none text for graphic
                    ApplVar.PoolForPrint[ApplVar.PoolPushIn].GraphicNo = buf[0];
                    ApplVar.PoolForPrint[ApplVar.PoolPushIn].Prn_Command = prnCMD;
                    ApplVar.PoolForPrint[ApplVar.PoolPushIn].data[0] = buf[2];//Position of graphic
#if BARCODESIZE
                    if (buf[0]==BARCODE_GRP_ID && count>3)
                    {
                        i=count-3;
                        if (i>PRTLEN-3)
                            i=PRTLEN-3;
                        memcpy(ApplVar.PoolForPrint[ApplVar.PoolPushIn].data+2,buf+3,i);
                        ApplVar.PoolForPrint[ApplVar.PoolPushIn].data[1]=i;
                    }
#endif
                    if (!Prn_Status.g_bHold)//ccr2018-03-01
                       Start_Motor(prnCMD);
                    return count;
                }
            }
        }
    }

    if (Prn_Status.g_bTextForGraph)
    {//为图像打印接受叠加的文字

        if (count==1 && buf[0]==0)
        {// 终止接受叠加文字 //

/*
            for (;Graph_TextCount<(Graph_TextLineMax);)//(sGraInfo->cByteALine+1))
            {
                Graph_TextCount++;
                GraphForPrint[Graph_TextCount].Length = 1;
                GraphForPrint[Graph_TextCount].GraphicNo = 0;
                GraphForPrint[Graph_TextCount].data[0] = ' ';
            }
*/
            Graph_TextLineMax = Graph_TextCount;
            retval = 1;//exit after copy to pool
        }

        if (Graph_TextCount>=Graph_TextLineMax)
        {//The maxim Count of the text for graphic is 7
            Prn_Status.g_bTextForGraph = false;//clean the over-print text flag

            GraphForPrint[0].Length = Graph_TextCount;//lines of Text for graphic

            for (i=0;i<=Graph_TextCount;i++)
            {
                if (++ApplVar.PoolPushIn>=MAX_PRNPOOL) ApplVar.PoolPushIn = 0;
                while (ApplVar.PoolPushIn==ApplVar.PoolPopOut)
                {//wait for a empty
                    Check_Paper();
                }
                memcpy(&ApplVar.PoolForPrint[ApplVar.PoolPushIn],&GraphForPrint[i],sizeof(GraphForPrint[i]));
            }

            Graph_TextCount = 0;

            if (retval)//exit after copy to pool
            {
                if (!Prn_Status.g_bHold)//ccr2018-03-01
                  Start_Motor(prnCMD);
                return 1;
            }
        }
        else
            Graph_TextCount++;
    }
    if (Prn_Status.g_bTextForGraph)
    {// The text recived used for over-print on graphic
        sPrnPool = &GraphForPrint[Graph_TextCount];
    }
    else
    {
        if (++ApplVar.PoolPushIn>=MAX_PRNPOOL) ApplVar.PoolPushIn = 0;
        sPrnPool = &ApplVar.PoolForPrint[ApplVar.PoolPushIn];
    }
    // copy data to pool
    sPrnPool->Prn_Command = prnCMD;
    sPrnPool->GraphicNo = 0;

    if (buf[0]=='@')
    {
        SETBIT(sPrnPool->Prn_Command,PRN_DBLH_Mode);
        i=1;
    }
    else
        i=0;
    for (retval=0;i<count && retval<PRNWIDTH;i++)
    {

        sPrnPool->data[retval] = buf[i];
        retval++;
    }

    sPrnPool->Length = retval;

    if (!Prn_Status.g_bTextForGraph && !Prn_Status.g_bHold)//ccr2018-03-01
          Start_Motor(prnCMD);

    return retval;
}
#endif

#if 0 //jdb2018-09-02
//ccr2018-03-07 保存收据第一行的打印位置
void SavePopOutOfReceipt()
{
   ApplVar.PopOutForReceipt=ApplVar.PoolPushIn;// PoolPopOut;
}

void ResetPopOutofReceipt()
{
//   if ()
   {
         ApplVar.PoolPopOut=ApplVar.PopOutForReceipt;
         Start_When_Ready(NULL);
   }
}
//ccr2018-03-07<<<<<<<<<<<<<<<<<<<<<<<<
#endif

/*************************************************************************
  掉电、关电、缺纸后，如果有未打印完毕的的数据，那么继续完成打印
  但是，在第一行追加掉电续打印信息Re-Print When Power ON
**************************************************************************/
void Start_When_Ready(const char *str)
{

    BYTE pushIDX,cmd;

    //如果有未完成的打印，开机后继续打印
    if (ApplVar.PoolPopOut!=ApplVar.PoolPushIn)
    {
        cmd = ApplVar.Prn_Command;
        if (str)
        {
            //在行首追加掉电续打印标志行
            if (ApplVar.PoolPopOut==0)
                pushIDX = MAX_PRNPOOL-1;
            else
                pushIDX = ApplVar.PoolPopOut-1;

            if (pushIDX != ApplVar.PoolPushIn)
            {//打印队列未满,追加数据
                ApplVar.PoolForPrint[ApplVar.PoolPopOut].GraphicNo = 0;
                ApplVar.PoolForPrint[ApplVar.PoolPopOut].Length = PRNWIDTH-1;
                ApplVar.PoolForPrint[ApplVar.PoolPopOut].Prn_Command = CMDP_R;
                strncpy(ApplVar.PoolForPrint[ApplVar.PoolPopOut].data,str,PRNWIDTH);;
                ApplVar.PoolPopOut = pushIDX;
                cmd = CMDP_R;
			}
        }
        Prn_TaskNow = -1;
        Start_Motor(cmd);
    }
//     else
//     {
//         ApplVar.PoolPopOut=ApplVar.PoolPushIn=0;
//     }
}
// get the printer status:Prn_Status
PRNSTATUS Get_Printer_status(void)
{

    Printer_Status.g_bHANDUP = 0;

    if (!TESTPAPER())//paper out
    {
        Prn_FeedCount = 0;
        Printer_Status.g_bNOPAPER = true;
        Printer_Status.g_bPaperIn = false;
    }
    else
    {
        if (Printer_Status.g_bNOPAPER)
        {
            Prn_FeedCount = 0;
            Printer_Status.g_bPaperIn = true;
            Printer_Status.g_bWaitPaper = false;
        }
        Printer_Status.g_bNOPAPER = false;
    }

    Printer_Status.g_bDisconnect =!PrinterDetect();//ccr2018-04-02

    if (Prn_FeedCount>FEEDMAX)
        Printer_Status.g_bTOOHOT = true;

#if CASE_HANDUP
    if (!TESTHAND())//paper out
    {
        Printer_Status.g_bHANDUP = true;
        Printer_Status.g_bHANDDN = false;
    }
    else
    {
        if (Printer_Status.g_bHANDUP)
        {
            Printer_Status.g_bWaitPaper = false;
            Printer_Status.g_bHANDDN = true;
        }
        Printer_Status.g_bHANDUP = false;
    }
#endif

    return Printer_Status;
}

#if GROUPMODE==2
//===========================================================================
//根据各组的加热点数,生成可以组合的加热组,最大限度减少加热次数
//Prn_GroupHot[n]中的一位对应一个加热组,
//多个加热组可以出现在同一个Prn_GroupHot[n]中,即一次可以加热多个组.
//当Prn_GroupHot[n]=0时,标示加热结束(即只需加热n次.
//===========================================================================

void  Set_Hot_Group()
{
//    WORD hot;
//    BYTE i,j;

    memset(Prn_GroupHot,0,sizeof(Prn_GroupHot));

#if DCPOWER //采用电源供电，分左右两组进行加热

#if (!CASE_FTP628MCL401) //HJ
#if (0)//POWERCTRL //HJ 设置加热分组
    if (BIT(PowerFlags,PWR_BY_BAT))
    {//ccr20131015电池供电,分六组进行加热
        Prn_HotMAX = 6;
        Prn_GroupHot[0] = (PRN_DS1);
        Prn_GroupHot[1] = (PRN_DS2);
        Prn_GroupHot[2] = (PRN_DS3);
        Prn_GroupHot[3] = (PRN_DS4);
        Prn_GroupHot[4] = (PRN_DS5);
        Prn_GroupHot[5] = (PRN_DS6);
    }
    else
#endif
	{
	    Prn_HotMAX = 2;
	    Prn_GroupHot[0] = (PRN_DS1 | PRN_DS2 | PRN_DS3 );
	    Prn_GroupHot[1] = (PRN_DS4 | PRN_DS5 | PRN_DS6);
	}
#else //hj CASE_FTP628MCL401
#if POWERCTRL //HJ 设置加热分组
    if (BIT(PowerFlags,PWR_BY_BAT))
    {//ccr20131015电池供电,分六组进行加热
        Prn_HotMAX = 6;
        Prn_GroupHot[0] = (PRN_DS1);
        Prn_GroupHot[1] = (PRN_DS2);
        Prn_GroupHot[2] = (PRN_DS3);
        Prn_GroupHot[3] = (PRN_DS4);
        Prn_GroupHot[4] = (PRN_DS5);
        Prn_GroupHot[5] = (PRN_DS6);
    }
    else
#endif
	{
	    Prn_HotMAX = 2;
	    Prn_GroupHot[0] = (PRN_DS1 | PRN_DS3 | PRN_DS5 );
	    Prn_GroupHot[1] = (PRN_DS2 | PRN_DS4 | PRN_DS6);
	}
#endif

#elif 1 //对需要加热的数据按最大加热点数进行分组组合
    Prn_HotMAX = 0;

    for (i=0;i<GROUPS;i++)
    {
        hot =  Prn_GroupDots[i];

        if (hot)
        {//有需要加热的点
            Prn_GroupDots[i] = 0;//表示此组处理完毕
            Prn_GroupHot[Prn_HotMAX] |= (1<<i);//置位第i组要加热标志,
            for (j=i+1;j<GROUPS;j++)
            {
                if (Prn_GroupDots[j])
                {
                    if ((hot+Prn_GroupDots[j])<=DOTS1GROUP)//每组最多加热点数
                    {
                        hot += Prn_GroupDots[j];
                        Prn_GroupDots[j] = 0;   //表示此组处理完毕
                        Prn_GroupHot[Prn_HotMAX] |= (1<<j);//置位第j组要加热标志
                    }
                }
            }
            Prn_HotMAX++;
        }
    }
#else //完全仿照DITRON的分组方式
    Prn_HotMAX = 0;
    if (Prn_LineDots<=DOTS1GROUP)
    {//总点数<=一组点数
        if (Prn_LineDots!=0)
        {//6组全部一次加热,但是将分两次间隔进行
            Prn_HotMAX = 1;
            Prn_GroupHot[0] = (PRN_DS1 | PRN_DS2 | PRN_DS3 | PRN_DS4 | PRN_DS5 | PRN_DS6);
        }
    }
    else//总点数>一组点数
    {
        i = Prn_GroupDots[0]+Prn_GroupDots[1]+Prn_GroupDots[2];//左半总点数
        if (i<=DOTS1GROUP)
        {//左半总点数<=一组点数
            if (i!=0)
            {//左半全部加热
                Prn_HotMAX = 1;
                Prn_GroupHot[0] = (PRN_DS1 | PRN_DS2 | PRN_DS3);
            }
        }
        else//左半总点数>一组点数
        {
            Prn_HotMAX = 2;
            Prn_GroupHot[0] = (PRN_DS1);//先加热第一组
            Prn_GroupHot[1] = (PRN_DS2 | PRN_DS3);//再加热第二,三组
        }
        i = Prn_GroupDots[3]+Prn_GroupDots[4]+Prn_GroupDots[5];//右半总点数
        if (i<=DOTS1GROUP)
        {//右半总点数<=一组点数
            if (i!=0)
            {//右半全部加热
                Prn_GroupHot[Prn_HotMAX++] = ((1<<(GROUPS/2))-1)<<(GROUPS/2);
            }
        }
        else//右半总点数>一组点数
        {
            if(Prn_GroupDots[3]+Prn_GroupDots[4]!=0)
            {
                Prn_GroupHot[Prn_HotMAX++] = (PRN_DS4 | PRN_DS5);//加热第4,5组
            }
            Prn_GroupHot[Prn_HotMAX++] = (PRN_DS6);//加热第6组

        }
    }
#endif
}
//=====================================================================
//根据Prn_GroupHot中设定的加热参数,使能加热管脚
void Set_HotPIN()
{
   Hot_On0(((uint16_t)Prn_GroupHot[Prn_HotIndex])<<8); //*(WORD *)PRT_DST_ADDR=Prn_GroupHot[Prn_HotIndex] & 0x7f;

}
#elif GROUPMODE==1

//---------------------------------------------------------------------------
//每次只向打印机发送加热一组所允许的最多点数(如:64点),加热完毕后,再发送下一组,
//直到所有点阵数据发送并完成加热. 这种加热方式的优点是,最大限度减少加热次数
//一次移送一组所允许的数据点数,Prn_LineDots含有一行点数总数,Prn_LineDots=0时,点阵全部发送完毕
void    Shift_Hot_Data()
{
    WORD hot;
    BYTE i,j,k,d;

    hot = 0;
    for (i=0;i<DOTSBUFF;i++)
    {
       d = BitsInByte[DotsOfCurrent[i]];
       if ((hot+d)<DOTS1GROUP)
       {
           Prn_LineDots-=d;//计算剩余点数
           hot += d;//统计当前组点数
           Send_Byte(DotsOfCurrent[i]);//将点阵发送给打印机
           DotsOfCurrent[i] = 0;

       }
       else
            break;//超过一组点数
    }
    if (i<DOTSBUFF && hot<DOTS1GROUP)
    {//未送满一组数据
        d=0x80;
        k = DotsOfCurrent[i];
        for (j=0;j<8;j++)
        {
            if (k & d)
            {//测试点为1
                hot++;//增加1加热点
                Prn_LineDots--;//减少总点数
                if (hot==DOTS1GROUP)
                {//一组满
                    d--;
                    DotsOfCurrent[i] &= d;//清除已送出的点
                    d = ~d;
                    k &= d;//保留要送出的点
                    break;
                }
            }
            d >>= 1;//测试下一点是否为1
        }
        Send_Byte(k);//将点阵发送给打印机
        i++;
    }
    for (;i<DOTSBUFF;i++)
        Send_Byte(0);//剩余部分送0
}

#endif

#if (DEBUGONPC==0)

#if (defined(CASE_FORHANZI) )
/*******************************************************************
  从字库芯片中读取3字节的汉字点阵
  *****************************************************************/
void ReadHZKU(ULONG addr,BYTE *hzBuf)
{
    addr+=NOR_FLASH_ADDR;
    hzBuf[0]=FlashReadByte(addr);addr++;
    hzBuf[1]=FlashReadByte(addr);addr++;
    hzBuf[2]=FlashReadByte(addr);
}
#endif

#if (WORDorBYTE)
//以WORD方式将数据发送给打印机 Send FONTS-Data to printer(384dots)
void    Set_Hot_DataW(WORD dot,BYTE pos)
{
     Send_Word(dot);

    Prn_LineDots += BitsInByte[dot & 0xff];//统计需要加热的总点数
    Prn_LineDots += BitsInByte[dot>>8];//统计需要加热的总点数

#if GROUPMODE==2
    Prn_GroupDots[pos/BYTES1GROUP]+=BitsInByte[dot>>8];//统计各组加热点数
    Prn_GroupDots[(pos+1)/BYTES1GROUP]+=BitsInByte[dot & 0xff];//统计各组加热点数
#endif

}

#else
//以BYTE方式将数据发送给打印机 Send FONTS-Data to printer(384dots)
void    Set_Hot_Data(BYTE dot,BYTE pos)
{
    Send_Byte(dot);//以字节方式发送数据

    Prn_LineDots += BitsInByte[dot];//统计需要加热的总点数

#if GROUPMODE==2
    Prn_GroupDots[pos/BYTES1GROUP]+=BitsInByte[dot];//统计各组加热点数
#endif

}
#endif

// Drive printer motor by one step:GPIO1
void  Drive_Motor(WORD phase)
{
    Prn_FeedCount++;
    Drive_On(phase);
}

#if ENCUTTER
// Drive printer motor by one step:GPIO1
void  Drive_CutMotor(WORD phase)
{
    Prn_FeedCount++;
    CutDrive_On(phase);
}
#endif
// Set the speed of motor: RIT
void Set_Motor_Speed(ULONG speed)
{
#if defined(CASE_FORHANZI)
    (speed<Prn_HZTime)?(TIM2->ARR = Prn_HZTime):(TIM2->ARR = speed);
#else
    TIM2->ARR = speed;
#endif
    /* Generate an update event to reload the Prescaler */
    //TIM2->EGR = TIM_PSCReloadMode_Immediate;
    StartTimer(TIM2);
}
#if (!CASE_FTP628MCL401) //HJ
// Set the hot time
void Start_Hot()
{
    if (Prn_LineDots)//Prn_LineDots=0时,无须加热
    {
        Latch_Data();
        Prn_CalcHotTime();
        if (Prn_HotIndex>=Prn_HotMAX)//上一行数据加热完毕
        {
            Prn_HotIndex=0;
#if GROUPMODE==0
#if GROUPS>1
        Hot_On0(HOT1_PIN);//分多组加热
#else
        Hot_On0(HOTMASK0);//只进行一次加热
#endif
#elif GROUPMODE==1
        Hot_On0(HOTMASK0);//只进行一次加热
#else//GROUPMODE==2
        Set_HotPIN();//多组合并加热
#endif

        Set_HotTime(Prn_HotTime);
    }
        else//还在加热中,将其设置为0xff后,在TIM3_IRQHandler中会将其置为0,因此会启动第一组加热
            Prn_HotIndex=0xff;
    }
    else
        PrnHotOFF();//!!!!@@@@@@@@@!!!!!!
}
#else
// Set the hot time
void Start_Hot()
{
    if (Prn_LineDots)//Prn_LineDots=0时,无须加热
    {
        Latch_Data();
		Prn_Status.g_bHotLatch = 1;

        Prn_HotIndex=0;
        Prn_CalcHotTime();
        Set_HotPIN();//多组合并加热

        Set_HotTime(Prn_HotTime);
    }
    else
        PrnHotOFF();//!!!!@@@@@@@@@!!!!!!
}
#endif


#endif

//Stop print motor
void Stop_Motor()
{
    Set_Motor_Speed( TIMESTOP );
    Prn_AccStep = 0;
    Prn_TaskNow = TASK00_Stop;
}

//Pause print motor when too hot or to far
void Suspend_Motor()
{
    PrnHotOFF();
    Drive_Off(); //stop print motor
    Set_Motor_Speed( TIMESTOP );
    PrnPowerOFF();
    Prn_AccStep = 0;
    Prn_Jphase = 0;
    Prn_Status.g_bSuspend = true;//suspend when too hot or too far of feed
    Prn_Delay = 1;//打印机暂停计时开始
}

//start motor
void Start_Motor(WORD prnCMD)
{
    if (Prn_TaskNow<=0)
//        && !Prn_Status.g_bNOPAPER
//        && !Prn_Status.g_bHANDUP
//        && !Prn_Status.g_bTOOHOT
//        && !Prn_Status.g_bTOOHOT)//motor stoped
    {
        Prn_FeedCount = Prn_Delay = 0;
        Prn_Status.g_bPrintOver = false;//some data must be print out
        ApplVar.Prn_Command = prnCMD;
#if ENCUTTER
        if (BIT(prnCMD,PRN_CUTRESET_Mode | PRN_CUTF_Mode ))
        {
            PrnPowerON();
            Drive_CutMotor(CPHSTART);//out
        }
        else
#endif
			if (!Prn_Status.g_bWaitPaper)
            {
                PrnPowerON();
             	Drive_Motor(PHSTART);//out
            }
        Set_Motor_Speed( TIMESTART );
        Prn_Jphase = 0;
        Prn_AccStep = 0;
        Prn_TaskNow = TASK01_Start;
#if DEBUGONPC
        pcStart_Motor();
#endif
    }
    else
        StartTimer(TIM2);//ccr2014-08-19 使能计时器
}
//Restart motor after motor paused
void Restart_Motor()
{
    Prn_Status.g_bTOOHOT = false;
    Prn_Status.g_bSuspend = false;
    Prn_FeedCount = Prn_Delay = 0;
    {
        PrnPowerON();
        Drive_Motor(PHSTART);//out
        Set_Motor_Speed( TIMESTART );
        Prn_Jphase = 0;
        Prn_AccStep = 0;

#if DEBUGONPC
        pcStart_Motor();
#else
//?????        LPC_RIT->RICTRL |= RIT_CTRL_TEN;//Enable counter
#endif

    }
}

//POS command ---------------------------------------------------------------------------------------------------
#if DEBUGONPC==0

void Excute_data(BYTE ch);

/*********************************************************************************
*
* File: ESCPOS.C
*
*********************************************************************************/
void Excute_ESC(void);		   	// ESC sequence
void Excute_GS(void);		   		// GS sequence
void Excute_LF(void);		   		//LF (Line-feed)
void Excute_FF(void);		   		//FF (form-feed)
void Excute_CR(void);		   		//CR
void Excute_DLE(void);		   	//**DLE (real time status trasmission)

#define MAINCOMMAX  6

const type_Command  MAINCOMMAND[MAINCOMMAX]=
{
		 {0x1B,		*Excute_ESC},		   	// ESC sequence
		 {0x1D,		*Excute_GS},		   		// GS sequence
		 {0x0A,		*Excute_LF},		   		//LF (Line-feed)
		 {0x0C,		*Excute_FF},		   		//FF (form-feed)
		 {0x0D,		*Excute_CR},		   		//CR
		 {0x10,		*Excute_DLE},		   	//**DLE (real time status trasmission)

};

#define ESCCOMMAX  5	//58 20051013 test

void sel_print_mode(void);                                 // Select print mode
void turn_underline_mode(void);                            //Turn underline mode on/off
void total_cut(void);                                      //*Total cut
void partial_cut(void);                                    //*Partial cut
void tx_printer_status(void);                              //** Trasmit printer status


const type_Command  ESCCOMMAND[ESCCOMMAX]=
{
		{'!',		*sel_print_mode},                                 // Select print mode
		{'-',		*turn_underline_mode},                            //Turn underline mode on/off
		{'i',		*total_cut},                                      //*Total cut
		{'m',		*partial_cut},                                    //*Partial cut
		{'v',		*tx_printer_status},                              //** Trasmit printer status

};


#define GSCOMMAX  5
void tx_status(void);              // Trasmit status
void sel_raster_bit_image(void);		  // Block inmage mode
void Download_Image(void);              // download image
void Print_Download_Image(void);              // print download image
void Cut_Paper(void);               //cut paper
void TestFlashBWrite(void);               //Send CCLIB to host

const type_Command  GSCOMMAND[GSCOMMAX]=
{
		{ 'r',		*tx_status},              // Trasmit status
		{ 'v',		*sel_raster_bit_image},		  // Block inmage mode
		{ '*',		*Download_Image},              // download image
		{ '/',		*Print_Download_Image},              // print download image
        { 'V',      *Cut_Paper},                 //cut paper
//        { 0xff,     *TestFlashBWrite}                 //cut paper
};



//////////////////////////////////////////////////////////////////////////////
// ESC+ command from Host
void Print_ESC_Command()
{
    BYTE carattere;
    int	sLp;

    while (true)
    {
        if (UARTGet(PORTHOST,&carattere,0)==true)
        {
            for (sLp=0;sLp<MAINCOMMAX;sLp++)
                if (carattere==MAINCOMMAND[sLp].Command)
                {
                    (*(MAINCOMMAND[sLp].Procedure))();

                    break;
                }
            if (sLp==MAINCOMMAX)
                Excute_data(carattere);	//gestione dati da stampare
        }
    }// end for(;;) loop
}


/******************************************************************
 Function:接受要打印的数据
 Input	: riceve in ch il carattere da da caricare nella riga di stampa
 Describe::   Gestisce i dati da stampare
******************************************************************/
void Excute_data(BYTE ch)
{

//	if(ch <' ')	return;// 启用此语句,将不能接受处理从串口发来的打印图片的命令


    if (DataFromHost.Length>PRNWIDTH)
        DataFromHost.Length = PRNWIDTH;
    if (DataFromHost.Length==PRNWIDTH)
    {
        Excute_CR();
    }

    if (DataFromHost.Length<PRNWIDTH)
    {
        DataFromHost.data[DataFromHost.Length]=ch;
        DataFromHost.Length++;
    }

}



/******************************************************************
 Function: void Excute_ESC(void)
 Describe::   Elabora tutte le sequenze di ESCAPE
******************************************************************/
void Excute_ESC()
{
	BYTE ch;
	int	sLp;

	// attende un carattere con un timeout di 60 secondi
	if(UARTGet(PORTHOST,&ch,DELAYHOST)==false)
		return;

	for (sLp=0;sLp<ESCCOMMAX;sLp++)
		if (ch==ESCCOMMAND[sLp].Command)
		{
			(*(ESCCOMMAND[sLp].Procedure))();
			return;
		}
    Excute_data(ch);
}


/******************************************************************
 Function: void Excute_GS(void)
 Describe::   Elabora tutte le sequenze di GS
******************************************************************/
void Excute_GS(void)
{

	BYTE 	ch;
	int		sLp;
	// attende un carattere con un timeout di 60 secondi

	if(UARTGet(PORTHOST,&ch,DELAYHOST)==false)
		return;

	for (sLp=0;sLp<GSCOMMAX;sLp++)
		if (ch==GSCOMMAND[sLp].Command)
		{
			(*(GSCOMMAND[sLp].Procedure))();
			return;
		}
    Excute_data(ch);

}



/******************************************************************
 Function: void Excute_LF(void)
 Describe: print out and then feed 1 line
******************************************************************/
void Excute_LF(void)
{
    BYTE cpMode = DataFromHost.Prn_Command;
    if (DataFromHost.Length)
        Excute_CR();
    else
    {
        DataFromHost.Prn_Command = CMDP_R;
        Excute_CR();
        DataFromHost.Prn_Command = cpMode;
    }
}




/******************************************************************
 Function: void Excute_FF(void)
 Describe: Feed 15 lines
******************************************************************/
void Excute_FF(void)
{
    BYTE cpMode = DataFromHost.Prn_Command;
    int i;

    if (DataFromHost.Length)
        Excute_CR();

    DataFromHost.Prn_Command = CMDP_R;
    for (i=0;i<15;i++)
    {
        Excute_CR();
    }
    DataFromHost.Prn_Command = cpMode;
}


/******************************************************************
 Function: void Excute_CR(void)
 Describe:
******************************************************************/
void Excute_CR(void)
{
    SETBIT(DataFromHost.Prn_Command,CMDP_R);
    CLRBIT(DataFromHost.Prn_Command,PRN_GRAPHIC_Mode | PRN_CUTF_Mode );
    DataFromHost.GraphicNo = 0;
    RSPrint(DataFromHost.data,DataFromHost.Length,DataFromHost.Prn_Command);
    DataFromHost.Length = 0;

}



/******************************************************************
 Function: void Excute_DLE(void)
 Describe::   Gestisce il DLE (real time status trasmission)

******************************************************************/
void Excute_DLE(void)
{
	BYTE ch;
	WORD PrnStatus=0;

	// attende un carattere (EOT) con un timeout di 60 secondi
	if(UARTGet(PORTHOST,&ch,DELAYHOST)==false || ch != EOT)
		return;
	if(UARTGet(PORTHOST,&ch,DELAYHOST)==false || ch <1 || ch>6)
		return;
    Get_Printer_status();

	switch (ch){
	case 2:
		if (Printer_Status.g_bHANDUP)
			PrnStatus |= 0x04;
	case 1:
		PrnStatus |= 0x12;
		if (Printer_Status.g_bNOPAPER || Printer_Status.g_bHANDUP )
			PrnStatus |= 0x40;
		break;
	case 3:
		if (Printer_Status.g_bHANDUP)
			PrnStatus |= 0x20;
		break;
	case 4:
		if (Printer_Status.g_bNOPAPER)
			PrnStatus |= 0x6c;
		break;
	}
	UARTSend(PORTHOST,PrnStatus);
}


//*****************************************************************************************************


/* [ ESC + '!' + n ]
* void sel_print_mode(void)
*
*  Select print mode
*  Attende altro carattere.
*/
void sel_print_mode(void)
{
}



/* [ ESC + '-' + n ]
* void turn_underline_mode(void)
*
*  Turn underline mode on/off
*  Attende altro carattere.
*/
void turn_underline_mode(void)
{
    BYTE ch;
				// attende un carattere con un timeout di 60 secondi
	if(UARTGet(PORTHOST,&ch,DELAYHOST)==false)
		return;

	return;
}


/* [ ESC + '@']
* void Reset_printer(void)
*
*  inizialize printer
*  Attende altro carattere.
*/
void Reset_printer(void)
{
#if ENCUTTER
    RSPrint(0,0,PRN_CUTRESET_Mode);
#endif
}





/* [ ESC + 'i']
* void total_cut(void)
*
*  Total cut
*  .
*/
void total_cut(void)
{

    BYTE cpMode = DataFromHost.Prn_Command;
    if (DataFromHost.Length)
        Excute_CR();

    DataFromHost.Prn_Command = PRN_CUTF_Mode;
    RSPrint(DataFromHost.data,0,DataFromHost.Prn_Command);
    DataFromHost.Prn_Command = cpMode;
	return;
}



/* [ ESC + 'm']
* void partial_cut(void)
*
*  Partial cut
*  .
*/
void partial_cut(void)
{
// qui deve gestire la sequenza di escape come total_cut()
    BYTE cpMode = DataFromHost.Prn_Command;
    if (DataFromHost.Length)
        Excute_CR();

    DataFromHost.Prn_Command = PRN_CUTF_Mode;
    RSPrint(DataFromHost.data,0,DataFromHost.Prn_Command);
    DataFromHost.Prn_Command = cpMode;
    return;

}

/* [ GS + 'V' + m + n]
* void Cut_Paper(void)
*
*  Partial cut
*  .
*/
void Cut_Paper()
{

    BYTE ch;
    BYTE cpMode = DataFromHost.Prn_Command;

    if(UARTGet(PORTHOST,&ch,DELAYHOST)==false)
        return;

    if (DataFromHost.Length)
        Excute_CR();

    if (ch==0 || ch=='0' || ch==65)
    {//Executes a full cut (cuts the paper completely).
        DataFromHost.Prn_Command = PRN_CUTF_Mode;
    }
    else if (ch==1 || ch=='1' || ch==66)
    {// Executes a partial cut (one point left uncut)
        DataFromHost.Prn_Command = PRN_CUTF_Mode;
    }
    else if (ch==2 || ch=='2')
    {//reset cutter
        DataFromHost.Prn_Command = PRN_CUTRESET_Mode;
    }
    else
        return;

    RSPrint(DataFromHost.data,0,DataFromHost.Prn_Command);
    DataFromHost.Prn_Command = cpMode;
    return;

}



/* [ ESC + 'v']
* void tx_printer_status(void)
*
*  Trasmit printer status
*  .
*/
void tx_printer_status(void)
{

	WORD PrnStatus=0;


    Get_Printer_status();

    if (Printer_Status.g_bNOPAPER)
		PrnStatus |= 0x05;
	if (Printer_Status.g_bHANDUP)
		PrnStatus |= 0x42;

    UARTSend(PORTHOST,PrnStatus);

}


/* [ GS + 'V' + m + n ]
* void sel_cut_mode_cut_paper(void)
*
*  Set printing area width
*  Attende altri 2 caratteri.
*/
void sel_cut_mode_cut_paper(void)
{
	BYTE m,n;

	if(UARTGet(PORTHOST,&m,DELAYHOST)==false)
		return;
	if(m==66 || m==65){
		if(UARTGet(PORTHOST,&n,DELAYHOST)==false)
			return;

	}

    total_cut();//ccr 060331
}




/* [ GS + 'r' + n ]
* void tx_status(void)
*
*  Trasmit status
*  Attende altro carattere.
*/
void tx_status(void)
{
	BYTE ch;
	WORD PrnStatus;

				// attende un carattere con un timeout di 60 secondi
	if(UARTGet(PORTHOST,&ch,DELAYHOST)==false)
		return;

	if( (ch != 1) && (ch !=2) && (ch!=49) && (ch !=50))
		return;

	ch &= 0x0f;

    Get_Printer_status();

    if (Printer_Status.g_bNOPAPER)/* 无纸 */
        PrnStatus |= 0x03;
    if (Printer_Status.g_bHANDUP)/* 压杆抬起 */
        PrnStatus |= 0x0c;

    UARTSend(PORTHOST,PrnStatus);
}


//esegue l'operazione inversa della funzione AToH()
BYTE *HToA(BYTE ch, BYTE *ptr)
{
	BYTE	h;
	h=(ch>>4);
	if(h<=9)
		*ptr=h+'0';
	else
		*ptr=h+'A'-0xA;

	h=(ch & 0x0F);
	if(h<=9)
		*(ptr+1)=h+'0';
	else
		*(ptr+1)=h+'A'-0xA;

	*(ptr+2)=NULL;

	return(ptr);
}



// converte buffer binario 'src' in buffer ASCII-HEX 'dst' e termina la stringa con NULL
void BuffHToA(BYTE *src, BYTE *dst, BYTE len,BYTE space)
{
	BYTE i,j,ch;
	for(i=0,j=0;i<len;i++){
		ch=src[i];
		(void)HToA(ch, (BYTE *)(&dst[j]));
		if(space==TRUE){
			dst[j+2]=' ';
			j++;
		}
		//i++;
		j +=2;
	}
	dst[j]=0;

}

// elabora buffer binario in ASCI convertendo i caratteri non visualizzabili in '.'
BYTE *ConvAsci(BYTE *src,BYTE *dst,BYTE len)
{
	BYTE i;
	for(i=0;i<len;i++){
		if((src[i] <' ') || (src[i] > '~'))
			dst[i]='.';
		else
			dst[i]=src[i];

	}
	dst[i]=NULL;
	return(dst);
}



// converte intero in stringa ASCII nel buffer puntato da buff, termina stringa con NULL
// omette i zeri a sinistra
void itoa(WORD n,BYTE *buff)
{
	WORD i;
	BYTE j,flag;

	j=0;
	for(i=10000;i;i/=10){
		flag = n/i;
		buff[j]=flag+0x30;
		n=n- flag * i;	//n=resto della divisione fatta in precedenza
		if((j>0)||(buff[0]!='0'))
			j++;
	}
	if(j==0){
//		buff[j]='0';
		j++;
	}
	buff[j]=0;
}



/* [ GS + 'v' + c +m+xl+xh+yl+yh d1+...+dk] 打印一个矩阵块图形  //
* void sel_raster_bit_image(void)
* _______xl+xh*256(每行字节数)________
 /                                    \
*--------------------------------------\
*-------------------------------------- |
*-------------------------------------- |
*-------------------------------------- |
*-------------------------------------- |yl+yh*256 (行数)
*-------------------------------------- |
*-------------------------------------- |
*-------------------------------------- |
*-------------------------------------- |
*--------------------------------------/
*k=(xl+xh*256)*(yl+yh*256)
*  Graphic command in emulaztion TM-T88II
*  wait for ather 6 char
*
* To adapt T88II resolution (180 dpi)  to our printer resolution (203 dpi)
* add 1 dot every 9 dot
*
* To adapt T88II graphic line to our printer 60 mm 2 dot every 7 dot are in logic OR
*
*/

void sel_raster_bit_image(void)
{

	BYTE 	c,m,xl,xh,yl,yh;
	WORD	LineByte,TotBytes,i;

	if(UARTGet(PORTHOST,&c,DELAYHOST)==false)
		return;


	if(UARTGet(PORTHOST,&m,DELAYHOST)==false)
		return;
	if(UARTGet(PORTHOST,&xl,DELAYHOST)==false)
		return;
	if(UARTGet(PORTHOST,&xh,DELAYHOST)==false)
		return;
	if(UARTGet(PORTHOST,&yl,DELAYHOST)==false)
		return;
	if(UARTGet(PORTHOST,&yh,DELAYHOST)==false)
		return;


	LineByte=xl+xh*256;// 一行数据的字节数目 //
	TotBytes = LineByte * (yl+yh*256);// 总共行数 //
    if (TotBytes<SELFGRASIZE && LineByte<BYTES_OF_PAPER_LINE)
    {

        InfoSelf_Graph.cGrapCols = LineByte;
        InfoSelf_Graph.cLabels = 0;
        InfoSelf_Graph.cPrintAt = 0;
        InfoSelf_Graph.cWidthAChar = 8;
        InfoSelf_Graph.cHighAChar = 24;
        InfoSelf_Graph.cByteALine = 0;
        InfoSelf_Graph.cGrapSize = TotBytes;

        for (i=0;i<TotBytes;i++)
        {
            if(UARTGet(PORTHOST,&Self_Graph[i],DELAYHOST)==false)
                return;
        }

        xl = DataFromHost.Prn_Command;
        if (DataFromHost.Length)
            Excute_CR();

        DataFromHost.Prn_Command = PRN_GRAPHIC_Mode;
        DataFromHost.data[0] = 1;
        DataFromHost.data[1] = '0';


        RSPrint(DataFromHost.data,2,DataFromHost.Prn_Command);
        DataFromHost.Prn_Command = xl;
        return;


    }

}

/* [GS+*+x+y+d1+.....+dx*y*8]
*Defines a downloaded bit image using (x*8) dots in the
*horizontal direction and (y*8) dots in the vertical direction.
* d indicates the bit image data.
*/
void Download_Image()              // download image
{


    BYTE 	xl,yl;
    WORD	LineByte,TotBytes,i;

    if(UARTGet(PORTHOST,&xl,DELAYHOST)==false)
        return;
    if(UARTGet(PORTHOST,&yl,DELAYHOST)==false)
        return;


    LineByte=xl;// 一行数据的字节数目 //
    TotBytes = LineByte * (yl*8);// 总共行数 //
    if (TotBytes<SELFGRASIZE && LineByte<BYTES_OF_PAPER_LINE)
    {
        InfoSelf_Graph.cGrapCols = LineByte;
        InfoSelf_Graph.cLabels = 0;
        InfoSelf_Graph.cPrintAt = 0;
        InfoSelf_Graph.cWidthAChar = 8;
        InfoSelf_Graph.cHighAChar = 24;
        InfoSelf_Graph.cByteALine = 0;
        InfoSelf_Graph.cGrapSize = TotBytes;

        for (i=0;i<TotBytes;i++)
        {//recive graphic data from host
            if(UARTGet(PORTHOST,&Self_Graph[i],DELAYHOST)==false)
                return;
        }
    }

}


/* [GS+/+m+t]
* Print bit image
* if m<=GRAPHICMAX, Print bitmap from ROM
* if m=1,Print downloaded bit image by Download_Image
* if m<>0 and m<=GRAPHICMAX, t is the text flag
*      t = '0',don't attach text
*      t = '1',Attach 8 lines of text from host
*      t = '2',Attach text from graphic
*/
void Print_Download_Image()              // print download image
{
	BYTE m,t,j;

	if(UARTGet(PORTHOST,&m,DELAYHOST)==false)
		return;

    if (m==1)
    {//print graphic dowload from host
        if(UARTGet(PORTHOST,&t,DELAYHOST)==false)
            return;//text attribute:'0'-no text;'1'-attached text from host;'2'-attached text from grphia

        if (InfoSelf_Graph.cGrapSize)
        {
            m = DataFromHost.Prn_Command;//save Prn_mode
            if (DataFromHost.Length)
                Excute_CR();// print out the text recived

            DataFromHost.Prn_Command = CMDP_PRGRA;
            DataFromHost.data[0] = 1;
            DataFromHost.data[1] = t;
            Prn_Status.g_bTextForGraph = false;

            RSPrint(DataFromHost.data,2,DataFromHost.Prn_Command);
            DataFromHost.Length = 0;
            DataFromHost.Prn_Command = m;
        }
    }
    else if (m<=GRAPHICMAX)
    {
        j = DataFromHost.Prn_Command;
        if (DataFromHost.Length)
            Excute_CR();

        if (m!=0)
        {// print a graphic from ROM
            if(UARTGet(PORTHOST,&t,DELAYHOST)==false)
                return;

            DataFromHost.data[1] = t;
            DataFromHost.Length = 2;
            Prn_Status.g_bTextForGraph = false;
        }
        else//stop the text from host
            DataFromHost.Length = 1;
        DataFromHost.Prn_Command = CMDP_PRGRA;
        DataFromHost.data[0] = m;

        RSPrint(DataFromHost.data,DataFromHost.Length,DataFromHost.Prn_Command);
		DataFromHost.Length = 0;
        DataFromHost.Prn_Command = j;
    }
    return;
}
#endif

// 读取打印机的温度 //
WORD PrnTemperature()
{
    //return ADC_GetConversionValue(ADC1);
    return ADC2->DR & 0x0fff;
}
//检测打印机是否已连接 PrinterDetect //ouhs20180330
//返回：1--已连接,  0--已断开
BYTE  PrinterDetect(void)
{
	WORD i, tempval;

	tempval = PrnTemperature();

	if(tempval>0xE00)	 //经测试打印机没有连接时，tempval>0xF00, 连接后空闲状态 tempval<0xD00,温度越高值越低
	{
		for (i=0;i<5000;i++){};
		tempval = PrnTemperature();
		if(tempval>0xE00)
		{
			return (0); //打印机没有连接
		}
	}

	return (1); //打印机已连接
}
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// 检查是否有打印任务正在进行中 //
BYTE  TestPrintGoingOn()
{
    BYTE sIn,sOut;

    sIn=ApplVar.PoolPushIn; //ccr2018-01-24避免被优化掉
    sOut=ApplVar.PoolPopOut;//ccr2018-01-24避免被优化掉

    return !(sIn == sOut && //无打印任务   //ccr2018-01-24
             (Prn_Status.g_bSuspend ||   //打印机处在挂起状态
              Prn_Status.g_bWaitPaper || //打印机处等待打印纸状态
             (Prn_TaskNow<=TASK00_Stop || Prn_TaskNow>=TASK05_FeedHalf))  //任务不涉及读字库
            );//ccr2016-05-17

}


/**
 * 将老收款机的GRB图片数据转换为新收款机的图片数据,老图片数据中必须包含文字(cTextFlag=0x80)
 *
 * @author EutronSoftware (2014-03-25)
 */
void ChangeGRBToMyself(void)
{
    char *GRBBuff=(char *)BKPSRAM_BASE;
    int newLength,textFrom,sGRBSize;
    int i,j,k;
    int sPOS;//新数据的存放位置
    BYTE TextC;//文本记录数
    BYTE TextL;//文本长度


//    if (GRB_Define.cGraSize>0 && GRB_Define.cTextFlag==0x80 //有用户自定义图片数据
//        && InfoSelf_Graph.cGrapCols==0) //图片数据未转换
    {
            //GRB_Define.cGraSize为GRB数据长度
            //将GRB数据进行转换,转换为ER260的图片格式
            if (GRB_Define.cTextFlag==0)//无文本数据
            {
              newLength=5;//指向文件名称存放位置
              GRB_Define.cTextOffset=0;
              GRB_Define.cHighAChar=24;
			  sGRBSize = GRB_Define.cGraSize;
            }
            else
			{
			  sGRBSize = GRB_Define.cTextOffset;
              newLength=10;//指向文件名称存放位置
			}

            for (i=0;GRBBuff[newLength]!=0;newLength++)//读取文件名称数据
            {
//              if (i<sizeof(GRB_Define.cName))
//                  GRB_Define.cName[i++]=GRBBuff[newLength];
//              if (GRBBuff[newLength]==0)
//                break;
            }
            newLength++;//为图像点阵开始位置

            InfoSelf_Graph.cGrapCols = GRB_Define.cWidth * 8 / 12;//新图片的所占字符宽度,字符的点阵数为12
            InfoSelf_Graph.cPrintAt = GRB_Define.cPrintAt;
            InfoSelf_Graph.cWidthAChar = 12;
            InfoSelf_Graph.cHighAChar = 24;
            InfoSelf_Graph.cByteALine = 24; //长度固定为24 !!!!!!!

            sPOS=0;TextC=0;
            //先解析文本数据
            if (GRB_Define.cTextOffset)
            {//有文本数据
                InfoSelf_Graph.cLabels=0;
                textFrom=GRB_Define.cTextOffset;
                j=0;//为每行字符数计数器
//                for (;textFrom<GRB_Define.cGraSize && InfoSelf_Graph.cLabels<GRB_Define.cLabels;)
                for (;textFrom<GRB_Define.cGraSize;)//&& TextC<GRB_Define.cLabels;)
                {
                    i = sPOS;
                    if (GRBBuff[textFrom]==0)//为空行
                    {
                      memset(Self_Graph+sPOS,' ',InfoSelf_Graph.cByteALine);
                      sPOS+=InfoSelf_Graph.cByteALine;
                      Self_Graph[sPOS++]=0;//有效打印长度为0
                      textFrom++;
                      InfoSelf_Graph.cLabels++;//行数加1;
                    }
                    else if (GRBBuff[textFrom]==1)
                    {
                        textFrom+=2;//踢出覆盖标志和记录号
                    }
                    else
                    {
                        for (j = 0;textFrom<GRB_Define.cGraSize && GRBBuff[textFrom]!=0;textFrom++)
                        {
                            if (j<InfoSelf_Graph.cByteALine)
                            {
                                Self_Graph[sPOS++]=GRBBuff[textFrom];
                                j++;
                            }
                        }
                        textFrom++;//skip \0
                        TextL = j;
                        for (;j<InfoSelf_Graph.cByteALine;j++)
                            Self_Graph[sPOS++]=' ';

                        Self_Graph[sPOS++]=TextL;
                        InfoSelf_Graph.cLabels++;//行数加1;
                        TextC++;
                    }
                }
                InfoSelf_Graph.cGrapSize = (InfoSelf_Graph.cLabels) * (InfoSelf_Graph.cByteALine + 1);
            }
            else
            {
                InfoSelf_Graph.cLabels = 0;
                InfoSelf_Graph.cGrapSize = 0;
            }
            //字符数据解析完毕.开始解析图像点阵
            textFrom=sPOS; //点阵在ER260Grp_Def中的开始位置

            //sGRBSize = (int)GRB_Define.cWidth*GRB_Define.cHeight*24;//固定24点阵高 GRB_Define.cHighAChar;//
            for (i=0;i<sGRBSize && sPOS<sizeof(Self_Graph);)
            {
                j = i + newLength;
                if (GRBBuff[j]==0)
                {
                  j++;
                  for (k=0;k<GRBBuff[j] && sPOS<sizeof(Self_Graph);k++)
                     Self_Graph[sPOS++]=0;
                     i+=2;
                }
                else
                {
                    Self_Graph[sPOS++]=GRBBuff[j];
                    i++;
                }
            }
            InfoSelf_Graph.cGrapSize += (sPOS-textFrom);
            SelfGrpReady=1;
    }
}

/*******************************************************
 * 判断要打印的行是否是销售收据数据
 *
 * @author EutronSoftware (2018-01-31)
 *
 * @return BYTE
 ******************************************************/
BYTE TaskForReceipt()
{
    BYTE sCurrentLine;
    Prn_LineIndex = 0;///////////////


    if (ApplVar.PoolPushIn == ApplVar.PoolPopOut)//无打印任务
        return false;
    else
    {
        sCurrentLine = (ApplVar.PoolPopOut + 1);
        if (sCurrentLine>=MAX_PRNPOOL)
            sCurrentLine = 0;
        return (BIT(ApplVar.PoolForPrint[sCurrentLine].Prn_Command,PRN_RECEIPT_LINE)!=0);
    }
}


/***********************************************************************************
// push data into the POOL
// INPUT:
//      *buf:   Text data for print
//      count:  Bytes in *buf must be print out
//      prnCMD: Command for TEXT ,must be one of:
                -PRN_DBLW_Mode        //double with print mode for text
                -PRN_DBLH_Mode        //double high print mode for text
                -PRN_RECEIPT_Mode     //print on receipt printer
                -PRN_STUB_Mode        //print on stub printer
                -PRN_GRAPHIC_Mode     //print a graphic
                -PRN_CUTF_Mode        //Cut paper full
                -PRN_RECEIPT_LINE     //ccr2018-01-31 打印收据行
                -PRN_CUTRESET_Mode    //Cutter reset


// return:<0;POOL is full & the data don't push in
//        >=0;data has pushed into the pool and the value is the
//        length of the data

//REMARK:
//    if prnCMD is the GRAPHIC command:
//        buf[0] must be the number of the graphic.(1,2,3....)
//               buf[0]=0,stop attach the text;
//        buf[1] must be '0'(don't attache text)
//              or '1' (8 lines of Text must attache for the graphic)
//              or '2' (copy text from graphic)
//       buf[2] Postion by character of the graphic on paper(0,1,2,..,32), It's not the must,
//              =0,Print at the default position defind by the graphic
//    if prnCMD is the CUT PAPER command:
//        buf[0] must be the FULL(1) cut or HALF cut(0).
//
********************************************************************************/
#if (0)
int RSPrintGREECE(BYTE *buf, int count, WORD prnCMD)//ccr2018-07-30
{
    struct GRAPH_DEFINE *sGraInfo;
    BYTE *sGraph;
    struct TPRN_POOL *sPrnPool;

    BYTE i;
    int  retval; /* value used in "goto out" statements */

    if ((ApplVar.PoolPushIn + Graph_TextCount + 1)%MAX_PRNPOOL == ApplVar.PoolPopOut)
    {
        do
        {
            if (!TestPrintGoingOn())//ccr2018-01-26
                Start_When_Ready(NULL);//ccr2018-01-26
//            rtc_Delayms(10);
//			msDelay(1000);

            i = (ApplVar.PoolPushIn + Graph_TextCount + 1)%MAX_PRNPOOL;
            if (i>ApplVar.PoolPopOut)
                i -= ApplVar.PoolPopOut;
            else
                i = ApplVar.PoolPopOut - i;
            Check_Paper();
        } while (i<3 && ApplVar.PoolPushIn !=ApplVar.PoolPopOut);//等待队列空余3个空间
    }

    if (count > PRNWIDTH+1)//include double high flag '@'
        count = PRNWIDTH+1;

    retval = 0;//set the flag: does not copy to pool
    if (BIT(prnCMD,PRN_GRAPHIC_Mode))//Print graphic
    {
        {//为打印图片命令
            if (count<=2)
                buf[2] = 0;// Use the default position
            if (count>=2 && buf[0]>0)
            {
                Prn_Status.g_bTextForGraph = false;//no text fot over-print

                memset(&GraphForPrint,0,sizeof(GraphForPrint));

                {
                    if (++ApplVar.PoolPushIn>=MAX_PRNPOOL) ApplVar.PoolPushIn = 0;
                    ApplVar.PoolForPrint[ApplVar.PoolPushIn].Length = 0;//flag-0: none text for graphic
                    ApplVar.PoolForPrint[ApplVar.PoolPushIn].GraphicNo = buf[0];
                    ApplVar.PoolForPrint[ApplVar.PoolPushIn].Prn_Command = prnCMD;
                    ApplVar.PoolForPrint[ApplVar.PoolPushIn].data[0] = buf[2];//Position of graphic
#if BARCODESIZE
                    if (buf[0]==BARCODE_GRP_ID && count>3)
                    {
                        i=count-3;
                        if (i>PRTLEN-3)
                            i=PRTLEN-3;
                        memcpy(ApplVar.PoolForPrint[ApplVar.PoolPushIn].data+2,buf+3,i);
                        ApplVar.PoolForPrint[ApplVar.PoolPushIn].data[1]=i;
                    }
#endif
                    Start_Motor(prnCMD);
                    return count;
                }
            }
        }
    }
    if (++ApplVar.PoolPushIn>=MAX_PRNPOOL) ApplVar.PoolPushIn = 0;
        sPrnPool = &ApplVar.PoolForPrint[ApplVar.PoolPushIn];

    // copy data to pool

    sPrnPool->Prn_Command = prnCMD;
    sPrnPool->GraphicNo = 0;
    for (i=retval=0;i<count && buf[i] == ' ' && retval<PRNWIDTH;i++)
    {

        sPrnPool->data[retval] = buf[i];
        retval++;
    }
    if (i<count)
    {
        if (buf[i] == '@')//double high flag
        {
            SETBIT(sPrnPool->Prn_Command,PRN_DBLH_Mode);
            i++;
        }

        for (;i<count && retval<PRNWIDTH;i++)
        {
            sPrnPool->data[retval] = buf[i];
            retval++;
        }
    }
    sPrnPool->Length = retval;
    return retval;
}
#endif
//ccr2018-03-01>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/*******************************************************
 * 设置Prn_Status.g_bHold标志
 *
 * @author EutronSoftware (2018-03-01)
 *****************************************************/
void PrintSetHold()
{
   Prn_Status.g_bHold=1;
}
/*******************************************************
 * 清除Prn_Status.g_bHold标志,并启动打印
 *
 * @author EutronSoftware (2018-03-01)
 *****************************************************/
void PrinAllOfHold()
{
   BYTE sOut;

   if (Prn_Status.g_bHold)
   {
	   Prn_Status.g_bHold=0;
	//ccr2018-04-28>>>>>>>
	   sOut=ApplVar.PoolPopOut;//ccr2018-04-28避免被优化掉
//		if ((Prn_TaskNow==TASK00_Stop || Prn_TaskNow==0xff))
	   if (Prn_TaskNow<=TASK00_Stop && ApplVar.PoolPushIn!=sOut)
	   {
		   sOut++;
		   if (sOut>=MAX_PRNPOOL) sOut = 0;
		   ApplVar.Prn_Command = ApplVar.PoolForPrint[sOut].Prn_Command;
		   Prn_TaskNow = -1;
		   Start_Motor(ApplVar.Prn_Command);
	   }
   }
//ccr2018-04-28<<<<<<<
}
//ccr2018-03-01<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
