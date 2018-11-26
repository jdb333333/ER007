/****************************************Copyright (c)**************************************************
**                               Hunan EUTRON information Equipment Co.,LTD.
**
**                                 http://www.eutron.com
**
**--------------File Info-------------------------------------------------------------------------------
** File name:       LCD_Drv.c
** Descriptions:    S6A0069驱动
**
**------------------------------------------------------------------------------------------------------
** Created by:      Ruby
** Created date:    2011-12-05
** Version:         1.0
** Descriptions:    The original version
**
**------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Descriptions:
********************************************************************************************************/
#include "stm32f2xx.h"
#include "lcd_160x2.h"
#include "fsmc_sram.h"
#include "SysTick.h"
#include "pwr_ctrl.h"
#include "TypeDef.h"

#define LCD_D0	GPIO_Pin_3
#define LCD_D1	GPIO_Pin_4
#define LCD_D2	GPIO_Pin_5
#define LCD_D3	GPIO_Pin_6
#define LCD_DataPort	GPIOA

#define LCDC_E	GPIO_Pin_15	//Customer
#define LCDO_E	GPIO_Pin_10	//Operator
#define LCD_E_Port		GPIOA

#define LCD_RS	GPIO_Pin_0
#define LCD_RW	GPIO_Pin_8
#define LCD_BL	GPIO_Pin_2

#define	RS_Low()	GPIOA->BSRRH =	LCD_RS
#define	RS_High()	GPIOA->BSRRL =	LCD_RS

#define	RW_Low()	GPIOA->BSRRH =	LCD_RW
#define	RW_High()	GPIOA->BSRRL =	LCD_RW

#define	C_E_Low()		LCD_E_Port->BSRRH =	LCDC_E
#define	C_E_High()	LCD_E_Port->BSRRL =	LCDC_E
#define	O_E_Low()		LCD_E_Port->BSRRH =	LCDO_E
#define	O_E_High()	LCD_E_Port->BSRRL =	LCDO_E

#define	LINE_NUM	2

#if CASE_ER260
const char  Disp[LINE_NUM][DISLEN] = {"Fiscal ECR ER260", "  Good For You! "};

#elif CASE_ER380
const char  Disp[LINE_NUM][DISLEN] = {"Fiscal ECR ER380", "  Good For You! "};
#elif CASE_PCR01
const char  Disp[LINE_NUM][DISLEN] = {"Fiscal ECR PCR01", "  Good For You! "};
#elif CASE_MCR30
const char  Disp[LINE_NUM][DISLEN] = {"Fiscal ECR MCR30", " Good For You! "};
#elif CASE_ECR100F
const char  Disp[LINE_NUM][DISLEN] = {"    ECR-100F    ", "  Good For You! "};
#elif CASE_ECR99
const char  Disp[LINE_NUM][DISLEN] = {"    ECR-99F     ", "  Good For You! "};
#elif CASE_ER260F
const char  Disp[LINE_NUM][DISLEN] = {"    ER-260F    ", "  Good For You! "};
#endif

char SaveDispO[LINE_NUM][DISLEN],SaveDispC[LINE_NUM][DISLEN];//保护显示内容
volatile uint8_t LCD_BL_CTRL;

void LCD_CheckBusy(uint8_t LCDn);
void LCD_ModeInit4Bit(uint8_t LCDn);

void LCD160x_Test(void);

//增加Greek字符
const uint8_t ASCII_Table[] ={
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x03, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,

		//0x80-0x87: 自定义字符  0x88-0xc0: 空
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,

		//Greek
    0x20, 0x41, 0x42, 0xd4, 0x7f, 0x45, 0x5a, 0x48, 0xd6, 0x49, 0x4b, 0xd7, 0x4d, 0x4e, 0xd8, 0x4f,
    0xd9, 0x50, 0x20, 0xda, 0x54, 0xdb, 0xdc, 0x58, 0xdd, 0xde, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0x6f,
    0xed, 0xee, 0x87, 0xef, 0xf0, 0xf1, 0xaf, 0xf2, 0xf3, 0xf4, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
};

