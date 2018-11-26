/**
  ******************************************************************************
  * @file    tcp_echoclient.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    07-October-2011
  * @brief   Header file for tcp_echoclient.c
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
#ifndef __TCP_ECHOCLIENT_H__
#define __TCP_ECHOCLIENT_H__

#include "king.h"

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
/* ECHO protocol states */
enum echoclient_states
{
  ES_NOT_CONNECTED = 0,
  ES_CONNECTED,
  ES_RECEIVED,
  ES_CLOSING,
};



typedef struct
{
    BYTE XLine:1;                      /*!< bit0: 网线断开  */
    BYTE Ready:1;                  /*!< bit1: LWIP初始化好 */
    BYTE Connected:1;        /*!< bit2: 标示是否建立了Client,收款机只允许一个Client连接 */
} NETFLAGS;

/* Exported functions ------------------------------------------------------- */
/* Private function prototypes -----------------------------------------------*/
#if defined(__TCP_CLIENT_C__)

/* structure to be passed as argument to the tcp callbacks */
struct echoclient
{
  enum echoclient_states state; /* connection status */
  struct tcp_pcb *pcb;          /* pointer on the current tcp_pcb */
  struct pbuf *p_tx;            /* pointer on pbuf to be transmitted */
};


//供Tcp_Client.c说明用
void tcp_echoclient_connect(err_t (* connected)(void *arg,struct tcp_pcb *tpcb,err_t err));
static err_t tcp_echoclient_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void tcp_echoclient_connection_close(struct tcp_pcb *tpcb, struct echoclient * es);
static err_t tcp_echoclient_poll(void *arg, struct tcp_pcb *tpcb);
static err_t tcp_echoclient_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static void tcp_echoclient_send(struct tcp_pcb *tpcb, struct echoclient * es);
static err_t tcp_echoclient_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
static err_t TCP_SendFM_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
#endif

#if defined(CASE_ETHERNET)
void PingEthernet(void);
void ETHERNET_SetSendMode(void);
void ETHERNETSendECR_FM(void);
void ETHERNETSendECR_FM_All(void);
void ETHERNET_DownloadPLU(void);
void ETHERNET_DownloadDEPT(void);
void ETHERNET_DownloadCLERK(void);
void ETHERNET_DownloadHEAD(void);
void ETHERNET_DownloadPTRAIL(void);
void ETHERNET_DownloadALL(void);
void ETHERNET_Restart(void);

extern NETFLAGS Ethernet;
#endif

#endif /* __TCP_ECHOCLIENT_H__ */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
