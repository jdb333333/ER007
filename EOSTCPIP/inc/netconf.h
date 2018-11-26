/**
  ******************************************************************************
  * @file    netconf.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    07-October-2011
  * @brief   This file contains all the functions prototypes for the netconf.c
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
#ifndef __NETCONF_H
#define __NETCONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void LwIP_Init(void);
void LwIP_Pkt_Handle(void);
void LwIP_Periodic_Handle(__IO uint32_t localtime);

#ifdef __cplusplus
}
#endif

//ccr2017-02-16>>>>>>>>>>>>>>>>>>>>
#define DEST_IP_ADDR0   ApplVar.AP.NetWork.ServerIP[0]  //192
#define DEST_IP_ADDR1   ApplVar.AP.NetWork.ServerIP[1]  //168
#define DEST_IP_ADDR2   ApplVar.AP.NetWork.ServerIP[2]  //1
#define DEST_IP_ADDR3   ApplVar.AP.NetWork.ServerIP[3]  //72

#define DEST_PORT       ApplVar.AP.NetWork.ServerPort//8080


/*Static IP ADDRESS: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3 */
#define IP_ADDR0   ApplVar.AP.NetWork.IPAddress[0]  //192
#define IP_ADDR1   ApplVar.AP.NetWork.IPAddress[1]  //168
#define IP_ADDR2   ApplVar.AP.NetWork.IPAddress[2]  //0
#define IP_ADDR3   ApplVar.AP.NetWork.IPAddress[3]  //1

/*NETMASK*/
#define NETMASK_ADDR0   ApplVar.AP.NetWork.IPMask[0]//255
#define NETMASK_ADDR1   ApplVar.AP.NetWork.IPMask[1]//255
#define NETMASK_ADDR2   ApplVar.AP.NetWork.IPMask[2]//255
#define NETMASK_ADDR3   ApplVar.AP.NetWork.IPMask[3]//0

/*Gateway Address*/
#define GW_ADDR0   ApplVar.AP.NetWork.GateWay[0]//192
#define GW_ADDR1   ApplVar.AP.NetWork.GateWay[1]//168
#define GW_ADDR2   ApplVar.AP.NetWork.GateWay[2]//1
#define GW_ADDR3   ApplVar.AP.NetWork.GateWay[3]//1
//ccr2017-02-16<<<<<<<<<<<<<<<

//ccr2017-10-30>>>>>>>>>>>>>>>
      /* initialize default DNS primary server */
#define  DNSPri_ADDR0   ApplVar.AP.NetWork.PrimaryDNS[0]//192
#define  DNSPri_ADDR1   ApplVar.AP.NetWork.PrimaryDNS[1]//168
#define  DNSPri_ADDR2   ApplVar.AP.NetWork.PrimaryDNS[2]//1
#define  DNSPri_ADDR3   ApplVar.AP.NetWork.PrimaryDNS[3]//1
      /* initialize default DNS secondary server */
#define  DNS2nd_ADDR0   ApplVar.AP.NetWork.SecondDNS[0]//192
#define  DNS2nd_ADDR1   ApplVar.AP.NetWork.SecondDNS[1]//168
#define  DNS2nd_ADDR2   ApplVar.AP.NetWork.SecondDNS[2]//1
#define  DNS2nd_ADDR3   ApplVar.AP.NetWork.SecondDNS[3]//1
//ccr2017-10-30<<<<<<<<<<<<<<


#endif /* __NETCONF_H */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

