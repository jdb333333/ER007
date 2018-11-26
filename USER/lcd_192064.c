
#include <stdint.h>
#include "king.h"
#include "exthead.h"
//#include "exth.h"

#include "stm32f2xx.h"
#include "TypeDef.h"
#include "lcd_192064.h"
#include "flashb.h"

#include "fsmc_sram.h"

#define Y_Height	16				// ����߶�
#define X_Width		8				// ������С���

uint8_t ActiveLines = SCREENLN;

uint8_t DotBuffer[8][192]; //page addr:8  column addr:192

const uint8_t pagemap[8] = {0, 1, 2, 3, 4, 5, 6, 7,};//{ 1, 0, 3, 2, 5, 4, 7, 6,};//{3, 2, 1, 0, 7, 6, 5, 4,};//
extern const uint8_t ASCIITAB_8x16[];
extern const uint8_t RByte[256];

void KS0108_IO_Init(void);
void KS0108_L_WR_CMD(uint8_t cmd);
void KS0108_M_WR_CMD(uint8_t cmd);
void KS0108_R_WR_CMD(uint8_t cmd);
void KS0108_L_WR_DATA(uint8_t dat);
void KS0108_M_WR_DATA(uint8_t dat);
void KS0108_R_WR_DATA(uint8_t dat);
void KS0108_SetXY(uint8_t pos_x,uint8_t pos_y);
void KS0108_Draw8bit(uint8_t pos_x,uint8_t pos_y, uint8_t dat);
void KS0108_DrawPixel( uint8_t x, uint8_t y );
void KS0108_ClearPixel( uint8_t x, uint8_t y );
void KS0108_Refresh( void );


extern uint8_t NorFlash_WReadBuffer(uint16_t* pBuffer, uint32_t ReadAddr, uint32_t NumHalfwordToRead);

/**************************************************************************/
/*  ��ʱԼ1ms
***************************************************************************/

void KS0108_Delay_ms(uint16_t cnt)
{
	uint16_t i;
	while (cnt--)
		for(i=0; i<20000; i++);
}

void KS0108_NOP(void)
{
	uint16_t cnt=200;
	while(cnt--)
		__NOP(); // __NOP()Լ5~6ns
}

/**************************************************************************/
/* IO��ʼ�����������
***************************************************************************/
void KS0108_IO_Init(void)
{

	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);


	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	//GPIOB: BL--GPIO_Pin_2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

}

/**************************************************************************/
/* ��ʼ������ģ��
***************************************************************************/
extern void SysDelay_ms(uint16_t delay_ms);
extern void Test_OPDisplay(void);

void mInitialLCD(void)
{

	uint8_t c,p;

	ApplVar.ScreenStat = 0;

	KS0108_IO_Init();

	KS0108_L_WR_CMD(0xC0);	//������ʾ��ʼ��
	KS0108_M_WR_CMD(0xC0);
	KS0108_R_WR_CMD(0xC0);

	KS0108_L_WR_CMD(0x3F);	//����ʾ
	KS0108_M_WR_CMD(0x3F);
	KS0108_R_WR_CMD(0x3F);


	mClearScreen();
  mLightLCDOn();

	Test_OPDisplay();
}



/**************************************************************************/
/* д����
***************************************************************************/
void KS0108_L_WR_CMD(uint8_t cmd)
{
	KS0108_NOP();
	while((*(uint16_t*)(LCD_KS0108_L_RSTAT_ADDR)) & 0x80);
	KS0108_NOP();
	*(uint16_t *)LCD_KS0108_L_WCMD_ADDR = cmd;
}

void KS0108_M_WR_CMD(uint8_t cmd)
{
	KS0108_NOP();
	while((*(uint16_t*)(LCD_KS0108_M_RSTAT_ADDR)) & 0x80);
	KS0108_NOP();
	*(uint16_t *)LCD_KS0108_M_WCMD_ADDR = cmd;
}

void KS0108_R_WR_CMD(uint8_t cmd)
{
	KS0108_NOP();
	while((*(uint16_t*)(LCD_KS0108_R_RSTAT_ADDR)) & 0x80);
	KS0108_NOP();
	*(uint16_t *)LCD_KS0108_R_WCMD_ADDR = cmd;
}

