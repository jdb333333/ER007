/**
  ******************************************************************************
  * @file    Project/STM32F2xx_StdPeriph_Template/stm32f2xx_it.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    13-April-2012
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "stm32f2xx_it.h"

//#include "king.h"		//king.h中的某个定义与以下的某个定义有冲突,会引起串口通信异常
//#include "exthead.h"
//#include "bios_dir.h"

#ifdef SPI_SD
#include "spi_sd.h"
#else
#include "sdio_sd.h"
#endif
#include "usart.h"
#include "TypeDef.h"
#if defined(CASE_ASCIIDISPLAY)
#include "lcd_160x.h"
#else
#include "chnDisp.h"
#endif

#include "usb_bsp.h"
#include "usb_hcd_int.h"
#include "usbh_core.h"

#include "usb_core.h"
#include "usbd_core.h"
#include "usb_conf.h"

#include "pwr_ctrl.h"
#include "SysTick.h"

#include "stm32f2x7_eth.h"
#include "Ethernet_app.h"
#include "netconf.h"


#if defined(CASE_ASCIIDISPLAY)	
extern volatile uint8_t LCD_BL_CTRL;
#endif

/** @addtogroup Template_Project
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/** SysTick Interrupt Handler (1ms)
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
volatile unsigned long RTCTimer=0;//系统计时器,以毫秒为单位递增计时
volatile unsigned long msTimer_Delay0=0,//延时计时器0
         msTimer_Delay1=0,//延时计时器1
         msTimer_Delay2=0;//延时计时器2

void SysTick_Handler(void)
{
  if (msTimer_Delay0)    msTimer_Delay0--;
  if (msTimer_Delay1)    msTimer_Delay1--;
  if (msTimer_Delay2)    msTimer_Delay2--;

  RTCTimer++;

  if (Prn_Delay)
  {//打印机因为过热而暂停
	  Prn_Delay++;//打印机暂停时长
	  if (Prn_Delay>PRNPAUSEMAX)
		  Prn_Delay = 0;//打印机结束暂停
  }
}

/*****************************************
* @brief  闹钟中断服务函数
* @param  None
* @retval None
*****************************************/
extern void RTC_AlarmSet(void);
extern void SetZReportMust(void);

void RTC_Alarm_IRQHandler(void)
{
  if(RTC_GetITStatus(RTC_IT_ALRA) != RESET)
  {
    RTC_ClearITPendingBit(RTC_IT_ALRA);
    EXTI_ClearITPendingBit(EXTI_Line17);
  }
  SetZReportMust();//ccr2017-12-18
	// 此处添加其它需处理的事情
	//
	//xputs("RTC_Alarm_IRQHandler...\n");

	//再次开启闹钟
	//RTC_AlarmSet();
}


/**
* @brief  This function handles USRAT interrupt request.
* @param  None
* @retval None
*/
extern void SetRTS(BYTE port);  //ouhs 20140814
void USART1_IRQHandler(void)
{
	uint8_t RxData;
	uint32_t sCount;

	/* USART in mode Tramitter -------------------------------------------------*/
	if (USART_GetITStatus(USART1, USART_IT_TXE) == SET)
	{
		/* Disable the USARTx transmit data register empty interrupt */
		//USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);
		UART1TxEmpty = 1;
	}

	/* USART in mode Receiver --------------------------------------------------*/
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		/* Receive the USART data */
		RxData = USART_ReceiveData(USART1);

		sCount = RxUART[1].PutIn+1;
		if (sCount>=rxBUFSIZE)
			sCount = 0;

		if (sCount != RxUART[1].GetOut)
		{
			RxUART[1].RxBuffer[RxUART[1].PutIn] = RxData;
			RxUART[1].PutIn = sCount;
			if (sCount>RxUART[1].GetOut)
				sCount -= RxUART[1].GetOut;
			else
				sCount += (rxBUFSIZE-RxUART[1].GetOut);
            if (RxUART[1].Status & XOFF_FLOWCTRL)
            {
	            if (sCount>XOFF_AT && !BIT(RxUART[1].Status ,XOFF_Flag))
	            {
	                SETBIT(RxUART[1].Status ,XOFF_Flag);
					UARTSend(LOGIC_COM2,XOFF);  //by ouhs
				}
            }
            else
            {
                if (RxUART[1].Status & RTS_FLOWCTRL)
                {
                    if (sCount>XOFF_AT && !BIT(RxUART[1].Status ,RTS_Flag))
                    {
                        SetRTS(0);
                        SETBIT(RxUART[1].Status ,RTS_Flag);
                    }
                }
            }
		}
	}
}

