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

/* Includes ------------------------------------------------------------------*/
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "Ethernet_app.h"
#include "memp.h"

#include <stdio.h>
#include <string.h>

#include "netconf.h" //ccr2017-02-17
#include "king.h"  //ccr2017-02-17
#include "exthead.h" //ccr2017-02-17
#include "exth.h" //ccr2017-02-17
#include "interface.h"//ccr2017-02-17

//#define MAXLOGPACKAGE      //ccr2015-08-25����ˮ�������������ʽ����

#if LWIP_TCP
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

u8_t  recev_buf[50];
__IO uint32_t message_count=0;

u8_t   data[200];
int received=0;//testonly

struct tcp_pcb *echoclient_pcb;


/* ECHO protocol states */
enum echoclient_states
{
  ES_NOT_CONNECTED = 0,
  ES_CONNECTED,
  ES_RECEIVED,
  ES_CLOSING,
};


/* structure to be passed as argument to the tcp callbacks */
struct echoclient
{
  enum echoclient_states state; /* connection status */
  struct tcp_pcb *pcb;          /* pointer on the current tcp_pcb */
  struct pbuf *p_tx;            /* pointer on pbuf to be transmitted */
};


/* Private function prototypes -----------------------------------------------*/
static err_t tcp_echoclient_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void tcp_echoclient_connection_close(struct tcp_pcb *tpcb, struct echoclient * es);
static err_t tcp_echoclient_poll(void *arg, struct tcp_pcb *tpcb);
static err_t tcp_echoclient_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static void tcp_echoclient_send(struct tcp_pcb *tpcb, struct echoclient * es);
static err_t tcp_echoclient_connected(void *arg, struct tcp_pcb *tpcb, err_t err);

/* Private functions ---------------------------------------------------------*/



/**
 * ��FiscalMemory�е�����(FISCALRECORD)���͸�������
 * ����˰�����ݵ�������Ϊ'T'
 * @author EutronSoftware (2017-02-20)
  * @brief Function called when TCP connection established
  * @param tpcb: pointer on the connection contol block
  * @param err: when connection correctly established err should be ERR_OK
  * @retval err_t: returned error
  */
