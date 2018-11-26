#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"


#define HTTP_ONLY1      1  //ccr2017-10-17改用静态变量,只允许一个HTTP通讯(本系统只支持HTTP_ONLY1=1的方式)
#define AESCIPHERFILE   0   //=1,使用临时文件保存加密数据,然后再发送;=0,从文件读取数据直接加密后发送
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
    char SFileN4POST[34];       //提供给HTTP POST中的文件名称
    unsigned char AESIV[AES_ENCR_LEN];	// 初始化向量
    volatile char FromServer[AES_ENCR_LEN];//保存服务器返回的HTTP中结果数据,如:"ERR;99;0",不得少于AES_ENCR_LEN字节
    char FileName[_MAX_PATH];//要发送的s.txt文件的路径+文件名称
    //uint8_t* dataptr;
};

#if (AESCIPHERFILE==1)
extern char AESCipherFile[];//="\0:/AESCipher.txt";//加密后存储的文件名称,AESCipherFile[0]=0时,不发送
#endif

extern void  HEXtoASCL(char *too, BYTE *from, short len);

err_t HTTP_Client_connected(void *arg, struct tcp_pcb *pcb, err_t err);

#if HTTP_ONLY1

extern struct HTTP_Client_app_arg Http_arg;//ccr2017-10-17改用静态变量,只允许一个通讯
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
