/**
 * �˳����lwipTcpClient.c�ݱ����
 */
#include "lwip/tcp.h"
#include "lwip/err.h"
#include "memp.h"

#include <string.h>
#include <stdio.h>

#include "king.h"  //ccr2017-02-17
#include "exthead.h" //ccr2017-02-17
#include "exth.h" //ccr2017-02-17
#include "interface.h"//ccr2017-02-17
#include "tcp_client.h"//���õ�"king.h"֮��
#include "message.h"

#include "AES256.h"
#include "httpClient.h"
#include "Ethernet_app.h"



#if HTTP_ONLY1
struct HTTP_Client_app_arg Http_arg={CLIENT_CLOSE,0};//ccr2017-10-17���þ�̬����,ֻ����һ��ͨѶ
#endif
                                            //0123456789012345678901234567890123456789
char DateTimeStrFromHttp[HTTPDATELEN];//ccr2018-08-13,"Date: Thu, 28 Dec 2017 05:29:59 GMT"

void HTTP_Client_errf(void *arg, err_t err)
{
    struct HTTP_Client_app_arg* pro_arg = (struct HTTP_Client_app_arg*)arg;

    pro_arg->app_state = CLIENT_ERROR;
#if (1)//ccr2017-10-16>>>>>>>>
    XPRINTF((">tcp client err:%s\n",lwip_strerr(err)));
#else//ccr2017-10-16<<<<<<<<<<
    xprintf("tcp client err:");
    switch (err)
    {
    case ERR_MEM: xprintf("Out of memory error\n"); break;
    case ERR_BUF: xprintf("Buffer error\n"); break;
    case ERR_TIMEOUT: xprintf("Timeout\n"); break;
    case ERR_RTE: xprintf("Routing problem\n"); break;
    case ERR_ABRT: xprintf("Connection aborted\n"); break;
    case ERR_RST: xprintf("Connection reset\n"); break;
    case ERR_CLSD: xprintf("Connection closed\n"); break;
    case ERR_CONN: xprintf("Not connected\n"); break;
    case ERR_VAL: xprintf("Illegal value\n"); break;
    case ERR_ARG: xprintf("Illegal argument\n"); break;
    case ERR_USE: xprintf("Address in use\n"); break;
    case ERR_IF: xprintf("Low-level netif error\n"); break;
    case ERR_ISCONN: xprintf("Already connected\n"); break;
    case ERR_INPROGRESS: xprintf("Operation in progress\n"); break;
    default: xprintf("Unknown error\n");
    }
#endif
}
/***************************************************************************************************
 * ���ܴӷ��������ص�HTTPҳ������,�������ݴ洢����ʹ��PCBuffer,
 * �������HTTP���ص����ݲ�Ӧ�ó���PCBuffer�Ŀռ��С,���򽫻�ȡ����Server����������
 * ��˾����ܴ�ض���PCBuffer�ռ�
 *
 * @author EutronSoftware (2018-08-13)
 *
 * @param arg
 * @param pcb
 * @param p
 * @param err
 *
 * @return err_t
 ***************************************************************************************************/
