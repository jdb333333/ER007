
#include <stdint.h>
#include "king.h"
#include "exthead.h"
//#include "exth.h"

#include "stm32f2xx.h"
#include "TypeDef.h"
#include "lcd_st7567.h"
#include "flashb.h"

//#define  MMD_HALUO  1 //ccr2015-05-08��

#define Y_Height_C	16				// ����߶�
#define X_Width_C		8				// ������С���

//uint8_t ActiveLines = SCREENLN;

uint8_t DotBuffer_C[4][128]; //page addr:8  column addr:128
const uint8_t pagemap_c[4] = {3, 2, 1, 0,};
#define PAGE_NUM	4

extern uint8_t SaveDispC[2][SCREENWD];	//������ʾ����

extern const uint8_t ASCIITAB_8x16[];
extern const uint8_t RByte[256];

void ST7565R_WR_CMD_C(uint8_t cmd);
void ST7565R_WR_DATA_C(uint8_t dat);
void st7565SetXY_C(uint8_t pos_x,uint8_t pos_y);
void st7565DrawPixel( uint8_t x, uint8_t y );
void st7565ClearPixel( uint8_t x, uint8_t y );
void st7565Refresh_C(void);


/**************************************************************************/
/* ��ʼ������ģ��
***************************************************************************/
extern void SysDelay_ms(uint16_t delay_ms);
void cInitialLCD(void)
{

	//ST7565R_IO_Init();

	ST7565R_WR_CMD_C(ST7565R_CMD_RESET);
	ST7565R_NOP();

	ST7565R_WR_CMD_C(ST7565R_CMD_ADC_NORMAL);
	ST7565R_NOP();
	
	ST7565R_WR_CMD_C(ST7565R_CMD_NORMAL_SCAN_DIRECTION);
	ST7565R_NOP();

	ST7565R_WR_CMD_C(ST7565R_CMD_LCD_BIAS_9);
	ST7565R_NOP();

	ST7565R_WR_CMD_C(ST7565R_CMD_POWER_CTRL_ALL_ON);
	ST7565R_NOP();

	ST7565R_WR_CMD_C(ST7565R_CMD_VOLTAGE_RESISTOR_RATIO_2/*ST7565R_CMD_VOLTAGE_RESISTOR_RATIO_4*/);
	ST7565R_NOP();

	ST7565R_WR_CMD_C(ST7565R_CMD_ELECTRONIC_VOLUME_MODE_SET);
	ST7565R_WR_CMD_C(ST7565R_CMD_ELECTRONIC_VOLUME(0x32/*0x0C*/));
	ST7565R_NOP();

	ST7565R_WR_CMD_C(ST7565R_CMD_BOOSTER_RATIO_SET);
	ST7565R_WR_CMD_C(ST7565R_CMD_BOOSTER_RATIO_5X);
	ST7565R_NOP();
	
	ST7565R_WR_CMD_C(ST7565R_CMD_DISPLAY_ON);
	ST7565R_NOP();
	
	cSetContrast(0);
#if !defined(OD_4ROW)
	LCDSet_Cust(1);//������������ʾ��ͬ������
#endif	
}

/**************************************************************************/
/* д����  ����
***************************************************************************/
void ST7565R_WR_CMD_C(uint8_t cmd)
{

	ST7565R_CSC_LOW();
	ST7565R_A0_CMD();
	ST7565R_SendData(cmd);
	ST7565R_CSC_HIGH();
}

/**************************************************************************/
/* д����  ����
***************************************************************************/
void ST7565R_WR_DATA_C(uint8_t dat)
{
	ST7565R_CSC_LOW();
	ST7565R_A0_DATA();
	ST7565R_SendData(dat);
	ST7565R_CSC_HIGH();
}

/*************************************************************************
 ������ʾ�Աȶ�  contrast��0-->6,7��΢��
***************************************************************************/
void cSetContrast(uint8_t contrast)
{//������ʾ�Աȶ�
	
	contrast += 0x31;//contrast *= 4;// (ApplVar.AP.Flag[CONTRAST] & 0x0f) * 5;
	ST7565R_WR_CMD_C(ST7565R_CMD_ELECTRONIC_VOLUME_MODE_SET);
	ST7565R_WR_CMD_C(ST7565R_CMD_ELECTRONIC_VOLUME(contrast));
}


