/**
  ******************************************************************************
  * @file    Project/STM32F2xx_StdPeriph_Template/main.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    13-April-2012
  * @brief   Main program body
  *****************************************************************************/

#include <stdint.h>

/* Includes ------------------------------------------------------------------*/
#include "king.h"                               /* Kingtron library and I/O routines */
#include "appl.h"
#include "include.h"
#include "spi_m25p80.h"
#include "monitor.h"
#include "test.h"
#include "Ethernet_app.h"
#include "UDisk_demo.h"
#include "diskio.h"
#include "lcd_160x.h"

extern APPLICATION_SAVE  ApplVar;

extern BYTE TESTPAPER(void);
extern void Test_SD(char mode);

/* buffer size (in byte) for R/W operations */
//ouhs 20140814
#if defined   (__CC_ARM)      /* ARM Compiler */
unsigned char UserBuffer[MAX_BUFFER_SIZE] __attribute__ ((aligned (4)));    /*ccr20110829 Buffer used by application                             */
#elif defined (__ICCARM__)    /* IAR Compiler */
    #pragma data_alignment=4
unsigned char UserBuffer[MAX_BUFFER_SIZE];
#endif


uint8_t UserBuffer1[512];
uint8_t UserBuffer2[512];
uint8_t UserBufferSD[1024];

char ConsoleLine[256];              /* Console input buffer */


BYTE    KeyFrHost=0;       //ApplVar.Key code from host computer.
BYTE Bios_Key0_Key;         //    key number: 00 = No key

//extern int SDMMC_main(void);
//extern int FLASH_main(void);

#define TIMER_INTERVAL 	500

/**********************************************************************************************/

/* Uncomment the corresponding line to select the RTC Clock source */
#define RTC_CLOCK_SOURCE_LSE   /* LSE used as RTC source clock */
/* #define RTC_CLOCK_SOURCE_LSI */ /* LSI used as RTC source clock. The RTC Clock
                                      may varies due to LSI frequency dispersion. */

extern __IO uint32_t AsynchPrediv, SynchPrediv;



/* Delay microseconds use RTC*/
void rtc_Delayms(uint32_t ms)
{
    msTimer_Delay0 = 0;
    while (msTimer_Delay0<ms)
    {
    }
}


/*************************************
以下空函数仅用于驱动测试时编译通过
************************************/
void Copy_LCD(BYTE ctrl)
{
}

void SetZReportMust(void)
{}

long DateToLONG(void)
{
	return 0xffff;
}
void DispSt1C(uint8_t * str)
{}
	
BYTE IPToNum(BYTE *Num,char *IPstr,short Counter)
{}

static void ApplicationInit(void)
{
}
short Fiscal_AddDisconnect(WORD disType)
{}
	
void CheckError(short  a)
{}	
void ClearEntry(void)
{}
	

WORD EncordBCDDate(WORD sYear,BYTE sMonth,BYTE sDay)
{}
void GetDateTime(struct TimeDate *pTimeDate)
{}
int  ULongtoASC(char *to, ULONG num1)
{}
BYTE WaitForYesNo(const char *pMessage,BYTE x,BYTE y,BYTE pDefault)
{}
void HEXtoASCL(char *too, BYTE *from, short len)
{}

BYTE  SDRW_Buffer[SD_BLOCKSIZE*2];
BYTE	ProgLine1Mes[64];	
BYTE  PCBuffer[PCBUFMAX];	
CONST FSTRING GPRSMess[G_GPRSMessMax];
CONST struct PROMPTTEXT Prompt;
	
	
int ff_ReadFile(BYTE *data,int length,BYTE mode)
{}
int ff_EofFile(void)
{}
void ff_CloseFile(void)
{}	
FRESULT ff_OpenFileQ(char *filename,BYTE mode)
{}
FRESULT ff_MountSDisk(BYTE mount)
{}	
int ff_FileSize(void)
{}
	
