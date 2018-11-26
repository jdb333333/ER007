
#if 1
#include "stm32f2xx.h"
#include "monitor.h"
#include "nand.h"
#include "ftl.h"

#include "usb_msc.h"

static void DispMenu(void);

/*
*********************************************************************************************************
*********************************************************************************************************
*/
void Test_USBDevice (void) 
{
	uint32_t i;
	uint16_t usLBN = 0;
	char *ptr;
	
	xprintf("Test_USBDevice...\r\n");	
	
	//NAND_DispBadBlockInfo();
	
	/* ɨ����Ի���(��ʱ�ܳ�) */
#if 0
	for (i = 0; i < NAND_BLOCK_SIZE; i++)
	{
		if (NAND_ScanBlock(i) == NAND_OK)
		{
			xprintf("Scan Block%d, Ok\r\n", i);
		}
		else
		{
			xprintf("Scan Block%d, Err\r\n", i);
		}
	}
#endif	

	/* ����FSMC����NAND Flash�� ��λNAND Flash���ؽ�LUT�� */
	if (FTL_Init() == 0)
	{
		xprintf("NAND Init() Ok\r\n");
	}
	else
	{
		/* ��������ʽ�Ĳ�Ʒ�в�����Ϊ��Ԥ�ķ�ʽ�����ͼ���ʽ�� */
		/* �Զ����nand flash�Ƿ�����˵ͼ���ʽ�������û����ִ�и�ʽ����2�룩 */
		xprintf("NAND Init() Error! \r\n");
		xprintf("Start Format(Low Level) NAND Flash......\r\n");
		//NAND_Format();
		if(FTL_Format())
			xprintf("NAND Flash Format Failed\r\n");
		else
			xprintf("NAND Flash Format Ok\r\n");	
	}

	DispMenu();
	usbd_OpenMassStorage();		/* ��ʼ��USB Device������ΪMass Storage */

	while(1)
	{
		ptr = ConsoleLine;
		ConsoleLine[0]=0;
		get_line(ptr, sizeof(ConsoleLine));
		switch (*ptr++)
		{
			case '1':	/* �Ƴ�U��, ���ģ��U�̰γ� */
				xprintf("��1 - �Ƴ�U�̡�\r\n");
				usbd_CloseMassStorage();
				break;

			case '2':	/* ʹ��U�̣����ģ��U�̲��� */
				xprintf("��2 - ʹ��U�̡�\r\n");
				usbd_OpenMassStorage();
				break;

			case '3':	/* �ͼ���ʽ��(�����ƻ�������) */
				xprintf("Start Format(Low Level) NAND Flash......\r\n");
				FTL_Format();//NAND_Format();
				xprintf("NAND Flash Format Ok\r\n");
				break;

			case 'z':
			case 'Z':
				if (usLBN > 0)
				{
					usLBN--;
				}
				else
				{
					xprintf("�Ѿ�����ǰ\r\n");
				}
				//NAND_DispLogicPageData(usLBN);
				break;

			case 'x':
			case 'X':
				if (usLBN < MAX_LOG_BLOCKS_PER_ZONE - 1)
				{
					usLBN++;
				}
				else
				{
					xprintf("�Ѿ������\r\n");
				}
				//NAND_DispLogicPageData(usLBN);
				break;
				
			case 'Q':
        return;
			
			default:
				DispMenu();
				break;
		}
	}	
}

/*
*********************************************************************************************************
*	�� �� ��: DispMenu
*	����˵��: ��ʾ������ʾ�˵�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispMenu(void)
{
	xprintf("\r\n------------------------------------------------\r\n");
	xprintf("��ѡ���������:\r\n");
	xprintf("1 - ����Ƴ�U��\r\n");
	xprintf("2 - ���ʹ��U��\r\n");
	xprintf("3 - �ͼ���ʽ��U��\r\n");
	xprintf("Z - ��ʾǰһҳ���ݣ��߼�ҳ��\r\n");
	xprintf("X - ��ʾ��һҳ���ݣ��߼�ҳ��\r\n");
	xprintf("Q - �˳� \r\n");
}





#else	//����MDK RL USB device
//---------------------------------------------------------------------------------------------------
#include <RTL.h>
#include <File_Config.h>
#include <rl_usb.h>

FAT_VI *nand0;  
Media_INFO info;

void Test_USBDevice (void) 
{
	xprintf("Test_USBDevice...\r\n");
	
  nand0 = ioc_getcb (NULL);
  if (ioc_init (nand0) == 0) {
    ioc_read_info (&info, nand0);
    usbd_init();                           
    usbd_connect(__TRUE);                   
	} else {
    xprintf("Memory Failure!");
  }
	
}

#endif

