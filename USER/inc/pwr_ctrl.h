
#ifndef __PWR_CTRL_H
#define __PWR_CTRL_H

#define POWERCTRL       1 //ouhs 1   //=1,允许进行电池检测


#define	VIN_Pin		   	GPIO_Pin_2
#define V_RTER				GPIO_Pin_0

#if defined(CASE_ECR100F) || defined(CASE_ECR100S)

#define BAT_CHG_Pin			GPIO_Pin_5
#define BAT_STAT_PORT		GPIOE
#define POWER_Pin				GPIO_Pin_6
#define POWER_PORT			GPIOE	

#define PRT_3V3_ON()			GPIOD->BSRRH = GPIO_Pin_7	//ResetBit
#define PRT_3V3_OFF()			GPIOD->BSRRL = GPIO_Pin_7	//SetBit

#elif defined(CASE_ECR99) || defined(CASE_MCR30)

#define BAT_CHG_Pin			GPIO_Pin_8
#define BAT_STAT_PORT		GPIOF
#define POWER_Pin				GPIO_Pin_9
#define POWER_PORT			GPIOF	

#define PRT_3V3_ON()		{};
#define PRT_3V3_OFF()		{};
#endif

#define POWER_ON()			POWER_PORT->BSRRL = POWER_Pin	//SetBit
#define POWER_OFF()			POWER_PORT->BSRRH = POWER_Pin	//ResetBit

#define STAT_CHARGING		0x01
#define STAT_DONE_IDLE	0x02
#define STAT_NOBATT_ERR 0x03

#define ADC1_DR_ADDR   ((uint32_t)0x4001204C)	
#define ADC2_DR_ADDR   ((uint32_t)0x4001214C)	//(APB2PERIPH_BASE + 0x2100) + 0x4C

//锂电池电压检测：Vad=Vbat-5.625     掉电阀值Vpfi=5.65V
//实测1：Vbat=8.1V，Vad=2.48V，计算理论AD值为：(2.48/3.3)*0xfff=3077,AD采样值为3120~3150
//所以，设定值应在理论值+修正值50

#define VIN_8V3	3376	//2.68V---3326+50		
#define VIN_8V2	3252	//2.58V---3202+50	
#define VIN_8V0	3003	//2.38V---2953+50
#define VIN_7V9	2880	//2.28V---2830+50	
#define VIN_7V8	2755	//2.18V---2705+50
#define VIN_7V4	2258	//1.78V---2208+50	
#define VIN_7V2	2010	//1.58V---1960+50
#define VIN_7V0	1762	//1.38V---1712+50
#define VIN_6V8	1514	//1.18V---1464+50
#define VIN_6V6	1266	//0.98V---1216+50

#define VIN_OK  VIN_7V0
#define VIN_LOW VIN_6V8

//纽扣电池检测标准值
#define	VRTC_2V5	0xB2B	//2.5V---3102---0xC1E-0x20
#define	VRTC_2V2	0x9D2	//2.2V---2730---0xAAA-0x20

//锂电池电量百分比自定义图标
#define BAT_PERCENT_NULL  0x84	//显示LCD1602自定义字符
#define BAT_PERCENT_20  	0x85
#define BAT_PERCENT_60  	0x86
#define BAT_PERCENT_100  	0x87
#define BAT_CHARGING			0x84

//检测系统供电方式
#define DC_DET_Pin		GPIO_Pin_2
#define DC_DET_GET()	((GPIOE->IDR & DC_DET_Pin) == 0)    /* TRUE--适配器供电, FALSE--电池供电*/


#define	GET_VIN_STATUS	1
#define	GET_VRTC_STATUS	2

#define PWR_BY_BAT      0x01    //为电池供电

#define PWR_STAT_OK		1

#define PWR_WARNING1	0x02        //供电电池电压报警点1
#define PWR_WARNING2	0x04        //供电电池电压报警点2

#define PWR_BUT_WARNING1     0x10   //纽扣电池电压报警点1
#define PWR_BUT_WARNING2     0x20   //纽扣电池电压报警点2

#if (!defined(DEBUGBYPC))
extern __IO uint16_t Vin_Vrtc_Val[2];
extern uint8_t PowerFlags;		//检测到的电源状态
/*
    Bit0:=0,为适配器供电;=1,为电池供电
    Bit1:=1,为电池供电时,出现报警PWR_WARNING1(电压次低)
    Bit2:=1,为电池供电时,出现报警PWR_WARNING2(电压过低)

    Bit4:=1,纽扣电池出现报警PWR_WARNING1(电压次低)
    Bit5:=1,纽扣电池出现报警PWR_WARNING2(电压过低)
*/

#define BATTERYTooLow(v)  ((PowerFlags & (v))==(v))
#define BatteryVoltage()   (Vin_Vrtc_Val[1])
#define BatteryExist()   (Vin_Vrtc_Val[1]<0xfff)//检测是否有电池(4095)

void pwr_Init(void);
uint8_t pwrGetStatus(uint8_t GetStatus);
void EnterSTOP(void);
uint8_t BattaryCheck(uint8_t disp);
#endif


#endif	//__PWR_CTRL_H
