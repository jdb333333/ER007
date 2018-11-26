#define __TCP_CLIENT_C__

/**
  ******************************************************************************
  * @file    tcp_echoclient.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    07-October-2011
  * @brief   tcp echoclient application using LwIP RAW API
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
#ifdef CASE_ETHERNET
/* Includes ------------------------------------------------------------------*/
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "Ethernet_app.h"
#include "memp.h"

#if LWIP_TCP

#include <stdio.h>
#include <string.h>

#include "netconf.h" //ccr2017-02-17
#include "king.h"  //ccr2017-02-17
#include "exthead.h" //ccr2017-02-17
#include "exth.h" //ccr2017-02-17
#include "interface.h"//ccr2017-02-17

#include "tcp_client.h"//放置到"king.h"之后

#include "message.h"


/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
int sendCount=0;//发送计数器

ULONG FMReadCopyNext=0;//下一条即将要读取的流水数据

struct tcp_pcb *echoclient_pcb=NULL;//ccr2017-02-23

NETFLAGS Ethernet={0};//存储以太网状态

/* Private functions ---------------------------------------------------------*/
#if defined(CASE_ETHERNET)

void PingEthernet()
{
    tcp_echoclient_connect(tcp_echoclient_connected);
}

void ETHERNET_SetSendMode()
{
    BYTE y;
	#if 0//jdb2018-09-03
    // 设置发送销售收据的发送参数
    //mClearScreen();
    y = WaitForYesNo(SENDLOGAUTO,0,0,BIT(ApplVar.DaysMust,BIT7));
    if (y!=0)
    {
        if (y=='Y')
            SETBIT(ApplVar.DaysMust,BIT7);//为自动发送流水，即结帐后立即发送流水
        else
            RESETBIT(ApplVar.DaysMust,BIT7);//非自动发送流水，需要手工操作发送流水数据
    }
    ClearEntry();
		#endif
}

void ETHERNETSendECR_FM()
{
	#if 0 //jdb2018-09-03
    if (ApplVar.FiscalHead.ReadP<ApplVar.FiscalHead.MaxP)
    {
        FMReadCopyNext = ApplVar.FiscalHead.ReadP;
        sendCount=0;
        tcp_echoclient_connect(TCP_SendFM_connected);
//        if (ApplVar.CentralLock==SET || ApplVar.CentralLock==X)
//            PRESS_ANY_KEY(1);

    }
		#endif
}

void ETHERNETSendECR_FM_All()
{
	#if 0//jdb2018-09-03
    if (WaitForYesNo(Prompt.Caption[QUERENF],0,1,0)=='Y')
    {
        ApplVar.FiscalHead.ReadP = FISDATAADDR;
        ETHERNETSendECR_FM();
    }
		#endif
}

void ETHERNET_DownloadPLU()
{
}
void ETHERNET_DownloadDEPT()
{
}
void ETHERNET_DownloadCLERK()
{
}
void ETHERNET_DownloadHEAD()
{
}
void ETHERNET_DownloadPTRAIL()
{
}
void ETHERNET_DownloadALL()
{
}
void ETHERNET_Restart()
{
}

/**
  * 当服务器发送数据给Client是,此函数收取服务器发送来的数据
  * @brief tcp_receiv callback
  * @param arg: argument to be passed to receive callback
  * @param tpcb: tcp connection control block
  * @param err: receive error code
  * @retval err_t: retuned error
  */
