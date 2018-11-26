/**
  ******************************************************************************
  * @file    main.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    07-October-2011
  * @brief   This file contains all the functions prototypes for the main.c
  *          file.
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
#ifndef __ETHERNET_APP_H
#define __ETHERNET_APP_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"
#include "stm32f2x7_eth_bsp.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

#define USE_DHCP       /* enable DHCP, if disabled static address is used */

/* Uncomment SERIAL_DEBUG to enables retarget of printf to serial port (COM1 on STM32 evalboard)
   for debug purpose */
//#define SERIAL_DEBUG

#define OTP_FLASH_ADDR	0x1FFF7800

#ifndef MAC_ADDR
#define MAC_ADDR ((__IO uint8_t *)0x1FFF7800) //ccr2017-03-01MAC地址的存储为止
#endif
/* MAC ADDRESS: MAC_ADDR0:MAC_ADDR1:MAC_ADDR2:MAC_ADDR3:MAC_ADDR4:MAC_ADDR5 */
//#define MAC_ADDR0   0x00
//#define MAC_ADDR1   0x01
//#define MAC_ADDR2   0x6c
//#define MAC_ADDR3   0xa3
//#define MAC_ADDR4   0x7e
//#define MAC_ADDR5   0x0a

//ccr2017-02-16>>>>>>>>>>>>>>>>>>>>
/*Static IP ADDRESS: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3 */
//#define IP_ADDR0   192
//#define IP_ADDR1   168
//#define IP_ADDR2   0
//#define IP_ADDR3   1
//
///*NETMASK*/
//#define NETMASK_ADDR0   //255
//#define NETMASK_ADDR1   //255
//#define NETMASK_ADDR2   //255
//#define NETMASK_ADDR3   //0
//
///*Gateway Address*/
//#define GW_ADDR0   192
//#define GW_ADDR1   168
//#define GW_ADDR2   1
//#define GW_ADDR3   1
//ccr2017-02-16<<<<<<<<<<<<<<<
/* MII and RMII mode selection, for STM322xG-EVAL Board(MB786) RevB ***********/
#define RMII_MODE  // User have to provide the 50 MHz clock by soldering a 50 MHz
                     // oscillator (ref SM7745HEV-50.0M or equivalent) on the U3
                     // footprint located under CN3 and also removing jumper on JP5.
                     // This oscillator is not provided with the board.
                     // For more details, please refer to STM3220G-EVAL evaluation
                     // board User manual (UM1057).


//#define MII_MODE

/* Uncomment the define below to clock the PHY from external 25MHz crystal (only for MII mode) */
#ifdef 	MII_MODE
 #define PHY_CLOCK_MCO
#endif

/* STM322xG-EVAL jumpers setting
    +==========================================================================================+
    +  Jumper |       MII mode configuration            |      RMII mode configuration         +
    +==========================================================================================+
    +  JP5    | 2-3 provide 25MHz clock by MCO(PA8)     |  Not fitted                          +
    +         | 1-2 provide 25MHz clock by ext. Crystal |                                      +
    + -----------------------------------------------------------------------------------------+
    +  JP6    |          2-3                            |  1-2                                 +
    + -----------------------------------------------------------------------------------------+
    +  JP8    |          Open                           |  Close                               +
    +==========================================================================================+
  */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Time_Update(void);
void Delay(uint32_t nCount);
void Ethernet_Start(void);
uint8_t Get_ETHERNET_Ready(void);
void Set_ETHERNET_Ready(uint8_t ready);

extern __IO uint32_t LocalTime;

#ifdef __cplusplus
}
#endif

#define cmdUPDATE       'U'     //为更新数据请求
#define cmdDOWNLOAD     'D'     //为下载数据请求
#define cmdFLOWDATA     'F'     //为请求收款机流水数据
#define cmdANSWER       'A'     //为对host的应答命令
#define cmdGRAPHIC      'G'     //为下载图片
#define cmdCLEAR        'C'     //清除报表
#define cmdINIT         'I'     //收款机初始化
#define cmdSERVER       'S'     //设置收款机的服务器地址
#define cmdPASSWORD     'W'     //设置收款员密码


//ccr2016-12-26>>>>>>>>>>>>>>>>>>>>>>
#define SCREENWD    DISLEN

//ccr2016-12-26<<<<<<<<<<<<<<<<<<<<<<

#endif /* __ETHERNET_APP_H */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