err_t HTTP_Client_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    struct pbuf *q;
    struct HTTP_Client_app_arg *appcmd = (struct HTTP_Client_app_arg *)arg;
    uint16_t i = 0;
    uint8_t ip[4];
    uint8_t* str;


    XPRINTF((">tcp client recieve\n"));

    /* We perform here any necessary processing on the pbuf */
    if (p != NULL)
    {//ccr2017-10-17��ȡ���������ص�����
        XPRINTF((">Length recieve:%d\n",p->tot_len));
        /* We call this function to tell the LwIp that we have processed the data */
        /* This lets the stack advertise a larger window, so more data can be received*/
        tcp_recved(pcb, p->tot_len);

        /* Check the name if NULL, no data passed, return withh illegal argument error */
        if (appcmd == NULL)
        {
            XPRINTF((">appcmd == NULL\n"));
            pbuf_free(p);
            return ERR_ARG;
        }
//ccr2017-10-17        appcmd->textlen = p->tot_len;//�յ��������ܳ���

        for (q=p; q!=NULL; q=q->next)
        {//ccr2017-10-17 ��ȡ�ӷ��������ص�����
            if ((i+q->len)>sizeof(PCBuffer))
            {
                if (BIT(ApplVar.AP.NetWork.Option,SYNSERVERTIMEflag) && !DateTimeStrFromHttp[0])
                {
                    str=strstr(PCBuffer,"Date:");
                    if (str!=NULL)
                        memcpy(DateTimeStrFromHttp,str,HTTPDATELEN-1);//POSTResult
                }

                appcmd->textlen += i;//ccr2017-10-17//�յ��������ܳ���
                PCBuffer[i]=0;
#if defined(FOR_DEBUG)
				xputs(PCBuffer);
#endif
                i=0;
            }
            memcpy(PCBuffer+i, q->payload, q->len);  //ͬһ�����ݰ������һ��pbuf������
            i+=q->len;
        }
        if (i)
        {
            if (BIT(ApplVar.AP.NetWork.Option,SYNSERVERTIMEflag) && !DateTimeStrFromHttp[0])
            {
                str=strstr(PCBuffer,"Date:");
                if (str!=NULL)
                    memcpy(DateTimeStrFromHttp,str,HTTPDATELEN-1);//POSTResult
            }
            appcmd->textlen += i;//ccr2017-10-17//�յ��������ܳ���
            PCBuffer[i]=0;
#if defined(FOR_DEBUG)
            xputs(PCBuffer);
            if (DateTimeStrFromHttp[0])
            {
				       xputs("\nDateTime From server>");
				       xputs(DateTimeStrFromHttp);
            }
#endif
            //if (i)
            {
              str=strstr(PCBuffer,"ERR;");
              if (str!=NULL)
                  strncpy(appcmd->FromServer,str,sizeof(appcmd->FromServer)-1);//POSTResult
              else
              {
                  str=strstr(PCBuffer,"OK");
                  if (str!=NULL)
                      strncpy(appcmd->FromServer,str,sizeof(appcmd->FromServer)-1);//POSTResult
                  else
                      strcpy(appcmd->FromServer,"ERR;99;2");//POSTResult,�Զ������99
              }
            }
        }

        //ȡ��Ӧ�ò����ݺ����ͷ�pbuf������Ӧ���������ٶ����ݽ��н�����ִ�в���
        /* End of processing, we free the pbuf */
        pbuf_free(p);
    } else if (err == ERR_OK)
    {//ccr2017-10-17������ȡ���,�������ر�����
         XPRINTF((">The Server close the connection!\n"));
        /* When the pbuf is NULL and the err is ERR_OK, the remote end is closing the connection. */
        /* We free the allocated memory and we close the connection */
#if HTTP_ONLY1
        appcmd->app_state=CLIENT_CLOSE;
#else
        mem_free(appcmd);//ccr2017-10-17�ͷſռ�HTTPPost_DailyZ��tcp_arg(client_pcb, mem_calloc(sizeof(struct HTTP_Client_app_arg), 1));����Ŀռ�
#endif

        return tcp_close(pcb);
    }
    else
        XPRINTF((">Error on HTTP_Client_recv:%d!\n",err));

    return ERR_OK;
}

err_t HTTP_Client_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    struct HTTP_Client_app_arg* pro_arg = (struct HTTP_Client_app_arg*)arg;

    XPRINTF((">tcp client sent\n"));

    switch (pro_arg->app_state)
    {
    case CLIENT_WAITING_FOR_CMD:
        {
            XPRINTF((">Client waiting for CMD!\n"));
            break;
        }
    case CLIENT_CLOSE:
        {
            XPRINTF((">CLIENT CLOSE!\n"));
#if (HTTP_ONLY1==0)
            mem_free(pro_arg);//ccr2017-10-17�ͷſռ�HTTPPost_DailyZ��tcp_arg(client_pcb, mem_calloc(sizeof(struct HTTP_Client_app_arg), 1));����Ŀռ�
#endif
            return tcp_close(pcb);
        }
    default:
        {
             pro_arg->app_state=CLIENT_SENT;//ccr2017-10-27
             XPRINTF((">Client Send OK!\n"));
             break;
        }
    }

    return ERR_OK;
}


