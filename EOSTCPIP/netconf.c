/**
  ******************************************************************************
  * @file    netconf.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    07-October-2011
  * @brief   Network connection configuration
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
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "ethernetif.h"
#include "Ethernet_app.h"
#include "netconf.h"
#include <stdio.h>

#include "king.h"  //ccr2017-02-17
#include "exthead.h"
#include "exth.h"
#include "interface.h"//ccr2017-02-17
/* Private typedef -----------------------------------------------------------*/
#define MAX_DHCP_TRIES        4

//jdb2018-09-03
#define XPRINTF(message) do { \
                                 xprintf message; \
                             } while(0)

err_t netconn_gethostbyname(const char *name, struct ip_addr *addr);
/* Private define ------------------------------------------------------------*/
typedef enum
{
  DHCP_START=0,
  DHCP_WAIT_ADDRESS,
  DHCP_ADDRESS_ASSIGNED,
  DHCP_TIMEOUT
}
DHCP_State_TypeDef;
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
struct netif netif;
uint32_t TCPTimer = 0;
uint32_t ARPTimer = 0;

#ifdef USE_DHCP
uint32_t DHCPfineTimer = 0;
uint32_t DHCPcoarseTimer = 0;
DHCP_State_TypeDef DHCP_state = DHCP_START;
#endif

/* Private functions ---------------------------------------------------------*/
void LwIP_DHCP_Process_Handle(void);

/**
 *
 *
 * @author EutronSoftware (2017-02-15)
  * @brief  Initializes the lwIP stack
  * @param  None
  * @retval None
  */
void LwIP_Init(void)
{
  struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;


  /* Initializes the dynamic memory heap defined by MEM_SIZE.*/
  mem_init();

  /* Initializes the memory pools defined by MEMP_NUM_x.*/
  memp_init();
#ifdef USE_DHCP//ccr2017-09-25>>>>>>
  if (BIT(ApplVar.AP.NetWork.Option,DHCPFlag))
  {
      ipaddr.addr = 0;
      netmask.addr = 0;
      gw.addr = 0;
  }
  else//ccr2017-09-25
#endif//#else
  {
      IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
      IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1 , NETMASK_ADDR2, NETMASK_ADDR3);
      IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);

      netif_set_addr(&netif, &ipaddr , &netmask, &gw);//ccr2018-07-13

      XPRINTF(("\n>Static IP address: %u.%u.%u.%u\n", IP_ADDR0, IP_ADDR1,IP_ADDR2, IP_ADDR3));

  }//ccr2017-09-25#endif
//ccr2017-09-25<<<<<<<<<<<<<<<<

  /* - netif_add(struct netif *netif, struct ip_addr *ipaddr,
            struct ip_addr *netmask, struct ip_addr *gw,
            void *state, err_t (* init)(struct netif *netif),
            err_t (* input)(struct pbuf *p, struct netif *netif))

   Adds your network interface to the netif_list. Allocate a struct
  netif and pass a pointer to this structure as the first argument.
  Give pointers to cleared ip_addr structures when using DHCP,
  or fill them with sane numbers otherwise. The state pointer may be NULL.

  The init function pointer must point to a initialization function for
  your ethernet netif interface. The following code illustrates it's use.*/
  netif_add(&netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);

  /*  Registers the default network interface.*/
  netif_set_default(&netif);

  /*  When the netif is fully configured this function must be called.*/
  netif_set_up(&netif);
  //ccr2017-10-30>>>>>>>>>>>>>>>>>>
#if LWIP_DNS
  dns_init();
#endif /* LWIP_DNS */
  //ccr2017-10-30<<<<<<<<<<<<<<<<<<

}

/**
  * @brief  Called when a frame is received
  * @param  None
  * @retval None
  */
void LwIP_Pkt_Handle(void)
{
  /* Read a received packet from the Ethernet buffers and send it to the lwIP for handling */
  ethernetif_input(&netif);
}

/**
  * @brief  LwIP periodic tasks
  * @param  localtime the current LocalTime value
  * @retval None
  */
