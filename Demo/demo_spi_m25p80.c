#include "stm32f2xx.h"
#include "typedef.h"
#include "ff.h"		
#include "monitor.h"
#include "SysTick.h"
#include "demo_spi_m25p80.h"
#include "spi_m25p80.h"

#define TEST_ADDR		0			/* ��д���Ե�ַ */
#define TEST_SIZE		4096		/* ��д�������ݴ�С */

#define FiscalMemSize (FiscalMem_SPI_MAXADDR+1)

/* �������ļ��ڵ��õĺ������� */
static void sfDispMenu(void);
static void sfReadTest(void);
static void sfWriteTest(void);
static void sfErase(void);
static void sfViewData(uint32_t _uiAddr);
static void sfWriteAll(uint8_t _ch);
static void sfTestReadSpeed(void);


/*
*********************************************************************************************************
*	�� �� ��: DemoSpiFlash
*	����˵��: ����EEPROM��д����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DemoSpiFlash_FM(void)
{
	char *ptr;
	uint32_t ChipID,ChipRES;
	uint32_t uiReadPageNo = 0;
	
	FM_Init();
	
	if(FMDetect()==ERROR)
	{
		xprintf("ERROR:Fiscal Memory is not present!");
		return;
	}
	else
	{
		ChipID = FiscalMem_ReadID();
		ChipRES = FiscalMem_ReadRES();
		xprintf("��⵽����Flash, ID = %08x, RES = %08x",ChipID,ChipRES);
		xprintf("    ���� : %dM�ֽ�, ������С : %d�ֽ�\r\n", FiscalMemSize/(1024*1024), FiscalMem_SPI_PAGESIZE);
	}
		
	sfDispMenu();		/* ��ӡ������ʾ */
	
	while(1)
	{
		ptr = ConsoleLine;
		ConsoleLine[0]=0;
		get_line(ptr, sizeof(ConsoleLine));

		switch (*ptr++)
		{
				case '1':
					xprintf("\r\n��1 - ������Flash, ��ַ:0x%X,����:%d�ֽڡ�\r\n", TEST_ADDR, TEST_SIZE);
					sfReadTest();		/* ������Flash���ݣ�����ӡ������������ */
					break;

				case '2':
					xprintf("\r\n��2 - д����Flash, ��ַ:0x%X,����:%d�ֽڡ�\r\n", TEST_ADDR, TEST_SIZE);
					sfWriteTest();		/* д����Flash���ݣ�����ӡд���ٶ� */
					break;

				case '3':
					xprintf("\r\n��3 - ������������Flash��\r\n");
					sfErase();			/* ��������Flash���ݣ�ʵ���Ͼ���д��ȫ0xFF */
					break;

				case '4':
					xprintf("\r\n��4 - д��������Flash, ȫ0x55��\r\n");
					sfWriteAll(0x55);			/* ��������Flash���ݣ�ʵ���Ͼ���д��ȫ0xFF */
					break;

				case '5':
					xprintf("\r\n��5 - д��������Flash, ȫ0xAA��\r\n");
					sfWriteAll(0xAA);			/* ��������Flash���ݣ�ʵ���Ͼ���д��ȫ0xFF */
					break;

				case '6':
					xprintf("\r\n��6 - ����������Flash, %dM�ֽڡ�\r\n", (FiscalMem_SPI_MAXADDR+1)/(1024*1024));
					sfTestReadSpeed();		/* ����������Flash���ݣ������ٶ� */
					break;

				case 'z':
				case 'Z': /* ��ȡǰ1K */
					if (uiReadPageNo > 0)
					{
						uiReadPageNo--;
					}
					else
					{
						xprintf("�Ѿ�����ǰ\r\n");
					}
					sfViewData(uiReadPageNo * 1024);
					break;

				case 'x':
				case 'X': /* ��ȡ��1K */
					if (uiReadPageNo < FiscalMemSize / 1024 - 1)
					{
						uiReadPageNo++;
					}
					else
					{
						xprintf("�Ѿ������\r\n");
					}
					sfViewData(uiReadPageNo * 1024);
					break;
			case 'Q':
         return;
			
				default:
					sfDispMenu();	/* ��Ч������´�ӡ������ʾ */
					break;
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: sfReadTest
*	����˵��: ������Flash����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfReadTest(void)
{
	uint16_t i;
	int32_t iTime1, iTime2;
	uint8_t buf[TEST_SIZE];

	/* ��ʼ��ַ = 0�� ���ݳ���Ϊ 256 */
	iTime1 = GetSystemTimer();	/* ���¿�ʼʱ�� */
	FiscalMem_ReadBuffer(buf, TEST_ADDR, TEST_SIZE);
	iTime2 = GetSystemTimer();	/* ���½���ʱ�� */
	xprintf("������Flash�ɹ����������£�\r\n");

	/* ��ӡ���� */
	for (i = 0; i < TEST_SIZE; i++)
	{
		xprintf(" %02X", buf[i]);

		if ((i & 31) == 31)
		{
			xprintf("\r\n");	/* ÿ����ʾ16�ֽ����� */
		}
		else if ((i & 31) == 15)
		{
			xprintf(" - ");
		}
	}

	/* ��ӡ���ٶ� */
	xprintf("���ݳ���: %d�ֽ�, ����ʱ: %dms, ���ٶ�: %d Bytes/s\r\n", TEST_SIZE, iTime2 - iTime1, (TEST_SIZE * 1000) / (iTime2 - iTime1));
}


/*
*********************************************************************************************************
*	�� �� ��: sfTestReadSpeed
*	����˵��: ���Դ���Flash���ٶȡ���ȡ��������Flash�����ݣ�����ӡ���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfTestReadSpeed(void)
{
	uint16_t i;
	int32_t iTime1, iTime2;
	uint8_t buf[TEST_SIZE];
	uint32_t uiAddr;

	/* ��ʼ��ַ = 0�� ���ݳ���Ϊ 256 */
	iTime1 = GetSystemTimer();	/* ���¿�ʼʱ�� */
	uiAddr = 0;
	for (i = 0; i < FiscalMemSize / TEST_SIZE; i++, uiAddr += TEST_SIZE)
	{
		FiscalMem_ReadBuffer(buf, uiAddr, TEST_SIZE);
	}
	iTime2 = GetSystemTimer();	/* ���½���ʱ�� */

	/* ��ӡ���ٶ� */
	xprintf("���ݳ���: %d�ֽ�, ����ʱ: %dms, ���ٶ�: %d Bytes/s\r\n", FiscalMemSize, iTime2 - iTime1, (FiscalMemSize * 1000) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	�� �� ��: sfWriteTest
*	����˵��: д����Flash����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfWriteTest(void)
{
	uint16_t i;
	int32_t iTime1, iTime2;
	uint8_t buf[TEST_SIZE];

	/* �����Ի����� */
	for (i = 0; i < TEST_SIZE; i++)
	{
		buf[i] = i;
	}

	/* дEEPROM, ��ʼ��ַ = 0�����ݳ���Ϊ 256 */
	iTime1 = GetSystemTimer();	/* ���¿�ʼʱ�� */
	if (FiscalMem_WriteBuffer(buf, TEST_ADDR, TEST_SIZE) == 0)
	{
		xprintf("д����Flash����\r\n");
		return;
	}
	else
	{
		iTime2 = GetSystemTimer();	/* ���½���ʱ�� */
		xprintf("д����Flash�ɹ���\r\n");
	}


	/* ��ӡ���ٶ� */
	xprintf("���ݳ���: %d�ֽ�, д��ʱ: %dms, д�ٶ�: %dB/s\r\n", TEST_SIZE, iTime2 - iTime1, (TEST_SIZE * 1000) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	�� �� ��: sfWriteAll
*	����˵��: д����EEPROMȫ������
*	��    �Σ�_ch : д�������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfWriteAll(uint8_t _ch)
{
	uint16_t i;
	int32_t iTime1, iTime2;
	uint8_t buf[4 * 1024];

	/* �����Ի����� */
	for (i = 0; i < TEST_SIZE; i++)
	{
		buf[i] = _ch;
	}

	/* дEEPROM, ��ʼ��ַ = 0�����ݳ���Ϊ 256 */
	iTime1 = GetSystemTimer();	/* ���¿�ʼʱ�� */
	for (i = 0; i < FiscalMemSize / FiscalMem_SPI_PAGESIZE; i++)
	{
		if (FiscalMem_WriteBuffer(buf, i * FiscalMem_SPI_PAGESIZE, FiscalMem_SPI_PAGESIZE) == 0)
		{
			xprintf("д����Flash����\r\n");
			return;
		}
		xprintf(".");
		if (((i + 1) % 128) == 0)
		{
			xprintf("\r\n");
		}
	}
	iTime2 = GetSystemTimer();	/* ���½���ʱ�� */

	/* ��ӡ���ٶ� */
	xprintf("���ݳ���: %dK�ֽ�, д��ʱ: %dms, д�ٶ�: %dB/s\r\n", FiscalMemSize / 1024, iTime2 - iTime1, (FiscalMemSize * 1000) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	�� �� ��: sfErase
*	����˵��: ��������Flash
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfErase(void)
{
	int32_t iTime1, iTime2;
	uint8_t buf[FiscalMem_SPI_PAGESIZE];
	uint16_t i,j,stat=0;

	iTime1 = GetSystemTimer();	/* ���¿�ʼʱ�� */
	FiscalMem_EraseBulk();
	iTime2 = GetSystemTimer();	/* ���½���ʱ�� */

	/* ��ӡ���ٶ� */
	xprintf("��������Flash��ɣ�, ��ʱ: %dms\r\n", iTime2 - iTime1);
	
	xprintf("��ȡ��֤�Ƿ�ȫΪ0xFF\r\n");	

	for (i = 0; i < FiscalMemSize / FiscalMem_SPI_PAGESIZE; i++)
	{
		FiscalMem_ReadBuffer(buf, i * FiscalMem_SPI_PAGESIZE, FiscalMem_SPI_PAGESIZE);		/* ������ */
		for(j=0; j<FiscalMem_SPI_PAGESIZE; j++)
		{
			if(buf[j] != 0xFF)
			{
				xprintf("ERROR:PageAddr=%d, InPageAddr=%d, value=0x%x\r\n", i,j,buf[j]);
				stat++;
			}
		}
		xprintf(".");
	}
	if(stat==0)
		xprintf("\r\n��֤��ȷ��ȫΪ0xFF�������ɹ���\r\n");	
	else
		xprintf("\r\n��֤����,����ʧ�ܣ�\r\n");	
	
	return;
}


/*
*********************************************************************************************************
*	�� �� ��: sfViewData
*	����˵��: ������Flash����ʾ��ÿ����ʾ1K������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfViewData(uint32_t _uiAddr)
{
	uint16_t i;
	uint8_t buf[1024];

	FiscalMem_ReadBuffer(buf, _uiAddr,  1024);		/* ������ */
	xprintf("��ַ��0x%08X; ���ݳ��� = 1024\r\n", _uiAddr);

	/* ��ӡ���� */
	for (i = 0; i < 1024; i++)
	{
		xprintf(" %02X", buf[i]);

		if ((i & 31) == 31)
		{
			xprintf("\r\n");	/* ÿ����ʾ16�ֽ����� */
		}
		else if ((i & 31) == 15)
		{
			xprintf(" - ");
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: sfDispMenu
*	����˵��: ��ʾ������ʾ�˵�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfDispMenu(void)
{
	xprintf("\r\n*******************************************\r\n");
	xprintf("��ѡ���������:\r\n");
	xprintf("��1 - ������Flash, ��ַ:0x%X,����:%d�ֽڡ�\r\n", TEST_ADDR, TEST_SIZE);
	xprintf("��2 - д����Flash, ��ַ:0x%X,����:%d�ֽڡ�\r\n", TEST_ADDR, TEST_SIZE);
	xprintf("��3 - ������������Flash��\r\n");
	xprintf("��4 - д��������Flash, ȫ0x55��\r\n");
	xprintf("��5 - д��������Flash, ȫ0xAA��\r\n");
	xprintf("��6 - ����������Flash, ���Զ��ٶȡ�\r\n");
	xprintf("��Z - ��ȡǰ1K����ַ�Զ����١�\r\n");
	xprintf("��X - ��ȡ��1K����ַ�Զ����ӡ�\r\n");
	xprintf("��Q - �˳� ��\r\n");		
	xprintf("��������� - ��ʾ������ʾ\r\n");
	xprintf("\r\n");
}

