/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    07-October-2011
  * @brief   Main program body
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
#ifdef CASE_ETHERNET

#include "stm32f2x7_eth.h"
#include "netconf.h"
#include "Ethernet_app.h"
#include "Tcp_Client.h"
//ccr2017-01-19#include "httpd.h"

#include "king.h"  //ccr2017-02-17
#include "exthead.h" //ccr2017-02-17
#include "interface.h"//ccr2017-02-17

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SYSTEMTICK_PERIOD_MS  10


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
 * Ethernet 初始化
 *
 * @author EutronSoftware (2017-02-15)
 *
 */
void Ethernet_Start()
{
  if (BIT(ApplVar.AP.NetWork.Option,DHCPFlag) || CLONG(ApplVar.AP.NetWork.IPAddress[0])==0)//ccr2017-09-25
      RESETBIT(ApplVar.AP.NetWork.Option,IPByDHCPFlag);//ccr2017-09-25
  else
      SETBIT(ApplVar.AP.NetWork.Option,IPByDHCPFlag);//ccr2017-09-28非DHCP方式,默认IP有效

  RESETBIT(ApplVar.AP.NetWork.Option,DNSPROGRESSFlag);//ccr2017-09-25

  Set_ETHERNET_Ready(0);
  /* configure ethernet (GPIOs, clocks, MAC, DMA) */
  ETH_BSP_Config();

  /* Initilaize the LwIP stack */
  LwIP_Init();//ccr2017-02-15

  /* Http webserver Init */
//ccr2017-01-19  httpd_init();
  Set_ETHERNET_Ready(1);

}


/**
  * @brief  Updates the system local time
  * @param  None
  * @retval None
  */
void Time_Update(void)
{
  LocalTime += SYSTEMTICK_PERIOD_MS;
}



/**
 *
 *
 * @author EutronSoftware (2017-02-24)
 *
 * @param ready :=0时,强制设置网络为断网状态;
 *              >0时,根据网络检测状态设置网络状态
 */
extern __IO uint32_t  EthInitStatus;
void Set_ETHERNET_Ready(uint8_t ready)
{
    Ethernet.Ready=(EthInitStatus!=0) && ready;
    Ethernet.XLine=!Ethernet.Ready;
}

uint8_t Get_ETHERNET_Ready()
{
    return (Ethernet.Ready);
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
#endif