void LwIP_Periodic_Handle(__IO uint32_t localtime)
{
#if LWIP_TCP
  /* TCP periodic process every 250 ms */
  if (localtime - TCPTimer >= TCP_TMR_INTERVAL)
  {
    TCPTimer =  localtime;
    tcp_tmr();
  }
#endif

  /* ARP periodic process every 5s */
  if ((localtime - ARPTimer) >= ARP_TMR_INTERVAL)
  {
    ARPTimer =  localtime;
    etharp_tmr();
  }
#ifdef USE_DHCP
  if (BIT(ApplVar.AP.NetWork.Option,DHCPFlag))//ccr2017-09-25
  {
      /* Fine DHCP periodic process every 500ms */
      if (localtime - DHCPfineTimer >= DHCP_FINE_TIMER_MSECS)
      {
        DHCPfineTimer =  localtime;
        dhcp_fine_tmr();
        if ((DHCP_state != DHCP_ADDRESS_ASSIGNED)&&(DHCP_state != DHCP_TIMEOUT))
        {
          /* toggle LED1 to indicate DHCP on-going process */
          //STM_EVAL_LEDToggle(LED1);

          /* process DHCP state machine */
          LwIP_DHCP_Process_Handle();
        }
      }

      /* DHCP Coarse periodic process every 60s */
      if (localtime - DHCPcoarseTimer >= DHCP_COARSE_TIMER_MSECS)
      {
        DHCPcoarseTimer =  localtime;
        dhcp_coarse_tmr();
      }

  }
#endif
}

#ifdef USE_DHCP
/**
  * @brief  LwIP_DHCP_Process_Handle
  * @param  None
  * @retval None
  */
void LwIP_DHCP_Process_Handle()
{
  struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;
  uint32_t IPaddress;

  switch (DHCP_state)
  {
    case DHCP_START:
    {
      RESETBIT(ApplVar.AP.NetWork.Option,IPByDHCPFlag);//ccr2017-09-28
      dhcp_start(&netif);
      IPaddress = 0;
      DHCP_state = DHCP_WAIT_ADDRESS;
      XPRINTF(("\n>Looking for DHCP server,please wait...\n"));
    }
    break;

    case DHCP_WAIT_ADDRESS:
    {
      /* Read the new IP address */
      IPaddress = netif.ip_addr.addr;

      if (IPaddress!=0)
      {
        DHCP_state = DHCP_ADDRESS_ASSIGNED;

        /* Stop DHCP */
        dhcp_stop(&netif);
        SETBIT(ApplVar.AP.NetWork.Option,IPByDHCPFlag);//ccr2017-09-28得到了IP地址

        IP_ADDR3 = (uint8_t)(IPaddress >> 24); //ccr2017-09-25
        IP_ADDR2 = (uint8_t)(IPaddress >> 16); //ccr2017-09-25
        IP_ADDR1 = (uint8_t)(IPaddress >> 8);  //ccr2017-09-25
        IP_ADDR0 = (uint8_t)(IPaddress);       //ccr2017-09-25

        IPaddress = netif.gw.addr;
        GW_ADDR3 = (uint8_t)(IPaddress >> 24); //ccr2017-09-25
        GW_ADDR2 = (uint8_t)(IPaddress >> 16); //ccr2017-09-25
        GW_ADDR1 = (uint8_t)(IPaddress >> 8);  //ccr2017-09-25
        GW_ADDR0 = (uint8_t)(IPaddress);       //ccr2017-09-25

        IPaddress = netif.netmask.addr;
        NETMASK_ADDR3 = (uint8_t)(IPaddress >> 24); //ccr2017-09-25
        NETMASK_ADDR2 = (uint8_t)(IPaddress >> 16); //ccr2017-09-25
        NETMASK_ADDR1 = (uint8_t)(IPaddress >> 8);  //ccr2017-09-25
        NETMASK_ADDR0 = (uint8_t)(IPaddress);       //ccr2017-09-25

#if 0 //ccr2017-10-31>>>>>
        //自动获取DNS服务器
        IPaddress = netif.dhcp->offered_dns_addr[0].addr;
        ApplVar.AP.NetWork.PrimaryDNS[3] = (uint8_t)(IPaddress >> 24); //ccr2017-09-25
        ApplVar.AP.NetWork.PrimaryDNS[2] = (uint8_t)(IPaddress >> 16); //ccr2017-09-25
        ApplVar.AP.NetWork.PrimaryDNS[1] = (uint8_t)(IPaddress >> 8);  //ccr2017-09-25
        ApplVar.AP.NetWork.PrimaryDNS[0] = (uint8_t)(IPaddress);       //ccr2017-09-25

		IPaddress = netif.dhcp->offered_dns_addr[1].addr;
        ApplVar.AP.NetWork.SecondDNS[3] = (uint8_t)(IPaddress >> 24); //ccr2017-09-25
        ApplVar.AP.NetWork.SecondDNS[2] = (uint8_t)(IPaddress >> 16); //ccr2017-09-25
        ApplVar.AP.NetWork.SecondDNS[1] = (uint8_t)(IPaddress >> 8);  //ccr2017-09-25
        ApplVar.AP.NetWork.SecondDNS[0] = (uint8_t)(IPaddress);       //ccr2017-09-25
#endif //ccr2017-10-31<<<<<
        /* Display the IP address */
        XPRINTF((">IP address from DHCP server:"));
        XPRINTF(("  %u.%u.%u.%u\n", IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3));

        //STM_EVAL_LEDOn(LED1);
      }
      else
      {
        /* DHCP timeout */
        if (netif.dhcp->tries > MAX_DHCP_TRIES)
        {
          DHCP_state = DHCP_TIMEOUT;

          /* Stop DHCP */
          dhcp_stop(&netif);

          /* Static address used */
          IP4_ADDR(&ipaddr, IP_ADDR0 ,IP_ADDR1 , IP_ADDR2 , IP_ADDR3 );
          IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
          IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
          netif_set_addr(&netif, &ipaddr , &netmask, &gw);

          XPRINTF((">DHCP timeout,Static IP used:"));
          XPRINTF(("  %u.%u.%u.%u\n", IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3));
          //STM_EVAL_LEDOn(LED1);
        }
      }
    }
    break;
    default: break;
  }
}
#endif

