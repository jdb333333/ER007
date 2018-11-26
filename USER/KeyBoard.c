/****************************************Copyright (c)**************************************************
**                               Hunan EUTRON information device Co.,LTD.
**
**                                 http://www.eutron.com
**
**--------------File Info-------------------------------------------------------------------------------
** File name:       KeyBoardDrv.c
** Descriptions:    ��������
**
**------------------------------------------------------------------------------------------------------
** Created by:      Ruby
** Created date:    2011-12-06
** Version:         1.0
** Descriptions:    The original version
**
**------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Descriptions:
********************************************************************************************************/
#include "king.h"
#include "exthead.h"

#include "stm32f2xx.h"
#include "KeyBoard.h"
#include "TypeDef.h"
#include "interface.h"
#include "fsmc_sram.h"

#if defined(CASE_ETHERNET)
#include "netconf.h"  //ccr2015-01-22
#endif

#include "stm32f2x7_eth_bsp.h"

extern BYTE    KeyFrHost;       //ApplVar.Key code from host computer.
extern BYTE Bios_Key0_Key;			//    key number: 00 = No key

#define ADJINPUT()	if(key_putin >= KEY_BUFFER_SIZE) key_putin = 0

#if defined(CASE_ECR99) || defined(CASE_MCR30)
#define KB_ROW	0xF800
#endif

#define KEY_STATE_0 0
#define KEY_STATE_1 1
#define KEY_STATE_2 2
#define KEY_STATE_3 3

#define CLOSE_KEY	0x19

uint8_t dis_buf[KEY_BUFFER_SIZE]= {0}; //��ʾ����
uint8_t key_number[COMB_KEY_TOTAL],key_total;
uint8_t key_state = 0;
uint8_t comb_index = 0,flag_press = 0;
uint8_t bitflag_key = 0; /* bit0:���¼���־ bit1:�������±�־ bit7:���������־ */


