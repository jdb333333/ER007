
#include "king.h"
#include "exthead.h"
#include "interface.h"
#include "stm32f2xx_it.h"
#include "pwr_ctrl.h"
#include "Ethernet_app.h"
#include "malloc.h"

#if defined(CASE_ASCIIDISPLAY)
#include "lcd_160x.h"
#else
extern void LED_Init(void);
#endif

extern SD_CardInfo SDCardInfo;//ccr2017-06-20

#define YEARFrDEC 2000      //RTC���꿪ʼ���
#define YEARFrHEX 0x2000    //RTC���꿪ʼ���

#define DST_STAR_MONTH	3		//��ʱ�ƿ�ʼ�·�(���һ������)
#define DST_END_MONTH		10	//��ʱ�ƽ���ʼ�·�(���һ������)

#define DST_STAR_HOUR		3		//��ʱ�ƿ�ʼʱ��(DST_STAR_MONTH�����һ������)
#define DST_END_HOUR		4		//��ʱ�ƽ���ʱ��(DST_END_MONTH�����һ������)

DATE_BCD rtc_date;          //    buffer per get/set data corrente
TIME_BCD rtc_time;          //    buffer per get/set orario corrente

void KeyBoardProc(void);
void PFOInt_Init(void);

extern void Beep(uint8_t blen);
void msDelay(__IO uint32_t nTime);

volatile BYTE DrawerOpen_time = 0;

#if defined(FOR_DEBUG)
#define FORJLINK	1	//ʹ��JLINK�������غ͵���ʱ,������1
#else
#define FORJLINK	0	//ʹ��JLINK�������غ͵���ʱ,������1
#endif

/**
  * @brief  Configure the RTC peripheral by selecting the clock source.
  * @param  None
  * @retval None
  */


/* Get RTC time */
void RTC_GetDateTime (RTCTime *rtc)
{
    RTC_TimeTypeDef  RTC_TimeStruct;
    RTC_DateTypeDef  RTC_DateStruct;
    RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);
    RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);

    rtc->year = RTC_DateStruct.RTC_Year + YEARFrDEC;
    rtc->mon = RTC_DateStruct.RTC_Month;
    rtc->mday = RTC_DateStruct.RTC_Date;
    rtc->wday = RTC_DateStruct.RTC_WeekDay;

    rtc->hour = RTC_TimeStruct.RTC_Hours;
    rtc->min = RTC_TimeStruct.RTC_Minutes;
    rtc->sec = RTC_TimeStruct.RTC_Seconds;
}

/* Set RTC time */
extern void RTC_DayLightSavingConfig_BCK(uint32_t RTC_StoreOperation);
void RTC_SetDateTime (const RTCTime *rtc)
{
    RTC_TimeTypeDef  RTC_TimeStruct;
    RTC_DateTypeDef  RTC_DateStruct;
	uint8_t years, march_day, octo_day;
	uint32_t seconds;

    years=rtc->year % 100;//ccr2017-08-11

    RTC_DateStruct.RTC_Year        = years;//ccr2017-08-11
    RTC_DateStruct.RTC_Month       = rtc->mon ;
    RTC_DateStruct.RTC_Date        = rtc->mday;
    RTC_DateStruct.RTC_WeekDay     = rtc->wday;

    RTC_TimeStruct.RTC_Hours       = rtc->hour;
    RTC_TimeStruct.RTC_Minutes     = rtc->min ;
    RTC_TimeStruct.RTC_Seconds     = rtc->sec ;

    octo_day=RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct);
//    if (octo_day==ERROR)  ApplVar.ErrorNumber=ERROR_ID(CWXXI02);
//	xprintf("%d-%d-%d.%d ERR:%d\n",RTC_DateStruct.RTC_Year,RTC_DateStruct.RTC_Month,RTC_DateStruct.RTC_Date,RTC_DateStruct.RTC_WeekDay,ApplVar.ErrorNumber);//testonly

    octo_day=RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct);

//    if (octo_day==ERROR) ApplVar.ErrorNumber=ERROR_ID(CWXXI03);
// 	xprintf("%d:%d:%d ERR:%d\n",RTC_TimeStruct.RTC_Hours,RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds,ApplVar.ErrorNumber);//testonly

	//if(SummerTimeEnabled())
	{
		//ccr2017-08-11 years = rtc->year%100;
		march_day = (31-(5*years/4+5)%7);
		octo_day = (31-(5*years/4+2)%7);
		seconds = rtc->sec + rtc->min*60 + rtc->hour*60*60;

		//���ֶ��������ں�ʱ��ʱ���Զ���������ʱ�ı�־λ
		if((rtc->mon>DST_STAR_MONTH && rtc->mon<DST_END_MONTH) ||
		((rtc->mon==DST_STAR_MONTH)&&(rtc->mday>march_day)) ||
		((rtc->mon==DST_END_MONTH)&&(rtc->mday<octo_day)) ||
		((rtc->mon==DST_STAR_MONTH)&&(rtc->mday==march_day)&&(seconds>DST_STAR_HOUR*60*60)) ||
		((rtc->mon==DST_END_MONTH)&&(rtc->mday==octo_day)&&(seconds<DST_END_HOUR*60*60)) )

			RTC_DayLightSavingConfig_BCK(RTC_StoreOperation_Set);
		else
			RTC_DayLightSavingConfig_BCK(RTC_StoreOperation_Reset);
	}
}

/**--------------------------------------------------------------------
 ��������ʱ
--------------------------------------------------------------------**/
extern BYTE SummerTimeEnabled();
BYTE DaylightSavingTimeSet(void)
{
	RTCTime rtc_tmp;

	static uint32_t delayms=0;

	uint8_t years, march_day, octo_day;
	uint32_t seconds;

	if(SummerTimeEnabled())
	{

		if((GetSystemTimer()-delayms)>=1000)	//1s
		{
				delayms = GetSystemTimer();

				RTC_GetDateTime(&rtc_tmp);
				years = rtc_tmp.year%100;
				march_day = (31-(5*years/4+5)%7);	// ƫ��һ�죿����march_day = (31-(5*years/4+4)%7);
				octo_day = (31-(5*years/4+2)%7);	//octo_day = (31-(5*years/4+1)%7);
				seconds = rtc_tmp.sec + rtc_tmp.min*60 + rtc_tmp.hour*60*60;

				if(RTC_GetStoreOperation() == RTC_StoreOperation_Set) //����������ʱ
				{
					if((rtc_tmp.mon>DST_END_MONTH) ||
						((rtc_tmp.mon==DST_END_MONTH)&&(rtc_tmp.mday>octo_day)) ||
						((rtc_tmp.mon==DST_END_MONTH)&&(rtc_tmp.mday==octo_day)&&(seconds>=DST_END_HOUR*60*60))) 	//10�����һ��������AM4:00��ȡ������ʱ
								{
						RTC_DayLightSavingConfig(RTC_DayLightSaving_SUB1H, RTC_StoreOperation_Reset);
										return true;
								}
				}
				else //û����������ʱ
				{
					if((rtc_tmp.mon>DST_STAR_MONTH && rtc_tmp.mon<DST_END_MONTH) ||
					((rtc_tmp.mon==DST_STAR_MONTH)&&(rtc_tmp.mday>march_day)) ||
					((rtc_tmp.mon==DST_END_MONTH)&&(rtc_tmp.mday<octo_day)) ||
					((rtc_tmp.mon==DST_STAR_MONTH)&&(rtc_tmp.mday==march_day)&&(seconds>=DST_STAR_HOUR*60*60)) ||
					((rtc_tmp.mon==DST_END_MONTH)&&(rtc_tmp.mday==octo_day)&&(seconds<(DST_END_HOUR-1)*60*60)) )	//3�����һ��������AM3:00����������ʱ
								{
						RTC_DayLightSavingConfig(RTC_DayLightSaving_ADD1H, RTC_StoreOperation_Set);
										return true;
								}
				}
			}
	}
    return false;
}