err_t HTTP_Client_poll(void *arg, struct tcp_pcb *pcb)
{
//ccr2017-10-16    const char* str = "The data come from the stm32 client\n";

    XPRINTF((">tcp client poll\n"));

//ccr2017-10-16    tcp_write(pcb, (const char*)str, strlen(str), 1);

    return ERR_OK;
}

//............................................................................................
const char EUTRONBoundary[]="------EuTrOnyb34ZRmyZ28549";

//............................................................................................
/*********************************************************************
 *
 * ��TCP-IP���ӳɹ���,��������HTTP POSTЭ�鷢�͸�������,���ݵķ��ʹ���:
 * 1.head_str
 * 2.cont_str
 * 3.�ļ�����
 * 4.ends
 *
 * @author EutronSoftware (2017-11-01)
 *
 * @param arg
 * @param pcb
 * @param hostname
 * @param api
 ********************************************************************/
void HTTP_Post(void *arg,struct tcp_pcb *pcb,char* hostname, char* api)
{
#define head_str PCBuffer   //HTTP POSTͷ������,����PCBuffer
#define ends ProgLine1Mes   //HTTP POST����������,����ProgLine1Mes
#define cont_str SDRW_Buffer //HTTP POST���ݲ��ֵı�ʾ,����SDRW_Buffer

    int l,file_len,cont_len,ASELen,lp,endsLen;
    FRESULT result;
    int32_t rLen;
    err_t tcpErr;

#if (ENCRYPTBYAES && (AESCIPHERFILE==0))//�����ݽ��м���>>>>>>>>
    unsigned char pCipherText[AES_ENCR_LEN];
//#define pCipherText app_arg->FromServer
#endif

    struct HTTP_Client_app_arg* app_arg = (struct HTTP_Client_app_arg*)arg;

    app_arg->app_state = CLIENT_WAITING_FOR_CMD;

#if defined(TWO_SDCARDS)
   ff_MountSDisk(FS_SD_2ND);
#else
   ff_MountSDisk(FS_SD);
#endif
   /* ���ļ� */
   result = ff_OpenFileQ(app_arg->FileName, FA_OPEN_EXISTING | FA_READ);

   if (result ==  FR_OK)
   {
       if ((file_len=ff_FileSize())>0)
       {
#if (ENCRYPTBYAES && (AESCIPHERFILE==0))//����,���ļ��ж�ȡ����,�����ݽ��м���,Ȼ���͸�������
           for (l=0;l<AES_ENCR_LEN;l++)
                app_arg->AESIV[l]=random(255);//ccr2018-06-20
            AES_Init(ApplVar.AP.NetWork.SecurityCode);
#endif
            //ccr2018-06-21 �������ݱ���ΪAES_ENCR_LEN����>>>>>>
            ASELen=file_len / AES_ENCR_LEN;
            ASELen*=AES_ENCR_LEN;
            if (ASELen<file_len) ASELen+=AES_ENCR_LEN ;
            //<<<<<<<<ccr2018-06-21 �������ݱ���ΪAES_ENCR_LEN����
        //>>>>>>>>>>>>>>cont_str
            sprintf(cont_str,"--%s\r\n",EUTRONBoundary);
#if (ENCRYPTBYAES)//׷��IV����(HEX��ʽ������
            strcat(cont_str,"Content-Disposition: form-data; name=\"IV\"\r\n\r\n");//��˫CRLF��ʾ��Ϣ��ͷ�Ѿ��������ڴ�֮ǰΪ��Ϣ��ͷ
            l=strlen(cont_str);
            HEXtoASCL(cont_str+l,app_arg->AESIV,AES_ENCR_LEN);//sizeof(app_arg->AESIV));
            l+=AES_ENCR_LEN*2;
            cont_str[l++]='\r';        cont_str[l++]='\n';
            sprintf(cont_str+l,"--%s\r\n",EUTRONBoundary);//IV���ݽ���<<<<
#endif
            sprintf(cont_str+strlen(cont_str),"Content-Disposition: form-data; name=\"S_TXT_FILE\"; filename=\"%s\"\r\n",app_arg->SFileN4POST);
            strcat(cont_str,"Content-Type: text/plain\r\n\r\n");//��CRLF��ʾ��Ϣ��ͷ�Ѿ��������ڴ�֮ǰΪ��Ϣ��ͷ
         //<<<<<<<<<<<<<<<<<<<<<<<<
         //>>>>>>>>>>>>>>>>>>>>ends
            sprintf(ends,"\r\n--%s--\r\n\r\n",EUTRONBoundary);//�������ݷ�����ϱ�־(--EUTRONBoundary--)
            endsLen=strlen(ends);
        //<<<<<<<<<<<<<<<<<<<<<<<<<
        //>>>>>>>>>>>>��ʼ��HTTPͷ��Ϣ
            head_str[0] = 0;
            sprintf(head_str, "POST %s HTTP/1.1\r\nUser-Agent: DocMsign\r\n",api);//sprintf(head_str, "POST %s HTTP/1.1\r\n","/myweb/websend.php");
            sprintf(head_str+strlen(head_str), "Host: %u.%u.%u.%u \r\nAccept: */*\r\n",(pcb->remote_ip.addr & 0xff),
                                                                        ((pcb->remote_ip.addr>>8) & 0xff),
                                                                        ((pcb->remote_ip.addr>>16) & 0xff),
                                                                        ((pcb->remote_ip.addr>>24) & 0xff));

            //strcat(head_str, "User-Agent: DocMsign\r\n");
            //sprintf(head_str+strlen(head_str), "Referer: %s\r\n",api);//���߷������Ǵ��ĸ�ҳ�����ӹ�����
            //strcat(head_str, "Accept-Encoding: gzip,deflate,sdch\r\n");
            //strcat(head_str, "Accept-Language: GR-ELOT928\r\n");
            //strcat(head_str,"Expect: 100-continue\r\n");//��Ҫ�ȴ�������Ӧ��"100 continue"���ڷ��ͺ�������
            //strcat(head_str, "Connection: keep-alive\r\n");
            //strcat(head_str, "Cache-Control: no-cache\r\n");
            //strcat(head_str, "Origin: http://yyfs.yy.com\r\n");
            //strcat(head_str, "Upgrade-Insecure-Requests: 1\r\n");//���߷���������������Դ���httpsЭ��
            sprintf(head_str+strlen(head_str), "Content-Type: multipart/form-data; boundary=%s\r\n",EUTRONBoundary);

            cont_len=ASELen+strlen(cont_str)+endsLen;
            sprintf(head_str+strlen(head_str), "Content-Length: %d\r\n\r\n",cont_len);//��˫CRLF��ʾ��Ϣ��ͷ�Ѿ��������ڴ�֮ǰΪ��Ϣ��ͷ
        //<<<<<<<<<<<<<<<<<<<<<<<<<<
            //������Ϣ
        #if defined(FOR_DEBUG)
            xputs(head_str);
            xputs(cont_str);
        #endif
            tcpErr=tcp_write(pcb, (const char *)head_str, strlen(head_str), 1);
            if (tcpErr==ERR_OK)
                tcpErr=tcp_write(pcb, (const char *)cont_str, strlen(cont_str), 1);

            //���ļ��ж�ȡ���ݲ����ͳ�ȥ>>>>>>>>>>>>>>>>>>>>>>>>>>
            //tcp_write(pcb, (const char *)content, strlen(content), 1);//testonly
           l=0;

           if (tcpErr==ERR_OK)
           {
               while (!ff_EofFile())
               {
                   memset(SDRW_Buffer,0,SENDLEN);
                   rLen = ff_ReadFile(SDRW_Buffer,SENDLEN,FS_BINARY);
                   if (rLen>0)
                   {
#if (ENCRYPTBYAES && (AESCIPHERFILE==0))//�����ݽ��м���>>>>>>>>
                       for (lp=0;lp<rLen;lp+=AES_ENCR_LEN)
                       {
                           memcpy(pCipherText,SDRW_Buffer+lp,AES_ENCR_LEN);
#if AES_MODE == AES_MODE_CBC
                           XorBytes(pCipherText, app_arg->AESIV, AES_ENCR_LEN);
#endif
                           BlockEncrypt(pCipherText);
#if AES_MODE == AES_MODE_CBC
                           memcpy(app_arg->AESIV,pCipherText,AES_ENCR_LEN);
#endif
                           memcpy(SDRW_Buffer+lp,pCipherText,AES_ENCR_LEN);
                       }
#endif//�����ݽ��м���<<<<<<<<<<<<<<<<<
                       //ccr2017-10-27!!!!!>>>>>�ᵼ�����ݷ��ʹ���
                       if (tcpErr==ERR_OK)
                       {
                           tcpErr=tcp_write(pcb, (const char *)SDRW_Buffer, lp, 1);//==ERR_MEM)

                           l+=lp;
#if defined(FOR_DEBUG)
                           SDRW_Buffer[lp]=0;
                           xputs(SDRW_Buffer);
#endif
                       }
                   }
                   else
                       break;
               }//while
           //����ļ��е����ݳ��Ȳ���,����ȱʧ����
               if (tcpErr==ERR_OK)
               {
                   if (l<file_len)
                   {
                       memset(SDRW_Buffer,0,sizeof(SDRW_Buffer));
                       while (l<ASELen)
                       {
                           rLen=ASELen-l;
                           if (rLen>SENDLEN) (rLen=SENDLEN);
                           if (tcpErr==ERR_OK)
                               tcpErr=tcp_write(pcb, (const char *)SDRW_Buffer, rLen, 1);//==ERR_MEM)

#if defined(FOR_DEBUG)
                           SDRW_Buffer[rLen]=0;
                           xputs(SDRW_Buffer);
#endif
                           l+=rLen;
                       }
                       //���ͳ�����;
                       XPRINTF((">file read Error(%s)\n",app_arg->FileName));
                       strcpy(app_arg->FromServer,"ERR;99;1");//POSTResult,�Զ������
                   }
                   if (tcpErr==ERR_OK)
                       tcpErr=tcp_write(pcb, (const char *)ends, endsLen, 1);
#if defined(FOR_DEBUG)
                   xputs(ends);
                   XPRINTF(("\n(%d)\n",cont_len+strlen(head_str)));
#endif
               }
           }
       }

       /* �ر��ļ�*/
       ff_CloseFile();
       if (tcpErr!=ERR_OK)
       {
           app_arg->app_state=CLIENT_CLOSE;//���ڿ���HTTP_Client_sentִ��tcp_close(pcb);
           XPRINTF((">tcp write Error(%s)\n",lwip_strerr(tcpErr)));
       }
    }
    else
    {
#if HTTP_ONLY1
        app_arg->app_state=CLIENT_CLOSE;
#else
        mem_free(app_arg);//ccr2017-10-17�ͷſռ�HTTPPost_DailyZ��tcp_arg(client_pcb, mem_calloc(sizeof(struct HTTP_Client_app_arg), 1));����Ŀռ�
#endif
        tcp_close(pcb);
        XPRINTF((">tcp close when OpenFile Error\n"));
    }
    ff_MountSDisk(FS_RELEASE);

}


