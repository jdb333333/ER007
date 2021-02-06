
#include "spi_w25qxx.h"

#define SF_CS_GPIO		GPIOD
#define SF_CS_PIN			GPIO_Pin_7

/* Ƭѡ�����õ�ѡ��  */
#define SF_CS_LOW()       SF_CS_GPIO->BSRRH = SF_CS_PIN

/* Ƭѡ�����ø߲�ѡ�� */
#define SF_CS_HIGH()      SF_CS_GPIO->BSRRL = SF_CS_PIN

#define CMD_AAI       0xAD  	/* AAI �������ָ��(FOR SST25VF016B) */
#define CMD_DISWR	  	0x04		/* ��ֹд, �˳�AAI״̬ */
#define CMD_EWRSR	  	0x50		/* ����д״̬�Ĵ��������� */
#define CMD_WRSR      0x01  	/* д״̬�Ĵ������� */
#define CMD_WREN      0x06		/* дʹ������ */
#define CMD_READ      0x03  	/* ������������ */
#define CMD_RDSR      0x05		/* ��״̬�Ĵ������� */
#define CMD_RDID      0x9F		/* ������ID���� */
#define CMD_SE        0x20		/* ������������ */
#define CMD_BE        0xD8		//64KB Block Erase
#define CMD_CE        0xC7		/* ������������ */
#define DUMMY_BYTE    0xA5		/* ���������Ϊ����ֵ�����ڶ����� */

#define WIP_FLAG      0x01		/* ״̬�Ĵ����е����ڱ�̱�־��WIP) */

SFLASH_T g_tSF;

void sf_ReadInfo(void);
static uint8_t sf_SendByte(uint8_t _ucValue);
static void sf_WriteEnable(void);
static void sf_WriteStatus(uint8_t _ucValue);
static uint8_t sf_WaitForWriteEnd(void);
static uint8_t sf_NeedErase(uint8_t * _ucpOldBuf, uint8_t *_ucpNewBuf, uint16_t _uiLen);
static uint8_t sf_CmpData(uint32_t _uiSrcAddr, uint8_t *_ucpTar, uint32_t _uiSize);
static uint8_t sf_AutoWritePage(uint8_t *_ucpSrc, uint32_t _uiWrAddr, uint16_t _usWrLen);

static void bsp_CfgSPIForSFlash(void);

static uint8_t s_spiBuf[4*1024];	/* ����д�������ȶ�������page���޸Ļ�������������page��д */

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitSpiFlash
*	����˵��: ��ʼ������FlashӲ���ӿڣ�����STM32��SPIʱ�ӡ�GPIO)
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void sf_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;

	// ʹ��SPI2ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	// ʹ��GPIOBʱ��
  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOF, ENABLE);

	// ����SPI2---AF5, ����SPI2(SCK/MISO/MOSI)���졢����
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);	  // SPI2_SCK
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);	  // SPI2_MISO
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);	  // SPI2_MOSI

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// ����SPI FLASH CS
	GPIO_InitStructure.GPIO_Pin = SF_CS_PIN;			
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(SF_CS_GPIO, &GPIO_InitStructure);

	SF_CS_HIGH();  // �ر�Ƭѡ

	// SPI2 ����:ȫ˫����������8λ֡��SCK�����ز���(SCK����Ϊ1����λΪ1)��CS��������ơ�
	//			 SCK����ٶ�APB1/2=15MHz����λ��ǰ
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;	  //SPI_BaudRatePrescaler_2 ���½ϳ�ʱ���ȶ� SPI1λAPB2�����£��ٶ����ɴ�30MHz  //

	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &SPI_InitStructure);

	// ʹ��SPI2
	SPI_Cmd(SPI2, ENABLE);

#if 1 //�̶�оƬ�ͺ� W25Q16DV
	g_tSF.ChipID = W25Q16DV_ID;
	g_tSF.TotalSize = SFLASH_SIZE;		/* ������ = 2M */
	g_tSF.PageSize = SFLASH_PAGESIZE;	/* ҳ���С = 256 byte */
#else
	sf_ReadInfo();				/* �Զ�ʶ��оƬ�ͺ� */