/****************************************************************************
ע��:
    �տ��������ҵ�����ݵĴ�/ȡ����ͨ��ReadRam��WriteRam������,
ReadRam��WriteRam����洢�ռ�Ĺ���,�����Ŀռ��ַ��Ӧ�ó�����͸����.
��:Ӧ�ó���ֻ��ReadRam��WriteRam�ṩ��Ч�洢�ռ�������ַ����������
   ���ڴ洢�ռ��Ƿ�����,�洢������FLASHоƬ����SRAM������������,
   �� ReadRam��WriteRam�ڲ�����.

   Ӧ�ó���������������ռ䲻��ReadRam��WriteRam����֮��.
****************************************************************************/
/**
 * ����RamOffSet��ָ���ĵ�ַ,���ļ����ݴ洢���ж�ȡlen�ֽڵ�����
 * ��ȡ���֮��,RamOffSet�Զ�+len
 * @author EutronSoftware (2014-08-26)
 *
 * @param buf :��ȡ�����ݴ����
 * @param len :��ȡ�����ֽ���
 */
void ReadRam( BYTE *buf,WORD len)
{

#if FLASH_AppRAM
    WORD temp_len;
    DWORD temp_addr;

    if (RamOffSet<FLASH1SIZE)
    {
        FlashOpt = 0;
        if ((RamOffSet + len)<=FLASH1SIZE)
            SSTF080B_RD(RamOffSet, len, buf);
        else
        {
            temp_len = FLASH1SIZE-RamOffSet;
            SSTF080B_RD(RamOffSet, temp_len, buf);
            FlashOpt = 1;
            SSTF080B_RD(0,  len-temp_len,  &buf[temp_len]);//second flash
        }
    }
    else if (RamOffSet<FLASHTOTAL)                           //second flash
    {
        FlashOpt = 1;
        temp_addr = RamOffSet - FLASH1SIZE;
        if ((RamOffSet + len)>FLASHTOTAL)
        {
            len = FLASHTOTAL-RamOffSet;
        }
        SSTF080B_RD(temp_addr, len, buf);
    }
#else
    //if ((RamOffSet + len)<=ApplVar.SIZE_EXTRAM)
        SRAM_ReadBuffer(buf,RamOffSet, len);

#endif
    RamOffSet += len;
}
/**
 * ����RamOffSet��ָ���ĵ�ַ,���ļ����ݴ洢����д��len�ֽڵ�����
 * д�����֮��,RamOffSet�Զ�+len
 * @author EutronSoftware (2014-08-26)
 *
 * @param buf :Ҫд�������
 * @param len :�����ֽ���
 */
void WriteRam( BYTE *buf, WORD len)   /* (void *from, WORD bytes) write bytes to ram banks */
{
#if FLASH_AppRAM
    WORD temp_len;
    DWORD temp_addr;

    if (RamOffSet<FLASH1SIZE)
    {
        FlashOpt = 0;
        if ((RamOffSet + len)<=FLASH1SIZE)
            WriteBlock(RamOffSet, buf, len);
        else
        {
            temp_len = FLASH1SIZE-RamOffSet;
            WriteBlock(RamOffSet, buf, temp_len);
            FlashOpt = 1;
            WriteBlock(0, &buf[temp_len], len-temp_len);   //second flash
        }
    }
    else if (RamOffSet<FLASHTOTAL)                           //second flash
    {
        FlashOpt = 1;
        temp_addr = RamOffSet - FLASH1SIZE;
        if ((RamOffSet + len)>FLASHTOTAL)
        {
            len = FLASHTOTAL-RamOffSet;
        }
        WriteBlock(temp_addr, buf, len);
    }
#else
//    if ((RamOffSet + len)<=ApplVar.SIZE_EXTRAME)
        SRAM_WriteBuffer(buf, RamOffSet, len);
#endif
    RamOffSet += len;
}
/****************************************************************************/



void DrawerOpen(void)//��Ǯ��
{
    DRAWER_OPEN();
    DrawerOpen_time = 5;
}