err_t HTTP_Client_connected(void *arg, struct tcp_pcb *pcb, err_t err)
{
    struct HTTP_Client_app_arg* app_arg = (struct HTTP_Client_app_arg*)arg;
    char strIP[16];

    XPRINTF((">tcp client connected\n"));

    tcp_err(pcb, HTTP_Client_errf);             //ָ������ʱ�Ļص�����
    tcp_recv(pcb, HTTP_Client_recv);            //ָ�����յ�������ʱ�Ļص�����
    tcp_sent(pcb, HTTP_Client_sent);            //ָ��Զ�������ɹ����յ����ݵĻص�����
    tcp_poll(pcb, HTTP_Client_poll, 4);         //ָ����ѯ��ʱ�����ͻص�������*250ms��
    sprintf(strIP,"%u.%u.%u.%u",(app_arg->destip.addr & 0xff),
                                ((app_arg->destip.addr>>8) & 0xff),
                                ((app_arg->destip.addr>>16) & 0xff),
                                ((app_arg->destip.addr>>24) & 0xff));


    HTTP_Post(arg,pcb, strIP, ApplVar.AP.NetWork.ServerURL);//, (char*)SFileN4POST,(char*)AESCipherFile);
    //HTTP_Post(arg,pcb, strIP, "http://147.102.24.100/myweb/websend.php");//, (char*)SFileN4POST,(char*)AESCipherFile);

    return ERR_OK;
}
//===============================================================================================
#if 0   //>>>>>>>DEBUG HTTP POST ONLY>>>>>>>>>>>>>>>>>>>>