#endif

	SF_CS_LOW();				/* �����ʽ��ʹ�ܴ���FlashƬѡ */
	sf_SendByte(CMD_DISWR);		/* ���ͽ�ֹд�������,��ʹ�����д���� */
	SF_CS_HIGH();				/* �����ʽ�����ܴ���FlashƬѡ */

	sf_WaitForWriteEnd();		/* �ȴ�����Flash�ڲ�������� */

	sf_WriteStatus(0);			/* �������BLOCK��д���� */
}


/*
*********************************************************************************************************
*	�� �� ��: sf_EraseSector
*	����˵��: ����ָ��������
*	��    ��:  _uiSectorAddr : ������ַ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint8_t sf_EraseSector(uint32_t _uiSectorAddr)
{
	uint8_t reValue;
	
	sf_WriteEnable();								/* ����дʹ������ */

	/* ������������ */
	SF_CS_LOW();									/* ʹ��Ƭѡ */
	sf_SendByte(CMD_SE);								/* ���Ͳ������� */
	sf_SendByte((_uiSectorAddr & 0xFF0000) >> 16);	/* ����������ַ�ĸ�8bit */
	sf_SendByte((_uiSectorAddr & 0xFF00) >> 8);		/* ����������ַ�м�8bit */
	sf_SendByte(_uiSectorAddr & 0xFF);				/* ����������ַ��8bit */
	SF_CS_HIGH();									/* ����Ƭѡ */

	reValue = sf_WaitForWriteEnd();							/* �ȴ�����Flash�ڲ�д������� */
	
	return reValue;
}


/*
*********************************************************************************************************
*	�� �� ��: sf_EraseBlock
*	����˵��: ����ָ����Block
*	��    ��:  _uiBlockAddr : Block��ַ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint8_t sf_EraseBlock(uint32_t _uiBlockAddr)
{
	uint8_t reValue;
	
	sf_WriteEnable();								/* ����дʹ������ */

	/* ��������� */
	SF_CS_LOW();									/* ʹ��Ƭѡ */
	sf_SendByte(CMD_BE);								/* ���Ͳ������� */
	sf_SendByte((_uiBlockAddr & 0xFF0000) >> 16);	/* ���Ϳ��ַ�ĸ�8bit */
	sf_SendByte((_uiBlockAddr & 0xFF00) >> 8);		/* ���Ϳ��ַ�м�8bit */
	sf_SendByte(_uiBlockAddr & 0xFF);				/* ���Ϳ��ַ��8bit */
	SF_CS_HIGH();									/* ����Ƭѡ */

	reValue = sf_WaitForWriteEnd();							/* �ȴ�����Flash�ڲ�д������� */
	
	return reValue;
}


/*
*********************************************************************************************************
*	�� �� ��: sf_EraseChip
*	����˵��: ��������оƬ
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint8_t sf_EraseChip(void)
{
	uint8_t reValue;
	
	sf_WriteEnable();								/* ����дʹ������ */

	/* ������������ */
	SF_CS_LOW();									/* ʹ��Ƭѡ */
	sf_SendByte(CMD_CE);							/* ������Ƭ�������� */
	SF_CS_HIGH();									/* ����Ƭѡ */

	reValue = sf_WaitForWriteEnd();							/* �ȴ�����Flash�ڲ�д������� */
	
	return reValue;	
}