//Bios���ṩ�Ĺ����о�����
//�п���ȫ���о�Ӧ�ó�����ʹ�õ��Ĺ���.���Ӧ�ó���ʹ�����ڴ�δ�г��Ĺ���,
//��ô��Ӧ��Biosģ�齫���ӡCommand XX��ʾ,��ʱӦ��ΪXX�������Ӧ�ĳ���.
WORD Bios(WORD cmd, void  *ptr, WORD par1, WORD par2)
{
    BYTE i, BCount;
    DWORD sect;
    BYTE aBuf[3];
    switch (cmd)
    {
    case BiosCmd_CC_Compare://
        //(BOOL)Result = Bios(BiosCmd_CC_Compare, (char*)SourceBuff, (WORD)ChipAdrress, (WORD)NumBytes )
        //�Ƚ�SourceBuff�е������Ƿ���IC����ַChipAdrress�г���ΪNumBytes�������Ƿ�һ��,
        break;
    case BiosCmd_CC_Read:
        //(BOOL)Result = Bios(BiosCmd_CC_Read, (char*)DestBuff, (WORD)ChipAdrress, (WORD)NumBytes )
        //��IC����ַChipAdrress�ж�ȡ����ΪNumBytes�����ݴ���DestBuff
        return 0;
    case BiosCmd_CC_Write:
        //(BOOL)Result = Bios(BiosCmd_CC_Write, (far char*)SourceBuff, (WORD)ChipAdrress, (WORD)NumBytes )
        //��SourceBuff�е�����д��IC����ַChipAdrress��
        return 0;
#if (defined(DD_MMC) || defined(CASE_FATFS_EJ))
    case BiosCmd_SD_Erase:
        //Bios(BiosCmd_SD_Erase, (far char*)SourceBuff, (WORD)GROUP_L, (WORD)GROUP_H )
        //����MMC������GROUP_H*65536+GROUP_L��ָ�����������, MMC����Ļ�����Ҫ�μ�����ֲ�
//        if(SD_Detect()!= SD_PRESENT)
//            return (SD_Erase(((uint32_t)par1<<16)+par2,((uint32_t)par1<<16)+par2+SDBLOCKSIZE)==SD_OK);
//        else
            return false;
    case BiosCmd_SD_ReadInfo:
        //Bios(BiosCmd_SD_ReadInfo, (char*)sBuf, MMC_SEND_CID,16)
        //Bios(BiosCmd_SD_ReadInfo, (char*)sBuf, MMC_SEND_CSD,16)
        //��ȡMMC����CID��CSD,����sBuf��
//ccr2017-06-15>>>>>>>>>>>>>>>>>>>>>
#if (1)
        if((SD_Detect()!= SD_PRESENT) || SD_GetCardInfo(&SDCardInfo) != SD_OK)
             return false;
        if (ptr)
        {
            if(par1 == MMC_SEND_CID)
            {
                memcpy(ptr,(char*)&SDCardInfo.SD_cid,16);
            }
            else if(par1 == MMC_SEND_CSD)
            {
                memcpy(ptr,(char*)&SDCardInfo.SD_csd,16);
            }
        }
#endif
        return true;

    case BiosCmd_SD_UnProtect:
        //Bios(BiosCmd_SD_UnProtect, (far char*)SourceBuff, (WORD)GROUP_L, (WORD)GROUP_H )
        //ȡ����MMC������GROUP_H*65536+GROUP_L��ָ������, MMC����Ļ�����Ҫ�μ�����ֲ�
        break;
    case BiosCmd_SD_Init:
        //Bios(BiosCmd_SD_Init, (far char*)SourceBuff, 0, 0 )
        //��ʼ��MMC��, MMC�ĳ�ʼ��������ֲ�

#ifndef SPI_SD
        NVIC_SDConfiguration();
#endif
        if (SD_Detect()== SD_PRESENT)
        {
            //SD_PWR_ON();
            return (SD_Init () == SD_OK);
        }
        else
            return false;
    case BiosCmd_SD_ReadBlock:
        //Bios(BiosCmd_SD_ReadBlock, buf,addr_L,  addr_H)
        //��MMC���е�addr_h*65536+addr_L��ַ�ж�ȡһ������,����buf��,ͨ��һ��Ϊ512�ֽ�
//        return  SRAM23K256_RD(((par2 << 16) | par1)+0x4000, 512,(BYTE *)ptr);//testonly
        sect = ((par2 << 16) | par1);
        return SD_ReadMultiBlocks((BYTE *)ptr,sect,  SDBLOCKSIZE,2)==SD_OK;
    case BiosCmd_SD_WriteBlock:
        //Bios(BiosCmd_SD_WriteBlock, buf,addr_L,  addr_H)
        //��buf�е�һ������д���MMC���е�addr_h*65536+addr_L��ַ��,ͨ��һ��Ϊ512�ֽ�
//        return SRAM23K256_WR(((par2 << 16) | par1)+0x4000, (BYTE *)ptr, 512);//testonly
        sect = ((par2 << 16) | par1);
        return (SD_WriteMultiBlocks((BYTE *)ptr, sect,  SDBLOCKSIZE,2)==SD_OK);
    case BiosCmd_SD_CheckPresence:
        //Bios(BiosCmd_SD_CheckPresence, 0, 0, 0)
        //���MMC���Ƿ����
        return (SD_Detect()==SD_PRESENT);

#endif
    case BiosCmd_PrintGraph:
        //Bios(BiosCmd_PrintGraph, (char*)idx, option, position);
        //��ӡ��idx��ָ����ͼƬ
        //option:   bit0  =1,��GrafTexts�е����ֵ��ӵ�ͼƬ��
        //          bit1  =1,ʹ��ͼƬ�Դ������ֵ��ӵ�ͼƬ��
        //position:ָ��ͼƬ��ӡ���ַ�λ��

        //if (!MyFlags( CLOSEPRINT))
				if (!MyFlags( ENPRINTER))//jdb2018-09-02
        {
            aBuf[0] = (BYTE)ptr;//get the graphic index
            sect=aBuf[0]-1;
            BCount = 2;
            if (par1 & 0x01)
            {
                if (ApplVar.GrafTexts[0][0] == 0)
                    aBuf[1] = '0';//û����Ҫ���ӵ�����
                else
                {
                    if (par2 && par2<=PRTLEN)
                    {//ָ����ӡλ��
                        aBuf[2] = par2;
                        BCount=3;
                    }
                    aBuf[1] = '1';//�����ֵ�����ͼƬ��
                    RSPrint(aBuf, BCount, CMDP_PRGRA);
                    for (i=0; i<TEXTSOFGRAPH && ApplVar.GrafTexts[i][0]; i++)
                        RSPrint(ApplVar.GrafTexts[i], PRTLEN-1, CMDP_R);
                    aBuf[0] = 0;
                    RSPrint(aBuf, 1, CMDP_R);
                    return true;
                }

            }
            else if (par1 & 0x02)
            {//ʹ��ͼƬ�Դ������ֵ��ӵ�ͼƬ��
                aBuf[1] = '2';
            }
            if (par2 && par2<=PRTLEN)
            {//ָ����ӡλ��
                aBuf[2] = par2;
                BCount=3;
            }
            else if (aBuf[0] && aBuf[0]<=GRAPHICMAX &&
                     ((aBuf[1]!='2') || (gra_info[sect]->cLabels==0)) &&
                     gra_info[sect]->cPrintAt==0)
            {//ccr2017-08-09>>>ͼƬ�Զ����д�ӡ>>
                aBuf[2]=(PRTLEN-gra_info[sect]->cGrapCols)/2;
                BCount=3;
            }//ccr2017-08-09<<<<<<<<<<<<<<<<<<<<<<<
            RSPrint(aBuf, BCount, CMDP_PRGRA);
        }
        return true;
    case BiosCmd_PrintRestart:
        //����������ӡ,M3ϵͳ��û��ʹ��
        break;
    case BiosCmd_PrintX:
        //Bios(BiosCmd_PrintX, char *Line, WORD Cmd , WORD DotLinesBlank);
        //���ӡ�����ʹ�ӡ����,LineΪҪ��ӡ������;CmdΪ��ӡ����;DotLinesBlankΪ��ӡ��ɺ����ֽ����
        //Cmd�������¿����ĳ�������������
        if (ptr)
        {
            BCount = strlen(ptr);
            if (BCount > PRTLEN)
                BCount = PRTLEN;
        }
        else
            BCount = 0;

        //if (!MyFlags( CLOSEPRINT))
				if (!MyFlags( ENPRINTER))//jdb2018-09-02
            RSPrint((BYTE *)ptr, BCount, par1);//����Ҫ��ӡ��������Ҫ����
        return true;

    case BiosCmd_PrintDirect://Ŀǰ�Ĵ�ӡ���������޴˹���
        //Bios(BiosCmd_PrintDirect, char *Line, WORD Cmd , WORD DotLinesBlank);
        //���ӡ�����ʹ�ӡ����,LineΪҪ��ӡ������;CmdΪ��ӡ����;DotLinesBlankΪ��ӡ��ɺ����ֽ����
        //Cmd�������¿����ĳ�������������
        //ʹ��BiosCmd_PrintDirect����ӡʱ,��ӡ���ݲ������ӡ�����,����ֱ�Ӵ�ӡ
        break;
    default://����������������,����������δ����������ʱ,��ʾ�������,Ȼ���ٿ�����Ӧ����
        USART_printf("Command %d\n",cmd);
        break;
    }
    return 0;
}


