
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
	
	/* 扫描测试坏块(耗时很长) */
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

	/* 配置FSMC用于NAND Flash， 复位NAND Flash，重建LUT表 */
	if (FTL_Init() == 0)
	{
		xprintf("NAND Init() Ok\r\n");
	}
	else
	{
		/* 建议在正式的产品中采用人为干预的方式启动低级格式化 */
		/* 自动检测nand flash是否进行了低级格式化，如果没有则执行格式化（2秒） */
		xprintf("NAND Init() Error! \r\n");
		xprintf("Start Format(Low Level) NAND Flash......\r\n");
		//NAND_Format();
		if(FTL_Format())
			xprintf("NAND Flash Format Failed\r\n");
		else
			xprintf("NAND Flash Format Ok\r\n");	
	}

	DispMenu();
	usbd_OpenMassStorage();		/* 初始化USB Device，配置为Mass Storage */

	while(1)
	{
		ptr = ConsoleLine;
		ConsoleLine[0]=0;
		get_line(ptr, sizeof(ConsoleLine));
		switch (*ptr++)
		{
			case '1':	/* 移除U盘, 软件模拟U盘拔除 */
				xprintf("【1 - 移除U盘】\r\n");
				usbd_CloseMassStorage();
				break;

			case '2':	/* 使能U盘，软件模拟U盘插入 */
				xprintf("【2 - 使能U盘】\r\n");
				usbd_OpenMassStorage();
				break;

			case '3':	/* 低级格式化(不会破坏坏块标记) */
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
					xprintf("已经到最前\r\n");
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
					xprintf("已经到最后\r\n");
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
*	函 数 名: DispMenu
*	功能说明: 显示操作提示菜单
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispMenu(void)
{
	xprintf("\r\n------------------------------------------------\r\n");
	xprintf("请选择操作命令:\r\n");
	xprintf("1 - 软件移除U盘\r\n");
	xprintf("2 - 软件使能U盘\r\n");
	xprintf("3 - 低级格式化U盘\r\n");
	xprintf("Z - 显示前一页数据（逻辑页）\r\n");
	xprintf("X - 显示后一页数据（逻辑页）\r\n");
	xprintf("Q - 退出 \r\n");
}





#else	//用于MDK RL USB device
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

