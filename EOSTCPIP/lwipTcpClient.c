#include "lwip/tcp.h"
#include "lwip/err.h"
#include "stm32f10x.h"
#include <string.h>
#include <stdio.h>
extern struct netif *enc28j60;
enum tcp_client_state{
        CLIENT_CONNECTED = 0x00U,
        CLIENT_WAITING_FOR_CMD,
        CLIENT_BUSY,
        CLIENT_SENT,
        CLIENT_ERROR,
        CLIENT_CLOSE,
        CLIENT_WAITING_FOR_CONNECTION
};

struct tcp_client_app_arg{
        uint8_t app_state;
        uint8_t textlen;
        uint8_t* dataptr;
};

static struct ip_addr destip;

void tcp_client_errf(void *arg, err_t err)
{
        struct tcp_client_app_arg* pro_arg = (struct tcp_client_app_arg*)arg;

        printf("tcp client err\n");
        
        pro_arg->app_state = CLIENT_ERROR;

        switch(err){
                case ERR_MEM: printf("Out of memory error\n"); break;
                case ERR_BUF: printf("Buffer error\n"); break;
                case ERR_TIMEOUT: printf("Timeout\n"); break;
                case ERR_RTE: printf("Routing problem\n"); break;
                case ERR_ABRT: printf("Connection aborted\n"); break;
                case ERR_RST: printf("Connection reset\n"); break;
                case ERR_CLSD: printf("Connection closed\n"); break;
                case ERR_CONN: printf("Not connected\n"); break;
                case ERR_VAL: printf("Illegal value\n"); break;
                case ERR_ARG: printf("Illegal argument\n"); break;
                case ERR_USE: printf("Address in use\n"); break;
                case ERR_IF: printf("Low-level netif error\n"); break;
                case ERR_ISCONN: printf("Already connected\n"); break;
                case ERR_INPROGRESS: printf("Operation in progress\n"); break;
                default: printf("Unknown error\n");
        }
}
err_t tcp_client_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
        struct pbuf *q;
        struct tcp_client_app_arg *appcmd = (struct tcp_client_app_arg *)arg;
        uint16_t i = 0;
        uint8_t ip[4];
        uint8_t* str;

        const char* cmd1 = "hello";
        const char* cmd2 = "LEDON";
        const char* cmd3 = "LEDOFF";
        const char* cmd4 = "echo IP";
        const char* cmd5 = "bye";

        printf("tcp client recieve\n");
                
        /* We perform here any necessary processing on the pbuf */
        if (p != NULL) 
        {        
                /* We call this function to tell the LwIp that we have processed the data */
                /* This lets the stack advertise a larger window, so more data can be received*/
                tcp_recved(pcb, p->tot_len);
                
                /* Check the name if NULL, no data passed, return withh illegal argument error */
                if(appcmd == NULL) 
                {
                  pbuf_free(p);
                  return ERR_ARG;
                }
                appcmd->dataptr = (uint8_t*)mem_calloc(sizeof(uint8_t), p->tot_len);
                if(appcmd->dataptr == NULL){
                        printf("Memory error\n");
                        return ERR_MEM;
                }
                appcmd->textlen = p->tot_len;
                for(q=p; q!=NULL; q=q->next){
                        memcpy((uint8_t*)&appcmd->dataptr[i], q->payload, q->len);  //同一个数据包会存在一个pbuf链表中
                        i = i + q->len;
                }
                //应用层代码
                switch(appcmd->app_state){
                        case CLIENT_CONNECTED:
                        case CLIENT_WAITING_FOR_CMD: {
                                if(memcmp(appcmd->dataptr, cmd1, strlen(cmd1)) == 0)
                                {
                                        str = "hello\n";
//                                        tcp_write(pcb, (uint8_t*)str, strlen(str), 1);
                                }
                                else if(memcmp(appcmd->dataptr, cmd2, strlen(cmd2)) == 0)
                                {
                                        GPIO_ResetBits(GPIOF,GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);
                                        str="LEDON\n";
                                        //还可以加上这样
                                        /*if(GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_6) == RESET)str="LEDON\n";*/
                                }
                                else if(memcmp(appcmd->dataptr, cmd3, strlen(cmd3)) == 0)
                                {
                                        GPIO_SetBits(GPIOF,GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);
                                        str="LEDOFF\n";
                                }
                                else if(memcmp(appcmd->dataptr, cmd4, strlen(cmd4)) == 0)
                                {
                                        ip[0] = 192;  
                                        ip[1] = 168;  
                                        ip[2] = 0;    
                                        ip[3] = 16;
//                                        ip[0] = enc28j60->ip_addr.addr>>24;         //打出来有点不对         
//                                        ip[1] = enc28j60->ip_addr.addr>>16;
//                                        ip[2] = enc28j60->ip_addr.addr>>8;   
//                                        ip[3] = enc28j60->ip_addr.addr;
//                                        ip[0] = pcb->remote_ip.addr>>24;                 
//                                        ip[1] = pcb->remote_ip.addr>>16;
//                                        ip[2] = pcb->remote_ip.addr>>8;   
//                                        ip[3] = pcb->remote_ip.addr;
                                        str = mem_calloc(sizeof(uint8_t), 30);
                                        sprintf((char*)str, "ipaddr:%d,%d,%d,%d\n", ip[3], ip[2], ip[1], ip[0]);
                                        tcp_write(pcb, (const char *)str, strlen(str), 1);
                                        mem_free(str);
                                        break;
                                }
                                else if(memcmp(appcmd->dataptr, cmd5, strlen(cmd5)) == 0)
                                {
                                        appcmd->app_state = CLIENT_CLOSE;
                                        
                                        break;
                                }
                                else
                                {
                                        str = "unknown cmd\n";
//                                        tcp_write(pcb, (uint8_t*)str, strlen(str), 1);
                                }
                                tcp_write(pcb, (const char *)str, strlen(str), 1);
                                break;
                        }
                        default: 
                        {
                                str = "lwip down\n";
                                tcp_write(pcb, (const char *)str, strlen(str), 1);
                                break;
                        }
                }
                mem_free(appcmd->dataptr);
                //取得应用层数据后，先释放pbuf队列相应数据链，再对数据进行解析与执行操作
                /* End of processing, we free the pbuf */
                pbuf_free(p);
        }  
        else if (err == ERR_OK) 
        {
                /* When the pbuf is NULL and the err is ERR_OK, the remote end 
                                                    is closing the connection. */
                /* We free the allocated memory and we close the connection */
                mem_free(appcmd);
                return tcp_close(pcb);
        }
        return ERR_OK;
}
err_t tcp_client_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
        struct tcp_client_app_arg* pro_arg = (struct tcp_client_app_arg*)arg;

        printf("tcp client sent\n");

        switch(pro_arg->app_state){
                case CLIENT_WAITING_FOR_CMD:{
                        printf("acked!\n");
                        break;
                }
                case CLIENT_CLOSE:{
                        mem_free(pro_arg);
                        return tcp_close(pcb);
                }
                default:{}
        }

        return ERR_OK;
}