/*-----------------------------------------------------------------------*/
/* CCR 20110824                                                         */
/* Display help messages                                                */
/*-----------------------------------------------------------------------*/
void DisplyMainHelp()
{
	xputs(" ? - Help me \r\n");
	xputs(" b - <UARTx> <baudrate>- Set baudrate of UARTx(0,1)\r\n");
	xputs("           <baudrate>:(0-1200,1-2400,2-4800,3-9600,4-19200,5-38400,6-57600,7-115200)\r\n");
	xputs(" c - cutomer display test\r\n");
	xputs(" d - Drawer test.\r\n");	
	xputs(" D - FatFS test.\r\n");
	xputs(" f - Serial flash test.\r\n");
	xputs(" F - Internal flash test.\r\n");
	xputs(" h - COM2 test.\r\n");
	xputs(" k - Keyboard test.\r\n");
	xputs(" u - USB test.\r\n");
	xputs(" m - External SRAM test.\r\n");
	xputs(" M - Backup SRAM test.\r\n");
	xputs(" n - Ethernet test.\r\n");
	xputs(" s - SD,MMC Card Single block Test.\r\n");
	xputs(" S - SD,MMC Card Multi block Test.\r\n");
	xputs(" g [Gn Gt Gp Gl] - Print Graphic Gn at Gp;\r\n");
	xputs("                   Gt=0/1/2 for text attached.\r\n");
	xputs("                   Gl= Lines of text attached.\r\n");
	xputs(" p <string>- Print test.\r\n");
	xputs(" P - Print test by POS command.\r\n");
	xputs(" t - [yy mm dd hh mm ss]- set Date & Time \r\n");
	xputs(" v - Battery and RTC test \r\n");
	xputs(" x - LowPowerMode test \r\n");
}
/*------------------------------------------------------------------------
  Main menual
-------------------------------------------------------------------------*/
const uint32_t Baudrate[8]={1200,2400,4800,9600,19200,38400,57600,115200};

extern void STM32_Initial(void);  //ouhs 20140815
extern void Print_Initial(void);
extern void Print_ESC_Command(void);
extern WORD PrnTemperature(void);
extern void RTC_SetDateTime (const RTCTime *rtc);
extern void RTC_GetDateTime (RTCTime *rtc);
extern uint8_t CheckKeyboard(void);
extern void Test_USBDevice(void);
/***********************************************************************************************
 * 显示当前日期时间,小数点后面的数据为毫秒计时器
 *
 * @author EutronSoftware (2017-04-14)
 *
 * @param t :=0,将毫秒计时器置0
 *           =1,显示毫秒计时时长
 **********************************************************************************************/
void DisplayTime(BYTE t)
{
    static DWORD sDelayFr=0;
    DWORD delay=0;
    RTCTime rtc;
    RTC_GetDateTime(&rtc);
    if (t==0)
    {
        delay=0;
        sDelayFr=GetSystemTimer();
    }
    else
    {
        delay=GetSystemTimer()-sDelayFr;
        sDelayFr=0;
    }
#if 1//def XPRINTF
    xprintf("%u/%u/%u %02u:%02u:%02u.%lu\r\n", rtc.year, rtc.mon, rtc.mday, rtc.hour, rtc.min, rtc.sec,delay);
#endif
}

void DateToStrYYMMDD(char *YYMMDD)
{
    RTCTime rtc;
    RTC_GetDateTime(&rtc);
    sprintf(YYMMDD,"%02u%02u%02u", rtc.year%100 , rtc.mon, rtc.mday);
}

void TimeToStrHHMM(char *HHMM)
{
    RTCTime rtc;
    RTC_GetDateTime(&rtc);
    sprintf(HHMM,"%02u%02u%", rtc.hour, rtc.min);
}