static err_t TCP_SendFM_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    struct echoclient *es;
    err_t ret_err;
    short size;

    BYTE OK_Send = false;
	
	#if 0//jdb2018-09-03


    XPRINTF(("\r\n TCP_SendFM_recv\r\n"));

    LWIP_ASSERT("arg != NULL",arg != NULL);

    es = (struct echoclient *)arg;

    /* if we receive an empty tcp frame from server => close connection */
    if (p == NULL)
    {
        /* remote host closed connection */
        es->state = ES_CLOSING;
        if (es->p_tx == NULL)
        {
            /* we're done sending, close connection */
            tcp_echoclient_connection_close(tpcb, es);
        } else
        {
            /* send remaining data*/
            tcp_echoclient_send(tpcb, es);
        }
        ret_err = ERR_OK;
    }
    /* else : a non empty frame was received from echo server but for some reason err != ERR_OK */
    else if (err != ERR_OK)
    {
        /* free received pbuf*/
        if (p != NULL)
        {
            pbuf_free(p);
        }
        ret_err = err;
    } else if (es->state == ES_CONNECTED)
    {
        /* Acknowledge data reception */
        tcp_recved(tpcb, p->tot_len);

        XPRINTF((("\r\n TCP_SendFM_recv len =%d\r\n data=%s\r\n", p->tot_len, (char*)p->payload)));

        size=((u8_t *)p->payload)[0];

        pbuf_free(p);


        //ccr2017-02-22要求服务器对收到的数据进行应答时,收到应答后发送下一笔数据>>>>>
        if (size!=ACK)
            ApplVar.ErrorNumber=ERROR_ID(CWXXI104);
        if (es->p_tx==NULL && size==ACK)//必须收到ACK应答后再发送下一批数据
        {
            sendCount++;

            if (FMReadCopyNext!=0)
                ApplVar.FiscalHead.ReadP = FMReadCopyNext;//ccr2015-08-24指向下一条要读取和发送的流水

            if (ApplVar.FiscalHead.ReadP<ApplVar.FiscalHead.MaxP)
            {
                size = Bios_FM_Read(&ApplVar.FiscalBuff.FiscalData,ApplVar.FiscalHead.ReadP,FISRECORDLEN);

                if (!size)
                    size=-1;//标示读数据出错
                else if (CWORD(ApplVar.FiscalBuff.FiscalData[0])==0xffff)
                    size=0;//标示数据读完
            } else
                size = 0;//标示无Fiscal数据

            FMReadCopyNext = ApplVar.FiscalHead.ReadP+FISRECORDLEN;//ccr2015-08-24保存下一条将要读取的流水数据
            if (size>0)
            {
                memset(SysBuf,' ',SCREENWD);
                OK_Send = ULongtoASC(SysBuf+SCREENWD-1,sendCount);
                SysBuf[SCREENWD]=0;
                if (ApplVar.CentralLock==SET)
                    DispStrXY(SysBuf,0,1);//显示已发送数据数目
                else
                {
                    SysBuf[0] = OK_Send;
                    DispStrXY(SysBuf+1,1,1);
                }
                size=FISRECORDLEN;
                PCBuffer[0]='T';
                PCBuffer[1]=FISRECORDLEN;
                memcpy(PCBuffer+2,(BYTE *)&ApplVar.FiscalBuff.FiscalData,FISRECORDLEN);
                /* allocate pbuf */
                es->p_tx = pbuf_alloc(PBUF_TRANSPORT, FISRECORDLEN+2 , PBUF_POOL);
                if (es->p_tx)
                {
                    OK_Send=true;
                    FMReadCopyNext = ApplVar.FiscalHead.ReadP+FISRECORDLEN;//ccr2015-08-24保存当前读取的流水数据地址

#if defined(FOR_DEBUG)
                    xputs("TCP_SendFM_recv>Send\n");
#endif
                    /* copy data to pbuf */
                    pbuf_take(es->p_tx, (char*)PCBuffer, FISRECORDLEN+2);

                    /* send data */
                    tcp_echoclient_send(tpcb,es);
                }
                else
                    tcp_echoclient_connection_close(tpcb, es);
            } else
            {
                OK_Send=true;
                if (ApplVar.CentralLock==SET)
                    DispStrXY(GPRSMess[G_FASONGCHENGGONG].str,0,1);
                else
                {
                    //PutsO(GPRSMess[G_FASONGCHENGGONG].str);//RG-DISPLAY
                    DispStrXY(SysBuf+1,1,1);
                }
                tcp_echoclient_connection_close(tpcb, es);
            }
        } else  //ccr2017-02-22<<<<<<<<<<<<<<<<
            tcp_echoclient_connection_close(tpcb, es);

        if (!OK_Send)//发送失败
        {
            if (ApplVar.CentralLock==SET || ApplVar.CentralLock==X)
                DispStrXY(GPRSMess[G_FASONGSHIBAI].str,0,1);
            else
            {
                CheckError(ERROR_ID(CWXXI103));
                //PutsO(GPRSMess[G_FASONGSHIBAI].str);
            }
        }
        ret_err = ERR_OK;
    }
    /* data received when connection already closed */
    else
    {
        /* Acknowledge data reception */
        tcp_recved(tpcb, p->tot_len);

        /* free pbuf and do nothing */
        pbuf_free(p);
        ret_err = ERR_OK;
    }
		#endif
    return ret_err;
}


