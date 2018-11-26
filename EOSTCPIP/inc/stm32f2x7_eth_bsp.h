/**
  ******************************************************************************
  * @file    stm32f2x7_eth_bsp.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    07-October-2011 
  * @brief   Header for stm32f2x7_eth_bsp.c file.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F2x7_ETH_BSP_H
#define __STM32F2x7_ETH_BSP_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define DP83848_PHY_ADDRESS       0x01 /* Relative to STM322xG-EVAL Board */
#define IP101_PHY_ADDRESS       	0x01
	 
/* Specific defines for EXTI line, used to manage Ethernet link status */
#define ETH_LINK_EXTI_LINE             EXTI_Line3
#define ETH_LINK_EXTI_PORT_SOURCE      EXTI_PortSourceGPIOC
#define ETH_LINK_EXTI_PIN_SOURCE       EXTI_PinSource3
#define ETH_LINK_EXTI_IRQn             EXTI3_IRQn 
/* PC03 */
#define ETH_LINK_PIN                   GPIO_Pin_3
#define ETH_LINK_GPIO_PORT             GPIOC
#define ETH_LINK_GPIO_CLK              RCC_AHB1Periph_GPIOC

#define PHY_DP83848C	0	 
	 
/* PHY registers */
#if PHY_DP83848C	//DP83848C
#define PHY_MICR                  0x11 /* MII Interrupt Control Register */
#define PHY_MICR_INT_EN           ((uint16_t)0x0002) /* PHY Enable interrupts */
#define PHY_MICR_INT_OE           ((uint16_t)0x0001) /* PHY Enable output interrupt events */
#define PHY_MISR                  0x12 /* MII Interrupt Status and Misc. Control Register */
#define PHY_MISR_LINK_INT_EN      ((uint16_t)0x0020) /* Enable Interrupt on change of link status */
#define PHY_LINK_STATUS           ((uint16_t)0x2000) /* PHY link status interrupt mask */

#else //IP101A
#define PHY_MICR                  0x11
#define PHY_MICR_INT_EN           ((uint16_t)0x0800)	//0:开中断
#define PHY_MICR_INT_OE           ((uint16_t)0x8000)	//1:使能中断输出
#define PHY_MICR_LINK_INT_EN      ((uint16_t)0x0100)	//0：开链路中断
#define PHY_LINK_INT_STATUS       ((uint16_t)0x0001)	

#define PHY_MISR                  0x12
#define PHY_LINK_STATUS       		((uint16_t)0x0400)	//1:Link status ok; 0:Link status fail

#define PHY_BMCR									0x0
#define PHY_BMCR_POWER_DOWN				0x0800							//1:Power down; 0:Normal operation
#define PHY_BMCR_RESET						0x8000							//1:Software reset; 0:Normal operation

//IP101G 需要设置以下寄存器用于INTR中断输出
#define PHY_IOCTRL                0x1D 								//Digital I/O Specific Control Register
#define PHY_IOCTRL_SELINTR        0x04								//1: INTR function   0: RXER function

#endif

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void  ETH_BSP_Config(void);
uint32_t Eth_Link_PHYITConfig(uint16_t PHYAddress);
void Eth_Link_EXTIConfig(void);
void Eth_Link_ITHandler(uint16_t PHYAddress);

uint8_t Eth_Link_Status(void);//检测是否插了网线；１－表示插入了网线；０－无网线
void Eth_PHY_PowerModeCtrl(FunctionalState ctrl);//是否关闭ＰＨＹ

uint8_t OTPFlash_WriteMAC(uint8_t* pBuffer);
uint8_t OTPFlash_ReadMAC(uint8_t* pBuffer);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F2x7_ETH_BSP_H */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
