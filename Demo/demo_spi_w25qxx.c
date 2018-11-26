#if !defined(NAND_DISK)

#include "stm32f2xx.h"
#include "typedef.h"
#include "ff.h"		
#include "monitor.h"
#include "SysTick.h"
#include "demo_spi_w25qxx.h"
#include "spi_w25qxx.h"

#define TEST_ADDR		0			/* 读写测试地址 */
#define TEST_SIZE		4096		/* 读写测试数据大小 */


/* 仅允许本文件内调用的函数声明 */
static void sfDispMenu(void);
static void sfReadTest(void);
static void sfWriteTest(void);
static void sfErase(void);
static void sfViewData(uint32_t _uiAddr);
static void sfWriteAll(uint8_t _ch);
static void sfTestReadSpeed(void);


/*
*********************************************************************************************************
*	函 数 名: DemoSpiFlash
*	功能说明: 串行EEPROM读写例程
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void DemoSpiFlash(void)
{
	char *ptr;
	uint32_t ChipID,ChipRES;
	uint32_t uiReadPageNo = 0;

	ChipID = sf_ReadID();
	xprintf("检测到串行Flash, ID = %08x",ChipID);
	xprintf("    容量 : %dM字节, 扇区大小 : %d字节\r\n", g_tSF.TotalSize/(1024*1024), g_tSF.PageSize);

		
	sfDispMenu();		/* 打印命令提示 */
	
	while(1)
	{
		ptr = ConsoleLine;
		ConsoleLine[0]=0;
		get_line(ptr, sizeof(ConsoleLine));

		switch (*ptr++)
		{
				case '1':
					xprintf("\r\n【1 - 读串行Flash, 地址:0x%X,长度:%d字节】\r\n", TEST_ADDR, TEST_SIZE);
					sfReadTest();		/* 读串行Flash数据，并打印出来数据内容 */
					break;

				case '2':
					xprintf("\r\n【2 - 写串行Flash, 地址:0x%X,长度:%d字节】\r\n", TEST_ADDR, TEST_SIZE);
					sfWriteTest();		/* 写串行Flash数据，并打印写入速度 */
					break;

				case '3':
					xprintf("\r\n【3 - 擦除整个串行Flash】\r\n");
					sfErase();			/* 擦除串行Flash数据，实际上就是写入全0xFF */
					break;

				case '4':
					xprintf("\r\n【4 - 写整个串行Flash, 全0x55】\r\n");
					sfWriteAll(0x55);			/* 擦除串行Flash数据，实际上就是写入全0xFF */
					break;

				case '5':
					xprintf("\r\n【5 - 写整个串行Flash, 全0xAA】\r\n");
					sfWriteAll(0xAA);			/* 擦除串行Flash数据，实际上就是写入全0xFF */
					break;

				case '6':
					xprintf("\r\n【6 - 读整个串行Flash, %dM字节】\r\n", g_tSF.TotalSize/(1024*1024));
					sfTestReadSpeed();		/* 读整个串行Flash数据，测试速度 */
					break;

				case 'z':
				case 'Z': /* 读取前1K */
					if (uiReadPageNo > 0)
					{
						uiReadPageNo--;
					}
					else
					{
						xprintf("已经是最前\r\n");
					}
					sfViewData(uiReadPageNo * 1024);
					break;

				case 'x':
				case 'X': /* 读取后1K */
					if (uiReadPageNo < g_tSF.TotalSize / 1024 - 1)
					{
						uiReadPageNo++;
					}
					else
					{
						xprintf("已经是最后\r\n");
					}
					sfViewData(uiReadPageNo * 1024);
					break;
			case 'Q':
         return;
			
				default:
					sfDispMenu();	/* 无效命令，重新打印命令提示 */
					break;
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: sfReadTest
*	功能说明: 读串行Flash测试
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void sfReadTest(void)
{
	uint16_t i;
	int32_t iTime1, iTime2;
	uint8_t buf[TEST_SIZE];

	/* 起始地址 = 0， 数据长度为 256 */
	iTime1 = GetSystemTimer();	/* 记下开始时间 */
	sf_ReadBuffer(buf, TEST_ADDR, TEST_SIZE);
	iTime2 = GetSystemTimer();	/* 记下结束时间 */
	xprintf("读串行Flash成功，数据如下：\r\n");

	/* 打印数据 */
	for (i = 0; i < TEST_SIZE; i++)
	{
		xprintf(" %02X", buf[i]);

		if ((i & 31) == 31)
		{
			xprintf("\r\n");	/* 每行显示16字节数据 */
		}
		else if ((i & 31) == 15)
		{
			xprintf(" - ");
		}
	}

	/* 打印读速度 */
	xprintf("数据长度: %d字节, 读耗时: %dms, 读速度: %d Bytes/s\r\n", TEST_SIZE, iTime2 - iTime1, (TEST_SIZE * 1000) / (iTime2 - iTime1));
}


/*
*********************************************************************************************************
*	函 数 名: sfTestReadSpeed
*	功能说明: 测试串行Flash读速度。读取整个串行Flash的数据，最后打印结果
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void sfTestReadSpeed(void)
{
	uint16_t i;
	int32_t iTime1, iTime2;
	uint8_t buf[TEST_SIZE];
	uint32_t uiAddr;

	/* 起始地址 = 0， 数据长度为 256 */
	iTime1 = GetSystemTimer();	/* 记下开始时间 */
	uiAddr = 0;
	for (i = 0; i < g_tSF.TotalSize / TEST_SIZE; i++, uiAddr += TEST_SIZE)
	{
		sf_ReadBuffer(buf, uiAddr, TEST_SIZE);
	}
	iTime2 = GetSystemTimer();	/* 记下结束时间 */

	/* 打印读速度 */
	xprintf("数据长度: %d字节, 读耗时: %dms, 读速度: %d Bytes/s\r\n", g_tSF.TotalSize, iTime2 - iTime1, (g_tSF.TotalSize * 1000) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	函 数 名: sfWriteTest
*	功能说明: 写串行Flash测试
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void sfWriteTest(void)
{
	uint16_t i;
	int32_t iTime1, iTime2;
	uint8_t buf[TEST_SIZE];

	/* 填充测试缓冲区 */
	for (i = 0; i < TEST_SIZE; i++)
	{
		buf[i] = i;
	}

	/* 写EEPROM, 起始地址 = 0，数据长度为 256 */
	iTime1 = GetSystemTimer();	/* 记下开始时间 */
	if (sf_WriteBuffer(buf, TEST_ADDR, TEST_SIZE) == 0)
	{
		xprintf("写串行Flash出错！\r\n");
		return;
	}
	else
	{
		iTime2 = GetSystemTimer();	/* 记下结束时间 */
		xprintf("写串行Flash成功！\r\n");
	}


	/* 打印读速度 */
	xprintf("数据长度: %d字节, 写耗时: %dms, 写速度: %dB/s\r\n", TEST_SIZE, iTime2 - iTime1, (TEST_SIZE * 1000) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	函 数 名: sfWriteAll
*	功能说明: 写串行EEPROM全部数据
*	形    参：_ch : 写入的数据
*	返 回 值: 无
*********************************************************************************************************
*/
static void sfWriteAll(uint8_t _ch)
{
	uint16_t i;
	int32_t iTime1, iTime2;
	uint8_t buf[4 * 1024];

	/* 填充测试缓冲区 */
	for (i = 0; i < TEST_SIZE; i++)
	{
		buf[i] = _ch;
	}

	/* 写EEPROM, 起始地址 = 0，数据长度为 256 */
	iTime1 = GetSystemTimer();	/* 记下开始时间 */
	for (i = 0; i < g_tSF.TotalSize / g_tSF.PageSize; i++)
	{
		if (sf_WriteBuffer(buf, i * g_tSF.PageSize, g_tSF.PageSize) == 0)
		{
			xprintf("写串行Flash出错！\r\n");
			return;
		}
		xprintf(".");
		if (((i + 1) % 128) == 0)
		{
			xprintf("\r\n");
		}
	}
	iTime2 = GetSystemTimer();	/* 记下结束时间 */

	/* 打印读速度 */
	xprintf("数据长度: %dK字节, 写耗时: %dms, 写速度: %dB/s\r\n", g_tSF.TotalSize / 1024, iTime2 - iTime1, (g_tSF.TotalSize * 1000) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	函 数 名: sfErase
*	功能说明: 擦除串行Flash
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void sfErase(void)
{
	int32_t iTime1, iTime2;
	uint8_t buf[256];//buf[g_tSF.PageSize];
	uint16_t i,j,stat=0;

	iTime1 = GetSystemTimer();	/* 记下开始时间 */
	sf_EraseChip();
	iTime2 = GetSystemTimer();	/* 记下结束时间 */

	/* 打印读速度 */
	xprintf("擦除串行Flash完成！, 耗时: %dms\r\n", iTime2 - iTime1);
	
	xprintf("读取验证是否全为0xFF\r\n");	

	for (i = 0; i < g_tSF.TotalSize / g_tSF.PageSize; i++)
	{
		sf_ReadBuffer(buf, i * g_tSF.PageSize, g_tSF.PageSize);		/* 读数据 */
		for(j=0; j<g_tSF.PageSize; j++)
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
		xprintf("\r\n验证正确，全为0xFF，擦除成功！\r\n");	
	else
		xprintf("\r\n验证出错,擦除失败！\r\n");	
	
	return;
}


/*
*********************************************************************************************************
*	函 数 名: sfViewData
*	功能说明: 读串行Flash并显示，每次显示1K的内容
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void sfViewData(uint32_t _uiAddr)
{
	uint16_t i;
	uint8_t buf[1024];

	sf_ReadBuffer(buf, _uiAddr,  1024);		/* 读数据 */
	xprintf("地址：0x%08X; 数据长度 = 1024\r\n", _uiAddr);

	/* 打印数据 */
	for (i = 0; i < 1024; i++)
	{
		xprintf(" %02X", buf[i]);

		if ((i & 31) == 31)
		{
			xprintf("\r\n");	/* 每行显示16字节数据 */
		}
		else if ((i & 31) == 15)
		{
			xprintf(" - ");
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: sfDispMenu
*	功能说明: 显示操作提示菜单
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void sfDispMenu(void)
{
	xprintf("\r\n*******************************************\r\n");
	xprintf("请选择操作命令:\r\n");
	xprintf("【1 - 读串行Flash, 地址:0x%X,长度:%d字节】\r\n", TEST_ADDR, TEST_SIZE);
	xprintf("【2 - 写串行Flash, 地址:0x%X,长度:%d字节】\r\n", TEST_ADDR, TEST_SIZE);
	xprintf("【3 - 擦除整个串行Flash】\r\n");
	xprintf("【4 - 写整个串行Flash, 全0x55】\r\n");
	xprintf("【5 - 写整个串行Flash, 全0xAA】\r\n");
	xprintf("【6 - 读整个串行Flash, 测试读速度】\r\n");
	xprintf("【Z - 读取前1K，地址自动减少】\r\n");
	xprintf("【X - 读取后1K，地址自动增加】\r\n");
	xprintf("【Q - 退出 】\r\n");		
	xprintf("其他任意键 - 显示命令提示\r\n");
	xprintf("\r\n");
}

#endif

