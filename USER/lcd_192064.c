
#include <stdint.h>
#include "king.h"
#include "exthead.h"
//#include "exth.h"

#include "stm32f2xx.h"
#include "TypeDef.h"
#include "lcd_192064.h"
#include "flashb.h"

#include "fsmc_sram.h"

#define Y_Height	16				// 字体高度
#define X_Width		8				// 字体最小宽度

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
/*  延时约1ms
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
		__NOP(); // __NOP()约5~6ns
}

/**************************************************************************/
/* IO初始化：背光控制
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
/* 初始化汉显模块
***************************************************************************/
extern void SysDelay_ms(uint16_t delay_ms);
extern void Test_OPDisplay(void);

void mInitialLCD(void)
{

	uint8_t c,p;

	ApplVar.ScreenStat = 0;

	KS0108_IO_Init();

	KS0108_L_WR_CMD(0xC0);	//设置显示初始行
	KS0108_M_WR_CMD(0xC0);
	KS0108_R_WR_CMD(0xC0);

	KS0108_L_WR_CMD(0x3F);	//开显示
	KS0108_M_WR_CMD(0x3F);
	KS0108_R_WR_CMD(0x3F);


	mClearScreen();
  mLightLCDOn();

	Test_OPDisplay();
}



/**************************************************************************/
/* 写命令
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
/* 写数据
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
 设置显示对比度
***************************************************************************/
void mSetContrast(uint8_t contrast)
{//设置显示对比度
	//无此功能
}

/**************************************************************************/
/* 打开显示背光
***************************************************************************/
void mLightLCDOn(void)
{
	KS0108_BL_ON();
	ApplVar.ScreenStat |= LIGHTON;
}

/**************************************************************************/
/* 关闭显示背光
***************************************************************************/
void mLightLCDOff(void)
{
	KS0108_BL_OFF();
	ApplVar.ScreenStat &= ~LIGHTON;
}

/**************************************************************************/
/* 允许显示光标
***************************************************************************/
void mEnCursor(uint8_t NewState )
{

	if(NewState)
	{//打开光标
        SETBIT(ApplVar.ScreenStat,CURSOREN);
	}
	else
	{
        if (BIT(ApplVar.ScreenStat,CURSOREN |CURSORON)==(CURSOREN |CURSORON))
            mDispCursor();//关闭光标
        RESETBIT(ApplVar.ScreenStat,(CURSOREN | CURSORON));
	}
}

/**************************************************************************/
/* 显示光标
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
			x--;//在屏幕的右边界外显示光标

		INVERTBIT(ApplVar.ScreenStat,CURSORON);
		if(BIT(ApplVar.ScreenStat,CURSORON))
		{
			valh = vall = 0xff;
		}
		else /*ccr2013 if (ApplVar.sCursor.x<SCREENWD)*/ //在屏幕界内处显示光标
		{
			valh = DotBuffer[y][x];
			vall = DotBuffer[y+1][x];
		}