const uint8_t Euro[8]={0x07,0x08,0x1e,0x08,0x1e,0x08,0x07,0x00};   	//欧元符号 0x80

const uint8_t Signal1[8]={0x00,0x00,0x00,0x00,0x00,0x08,0x18,0x00};	//GPRS信号 0x81
const uint8_t Signal3[8]={0x00,0x00,0x00,0x00,0x04,0x0c,0x1c,0x00}; //0x82
const uint8_t Signal5[8]={0x00,0x00,0x01,0x03,0x07,0x0f,0x1f,0x00};	//0x83

//const uint8_t Battery0[8]={0x06,0x0f,0x09,0x09,0x09,0x09,0x0f,0x00};	//电池电量  空 0x84
//const uint8_t Battery1[8]={0x06,0x0f,0x09,0x09,0x09,0x0f,0x0f,0x00};	//0x85
//const uint8_t Battery3[8]={0x06,0x0f,0x09,0x0f,0x0f,0x0f,0x0f,0x00};	//0x86
//const uint8_t Battery4[8]={0x06,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x00};	//电池电量  满 0x87

const uint8_t Battery5[8]={0x02,0x06,0x0C,0x1F,0x06,0x0C,0x08,0x00};	//充电闪电图标
//const uint8_t Battery5[8]={0x0A,0x0A,0x1F,0x1F,0x1F,0x0E,0x0E,0x00};	//适配器插头图标

// 延时1ms
void LCD160x_Delay_ms(uint16_t cnt)
{
	uint16_t i;
	while (cnt--)
		for(i=0; i<20000; i++);
}

void LCD160x_Dly80ns(uint8_t cnt)
{
	while(cnt--)
		__NOP(); // __NOP()约80ns
//>>>>>>>>>>>>>ouhs 20151106
	__NOP();__NOP();__NOP();__NOP();__NOP();

//<<<<<<<<<<<<<<<
}

// 发送4bit数据
void LCD_Sendbit(uint16_t ch)
{
	ch <<= 3;
	LCD_DataPort->BSRRL = ch;
	LCD_DataPort->BSRRH = (~ch)&0x78;

}

// 发送数据到LCD
void LCD_SendData(uint8_t LCDn, uint8_t data)
{
	if(LCDn)
		C_E_Low();
	else
		O_E_Low();

	LCD_Sendbit(data);
	//LCD160x_Dly80ns(1);
	if(LCDn)
		C_E_High();
	else
		O_E_High();
	LCD160x_Dly80ns(5);	 //410ns

	if(LCDn)
		C_E_Low();
	else
		O_E_Low();
	LCD160x_Dly80ns(1);
}

// 设置4位数据线模式
void LCD_ModeInit4Bit(uint8_t LCDn)
{
	RW_Low();
	RS_Low();
	LCD_SendData(LCDn, 0x02);		// 命令：0x28  4bit,16x1-line,5x8dots
	LCD_SendData(LCDn, 0x08);
}

// 读取状态标志
void LCD_CheckBusy(uint8_t LCDn)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	uint8_t ReadData, ReadData_H, ReadData_L;
	uint16_t temp = 0xfff;

    LCD_DataPort->BSRRL = LCD_D0 | LCD_D1 | LCD_D2 | LCD_D3; //先置1再读取

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = LCD_D0 | LCD_D1 | LCD_D2 | LCD_D3;
	GPIO_Init(LCD_DataPort, &GPIO_InitStructure);
	RW_High();
	RS_Low();
	do{

		if(LCDn)
			C_E_High();
		else
			O_E_High();
		LCD160x_Dly80ns(5);
		ReadData_H = (LCD_DataPort->IDR)<<1;	// Pin6|Pin5|Pin4|Pin3

		if(LCDn)
			C_E_Low();
		else
			O_E_Low();
		LCD160x_Dly80ns(5);
	   	if(LCDn)
			C_E_High();
		else
			O_E_High();
		LCD160x_Dly80ns(5);
		ReadData_L = (LCD_DataPort->IDR)>>3;	// Pin6|Pin5|Pin4|Pin3

		if(LCDn)
			C_E_Low();
		else
			O_E_Low();
		LCD160x_Dly80ns(5);

		ReadData = ((ReadData_H) | (ReadData_L & 0x0F));
	}while((ReadData & 0x80) && (temp--));

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(LCD_DataPort, &GPIO_InitStructure);
}