//............................................................................................
const char S_FileN[]="CNN7700000301211710171219_s.txt";
const char S_TXT[]="800702480;CCN77000003;;201710171219;0001;0000289;0121;14.08;0.00;0.00;0.00;0.00;0.92;0.00;0.00;0.00;0;B0CE08B5EAC7B913C8A982EB6441D7FA905065F5\r\n"
                    "800702480;CCN77000003;;201710171219;0002;0000290;0121;0.00;10.62;10.57;9.56;15.00;0.00;1.38;2.43;3.44;0;C95B9AB6EB09F56D12EDFE688F035960430C2AAC\r\n"
                    "72A2D390CB7834FEC5EADE6329BD8113A9004BC9";

//............................................................................................
void HTTP_Post_DEBUG(void *arg,struct tcp_pcb *pcb,char* hostname, char* api,char* filen,char* content)
{
#define send_str PCBuffer
#define ends ProgLine1Mes
#define endsss SDRW_Buffer


    struct HTTP_Client_app_arg* app_arg = (struct HTTP_Client_app_arg*)arg;

    app_arg->app_state = CLIENT_WAITING_FOR_CMD;

    //��ʼ��������Ϣ
    send_str[0] = 0;
     //ͷ��Ϣ
    sprintf(send_str, "POST %s HTTP/1.1\r\n",api);
    sprintf(send_str+strlen(send_str), "Host: %s \r\n",hostname);
    //strcat(send_str, "Connection: keep-alive\r\n");

    sprintf(endsss,"--%s\r\n",EUTRONBoundary);
    strcat(endsss,"Content-Disposition: form-data; name=\"S_TXT_FILE\"; filename=\"");
    strcat(endsss,filen);
    strcat(endsss,"\"\r\n");
    strcat(endsss,"Content-Type: application/octet-stream\r\n\r\n");//��CRLF��ʾ��Ϣ��ͷ�Ѿ��������ڴ�֮ǰΪ��Ϣ��ͷ

    sprintf(ends,"\r\n--%s--\r\n",EUTRONBoundary);

    sprintf(send_str+strlen(send_str), "Content-Length: %d\r\n",strlen(content)+strlen(endsss)+strlen(ends));
    strcat(send_str, "Cache-Control: no-cache\r\n");
    //strcat(send_str, "Origin: http://yyfs.yy.com\r\n");
    //strcat(send_str, "Upgrade-Insecure-Requests: 1\r\n");
    //strcat(send_str, "User-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/535.1 (KHTML, like Gecko) Chrome/15.0.849.0 Safari/535.1\r\n");
    strcat(send_str, "Content-Type: multipart/form-data; boundary=");
    strcat(send_str, EUTRONBoundary);
    strcat(send_str, "\r\nAccept: text/plain,charset=windows-1253\r\n");
    //strcat(send_str, "Referer: http://yyfs.yy.com/\r\n");
    //strcat(send_str, "Accept-Encoding: gzip,deflate,sdch\r\n");
    //strcat(send_str, "Accept-Language: GR-ELOT928\r\n");
    //strcat(send_str,"Expect: 100-continue\r\n");
    //������Ϣ
    strcat(send_str, "\r\n");//��CRLF��ʾ��Ϣ��ͷ�Ѿ��������ڴ�֮ǰΪ��Ϣ��ͷ
    //strcat(send_str,"------EUTRONBoundarybZRmyZBq9p09AotO\r\n");
    //strcat(send_str,"Content-Disposition: form-data; name=\"type\"\r\n\r\n");
    //strcat(send_str, "\r\n");
    //strcat(send_str, "HTTP/1.1 100 Continue\r\n");
    //strcat(send_str,"Content-Transfer-Encoding: binary\r\n");
#if defined(FOR_DEBUG)
    xputs(send_str);
    xputs(endsss);
    xputs(content);
    xputs(ends);
    XPRINTF(("\n(%d)\n",strlen(send_str)+strlen(endsss)+strlen(content)+strlen(ends)));
#endif
    tcp_write(pcb, (const char *)send_str, strlen(send_str), 1);
    tcp_write(pcb, (const char *)endsss, strlen(endsss), 1);
    tcp_write(pcb, (const char *)content, strlen(content), 1);
    tcp_write(pcb, (const char *)ends, strlen(ends), 1);
}