static err_t TCP_SendFM_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
//static void TCP_SendECR_FM(struct tcp_pcb *tpcb, struct echoclient * es)
{
    struct TimeDate sDate;
    short i,size;
    ULONG sendCount=0;//���ͼ�����
    ULONG logReadCopyNext=0;//��һ������Ҫ��ȡ����ˮ����
    ULONG logReadCopyLast=0;//��ǰ��ȡ����ˮ����
    err_t OK_Send = ERR_OK;

#if defined(MAXLOGPACKAGE)
    ULONG logSize;//����ָ��������͵���־���ݰ���С
    int  sendFlag;//���ͼ�����
#endif

    if (ApplVar.FiscalHead.ReadP==ApplVar.FiscalHead.MaxP)
    {
        /* close tcp connection */
        tcp_close(tpcb);
        return ERR_OK;
    }
    if (1)//ccr2017-02-20 TESTBIT(GPRS_Ready,GPRS_OK_BIT) && GetSystemTimer()>GPRS_Wait)//ccr2015-03-11
    {
        ////mClearScreen();
        //DispStrXY(GPRSMess[G_CONNECTING].str,0,0);
        if (1)//ccr2017-02-20 GPRSStartGprs())
        {
            if (ApplVar.CentralLock==SET)
                DispStrXY(GPRSMess[G_SHUJUFASONGZHONG].str,0,1);
            else
                PutsO(GPRSMess[G_SHUJUFASONGZHONG].str);//RG-DISPLAY

            GetTimeDate(&sDate);
            ApplVar.FiscalBuff.Fis_Register.FDate = EncordDate((sDate.year & 0x00ff),sDate.month,sDate.day);
            ApplVar.FiscalBuff.Fis_Register.FLocation=LOCATION;
            ApplVar.FiscalBuff.Fis_Register.FRegister=REGISTER;
            ApplVar.FiscalBuff.Fis_Register.FTime[0] = sDate.hour;
            ApplVar.FiscalBuff.Fis_Register.FTime[1] = sDate.min;
            ApplVar.FiscalBuff.Fis_Register.FTime[2] = sDate.sec;

            ApplVar.FiscalBuff.Fis_Register.FCRC = 0;
            ApplVar.FiscalBuff.Fis_Register.FunN = FISREGISTER;

            size = FRECORDLEN;//�����ͳ�˰�����ݵ��տ�������־
            PCBuffer[0]='T';
            PCBuffer[1]=size & 0xff;
            memcpy(PCBuffer+2,(BYTE *)&ApplVar.FiscalBuff.Fis_Register,FRECORDLEN);
#if defined(MAXLOGPACKAGE)
            logSize = size+2;//ccr2015-08-26

#if (PCBUFMAX<1024)
#error "PCBUFMAX<1024"
#endif
            //����PCBuffer,һ�η��Ͳ�����1024�ֽڵ���ˮ����
            while (size>0)// && ApplVar.FiscalHead.ReadP<ApplVar.FiscalHead.MaxP)
            {//�ڴ�ѭ����,���ȫ����ˮ���ݵķ���
                sendFlag = 0;//������ʾÿ������е���ˮ����
                if (logReadCopyNext!=0)
                    ApplVar.FiscalHead.ReadP = logReadCopyNext;//ccr2015-08-24ָ����һ��Ҫ��ȡ�ͷ��͵���ˮ
                logReadCopyLast = ApplVar.FiscalHead.ReadP;//ccr2015-08-24���浱ǰ��ȡ����ˮ���ݵ�ַ
                logReadCopyNext = logReadCopyLast;//ccr2016-04-18
                while (size>0)
                {//�ڴ�ѭ����,���һ������ͷ���
                    if (ApplVar.FiscalHead.ReadP<ApplVar.FiscalHead.MaxP)
                    {
                        size = Bios_FM_Read(&ApplVar.FiscalBuff.FiscalData,ApplVar.FiscalHead.ReadP,FRECORDLEN);
                        if (!size)
                             size=-1;//��ʾ�����ݳ���
                        else if (CWORD(ApplVar.FiscalBuff.FiscalData[0])==0xffff)
                             size=0;//��ʾ���ݶ���
                    }
                    else
                        size = 0;//��ʾ��Fiscal����

                    if (size>0 && (FRECORDLEN+logSize)<=1020)
                    {
                        //�����ݴ���PCBuffer,��ɴ��
                        PCBuffer[logSize++]='T';
                        PCBuffer[logSize++]=FRECORDLEN;
                        memcpy(PCBuffer+logSize,(BYTE *)&ApplVar.FiscalBuff.FiscalData,FRECORDLEN);
                        logSize+=FRECORDLEN;
                        sendFlag++;
                        ApplVar.FiscalHead.ReadP+=FRECORDLEN;//ָ����һ����¼
                    }
                    else if (size>=0)// (FRECORDLEN+logSize)>1020)
                    {
                        if (sendFlag)
                        {//��Ҫ���͵�����,�����ݷ��ͳ�ȥ
                            memset(SysBuf,' ',SCREENWD);
                            OK_Send = ULongtoASC(SysBuf+SCREENWD-1,sendFlag);
                            SysBuf[SCREENWD]=0;
                            if (ApplVar.CentralLock==SET)
                                DispStrXY(SysBuf,0,1);//��ʾ�ѷ���������Ŀ
                            else
                            {
                                SysBuf[0] = OK_Send;
                                DispStrXY(SysBuf+1,1,1);
                            }

                            logReadCopyLast = logReadCopyNext;
                            logReadCopyNext = ApplVar.FiscalHead.ReadP;
                            ApplVar.FiscalHead.ReadP = logReadCopyLast;//���ͳ���ʱ,�´δӴ����·���
                            OK_Send = tcp_write(tpcb,PCBuffer,logSize,1);//������ˮ����
                            if  (OK_Send==ERR_OK)
                                OK_Send=tcp_output(tpcb);//�������������ͳ�ȥ

                            if (OK_Send!=ERR_OK
#if defined(GPRSWAITACK)
                           !!!   || !GPRSCheckACK(ACK)//�ȴ�������Ӧ��
#endif
                                )
                            {//����ʧ��ʱ
                                OK_Send = false;
                                size = 0;//��������.
                            }
                            else
                            {//���ͳɹ�
                                ApplVar.FiscalHead.ReadP = logReadCopyNext;//���ͳɹ�,�´δӴ˷���
                                if (sendCount==0) sendCount=1;//���տ��NETID����ˮ
                                sendCount+=sendFlag;
                                OK_Send = true;
                            }
                            logSize = 0;
                        }
                        break;
                    }
                }
            }

            memset(SysBuf,' ',SCREENWD);
            SysBuf[0] = ULongtoASC(SysBuf+SCREENWD-1,sendCount);
            SysBuf[SCREENWD]=0;
            if (OK_Send)
                i = G_FASONGCHENGGONG;
            else
                i = G_FASONGSHIBAI;
            if (ApplVar.CentralLock==SET)
            {
                SysBuf[0]=' ';
                //DispStrXY(SysBuf,0,2);//��ʾ�ѷ���������Ŀ
                DispStrXY(GPRSMess[i].str,0,1);
            }
            else
            {
                PutsO(GPRSMess[i].str);//RG-DISPLAY
                //DispStrXY(SysBuf+1,1,1);
            }
#else//����������ˮ���ݷ�ʽ>>>>>>>>>>>>
            do
            {
                //����������ˮ����
                OK_Send = tcp_write(tpcb,PCBuffer,size+2,0);
                //if  (OK_Send==ERR_OK)
                    OK_Send=tcp_output(tpcb);//�������������ͳ�ȥ

                if (1)//testonlyOK_Send!=ERR_OK)//������ˮ����
                {
                    OK_Send = false;
                    break;
                }
                sendCount++;

                if (logReadCopyNext!=0)
                    ApplVar.FiscalHead.ReadP = logReadCopyNext;//ccr2015-08-24ָ����һ��Ҫ��ȡ�ͷ��͵���ˮ

                logReadCopyLast = ApplVar.FiscalHead.ReadP;//ccr2015-08-24���浱ǰ��ȡ����ˮ���ݵ�ַ


                if (ApplVar.FiscalHead.ReadP<ApplVar.FiscalHead.MaxP)
                {
                    //testonly size = Bios_FM_Read(&ApplVar.FiscalBuff.FiscalData,ApplVar.FiscalHead.ReadP,FRECORDLEN);
                    memset(&ApplVar.FiscalBuff.FiscalData,(sendCount & 0x0f)+'A',FRECORDLEN);//testonly
                    if (sendCount>100) size=0;//testonly
                    if (!size)
                         size=-1;//��ʾ�����ݳ���
                    else if (CWORD(ApplVar.FiscalBuff.FiscalData[0])==0xffff)
                         size=0;//��ʾ���ݶ���
                }
                else
                    size = 0;//��ʾ��Fiscal����

                //testonly logReadCopyNext = ApplVar.FiscalHead.ReadP+FRECORDLEN;//ccr2015-08-24������һ����Ҫ��ȡ����ˮ����
                if (size>0)
                {
                    memset(SysBuf,' ',SCREENWD);
                    OK_Send = ULongtoASC(SysBuf+SCREENWD-1,sendCount);
                    SysBuf[SCREENWD]=0;
                    if (ApplVar.CentralLock==SET)
                        DispStrXY(SysBuf,0,1);//��ʾ�ѷ���������Ŀ
                    else
                    {
                        SysBuf[0] = OK_Send;
                        DispStrXY(SysBuf+1,1,1);
                    }
                    size=FRECORDLEN;
                    ApplVar.FiscalHead.ReadP = logReadCopyLast;//ccr2015-08-24�ָ�����ȡ����ˮ���ݵ�ַ,�Է����������ȳ���
                    PCBuffer[0]='T';
                    PCBuffer[1]=FRECORDLEN;
                    memcpy(PCBuffer+2,(BYTE *)&ApplVar.FiscalBuff.FiscalData,FRECORDLEN);

                }
                OK_Send = true;

            } while (size>0);
#if defined(GPRSWAITACK)
            if (sendCount>0)//��ʾ�ڷ�������ˮ
           !!!   GPRSCheckACK(ACK);//�ȴ�������Ӧ��
#endif
#endif//defined(MAXLOGPACKAGE)

            if (!OK_Send)//����ʧ��
            {
                if (ApplVar.CentralLock==SET || ApplVar.CentralLock==X)
                    DispStrXY(GPRSMess[G_FASONGSHIBAI].str,0,1);
                else
                {
                    CheckError(ERROR_ID(CWXXI103));
                    //PutsO(GPRSMess[G_FASONGSHIBAI].str);
                }
            }
            else if (size==0)//��ˮ���ݷ������
            {
#if defined(CASE_INDONESIA)
                ApplVar.LastDaysLog = GetDaysFrom1900(Now.year,Now.month,Now.day);
#endif
#if !defined(MAXLOGPACKAGE)
                if (ApplVar.CentralLock==SET)
                    DispStrXY(GPRSMess[G_FASONGCHENGGONG].str,0,1);
                else
                {
                    //PutsO(GPRSMess[G_FASONGCHENGGONG].str);//RG-DISPLAY
                    DispStrXY(SysBuf+1,1,1);
                }
#endif
            }
            else if (size==-1)//��ˮ���ݹ��ܺų���
            {
                ApplVar.ErrorNumber=ERROR_ID(CWXXI48);
            }
            /* close tcp connection */
            tcp_close(tpcb);
        }
    }
    else
    {
        if (ApplVar.CentralLock!=SET)
        {
            ApplVar.ErrorNumber=ERROR_ID(CWXXI108);
        }
        PutsO(GPRSMess[G_WAITFOR].str);//RG-DISPLAY
    }
    if (ApplVar.CentralLock==SET || ApplVar.CentralLock==X)
        PRESS_ANY_KEY(1);
}