/**
 * 将FiscalMemory中的数据(FISCALRECORD)发送给服务器
 * 发送税控数据的命令字为'T'
 * @author EutronSoftware (2017-02-20)
  * @brief Function called when TCP connection established
  * @param tpcb: pointer on the connection contol block
  * @param err: when connection correctly established err should be ERR_OK
  * @retval err_t: returned error
  */

static err_t TCP_SendFM_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{

    struct echoclient *es = NULL;
    struct TimeDate sDate;
	
		#if 0//jdb2018-09-03

    if (err == ERR_OK)
    {
        Ethernet.Connected=true;//ccr2017-02-23

        /* allocate structure es to maintain tcp connection informations */
        es = (struct echoclient *)mem_malloc(sizeof(struct echoclient));

        if (es != NULL)
        {
            es->state = ES_CONNECTED;
            es->pcb = tpcb;

#if defined(FOR_DEBUG)
            xputs("TCP_SendFM_connected>>>\n");  //ccr2017-02-20 testonly
#endif

            if (ApplVar.CentralLock==SET)
                DispStrXY(GPRSMess[G_SHUJUFASONGZHONG].str,0,1);
            else
                PutsO(GPRSMess[G_SHUJUFASONGZHONG].str);//RG-DISPLAY

            GetDateTime(&sDate);
            ApplVar.FiscalBuff.Fis_Register.FDate = EncordBCDDate((sDate.year & 0x00ff),sDate.month,sDate.day);
            ApplVar.FiscalBuff.Fis_Register.FLocation=LOCATION;
            ApplVar.FiscalBuff.Fis_Register.FRegister=REGISTER;
            ApplVar.FiscalBuff.Fis_Register.FTime[0] = sDate.hour;
            ApplVar.FiscalBuff.Fis_Register.FTime[1] = sDate.min;
            ApplVar.FiscalBuff.Fis_Register.FTime[2] = sDate.sec;

            ApplVar.FiscalBuff.Fis_Register.FCRC = 0;
            ApplVar.FiscalBuff.Fis_Register.FunN = FISREGISTER;

            //首先送出税控数据的收款机编号日志
            PCBuffer[0]='T';
            PCBuffer[1]=FISRECORDLEN;
            memcpy(PCBuffer+2,(BYTE *)&ApplVar.FiscalBuff.Fis_Register,FISRECORDLEN);

            /* allocate pbuf */
            es->p_tx = pbuf_alloc(PBUF_TRANSPORT, FISRECORDLEN+2 , PBUF_POOL);

            if (es->p_tx)
            {
                /* copy data to pbuf */
                pbuf_take(es->p_tx, (char*)PCBuffer, FISRECORDLEN+2);

                /* pass newly allocated es structure as argument to tpcb */
                tcp_arg(tpcb, es);

                /* initialize LwIP tcp_recv callback function */
                /* 当接收到数据时，本函数设置的回调函数将被调用。*/
                /*  如果传递给回调函数一个NULL pbuf则说明远程主机关闭了这个连接。*/
                /* 如果函数正常运行并且回调函数返回ERR_OK，则必须释放这个pbuf， */
                /* 如果其它情况，必须保存这个pbuf，这样才能让lwIP内核保存它以供应用程序检查并恢复错误。 */
                tcp_recv(tpcb, TCP_SendFM_recv);

                /* initialize LwIP tcp_sent callback function */
                /* 当远程主机成功接收（也就是应答信号）到数据时，该函数指定的回调函数被调用。*/
                /* 传送给回调函数的"len"参数给出了上一次已经被确认的发送的最大字节数。*/
                tcp_sent(tpcb, tcp_echoclient_sent);

                /* initialize LwIP tcp_poll callback function */
                tcp_poll(tpcb, tcp_echoclient_poll, 1);

                /* send data */
                tcp_echoclient_send(tpcb,es);

                return ERR_OK;
            }
        } else
        {
#if defined(FOR_DEBUG)
            xputs("tcp_connect ERROR(ERR_MEM)>>>\n");  //ccr2017-02-20 testonly
#endif
            /* close connection */
            tcp_echoclient_connection_close(tpcb, es);

            /* return memory allocation error */
            return ERR_MEM;
        }
    } else
    {
#if defined(FOR_DEBUG)
        xputs("tcp_connect ERROR>>>\n");  //ccr2017-02-20 testonly
#endif

        ApplVar.ErrorNumber=ERROR_ID(CWXXI105);//连接服务器失败
        /* close connection */
        tcp_echoclient_connection_close(tpcb, es);

    }
		#endif
    return err;
}