/**************************************************************************/
/* д����
***************************************************************************/
void KS0108_L_WR_DATA(uint8_t dat)
{
	KS0108_NOP();
	while((*(uint16_t*)(LCD_KS0108_L_RSTAT_ADDR)) & 0x80);
	KS0108_NOP();
	*(uint16_t *)LCD_KS0108_L_WDATA_ADDR = dat;
}

void KS0108_M_WR_DATA(uint8_t dat)
{
	KS0108_NOP();
	while((*(uint16_t*)(LCD_KS0108_M_RSTAT_ADDR)) & 0x80);
	KS0108_NOP();
	*(uint16_t *)LCD_KS0108_M_WDATA_ADDR = dat;//????????????????????????????????????????????????????????
}

void KS0108_R_WR_DATA(uint8_t dat)
{
	KS0108_NOP();
	while((*(uint16_t*)(LCD_KS0108_R_RSTAT_ADDR)) & 0x80);
	KS0108_NOP();
	*(uint16_t *)LCD_KS0108_R_WDATA_ADDR = dat;
}


/*************************************************************************
 ������ʾ�Աȶ�
***************************************************************************/
void mSetContrast(uint8_t contrast)
{//������ʾ�Աȶ�
	//�޴˹���
}

/**************************************************************************/
/* ����ʾ����
***************************************************************************/
void mLightLCDOn(void)
{
	KS0108_BL_ON();
	ApplVar.ScreenStat |= LIGHTON;
}

/**************************************************************************/
/* �ر���ʾ����
***************************************************************************/
void mLightLCDOff(void)
{
	KS0108_BL_OFF();
	ApplVar.ScreenStat &= ~LIGHTON;
}

/**************************************************************************/
/* ������ʾ���
***************************************************************************/
void mEnCursor(uint8_t NewState )
{

	if(NewState)
	{//�򿪹��
        SETBIT(ApplVar.ScreenStat,CURSOREN);
	}
	else
	{
        if (BIT(ApplVar.ScreenStat,CURSOREN |CURSORON)==(CURSOREN |CURSORON))
            mDispCursor();//�رչ��
        RESETBIT(ApplVar.ScreenStat,(CURSOREN | CURSORON));
	}
}

/**************************************************************************/
/* ��ʾ���
***************************************************************************/
void mDispCursor(void)
{
	uint8_t valh,vall;
	int x,y;

	if(BIT(ApplVar.ScreenStat,CURSOREN))
	{
		SETBIT(ApplVar.ScreenStat,CRITICAL);
		x = ApplVar.sCursor.x;
		y = ApplVar.sCursor.y;

        y *= 2;
        x *= X_Width;

		if (ApplVar.sCursor.x>=SCREENWD)
			x--;//����Ļ���ұ߽�����ʾ���

		INVERTBIT(ApplVar.ScreenStat,CURSORON);
		if(BIT(ApplVar.ScreenStat,CURSORON))
		{
			valh = vall = 0xff;
		}
		else /*ccr2013 if (ApplVar.sCursor.x<SCREENWD)*/ //����Ļ���ڴ���ʾ���
		{
			valh = DotBuffer[y][x];
			vall = DotBuffer[y+1][x];
		}
/*ccr2013		else //����Ļ���ұ߽�����ʾ���
		{
			valh = 0;
			vall = 0;
		}
*/

		KS0108_Draw8bit(x, pagemap[y], valh);
		KS0108_Draw8bit(x, pagemap[y+1], vall);

		RESETBIT(ApplVar.ScreenStat,CRITICAL);
	}
}

/**************************************************************************/
/* �ƶ����	�ڵ�y(0~3)��,��x(0~15)��λ����ʾ��꣬��꿿�Ҷ���
***************************************************************************/
void mMoveCursor(uint8_t x, uint8_t y)
{
	uint8_t i;
    uint8_t cur=BIT(ApplVar.ScreenStat,CURSOREN);

    if (cur)
        mEnCursor(DISABLE);//�رչ��

	for(i = 0; i < x; )
	{
		if(ApplVar.ScreenMap[y][i]<0xA1)  //���ֻ�ASCII
			i++;
		else
			i += 2;
	}
	ApplVar.sCursor.x = i;
	ApplVar.sCursor.y = y;
    if (cur)
        mEnCursor(ENABLE);
}
/**************************************************************************/
/* �����������
***************************************************************************/
//void mClearInvAttr(void)
//{
//	RESETBIT(ApplVar.ScreenStat,INVATTR);
//}