/**
  * @brief  Connects to the TCP echo server
  * @param  None
  * @retval None
  */

void tcp_echoclient_connect(void)
{
  struct ip_addr DestIPaddr;
  uint8_t tmp;

  /* create new tcp pcb */
  echoclient_pcb = tcp_new();

  xputs("tcp_new return...\n");  //ccr2017-02-20 testonly

  if (echoclient_pcb != NULL)
  {
    xputs("Client_pcb is OK\n");  //ccr2017-02-20 testonly

    IP4_ADDR( &DestIPaddr, DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3 );

    /* connect to destination address/port */

    //tmp = tcp_connect(echoclient_pcb,&DestIPaddr,DEST_PORT,TCP_SendFM_connected);
    tmp = tcp_connect(echoclient_pcb,&DestIPaddr,DEST_PORT,tcp_echoclient_connected);

	xprintf("tcp_connect=0x%x\r\n",tmp);
  }
  else
  {
    /* deallocate the pcb */
    memp_free(MEMP_TCP_PCB, echoclient_pcb);

    xputs("can not create tcp pcb...\n");

  }

}

#define PACKLEN	150	//���ݰ�����
/**
  * @brief Function called when TCP connection established
  * @param tpcb: pointer on the connection contol block
  * @param err: when connection correctly established err should be ERR_OK
  * @retval err_t: returned error
  */