#endif



/**
 *
 * @brief  Connects to the TCP echo server
 * @author EutronSoftware (2017-02-23)
 * @param connected :连接成功后要运行的功能函数
 * @retval None
 */
void tcp_echoclient_connect(err_t (* connected)(void *arg,struct tcp_pcb *tpcb,err_t err))
{
    struct ip_addr DestIPaddr;
    uint8_t tmp;
	
	#if 0//jdb2018-09-03

    if (Ethernet.XLine)//ccr2017-02-23网线未插好
    {
        ApplVar.ErrorNumber=ERROR_ID(CWXXI109);
        return;// false;
    }
    if (Ethernet.Connected)//ccr2017-02-23一次只允许一个Client运行
    {
        ApplVar.ErrorNumber=ERROR_ID(CWXXI110);
        return;// false;
    }
#if defined(FOR_DEBUG)
    xputs("tcp_echoclient_connect>>>\n");  //ccr2017-02-20 testonly
#endif

    /* create new tcp pcb */
    echoclient_pcb = tcp_new();

    if (echoclient_pcb != NULL)
    {
        IP4_ADDR( &DestIPaddr, DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3 );

        /* connect to destination address/port */

        //tmp = tcp_connect(echoclient_pcb,&DestIPaddr,DEST_PORT,TCP_SendFM_connected);
        tmp = tcp_connect(echoclient_pcb,&DestIPaddr,DEST_PORT,connected);

        XPRINTF(("Return by tcp_connect=0x%x\r\n",tmp));
        return;// true;
    } else
    {
        ApplVar.ErrorNumber=ERROR_ID(CWXXI110);
        /* deallocate the pcb */
        memp_free(MEMP_TCP_PCB, echoclient_pcb);

        XPRINTF(("Can not create tcp pcb...\n"));
        return;// false;
    }
		
		#endif

}

#define PACKLEN	150	//数据包长度
/**
  * @brief Function called when TCP connection established,
  * ccr2017-02-23仅供测试用
  * @param tpcb: pointer on the connection contol block
  * @param err: when connection correctly established err should be ERR_OK
  * @retval err_t: returned error
  */
static err_t tcp_echoclient_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    struct echoclient *es = NULL;
	
	#if 0//jdb2018-09-03

    if (err == ERR_OK)
    {
        Ethernet.Connected=true;//ccr2017-02-23
        /* allocate structure es to maintain tcp connection informations */
        es = (struct echoclient *)mem_malloc(sizeof(struct echoclient));

        if (es != NULL)
        {
            es->state = ES_CONNECTED;
            es->pcb = tpcb;

            sendCount=0;

            memset(PCBuffer,(sendCount & 0x0f)+'A',sizeof(PCBuffer));

            PCBuffer[0]='E';      PCBuffer[1]=PACKLEN;      PCBuffer[2]=sendCount & 0xff;
            sprintf(&PCBuffer[3],"<<<<<%15u",sendCount++);//必须为固定格式

            /* allocate pbuf */
            es->p_tx = pbuf_alloc(PBUF_TRANSPORT, PACKLEN+2 , PBUF_POOL);

            if (es->p_tx)
            {
                /* copy data to pbuf */
                pbuf_take(es->p_tx, (char*)PCBuffer, PACKLEN+2);

                /* pass newly allocated es structure as argument to tpcb */
                tcp_arg(tpcb, es);

                /* initialize LwIP tcp_recv callback function */
                /* 当接收到数据时，本函数设置的回调函数将被调用。*/
                /*  如果传递给回调函数一个NULL pbuf则说明远程主机关闭了这个连接。*/
                /* 如果函数正常运行并且回调函数返回ERR_OK，则必须释放这个pbuf， */
                /* 如果其它情况，必须保存这个pbuf，这样才能让lwIP内核保存它以供应用程序检查并恢复错误。 */
                tcp_recv(tpcb, tcp_echoclient_recv);

                /* initialize LwIP tcp_sent callback function */
                /* 当远程主机成功接收（也就是应答信号）到数据时，该函数指定的回调函数被调用。*/
                /* 传送给回调函数的"len"参数给出了上一次已经被确认的发送的最大字节数。*/
                tcp_sent(tpcb, tcp_echoclient_sent);

                /* initialize LwIP tcp_poll callback function */
                tcp_poll(tpcb, tcp_echoclient_poll, 1);

                /* send data */
                tcp_echoclient_send(tpcb,es);

                return ERR_OK;
            }
        } else
        {
            /* close connection */
            tcp_echoclient_connection_close(tpcb, es);

            /* return memory allocation error */
            return ERR_MEM;
        }
    } else
    {
        ApplVar.ErrorNumber=ERROR_ID(CWXXI105);//连接服务器失败
        /* close connection */
        tcp_echoclient_connection_close(tpcb, es);

    }
		#endif
    return err;
}