/**************************************************************************/
/* ���÷�������,�˺��������ʾ���Է�����ʾ
***************************************************************************/
//void mSetInvAttr(void)
//{
//	SETBIT(ApplVar.ScreenStat,INVATTR);
//}

/**************************************************************************/
/* �򿪻�ر�״̬��
***************************************************************************/
void mSetState(uint8_t NewState)
{
	if(NewState)
	{
		SETBIT(ApplVar.ScreenStat,STATEON);
		ActiveLines = (SCREENLN-STATELN);
	}
	else
	{
		RESETBIT(ApplVar.ScreenStat,STATEON);
		ActiveLines = SCREENLN;
	}
}
/**************************************************************************/
/* �����Ļ����
***************************************************************************/
void mClearScreen(void)
{
	uint8_t p, pn;
	uint16_t c;

    SETBIT(ApplVar.ScreenStat,CRITICAL);

	if (BIT(ApplVar.ScreenStat,STATEON))
	{//״̬�д�ʱ��ֻ���������ʾ�У������״̬��
		pn = 6;
		memset(ApplVar.ScreenMap,' ',(SCREENLN-1)*SCREENWD);
	}
	else
	{
		pn = 8;
		memset(ApplVar.ScreenMap,' ',sizeof(ApplVar.ScreenMap));
	}

	for(p=0; p<pn; p++)
	{
		/*
		KS0108_SetXY(0, pagemap[p]);
		KS0108_SetXY(0x40, pagemap[p]);
		KS0108_SetXY(0x80, pagemap[p]);

		for(c = 0; c < 0x40; c++)
			KS0108_L_WR_DATA(0);
		for(; c < 0x80; c++)
			KS0108_M_WR_DATA(0);
		for(; c < 0xC0; c++)
			KS0108_R_WR_DATA(0);
		*/
		for(c=0; c < 0xC0; c++)
			KS0108_Draw8bit(c, pagemap[p], 0);
	}

	memset(DotBuffer,0,pn*192);//���buffer����
	GotoXY(0,0);
  RESETBIT(ApplVar.ScreenStat,CRITICAL);

}

/**************************************************************************/
/* ������ʾλ�� x=(0..191), y=(0..63)
***************************************************************************/
void KS0108_SetXY(uint8_t pos_x,uint8_t pos_y)
{
	if(pos_x<0x40)
	{
		KS0108_L_WR_CMD(0xb8+pos_y);
		KS0108_L_WR_CMD(0x40+pos_x);
	}
	else if(pos_x<0x80)
	{
		KS0108_M_WR_CMD(0xb8+pos_y);
		KS0108_M_WR_CMD(pos_x);
	}
	else if(pos_x<0xC0)
	{
		KS0108_R_WR_CMD(0xb8+pos_y);
		KS0108_R_WR_CMD(pos_x-0x40);
	}

}

/**************************************************************************/
/* ����һҳһ�����ݵ�(8bit)
***************************************************************************/
void KS0108_Draw8bit(uint8_t pos_x,uint8_t pos_y, uint8_t dat)
{
	*(uint16_t *)LCD_KS0108_DUMMY_ADDR = 0xFF;
	if(pos_x<0x40)
	{
		KS0108_L_WR_CMD(0xb8+pos_y);
		KS0108_L_WR_CMD(0x40+pos_x);
		KS0108_L_WR_DATA(dat);
	}
	else if(pos_x<0x80)
	{
		KS0108_M_WR_CMD(0xb8+pos_y);
		KS0108_M_WR_CMD(pos_x);
		KS0108_M_WR_DATA(dat);
	}
	else if(pos_x<0xC0)
	{
		KS0108_R_WR_CMD(0xb8+pos_y);
		KS0108_R_WR_CMD(pos_x-0x40);
		KS0108_R_WR_DATA(dat);
	}

}