/**************************************************************************/
/* �����Ļ����
***************************************************************************/
void cClearScreen(void)
{
	uint8_t p, pn;
	uint16_t c;

	pn = PAGE_NUM;
	//memset(ApplVar.ScreenMap,' ',sizeof(ApplVar.ScreenMap));

	for(p=0; p<pn; p++)
	{
		st7565SetXY_C(0, pagemap_c[p]);
		for(c = 0; c < 129; c++)
		{
			ST7565R_WR_DATA_C(0x0);
		}
	}
	memset(DotBuffer_C,0,pn*128);//���buffer����
//	GotoXY(0,0);

}

/**************************************************************************/
/* ������ʾλ�� x=(0..127), y=(0..63)
***************************************************************************/
void st7565SetXY_C(uint8_t pos_x,uint8_t pos_y)
{
    if (pos_x) pos_x++;//ccr2016-05-11�����2���ַ������������????????
	ST7565R_WR_CMD_C(ST7565R_CMD_PAGE_ADDRESS_SET(pos_y));				// ҳ��ַ
	ST7565R_WR_CMD_C(ST7565R_CMD_COLUMN_ADDRESS_SET_MSB(pos_x >> 4));  	// �е�ַ��4λ
	ST7565R_WR_CMD_C(ST7565R_CMD_COLUMN_ADDRESS_SET_LSB(pos_x & 0x0f));	// �е�ַ��4λ
}

/**************************************************************************/
/* ��DotBuffer��ָ����LinNum(0~3),��ColNumλ�õ��ַ�����������ʾ����Ļ��,
���ã�������ָ��λ��һ���ַ�ʱʹ��
hzFlag:=0,��ʾascii�ַ�;=1,��ʾ�����ַ�
***************************************************************************/
void cDrawChar(uint8_t ColNum, uint8_t LinNum,uint8_t hzFlag)
{
	uint8_t j;
	uint8_t *pDot;

	ColNum = ColNum*X_Width_C;
	LinNum *= 2;
#if defined(CASE_FORHANZI)
	if(hzFlag)	 // ����ģʽ��ʾΪ16λ���
		hzFlag = 16;
	else
#endif
		hzFlag = 8;

	//��ʾ�ϰ벿��
	pDot = &DotBuffer_C[LinNum][ColNum];
	if(ColNum==0)
		st7565SetXY_C(ColNum, pagemap_c[LinNum]);
	else//ouhs20170918 ��֪ԭ�򣬴ӵڶ����ַ���ʼλ������ƫ��һ�㣬���Գ�������Ҫ������һ�������ȷ
		st7565SetXY_C(ColNum-1, pagemap_c[LinNum]);  

	for(j = 0; j < hzFlag; j++)
		ST7565R_WR_DATA_C(*pDot++);

	//��ʾ�°벿��
	LinNum++;
	pDot = &DotBuffer_C[LinNum][ColNum];
	if(ColNum==0)
		st7565SetXY_C(ColNum, pagemap_c[LinNum]);
	else//ouhs20170918 ��֪ԭ��ͬ��
		st7565SetXY_C(ColNum-1, pagemap_c[LinNum]);  
	for(j = 0; j < hzFlag; j++)
		ST7565R_WR_DATA_C(*pDot++);

    //RESETBIT(ApplVar.ScreenStat,CRITICAL);

}

/**************************************************************************/
/* ��DotBuffer��ָ����LinNum(0~3)���ַ�����������ʾ����Ļ��,
   ������Ļ�ϵĵ�LinNum��������ʾһ��,���ã���Ҫ����ָ���ж���ַ�ʱʹ��
***************************************************************************/
void cDraw1Line(uint8_t LinNum)
{
	uint8_t i, j;

  //  SETBIT(ApplVar.ScreenStat,CRITICAL);

	for(i = 0; i < 2; i++)
	{
		st7565SetXY_C(0, pagemap_c[LinNum*2+i]);
		for(j = 0; j < 128; j++)
		{
			ST7565R_WR_DATA_C(DotBuffer_C[LinNum*2+i][j]);
		}
	}
  //  RESETBIT(ApplVar.ScreenStat,CRITICAL);

}