err_t HTTP_Client_connected_DEBUG(void *arg, struct tcp_pcb *pcb, err_t err)
{
    struct HTTP_Client_app_arg* app_arg = (struct HTTP_Client_app_arg*)arg;

    XPRINTF(("tcp client connected\n"));

    tcp_err(pcb, HTTP_Client_errf);             //ָ������ʱ�Ļص�����
    tcp_recv(pcb, HTTP_Client_recv);            //ָ�����յ�������ʱ�Ļص�����
    tcp_sent(pcb, HTTP_Client_sent);            //ָ��Զ�������ɹ����յ����ݵĻص�����
    tcp_poll(pcb, HTTP_Client_poll, 4);         //ָ����ѯ��ʱ�����ͻص�������*250ms��

    HTTP_Post_DEBUG(arg,pcb, "147.102.24.100", "http://147.102.24.100/myweb/websend.php", (char*)S_FileN,(char*)S_TXT);

    return ERR_OK;
}

/****************************************************************************
 * ����HTTP ��������ͨѶ
 *
 * @author EutronSoftware (2017-10-17)
 *
 * @return BYTE
 *****************************************************************************/
BYTE HTTPPost_DailyZ_DEBUG(void)
{
    struct tcp_pcb* client_pcb;
#if !HTTP_ONLY1
    struct HTTP_Client_app_arg Http_arg;//ccr2017-10-17���þ�̬����,ֻ����һ��ͨѶ
#else
    if (HTTP_state!=CLIENT_CLOSE && HTTP_state!=CLIENT_ERROR)
        return false;
#endif
    XPRINTF(("tcp client inti\n"));


#if (LWIP_DNS)
    //����ͨ��URL��������ȡ��������IP��ַ
    if (BIT(ApplVar.AP.NetWork.Option,IPByDHCPFlag))//&& ApplVar.AP.NetWork.ServerURL[0]
    {
        GetIPByDNSFromURL(ApplVar.AP.NetWork.ServerIP,ApplVar.AP.NetWork.ServerURL,true);
    }
#endif


    client_pcb = tcp_new();
    if (client_pcb != NULL)
    {
//                tcp_bind(client_pcb,IP_ADDR_ANY,2200);
#if HTTP_ONLY1
        tcp_arg(client_pcb, (void*)&Http_arg);
        Http_arg.destip.addr = (uint32_t)147+(102<<8)+(24<<16)+(100<<24);
        HTTP_state = CLIENT_WAITING_FOR_CONNECTION;
        tcp_connect(client_pcb, &Http_arg.destip, 80, HTTP_Client_connected_DEBUG);         //���������ӣ���������
#else
        tcp_arg(client_pcb, mem_calloc(sizeof(struct HTTP_Client_app_arg), 1));
        Http_arg = client_pcb->callback_arg;
        Http_arg->destip.addr = (uint32_t)147+(102<<8)+(24<<16)+(100<<24);
        HTTP_state = CLIENT_WAITING_FOR_CONNECTION;
        tcp_connect(client_pcb, &Http_arg->destip, 80, HTTP_Client_connected_DEBUG);         //���������ӣ���������
#endif
        return true;
    } else
    {
        XPRINTF(("tcp create failed\n"));
        return false;
    }
}
#endif//<<<<<<<<<<<<<<<<<DEBUG HTTP POST ONLY<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