/**************************************************************************/
/* ��DotBuffer��ָ����LinNum(0~3),��ColNumλ�õ��ַ�����������ʾ����Ļ��,
���ã�������ָ��λ��һ���ַ�ʱʹ��
hzFlag:=0,��ʾascii�ַ�;=1,��ʾ�����ַ�
***************************************************************************/
void mDrawChar(uint8_t ColNum, uint8_t LinNum,uint8_t hzFlag)
{
	uint8_t j;
	uint8_t *pDot;

	ColNum = ColNum*X_Width;
	LinNum *= 2;
#if 1 //ouhs defined(CASE_FORHANZI)
	if(hzFlag)	 // ����ģʽ��ʾΪ16λ���
		hzFlag = 16;
	else
#endif
		hzFlag = 8;

	//��ʾ�ϰ벿��
	pDot = &DotBuffer[LinNum][ColNum];
    SETBIT(ApplVar.ScreenStat,CRITICAL);

	//KS0108_SetXY(ColNum, pagemap[LinNum]);
	for(j = 0; j < hzFlag; j++)
		//KS0108_Draw8bit(ColNum++, pagemap[LinNum], *pDot++);
		KS0108_Draw8bit(ColNum++, pagemap[LinNum], RByte[*pDot++]); //ouhs 2015.07.03

	//��ʾ�°벿��
	ColNum -= hzFlag;
	LinNum++;
	pDot = &DotBuffer[LinNum][ColNum];

#if 1
	for(j = 0; j < hzFlag; j++)
		KS0108_Draw8bit(ColNum++, pagemap[LinNum], RByte[*pDot++]);	//ouhs 2015.07.03
#else
	if(ColNum <0x40 )
	{
		if((ColNum+hzFlag)<0x40)
		{
			KS0108_SetXY(ColNum, pagemap[LinNum]);
			for(j = 0; j < hzFlag; j++)
				KS0108_L_WR_DATA(RByte[*pDot++]);
		}
		else
		{
			for(j = 0; j < hzFlag; j++)
				KS0108_Draw8bit(ColNum++, pagemap[LinNum], RByte[*pDot++]);
		}
	}
	else if(ColNum <0x80 )
	{
		if((ColNum+hzFlag)<0x80)
		{
			KS0108_SetXY(ColNum, pagemap[LinNum]);
			for(j = 0; j < hzFlag; j++)
				KS0108_M_WR_DATA(RByte[*pDot++]);
		}
		else
		{
			for(j = 0; j < hzFlag; j++)
				KS0108_Draw8bit(ColNum++, pagemap[LinNum], RByte[*pDot++]);
		}
	}
	else 	if(ColNum <0xc0 )
	{
		if((ColNum+hzFlag)<0xc0)
		{
			KS0108_SetXY(ColNum, pagemap[LinNum]);
			for(j = 0; j < hzFlag; j++)
				KS0108_R_WR_DATA(RByte[*pDot++]);
		}
		else
		{
			for(j = 0; j < hzFlag; j++)
				KS0108_Draw8bit(ColNum++, pagemap[LinNum], RByte[*pDot++]);
		}
	}
#endif

	RESETBIT(ApplVar.ScreenStat,CRITICAL);

}

/**************************************************************************/
/* ��DotBuffer��ָ����LinNum(0~3)���ַ�����������ʾ����Ļ��,
   ������Ļ�ϵĵ�LinNum��������ʾһ��,���ã���Ҫ����ָ���ж���ַ�ʱʹ��
***************************************************************************/
void mDraw1Line(uint8_t LinNum)
{
	uint8_t i, j;

  SETBIT(ApplVar.ScreenStat,CRITICAL);

	for(i = 0; i < 2; i++)
	{
		KS0108_SetXY(0, pagemap[LinNum*2+i]);
		KS0108_SetXY(0x40, pagemap[LinNum*2+i]);
		KS0108_SetXY(0x80, pagemap[LinNum*2+i]);

		for(j = 0; j < 0x40; j++)
			KS0108_L_WR_DATA(DotBuffer[LinNum*2+i][j]);
		for(; j < 0x80; j++)
			KS0108_M_WR_DATA(DotBuffer[LinNum*2+i][j]);
		for(; j < 0xC0; j++)
			KS0108_R_WR_DATA(DotBuffer[LinNum*2+i][j]);
	}
    RESETBIT(ApplVar.ScreenStat,CRITICAL);

}