WORD  Bios_1(WORD cmd)
{
    volatile RTC_DateTypeDef  RTC_DateStruct;
    volatile RTC_TimeTypeDef  RTC_TimeStruct;

    switch (cmd)
    {
    case BiosCmd_PrintCheck:
        //Bios_1(BiosCmd_PrintCheck);
        //��ȡ��ӡ����״̬:ȱֽ/ѹ��̧��/���ȵ�
	Get_Printer_status();
        return CLONG(Printer_Status);

    case BiosCmd_ResetCutter:
        //Bios_1(BiosCmd_ResetCutter)
        //��λ��ӡ���е�
        //if (!MyFlags( CLOSEPRINT))
				if (!MyFlags( ENPRINTER))//jdb2018-09-02
            RSPrint(0, 0, PRN_CUTRESET_Mode);
        break;
    case BiosCmd_RestDisp0:
        //Bios_1(BiosCmd_RestDisp0);
        //������ʾ��ʾ��Ļ������,ͨ�����ڹػ����ٿ���ʱ��ʾ�ػ�ǰ�������Ļ����
#if !defined(CASE_ASCIIDISPLAY)
#ifndef CASE_MCR30
        LED_On();
        PutsO_Saved();
#else
				//
				//
#endif
#endif
        break;
    case BiosCmd_SaveDisp0:
        //������Ļ�ϵ���ʾ����.
        //��:���տ����ʾ����������Ϣ,ͻȻ��������ʱ,�Ƚ���ǰ���ݱ���,Ȼ������ʾ������Ϣ
        break;
    case BiosCmd_ClrFip0:
#if !defined(CASE_ASCIIDISPLAY)
#if defined(CASE_ECR99) || defined(CASE_ECR100F) || defined(CASE_ECR100S)
        //���LED��ʾ
        LED_Off();
#else
				//
				//
#endif
#endif
        return 0;

    case BiosCmd_AutoClickOff:
        //�رհ����Զ���������
        EnableBeep(false);
        break;
    case BiosCmd_AutoClickOn:
        //�򿪰����Զ���������
        EnableBeep(true);
        break;
    case BiosCmd_CC_Close:
        //�ر�IC��,���ر�IC����������Դ
        return 0;
    case BiosCmd_BuzzL:
        Beep(3);
        break;
    case BiosCmd_BuzzM:
        Beep(2);
        break;
    case BiosCmd_BuzzS:
        //�ֱ�Ϊ��/��/��ʱ�����
        Beep(1);
        break;
    case BiosCmd_GetDate:
        //��ȡ���ڵ�rtc_date��,���ڸ�ʽΪ
        RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);

        rtc_date.dd = DECtoBCD(RTC_DateStruct.RTC_Date);
        rtc_date.mm = DECtoBCD(RTC_DateStruct.RTC_Month);
        rtc_date.yy = DECtoBCD(RTC_DateStruct.RTC_Year);
        rtc_date.w = DECtoBCD(RTC_DateStruct.RTC_WeekDay);
        return 1;

    case BiosCmd_GetTime:
        //��ȡʱ�䵽rtc_time��,ʱ���ʽΪ:
        RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);
        rtc_time.hh = DECtoBCD(RTC_TimeStruct.RTC_Hours);
        rtc_time.mm = DECtoBCD(RTC_TimeStruct.RTC_Minutes);
        rtc_time.ss = DECtoBCD(RTC_TimeStruct.RTC_Seconds);
        return 0;

    case BiosCmd_SetDate:
        //����rtc_date�е�������������
        RTC_DateStruct.RTC_Year        = BCDtoDEC(rtc_date.yy);
        RTC_DateStruct.RTC_Month       = BCDtoDEC(rtc_date.mm) ;
        RTC_DateStruct.RTC_Date        = BCDtoDEC(rtc_date.dd);
        RTC_DateStruct.RTC_WeekDay     = BCDtoDEC(rtc_date.w);

        RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct);
        return 0;
    case BiosCmd_SetTime:
        //����rtc_time�е�ʱ������ʱ��
        RTC_TimeStruct.RTC_Hours       = BCDtoDEC(rtc_time.hh);
        RTC_TimeStruct.RTC_Minutes     = BCDtoDEC(rtc_time.mm) ;
        RTC_TimeStruct.RTC_Seconds     = BCDtoDEC(rtc_time.ss) ;
        RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct) ;
        return 0;
    case BiosCmd_CheckKeyborad:

        //�ж��Ƿ��а�������,���ǲ���ȡ��ֵ��1���м����£�0�� �޼�����
        return CheckKeyboard();

    default://����������������,����������δ����������ʱ,��ʾ�������,Ȼ���ٿ�����Ӧ����
        USART_printf("Command_1 %d\n",cmd);
        break;
    }
    return 0;
}

WORD Bios_2(WORD cmd, void  *ptr)
{

    switch (cmd)
    {
    case BiosCmd_CC_Open:
        //��IC��,����ͨIC���ĵ�Դ,ͬʱ��ȡIC����4�ֽ�����,���سɹ����
        break;
    case BiosCmd_CC_VerifyPsw:
        //У��IC������.���سɹ����
        break;
    case BiosCmd_CC_WritePsw:
        //�޸�IC������,���سɹ����
        break;
    default://����������������,����������δ����������ʱ,��ʾ�������,Ȼ���ٿ�����Ӧ����
        USART_printf("Command_2 %d\n",cmd);
        break;
    }
    return 0;
}

WORD Bios_SetKeyboard(BYTE Code, WORD NumKeys, CONSTBYTE *TabPnt, BYTE TabRecLen)
{//���¶������
    return 0;
}

WORD Bios_SetCutter(BYTE Stato)
{//���е�,Stato=0:�ر��е�;Stato=1:���е�
    return 0;
}



void SetRTS(BYTE port)
{//���ô���port��RTS
 //0-����1,1-����2.....
    if (port == 0)
        UART0_RTS_SET();
}

//set RTS to 'L' for Serial 1,2
void ClrRTS(BYTE port)
{//������ڵ�RTS
 //1-����1,2-����2.....
    if (port == 1)
        UART0_RTS_CLR();
}

// ��ȡ����CTS״̬,status=0��ʾ���账������״̬
BYTE GetCTS(BYTE port)
{
//1-����1,2-����2.....
    int i;
    BYTE status[2];

    status[0]=status[1]=0;

    do
    {
        for (i=0;i<2;i++)
        {
            if (i)
                for (status[1]=0;status[1]<10;status[1]++);
            if (port == 1)
                status[i] = UART0_CTS_GET();
        }

    } while (status[0]!=status[1]);
    return(status[0]);
}



