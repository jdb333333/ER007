
#ifndef __LCD_KS0108_H
#define __LCD_KS0108_H

#include "TypeDef.h"



//A0(RS),A1(RW),A2(CS1),A3(CS2)
#define LCD_KS0108_L_WCMD_ADDR		(LCD_ADDR+0x00)	//A3-A0:0000 	//×ó±ß
#define LCD_KS0108_R_WCMD_ADDR		(LCD_ADDR+0x10)	//A3-A0:1000	//ÖÐ¼ä
#define LCD_KS0108_M_WCMD_ADDR		(LCD_ADDR+0x08)	//A3-A0:0100	//ÓÒ±ß

#define LCD_KS0108_L_RSTAT_ADDR		(LCD_ADDR+0x04)	//A3-A0:0010
#define LCD_KS0108_R_RSTAT_ADDR		(LCD_ADDR+0x14)	//A3-A0:1010
#define LCD_KS0108_M_RSTAT_ADDR		(LCD_ADDR+0x0C)	//A3-A0:0110

#define LCD_KS0108_L_WDATA_ADDR		(LCD_ADDR+0x02)	//A3-A0:0001
#define LCD_KS0108_R_WDATA_ADDR  	(LCD_ADDR+0x12)	//A3-A0:1001
#define LCD_KS0108_M_WDATA_ADDR  	(LCD_ADDR+0x0A)	//A3-A0:0101

#define LCD_KS0108_DUMMY_ADDR 	(LCD_ADDR+0x1A)	//A3-A0:1101


#define KS0108_BL_ON()			GPIOB->BSRRL = GPIO_Pin_2
#define KS0108_BL_OFF()			GPIOB->BSRRH = GPIO_Pin_2


void mInitialLCD(void);
void mClearScreen( void );
void mSetContrast(uint8_t contrast);

void mLightLCDOn(void);
void mLightLCDOff(void);
//void mSetInvAttr(void);
//void mClearInvAttr(void);
void mSetState(uint8_t NewState);
void mEnCursor(uint8_t NewState);
void mDispCursor(void);
void mMoveCursor(uint8_t x, uint8_t y);
void mRefreshLine(uint8_t LinNum);
void mDrawChar(uint8_t ColNum, uint8_t LinNum,uint8_t hzFlag);
void mDispCharXY(WORD ch, BYTE x, BYTE y);
void mDispStringXY(BYTE* str, BYTE x, BYTE y);

void  mDispString(BYTE y);

#endif //__LCD_KS0108_H