/**************************************************************************/
/* ���͵�������
***************************************************************************/
void KS0108_Refresh(void)
{
	uint8_t c, p;

  SETBIT(ApplVar.ScreenStat,CRITICAL);
	for(p = 0; p < 8; p++)
	{
		KS0108_SetXY(0, pagemap[p]);
		KS0108_SetXY(0x40, pagemap[p]);
		KS0108_SetXY(0x80, pagemap[p]);

		for(c = 0; c < 0x40; c++)
			KS0108_L_WR_DATA(DotBuffer[p][c]);
		for(; c < 0x80; c++)
			KS0108_M_WR_DATA(DotBuffer[p][c]);
		for(; c < 0xC0; c++)
			KS0108_R_WR_DATA(DotBuffer[p][c]);

	}
    RESETBIT(ApplVar.ScreenStat,CRITICAL);
}

/**************************************************************************/
/* ���� x=(0..191), y=(0..63)
***************************************************************************/
void KS0108_DrawPixel(uint8_t x, uint8_t y)
{
	if ((x >= 192) || (y >= 64))
		return;

	DotBuffer[y/8][x] |= (1 << (y%8));
}

/**************************************************************************/
/* ��� x=(0..191), y=(0..63)
***************************************************************************/
void KS0108_ClearPixel(uint8_t x, uint8_t y)
{
	if ((x >= 192) || (y >= 64))
		return;

	DotBuffer[y/8][x] &= ~(1 << (y%8));
}

/**************************************************************************/
/* ָ��λ����ʾһ���ַ�
   ��LCD��Ļ��y(0~3)��,��x(0~15)��λ����ʾ�ַ�ch�ĵ���
   ch������ASCII���ߺ���
***************************************************************************/
void mDispCharXY(WORD ch, BYTE x, BYTE y)
{
	uint8_t col, ch_l, x_c, y_p,hzFlag;
	uint32_t addr;
	uint8_t *fontH,*fontL;

	if(y==SCREENLN)
	{
		if (BIT(ApplVar.ScreenStat,STATEON))
			y--;
		else
			return;
	}

	x_c = x*X_Width;	// column address
	y_p = y*2;		 	// page


#if 1//ouhs defined(CASE_FORHANZI)
	if ((ch <= 0x7E))
#endif
	{/* ASCII���� */
		hzFlag = 0;
        if (ch < 0x20) ch =0x20;
		addr = (ch - 0x20) * 16;

		fontH = &DotBuffer[y_p][x_c];
		fontL = &DotBuffer[y_p+1][x_c];

		memcpy(fontH,ASCIITAB_8x16+addr,8); 	 // �ϰ벿��
		memcpy(fontL,ASCIITAB_8x16+addr+8,8); 	 // �°벿��

		if(BIT(ApplVar.ScreenStat,INVATTR))
		{
			for (col = 0; col < 8; col++)
			{
				*fontH++ ^= 0xff; 	 // �ϰ벿��
				*fontL++ ^= 0xff;	 //�°벿��
			}
			fontH -= 8;
		}
		if((y>=(SCREENLN-1)) && (BIT(ApplVar.ScreenStat,STATEON)))		   // ״̬����ʾ�ָ���
		{
			for (col = 0; col < 8; col++)
			{
				*fontH++ |= 0x80; 	// �ϰ벿��
			}
		}

	}
#if 1//ouhs defined(CASE_FORHANZI)
	else
	{/* ���ִ��� CCLIB16  */
		hzFlag = 1;
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
		if(BIT(ApplVar.ScreenStat,INVATTR))
		{
			for (col = 0; col < 16; col++)
			{
				*fontH++ ^= 0xff; 	 // �ϰ벿��
				*fontL++ ^= 0xff;	 //�°벿��
			}
			fontH -= 16;
		}
		if((y>=(SCREENLN-1)) && (BIT(ApplVar.ScreenStat,STATEON)))		   // ״̬����ʾ�ָ���
		{
			for (col = 0; col < 16; col++)
			{
				*fontH++ |= 0x80; 	// �ϰ벿��
			}
		}
	}
#endif

	mDrawChar(x, y,hzFlag);
}

/**************************************************************************/
/* ָ��λ����ʾ�ַ���
***************************************************************************/
void mDispStringXY(BYTE* str, BYTE x, BYTE y)
{
	uint8_t i;
	uint8_t *pstr;

	pstr = str;

	for (i = 0; i < SCREENWD && *pstr; )
	{
		if(*pstr<0xA1)
		{//ASCII
			mDispCharXY(*pstr, (x + i) , y);
			pstr++;
			i++;
		}
		else
		{//HZK
			mDispCharXY(CWORD(pstr[0]), (x + i) , y);
			pstr+=2;
			i += 2;
		}
	}
}

