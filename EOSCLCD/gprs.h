
#ifndef GPRS_H
#define GPRS_H

#if (defined(CASE_GPRS) && defined(CASE_QRCODE))
#error "CASE_GPRS & CASE_QRCODE"
#endif

//-------------------------------------------------------------
#define RET_OK   ('O'+('K'<<8))
#define RET_ERROR   ('E'+('R'<<8)+('R'<<16)+('O'<<24))

//----Flags for GPRS_Ready----------
#define GPRS_OK_BIT     BIT0       //������ʾGPRS�Ƿ�׼���ã��Ƿ���ڣ�
#define GPRS_TRY_BIT    BIT1       //��ʾ�ظ����һ��GPRS
#define GPRS_NO_BIT     BIT2       //��ʾ����Σ�GPRS��Ȼû��׼���ã����ٽ��м��
#define GPRS_NO_SIM     BIT3       //��ʾ��SIM��
#define GPRS_SKIP_SIM   BIT4        //���ټ��SIM��
#define GPRS_LINK_ERR    BIT5        //�������ͨѶ�쳣
//-------------------------------------------------------------
#define RETRYTIMES  5

#if defined(CASE_INDIA)
#define ACK_MUST     1   //ccr2016-08-11����ͨ��ʱ���������տ��֮����Ҫ����ACKӦ��(ӡ������,��Ӱ��Ч��_
#endif

#if(defined(DEBUGBYPC))
#define GPRSPORT COMPUTER_1
#else
//#define GPRSPORT 2   //GPRSPORT,0,1,2
#define GPRSPORT 2   //jdb2019-02-27
#endif

extern void mDrawGPRS(BYTE ColNum, BYTE LinNum,BYTE force);

extern void TestSendMess(void);
extern void GPRSSendECR_LOG(void);


extern void ProcessGprs(void);

BYTE GPRSStartGprs(void);

BYTE ASC_2_HEX(BYTE pASC);
void GPRSSendHEX(BYTE byte);  //send data one byte, such as  SOH and so on..
BYTE GPRSSkipReceived(ULONG mWaitFor);

short GPRSReadAString(BYTE *str,short size,ULONG sWaitFor);
// "ֱ�ӴӴ��ڶ�ȡGPRS���ص����� %IPDATA......"//
short GPRSRead_IPDATA(char *pBuff,short size,ULONG sWaitFor);
// "ͨ��AT%IPSEND ��������" //

BYTE GPRSConnectAPN(void);
BYTE GPRSRegisterTCPIP(void);
BYTE GPRSConnect_TCPIP(void);
BYTE GPRSSIM_Exist(void);
void GPRS_Restart(void);
BYTE GPRSCloseTCPIP(BYTE sMode);
BYTE GPRSReset(WORD pSET);
void GPRSWaitForReady(BYTE init);
BYTE GPRSSend_ATE0(void);
BYTE GPRSCheckACK(BYTE ACKmust);
void GPRSSendECR_HeartBeat(BYTE force);//ccr2015-08-03
BYTE GPRS_ProcessRecord(char cmd,BYTE *srvData,BYTE psize);//ccr2016-08-19
BYTE GPRSWaitForOK(ULONG sWaitFor);
void GPRS_DrawSignal(void);

short GPRSDataDelRD(BYTE *str, short size);//jdb2019-02-27ȥ���س��ͻ��з�
int GPRS_AT();//jdb2019-02-27�����ж�AT�����Ƿ���ã�����GPRSģ��������������GPRSģ��������
int GPRS_ATCPIN();//jdb2019-03-11���SIM��״̬
int GPRS_ATCSTT();//jdb2019-03-11��ʼAPN����
int GPRS_ATCIISR();//jdb2019-03-11��ʼ������������
int GPRS_ATCIFSR();//jdb2019-03-11��ȡIP��ַ

BYTE GPRS_APN_CSD();//jdb2019-02-27��ʼAPN���񣬽���������·
BYTE GPRS_TCP_CONNECT();//jdb2019-02-27���ӷ�����
BYTE GPRS_TCP_CLOSE();//jdb2019-02-27�ر�TCP����
BYTE GPRS_TCP_STATUS();//jdb2019-02-27���TCP����״̬
void GPRS_RESET_DELAY();//jdb2019-02-27����TCP�������ӳ�ʱʱ��


void GPRSDealy(ULONG sWaitFor);//jdb2019-02-27��ʱ

#define cmdUPDATE       'U'     //Ϊ������������
#define cmdDOWNLOAD     'D'     //Ϊ������������
#define cmdFLOWDATA     'F'     //Ϊ�����տ����ˮ����
#define cmdANSWER       'A'     //Ϊ��host��Ӧ������
#define cmdGRAPHIC      'G'     //Ϊ����ͼƬ
#define cmdCLEAR        'C'     //�������
#define cmdINIT         'I'     //�տ����ʼ��
#define cmdSERVER       'S'     //�����տ���ķ�������ַ
#define cmdPASSWORD     'W'     //�����տ�Ա����

#endif