/**
  * 当服务器发送数据给Client是,此函数收取服务器发送来的数据
  * ccr2017-02-23仅供测试用
  * @brief tcp_receiv callback
  * @param arg: argument to be passed to receive callback
  * @param tpcb: tcp connection control block
  * @param err: receive error code
  * @retval err_t: retuned error
  */
static err_t tcp_echoclient_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    struct echoclient *es;
    err_t ret_err;

#if defined(FOR_DEBUG)
    USART_printf("\r\n tcp_echoclient_recv\r\n");
#endif
    LWIP_ASSERT("arg != NULL",arg != NULL);

    es = (struct echoclient *)arg;

    /* if we receive an empty tcp frame from server => close connection */
    if (p == NULL)
    {
        /* remote host closed connection */
        es->state = ES_CLOSING;
        if (es->p_tx == NULL)
        {
            /* we're done sending, close connection */
            tcp_echoclient_connection_close(tpcb, es);
        } else
        {
            /* send remaining data*/
            tcp_echoclient_send(tpcb, es);
        }
        ret_err = ERR_OK;
    }
    /* else : a non empty frame was received from echo server but for some reason err != ERR_OK */
    else if (err != ERR_OK)
    {
        /* free received pbuf*/
        if (p != NULL)
        {
            pbuf_free(p);
        }
        ret_err = err;
    } else if (es->state == ES_CONNECTED)
    {
        /* Acknowledge data reception */
        tcp_recved(tpcb, p->tot_len);
#if defined(FOR_DEBUG)
        USART_printf("\r\n tcp_echoclient_recv len =%d\r\n data=%s\r\n", p->tot_len, p->payload);
#endif
        pbuf_free(p);

#if 1
        //ccr2017-02-22要求服务器对收到的数据进行应答时,收到应答后发送下一笔数据>>>>>
        if (es->p_tx==NULL && sendCount<100)//发送下一批数据
        {
            memset(PCBuffer,(sendCount & 0x0f)+'a',sizeof(PCBuffer));

            PCBuffer[0]='E';      PCBuffer[1]=PACKLEN;      PCBuffer[2]=sendCount & 0xff;
            sprintf(&PCBuffer[3],"<<<<<%15u",sendCount++);//必须为固定格式
            /* allocate pbuf */
            es->p_tx = pbuf_alloc(PBUF_TRANSPORT, PACKLEN+2 , PBUF_POOL);
            if (es->p_tx)
            {
#if defined(FOR_DEBUG)
                xputs("tcp_echoclient_recv>Send\n");
#endif
                /* copy data to pbuf */
                pbuf_take(es->p_tx, (char*)PCBuffer, PACKLEN+2);

                /* send data */
                tcp_echoclient_send(tpcb,es);
            }
        } else
            //ccr2017-02-22<<<<<<<<<<<<<<<<
#endif
            tcp_echoclient_connection_close(tpcb, es);

        ret_err = ERR_OK;
    }

    /* data received when connection already closed */
    else
    {
        /* Acknowledge data reception */
        tcp_recved(tpcb, p->tot_len);

        /* free pbuf and do nothing */
        pbuf_free(p);
        ret_err = ERR_OK;
    }
    return ret_err;
}

/**
  * @brief function used to send data
  * @param  tpcb: tcp control block
  * @param  es: pointer on structure of type echoclient containing info on data
  *             to be sent
  * @retval None
  */
