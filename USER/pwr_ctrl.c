
#include "stm32f2xx_it.h"
#include "pwr_ctrl.h"

#if defined(CASE_ASCIIDISPLAY)
#include "lcd_160x2.h"
#else
#if defined(CASE_ECR100S)
#include "lcd_st7565r.h"
#elif defined(CASE_MCR30)
#include "lcd_st7567.h"
#endif
#endif
                            

__IO uint16_t Vin_Vrtc_Val[2]; // Vin_Vrtc_Val[0]---��ص�ѹ ,Vin_Vrtc_Val[1]---RTC��ѹ

uint8_t PowerFlags;		//��⵽�ĵ�Դ״̬
/*
    Bit0:=0,Ϊ����������;=1,Ϊ��ع���
    Bit1:=1,Ϊ��ع���ʱ,���ֱ���PWR_WARNING1(��ѹ�ε�)
    Bit2:=1,Ϊ��ع���ʱ,���ֱ���PWR_WARNING2(��ѹ����)

    Bit4:=1,Ŧ�۵�س��ֱ���PWR_WARNING1(��ѹ�ε�)
    Bit5:=1,Ŧ�۵�س��ֱ���PWR_WARNING2(��ѹ����)
*/

void ADC1_DMA_Config(void);
void pwrIO_Init(void);
void pwrVolDetecCtrl(uint8_t NewState);
void pwrADCCtrl(FunctionalState NewState);


extern void	USART_printf (const uint8_t *format, ...);

/**************************************************************************/
/* ��Դ�����ʼ������
***************************************************************************/
void pwr_Init(void)
{
	pwrIO_Init();
	ADC1_DMA_Config();
	pwrADCCtrl(ENABLE);
}

void pwrIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_InitStructure.GPIO_Pin = DC_DET_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* ADC Channel 10 -> PC0 12 -> PC2*/
	GPIO_InitStructure.GPIO_Pin = VIN_Pin ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	//ϵͳ��Դ����
	GPIO_InitStructure.GPIO_Pin = POWER_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(POWER_PORT, &GPIO_InitStructure);

	//��ӡ���߼���Դ����
#if defined(CASE_ECR100F) || defined(CASE_ECR100S)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	PRT_3V3_ON();
#endif
}


/**************************************************************************/
/* ADC1��������:ADC1_12���ڼ������Դ��ѹ  ADC1_18 RTC���
***************************************************************************/
void ADC1_DMA_Config(void)
{
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	DMA_InitTypeDef	DMA_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
#if (1)
	/* DMA2_Stream2 channel1 configuration **************************************/
	DMA_DeInit(DMA2_Stream0);
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_ADDR;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&Vin_Vrtc_Val;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = 2;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;//DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);
	/* DMA2_Stream2 enable */
	DMA_Cmd(DMA2_Stream0, ENABLE);
#endif
	/* ADC Common Init */
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div8;//ADC_Prescaler_Div2;	//testonly
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;	//ADC_DMAAccessMode_1;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_15Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	/* ADC1 regular channels 12 18 configuration*/

	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;	//ת������������
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 2;	   //ɨ��ͨ����
	ADC_Init(ADC1, &ADC_InitStructure);

	/* ADC1 regular channels 12 18 configuration */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 1, ADC_SampleTime_15Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_18, 2, ADC_SampleTime_15Cycles);

	/* Enable DMA request after last transfer (Single-ADC mode) */
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

	//ADC_DMACmd(ADC1, ENABLE);

	/* Enable ADC1 */
	//ADC_Cmd(ADC1, ENABLE);

	/* Start ADC1 Software Conversion */
	//ADC_SoftwareStartConv(ADC1);
}

/**************************************************************************/
/* ��ѹ���ADC���ܿ�����رա�������NewState=ENABLE---������DISABLE---�ر�
***************************************************************************/
void pwrADCCtrl(FunctionalState NewState)
{
	// ����ADC�������һ��PFO�����źţ�����
	ADC_DMACmd(ADC1, NewState);

	/* Enable VBAT channel */
  //ouhs ������ᵼ��VBAT��������,ȡ����VBAT���ŵ�ѹ�ļ��
	ADC_VBATCmd(NewState);

	ADC_Cmd(ADC1, NewState);
	ADC_SoftwareStartConv(ADC1);
	if(NewState == DISABLE)
	{//�رյ�ѹ���ʱ��0
		Vin_Vrtc_Val[0] = 0;
		Vin_Vrtc_Val[1] = 0;
	}
}