// 写命令
uint8_t LCD_WRCmd(uint8_t LCDn, uint8_t commmand)
{
	LCD_CheckBusy(LCDn);
	RW_Low();
	RS_Low();
   	LCD_SendData(LCDn, commmand>>4);	   // 先发送高4位
	LCD_SendData(LCDn, commmand&0x0F);	   // 后发送低4位
	return 1;
}

// 写数据
uint8_t LCD_WRData(uint8_t LCDn, uint8_t data)
{
	LCD_CheckBusy(LCDn);
	RW_Low();
	RS_High();
   	LCD_SendData(LCDn, data>>4);	   // 先发送高4位
	LCD_SendData(LCDn, data&0x0F);	   // 后发送低4位
	return 1;
}

void LCD_Pos(uint8_t LCDn, uint8_t LinNum, uint8_t pos)
{
	pos &= 0x1f;
	if(LinNum == 0)
		LCD_WRCmd(LCDn, pos | 0x80);
	else
		LCD_WRCmd(LCDn, (pos&0x0F)|0xC0);

}

void LCD160x_IO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

	GPIO_InitStructure.GPIO_Pin = LCD_D0 | LCD_D1 | LCD_D2 | LCD_D3 | LCD_RS | LCD_RW;
	GPIO_Init(LCD_DataPort, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = LCD_BL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = LCDC_E | LCDO_E;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_Init(LCD_E_Port, &GPIO_InitStructure);
}

//将自定义字符写入LCD CGRAM
void WriteToCGRAM(uint8_t addr, const uint8_t * str)
{
	uint8_t i;

	LCD_WRCmd(0, addr);	//主显
	for(i=0;i<8;i++)
	{
			LCD_WRData(0, str[i]);
	}

	LCD_WRCmd(1, addr);	//客显
	for(i=0;i<8;i++)
	{
			LCD_WRData(1, str[i]);
	}
}


void LCDInit(void)
{
	LCD160x_IO_Init();

	// 初始化显示模式设置4次
	LCD_ModeInit4Bit(0);	// 4bit, 16x1-line, 5x8dots
	LCD_ModeInit4Bit(1);
	LCD160x_Delay_ms(30); //ouhs 20151106 //LCD160x_Delay_ms(3);		// 3ms

	LCD_ModeInit4Bit(0);
	LCD_ModeInit4Bit(1);
	LCD160x_Delay_ms(30); //ouhs 20151106 //LCD160x_Delay_ms(3);
//>>>>>>>>>>>>>>ouhs 20151106

	LCD_ModeInit4Bit(0);
	LCD_ModeInit4Bit(1);
	LCD160x_Delay_ms(30);

	LCD_ModeInit4Bit(0);
	LCD_ModeInit4Bit(1);
	LCD160x_Delay_ms(30);

//<<<<<<<<<<<<<<<<<<<<<<<<<<<

	LCD_WRCmd(0, 0x08);		// 关显示
	LCD_WRCmd(1, 0x08);
	LCD_WRCmd(0, 0x01);		// 清屏
	LCD_WRCmd(1, 0x01);
	LCD160x_Delay_ms(10);	//ouhs 20151106 LCD160x_Delay_ms(3);	// 3ms

	LCD_WRCmd(0, 0x06);		// 写入新数据后,光标向右移动,画面不移动
	LCD_WRCmd(1, 0x06);

	LCD_WRCmd(0, 0x0c);		// 显示功能开、有光标、 无闪烁
	LCD_WRCmd(1, 0x0c);

	LCDOpen();			//开背光
    LCD_BL_CTRL |= 2;//=1,客显与主显显示相同的内容


	WriteToCGRAM(0x40, Euro);
	WriteToCGRAM(0x48, Signal1);
	WriteToCGRAM(0x50, Signal3);
	WriteToCGRAM(0x58, Signal5);
	//WriteToCGRAM(0x60, Battery0);
	//WriteToCGRAM(0x68, Battery1);
	//WriteToCGRAM(0x70, Battery3);
	//WriteToCGRAM(0x78, Battery4);
	WriteToCGRAM(0x60, Battery5);

	PutsO_Only(Disp[0]);
	Puts1_Only(Disp[1]);
	//LCD160x_Test();
}