WORD Bios_SetTic(WORD Interval)
{//��
    return 0;
}

WORD Bios_SetPortBusy(BYTE NumCom, BYTE Satato, BYTE ChrRx, BYTE ChrTx)
{//��
    return 0;
}

WORD Bios_TestMAC(void)
{//����MAC������״̬

    int sMACCount=0;
    ULONG t1,t2;

    do
    {
        t1 = (MAC_PORT->IDR & MAC_PIN);
        usDelay(1000);//���1ms���һ��
        t2 = (MAC_PORT->IDR & MAC_PIN);
        if (t1==t2)
		{
            sMACCount++;
			if (t1 && sMACCount>2)
				break;//������Ϊδ�̽�MAC,ֻ�������,�ӿ쿪���ٶ�
		}
        else
            sMACCount = 0;

    } while (sMACCount<50);//�����50��

    if (!t1)
        return true;  // MAC on
    else
        return false;  // MAC OFF
}


WORD Bios_FM_BlankCheck(UnLong SrcAdr, WORD Num)
{//���˰�ش洢����ַSrcAdr�е�Num�ֽ��Ƿ�Ϊ��
#if 0
    BYTE temp_checkBuf[Num];
    BYTE i;
    M25P80_RD(SrcAdr, Num, temp_checkBuf);
    for (i=0; i<Num; i++)
    {
        if (temp_checkBuf[i] != 0xFF)
            return 0;
    }
#endif
    return 1;
}
//////

BYTE Bios_FM_CheckPresence(void)
{//���˰�ش洢���Ƿ����
    return FMDetect();
}


#define FIRMWARE_ADDR  ((uint32_t)0x08000000L)    //����ռ��ַ
#define FIRMWARE_SIZE  ((uint32_t)0x80000L)      //����ռ��С,ʵ�ʴ�СΪ0x80000,�����С������0x60000

WORD CheckSum(void)
{//���������У���

    WORD checksum=0;
    uint32_t TempAddr;

    for (TempAddr = 0; TempAddr<FIRMWARE_SIZE; TempAddr+=2) /* while there is data to write */
    {
        /* Transfer data to the memory */
        checksum += *(WORD *) (FIRMWARE_ADDR + TempAddr);
    }

    return checksum;
}

long TestRam(void)
{//���RAM

    return 0x20000l;//sizeof the RAM in MCU
}
//���ֽڲ�����ʽ������չRAM�Ķ�д��ȷ��
long TestLMM(WORD lmmPage)
{

    BYTE temp_save,temp_check;
    long i,SizeOfSRAM;
    BYTE *pSRAM;

    SizeOfSRAM=ApplVar.SIZE_EXTRAM;
    if (SizeOfSRAM)
    {//װ����SRAM,ÿ���ڴ�256K
        pSRAM = (BYTE *)SRAM_ADDR;
        for (i=0;i<SizeOfSRAM;i++)
        {
            temp_check = 0x55;
            temp_save = *pSRAM;
            *pSRAM = temp_check;
            temp_check = *pSRAM;

            if (temp_check != 0x55)
            {
                *pSRAM = temp_save;
                return(-(i+SRAM_ADDR));
            }
            temp_check = 0xAA;
            *pSRAM = temp_check;
            temp_check = *pSRAM;
            if (temp_check != 0xAA)
            {
                *pSRAM = temp_save;
                return(-(i+SRAM_ADDR));
            }
            *pSRAM++ = temp_save;
        }
        return SizeOfSRAM;
    }
   return 0;

}
//
//
/*******************************************************************************
 * ����ⲿRAM(���籣��RAM)Ӧ�����ݴ洢��,ͬʱ��EXTRAMN�б�ʾSRAM����(256K/��),
 * ��ApplVar.SIZE_EXTRAM�м�¼SRAM��С
 *
 * @author EutronSoftware (2018-01-24)
 *******************************************************************************/
void ClsXRam(void)
{
#if FLASH_AppRAM
    FlashOpt = 0;
    SSTF080B_EraseChip();
    FlashOpt = 1;
//testonly	SSTF080B_EraseChip();
    FlashErase = 0;
    ApplVar.SIZE_EXTRAM=0x100000L;    //size of flash
    EXTRAMN |= (ApplVar.SIZE_EXTRAM>>18);   //��ʾ��һ��SRAM
#else

//    WORD *pSRAM=(WORD*)SRAM_ADDR;

    EXTRAMN &= 0xf0;
    //����һ��SRAM�Ƿ����

//    *pSRAM = 0x55AA;

//    if (*pSRAM == 0x55AA)//����Ƿ�����չSRAM
    {
#if defined(DEBUGBYPC)
        ApplVar.SIZE_EXTRAM=0x80000L;
#else
        ApplVar.SIZE_EXTRAM=SRAMSIZE;//testonly SRAM_Detect();
#endif
        if (ApplVar.SIZE_EXTRAM)
        {
            EXTRAMN |= (ApplVar.SIZE_EXTRAM>>18);   //��ʾSRAM����(256K/ÿ��)

            //�����չRAM�е�����
#if defined(ApplVarInSRAM)
            memset((void*)(SRAM_ADDR+sizeof(WORD)+sizeof(ULONG)+sizeof(struct APPTEXT)+sizeof(struct APPLICATION)),0,
                   ApplVar.SIZE_EXTRAM-(sizeof(WORD)+sizeof(ULONG)+sizeof(struct APPTEXT)+sizeof(struct APPLICATION)));
#else
            memset((void*)SRAM_ADDR,0,ApplVar.SIZE_EXTRAM);
#endif
        }
        else
        {//����չ�ڴ�,ϵͳ�޷�����
            while (KbHit()) Getch();//����Ѱ���
            while (!KbHit())
            {
                Beep(3);
#if defined(CASE_ASCIIDISPLAY)
                PutsO(MsgSRAMNOTFOUND);
#else
                mClearScreen();
                Delay(500);
                //DispStrXY(MsgSRAMNOTFOUND,0,0);
							  DispStrXY("no SRAM!HALT!",0,0);//jdb2018-09-02
                Delay(500);
#endif
            }
        }
    }

#endif
}



short Bell(short len)
{//����len�ֱ����峤/��/��ʱ��
    Beep(len);
    return 0;
}


WORD CC_Insert(void)
{//���IC���Ƿ����
//  return 1;//����
    return 0;//û�в忨
}


//------------------------------------------------------------------



void CutRPaper(short lines)
{//��ӡ����ֽ
#if DD_CUTTER
    if (CUTTER & 0x03)
    {
        RFeed(lines);
        if (!MyFlags( ENPRINTER))
            RSPrint(0, 0, PRN_CUTF_Mode);
    }
#endif
}


///////////////////////////////////////////////////////////////////////////////////////////////

#define TIMER_INTERVAL 	500

/**********************************************************************************************/

/* Uncomment the corresponding line to select the RTC Clock source */
#define RTC_CLOCK_SOURCE_LSE   /* LSE used as RTC source clock */
/* #define RTC_CLOCK_SOURCE_LSI */ /* LSI used as RTC source clock. The RTC Clock
                                      may varies due to LSI frequency dispersion. */