/**************************************************************************/
/*@param	GET_VIN_STATUS
			GET_VRTC_STATUS
  @retval	PWR_STAT_OK
			PWR_WARNING1
			PWR_WARNING2
***************************************************************************/
/*
    Bit0:=0,Ϊ����������;=1,Ϊ��ع���
    Bit1:=1,Ϊ��ع���ʱ,���ֱ���PWR_WARNING1(��ѹ�ε�)
    Bit2:=1,Ϊ��ع���ʱ,���ֱ���PWR_WARNING2(��ѹ����)

    Bit4:=1,Ŧ�۵�س��ֱ���PWR_WARNING1(��ѹ�ε�)
    Bit5:=1,Ŧ�۵�س��ֱ���PWR_WARNING2(��ѹ����)
*/
uint16_t Vrt_val_temp = 0;
uint8_t pwrGetStatus(uint8_t GetStatus)
{
	uint8_t status=0;
	uint16_t Vin_Val, Vrtc_Val;
	uint8_t i=100;
	PowerFlags = 0;

#if POWERCTRL
	//return 0;//testonly skip test
	if (!DC_DET_GET())
	{//Ϊ��ع���
		PowerFlags = PWR_BY_BAT;//=1,Ϊ��ع���

		if (GetStatus == GET_VIN_STATUS)
		{
			Vin_Val = Vin_Vrtc_Val[0];
#ifdef FOR_DEBUG			
			xprintf("VIN = %d\n", Vin_Val);	//testonly by ouhs
#endif			
		  if(Vin_Val > VIN_OK)
			{
				status = PWR_STAT_OK;
			}
			else if(Vin_Val > VIN_LOW)
			{
				status = PWR_WARNING1;
				PowerFlags = PWR_BY_BAT | PWR_WARNING1;//Ϊ��ع���ʱ,���ֱ���PWR_WARNING1(��ѹ�ε�)����ʾ���
			}
			else
			{
				status = PWR_WARNING2;
				PowerFlags = PWR_BY_BAT | PWR_WARNING2;//Ϊ��ع���ʱ,���ֱ���PWR_WARNING2(��ѹ����),ֹͣʹ��
			}
		}
	}
#endif
	if(GetStatus == GET_VRTC_STATUS)
	{
		if(Vrt_val_temp==0)
		{
			while(i--) __NOP(); i=100;
			Vrt_val_temp = Vin_Vrtc_Val[1]; //CPU�ڲ�1/2��ѹ��AD����
			while(i--) __NOP();
			Vrt_val_temp += Vin_Vrtc_Val[1];

			//ouhs ��CPU�ڲ���ȡ��VBAT��ѹ��������ᵼ��VBAT��������
			//ֻ�ڿ���ʱ��VBAT��RTC��ѹ���һ��
			ADC_VBATCmd(DISABLE);
		}
#ifdef FOR_DEBUG
		xprintf("\r\nVrtc = %d  \n", Vrt_val_temp);	//test by ouhs
#endif
		if(Vrt_val_temp > VRTC_2V5)
		{
			status = PWR_STAT_OK;
		}
		else if(Vrt_val_temp > VRTC_2V2)
		{
			status = PWR_WARNING1;
			PowerFlags |= PWR_BUT_WARNING1;//Ŧ�۵�س��ֱ���PWR_WARNING1(��ѹ�ε�)
		}
		else
		{
			status = PWR_WARNING2;
			PowerFlags |= PWR_BUT_WARNING2;//Ŧ�۵�س��ֱ���PWR_WARNING2(��ѹ����)
		}
	}
	return status;
}

/**************************************************************************/
/* ���﮵�س��״̬
���оƬ״ָ̬ʾ��
	�͵�ƽ�����ڳ��
	�ߵ�ƽ�������ɻ�ر�״̬�����״̬;
	��˸��ʵ��û�е��ʱ����˸����6ms(��4ms����2ms)

û�г��оƬʱ��һֱΪ�ߵ�ƽ��
***************************************************************************/
uint8_t pwrBATStatus(void)
{
	uint8_t status = 0;
	uint8_t cnt=0;
	uint16_t t1,t2,t3,t4;

	do
	{
		t1 = (BAT_STAT_PORT->IDR & BAT_CHG_Pin);
		msDelay(3);
		t2 = (BAT_STAT_PORT->IDR & BAT_CHG_Pin);
		msDelay(2);
		t3 = (BAT_STAT_PORT->IDR & BAT_CHG_Pin);
		msDelay(2);
		t4 = (BAT_STAT_PORT->IDR & BAT_CHG_Pin);
	}while((++cnt<3) && (t1==t2)&&(t2==t3)&&(t3==t4));
		
	if(cnt==3)
	{
		if(t1)
			status = STAT_DONE_IDLE;//�ߵ�ƽ�������ɻ�ر�״̬�����״̬;
		else
			status = STAT_CHARGING;	//�͵�ƽ�����ڳ��
	}
	else
	{
		status = STAT_NOBATT_ERR;	//��˸��û�е�ػ������
	}	
	
	return status;
}