int main ()
{
    RTCTime rtc;
    char *ptr;
    long p1,p2,p3,p4;
    char c1,c2;
    int i;
    char dispBuf[30];
    BYTE secCurrent;
	
    xfunc_out = UART_PutChar;
    xfunc_in = UART_GetChar;

    ApplicationInit();
    STM32_Initial();
    //p1=0x12345678;
    //CLONG(dispBuf[0])=p1;

    //p1=0xabcdef39;
    //CLONG(dispBuf[4])=p1;
    xputs("Test Main.........\n");
		
		SD_Init();
	
#if defined(CASE_ETHERNET)
	//Ethernet_Start();
#endif

#if EN_PRINT
    Print_Initial();

    ApplVar.PoolPushIn=0;
    ApplVar.PoolPopOut=0;
    ApplVar.Prn_Command=0;

#endif
  
	Test_Keyboard();
	Test_CustomerDis();

#if !defined(CASE_ASCIIDISPLAY)
	Test_OPDisplay();
#endif
	
	DisplyMainHelp();
		 
#if 1
    while (1)
    {		
        xputc('>');
        ptr = ConsoleLine;
        ConsoleLine[0]=0;
        i = get_line(ptr, sizeof(ConsoleLine));			
			
        switch (*ptr++)
        {
        case '?':
            DisplyMainHelp();
            break;
				case 'a':
						Test_Alarm();
						break;
        case 'b':
            while (*ptr == ' ') ptr++;
            c1 = *ptr++;
            if (c1==0 || c1>'1')
            {
                xputs("UARTx needed for setup.\n");
                break;
            }
            while (*ptr == ' ') ptr++;
            c2 = *ptr;
            if (c2==0 || c2>'7')
            {
                xputs("Baudrate needed for setup.\n");
                break;
            }
            c1 &= 0x0f;
            c2 &= 0x0f;
            xprintf("Set UART%d to %d\n",c1,Baudrate[c2]);

            UARTInit(c1,Baudrate[c2],8,0,1);

            break;
        case 'f':		
#if NAND_DISK		
						DemoSpiFlash_FM();	
#else				
						DemoSpiFlash();
#endif
						break;
        case 'F':
            Test_OTPFlash_MAC();//Test_FlashSTM32();//NOR_Flash_Test();
            break;
        case 'm':
						Test_SRAM();
            break;
        case 'M':
            Test_BKPSRAM();
            break;
        case 's':
            Test_SD('s');//Test_znFAT();//Test_SD_FS(FS_SD); //
            break;
        case 'S':	
						if (SD_Detect() == SD_PRESENT)
							DemoFatFS_SD();//Test_SD_FS(FS_SD);//Test_SD('S');//
						else
							xputs(" SD is not present!\r\n");					           
						break;			
				case 'N':
#if defined(CASE_ECR99) || defined(CASE_MCR30)						
#if defined(TWO_SDCARDS)					
						DemoFatFS_Nand();//Test_NAND_FS();//Test_RLFlashFS();//Test_NandFlash();//
#endif	
#endif					
						break;
			
        case 'u':
						USB_Start();
            USB_Test();
            break;
				case 'U':
#if defined(CASE_ECR99) || defined(CASE_MCR30)						
						Test_USBDevice();
#endif						
						break;
        case 'D':
            SD_Init();DemoFatFS_SD();    /* SD FatFS文件操作功能测试 */
            break;
				case 'e':
						DrawerOpen();
						break;
        case 'n':
 #if defined(CASE_ETHERNET)					
            Test_Ethernet(); //Eth_PHY_PowerModeCtrl(ENABLE);
#endif				
            break;
				case 'G':
					Test_SIM800();
						break;
        case 'h':
            Test_COM(LOGIC_COM2);//Ethernet_Test();
            break;
        case 'k':				
						Test_Keyboard();
						break;
        case 'c':
            Test_CustomerDis();
            break;
#if !defined(CASE_ASCIIDISPLAY)
        case 'l':
            Test_OPDisplay();
            break;
#endif
        case 'v':
            Test_PWR();
            break;
				case 'x':
            Test_LowPowerMode();
            break;
        case 'd':
						Test_Drawer();
            break;
#if EN_PRINT
        case 'P':
					Test_Printer(); break;
            if (!TESTPAPER())
            {

               xputs("Paper out.\n");
                //break;
            }
            Print_ESC_Command();
            break;
        case 'p':	
            if (!TESTPAPER())
            {
                xputs("Paper out.\n");
                //break;
            }
            i--;
            while (*ptr == ' ')
            {
                ptr++;
                i--;
            }
            if (*ptr)
            {
                if (*ptr>='0' && *ptr<='9')
                {
                    xatoi(&ptr, &p1);

                    while (p1)
                    {
                        p2 = PrnTemperature();
                        xprintf("ADC1.CH10= %d\n",p2);
                        p1--;
                        for (p2=0;p2<1000000;p2++);
                    }
                } else
                    while (RSPrint(ptr,i,CMDP_RJ)<0);
            } else
                xputs("String needed for print.\n");
            break;
        case 'g':
            if (!TESTPAPER())
            {
                xputs("Paper out.\n");
                //break;
            }
            i--;
            while (*ptr == ' ')
            {
                ptr++;
                i--;
            }
            if (*ptr && *ptr>='0' && *ptr<='9')
            {
                p1=p2=p3=p4=0;
                xatoi(&ptr, &p1);
                xatoi(&ptr, &p2);
                xatoi(&ptr, &p3);
                xatoi(&ptr, &p4);
                if (p1)
                {
                    ConsoleLine[0]=p1;//图片序号
                    ConsoleLine[1]=(p2 & 3)+'0';//是否叠加文字
                    ConsoleLine[2]=p3;//图片打印位置
                    while (RSPrint(ConsoleLine,3,CMDP_PRGRA)<0);
                    if (p2==1)
                    {
                        for (i=0;i<p4;i++)
                        {
                            sprintf(ConsoleLine,"Attach line %d",i);
                            while (RSPrint(ConsoleLine,strlen(ConsoleLine),CMDP_RJ)<0);
                        }
                        ConsoleLine[0]=0;//图片叠加文字发送完毕完毕
                        while (RSPrint(ConsoleLine,1,CMDP_RJ)<0);
                    }
                    for (i=0;i<8;i++)
                    {//打印后续文字
                        sprintf(ConsoleLine,"Printer follow line %d",i);
                        while (RSPrint(ConsoleLine,strlen(ConsoleLine),CMDP_RJ)<0);
                    }
                }
            } else
            {
                for (i=0;i<GRAPHICMAX;i++)
                {
                    ConsoleLine[0]=i+1;
                    ConsoleLine[1]='0';
                    while (RSPrint(ConsoleLine,2,CMDP_PRGRA)<0);
                }
            }
            break;
#endif
        case 't' :  /* t [<year> <mon> <mday> <hour> <min> <sec>] */  /* t 2014 9 18 16 26 59 */
					
				if(RTC_AlarmStatus())
						xputs("RTC_Alarm_IRQHandler during power off.\n");						
				if (DaylightSavingTimeSet())
				{
					if (RTC_GetStoreOperation() == RTC_StoreOperation_Set)
					{
						xputs("to summer time...\n");
					}
					else
					{
						xputs("to winner time...\n");
					}
				}	
				
            if (xatoi(&ptr, &p1))
            {
                rtc.year = (WORD)p1;
                xatoi(&ptr, &p1); rtc.mon = (BYTE)p1;
                xatoi(&ptr, &p1); rtc.mday = (BYTE)p1;
                xatoi(&ptr, &p1); rtc.hour = (BYTE)p1;
                xatoi(&ptr, &p1); rtc.min = (BYTE)p1;
                if (!xatoi(&ptr, &p1)) p1=0;
                rtc.sec = (BYTE)p1;
                RTC_SetDateTime(&rtc);
            }
            RTC_GetDateTime(&rtc);
            xprintf("%u/%u/%u %02u:%02u:%02u\n", rtc.year, rtc.mon, rtc.mday, rtc.hour, rtc.min, rtc.sec);
            secCurrent = rtc.sec;
            while (!CheckKeyboard() && !UARTGet(PORT_DEBUG,0,0))
            {
                RTC_GetDateTime(&rtc);
                if (secCurrent!=rtc.sec)
                {
                    secCurrent = rtc.sec;
                    sprintf(dispBuf," %02u:%02u:%02u", rtc.hour, rtc.min, rtc.sec);
                    //PutsO(dispBuf);
                }
            }
            break;
        }//switch
    }//for
#endif
}