__IO uint32_t AsynchPrediv = 0, SynchPrediv = 0;


#if !defined(DEBUGBYPC)

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in 1 ms.
  * @retval None
  */
void msDelay(__IO uint32_t nTime)
{
    ULONG   oldSysTime=GetSystemTimer();
    while ((GetSystemTimer()-oldSysTime)<nTime)
    {
    }
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in 1 us.
  * @retval None
  */
void usDelay(__IO uint32_t nTime)
{
    if (nTime)
    {
        TIM5->SR = (uint16_t)~TIM_FLAG_Update;
        TIM5->ARR = uSECOND * nTime ;
        TIM_Cmd(TIM5, ENABLE);
        while (!(TIM5->SR & TIM_FLAG_Update))
        {
        };
        TIM_Cmd(TIM5, DISABLE);

    }
}

/*---------------------------------------------------------*/
/* User Provided Timer Function for FatFs module           */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called from     */
/* FatFs module. Any valid time must be returned even if   */
/* the system does not support a real time clock.          */
/* This is not required in read-only configuration.        */
DWORD get_fattime ()
{
    RTCTime rtc;

    /* Get local time */
    RTC_GetDateTime(&rtc);

    /* Pack date and time into a DWORD variable */
    return((DWORD)(rtc.year - 1980) << 25)
    | ((DWORD)rtc.mon << 21)
    | ((DWORD)rtc.mday << 16)
    | ((DWORD)rtc.hour << 11)
    | ((DWORD)rtc.min << 5)
    | ((DWORD)rtc.sec >> 1);

}
/**
  * @brief  Configure the RTC peripheral by selecting the clock source.
  * @param  None
  * @retval None
  */
void RTC_Config(void)
{
    /* Enable the PWR clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    /* Allow access to RTC */
    PWR_BackupAccessCmd(ENABLE);

#if defined (RTC_CLOCK_SOURCE_LSI)  /* LSI used as RTC source clock*/
/* The RTC Clock may varies due to LSI frequency dispersion. */
    /* Enable the LSI OSC */
    RCC_LSICmd(ENABLE);

    /* Wait till LSI is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
    {
    }

    /* Select the RTC Clock Source */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

    SynchPrediv = 0xFF;
    AsynchPrediv = 0x7F;

#elif defined (RTC_CLOCK_SOURCE_LSE) /* LSE used as RTC source clock */
    /* Enable the LSE OSC */
    RCC_LSEConfig(RCC_LSE_ON);

    /* Wait till LSE is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
    {
    }

    /* Select the RTC Clock Source */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

    SynchPrediv = 0xFF;
    AsynchPrediv = 0x7F;

#else
#error Please select the RTC Clock source inside the main.c file
#endif /* RTC_CLOCK_SOURCE_LSI */

    /* Enable the RTC Clock */
    RCC_RTCCLKCmd(ENABLE);

    /* Wait for RTC APB registers synchronisation */
    RTC_WaitForSynchro();

    /* Enable The TimeStamp */
    //ouhs 20171213 RTC_TimeStampCmd(RTC_TimeStampEdge_Falling, ENABLE);
}

/*********************************************************************************************************
** ʹ��RTC�����ж�
**
*********************************************************************************************************/
void RTC_AlarmSet(void)
{
	RTC_AlarmTypeDef  RTC_AlarmStructure;
	RTCTime rtc;

	//�ȶ�ȡʱ��
	RTC_GetDateTime(&rtc);

	//������--����--������
  RTC_AlarmCmd(RTC_Alarm_A, DISABLE);

  RTC_AlarmStructure.RTC_AlarmTime.RTC_H12     = RTC_H12_AM;  //RTC����ΪRTC_HourFormat_24�� AM��PM�������Ƿ��Զ���Ч��
  RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = rtc.hour;
  RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = rtc.min;
  RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = rtc.sec;
	RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_None;

	RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;

	//����24Сʱ���ӣ���Ҫ���Ǵ�С�¡�ƽ�������2��
	if((rtc.mday==31) || ((rtc.mday==30)&&((rtc.mon==4)||(rtc.mon==6)||(rtc.mon==9)||(rtc.mon==11))))
	{
		RTC_AlarmStructure.RTC_AlarmDateWeekDay = 1;
	}
	else if((rtc.mday==28)&&(rtc.mon==2))	//2��28��
	{
		if(rtc.year%4==0) //����
		{
			RTC_AlarmStructure.RTC_AlarmDateWeekDay = rtc.mday+1;
		}
		else	//ƽ��
		{
			RTC_AlarmStructure.RTC_AlarmDateWeekDay = 1;
		}
	}
	else if((rtc.mday==29)&&(rtc.mon==2))	//2��29��
	{
		RTC_AlarmStructure.RTC_AlarmDateWeekDay = 1;
	}
	else
		RTC_AlarmStructure.RTC_AlarmDateWeekDay = rtc.mday+1;


  // ����RTC Alarm A�Ĵ���
  RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure);

  RTC_ITConfig(RTC_IT_ALRA, ENABLE);

  RTC_AlarmCmd(RTC_Alarm_A, ENABLE);

	//�����־λ
  RTC_ClearFlag(RTC_FLAG_ALRAF);
  EXTI_ClearITPendingBit(EXTI_Line17);
}

/*********************************************************************************************************
** �����ڿ���ʱ��ѯһ������״̬������ֹ�ڹػ�״̬�´����ʱִ�е�����
**
*********************************************************************************************************/
uint8_t RTC_AlarmStatus(void)
{
	if(RTC_GetITStatus(RTC_IT_ALRA) != RESET)
  {
    RTC_ClearITPendingBit(RTC_IT_ALRA);
    EXTI_ClearITPendingBit(EXTI_Line17);

		return true;
  }
	else
		return false;
}

/*********************************************************************************************************
** ʹ��RTC�����ж�
**
*********************************************************************************************************/
void RTC_AlarmInit(void)
{
	NVIC_InitTypeDef  NVIC_InitStructure;
  EXTI_InitTypeDef  EXTI_InitStructure;

	//�����ж���
  EXTI_ClearITPendingBit(EXTI_Line17);
  EXTI_InitStructure.EXTI_Line = EXTI_Line17;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  //�����ж����ȼ�
  NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}