/**************************************************************************/
/* ���͵�������
***************************************************************************/
void st7565Refresh_C(void)
{
	uint8_t c, p;

	//SETBIT(ApplVar.ScreenStat,CRITICAL);
	for(p = 0; p < PAGE_NUM; p++)
	{
		st7565SetXY_C(0, pagemap_c[p]);
		for(c = 0; c < 128; c++)
		{
			ST7565R_WR_DATA_C(DotBuffer_C[p][c]);
		}
	}
   // RESETBIT(ApplVar.ScreenStat,CRITICAL);
}

/**************************************************************************/
/* ���� x=(0..127), y=(0..63)
***************************************************************************/
void st7565DrawPixel_C(uint8_t x, uint8_t y)
{
	if ((x >= 128) || (y >= 64))
		return;

	DotBuffer_C[y/8][x] |= (1 << (7-(y%8)));
}

/**************************************************************************/
/* ��� x=(0..127), y=(0..63)
***************************************************************************/
void st7565ClearPixel_C(uint8_t x, uint8_t y)
{
	if ((x >= 128) || (y >= 64))
		return;

	DotBuffer_C[y/8][x] &= ~(1 << (7-(y%8)));
}

/**************************************************************************/
/* ָ��λ����ʾһ���ַ�
   ��LCD��Ļ��y(0~3)��,��x(0~15)��λ����ʾ�ַ�ch�ĵ���
   ch������ASCII���ߺ���
***************************************************************************/
void cDispCharXY(WORD ch, BYTE x, BYTE y)
{
	uint8_t col, ch_l, x_c, y_p,hzFlag;
	uint32_t addr;
	uint8_t *fontH,*fontL;
/*
	if(y==SCREENLN)
	{
		if (TESTBIT(ApplVar.ScreenStat,STATEON))
			y--;
		else
			return;
	}
*/
	x_c = x*X_Width_C;	// column address
	y_p = y*2;		 	// page


#if defined(CASE_FORHANZI)
	if ((ch <= 0x7E))
#else
    ch &= 0xff;    //ccr2016-08-31
#endif
	{/* ASCII���� */
		hzFlag = 0;
        if (ch < 0x20) ch =0x20;
		addr = (ch - 0x20) * 16;

		fontH = &DotBuffer_C[y_p][x_c];
		fontL = &DotBuffer_C[y_p+1][x_c];

		memcpy(fontH,ASCIITAB_8x16+addr,8); 	 // �ϰ벿��
		memcpy(fontL,ASCIITAB_8x16+addr+8,8); 	 // �°벿��

/*
		if(TESTBIT(ApplVar.ScreenStat,INVATTR))
		{
			for (col = 0; col < 8; col++)
			{
				*fontH++ ^= 0xff; 	 // �ϰ벿��
				*fontL++ ^= 0xff;	 //�°벿��
			}
			fontH -= 8;
		}
		if((y>=(SCREENLN-1)) && (TESTBIT(ApplVar.ScreenStat,STATEON)))		   // ״̬����ʾ�ָ���
		{
			for (col = 0; col < 8; col++)
			{
				*fontH++ |= 0x80; 	// �ϰ벿��
			}
		}
*/

	}
#if defined(CASE_FORHANZI)
	else
	{/* ���ִ��� CCLIB16  */
		hzFlag = 1;
#if MMD_HALUO  //��
        if (ch==0x9af0)//�
            ch = 0xfad7;
        else if (ch==0x9bf0)//�
            ch = 0xfbd7;
#endif
		ch -= 0xA1A1;
		ch_l = ch & 0xff;
		if (ch_l >= 15)
			ch_l -= 1;

		//�ֿ�оƬ�ǰ��ֽ��б�ַ,��˱���32/2,64ΪASCII����(����ASCII=һ������):128/2
		addr = (uint32_t)(ch_l*94 + (ch>>8)+64)*32 + FLASH_HZK16_ADDR;		// ��ƫ�Ƶ�ַ(����ַ+ƫ�Ƶ�ַ,16bit)

		fontH = &DotBuffer[y_p][x_c];
		fontL = &DotBuffer[y_p+1][x_c];


		//������벿��
		wFlashReadBuffer((WORD*)fontH, addr, 4); 	//���ϲ���, ��ȡ8���ֽ�
		wFlashReadBuffer((WORD*)fontL, addr+8, 4); 	//���²���, ��ȡ8���ֽ�
		fontH+=8;fontL+=8;addr+=16;
		//�����Ұ벿��
		wFlashReadBuffer((WORD*)fontH, addr, 4); 	//���ϲ���, ��ȡ8���ֽ�
		wFlashReadBuffer((WORD*)fontL, addr+8, 4); 	//���²���, ��ȡ8���ֽ�


		fontH -= 8;fontL -= 8;
/*		
		if(TESTBIT(ApplVar.ScreenStat,INVATTR))
		{
			for (col = 0; col < 16; col++)
			{
				*fontH++ ^= 0xff; 	 // �ϰ벿��
				*fontL++ ^= 0xff;	 //�°벿��
			}
			fontH -= 16;
		}
		if((y>=(SCREENLN-1)) && (TESTBIT(ApplVar.ScreenStat,STATEON)))		   // ״̬����ʾ�ָ���
		{
			for (col = 0; col < 16; col++)
			{
				*fontH++ |= 0x80; 	// �ϰ벿��
			}
		}
*/		
	}
#endif

	cDrawChar(x, y,hzFlag);
}