/*ccr2013		else //在屏幕的右边界外显示光标
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
/* 移动光标	在第y(0~3)行,第x(0~15)列位置显示光标，光标靠右对齐
***************************************************************************/
void mMoveCursor(uint8_t x, uint8_t y)
{
	uint8_t i;
    uint8_t cur=BIT(ApplVar.ScreenStat,CURSOREN);

    if (cur)
        mEnCursor(DISABLE);//关闭光标

	for(i = 0; i < x; )
	{
		if(ApplVar.ScreenMap[y][i]<0xA1)  //汉字或ASCII
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
/* 清除反显属性
***************************************************************************/
//void mClearInvAttr(void)
//{
//	RESETBIT(ApplVar.ScreenStat,INVATTR);
//}

/**************************************************************************/
/* 设置反显属性,此后的所有显示将以反显显示
***************************************************************************/
//void mSetInvAttr(void)
//{
//	SETBIT(ApplVar.ScreenStat,INVATTR);
//}

/**************************************************************************/
/* 打开或关闭状态行
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
/* 清除屏幕内容
***************************************************************************/
void mClearScreen(void)
{
	uint8_t p, pn;
	uint16_t c;

    SETBIT(ApplVar.ScreenStat,CRITICAL);

	if (BIT(ApplVar.ScreenStat,STATEON))
	{//状态行打开时，只清除内容显示行，不清除状态行
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

	memset(DotBuffer,0,pn*192);//清除buffer点阵
	GotoXY(0,0);
  RESETBIT(ApplVar.ScreenStat,CRITICAL);

}

/**************************************************************************/
/* 设置显示位置 x=(0..191), y=(0..63)
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
/* 绘制一页一列数据点(8bit)
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
/* 把DotBuffer中指定行LinNum(0~3),列ColNum位置的字符点阵重新显示在屏幕上,
适用：仅更新指定位置一个字符时使用
hzFlag:=0,显示ascii字符;=1,显示汉字字符
***************************************************************************/
void mDrawChar(uint8_t ColNum, uint8_t LinNum,uint8_t hzFlag)
{
	uint8_t j;
	uint8_t *pDot;

	ColNum = ColNum*X_Width;
	LinNum *= 2;
#if 1 //ouhs defined(CASE_FORHANZI)
	if(hzFlag)	 // 汉字模式显示为16位宽度
		hzFlag = 16;
	else
#endif
		hzFlag = 8;

	//显示上半部分
	pDot = &DotBuffer[LinNum][ColNum];
    SETBIT(ApplVar.ScreenStat,CRITICAL);

	//KS0108_SetXY(ColNum, pagemap[LinNum]);
	for(j = 0; j < hzFlag; j++)
		//KS0108_Draw8bit(ColNum++, pagemap[LinNum], *pDot++);
		KS0108_Draw8bit(ColNum++, pagemap[LinNum], RByte[*pDot++]); //ouhs 2015.07.03

	//显示下半部分
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
/* 把DotBuffer中指定行LinNum(0~3)的字符点阵重新显示在屏幕上,
   即把屏幕上的第LinNum行重新显示一遍,适用：需要更新指定行多个字符时使用
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
/* 发送点阵数据
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
/* 画点 x=(0..191), y=(0..63)
***************************************************************************/
void KS0108_DrawPixel(uint8_t x, uint8_t y)
{
	if ((x >= 192) || (y >= 64))
		return;

	DotBuffer[y/8][x] |= (1 << (y%8));
}

/**************************************************************************/
/* 清点 x=(0..191), y=(0..63)
***************************************************************************/
void KS0108_ClearPixel(uint8_t x, uint8_t y)
{
	if ((x >= 192) || (y >= 64))
		return;

	DotBuffer[y/8][x] &= ~(1 << (y%8));
}

/**************************************************************************/
/* 指定位置显示一个字符
   在LCD屏幕第y(0~3)行,第x(0~15)列位置显示字符ch的点阵
   ch可以是ASCII或者汉字
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
	{/* ASCII点阵 */
		hzFlag = 0;
        if (ch < 0x20) ch =0x20;
		addr = (ch - 0x20) * 16;

		fontH = &DotBuffer[y_p][x_c];
		fontL = &DotBuffer[y_p+1][x_c];

		memcpy(fontH,ASCIITAB_8x16+addr,8); 	 // 上半部分
		memcpy(fontL,ASCIITAB_8x16+addr+8,8); 	 // 下半部分

		if(BIT(ApplVar.ScreenStat,INVATTR))
		{
			for (col = 0; col < 8; col++)
			{
				*fontH++ ^= 0xff; 	 // 上半部分
				*fontL++ ^= 0xff;	 //下半部分
			}
			fontH -= 8;
		}
		if((y>=(SCREENLN-1)) && (BIT(ApplVar.ScreenStat,STATEON)))		   // 状态行显示分隔线
		{
			for (col = 0; col < 8; col++)
			{
				*fontH++ |= 0x80; 	// 上半部分
			}
		}

	}
#if 1//ouhs defined(CASE_FORHANZI)
	else
	{/* 汉字处理 CCLIB16  */
		hzFlag = 1;
		ch -= 0xA1A1;
		ch_l = ch & 0xff;
		if (ch_l >= 15)
			ch_l -= 1;

		//字库芯片是按字进行编址,因此必须32/2,64为ASCII个数(两个ASCII=一个汉字):128/2
		addr = (uint32_t)(ch_l*94 + (ch>>8)+64)*32 + FLASH_HZK16_ADDR;		// 字偏移地址(基地址+偏移地址,16bit)

		fontH = &DotBuffer[y_p][x_c];
		fontL = &DotBuffer[y_p+1][x_c];


		//汉字左半部分
		wFlashReadBuffer((WORD*)fontH, addr, 4); 	//左上部分, 读取8个字节
		wFlashReadBuffer((WORD*)fontL, addr+8, 4); 	//左下部分, 读取8个字节
		fontH+=8;fontL+=8;addr+=16;
		//汉字右半部分
		wFlashReadBuffer((WORD*)fontH, addr, 4); 	//右上部分, 读取8个字节
		wFlashReadBuffer((WORD*)fontL, addr+8, 4); 	//右下部分, 读取8个字节


		fontH -= 8;fontL -= 8;
		if(BIT(ApplVar.ScreenStat,INVATTR))
		{
			for (col = 0; col < 16; col++)
			{
				*fontH++ ^= 0xff; 	 // 上半部分
				*fontL++ ^= 0xff;	 //下半部分
			}
			fontH -= 16;
		}
		if((y>=(SCREENLN-1)) && (BIT(ApplVar.ScreenStat,STATEON)))		   // 状态行显示分隔线
		{
			for (col = 0; col < 16; col++)
			{
				*fontH++ |= 0x80; 	// 上半部分
			}
		}
	}
#endif

	mDrawChar(x, y,hzFlag);
}

/**************************************************************************/
/* 指定位置显示字符串
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
将ApplVar.ScreenMap中的指定行重新显示在屏幕上
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





#if 0 //ouhs 该部分代码未修改未调试

/**************************************************************************
 在屏幕指定的ASCII字符位置右上角闪烁显示GPRS标示
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

			//显示上半部分
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
 * 在屏幕指定的ASCII字符位置显示电池容量
 *
 * @author EutronSoftware (2014-12-31)
 *
 * @param ctrl :0-不显示;=1测试并显示;=2显示
 * @param ColNum
 * @param LinNum
 */
extern void DisplayDecXY(WORD pDec,int x,int y,int pWdith);
#define VIN_FULL  VIN_8V7 //8.7V---3075mV---0xEE8   +修正值0x6B
#define VIN_NONE  VIN_7V0 //7.0V---1375mV---0x6AA
void mDrawPower(BYTE ctrl, uint8_t ColNum,uint8_t LinNum)
{
    static WORD gPower=0;

    uint8_t sByte;
	WORD pDot,j;

    //if (BatteryExist())//&& !DC_DET_GET())//只有在电池供电时,才显示电池容量
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

        //显示上半部分

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
