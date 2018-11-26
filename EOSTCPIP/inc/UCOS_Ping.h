#ifndef _PING_H_
#define _PING_H_


#include "ucos_ii.h"


#ifdef __cplusplus
extern "C"
{
#endif
//====================================头文件声明开始==================================
//====================================头文件声明结束==================================


//====================================宏定义开始======================================
typedef int pid_t;


#define IPPROTO_IP 0 /* dummy for IP */
#define IPPROTO_ICMP 1 /* control message protocol */
#define IPPROTO_IGMP 2 /* internet group management protocol */
#define IPPROTO_GGP 3 /* gateway^2 (deprecated) */
#define IPPROTO_TCP 6 /* tcp */
#define IPPROTO_PUP 12 /* pup */
#define IPPROTO_UDP 17 /* user datagram protocol */
#define IPPROTO_IDP 22 /* xns idp */
#define IPPROTO_ND 77 /* UNOFFICIAL net disk proto */
#define IPPROTO_RAW 255 /* raw IP packet */
#define IPPROTO_MAX 256
//====================================宏定义结束======================================


extern OS_EVENT *  pingEchoReply;    //ping回复任务
#endif