/*
*********************************************************************************************************
*	�� �� ��: sf_PageWrite
*	����˵��: ��һ��page��д�������ֽڡ��ֽڸ������ܳ���ҳ���С��4K)
*	��    ��:  	_pBuf : ����Դ��������
*				_uiWriteAddr ��Ŀ�������׵�ַ
*				_usSize �����ݸ��������ܳ���ҳ���С
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void sf_PageWrite(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize)
{
	uint32_t i, j;

	if (g_tSF.ChipID == SST25VF016B_ID)
	{
		/* AAIָ��Ҫ��������ݸ�����ż�� */
		if ((_usSize < 2) && (_usSize % 2))
		{
			return ;
		}

		sf_WriteEnable();								/* ����дʹ������ */

		SF_CS_LOW();									/* ʹ��Ƭѡ */
		sf_SendByte(CMD_AAI);							/* ����AAI����(��ַ�Զ����ӱ��) */
		sf_SendByte((_uiWriteAddr & 0xFF0000) >> 16);	/* ����������ַ�ĸ�8bit */
		sf_SendByte((_uiWriteAddr & 0xFF00) >> 8);		/* ����������ַ�м�8bit */
		sf_SendByte(_uiWriteAddr & 0xFF);				/* ����������ַ��8bit */
		sf_SendByte(*_pBuf++);							/* ���͵�1������ */
		sf_SendByte(*_pBuf++);							/* ���͵�2������ */
		SF_CS_HIGH();									/* ����Ƭѡ */

		sf_WaitForWriteEnd();							/* �ȴ�����Flash�ڲ�д������� */

		_usSize -= 2;									/* ����ʣ���ֽ��� */

		for (i = 0; i < _usSize / 2; i++)
		{
			SF_CS_LOW();								/* ʹ��Ƭѡ */
			sf_SendByte(CMD_AAI);						/* ����AAI����(��ַ�Զ����ӱ��) */
			sf_SendByte(*_pBuf++);						/* �������� */
			sf_SendByte(*_pBuf++);						/* �������� */
			SF_CS_HIGH();								/* ����Ƭѡ */
			sf_WaitForWriteEnd();						/* �ȴ�����Flash�ڲ�д������� */
		}

		/* ����д����״̬ */
		SF_CS_LOW();
		sf_SendByte(CMD_DISWR);
		SF_CS_HIGH();

		sf_WaitForWriteEnd();							/* �ȴ�����Flash�ڲ�д������� */
	}
	else	/* for MX25L1606E �� W25Q64BV */
	{
		for (j = 0; j < _usSize / 256; j++)
		{
			sf_WriteEnable();								/* ����дʹ������ */

			SF_CS_LOW();									/* ʹ��Ƭѡ */
			sf_SendByte(0x02);								/* ����AAI����(��ַ�Զ����ӱ��) */
			sf_SendByte((_uiWriteAddr & 0xFF0000) >> 16);	/* ����������ַ�ĸ�8bit */
			sf_SendByte((_uiWriteAddr & 0xFF00) >> 8);		/* ����������ַ�м�8bit */
			sf_SendByte(_uiWriteAddr & 0xFF);				/* ����������ַ��8bit */

			for (i = 0; i < 256; i++)
			{
				sf_SendByte(*_pBuf++);					/* �������� */
			}

			SF_CS_HIGH();								/* ��ֹƬѡ */

			sf_WaitForWriteEnd();						/* �ȴ�����Flash�ڲ�д������� */

			_uiWriteAddr += 256;
		}

		/* ����д����״̬ */
		SF_CS_LOW();
		sf_SendByte(CMD_DISWR);
		SF_CS_HIGH();

		sf_WaitForWriteEnd();							/* �ȴ�����Flash�ڲ�д������� */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: sf_ReadBuffer
*	����˵��: ������ȡ�����ֽڡ��ֽڸ������ܳ���оƬ������
*	��    ��:  	_pBuf : ����Դ��������
*				_uiReadAddr ���׵�ַ
*				_usSize �����ݸ���, ���Դ���PAGE_SIZE,���ǲ��ܳ���оƬ������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void sf_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
	/* �����ȡ�����ݳ���Ϊ0���߳�������Flash��ַ�ռ䣬��ֱ�ӷ��� */
	if ((_uiSize == 0) ||(_uiReadAddr + _uiSize) > g_tSF.TotalSize)
	{
		return;
	}

	/* ������������ */
	SF_CS_LOW();									/* ʹ��Ƭѡ */
	sf_SendByte(CMD_READ);							/* ���Ͷ����� */
	sf_SendByte((_uiReadAddr & 0xFF0000) >> 16);	/* ����������ַ�ĸ�8bit */
	sf_SendByte((_uiReadAddr & 0xFF00) >> 8);		/* ����������ַ�м�8bit */
	sf_SendByte(_uiReadAddr & 0xFF);				/* ����������ַ��8bit */
	while (_uiSize--)
	{
		*_pBuf++ = sf_SendByte(DUMMY_BYTE);			/* ��һ���ֽڲ��洢��pBuf�������ָ���Լ�1 */
	}
	SF_CS_HIGH();									/* ����Ƭѡ */
}

