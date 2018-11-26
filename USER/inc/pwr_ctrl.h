
#ifndef __PWR_CTRL_H
#define __PWR_CTRL_H

#define POWERCTRL       1 //ouhs 1   //=1,������е�ؼ��


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

//﮵�ص�ѹ��⣺Vad=Vbat-5.625     ���緧ֵVpfi=5.65V
//ʵ��1��Vbat=8.1V��Vad=2.48V����������ADֵΪ��(2.48/3.3)*0xfff=3077,AD����ֵΪ3120~3150
//���ԣ��趨ֵӦ������ֵ+����ֵ50

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

//Ŧ�۵�ؼ���׼ֵ
#define	VRTC_2V5	0xB2B	//2.5V---3102---0xC1E-0x20
#define	VRTC_2V2	0x9D2	//2.2V---2730---0xAAA-0x20

//﮵�ص����ٷֱ��Զ���ͼ��
#define BAT_PERCENT_NULL  0x84	//��ʾLCD1602�Զ����ַ�
#define BAT_PERCENT_20  	0x85
#define BAT_PERCENT_60  	0x86
#define BAT_PERCENT_100  	0x87
#define BAT_CHARGING			0x84

//���ϵͳ���緽ʽ
#define DC_DET_Pin		GPIO_Pin_2
#define DC_DET_GET()	((GPIOE->IDR & DC_DET_Pin) == 0)    /* TRUE--����������, FALSE--��ع���*/


#define	GET_VIN_STATUS	1
#define	GET_VRTC_STATUS	2

#define PWR_BY_BAT      0x01    //Ϊ��ع���

#define PWR_STAT_OK		1

#define PWR_WARNING1	0x02        //�����ص�ѹ������1
#define PWR_WARNING2	0x04        //�����ص�ѹ������2

#define PWR_BUT_WARNING1     0x10   //Ŧ�۵�ص�ѹ������1
#define PWR_BUT_WARNING2     0x20   //Ŧ�۵�ص�ѹ������2

#if (!defined(DEBUGBYPC))
extern __IO uint16_t Vin_Vrtc_Val[2];
extern uint8_t PowerFlags;		//��⵽�ĵ�Դ״̬
/*
    Bit0:=0,Ϊ����������;=1,Ϊ��ع���
    Bit1:=1,Ϊ��ع���ʱ,���ֱ���PWR_WARNING1(��ѹ�ε�)
    Bit2:=1,Ϊ��ع���ʱ,���ֱ���PWR_WARNING2(��ѹ����)

    Bit4:=1,Ŧ�۵�س��ֱ���PWR_WARNING1(��ѹ�ε�)
    Bit5:=1,Ŧ�۵�س��ֱ���PWR_WARNING2(��ѹ����)
*/

#define BATTERYTooLow(v)  ((PowerFlags & (v))==(v))
#define BatteryVoltage()   (Vin_Vrtc_Val[1])
#define BatteryExist()   (Vin_Vrtc_Val[1]<0xfff)//����Ƿ��е��(4095)

void pwr_Init(void);
uint8_t pwrGetStatus(uint8_t GetStatus);
void EnterSTOP(void);
uint8_t BattaryCheck(uint8_t disp);
#endif


#endif	//__PWR_CTRL_H