/**************************************************************************
��ApplVar.ScreenMap�е�ָ����������ʾ����Ļ��
***************************************************************************/
void  mRefreshLine(BYTE y)
{
	if(y < (SCREENLN - (ApplVar.ScreenStat & STATEON)))
		mDispStringXY((uint8_t *)ApplVar.ScreenMap[y], 0, y);
	else if	((ApplVar.ScreenStat & STATEON) && (y==SCREENLN))
	{
		mDispStringXY((uint8_t *)ApplVar.ScreenMap[SCREENLN], 0, SCREENLN);
	}
}


void Test_LCD12864(void)
{
}





#if 0 //ouhs �ò��ִ���δ�޸�δ����

/**************************************************************************
 ����Ļָ����ASCII�ַ�λ�����Ͻ���˸��ʾGPRS��ʾ
***************************************************************************/
void mDrawGPRS(uint8_t ColNum, uint8_t LinNum)
{
	uint8_t j;
	uint8_t *pDot;
    static uint8_t gFlash=0x40;

    if ((GetSystemTimer() & 0x100) == 0)
    {
		if (!(gFlash & 1))
		{
			ColNum = ColNum*X_Width;
			LinNum *= 2;

			//��ʾ�ϰ벿��
			pDot = &DotBuffer[LinNum][ColNum]+5;
			SETBIT(ApplVar.ScreenStat,CRITICAL);

			KS0108_SetXY(ColNum+5, pagemap[LinNum]);
			for(j = 0; j < 8-5; j++)
			{
				gFlash ^= 0xc0;
				KS0108_WR_DATA((*pDot | gFlash));
				pDot++;
			}
			gFlash |= 1;
		}
    }
	else
		gFlash &=0xc0;

}

/**
 * ����Ļָ����ASCII�ַ�λ����ʾ�������
 *
 * @author EutronSoftware (2014-12-31)
 *
 * @param ctrl :0-����ʾ;=1���Բ���ʾ;=2��ʾ
 * @param ColNum
 * @param LinNum
 */
extern void DisplayDecXY(WORD pDec,int x,int y,int pWdith);
#define VIN_FULL  VIN_8V7 //8.7V---3075mV---0xEE8   +����ֵ0x6B
#define VIN_NONE  VIN_7V0 //7.0V---1375mV---0x6AA
void mDrawPower(BYTE ctrl, uint8_t ColNum,uint8_t LinNum)
{
    static WORD gPower=0;

    uint8_t sByte;
	WORD pDot,j;

    //if (BatteryExist())//&& !DC_DET_GET())//ֻ���ڵ�ع���ʱ,����ʾ�������
    {
        if (ctrl==0)
        {
            gPower = 0;
            return;
        }
        else if (ctrl==1)
        {
            if (gPower==0)
                return;
        }
        gPower = Vin_Vrtc_Val[0];
        //DisplayDecXY(gPower,ColNum+1,4,6);//testonly

        if (gPower>=VIN_FULL)
            j=16;
        else if (gPower<=VIN_NONE)
            j = 0;
        else
            j = ((gPower-VIN_NONE)*16)/(VIN_FULL-VIN_NONE);

        pDot = (1<<j)-1;

        ColNum = ColNum*X_Width;
        LinNum *= 2;

        //��ʾ�ϰ벿��

        SETBIT(ApplVar.ScreenStat,CRITICAL);

        sByte = (pDot>>8) | 0x80;
        KS0108_SetXY(ColNum, pagemap[LinNum]);
        KS0108_WR_DATA(0xff);
        KS0108_WR_DATA(sByte | 0x40);
        for(j = 0; j < 4; j++)
            KS0108_WR_DATA(sByte);
        KS0108_WR_DATA(sByte | 0x40);
        KS0108_WR_DATA(0xff);

        sByte = (pDot & 0xff);
        KS0108_SetXY(ColNum, pagemap[LinNum+1]);
        KS0108_WR_DATA(0xff);
        for(j = 0; j < 6; j++)
            KS0108_WR_DATA(sByte);
        KS0108_WR_DATA(0xff);
    }
}

#endif //ouhs