/*
*********************************************************************************************************
*	�� �� ��: sf_CmpData
*	����˵��: �Ƚ�Flash������.
*	��    ��:  	_ucpTar : ���ݻ�����
*				_uiSrcAddr ��Flash��ַ
*				_uiSize �����ݸ���, ���Դ���PAGE_SIZE,���ǲ��ܳ���оƬ������
*	�� �� ֵ: 0 = ���, 1 = ����
*********************************************************************************************************
*/
static uint8_t sf_CmpData(uint32_t _uiSrcAddr, uint8_t *_ucpTar, uint32_t _uiSize)
{
	uint8_t ucValue;

	/* �����ȡ�����ݳ���Ϊ0���߳�������Flash��ַ�ռ䣬��ֱ�ӷ��� */
	if ((_uiSrcAddr + _uiSize) > g_tSF.TotalSize)
	{
		return 1;
	}

	if (_uiSize == 0)
	{
		return 0;
	}

	SF_CS_LOW();									/* ʹ��Ƭѡ */
	sf_SendByte(CMD_READ);							/* ���Ͷ����� */
	sf_SendByte((_uiSrcAddr & 0xFF0000) >> 16);		/* ����������ַ�ĸ�8bit */
	sf_SendByte((_uiSrcAddr & 0xFF00) >> 8);		/* ����������ַ�м�8bit */
	sf_SendByte(_uiSrcAddr & 0xFF);					/* ����������ַ��8bit */
	while (_uiSize--)
	{
		/* ��һ���ֽ� */
		ucValue = sf_SendByte(DUMMY_BYTE);
		if (*_ucpTar++ != ucValue)
		{
			SF_CS_HIGH();
			return 1;
		}
	}
	SF_CS_HIGH();
	return 0;
}

/*
*********************************************************************************************************
*	�� �� ��: sf_NeedErase
*	����˵��: �ж�дPAGEǰ�Ƿ���Ҫ�Ȳ�����
*	��    ��:   _ucpOldBuf �� ������
*			   _ucpNewBuf �� ������
*			   _uiLen �����ݸ��������ܳ���ҳ���С
*	�� �� ֵ: 0 : ����Ҫ������ 1 ����Ҫ����
*********************************************************************************************************
*/
static uint8_t sf_NeedErase(uint8_t * _ucpOldBuf, uint8_t *_ucpNewBuf, uint16_t _usLen)
{
	uint16_t i;
	uint8_t ucOld;

	/*
	�㷨��1����old ��, new ����
	      old    new
		  1101   0101
	~     1111
		= 0010   0101

	�㷨��2��: old �󷴵Ľ���� new λ��
		  0010   old
	&	  0101   new
		 =0000

	�㷨��3��: ���Ϊ0,���ʾ�������. �����ʾ��Ҫ����
	*/

	for (i = 0; i < _usLen; i++)
	{
		ucOld = *_ucpOldBuf++;
		ucOld = ~ucOld;

		/* ע������д��: if (ucOld & (*_ucpNewBuf++) != 0) */
		if ((ucOld & (*_ucpNewBuf++)) != 0)
		{
			return 1;
		}
	}
	return 0;
}