#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}
#endif


CONST FSTRING Msg[]={
    " ",                            // space
#if (DD_ZIP||DD_ZIP_21||DD_LCD_1601)
    "MessageM1",                         //    CWXXI01//    Ccr "无效输入!",
    "MessageM2",                             //    CWXXI02//    Ccr "无效日期!",
    "MessageM3",                             //    CWXXI03//    Ccr "无效时间!",
    "MessageM4",                         //    CWXXI04//    Ccr "禁止输入!",
    "MessageM5",                             //    CWXXI05//    Ccr "PLU代码为空!",
    "MessageM6",                         //    CWXXI06//    Ccr "PLU文件溢出!",
    "MessageM7",                         //    CWXXI07//    Ccr "桌台被占用!",
    "MessageM8",                         //    CWXXI08//    Ccr "桌台已打开!",
    "MessageM9",                         //    CWXXI09//    Ccr "桌台号不对!",
    "MessageM10",                        //    CWXXI10//    Ccr "请输入桌台号",
    "MessageM11",                    //    CWXXI11//    Ccr "桌台没有开台",
    "MessageM12",                        //    CWXXI12//    Ccr "桌台桌帐溢出",
    "MessageM13",                    //    CWXXI13//    Ccr "禁止修改时间",
    "MessageM14",                        //    CWXXI14//    Ccr "仍在销售中!",
    "MessageM15",                    //    CWXXI15//    Ccr "销售缓冲区满",
    "MessageM16",                    //    CWXXI16//    Ccr "商品没有售出",
    "MessageM17",                            //    CWXXI17//    Ccr "正在结算中!",
    "MessageM18",                        //    CWXXI18//    Ccr "输入数据超长",
    "MessageM19",                        //    CWXXI19//    Ccr "正在付帐!",
    "MessageM20",                            //    CWXXI20//    Ccr "人数溢出!",
    "MessageM21",                            //    CWXXI21//    Ccr "没有确认结帐",
    "MessageM22",                        //    CWXXI22//    Ccr "禁止取消付款",
    "MessageM23",                    //    CWXXI23//    Ccr "无菜名!",
    "MessageM24",                        //    CWXXI24//    Ccr "无此收款员!",
    "MessageM25",                //    CWXXI25//    Ccr "收款员改变",
    "MessageM26",                        //    CWXXI26//    Ccr "无此类报表!",
    "MessageM27",                        //    CWXXI27//    Ccr "报表打印中断",
    "MessageM28",                        //    CWXXI28//    Ccr "必须在经理档",
    "MessageM29",                    //    CWXXI29//    Ccr "不能分帐时用",
    "MessageM30",                        //    CWXXI30//    Ccr "必须输入号码",
    "MessageM31",                        //    CWXXI31//    Ccr "转入到",
    "MessageM32",                        //    CWXXI32//    Ccr "未授权!",
    "MessageM33",                        //    CWXXI33//    Ccr "请指定收款员",
    "MessageM34",                        //    CWXXI34//    Ccr "请指定营业员",
    "MessageM35",                        //    CWXXI35//    Ccr "禁止PLU变价！",
    "MessageM36",                        //    CWXXI36//    Ccr "密码不对!",
    "MessageM37",                        //    CWXXI37//    Ccr "厨房打印机错",
    "MessageM38",                        //    CWXXI38//    Ccr "票据打印机错",
    "MessageM39",                        //    CWXXI39//    Ccr "打印压杆抬起",
    "MessageM40",                        //    CWXXI40//    Ccr "打印纸用完!",
    "MessageM41",                            //    CWXXI41//    Ccr "打印温度太高",
    "MessageM42",                        //    CWXXI42//    Ccr "按键未定义!",
    "MessageM43",                        //    CWXXI43//    Ccr "必须输入金额",
    "MessageM44",                            //    CWXXI44//    Ccr "禁止多种付款",
    "MessageM45",                            //    CWXXI45//    Ccr "记帐无此功能",
    "MessageM46",                            //    CWXXI46//    Ccr "未按小计键!",
    "MessageM47",                            //    CWXXI47//    Ccr "正在管理库存",
    "MessageM48",                        //    CWXXI48//    Ccr "保存流水错",
    "MessageM49",                            //    CWXXI49//    Ccr "MODEM通讯错!",
    "MessageM50",                        //    CWXXI50//    Ccr "卡操作出错!",
    "MessageM51",                            //    CWXXI51//    Ccr "POS代码错!",
    "MessageM52",                            //    CWXXI52//    Ccr "卡数据错!",
    "MessageM53",                            //    CWXXI53//    Ccr "为过期卡!",
    "MessageM54",                            //    CWXXI54//    Ccr "为挂失卡!",
    "MessageM55",                            //    CWXXI55//    Ccr "客户类别错!",
    "MessageM56",                                    //    CWXXI56//    Ccr "为新卡!",
    "MessageM57",                            //    CWXXI57//    Ccr "不是购物卡!",
    "MessageM58",                            //    CWXXI58//    Ccr "写卡出错!",
    "MessageM59",                            //    CWXXI59//    Ccr "卡号不对!",
    "MessageM60",                        //    CWXXI60//    Ccr "禁用折扣卡!",
    "MessageM61",                        //    CWXXI61//    Ccr "禁用现金卡!",
    "MessageM62",                        //    CWXXI62//    Ccr "禁用赊帐卡!",
    "MessageM63",                            //    CWXXI63//    Ccr "禁用IC卡!",
    "MessageM64",                    //    CWXXI64//    Ccr "清卡出错!",
    "MessageM65",                            //    CWXXI65//    Ccr "数据溢出!",
    "MessageM66",                    //    CWXXI66//    Ccr "IC充值出错!",
    "MessageM67",                    //    CWXXI67//    Ccr "IC初始化出错",
    "MessageM68",                        //    CWXXI68//    Ccr "禁止初始化!",
    "MessageM69",                    //    CWXXI69//    Ccr "IC金额不足!",
    "MessageM70",                    //    CWXXI70//    Ccr "IC付款出错!",
    "MessageM71",                            //    CWXXT71//    Hf "IP地址错",
    "MessageM72",                //    CWXXI72//     ccr "射频卡座错!",
    "MessageM73",                //    CWXXI73//    Cc "连接失败!",
    "MessageM74",                //    CWXXI74>>>>>for fiscal
    "MessageM75",                //    CWXXI75
    "MessageM76",                //    CWXXI76
    "MessageM77",                    //    CWXXI77 FM将满 //
    "MessageM78",                    //    CWXXI78
    "MessageM79",                    //    CWXXI79
    "MessageM80",                //    CWXXI80
    "MessageM81",                //    CWXXI81
    "MessageM82",                //    CWXXI82 EJ中的数据有错 //
    "MessageM83",                        //    CWXXI83
    "MessageM84",                //    CWXXI84 EJ 读写出错 //
    "MessageM85",                //    CWXXI85 EJ无数据 //
    "MessageM86",                //    CWXXI86
    "MessageM87",                //    CWXXI87 EJ将满 //
    "MessageM88",                //    CWXXI88
    "MessageM89",                    //    CWXXI89
    "MessageM90",                        //    CWXXI90
    "MessageM91",                    //    CWXXI91 最多200次加电初始化 //
    "MessageM92",                    //    CWXXI92
    "MessageM93",                        //    CWXXI93 FM 与收款机不匹配 //
    "MessageM94",                            //    CWXXI94
    "MessageM95",                    //    CWXXI95
    "MessageM96",                    //    CWXXI96
    "MessageM97",                    //    CWXXI97
    "MessageM98",                        //    CWXXI98 EJ与收款机不匹配 //
    "MessageM99",                        //    CWXXI99
    "MessageM100",                       //    CWXXI100
#endif	//(DD_ZIP||DD_ZIP_21)
    "MessageN1",             //    KAHAO//    Ccr 	"卡号:",				//    KAHAO
    "MessageN2",             //    KLXING//    Ccr 	"卡类型:",				//    KLXING
    "MessageN3",         //    KNJE//    Ccr 	"卡内金额:",			//    KNJE
    "MessageN4",             //    KYJIN//    Ccr 	"卡押金:",				//    KYJIN
    "MessageN5",         //    XFZE//    Ccr 	"消费总额:",			//    XFZE
    "MessageN6",         //    CHZHZE//    Ccr 	"充值总额:",			//    CHZHZE
    "MessageN7",         //    SHYCSHU//    Ccr 	"使用次数:",			//    SHYCSHU
    "MessageN8",         //    JGLBIE//    Ccr 	"价格级别:",			//    JGLBIE
    "MessageN9",             //    PINMA//    Ccr 	"PIN码:",				//    PINMA
    "MessageN10",            //    BHJBIE//    Ccr 	"保护级别:",			//    BHJBIE
    "MessageN11",            //    ZDJZHANG//    Ccr 	"自动结帐:",			//    ZDJZHANG
    "MessageN12",            //    ZHKRQI//    Ccr 	"制卡日期:",			//    ZHKRQI
    "MessageN13",            //    KYXQI//    Ccr 	"卡有效期:",			//    KYXQI
    "MessageN14",            //    KHMCHEN//    Ccr 	"客户名称:",			//    KHMCHEN
    "MessageN15",                //    CHSHHICKA//    ccr	"初始化IC",								//    CHSHHICKA
    "MessageN16",                        //    ICKCHZHI//    ccr	"IC卡充值",									//    ICKCHZHI
    "MessageN17",                        //    QCHICKA//    ccr	"清除IC卡",									//    QCHICKA
    "MessageN18",  //    GUASHIIC//    ccr "挂失卡号",     //    GUASHIIC      //    ICCard Part ccr chipcard 2004-06-28
    "MessageN19", //    ZHEKOUCA//    ccr "折扣卡:",            //    ZHEKOUCA
    "MessageN20",        //    XIANJINKA//    ccr "现金卡:",           //    XIANJINKA
    "MessageN21",  //    SHEZHANGKA//    ccr "赊帐卡:",           //    SHEZHANGKA
    "MessageN22",//    XFJIDIAN//    ccr "消费计点:",           //    XFJIDIAN
    "MessageN23",        //    ZHKLVF//    Ccr "折扣率(%):",        //    ZHKLVF
    "MessageN24",            //    ICKTKUAN//    Ccr "IC卡退款",			//    ICKTKUAN
    "MessageN25",            //    ICKDJDIANG//    Ccr "IC卡兑奖点数",			//    ICKDJDIANG
    "MessageN26",            //    ICKSDIAN//    Ccr "IC卡送点数",			//    ICKSDIAN
    "MessageN27",                 //    ZHUOTAI//    Ccr "桌台",                       //    ZHUOTAI
    "MessageN28",            //    Cc	"开始收据号",	//    RECNUMFR
    "MessageN29",            //    Cc	"结束收据号",	//    RECNUMTO
    "MessageN30",        //    hf		"EJ初始化"		//    EJCSHUA
    "MessageN31",                        //    hf 	"EJ编号"			//    EJBHAO
    "MessageN32",        //    hf		"EJ 报表"			//    EJBBIAO
    "MessageN33",        //    hf 	"~净~销~售",		//    HSJXSHOU
#if (FISCAL ==1 || DD_FISPRINTER==1)
    "MessageN35",        //Cc	"税率A税额",		//VATA
    "MessageN36",        //Cc	"税率B税额",		//VATB
    "MessageN37",        //Cc	"税率C税额",		//VATC
    "MessageN38",        //Cc	"税率D税额",		//VATD
    "MessageN39",        //Cc	"税率E税额",		//VATE
    "MessageN40",        //Cc	"税率F税额",		//VATF
    "MessageN41",        //Cc	"税率G税额",		//VATG
    "MessageN42",        //Cc	"税率H税额",		//VATH
    "MessageN43",        //Cc	"税额总计",		//TAXTOTAL
    "MessageN44",    //Cc	"净销售",			//NETSALE
    "MessageN45",    //Cc	"交易笔数",		//RECNUMBER
    "MessageN46",        //FMPULLOUT  ccr071212
    "MessageN47",    //Cc	"更改票头",		//CHANGEHEAD
    "MessageN48",    //Cc	"更改税率A",		//TAXACHG
    "MessageN49",    //Cc	"更改税率B",		//TAXBCHG
    "MessageN50",    //Cc	"更改税率C",		//TAXCCHG
    "MessageN51",    //Cc	"更改税率D",		//TAXDCHG
    "MessageN52",    //Cc	"更改税率E",		//TAXECHG
    "MessageN53",    //Cc	"更改税率F",		//TAXFCHG
    "MessageN54",    //Cc	"更改税率G",		//TAXGCHG
    "MessageN55",    //Cc	"更改税率H",		//TAXHCHG
    "MessageN56",            //Cc	"开始日期",		//TAXFROM
    "MessageN57",                //Cc	"截止日期",		//TAXTO
    "MessageN58",    //Cc"税控存储器报表",//PRNFISCAL
    "MessageN59",    //Cc	"分类总计",		//PRNTOTAL
    "MessageN60",        //Cc	"清帐总计",		//CLEARTOTAL
    "MessageN61",        //Cc	"复位总计",		//MACRESET
    "MessageN62",        //Cc"税率更改总计",	//CHANGETAX
    "MessageN63",    //hf 	"税控日报表",	//SHKZBB
    "MessageN64",        //hf 	"税控码",			//TAXCODE
    "MessageN65",        //hf 	"RIF编码",			//RIFCODE
    "MessageN67",    //hf		"免税销售",		//MIANSHUI
    "MessageN68",            //hf 	"税目 A",			//SHUIMUA
    "MessageN69",        //hf		"客户名称",		//KHMINGCH		//20070308
    "MessageN70",            //hf		"机器号",			//JIQIHAO		//20070308
    "MessageN71",                        //hf		"日期",			//RIQI			//20070308
    "MessageN72",                        //hf		"时间",			//SHIJIAN		//20070308
    "MessageN73",                //hf		"销售总计",		//XSHZHJI
    "MessageN74",        //hf		"退货总计",		//THUOZHJI
    "MessageN75",        //hf		"折扣总计",		//ZHKOUZHJI
    "MessageN76",                    //20070313	" SALE    ",!!!!新的名称的长度不要超过TAXABLE//hf 	"普通销售",		//XIAOSHOUA
    "MessageN77",                //hf		" 退货销售",		//THXIAOSHOU
    "MessageN78",                //20070313	" DISC   ",//hf		"折扣销售",		//ZHKOUXSH
    "MessageN79",        //20070313 " REFUND TAX SUM",//hf		"退货税额"		//THZJSHUI
    "MessageN80",        //20070313 " DISC TAX SUM",//hf		"折扣税额"		//ZHKZJSHUI
    "MessageN81",        //    hf 	"EJ信息",			//    EJXINXI
    "MessageN82",            //    hf "EJ初始化时间",		//    EJSHJIAN
    "MessageN83",            //    Cc	"非发票",			//    NONFISCAL
    "MessageN84",        //    liuj 0531 PM
    "MessageN85",            //    Cc	"开始日期",		//    RIQIF
    "MessageN86",            //    Cc	"开始时间",		//    SHIJIANF
    "MessageN87",                //    Cc	"结束日期",		//    JSHRQIF
    "MessageN88",            //    hf		"总计发票号",	//    ZHJIFPHAO
    "MessageN89",                //    hf		"总计退货号",	//    ZHJITHHAO
    "MessageN90",                //    hf		"总计非税号",	//    ZHJIFSHHAO
    "MessageN91",            //    hf		"发票号",			//    FAPIAOHAO
    "MessageN92",                //    hf		"退货号",			//    THUOHAO
    "MessageN93",                //    hf		"非税号",			//    FEISHUIHAO
    "MessageN94",                //    hf		"税控报表号",	//    SHKBBHAO
    "MessageN95",                    //    Cc	"清帐计数"		//    QZHJSHU
    "MessageN96",                    //    REPLEFT
    "MessageN97",                            //SIZEFM
#endif//FISCAL
    "MessageN98 ",                            //SIZEEJ
    "MessageN99 ",            //    VALUEINIC
    "MessageN100",                               //    LOWBAT
    "MessageN101",               //    CLEARIC
    "MessageN102",           //    CHARGEIC
    "MessageN103",           //    ADPOINTS
    "MessageN104",               //    INITIC
    "MessageN105",           //WAITEICCARD
    "MessageN106",           //ICCARDOK
    "MessageN107",           //ePOSTAC //ccr epos//ccr "ePOS认证码:",			//ePOSTAC //ccr epos
    "MessageN108",                   //WAITEPOS
    "MessageN109",           //ePOSCARNO//ccr "易通卡号:",					//ePOSCARNO
    "MessageN110",   //eCARREMAIN//ccr "易通卡余额:",					//eCARREMAIN
    "MessageN111",           //TMHYHAO//ccr "会员号:",			//TMHYHAO
    "MessageN112",   //BARREPORT//ccr  会 员 消 费   //
    "MessageN113",               //XFJE//ccr "消费金额",				//XFJE
    "MessageN114",                 //ZONGJI//Ccr "总计",                       //ZONGJI
    "MessageN115",                       //    BREAK485
    "MessageN116",                       //    LINK485
    "MessageWait",                       // WAITING
    "",
};