/*********************************************************************************************************
** RTC ��ʼ������
**
*********************************************************************************************************/
void RTC_Initial(void)
{
	RTC_InitTypeDef RTC_InitStructure;

	/* ��ʼ��RTC */
	if (RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x32F2)
	{
		/* RTC configuration  */
		RTC_Config();

		/* Configure the RTC data register and RTC prescaler */
		RTC_InitStructure.RTC_AsynchPrediv = AsynchPrediv;
		RTC_InitStructure.RTC_SynchPrediv = SynchPrediv;
		RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;

		/* Check on RTC init */
		if (RTC_Init(&RTC_InitStructure) == ERROR)
		{
			xprintf("RTC Config Err.\n");
		}

		/* Configure the time register */
		//RTC_TimeRegulate();

		/* Indicator for the RTC configuration */
		//RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F2);

//ouhs 20171213 �������>>>>>>>>>>>>>>>>>>>>>
		RTC_AlarmInit();
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	}
	else
	{
		/* Check if the Power On Reset flag is set */
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
		{
			xprintf("Reset occurred.\n");
		}
		/* Check if the Pin Reset flag is set */
		else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
		{
			xprintf("ExReset occurred\n");
		}

		//xprintf("No need to config RTC.\n");//No need to config

		/* Enable the PWR clock */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

		/* Allow access to RTC */
		PWR_BackupAccessCmd(ENABLE);

		/* Wait for RTC APB registers synchronisation */
		RTC_WaitForSynchro();

		/* Clear the RTC Alarm Flag */
		RTC_ClearFlag(RTC_FLAG_ALRAF);

		/* Clear the EXTI Line 17 Pending bit (Connected internally to RTC Alarm) */
		EXTI_ClearITPendingBit(EXTI_Line17);

	}
}

extern void USB_Init(void);
extern void USB_TEST(void);

void STM32_Initial(void)
{
    RCC_ClocksTypeDef RCC_Clocks;

    /******** �����ļ����ѵ���SystemInit()������ƵΪ120MHz*********/


    //RCC_HCLKConfig(RCC_SYSCLK_Div8);

    //RCC_SYSCLKConfig(RCC_SYSCLKSource_HSE);

    /*!< At this stage the microcontroller clock setting is already configured,
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f2xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f2xx.c file
     */

    // �δ�ʱ��SysTickÿ1ms�ж�һ�Σ�120000000/1000*(1/120)us=1ms
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);//1ms for SysTick

    UARTInit(1,115200,8,0,1); 	//��Ӧ�ⲿ����RS232-I
    UARTInit(2,115200,8,0,1);	//��Ӧ�ⲿ����RS232-II
#if USART_EXTEND_EN
	UARTInit(3,115200,8,0,1);	//��Ӧ�ڲ�����(MFRICard)
#endif
	HW_GPIOInit();
    pwr_Init();

    FSMC_Init();

    BKPSRAM_Init();//��ʼ���ڲ�RAM ccr2014-03-25
#if !defined(CASE_ASCIIDISPLAY)
    mInitialLCD();
#endif

		POWER_ON();	//�����ϵ�

		RTC_Initial();
    TIM5_Init();
    KeyBoardInit();

#if defined(FISCAL)
    FM_Init();
#endif

#if defined(CASE_ASCIIDISPLAY)
    LCDInit();
#else
#if defined(CASE_ECR100S)
	HT1621_Init();
	TIM6_Init();
	//LED_Init();
#endif
#endif
	SD_LowLevel_Init();
	/**   NVIC PreemptionPriority���ȼ����䣺SD=0, ��ӡ����ʱ��TIM2��TIM3=1,
	                                        ����=0, ���̶�ʱ���ж�TIM7=3, ���̰����ж�=3,
											�����ж�=0, LED���Զ�ʱ��TIM6=3,
	**/
#ifndef SPI_SD
  NVIC_SDConfiguration();// For SD
#endif

	Beep(1);

#if (1)//defined(CASE_ECR100F) || defined(CASE_ECR100S)	//ECR99��MCR30��Ҫ����ģ��U�̹��ܣ�PC����NAND
	USB_Start();
#endif

#if defined(CASE_ETHERNET)
//ccr�Ƶ�InitApplication֮��	Ethernet_Start();
#endif

#if defined(CASE_GPRS)
#if defined(CASE_ECR99) || defined(CASE_ECR100F) || defined(CASE_ECR100S)  //GSM: EM310
	GPRS_PWR_HIGH();
#else //GSM: SIM800C
	//GPRS_OnOff();
#endif
#endif

	//��ȡһ��RTC��ص�ѹ�����ر�RTC��ѹ����ADC ������ADC���ᵼ��RTC��ص�������26uA
	//����ֻ��¼һ��RTC��ص�ѹ  �Ժ��ٵ���pwrGetStatus��ȡ�Ķ��ǵ�һ�ε�ֵ
	pwrGetStatus(GET_VRTC_STATUS);

#if (0)//defined(CASE_ECR100F) || defined(CASE_ECR100S) || defined(CASE_ECR99)  || defined(CASE_MCR30)
	//����ʱ����
	DaylightSavingTimeSet();
#endif

    RNG_Config();//ccr2017-10-20��ʼ�������������

#if defined(NAND_DISK)	//ouhs20180327 ֻ����NAND Flash�Ļ�����ʹ��,����NAND FTL
		my_mem_init(SRAMIN);
#else
		sf_Init();	//��û��NAND�Ļ����ϣ��ſ���ʹ��SPI Flash
#endif
}

void Beep(uint8_t blen)
{
#if 1//jdb2019-03-08 ȥ��������
#if !defined(FOR_DEBUG)
    BEEP_ON();
    if (blen == 1)
        beep_len = 10;
    else if (blen == 2)
        beep_len = 40;
    else if (blen == 3)
        beep_len = 80;
    else
        beep_len = 10;
#endif
#endif
}

//***************************************************//
void HW_GPIOInit(void)
{

	GPIO_InitTypeDef GPIO_InitStructure;

	// ����GPIO����ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB |
						   RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD |
						   RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG, ENABLE);

	// ����GPIO������������������ٶ�100MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	// ������
	GPIO_InitStructure.GPIO_Pin = BEEP_PIN;
	GPIO_Init(GPIOG, &GPIO_InitStructure);


	// UART0_RTS,UART1_RTS,UART2_RTS
	GPIO_InitStructure.GPIO_Pin = UART0_RTS;
	GPIO_Init(GPIOF, &GPIO_InitStructure);


#ifdef CASE_GPRS
	GPIO_InitStructure.GPIO_Pin = GPRS_PWR | GPRS_RST;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	GPRS_PWR_LOW();
	GPRS_RST_LOW();
#endif


	// Ǯ��
	GPIO_InitStructure.GPIO_Pin = DRAWER_PIN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(DRAWER_PORT, &GPIO_InitStructure);
    DRAWER_OFF();

	// UART0_CTS, UART1_CTS, UART2_CTS
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = UART0_CTS;
	GPIO_Init(GPIOG, &GPIO_InitStructure);


#ifndef CASE_GPRS
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;	//������Ƶˢ����ʱ���п�����ʱ���⵽�ߵ�ƽ
	GPIO_Init(GPIOG, &GPIO_InitStructure);
#endif


	// MAC (Init)
	GPIO_InitStructure.GPIO_Pin = MAC_PIN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(MAC_PORT, &GPIO_InitStructure);

	BEEP_OFF();

}

#endif