static err_t tcp_echoclient_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
  struct echoclient *es = NULL;

  if (err == ERR_OK)
  {
    /* allocate structure es to maintain tcp connection informations */
    es = (struct echoclient *)mem_malloc(sizeof(struct echoclient));

    if (es != NULL)
    {
      es->state = ES_CONNECTED;
      es->pcb = tpcb;


      memset(data,(message_count & 0x0f)+'A',sizeof(data));

      data[0]='E';      data[1]=PACKLEN;      data[2]=message_count & 0xff;
      sprintf(&data[3],"<<<<<%15u",message_count++);//����Ϊ�̶���ʽ

      /* allocate pbuf */
      es->p_tx = pbuf_alloc(PBUF_TRANSPORT, data[1]+2 , PBUF_POOL);

      if (es->p_tx)
      {
        /* copy data to pbuf */
        pbuf_take(es->p_tx, (char*)data, data[1]+2);

        /* pass newly allocated es structure as argument to tpcb */
        tcp_arg(tpcb, es);

        /* initialize LwIP tcp_recv callback function */
        /* �����յ�����ʱ�����������õĻص������������á�*/
        /*  ������ݸ��ص�����һ��NULL pbuf��˵��Զ�������ر���������ӡ�*/
        /* ��������������в��һص���������ERR_OK��������ͷ����pbuf�� */
        /* ���������������뱣�����pbuf������������lwIP�ں˱������Թ�Ӧ�ó����鲢�ָ����� */
        tcp_recv(tpcb, tcp_echoclient_recv);

        /* initialize LwIP tcp_sent callback function */
        /* ��Զ�������ɹ����գ�Ҳ����Ӧ���źţ�������ʱ���ú���ָ���Ļص����������á�*/
        /* ���͸��ص�������"len"������������һ���Ѿ���ȷ�ϵķ��͵�����ֽ�����*/
        tcp_sent(tpcb, tcp_echoclient_sent);

        /* initialize LwIP tcp_poll callback function */
        tcp_poll(tpcb, tcp_echoclient_poll, 1);

        /* send data */
        tcp_echoclient_send(tpcb,es);

        return ERR_OK;
      }
    }
    else
    {
      /* close connection */
      tcp_echoclient_connection_close(tpcb, es);

      /* return memory allocation error */
      return ERR_MEM;
    }
  }
  else
  {
    /* close connection */
    tcp_echoclient_connection_close(tpcb, es);

  }
  return err;
}