static void tcp_echoclient_send(struct tcp_pcb *tpcb, struct echoclient * es)
{
    struct pbuf *ptr;
    err_t wr_err = ERR_OK;
	
	#if 0//jdb2018-*09-03

    while ((wr_err == ERR_OK) &&
           (es->p_tx != NULL) &&
           (es->p_tx->len <= tcp_sndbuf(tpcb)))
    {

        /* get pointer on pbuf from es structure */
        ptr = es->p_tx;

        /* enqueue data for transmission */
        wr_err = tcp_write(tpcb, ptr->payload, ptr->len, 1);

        if (wr_err == ERR_OK)
        {
            /* continue with next pbuf in chain (if any) */
            es->p_tx = ptr->next;

            if (es->p_tx != NULL)
            {
                /* increment reference count for es->p */
                pbuf_ref(es->p_tx);
            }

            /* free pbuf: will free pbufs up to es->p (because es->p has a reference count > 0) */
            pbuf_free(ptr);
        } else if (wr_err == ERR_MEM)
        {
            ApplVar.ErrorNumber=ERROR_ID(CWXXI15);//存储溢出
            /* we are low on memory, try later, defer to poll */
            es->p_tx = ptr;
        } else
        {
            ApplVar.ErrorNumber=ERROR_ID(CWXXI103);//发送数据出错
            /* other problem ?? */
        }
    }
		
		#endif
}

/**
  * @brief  This function implements the tcp_poll callback function
  * @param  arg: pointer on argument passed to callback
  * @param  tpcb: tcp connection control block
  * @retval err_t: error code
  */
static err_t tcp_echoclient_poll(void *arg, struct tcp_pcb *tpcb)
{
    err_t ret_err;
    struct echoclient *es;

    es = (struct echoclient*)arg;
    if (es != NULL)
    {
        if (es->p_tx != NULL)
        {
            /* there is a remaining pbuf (chain) , try to send data */
            tcp_echoclient_send(tpcb, es);
        } else
        {
            /* no remaining pbuf (chain)  */
            if (es->state == ES_CLOSING)
            {
                /* close tcp connection */
                tcp_echoclient_connection_close(tpcb, es);
            }
        }
        ret_err = ERR_OK;
    } else
    {
        /* nothing to be done */
        tcp_abort(tpcb);
        ret_err = ERR_ABRT;
    }
    return ret_err;
}

/**
  * 当Server收到数据后,会调用此回调函数
  *
  * @brief  This function implements the tcp_sent LwIP callback (called when ACK
  *         is received from remote host for sent data)
  * @param  arg: pointer on argument passed to callback
  * @param  tcp_pcb: tcp connection control block
  * @param  len: length of data sent
  * @retval err_t: returned error code
  */
static err_t tcp_echoclient_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    struct echoclient *es;

    LWIP_UNUSED_ARG(len);

#if defined(FOR_DEBUG)
    xputs("tcp_echoclient_sent...\n");
#endif
    es = (struct echoclient *)arg;

    if (es->p_tx != NULL)
    {
        /* still got pbufs to send */
        tcp_echoclient_send(tpcb, es);
    }
#if 0
    //ccr2017-02-22服务器不对收到的数据进行应答时,确认服务器收到数据后发送下一笔数据>>>>>
    else//发送下一批数据
    {
        memset(data,(tstID & 0x0f)+'a',sizeof(data));

        PCBuffer[0]='H';      PCBuffer[1]=50;      PCBuffer[2]=NETIDLOG;
        sprintf(&PCBuffer[3],"<<<<<%15u",sendCount++);//必须为固定格式
        /* allocate pbuf */
        es->p_tx = pbuf_alloc(PBUF_TRANSPORT, PCBuffer[1]+2 , PBUF_POOL);
        if (es->p_tx)
        {
            xputs("tcp_echoclient_sent>New Send\n");

            /* copy data to pbuf */
            pbuf_take(es->p_tx, (char*)data, PCBuffer[1]+2);

            /* send data */
            tcp_echoclient_send(tpcb,es);
        }
    }
    //ccr2017-02-22<<<<<<<<<<<<<<<<
#endif
    return ERR_OK;
}

/**
  * @brief This function is used to close the tcp connection with server
  * @param tpcb: tcp connection control block
  * @param es: pointer on echoclient structure
  * @retval None
  */
static void tcp_echoclient_connection_close(struct tcp_pcb *tpcb, struct echoclient * es )
{
    /* remove callbacks */
    tcp_recv(tpcb, NULL);
    tcp_sent(tpcb, NULL);
    tcp_poll(tpcb, NULL,0);

    if (es != NULL)
    {
        mem_free(es);
    }

    /* close tcp connection */
    tcp_close(tpcb);
    Ethernet.Connected=false;//ccr2017-02-23
}

#endif /* LWIP_TCP */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
#endif