//ccr2017-09-27>>>For DNS>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//dns_gethostbyname通过域名获取IP地址成功时的回调函数
//BYTE IPByDNS[4];
//#define IPByDNS ApplVar.AP.NetWork.ServerIP

void IPByDNS_Found(const char *name, struct ip_addr *ipaddr, void *arg)
{
    BYTE *IPByDNS=arg;
    if (IPByDNS!=NULL)
    {
        IPByDNS[0]  = ipaddr->addr;
        IPByDNS[1]  = ipaddr->addr>>8;
        IPByDNS[2]  = ipaddr->addr>>16;
        IPByDNS[3]  = ipaddr->addr>>24;
        XPRINTF((">IP Address for %s:%d.%d.%d.%d\n",name,IPByDNS[0], IPByDNS[1], IPByDNS[2], IPByDNS[3]));
	}
    RESETBIT(ApplVar.AP.NetWork.Option,DNSPROGRESSFlag);
}


/************************************************************************
 * 从URL中分离出域名,然后根据DNS解析出其IP地址
 *
 * @author EutronSoftware (2017-10-30)
 *
 * @param pURL :URL,如:"http://www.abcd.efg/myweb/websend.php"
 * @param pIP :解析出的IP地址(4字节),
 * @param pBlock :=true,以阻塞方式等待解析出的IP地址(4字节),
 *
 * @return BYTE:=true,解析成功;=false,解析失败
 ************************************************************************/

struct ip_addr IPAddr;//ccr2017-10-31供dns_gethostbyname使用,但是在得到地址后,其值却总是为0xffffffff

BYTE GetIPByDNSFromURL(BYTE *pIP,char *pURL,char pBlock)
{
    err_t k;
    int i,j,l;
    char domain[32];
    ULONG delayFor;//+SECONDS(ACTIVE*60);//ccr2015-03-11

    if (BIT(ApplVar.AP.NetWork.Option,DNSPROGRESSFlag))
        return false;//上一次域名解析没有结束

    l=strlen(pURL);
    if (l>=5 && CLONG(pURL[0])==(('p'<<24) +('t'<<16)+('t'<<8)+('h')) && pURL[4]==':')
    {
        if (l>=7 && CWORD(pURL[5])==(('/'<<8) +'/'))
            i=7;
        else
            i=5;
    }
    else
        i=0;
    for (j=0;(j<sizeof(domain)-1)&&i<l;j++)
    {
        if (pURL[i]!='/')
            domain[j]=pURL[i++];
        else
            break;
    }
    domain[j]=0;//得到域名
    if (j>7)//为有效域名,至少是"www.a.b"
    {
        for (i=0;i<j;i++)
        {
            if (domain[i]<'0' && domain[i]!='.' || domain[i]>'9')
                break;
        }
        if (i<j)
        {//为字符组成的域名
            if (CLONG(ApplVar.AP.NetWork.PrimaryDNS[0]))
            {
                 //if (netconn_gethostbyname(URL, &ipAddr)==ERR_OK)
                 //CLONG(IPByDNS[0])=0;
                 SETBIT(ApplVar.AP.NetWork.Option,DNSPROGRESSFlag);
                 k=dns_gethostbyname(domain,&IPAddr,IPByDNS_Found,pIP);
                 if (pBlock && (k==ERR_OK || k==ERR_INPROGRESS))
                 {
                     delayFor=GetSystemTimer();
                     while (GetSystemTimer()-delayFor<SECONDS(60))
                     {
                         if (!BIT(ApplVar.AP.NetWork.Option,DNSPROGRESSFlag))//(CLONG(IPByDNS[0])!=0)
                         {
                             //CLONG(pIP)=CLONG(IPByDNS[0]);
                             return true;
                         }
                     }
                 }
                 else
                     return (pBlock);
            }
        }
        else
        {//域名为直接的IP地址
            return (IPToNum(pIP,domain,j)==0);
        }
    }
    return false;
}
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