/**
  * ���������������ݸ�Client��,�˺�����ȡ������������������
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

  USART_printf("\r\n tcp_echoclient_recv\r\n");

  LWIP_ASSERT("arg != NULL",arg != NULL);

  es = (struct echoclient *)arg;

  /* if we receive an empty tcp frame from server => close connection */
  if (p == NULL)
  {
    /* remote host closed connection */
    es->state = ES_CLOSING;
    if(es->p_tx == NULL)
    {
       /* we're done sending, close connection */
       tcp_echoclient_connection_close(tpcb, es);
    }
    else
    {
      /* send remaining data*/
      tcp_echoclient_send(tpcb, es);
    }
    ret_err = ERR_OK;
  }
  /* else : a non empty frame was received from echo server but for some reason err != ERR_OK */
  else if(err != ERR_OK)
  {
    /* free received pbuf*/
    if (p != NULL)
    {
      pbuf_free(p);
    }
    ret_err = err;
  }
  else if(es->state == ES_CONNECTED)
  {
    /* Acknowledge data reception */
    tcp_recved(tpcb, p->tot_len);
    USART_printf("\r\n tcp_echoclient_recv len =%d\r\n data=%s\r\n", p->tot_len, p->payload);

    pbuf_free(p);

#if 1
  //ccr2017-02-22Ҫ����������յ������ݽ���Ӧ��ʱ,�յ�Ӧ�������һ������>>>>>
  if (es->p_tx==NULL)//������һ������
  {
      memset(data,(message_count & 0x0f)+'a',sizeof(data));

      data[0]='E';      data[1]=PACKLEN;      data[2]=message_count & 0xff;
      sprintf(&data[3],"<<<<<%15u",message_count++);//����Ϊ�̶���ʽ
      /* allocate pbuf */
      es->p_tx = pbuf_alloc(PBUF_TRANSPORT, data[1]+2 , PBUF_POOL);
      if (es->p_tx)
      {
          xputs("tcp_echoclient_recv>Send\n");

          /* copy data to pbuf */
          pbuf_take(es->p_tx, (char*)data, data[1]+2);

          /* send data */
          tcp_echoclient_send(tpcb,es);
      }
  }
  else
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

      if(es->p_tx != NULL)
      {
        /* increment reference count for es->p */
        pbuf_ref(es->p_tx);
      }

      /* free pbuf: will free pbufs up to es->p (because es->p has a reference count > 0) */
      pbuf_free(ptr);
   }
   else if(wr_err == ERR_MEM)
   {
      /* we are low on memory, try later, defer to poll */
     es->p_tx = ptr;
   }
   else
   {
     /* other problem ?? */
   }
  }
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
    }
    else
    {
      /* no remaining pbuf (chain)  */
      if(es->state == ES_CLOSING)
      {
        /* close tcp connection */
        tcp_echoclient_connection_close(tpcb, es);
      }
    }
    ret_err = ERR_OK;
  }
  else
  {
    /* nothing to be done */
    tcp_abort(tpcb);
    ret_err = ERR_ABRT;
  }
  return ret_err;
}

/**
  * ��Server�յ����ݺ�,����ô˻ص�����
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

  xputs("tcp_echoclient_sent...\n");

  es = (struct echoclient *)arg;

  if(es->p_tx != NULL)
  {
    /* still got pbufs to send */
    tcp_echoclient_send(tpcb, es);
  }
#if 0
  //ccr2017-02-22�����������յ������ݽ���Ӧ��ʱ,ȷ�Ϸ������յ����ݺ�����һ������>>>>>
  else//������һ������
  {
      memset(data,(tstID & 0x0f)+'a',sizeof(data));

      data[0]='H';      data[1]=50;      data[2]=NETIDLOG;
      sprintf(&data[3],"<<<<<%15u",message_count++);//����Ϊ�̶���ʽ
      /* allocate pbuf */
      es->p_tx = pbuf_alloc(PBUF_TRANSPORT, data[1]+2 , PBUF_POOL);
      if (es->p_tx)
      {
          xputs("tcp_echoclient_sent>New Send\n");

          /* copy data to pbuf */
          pbuf_take(es->p_tx, (char*)data, data[1]+2);

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

}

#endif /* LWIP_TCP */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