#if 0
const char  Disp1[] = "0123456789abcdef";
void LCD160x_Test(void)
{
	uint8_t i;
	while(1)
	{
		for(i=0; i<50; i++)
		{
			PutsO_Only(Disp1);
			PutsO_Only(Disp);
		}
		LCD160x_Delay_ms(1000);

	}
}
#endif

void LCDClose(void)
{
	LCD_BL_OFF();

//ccr2017-09-30	LCD_WRCmd(0, 0x01);
//ccr2017-09-30	LCD_WRCmd(1, 0x01);
}

void LCDOpen(void)
{
	LCD_BL_ON();
}

// 清除屏幕LinNum行显示
void LCDClearLine(uint8_t LCDn, uint8_t LinNum)
{
	uint8_t i;
	LCD_Pos(LCDn, LinNum, 0);
	for(i=0; i<DISLEN; i++)
	{
		LCD_WRData(LCDn, ' ');
	}
	LCD_Pos(LCDn, LinNum, 0); //重定位
}

void PutsC0(const char *str)
{//在顾客显示屏幕上显示内容,16位1行显示
	unsigned char addr = 0;

	memset(SaveDispC[0],' ',DISLEN);

	//LCD_WRCmd(1, 0x01);
	//LCD160x_Delay_ms(3);	// 3ms
	LCDClearLine(1, 0);
	for(;*str!=0;str++) //需要修改结束字符标识
	{
		//skip LCD_Pos(1, 0, addr);
		LCD_WRData(1, ASCII_Table[*str]);
		SaveDispC[0][addr]=*str;
        if( ++addr >= DISLEN)
			break;
	}

}

void PutsC1(const char *str)
{//在顾客显示屏幕上显示内容,16位1行显示
	unsigned char addr = 0;

	memset(SaveDispC[1],' ',DISLEN);

	//LCD_WRCmd(1, 0x01);
	//LCD160x_Delay_ms(3);	// 3ms
	LCDClearLine(1, 1);
	for(;*str!=0;str++) //需要修改结束字符标识
	{
		//skip LCD_Pos(1, 1, addr);
		LCD_WRData(1, ASCII_Table[*str]);
		SaveDispC[1][addr]=*str;
        if( ++addr >= DISLEN)
			break;
	}
}

void PutsC0_Only(const char *str)
{//在顾客显示屏幕上显示内容,16位1行显示
	unsigned char addr = 0;

	//LCD_WRCmd(1, 0x01);
	//LCD160x_Delay_ms(3);
	LCDClearLine(1, 0);
	for(;*str!=0;str++) //需要修改结束字符标识
	{
		//skip LCD_Pos(1, 0, addr);
		LCD_WRData(1, ASCII_Table[*str]);
        if( ++addr >= DISLEN)
			break;
	}

}
void PutsC1_Only(const char *str)
{//在顾客显示屏幕上显示内容,16位1行显示
	unsigned char addr = 0;

	//LCD_WRCmd(1, 0x01);
	//LCD160x_Delay_ms(3);
	LCDClearLine(1, 1);
	for(;*str!=0;str++) //需要修改结束字符标识
	{
		//skip LCD_Pos(1, 1, addr);
		LCD_WRData(1, ASCII_Table[*str]);
        if( ++addr >= DISLEN)
			break;
	}
}

