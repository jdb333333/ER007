//==================================1.ͷ�ļ�������ʼ==================================
// ��Դ:http://www.05935.com/83/95541/
#include "ping.h"
#include "lwip/raw.h"
#include "DataProcess.h"
#include "lwip/icmp.h"
//==================================1.ͷ�ļ���������==================================
struct raw_pcb *ping_pcb;
OS_EVENT *  pingEchoReply;    //ping�ظ�����




//fuc:����pcb
unsigned char icmp_pcb_init(void)
{
    ping_pcb = raw_new(IP_PROTO_ICMP);
    if(!ping_pcb){
        return 1;
    }
}


//fuc:��װicmp�ı���


//fuc:����icmp���ݰ�
void ping_send(u8 ip_addr1,u8 ip_addr2,u8 ip_addr3,u8 ip_addr4)
{
    struct pbuf *p;
    struct ip_addr ipAddr;
    struct icmp_echo_hdr *iecho;
    
    //������ַ
    IP4_ADDR(&ipAddr, Para.MIP[0], Para.MIP[1], Para.MIP[2], Para.MIP[3]);
    ip_addr_set(&ping_pcb->local_ip, &ipAddr);


    //Ŀ���ַ
    IP4_ADDR(&ipAddr, ip_addr1, ip_addr2, ip_addr3, ip_addr4);
    ip_addr_set(&ping_pcb->remote_ip, &ipAddr);


    p = pbuf_alloc(PBUF_IP, sizeof(struct icmp_echo_hdr), PBUF_RAM);
    if(!p){
        DebugMsg("ping send failed\r\n", 0);
        return;
    }


    //��װicmp�ı���
    iecho = (struct icmp_echo_hdr *)p->payload;
    iecho->type = 8; //����
    iecho->code = 0; //code
    iecho->id =htons(0x0200)  ;     //id
    iecho->seqno =htons(0x5800); //�������к�
    iecho->chksum = 0;  //У�����0
    iecho->chksum = inet_chksum(p->payload, sizeof(struct icmp_echo_hdr));//����У���


    //����ping���ݰ�
    raw_sendto(ping_pcb, p, &ping_pcb->remote_ip);
    pbuf_free(p);
}


void PingTask(void *p_arg)
{
    INT8U err;
    while(1)
    {
        OSTimeDlyHMSM(0, 0, 5, 0);              //ÿ5��ִ��һ������
        
        ping_send(Para.GAT[0],Para.GAT[1],Para.GAT[2],Para.GAT[3]);
        //ping_send(192,168,1,27);
        OSSemPend(pingEchoReply,1500,&err); //�����ź���,������1.5s��ʱ
        if(err!=0){                             //����ʱ
            DebugMsg("ping time out\r\n", 0);
        }else{ 
            DebugMsg("ping success\r\n", 0);
        }
        DebugMsg("cur pipe=%d\r\n", My.DPipe);  //��ӡ��ǰͨ��
    }
}


//ping���񴴽�
#define PING_STACK_SIZE (1024)
#define PING_TASK_PRIO (5)
static OS_STK PingOut_stack[PING_STACK_SIZE];
void PingTaskCreate(void)
{
    icmp_pcb_init();
     pingEchoReply = OSSemCreate(0);
     OSTaskCreate(PingTask, NULL,&PingOut_stack[PING_STACK_SIZE-1],PING_TASK_PRIO);
}