/*
*********************************************************************************************************
*	�� �� ��: sf_AutoWritePage
*	����˵��: д1��PAGE��У��,�������ȷ������д���Ρ��������Զ���ɲ���������
*	��    ��:  	_pBuf : ����Դ��������
*				_uiWriteAddr ��Ŀ�������׵�ַ
*				_usSize �����ݸ��������ܳ���ҳ���С
*	�� �� ֵ: 0 : ���� 1 �� �ɹ�
*********************************************************************************************************
*/
static uint8_t sf_AutoWritePage(uint8_t *_ucpSrc, uint32_t _uiWrAddr, uint16_t _usWrLen)
{
	uint16_t i;
	uint16_t j;					/* ������ʱ */
	uint32_t uiFirstAddr;		/* ������ַ */
	uint8_t ucNeedErase;		/* 1��ʾ��Ҫ���� */
	uint8_t cRet;
	//uint8_t tmpbuf[1024] = {0};//jdb2019-03-11������

	/* ����Ϊ0ʱ����������,ֱ����Ϊ�ɹ� */
	if (_usWrLen == 0)
	{
		return 1;
	}

	/* ���ƫ�Ƶ�ַ����оƬ�������˳� */
	if (_uiWrAddr >= g_tSF.TotalSize)
	{
		return 0;
	}

	/* ������ݳ��ȴ����������������˳� */
	if (_usWrLen > g_tSF.PageSize)
	{
		return 0;
	}

	/* ���FLASH�е�����û�б仯,��дFLASH */
	sf_ReadBuffer(s_spiBuf, _uiWrAddr, _usWrLen);
	if (memcmp(s_spiBuf, _ucpSrc, _usWrLen) == 0)
	{
		return 1;
	}

	//sf_ReadBuffer(tmpbuf, 0x10000, 1024);//jdb2019-03-11������

	/* �ж��Ƿ���Ҫ�Ȳ������� */
	/* ����������޸�Ϊ�����ݣ�����λ���� 1->0 ���� 0->0, ���������,���Flash���� */
	ucNeedErase = 0;
	if (sf_NeedErase(s_spiBuf, _ucpSrc, _usWrLen))
	{
		ucNeedErase = 1;
	}

	uiFirstAddr = _uiWrAddr & (~(g_tSF.PageSize - 1));

	if (_usWrLen == g_tSF.PageSize)		/* ������������д */
	{
		for	(i = 0; i < g_tSF.PageSize; i++)
		{
			s_spiBuf[i] = _ucpSrc[i];
		}
	}
	else						/* ��д�������� */
	{
		/* �Ƚ��������������ݶ��� */
		sf_ReadBuffer(s_spiBuf, uiFirstAddr, g_tSF.PageSize);

		/* ���������ݸ��� */
		i = _uiWrAddr & (g_tSF.PageSize - 1);
		memcpy(&s_spiBuf[i], _ucpSrc, _usWrLen);
	}

	/* д��֮�����У�飬�������ȷ����д�����3�� */
	cRet = 0;
	for (i = 0; i < 3; i++)
	{

		/* ����������޸�Ϊ�����ݣ�����λ���� 1->0 ���� 0->0, ���������,���Flash���� */
		if (ucNeedErase == 1)
		{
			sf_EraseSector(uiFirstAddr);		/* ����1������ */
		}

		/* ���һ��PAGE */
		sf_PageWrite(s_spiBuf, uiFirstAddr, g_tSF.PageSize);

		if (sf_CmpData(_uiWrAddr, _ucpSrc, _usWrLen) == 0)
		{
			cRet = 1;
			break;
		}
		else
		{
			if (sf_CmpData(_uiWrAddr, _ucpSrc, _usWrLen) == 0)
			{
				cRet = 1;
				break;
			}

			/* ʧ�ܺ��ӳ�һ��ʱ�������� */
			for (j = 0; j < 10000; j++);
		}
	}

	return cRet;
}

