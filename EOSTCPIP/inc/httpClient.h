#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"


#define HTTP_ONLY1      1  //ccr2017-10-17���þ�̬����,ֻ����һ��HTTPͨѶ(��ϵͳֻ֧��HTTP_ONLY1=1�ķ�ʽ)
#define AESCIPHERFILE   0   //=1,ʹ����ʱ�ļ������������,Ȼ���ٷ���;=0,���ļ���ȡ����ֱ�Ӽ��ܺ���
                            //
#define HTTPDATELEN     37  //"Date: Thu, 28 Dec 2017 05:29:59 GMT"

enum HTTP_Client_state
{
    CLIENT_CONNECTED = 0x00U,
    CLIENT_WAITING_FOR_CMD,
    CLIENT_BUSY,
    CLIENT_SENT,
    CLIENT_ERROR,
    CLIENT_CLOSE,
    CLIENT_WAITING_FOR_CONNECTION
};

struct HTTP_Client_app_arg
{
    volatile uint8_t app_state;
    volatile uint8_t textlen;
    volatile struct ip_addr destip;
    char SFileN4POST[34];       //�ṩ��HTTP POST�е��ļ�����
    unsigned char AESIV[AES_ENCR_LEN];	// ��ʼ������
    volatile char FromServer[AES_ENCR_LEN];//������������ص�HTTP�н������,��:"ERR;99;0",��������AES_ENCR_LEN�ֽ�
    char FileName[_MAX_PATH];//Ҫ���͵�s.txt�ļ���·��+�ļ�����
    //uint8_t* dataptr;
};

#if (AESCIPHERFILE==1)
extern char AESCipherFile[];//="\0:/AESCipher.txt";//���ܺ�洢���ļ�����,AESCipherFile[0]=0ʱ,������
#endif

extern void  HEXtoASCL(char *too, BYTE *from, short len);

err_t HTTP_Client_connected(void *arg, struct tcp_pcb *pcb, err_t err);

#if HTTP_ONLY1

extern struct HTTP_Client_app_arg Http_arg;//ccr2017-10-17���þ�̬����,ֻ����һ��ͨѶ
#define POSTResult Http_arg.FromServer
#define s_File Http_arg.FileName
#define HTTP_state  Http_arg.app_state
#define s_FileN4POST Http_arg.SFileN4POST
#else

#define POSTResult Http_arg->FromServer
#define s_File Http_arg->FileName
#define HTTP_state  Http_arg->app_state
#define s_FileN4POST Http_arg->SFileN4POST
#endif


#endif