//显示被保护的内容
void PutsC_Saved(void)
{
	PutsC0_Only(SaveDispC[0]);
    PutsC1_Only(SaveDispC[1]);
}

//只显示，同时保存内容
void PutsO(const char *str)
{//在操作员显示屏幕上显示内容,16位1行显示
	unsigned char addr = 0;

    if (LCD_BL_CTRL & 0x02)
        PutsC0(str);

	memset(SaveDispO[0],' ',DISLEN);
	//LCD_WRCmd(0, 0x01);
	//LCD160x_Delay_ms(3);
	LCDClearLine(0, 0);
	for(;*str!=0;str++) //需要修改结束字符标识
	{
		//skip LCD_Pos(0, 0, addr);
		SaveDispO[0][addr]=*str;
		LCD_WRData(0, ASCII_Table[*str]);
		if( ++addr >= DISLEN)
			break;
	}
}


//ccr2017-07-26 在第二行靠右显示，同时保存内容
void PutsO_Right(const char *str)
{//在操作员显示屏幕上显示内容,16位1行显示
	int addr=0;
    int i,l;

    l = strlen(str);
    if (l<DISLEN)
    {
        addr=DISLEN-l;
        memset(SaveDispO[0],' ',DISLEN);
        memcpy(SaveDispO[0]+addr,str,l);
    }
    else
        memcpy(SaveDispO[0],str,DISLEN);

    if (LCD_BL_CTRL & 0x02)
        PutsC0(SaveDispO[0]);

	//LCD_WRCmd(0, 0x01);
	//LCD160x_Delay_ms(3);
	LCDClearLine(0, 0);
	for(;addr<DISLEN;addr++) //需要修改结束字符标识
	{
		//skip LCD_Pos(0, 1, addr);
		LCD_WRData(0, ASCII_Table[SaveDispO[0][addr]]);
	}

}

//ccr20131120 在屏幕第一行指定位置显示字符
void PutsO_At(uint8_t ch,uint8_t addr)
{//在操作员显示屏幕上显示内容,16位1行显示

	//LCD_WRCmd(0, 0x01);
	//LCD160x_Delay_ms(3);
    if (addr<DISLEN)
    {
        SaveDispO[0][addr] = ch;
        addr=0;
        LCDClearLine(0, 0);
        for(addr=0;addr<DISLEN;addr++) //需要修改结束字符标识
        {
            //skip LCD_Pos(0, 1, addr);
            LCD_WRData(0, ASCII_Table[SaveDispO[0][addr]]);
        }
    }
}

//只显示，同时保存内容
void Puts1(const char *str)
{//在操作员显示屏幕上显示内容,16位1行显示
	unsigned char addr = 0;

    if (LCD_BL_CTRL & 0x02)
        PutsC1(str);

	memset(SaveDispO[1],' ',DISLEN);
	//LCD_WRCmd(0, 0x01);
	//LCD160x_Delay_ms(3);
	LCDClearLine(0, 1);
	for(;*str!=0;str++) //需要修改结束字符标识
	{
		//skip LCD_Pos(0, 1, addr);
		SaveDispO[1][addr]=*str;
		LCD_WRData(0, ASCII_Table[*str]);
		if( ++addr >= DISLEN)
			break;
	}

}
//-----------------------------------------------
//ccr20131120 在第二行靠右显示，同时保存内容
void Puts1_Right(const char *str)
{//在操作员显示屏幕上显示内容,16位1行显示
	int addr=0;
    int i,l;

    l = strlen(str);
    if (l<DISLEN)
    {
        addr=DISLEN-l;
        memset(SaveDispO[1],' ',DISLEN);
        memcpy(SaveDispO[1]+addr,str,l);
    }
    else
        memcpy(SaveDispO[1],str,DISLEN);

    if (LCD_BL_CTRL & 0x02)
        PutsC1(SaveDispO[1]);

	//LCD_WRCmd(0, 0x01);
	//LCD160x_Delay_ms(3);
	LCDClearLine(0, 1);
	for(;addr<DISLEN;addr++) //需要修改结束字符标识
	{
		//skip LCD_Pos(0, 1, addr);
		LCD_WRData(0, ASCII_Table[SaveDispO[1][addr]]);
	}

}