/**************************************************************************/
/* ָ��λ����ʾ�ַ���
***************************************************************************/
void cDispStringXY(BYTE* str, BYTE x, BYTE y)
{
	uint8_t i;
	uint8_t *pstr;

	pstr = str;

	for (i = 0; i < SCREENWD && *pstr; )
	{
#if defined(CASE_FORHANZI)
		if(*pstr>=0xA1)
		{//HZK
			cDispCharXY(CWORD(pstr[0]), (x + i) , y);
			pstr+=2;
			i += 2;
		}
		else
#endif			
		{//ASCII
			cDispCharXY(*pstr, (x + i) , y);
			pstr++;
			i++;
		}
	}
}

/**************************************************************************
��ApplVar.ScreenMap�е�ָ����������ʾ����Ļ��
***************************************************************************/
void  cRefreshLine(BYTE y)
{
	//if(y < (SCREENLN - (ApplVar.ScreenStat & STATEON)))
		cDispStringXY((uint8_t *)ApplVar.ScreenMap[y], 0, y);
	//else if	((ApplVar.ScreenStat & STATEON) && (y==SCREENLN))
	//{
	//	cDispStringXY((uint8_t *)ApplVar.ScreenMap[SCREENLN], 0, SCREENLN);
	//}
}




/**************************************************************************/
/* ���Ե�0�� ��ʾ��ͬʱ��������
***************************************************************************/
void PutsC0(const char *str)
{
	uint8_t len;
	
	len=strlen(str);
	if(len>SCREENWD)
		len = SCREENWD;
	
	memset(SaveDispC[0],' ',SCREENWD);	
	memcpy(SaveDispC[0], str, len);
	cDispStringXY((uint8_t *)SaveDispC[0], 0, 0);
}

/**************************************************************************/
/* ���Ե�1�� ��ʾ��ͬʱ��������  
***************************************************************************/
void PutsC1(const char *str)
{
	uint8_t len;
	uint8_t addr=0;
	
	len=strlen(str);
	if(len>SCREENWD)
		len = SCREENWD;
	
//>>>>>������ʾ
	if(len<SCREENWD)
	{
		addr=SCREENWD-len;
		memset(SaveDispC[1],' ',SCREENWD);
		memcpy(SaveDispC[1]+addr,str,len);
	}
	else
		memcpy(SaveDispC[1],str,SCREENWD);
//<<<<<
	
	cDispStringXY((uint8_t *)SaveDispC[1], 0, 1);
}

/**************************************************************************/
/* ���Ե�0�� ֻ��ʾ������������
***************************************************************************/
void PutsC0_Only(const char *str)
{
	cDispStringXY((uint8_t *)str, 0, 0);
}

/**************************************************************************/
/* ���Ե�1�� ֻ��ʾ������������
***************************************************************************/
void PutsC1_Only(const char *str)
{
	cDispStringXY((uint8_t *)str, 0, 1);
}