/*
*********************************************************************************************************
*	�� �� ��: sf_WriteBuffer
*	����˵��: д1��������У��,�������ȷ������д���Ρ��������Զ���ɲ���������
*	��    ��:  	_pBuf : ����Դ��������
*				_uiWrAddr ��Ŀ�������׵�ַ
*				_usSize �����ݸ��������ܳ���ҳ���С
*	�� �� ֵ: 1 : �ɹ��� 0 �� ʧ��
*********************************************************************************************************
*/
uint8_t sf_WriteBuffer(uint8_t* _pBuf, uint32_t _uiWriteAddr, uint16_t _usWriteSize)
{
	uint16_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

	Addr = _uiWriteAddr % g_tSF.PageSize;
	count = g_tSF.PageSize - Addr;
	NumOfPage =  _usWriteSize / g_tSF.PageSize;
	NumOfSingle = _usWriteSize % g_tSF.PageSize;

	if (Addr == 0) /* ��ʼ��ַ��ҳ���׵�ַ  */
	{
		if (NumOfPage == 0) /* ���ݳ���С��ҳ���С */
		{
			if (sf_AutoWritePage(_pBuf, _uiWriteAddr, _usWriteSize) == 0)
			{
				return 0;
			}
		}
		else 	/* ���ݳ��ȴ��ڵ���ҳ���С */
		{
			while (NumOfPage--)
			{
				if (sf_AutoWritePage(_pBuf, _uiWriteAddr, g_tSF.PageSize) == 0)
				{
					return 0;
				}
				_uiWriteAddr +=  g_tSF.PageSize;
				_pBuf += g_tSF.PageSize;
			}
			if (sf_AutoWritePage(_pBuf, _uiWriteAddr, NumOfSingle) == 0)
			{
				return 0;
			}
		}
	}
	else  /* ��ʼ��ַ����ҳ���׵�ַ  */
	{
		if (NumOfPage == 0) /* ���ݳ���С��ҳ���С */
		{
			if (NumOfSingle > count) /* (_usWriteSize + _uiWriteAddr) > SPI_FLASH_PAGESIZE */
			{
				temp = NumOfSingle - count;

				if (sf_AutoWritePage(_pBuf, _uiWriteAddr, count) == 0)
				{
					return 0;
				}

				_uiWriteAddr +=  count;
				_pBuf += count;

				if (sf_AutoWritePage(_pBuf, _uiWriteAddr, temp) == 0)
				{
					return 0;
				}
			}
			else
			{
				if (sf_AutoWritePage(_pBuf, _uiWriteAddr, _usWriteSize) == 0)
				{
					return 0;
				}
			}
		}
		else	/* ���ݳ��ȴ��ڵ���ҳ���С */
		{
			_usWriteSize -= count;
			NumOfPage =  _usWriteSize / g_tSF.PageSize;
			NumOfSingle = _usWriteSize % g_tSF.PageSize;

			if (sf_AutoWritePage(_pBuf, _uiWriteAddr, count) == 0)
			{
				return 0;
			}

			_uiWriteAddr +=  count;
			_pBuf += count;

			while (NumOfPage--)
			{
				if (sf_AutoWritePage(_pBuf, _uiWriteAddr, g_tSF.PageSize) == 0)
				{
					return 0;
				}
				_uiWriteAddr +=  g_tSF.PageSize;
				_pBuf += g_tSF.PageSize;
			}

			if (NumOfSingle != 0)
			{
				if (sf_AutoWritePage(_pBuf, _uiWriteAddr, NumOfSingle) == 0)
				{
					return 0;
				}
			}
		}
	}
	return 1;	/* �ɹ� */
}

/*
*********************************************************************************************************
*	�� �� ��: sf_ReadID
*	����˵��: ��ȡ����ID
*	��    ��:  ��
*	�� �� ֵ: 32bit������ID (���8bit��0����ЧIDλ��Ϊ24bit��
*********************************************************************************************************
*/
uint32_t sf_ReadID(void)
{
	uint32_t uiID;
	uint8_t id1, id2, id3;

	SF_CS_LOW();									/* ʹ��Ƭѡ */
	sf_SendByte(CMD_RDID);								/* ���Ͷ�ID���� */
	id1 = sf_SendByte(DUMMY_BYTE);					/* ��ID�ĵ�1���ֽ� */
	id2 = sf_SendByte(DUMMY_BYTE);					/* ��ID�ĵ�2���ֽ� */
	id3 = sf_SendByte(DUMMY_BYTE);					/* ��ID�ĵ�3���ֽ� */
	SF_CS_HIGH();									/* ����Ƭѡ */

	uiID = ((uint32_t)id1 << 16) | ((uint32_t)id2 << 8) | id3;

	return uiID;
}