//ccr20131120 在屏幕第二行指定位置显示字符
void Puts1_At(uint8_t ch,uint8_t addr)
{//在操作员显示屏幕上显示内容,16位1行显示

	//LCD_WRCmd(0, 0x01);
	//LCD160x_Delay_ms(3);
    if (addr<DISLEN)
    {
        SaveDispO[1][addr] = ch;
        addr=0;
        LCDClearLine(0, 1);
        for(addr=0;addr<DISLEN;addr++) //需要修改结束字符标识
        {
            //skip LCD_Pos(0, 1, addr);
            LCD_WRData(0, ASCII_Table[SaveDispO[1][addr]]);
        }
    }
}


//只显示，不保存内容
void PutsO_Only(const char *str)
{//在操作员显示屏幕上显示内容,16位1行显示
	unsigned char addr = 0;

    if (LCD_BL_CTRL & 0x02)
        PutsC0_Only(str);

	//LCD_WRCmd(0, 0x01);
	//LCD160x_Delay_ms(3);
	LCDClearLine(0, 0);
	for(;*str!=0;str++) //需要修改结束字符标识
	{
		//skip LCD_Pos(0, 0, addr);
		LCD_WRData(0, ASCII_Table[*str]);
		if( ++addr >= DISLEN)
			break;
	}
}

//只显示，不保存内容
void Puts1_Only(const char *str)
{//在操作员显示屏幕上显示内容,16位1行显示
	unsigned char addr = 0;

    if (LCD_BL_CTRL & 0x02)
        PutsC1_Only(str);

	//LCD_WRCmd(0, 0x01);
	//LCD160x_Delay_ms(3);
	LCDClearLine(0, 1);
	for(;*str!=0;str++) //需要修改结束字符标识
	{
		//skip LCD_Pos(0, 1, addr);
		LCD_WRData(0, ASCII_Table[*str]);
		if( ++addr >= DISLEN)
			break;
	}
}

//显示被保护的内容
void PutsO_Saved(void)
{
	PutsO_Only(SaveDispO[0]);
	PutsC0_Only(SaveDispC[0]);
    Puts1_Only(SaveDispO[1]);
    PutsC1_Only(SaveDispC[1]);
}

//保护的LCD上显示的nrong内容
void Save_LCD(char *saveO,char *saveC)
{
	memcpy(saveO,SaveDispO,sizeof(SaveDispO));
	memcpy(saveC,SaveDispC,sizeof(SaveDispC));
}

//设置客显是否与主显同步显示
void LCDSet_Cust(char type)
{
    if (type)
        LCD_BL_CTRL |= 0x02;
    else
        LCD_BL_CTRL &= ~0x02;
}


/**
 * 在屏幕指定的ASCII字符位置显示电池容量
 *
 * @author EutronSoftware (2014-12-31)
 *
 * @param ctrl :0-不显示;=1测试并显示;=2显示
 */
#define VIN_FULL  VIN_8V0 
#define VIN_NONE  VIN_6V6
void mDrawPower(BYTE ctrl)
{
    static WORD gPower=0;

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

        if (gPower>=VIN_FULL)
            j=9;
        else if (gPower<=VIN_NONE)
            j = 0;
        else
            j = ((gPower-VIN_NONE)*9)/(VIN_FULL-VIN_NONE);
        Puts1_At(j +'0',0);
    }
}


/*********************************************************************************************************
** End Of File
*********************************************************************************************************/