/**************************************************************************/
/* ���﮵�ص�������ʾ
***************************************************************************/
uint8_t BattaryCheck(uint8_t disp)
{
	uint16_t BattVal=0;
	uint16_t reVal=255;
	uint8_t DispBattBuff[16];
	uint8_t i;
	static uint8_t cnt=0;//cnt��tmp����������������(Ƶ������)
	static uint16_t tmp=0;
   
	if (!disp) tmp=0;	
	for(i=0;i<3;i++)
	{
		BattVal += Vin_Vrtc_Val[0];
		msDelay(1);
	}
	BattVal /= 3;
#ifdef FOR_DEBUG
  //xprintf("\r\nBattVal = %d  \n", BattVal);
#endif
	if (!DC_DET_GET())  //��ع���
	{	
		if (BattVal>VIN_6V6)
		{
			reVal= (BattVal-VIN_6V6)*100 / (VIN_7V9-VIN_6V6);
			if (reVal>100) reVal=100;
		}
		else
			reVal=0;
		if (disp && (tmp != reVal) && (cnt++ == 100))
		{
			tmp = reVal;
			cnt = 0;
			sprintf(DispBattBuff, "BATTERY: %d%%", (uint8_t)reVal);
#if !defined(OD_4ROW)	
			PutsO(DispBattBuff);
#else
	//
#endif	
		}
	}
	else //��Դ����������
	{
		i = pwrBATStatus();
		if(i==STAT_NOBATT_ERR) //û�а�װ���	�����
		{
			reVal = 120;
		}
		else if(i==STAT_DONE_IDLE)
		{			
			if(BattVal>VIN_6V6)//�е�أ�û�г�� ��ص�ѹ��Ҫ��ѹ8.4*95%=7.98V���²��ٴγ��	
			{
				reVal = 100; //��Ϊ���紦��
			}
			else	//û�е�أ�û�г��оƬ
			{
				reVal = 120;
			}			
		} else if(i==STAT_CHARGING)
		{//�е�� �ڳ��
			if (BattVal>VIN_6V6)
			{
				reVal= (BattVal-VIN_6V6)*100 / (VIN_8V2-VIN_6V6);//���ʱ����ѹ��������8.4V��������û�г�����
				if (reVal>100) reVal=100;
			}
			else
				reVal=0;	
		}
		
		if (disp && (reVal<=100) && (tmp != reVal) && (cnt++ == 100))
		{
			tmp = reVal;
			cnt = 0;
			sprintf(DispBattBuff, "BATTERY: %d%%", (uint8_t)reVal);
#if !defined(OD_4ROW)				
			PutsO(DispBattBuff);
			PutsO_At(BAT_CHARGING, 15);
#else
	//
#endif			
		}				
	}
	
	return  reVal; 
}


/**>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>**/
/**>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>**/


/**************************************************************************/
/* ����STOPģʽ---�͹���
***************************************************************************/
void SYSCLKConfig_STOP(void);
void EnterSTOP(void)
{
	//ִ��EnterSTOPǰӦ�ر�LCD�����Ա���
#if defined(CASE_ASCIIDISPLAY)
	LCDClose();//ccr2017-09-30
#else
	mLightLCDOff();
#endif	
	//�رմ�ӡ���߼���Դ
	PRT_3V3_OFF();

	//CPU ����STOPģʽ
	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

	//�˳�STOPģʽ��,ʹ���ⲿʱ��HSE��PLL����ѡ��PLL��Ϊʱ��Դ
	SYSCLKConfig_STOP();

	//������ӡ���߼���Դ
	PRT_3V3_ON();
#if defined(CASE_ASCIIDISPLAY)	
    LCDOpen();//ccr2017-09-30
#else
	mLightLCDOn();
#endif
}

/**
  * @brief  Configures system clock after wake-up from STOP: enable HSE, PLL
  *         and select PLL as system clock source.
  * @param  None
  * @retval None
  */
void SYSCLKConfig_STOP(void)
{
  /* After wake-up from STOP reconfigure the system clock */
  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);

  /* Wait till HSE is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET)
  {}

  /* Enable PLL */
  RCC_PLLCmd(ENABLE);

  /* Wait till PLL is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
  {}

  /* Select PLL as system clock source */
  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

  /* Wait till PLL is used as system clock source */
  while (RCC_GetSYSCLKSource() != 0x08)
  {}
}

/**<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<**/
/**<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<**/