/*
*********************************************************************************************************
*	�� �� ��: sf_ReadInfo
*	����˵��: ��ȡ����ID,�������������
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void sf_ReadInfo(void)
{
	/* �Զ�ʶ����Flash�ͺ� */
	{
		g_tSF.ChipID = sf_ReadID();	/* оƬID */

		switch (g_tSF.ChipID)
		{
			case SST25VF016B_ID:
				strcpy(g_tSF.ChipName, "SST25VF016B");
				g_tSF.TotalSize = 2 * 1024 * 1024;	/* ������ = 2M */
				g_tSF.PageSize = 4 * 1024;			/* ҳ���С = 4K */
				break;

			case MX25L1606E_ID:
				strcpy(g_tSF.ChipName, "MX25L1606E");
				g_tSF.TotalSize = 2 * 1024 * 1024;	/* ������ = 2M */
				g_tSF.PageSize = 4 * 1024;			/* ҳ���С = 4K */
				break;

			case W25Q16DV_ID:
				strcpy(g_tSF.ChipName, "W25Q16DV");
				g_tSF.TotalSize = 2 * 1024 * 1024;	/* ������ = 2M */
				g_tSF.PageSize = 256;//4 * 1024;			/* ҳ���С = 256 byte */
				break;			
			
			case W25Q64BV_ID:
				strcpy(g_tSF.ChipName, "W25Q64BV");
				g_tSF.TotalSize = 8 * 1024 * 1024;	/* ������ = 8M */
				g_tSF.PageSize = 256;//4 * 1024;			/* ҳ���С = 4K */
				break;

			case W25Q128FV_ID:
				strcpy(g_tSF.ChipName, "W25Q128FV");
				g_tSF.TotalSize = 16 * 1024 * 1024;	/* ������ = 16M */
				g_tSF.PageSize = 256;//4 * 1024;			/* ҳ���С = 4K */
				break;
			
			default:
				strcpy(g_tSF.ChipName, "Unknow Flash");
				g_tSF.TotalSize = 2 * 1024 * 1024;
				g_tSF.PageSize = 4 * 1024;
				break;
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: sf_SendByte
*	����˵��: ����������һ���ֽڣ�ͬʱ��MISO���߲����������ص�����
*	��    ��:  _ucByte : ���͵��ֽ�ֵ
*	�� �� ֵ: ��MISO���߲����������ص�����
*********************************************************************************************************
*/
static uint8_t sf_SendByte(uint8_t _ucValue)
{
#if 0	
	/* �ȴ��ϸ�����δ������� */
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* ͨ��SPIӲ������1���ֽ� */
	SPI_I2S_SendData(SPI_FLASH, _ucValue);

	/* �ȴ�����һ���ֽ�������� */
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);

	/* ���ش�SPI���߶��������� */
	return SPI_I2S_ReceiveData(SPI2);
#else
	while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET);
	SPI2->DR = _ucValue;
	while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET);
	return SPI2->DR;	
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: sf_WriteEnable
*	����˵��: ����������дʹ������
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sf_WriteEnable(void)
{
	SF_CS_LOW();									/* ʹ��Ƭѡ */
	sf_SendByte(CMD_WREN);								/* �������� */
	SF_CS_HIGH();									/* ����Ƭѡ */
}

/*
*********************************************************************************************************
*	�� �� ��: sf_WriteStatus
*	����˵��: д״̬�Ĵ���
*	��    ��:  _ucValue : ״̬�Ĵ�����ֵ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sf_WriteStatus(uint8_t _ucValue)
{

	if (g_tSF.ChipID == SST25VF016B_ID)
	{
		/* ��1������ʹ��д״̬�Ĵ��� */
		SF_CS_LOW();									/* ʹ��Ƭѡ */
		sf_SendByte(CMD_EWRSR);							/* ������� ����д״̬�Ĵ��� */
		SF_CS_HIGH();									/* ����Ƭѡ */

		/* ��2������д״̬�Ĵ��� */
		SF_CS_LOW();									/* ʹ��Ƭѡ */
		sf_SendByte(CMD_WRSR);							/* ������� д״̬�Ĵ��� */
		sf_SendByte(_ucValue);							/* �������ݣ�״̬�Ĵ�����ֵ */
		SF_CS_HIGH();									/* ����Ƭѡ */
	}
	else
	{
		SF_CS_LOW();									/* ʹ��Ƭѡ */
		sf_SendByte(CMD_WRSR);							/* ������� д״̬�Ĵ��� */
		sf_SendByte(_ucValue);							/* �������ݣ�״̬�Ĵ�����ֵ */
		SF_CS_HIGH();									/* ����Ƭѡ */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: sf_WaitForWriteEnd
*	����˵��: ����ѭ����ѯ�ķ�ʽ�ȴ������ڲ�д�������
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static uint8_t sf_WaitForWriteEnd(void)
{
	uint8_t reValue;
	uint32_t DelayFor = 0;			// 	��ʱ����
	uint8_t flashstatus = 0;
	
	SF_CS_LOW();									/* ʹ��Ƭѡ */
	sf_SendByte(CMD_RDSR);							/* ������� ��״̬�Ĵ��� */
	do
	{
		flashstatus = sf_SendByte(DUMMY_BYTE);
	}while(((flashstatus & WIP_FLAG) == SET)&& (++DelayFor<SFLASH_TIMEOUT));	/* �ж�״̬�Ĵ�����æ��־λ */
	SF_CS_HIGH();									/* ����Ƭѡ */
	
  if ((flashstatus & WIP_FLAG) == SET)
  	return(ERROR);
  return (SUCCESS);	
}




//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>ouhs 20190304//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//��ȡһ���ֽ�
uint8_t FlashReadByte(uint32_t Addr)
{
	uint8_t c;
	
	sf_ReadBuffer(&c, Addr, 1);
	return c;
}

//���ֽڶ�ȡbuff
uint8_t spi_bFlashReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumBytesToRead)
{
	sf_ReadBuffer(pBuffer, ReadAddr, NumBytesToRead);
	
	return SUCCESS;
}