/*********************************************************************************************************
** Function name:      TIM7_Init
** Descriptions:      ��ʱ��7��ʼ������,10ms�жϣ����ڷ�������Ǯ�䡢����
*********************************************************************************************************/
void TIM7_Init(void)
{
    uint16_t PrescalerValue = 0;

    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

    /* Compute the prescaler value */
    // ��ʱ��ʱ��Ϊ:60MHz/PrescalerValue  //1MHz
    PrescalerValue = (uint16_t) ((SystemCoreClock / 2) / 1000000) - 1;

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = 9999;    //10ms�ж�һ��
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);

    /* Prescaler configuration */
    TIM_PrescalerConfig(TIM7, PrescalerValue, TIM_PSCReloadMode_Immediate);

    /* Enable the TIM gloabal Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  //ouhs 20140814 =3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* TIM Interrupts enable */
    TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);

    /* Enable counter */
    TIM_Cmd(TIM7, ENABLE);
}


/*********************************************************************************************************
** Function name:      TIM5_Init,������ʱ�ȴ�
** Descriptions:      ��ʱ��5��ʼ������,
*********************************************************************************************************/
void TIM5_Init(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    /* Enable TIM5 clocks */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

    /* TIM5 configuration */

    TIM_TimeBaseStructure.TIM_Period = uSECOND * 5000*1000; //1ms
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
    TIM_ARRPreloadConfig(TIM5,DISABLE);// ��ʹ��Ӱ�ӼĴ��� //
    TIM_OCStructInit(&TIM_OCInitStructure);

    /* Output Compare Timing Mode configuration: Channel1 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
    TIM_OCInitStructure.TIM_Pulse = 0x0;
    TIM_OC1Init(TIM5, &TIM_OCInitStructure);

    /* One Pulse Mode selection */
    TIM_SelectOnePulseMode(TIM4, TIM_OPMode_Single);
    /* Clear TIM5 update pending flags */
    TIM_ClearFlag(TIM5, TIM_FLAG_Update);

    /* TIM5 enable counters */
//  TIM_Cmd(TIM5, ENABLE);

}

/*********************************************************************************************************
** Function name:      TIM6_Init
** Descriptions:      ��ʱ��6��ʼ������1ms�жϣ�����LED���ԣ�LCD���
*********************************************************************************************************/
void TIM6_Init(void)
{
    uint16_t PrescalerValue = 0;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	/* Compute the prescaler value */
	// ��ʱ��ʱ��Ϊ:60MHz/PrescalerValue  //1MHz
    PrescalerValue = (uint16_t) ((SystemCoreClock / 2) / 1000000) - 1;
	/* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = 99999;//100ms    = 999; //1ms�ж�һ��

    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
	/* Prescaler configuration */
    TIM_PrescalerConfig(TIM6, PrescalerValue, TIM_PSCReloadMode_Immediate);

	/* Enable the TIM gloabal Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;  //ouhs 20140814 =2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* TIM Interrupts enable */
	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);

	/* Enable counter */
	TIM_Cmd(TIM6, ENABLE);
}

#if (defined(DD_MMC))
/*****************************************************************
  ��SD���Ŀ��СΪ1024ʱ��ÿ�ζ�дSD��������1024�ֽ�
  ���ǣ�������512�ֽ�Ϊ��ı߽�
*****************************************************************/
//��SD���϶�ȡһ������ ����ĵĴ�СΪSDCardInfo.CardBlockSize
BYTE MMC_ReadBlock(char *buf,DWORD addr)
{
	return (SD_ReadBlock(buf,addr, SDBLOCKSIZE)==SD_OK);
}
//��SD����д��һ�����ݣ���ĵĴ�СΪSDCardInfo.CardBlockSize
BYTE MMC_WriteBlock(char *buf,DWORD addr)
{
	return (SD_WriteBlock((BYTE *)buf, addr, SDBLOCKSIZE)==SD_OK);
}

//ccr2014-08-13>>>>>>>>>>>>>>>>
/*****************************************************************
  ��SD���Ŀ��СΪ1024ʱ��ÿ�ζ�дSD��������1024�ֽ�
  ���ǣ�������512�ֽ�Ϊ��ı߽�
*****************************************************************/
//��SD���϶�ȡһ������ ����ĵĴ�СΪSDCardInfo.CardBlockSize
BYTE MMC_Read2Blocks(char *buf,DWORD addr)
{
	return (SD_ReadMultiBlocks(buf,addr, SDBLOCKSIZE,2)==SD_OK);
}
//��SD����д��һ�����ݣ���ĵĴ�СΪSDCardInfo.CardBlockSize
BYTE MMC_Write2Blocks(char *buf,DWORD addr)
{
    return (SD_WriteMultiBlocks((BYTE *)buf, addr, SDBLOCKSIZE,2)==SD_OK);
}
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#endif

/**
 * =================================================================
 * ���ڲ�BackupSRAM������Ҫ�������´���
 * MCU�ڲ�RAM�����洢���ص��û��Զ���Ҫ��ͼƬ
 */
#define BKPSRAM_SIZE 0x1000

/**Backup_SRAM���ʳ�ʼ��**/
/**
 *
 *
 * @author EutronSoftware (2014-03-25)
 */
void BKPSRAM_Init(void)
{
  //ʹ�ܵ�Դģ��ʱ��
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  //ʹ�ܶԱ�����ķ���
  PWR_BackupAccessCmd(ENABLE);
  //ʹ�ܱ�����SRAM��ʱ��
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);

  //ʹ���ڲ�BackupSRAM��Դ
  PWR_BackupRegulatorCmd(ENABLE);

  //�ȴ���Դ�ȶ�
  while(PWR_GetFlagStatus(PWR_FLAG_BRR) == RESET)
  {
  }
}
/**
 * дBackupSRAM
 *
 * @author EutronSoftware (2014-03-25)
 *
 * @param pBuffer
 * @param WriteAddr:0~4095
 * @param NumHalfwordToWrite
 */
void BKPSRAM_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint32_t NumHalfwordToWrite)
{
    if (NumHalfwordToWrite)
        memcpy((uint8_t *)(WriteAddr+BKPSRAM_BASE),pBuffer,NumHalfwordToWrite);
}

/**
 * ��BackupSRAM
 *
 * @author EutronSoftware (2014-03-25)
 *
 * @param pBuffer
 * @param ReadAddr:0~4095
 * @param NumHalfwordToRead
 */
void BKPSRAM_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumHalfwordToRead)
{
    if (NumHalfwordToRead)
        memcpy(pBuffer,(uint8_t *)(ReadAddr+BKPSRAM_BASE), NumHalfwordToRead);
}

/**
  * @brief  RNG configuration
  * @param  None
  * @retval None
  */
void RNG_Config(void)
{
 /* Enable RNG clock source */
  RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);

  /* RNG Peripheral enable */
  RNG_Cmd(ENABLE);
}


uint32_t GetRandomNumber(void)
{
  uint32_t random32bit = 0;

  /* Wait until one RNG number is ready */
  while(RNG_GetFlagStatus(RNG_FLAG_DRDY)== RESET)
  {
  }

  /* Get a 32bit Random number */
  random32bit = RNG_GetRandomNumber();

  return (random32bit);
}