void USART3_IRQHandler(void)
{
	uint8_t RxData;
	uint32_t sCount;

	/* USART in mode Tramitter -------------------------------------------------*/
	if (USART_GetITStatus(USART3, USART_IT_TXE) == SET)
	{
        UART0TxEmpty = 1;
	}

	/* USART in mode Receiver --------------------------------------------------*/
	if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
	{
		/* Receive the USART data */
		RxData = USART_ReceiveData(USART3);
        sCount = RxUART[0].PutIn+1;//address for new data
        if (sCount>=rxBUFSIZE)
            sCount = 0;

        if (sCount != RxUART[0].GetOut)
        {//buffer dose not full
            RxUART[0].RxBuffer[RxUART[0].PutIn] = RxData;
            RxUART[0].PutIn = sCount;//point to the next address
            //count of the data in the buffer
            if (sCount>RxUART[0].GetOut)
                sCount -= RxUART[0].GetOut;
            else
                sCount += (rxBUFSIZE-RxUART[0].GetOut);
            if (RxUART[0].Status & XOFF_FLOWCTRL)
            {

	            if (sCount>XOFF_AT && !BIT(RxUART[0].Status ,XOFF_Flag))
	            {
	                SETBIT(RxUART[0].Status ,XOFF_Flag);
					UARTSend(LOGIC_COM1,XOFF);  //by ouhs
                }
            }
            else
            {
                if (RxUART[0].Status & RTS_FLOWCTRL)
                {
                    if (sCount>XOFF_AT && !BIT(RxUART[0].Status ,RTS_Flag))
                    {
                        SetRTS(1);
                        SETBIT(RxUART[0].Status ,RTS_Flag);
                    }
                }
            }
		}
	}
}

#if USART_EXTEND_EN
void USART6_IRQHandler(void)
{
	uint8_t RxData;
	uint32_t sCount;

	/* USART in mode Tramitter -------------------------------------------------*/
	if (USART_GetITStatus(USART6, USART_IT_TXE) == SET)
	{
		/* Disable the USARTx transmit data register empty interrupt */
		//USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);
		UART2TxEmpty = 1;
	}

	/* USART in mode Receiver --------------------------------------------------*/
	if (USART_GetITStatus(USART6, USART_IT_RXNE) == SET)
	{
		/* Receive the USART data */
		RxData = USART_ReceiveData(USART6);//        USART_SendData(UART0, RxData);//testonly//ccr2014-12-25   			// 写入数据

		sCount = RxUART[2].PutIn+1;
		if (sCount>=rxBUFSIZE)
			sCount = 0;

		if (sCount != RxUART[2].GetOut)
		{
			RxUART[2].RxBuffer[RxUART[2].PutIn] = RxData;
			RxUART[2].PutIn = sCount;
			if (sCount>RxUART[2].GetOut)
				sCount -= RxUART[2].GetOut;
			else
				sCount += (rxBUFSIZE-RxUART[2].GetOut);

            if (RxUART[2].Status & XOFF_FLOWCTRL)
            {
	            if (sCount>XOFF_AT && !BIT(RxUART[2].Status ,XOFF_Flag))
	            {
	                SETBIT(RxUART[2].Status ,XOFF_Flag);
					UARTSend(LOGIC_COM3,XOFF);  //by ouhs
                }
            }
            else
            {
                if (RxUART[2].Status & RTS_FLOWCTRL)
                {
                    if (sCount>XOFF_AT && !BIT(RxUART[2].Status ,RTS_Flag))
                    {
                        SetRTS(2);
                        SETBIT(RxUART[2].Status ,RTS_Flag);
                    }
                }
            }
		}
	}
}
#endif	//USART_EXTEND_EN



/******************************************************************************/
/*                 STM32F2xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f2xx.s).                                               */
/******************************************************************************/
extern volatile uint8_t KeyScanEnable;

/**
  * @brief  This function handles External line 2 interrupt request.
  * @param  None
  * @retval None
  */
extern void (*PWF_CallBack)(void);//ccr20110901
extern volatile uint8_t KeyScanEnable; //ouhs 20140814
void EXTI0_IRQHandler(void)
{//掉电中断服务程序
    uint16_t t1,t2;

//ccr2017-02-21	EXTI_ClearITPendingBit(EXTI_Line0);

	if((GPIOB->IDR & GPIO_Pin_0)==0)	//判断是掉电信号 PB0
	{
#if defined(CASE_ASCIIDISPLAY)
		Copy_LCD(false);
#else
		mLightLCDOff();
#if defined(CASE_ER260F)
		LED_Off();
#endif
#endif

#if (!defined(ApplVarInSRAM))
    Save_Config(false);//ccr2016-01-15
//    if (ApplVar_Restored==0x5a)
		Save_ApplRam(); //Save_Config(true);
#endif

#ifdef CASE_FATFS_EJ
    ff_CloseFile();
    ff_MountSDisk();
#endif


    //ccr2014-08-27>>>>>>>>>>>>>>>>
#if POWERCTRL
    if (DC_DET_GET())/* TRUE--适配器供电, FALSE--电池供电*/
#endif
    {//适配器供电
        t2 = GPIOB->IDR & GPIO_Pin_0;//判断掉电信号检测位
        do {
            t1 = t2;
            for (t2=0;t2<5000;t2++){};
            t2 = GPIOB->IDR & GPIO_Pin_0;
        } while (t1!=t2);

        if (t1==0)
        {//低电平:掉电
            __disable_irq();    //开机有打印时,有可能引起掉电,此时禁止所有中断将引起系统运行异常
            __disable_fault_irq();
            t2 = GPIOB->IDR & GPIO_Pin_0;//判断掉电信号检测位
            do {
                t1 = t2;
                for (t2=0;t2<5000;t2++){};
                t2 = GPIOB->IDR & GPIO_Pin_0;
            } while (t1!=t2 || t2==0);
            __enable_irq();    //开机有打印时,有可能引起掉电,此时禁止所有中断将引起系统运行异常
            __enable_fault_irq();
            Start_When_Ready(0);//如果有未完成的打印任务,启动打印
        }
    }
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	}
	EXTI_ClearITPendingBit(EXTI_Line0);
}