//д��һ���ֽ�
uint8_t spi_bFlashWrite( uint32_t ulOff, uint8_t ucVal )
{
	uint8_t reValue;

	#if defined(CASE_FORHANZI)
	while( TestPrintGoingOn() )	{};//��ӡʱ,��ֹ����Flash
#endif
	
	reValue = sf_WriteBuffer(&ucVal, ulOff, 1);
	
	return reValue;	
}

//���ֽ�д��buff
uint8_t spi_bFlashProgram( uint32_t ulOff, uint32_t NumBytes, uint8_t *Array )
{
	uint8_t reValue;

#if defined(CASE_FORHANZI)
	while( TestPrintGoingOn() )	{};//��ӡʱ,��ֹ����Flash
#endif
		
	reValue = sf_WriteBuffer(Array, ulOff, NumBytes);
	
	return reValue;
}


//����N��SECTOR
//ucSector:SECTOR��ʼ��ַ
//ucNumSectors:����SECTOR����
uint8_t bFlashMSectorErase(uint32_t ucSector, uint32_t ucNumSectors)
{
	uint8_t reValue;
	uint32_t CurrentSec;

#if defined(CASE_FORHANZI)
	while( TestPrintGoingOn() )	{};//��ӡʱ,��ֹ����Flash
#endif	
	
	// �����ڲ���
	if ((ucSector > SFLASH_SECTOR_MAXNUM)||((ucSector+ucNumSectors) > SFLASH_SECTOR_MAXNUM) || (ucNumSectors==0))
  {
		return (ERROR);
  }
	
  // ����0 sector��������ΪSFLASH_SECTOR_MAXNUM,��ִ��оƬ����
  if ((ucSector == 0) && (ucNumSectors == SFLASH_SECTOR_MAXNUM))
  {
  	reValue = sf_EraseChip();
  	return (reValue);
  }

  // ��ʼ������������
	CurrentSec = ucSector;
	while(ucNumSectors--);
	{
		reValue = sf_EraseSector(CurrentSec++);
		if (reValue == ERROR)
			return (ERROR);
	}

  return (SUCCESS);
}


//����N��BLOCK
//ucBlock:BLOCK��ʼ��ַ
//ucNumBlocks:����BLOCK����
uint8_t spi_bFlashMBlockErase(uint32_t ucBlock, uint32_t ucNumBlocks)
{
	uint8_t reValue;
	uint32_t CurrentSec;
	uint32_t ucBlockAddr;//jdb2019-03-11���ַ

#if defined(CASE_FORHANZI)
	while( TestPrintGoingOn() )	{};//��ӡʱ,��ֹ����Flash
#endif	
	
	// �����ڲ���
	if ((ucBlock > SFLASH_BLOCK_MAXNUM)||((ucBlock+ucNumBlocks) > SFLASH_BLOCK_MAXNUM) || (ucNumBlocks==0))
  {
		return (ERROR);
  }
	
  // ����0 sector��������ΪSFLASH_SECTOR_MAXNUM,��ִ��оƬ����
  if ((ucBlock == 0) && (ucNumBlocks == SFLASH_BLOCK_MAXNUM))
  {
  	reValue = sf_EraseChip();
  	return (reValue);
  }

  // ��ʼ������������
	CurrentSec = ucBlock;
	while(ucNumBlocks--)
	{
		ucBlockAddr = 0x10000 * CurrentSec++;//jdb2019-03-11������ַ
		reValue = sf_EraseBlock(ucBlockAddr);
		if (reValue == ERROR)
			return (ERROR);
	}

  return (SUCCESS);	
}
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