#if CASE_ER260F
uint8_t key_value[]=					   /*  ɨ�����봫��Ӧ�ò�ļ�ֵһһ��Ӧ��: �����±����ɨ���� */
{
	 0x10,0x0f,0x0e,0x0d,0x0c,0x0b,0x0a,0x09,
	 0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,
	 0x18,0x17,0x16,0x15,0x14,0x13,0x12,0x11,
	 0x20,0x1f,0x1e,0x1d,0x1c,0x1B,0x1a,0x19,
	 0x28,0x27,0x26,0x25,0x24,0x23,0x22,0x21,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

#elif CASE_ECR100F
uint8_t key_value[]=
{
	 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x00,
	 0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x00,
	 0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x00,
	 0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x00,
	 0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
#elif CASE_ECR100S
uint8_t key_value[]=
{
	 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x00,
	 0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x00,
	 0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x00,
	 0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x00,
	 0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
#elif CASE_ECR99
uint8_t key_value[]=
{
	 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x00,
	 0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x00,
	 0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x00,
	 0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x00,
	 0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
#elif CASE_MCR30
uint8_t key_value[]=
{
	 0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x00,0x00,
	 0x13,0x14,0x15,0x16,0x17,0x18,0x00,0x00,
	 0x0D,0x0E,0x0F,0x10,0x11,0x12,0x00,0x00,
	 0x07,0x08,0x09,0x0A,0x0B,0x0C,0x00,0x00,
	 0x01,0x02,0x03,0x04,0x05,0x06,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	
	/*
	 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
	 0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,
	 0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,
	 0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,
	 0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
	 0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x30,
	 0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,
	 0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,0x40,
	*/
};
#endif

uint32_t keydown_flag = 0;	                 // ÿһλ������Ӧ���İ���״̬�����3λΪ1�������ֵΪ3�ļ�������
uint8_t keydown_flag2 = 0;

uint8_t key_putin = 0;
uint8_t key_getout = 0;

volatile uint32_t Times_Of_10ms = 0;
volatile uint32_t	Times_Of_10ms_ForNet=0;

uint8_t fBeep = true;
volatile uint16_t beep_len=0;
volatile uint8_t KeyScanEnable=0;

extern volatile BYTE DrawerOpen_time;

extern void TIM7_Init(void);

void KeyBoardDeInit(void);
void EXTILine_Init(void);
void KeyBoardInit(void)
{

#if defined(CASE_ECR99) || defined(CASE_MCR30)
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15; //�����ź�
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10; 	//���
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_5 | GPIO_Pin_6;	//���
	GPIO_Init(GPIOE, &GPIO_InitStructure);
#endif
	
	KeyBoardDeInit();

	EXTILine_Init();
	TIM7_Init();


}

#if defined(CASE_MCR30)

//���뽫���̵ĸ���������Ϊ��������������������ʹ�ð����ػ�ʱ�ᷴ������
void POWER_KeyINPUT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;	//KB_C0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
}
#endif

#if defined(CASE_ECR99) || defined(CASE_MCR30)
void keyscan(void)
{
	uint8_t i, j;
	uint16_t k_col;
	uint8_t key_mask[5] = {11,12,13,14,15};
	uint8_t key_mask_out[4] = {0,1,5,6};
	key_total = 0;
	
	for(i=0; i<7; i++)
	{
		GPIOD->BSRRL = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
		GPIOE->BSRRL = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_5 | GPIO_Pin_6;
		if(i<3)
		{
			GPIOD->BSRRH = (uint16_t)(1<<(8+i)); 
		}
		else
		{
			GPIOE->BSRRH = (uint16_t)(1<<key_mask_out[i-3]); 			
		}	
		
		j = 0xFF;
		while(j--);
		
		k_col = (GPIOE->IDR) & KB_ROW;
		if(k_col != KB_ROW)
		{
			for(j=0; j<5; j++)
			{
				if(!((k_col >> key_mask[j])&0x01))
				{
					key_total++;
					if(key_total >= COMB_KEY_TOTAL)
						key_total = COMB_KEY_TOTAL;
                    key_number[key_total - 1] = key_value[(i | j<<3)];	//��ȡ���µļ���
				}
			}
		}
	}
}

#elif defined(CASE_ECR100F) || defined(CASE_ECR100S)
void keyscan(void)
{
	uint8_t i, j;
	uint16_t k_col;
	key_total = 0;
	for(i=0; i<8; i++)
	{
		*(uint16_t *)(KB_ADDR) = ~(1<<i);
		j = 0xC0;//0x80;
		while(j--);
		k_col = *(__IO uint16_t*) (KB_ADDR);
		if(k_col != 0xFF)
		{
			for(j=0; j<8; j++)
			{
				if(!((k_col >> j)&0x01))
				{
					key_total++;
					if(key_total >= COMB_KEY_TOTAL)
						key_total = COMB_KEY_TOTAL;
                    key_number[key_total - 1] = key_value[(i | j<<3)];	//��ȡ���µļ���
				}
			}
		}
	}
}
#endif 


void keydown(void)
{
    uint8_t i;
    static uint8_t key_total_old;

	//ɨ�����
    keyscan();

	//����״̬����
    switch (key_state)
    {
        //������ʼ̬ �ޱպϼ�
        case KEY_STATE_0:
			flag_press = 0;
			comb_index = 0;
            key_total_old = 0;
            //�������£�ת������ȷ��̬
            if (key_total)
            {
                key_total_old = key_total;

                key_state = KEY_STATE_1;
            }
            break;

        //����ȷ��̬
        case KEY_STATE_1:
			//��������������ԭ״̬
            if (key_total_old != key_total) //��ʱ key_total >= 1
            {
	            //��ʼ̬���������ذ�����ʼ̬
            	if (flag_press == 0)
	            {
	                key_state = KEY_STATE_0;
	            }
	            //�ȴ�̬���������ذ����ȴ�̬
            	else
	            {
	                key_state = KEY_STATE_2;
	            }
            }
            //�����ȶ���������ϼ����У�ת��ȴ�̬
        	else
        	{

				//���¼�����
	            if (key_total > comb_index) //���߲�������ȣ����򰴼�Ӧ�����ȶ��ȴ�̬
				{
					//����һ��ͬʱ���µļ�����ϼ�����ͷ
		            if (0 == comb_index)
		            {
						comb_index = key_total;
						keydown_flag = 0;
						keydown_flag2 = 0;
						for (i = 0; i < key_total; i++)
		                {
							if(key_number[i]<32)
								keydown_flag |= (1UL<<key_number[i]);
							else
								keydown_flag2 |= (1UL<<(key_number[i]-32));

							if (key_number[i]>0)//ccr
							{
								dis_buf[key_putin++] =key_number[i];
								ADJINPUT();
							}

		                }
						flag_press = 1;
		            }
					//���¼��ӵ���ϼ�����β
		            else
		            {
		                for (i = 0; i < key_total; i++)
		                {
							if(key_number[i]<32)
							{
								if(!(keydown_flag&(1UL<<key_number[i])))
								{
                                    if (key_number[i]>0)//ccr
                                    {
                                        dis_buf[key_putin++] = key_number[i];
                                        comb_index++;
										ADJINPUT();
                                    }

								}
							}
							else
							{
								if(!(keydown_flag2&(1UL<<(key_number[i]-32))))
								{
                                    if (key_number[i]>0)//ccr
									{
										dis_buf[key_putin++] =key_number[i];
										comb_index++;
										ADJINPUT();

									}
								}
							}
			            }
		            }

					bitflag_key |= 0x81;	//���ڷ��ͱ��

	                key_state = KEY_STATE_2;

				}
				//�м��ͷ�
				else
				{
					//��������ֵ
					keydown_flag = 0;
					keydown_flag2 = 0;
					comb_index = key_total;
					for (i = 0; i < key_total; i++)
	                {
						if(key_number[i]<32)
							keydown_flag |= (1UL<<key_number[i]);
						else
							keydown_flag2 |= (1UL<<key_number[i]);
	                }

		            //���а����ͷţ�ת����������ʼ̬
	            	if (key_total == 0)
		            {
		            	flag_press = 0;
						comb_index = 0;
						keydown_flag = 0;
						keydown_flag2 = 0;
		                key_state = KEY_STATE_0;
						bitflag_key &= (~0x80);
		            }
		            //���а���δ�ͷţ�ת���������ȴ�̬
	            	else
		            {
		                key_state = KEY_STATE_2;
		            }
				}
			}
            break;

        //�ȴ�̬ �бպϼ����ȴ��¶���
        case KEY_STATE_2:
            if (key_total != key_total_old)
        	{
                key_total_old = key_total;

                key_state = KEY_STATE_1;
        	}
			break;
    	}

}

#ifdef DD_LOCK
uint8_t GetchLockVal(void)
{
	return (LockCurVal);//����Կ�����λ�õļ�ֵ
}
#endif


/** ��ȡ��������İ���
  * @param  None
  * @retval :key pressed
  */


uint8_t Getch(void)
{  //��ȡ����,�ڶ�ȡ����֮ǰ,��ʹ��BiosCmd_CheckKeyborad�������Ƿ��а���.

	if(key_getout!=key_putin) //ccr2017-05-15
	{
		Bios_Key0_Key=dis_buf[key_getout++]-1;
		if(key_getout >= KEY_BUFFER_SIZE)
			 key_getout = 0;
	}
    else if (KeyFrHost!=0xff)
    {
				//jdb2018-09-02
        //RESETBIT(ApplVar.ArrowsAlfa,KEY2ndLAYER);//ccr2017-12-18
        Bios_Key0_Key=KeyFrHost; //ccr2017-05-15
        KeyFrHost=0xff;
    }
    return (Bios_Key0_Key);
}
/** �������Ƿ�������
  * @param  None
  * @retval true:a key pressed
  */
uint8_t CheckKeyboard(void)
{
	if (key_putin==key_getout)
		return 0;
	else
	{
#if !defined(CASE_ASCIIDISPLAY)
//		if (!BIT(ApplVar.ScreenStat,LIGHTON))
			mLightLCDOn();
#if defined(CASE_ECR99) || defined(CASE_ECR100F) || defined(CASE_ECR100S)
        LED_On();
#endif
#endif
		return (dis_buf[key_getout]);
	}
}

/** ������ֹ���̷�����
  * @param  on_off:
  * @retval None
  */
void EnableBeep(uint8_t on_off)
{
	fBeep = on_off;
}

//ccr2017-02-17>>>>>>>>>>>>>>>>>>>>>>>>
extern void Time_Update(void);
extern uint32_t ETH_CheckFrameReceived(void);

extern void LwIP_Pkt_Handle(void);
extern void LwIP_Periodic_Handle(__IO uint32_t localtime);

extern __IO uint8_t EthLinkStatus;
extern __IO uint32_t LocalTime; /* this variable is used to create a time reference incremented by 10ms */
//ccr2017-02-17<<<<<<<<<<<<<<<<<<<<<<<<<

/** ����������̰�������,10ms�жϣ����ڷ�������Ǯ�䡢����
  * @brief  This function handles TIM7 global interrupt request.
  * @param  None
  * @retval None
  */
void TIM7_IRQHandler(void)
{
	//if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);

		Times_Of_10ms++;
#if defined(CASE_ECR99) || defined(CASE_ECR100F) || defined(CASE_ECR100S) ||defined(CASE_MCR30) //�жϷ�ʽ���Ա�͹��ĵ�ʵ��
		if (KeyScanEnable < 10)		//100ms
#endif		
		{
			keydown();
#if defined(CASE_ECR99) || defined(CASE_ECR100F) || defined(CASE_ECR100S) || defined(CASE_MCR30)
			KeyScanEnable++;
			KeyBoardDeInit();	
#endif		
		}

		if((bitflag_key & 0x01))
		{
			if(fBeep)
				Beep(1);
			Times_Of_10ms = 0;
		}
		else
		{
			if(!(bitflag_key & 0x80))
				Times_Of_10ms = 0;
			if(dis_buf[key_putin-1] != CLOSE_KEY)
				Times_Of_10ms = 0;
		}

		bitflag_key &= (~0x01);

		/* start beep: beep_len*10ms  */
		if(beep_len)
		{
			if (--beep_len==0)
				BEEP_OFF();
		}
		/* start DRAWER pulse output:DrawerOpen_time*10ms  */

		if(DrawerOpen_time)
		{
			if(--DrawerOpen_time == 0)
				DRAWER_OFF();
		}

		/* end DRAWER pulse output   */

		//ʹ�ü��̹ػ�
#if defined(CASE_MCR30)
		//
		if(Times_Of_10ms > 200)//����2���ӹػ�
		{
			TIM_SetCounter(TIM7, 0);
			TIM_Cmd(TIM7, DISABLE);
			BEEP_OFF();
			//
			//�ò�����Ҫ�����Ҫ������\�ر���ʾ\�رյ�Դ
			//
			//Save_ConfigVar();
#if defined(CASE_ASCIIDISPLAY)
			Copy_LCD(false);
			LCDClear();
			LCDClose();
#else
			mClearScreen();
			cClearScreen();
			mLightLCDOff();
#endif
			POWER_KeyINPUT();
			POWER_OFF();
			Times_Of_10ms = 0;
			while (1);
		}

#endif //CASE_MCR30

	}

#if defined(CASE_ETHERNET)//ccr2017-02-17>>>>>>>>>>>>
	Time_Update();	//ethernet time update
	//����5����û������⵽������ʹPHY����POWER DOWNģʽ
	//�岦���ߡ����������ϵ綼���Բ���LINK�жϣ����ǣ�������һֱû�����߲������ܲ���LINK�жϣ��������Ӹü��
/* 20180718 ����Щ��������·�������󴥷��ر�?
	if(Times_Of_10ms_ForNet < 30000)
	{
		Times_Of_10ms_ForNet++;
	}
	else if(Times_Of_10ms_ForNet == 30000)
	{
		Times_Of_10ms_ForNet++;
		if(Eth_Link_Status()==0)
			Eth_PHY_PowerModeCtrl(DISABLE);
	}
*/
#endif //ccr2017-02-17<<<<<<<<<<<<<<<

}


void KeyBoardDeInit(void)
{
#if defined(CASE_ECR100F) || defined(CASE_ECR100S)
	*(uint16_t *)(KB_ADDR) = 0;	// ��KB_C[7..0]����Ϊ�͵�ƽ
#elif defined(CASE_ECR99) || defined(CASE_MCR30)
	GPIOD->BSRRH = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
	GPIOE->BSRRH = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_5 | GPIO_Pin_6;
#endif
}


/*********************************************************************************************************
** Function name: EXTILine_Init
** Descriptions:  �ⲿ�ж��źų�ʼ���������������������жϡ��������жϡ�
*********************************************************************************************************/
void EXTILine_Init(void)
{
	EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;
	GPIO_InitTypeDef   GPIO_InitStructure;

	/* Enable GPIOG clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOG, ENABLE);
	/* Enable SYSCFG clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	// PB0�������ź�
	/* Configure pin as input floating */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Connect EXTI Line to Selected pin */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource0);   //PFO

	/* Configure EXTI Line */
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  	  // �½��ش���
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

    /* Configure the NVIC Preemption Priority Bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //ouhs 20140814
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

#if defined(CASE_ECR100F) || defined(CASE_ECR100S) //ECR100F��һ�������İ����ж��ź�
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //KB_INT
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource8);   //KB_INT
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line8;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  	  //������
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  
	NVIC_Init(&NVIC_InitStructure);
	
#elif defined(CASE_ECR99)  || defined(CASE_MCR30)  //ECR99ͨ��5���������źŲ����ж�
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource11);  
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource12); 
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource13); 
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource14); 
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource15); 
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line11 |EXTI_Line12 |EXTI_Line13 |EXTI_Line14 |EXTI_Line15;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  	  //�½���
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);	

	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  
	NVIC_Init(&NVIC_InitStructure);
#endif

}
//ccr clean keyboar data
void CleanKeyboard()
{
    key_putin = 0;
    key_getout = key_putin;

}
/*********************************************** End Of File *******************************************/