err_t tcp_client_poll(void *arg, struct tcp_pcb *pcb)
{
        const char* str = "The data come from the stm32 client\n";

        printf("tcp client poll\n");

        tcp_write(pcb, (const char*)str, strlen(str), 1);

        return ERR_OK;
}
err_t tcp_client_connected(void *arg, struct tcp_pcb *pcb, err_t err)
{
        struct tcp_client_app_arg* app_arg = (struct tcp_client_app_arg*)arg;
        uint8_t* str = "Welcome to the client\n";
        
        printf("tcp client connected\n");

        tcp_err(pcb, tcp_client_errf);                 //指定出错时的回调函数
          tcp_recv(pcb, tcp_client_recv);                 //指定接收到新数据时的回调函数
        tcp_sent(pcb, tcp_client_sent);                 //指定远程主机成功接收到数据的回调函数
        tcp_poll(pcb, tcp_client_poll, 4);         //指定轮询的时间间隔和回调函数（*250ms）

        tcp_write(pcb, (const char *)str, strlen(str), 1);
        app_arg->app_state = CLIENT_WAITING_FOR_CMD;

        return ERR_OK;
}

void tcp_client_init(void)
{
        struct tcp_pcb* client_pcb;
        struct tcp_client_app_arg* app_arg;

        printf("tcp client inti\n");

        destip.addr = (uint32_t)192+(168<<8)+(0<<16)+(105<<24);

        client_pcb = tcp_new();
        if(client_pcb != NULL)
        {
//                tcp_bind(client_pcb,IP_ADDR_ANY,2200);
                tcp_arg(client_pcb, mem_calloc(sizeof(struct tcp_client_app_arg), 1));   
                app_arg = client_pcb->callback_arg;
                app_arg->app_state = CLIENT_WAITING_FOR_CONNECTION;          
                tcp_connect(client_pcb, &destip, 2200, tcp_client_connected);         //按需求连接，加上条件
        }
        else
        {

                printf("tcp alloc failed\n");
        }
}