/**
  * @brief  This function handles External line 3 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI3_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line3) != RESET)
  {
		if((ETH_LINK_GPIO_PORT->IDR & ETH_LINK_PIN)==0)	//判断是网络连接信号 PC3
		{
#ifdef CASE_ETHERNET
			Eth_Link_ITHandler(IP101_PHY_ADDRESS);

			//USART_printf("Network Cable is changed\r\n");  //testonly by ouhs
#endif
		}		
		/* Clear interrupt pending bit */
		EXTI_ClearITPendingBit(EXTI_Line3);
  }
}



//按键中断
void EXTI9_5_IRQHandler(void)
{
#if defined(CASE_ECR100F) || defined(CASE_ECR100S)
 
	if(EXTI_GetITStatus(EXTI_Line8) != RESET)
  {	
	  KeyScanEnable = 1;
	  EXTI_ClearITPendingBit(EXTI_Line8);
  }
#endif
}

#if defined(CASE_ECR99) || defined(CASE_MCR30)
//按键中断
void EXTI15_10_IRQHandler(void)
{
	//xprintf("EXTI15_10_IRQHandler...");
	if(EXTI_GetITStatus(EXTI_Line11) != RESET)
  {	
	  KeyScanEnable = 1;
	  EXTI_ClearITPendingBit(EXTI_Line11);
  }
	if(EXTI_GetITStatus(EXTI_Line12) != RESET)
  {	
	  KeyScanEnable = 1;
	  EXTI_ClearITPendingBit(EXTI_Line12);
  }
	
	if(EXTI_GetITStatus(EXTI_Line13) != RESET)
  {	
	  KeyScanEnable = 1;
	  EXTI_ClearITPendingBit(EXTI_Line13);
  }
	
	if(EXTI_GetITStatus(EXTI_Line14) != RESET)
  {	
	  KeyScanEnable = 1;
	  EXTI_ClearITPendingBit(EXTI_Line14);
  }

	if(EXTI_GetITStatus(EXTI_Line15) != RESET)
  {	
	  KeyScanEnable = 1;
	  EXTI_ClearITPendingBit(EXTI_Line15);
  }	

}
#endif

#ifndef SPI_SD

/**
  * @brief  This function handles SDIO global interrupt request.
  * @param  None
  * @retval None
  */
void SDIO_IRQHandler(void)
{
  /* Process All SDIO Interrupt Sources */
  SD_ProcessIRQSrc();
}

/**
  * @brief  This function handles DMA2 Stream3 or DMA2 Stream6 global interrupts
  *         requests.
  * @param  None
  * @retval None
  */
void SD_SDIO_DMA_IRQHANDLER(void)
{
  /* Process DMA2 Stream3 or DMA2 Stream6 Interrupt Sources */
  SD_ProcessDMAIRQ();
}


#endif



/**
  * @brief  OTG_FS_IRQHandler
  *          This function handles USB-On-The-Go FS global interrupt request.
  *          requests.
  * @param  None
  * @retval None
  */

extern USB_OTG_CORE_HANDLE          USB_OTG_Core;
extern USB_OTG_CORE_HANDLE  				USB_OTG_dev;

void OTG_FS_IRQHandler(void)
{
	USBH_OTG_ISR_Handler(&USB_OTG_Core);
	
	USBD_OTG_ISR_Handler (&USB_OTG_dev);
}

/** 当收到网络数据时,产生中断
  * @brief  This function handles ethernet DMA interrupt request.
  * @param  None
  * @retval None
  */
BYTE ETHCheck=0;//ccr2017-10-16/解决无法自动获取IP地址的问题
extern __IO uint32_t LocalTime;
void ETH_IRQHandler(void)
{
#ifdef CASE_ETHERNET
  //xputs("ETH_IRQHandler...\n");//testonly
  /* Frame received */
	//if ( ETH_GetDMAFlagStatus(ETH_DMA_FLAG_R | ETH_DMA_IT_NIS) == SET)
  if (Get_ETHERNET_Ready())//ccr2017-02-21
  {
    /* check if any packet received */
    if (ETH_CheckFrameReceived())
    {
      /* process received ethernet packet */
      LwIP_Pkt_Handle();
    }
    /* handle periodic timers for LwIP */
    ETHCheck=1;
    LwIP_Periodic_Handle(LocalTime);
  }
  /* Clear the interrupt flags. */
  /* Clear the Eth DMA Rx IT pending bits */
  ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
  ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
#endif

}

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
