/*
*********************************************************************************************************
*
*	模块名称 : FlashFS文件系统演示模块。
*	文件名称 : demo_nandflash_flashfs.c
*	版    本 : V1.0
*	说    明 : 该例程移植FlashFS文件系统.
*   注意事项 : 1. FlashFS不支持操作中文文件名的文件，仅支持在使用短文件名库FSN_CM3.lib列出8.3格式中文名。
*                 但是使用fopen函数操作中文文件名的文件时会出错。
*              2. FlashFS只有MDK的库，没有IAR和GCC。
*
*	修改记录 :
*		版本号    日期         作者            说明
*       V1.0    2015-09-19   Eric2013    1. RL-FlashFS短文件名版本V4.74
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include  <stdarg.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <math.h>

#include "stm32f2xx.h"
#include "monitor.h"
#include "SysTick.h"
#include "demo_nand_flashfs.h"

#include <RTL.h>
#include <File_Config.h>



/* 用于测试读写速度 */
#define TEST_FILE_LEN			(2*1024*1024)	/* 用于测试的文件长度 */
#define BUF_SIZE				(4*1024)		/* 每次读写SD卡的最大数据长度 */
uint8_t g_TestBuf[BUF_SIZE];

/* 仅允许本文件内调用的函数声明 */
static void ViewNandCapacity(void);
static void ViewRootDir(void);
static void CreateNewFile(void);
static void ReadFileData(void);
static void SeekFileData(void);
static void DeleteDirFile(void);
static void WriteFileTest(void);
static void WriteCSVFile(void);
static void ReadCSVFileData(void);
static void DispMenu(void);
static void DotFormat(uint64_t _ullVal, char *_sp);
static void CreateNewDirFileTest(void);

/* FlashFs API的返回值 */
static const char * ReVal_Table[]= 
{
	"0：成功",				                        
	"1：IO错误，I/O驱动初始化失败，或者没有存储设备，或者设备初始化失败",
	"2：卷错误，挂载失败，对于FAT文件系统意味着无效的MBR，启动记录或者非FAT格式",
	"3：FAT日志初始化失败，FAT初始化成功了，但是日志初始化失败",
};

/*
*********************************************************************************************************
*	函 数 名: DemoFlashFs
*	功能说明: FlashFs文件系统演示主程序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DemoFlashFs(void)
{
	char *ptr;

	/* 打印命令列表，用户可以通过串口操作指令 */
	DispMenu();
	
	while(1)
	{
		ptr = ConsoleLine;
		ConsoleLine[0]=0;
		get_line(ptr, sizeof(ConsoleLine));
		switch (*ptr++)
		{
			case '0':
				xprintf("【0 - ViewNandCapacity】\r\n");
				ViewNandCapacity();		/* 低级格式化，显示NAND Flash容量和剩余容量 */
				break;

			case '1':
				xprintf("【1 - ViewRootDir】\r\n");
				ViewRootDir();		     /* 显示NAND目录下的文件名 */
				break;
			
			case '2':
				xprintf("【2 - CreateNewFile】\r\n");
				CreateNewFile();	    /* 创建三个text文本并使用不同函数写入内容 */
				break;
			
			case '3':
				xprintf("【3 - ReadFileData】\r\n");
				ReadFileData();	        /* 使用三个不同函数读取文本  */
				break;

			case '4':
				xprintf("【4 - SeekFileData】\r\n");
				SeekFileData();	        /* 创建一个text文本，在指定位置对其进行读写操作  */
				break;
			
			case '5':
				xprintf("【5 - DeleteDirFile】\r\n");
				DeleteDirFile();	    /* 删除文件夹和文件  */
				break;
			
			case '6':
				xprintf("【6 - WriteFileTest】\r\n");
				WriteFileTest();	    /* 测试文件读写速度  */
				break;

			case '7':
				xprintf("【7 - WriteCSVFile】\r\n");
				WriteCSVFile();	        /* 写数据到CSV文件中  */
				break;
			
			case '8':
				xprintf("【8 - ReadCSVFileData】\r\n");
				ReadCSVFileData();	    /* 从CSV文件中读数据  */
				break;
			
			case '9':
				xprintf("【9 - CreateNewDirFileTest】\r\n");
				CreateNewDirFileTest();
				break;
			
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
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispMenu(void)
{
	xprintf("\r\n------------------------------------------------\r\n");
	xprintf("请选择串口操作命令，电脑键盘打印数字即可:\r\n");
	xprintf("首次使用，务必调用串口命令0，进行NAND Flash的低级格式化和文件系统格式化\r\n");	
	xprintf("0 - 低级格式化，显示NAND Flash容量和剩余容量\r\n");
	xprintf("1 - 显示NAND Flash根目录下的文件\r\n");
	xprintf("2 - 创建三个text文本并使用不同函数写入内容\r\n");
	xprintf("3 - 使用三个不同函数读取文本\r\n");
	xprintf("4 - 创建一个text文本并指定一个位置对其进行读写操作\r\n");
	xprintf("5 - 删除文件夹和文件\r\n");
	xprintf("6 - 测试文件读写速度\r\n");
	xprintf("7 - 写数据到CSV文件中\r\n");
	xprintf("8 - 从CSV文件中读数据\r\n");
	xprintf("9 - 测试文件和文件夹创建速度\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: DotFormat
*	功能说明: 将数据规范化显示，方便用户查看
*             比如
*             2345678   ---->  2.345.678
*             334426783 ---->  334.426.783
*             以三个数为单位进行显示
*	形    参: _ullVal   需要规范显示的数值
*             _sp       规范显示后数据存储的buf。
*	返 回 值: 无
*********************************************************************************************************
*/
static void DotFormat(uint64_t _ullVal, char *_sp) 
{
	/* 数值大于等于10^9 */
	if (_ullVal >= (U64)1e9) 
	{
		_sp += sprintf (_sp, "%d.", (uint32_t)(_ullVal / (uint64_t)1e9));
		_ullVal %= (uint64_t)1e9;
		_sp += sprintf (_sp, "%03d.", (uint32_t)(_ullVal / (uint64_t)1e6));
		_ullVal %= (uint64_t)1e6;
		sprintf (_sp, "%03d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
		return;
	}
	
	/* 数值大于等于10^6 */
	if (_ullVal >= (uint64_t)1e6) 
	{
		_sp += sprintf (_sp,"%d.", (uint32_t)(_ullVal / (uint64_t)1e6));
		_ullVal %= (uint64_t)1e6;
		sprintf (_sp,"%03d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
		return;
	}
	
	/* 数值大于等于10^3 */
	if (_ullVal >= 1000) 
	{
		sprintf (_sp, "%d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
		return;
	}
	
	/* 其它数值 */
	sprintf (_sp,"%d",(U32)(_ullVal));
}

/*
*********************************************************************************************************
*	函 数 名: ViewNandCapacity
*	功能说明: NAND Flash低级格式化，挂载，卸载及其容量显示
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void ViewNandCapacity(void)
{
	uint8_t result;
	Media_INFO info;
	uint64_t ullNANDCapacity;
	FAT_VI *mc0;  
	uint8_t buf[15];
	
	/* 
	   低级格式化，注意一定要执行了函数NAND_Init后再执行NAND_Format()，因为：
	   1. finit("N0:") 会调用NAND_Init，见FlahFS对nandflash的接口文件FS_NAND_FlashPrg.c
	   2. funinit("N0:") 会调用UnInit，见FlahFS对nandflash的接口文件FS_NAND_FlashPrg.c
	*/
	NAND_Init();
	xprintf("正在进行低级格式化中....\r\n");
	NAND_Format();
	xprintf("低级格式化完成....\r\n");

	/* 加载SD卡 */
	result = finit("N0:");
	if(result != NULL)
	{
		xprintf("挂载文件系统失败 (%s)\r\n", ReVal_Table[result]);
		xprintf("挂载失败，NAND Flash需要进行FAT32格式化\r\n");
	}
	else
	{
		xprintf("挂载文件系统成功 (%s)\r\n", ReVal_Table[result]);
	}
	
	xprintf("正在进行FAT32格式化中....\r\n");
	if (fformat ("N0:") != 0)  
	{            
		xprintf ("格式化失败\r\n");
	}
	else  
	{
		xprintf ("格式化成功\r\n");
	}
	
	xprintf("------------------------------------------------------------------\r\n");
	/* 获取volume label */
	if (fvol ("N0:", (char *)buf) == 0) 
	{
		if (buf[0]) 
		{
			xprintf ("NAND Flash的volume label是 %s\r\n", buf);
		}
		else 
		{
			xprintf ("NAND Flash没有volume label\r\n");
		}
	}
	else 
	{
		xprintf ("Volume访问错误\r\n");
	}

	/* 获取NAND Flash剩余容量 */
	ullNANDCapacity = ffree("N0:");
	DotFormat(ullNANDCapacity, (char *)buf);
	xprintf("NAND Flash剩余容量 = %10s字节\r\n", buf);
	
	/* 卸载NAND Flash */
	result = funinit("N0:");
	if(result != NULL)
	{
		xprintf("卸载文件系统失败\r\n");
	}
	else
	{
		xprintf("卸载文件系统成功\r\n");
	}
	
	/* 获取相应存储设备的句柄 */
	mc0 = ioc_getcb("N0:");          
   
	/* 初始化FAT文件系统格式的存储设备 */
	if (ioc_init (mc0) == 0) 
	{
		/* 获取存储设备的扇区信息 */
		ioc_read_info (&info, mc0);

		/* 总的扇区数 * 扇区大小，NAND Flash的扇区大小是512字节 */
		ullNANDCapacity = (uint64_t)info.block_cnt << 9;
		DotFormat(ullNANDCapacity, (char *)buf);
		xprintf("NAND Flash总容量 = %10s字节\r\nNAND Flash的总扇区数 = %d \r\n", buf, info.block_cnt);
	
		xprintf("NAND Flash读扇区大小 = %d字节\r\n", info.read_blen);
		xprintf("NAND Flash写扇区大小 = %d字节\r\n", info.write_blen);
	}
	else 
	{
		xprintf("初始化失败\r\n");
	}
	
	/* 卸载NAND Flash */
	if(ioc_uninit (mc0) != NULL)
	{
		xprintf("卸载失败\r\n");		
	}
	else
	{
		xprintf("卸载成功\r\n");	
	}
	xprintf("------------------------------------------------------------------\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: ViewRootDir
*	功能说明: 显示NAND Flash目录下的文件名
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void ViewRootDir(void)
{
	uint8_t result;
	FINFO info;
	uint64_t ullNANDCapacity;
	uint8_t buf[15];
	
    info.fileID = 0;                /* 每次使用ffind函数前，info.fileID必须初始化为0 */

	/* 加载NAND Flash */
	result = finit("N0:");
	if(result != NULL)
	{
		/* 如果挂载失败，务必不要再调用FlashFS的其它API函数，防止进入硬件异常 */
		xprintf("挂载文件系统失败 (%s)\r\n", ReVal_Table[result]);
		goto access_fail;
	}
	else
	{
		xprintf("挂载文件系统成功 (%s)\r\n", ReVal_Table[result]);
	}
	xprintf("------------------------------------------------------------------\r\n");
	xprintf("文件名                 |  文件大小     | 文件ID  | 属性      |日期\r\n");
	
	/* 
	   将根目录下的所有文件列出来。
	   1. "*" 或者 "*.*" 搜索指定路径下的所有文件
	   2. "abc*"         搜索指定路径下以abc开头的所有文件
	   3. "*.jpg"        搜索指定路径下以.jpg结尾的所有文件
	   4. "abc*.jpg"     搜索指定路径下以abc开头和.jpg结尾的所有文件
	
	   以下是实现搜索根目录下所有文件
	*/
	while(ffind ("N0:*.*", &info) == 0)  
	{ 
		/* 调整文件显示大小格式 */
		DotFormat(info.size, (char *)buf);
		
		/* 打印根目录下的所有文件 */
		xprintf ("%-20s %12s bytes, ID: %04d  ",
				info.name,
				buf,
				info.fileID);
		
		/* 判断是文件还是子目录 */
		if (info.attrib & ATTR_DIRECTORY)
		{
			xprintf("(0x%02x)目录", info.attrib);
		}
		else
		{
			xprintf("(0x%02x)文件", info.attrib);
		}
		
		/* 显示文件日期 */
		xprintf ("  %04d.%02d.%02d  %02d:%02d\r\n",
                 info.time.year, info.time.mon, info.time.day,
               info.time.hr, info.time.min);
    }
	
	if (info.fileID == 0)  
	{
		xprintf ("NAND Flash中没有存放文件\r\n");
	}
	
	/* 获取NAND Flash剩余容量 */
	ullNANDCapacity = ffree("N0:");
	DotFormat(ullNANDCapacity, (char *)buf);
	xprintf("NAND FLASH的容量 = %10s字节\r\n", buf);

access_fail:
	/* 卸载NAND Flash */
	result = funinit("N0:");
	if(result != NULL)
	{
		xprintf("卸载文件系统失败\r\n");
	}
	else
	{
		xprintf("卸载文件系统成功\r\n");
	}
	
	xprintf("------------------------------------------------------------------\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: CreateNewFile
*	功能说明: 在NAND Flash创建三个text文本，分别使用fwrite，fprintf和fputs写数据。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void CreateNewFile(void)
{
	const uint8_t WriteText[] = {"STM32 Demo application\r\n2018-03-09\r\n"};
	const uint8_t WriteText1[] = {"STM32 Demo application Test...\r"};
	FILE *fout;
	uint32_t bw;
	uint32_t i = 2;
	uint8_t result;

	/* 加载NAND Flash */
	result = finit("N0:");
	if(result != NULL)
	{
		/* 如果挂载失败，务必不要再调用FlashFS的其它API函数，防止进入硬件异常 */
		xprintf("挂载文件系统失败 (%s)\r\n", ReVal_Table[result]);
		goto access_fail;
	}
	else
	{
		xprintf("挂载文件系统成功 (%s)\r\n", ReVal_Table[result]);
	}
	xprintf("------------------------------------------------------------------\r\n");
	
	/**********************************************************************************************************/
	/* 打开文件夹test中的文件test1.txt，如果没有子文件夹和txt文件会自动创建*/
	fout = fopen ("N0:\\test\\test1.txt", "w"); 
	if (fout != NULL) 
	{
		xprintf("打开文件N0:\\test\\test1.txt成功，如果没有子文件夹和txt文件会自动创建\r\n");
		/* 写数据 */
		bw = fwrite (WriteText, sizeof(uint8_t), sizeof(WriteText)/sizeof(uint8_t), fout);
		if(bw == sizeof(WriteText)/sizeof(uint8_t))
		{
			xprintf("写入内容成功\r\n");
		}
		else
		{ 
			xprintf("写入内容失败\r\n");
		}
		
		/* 关闭文件 */
		fclose(fout);
	}
	else
	{
		xprintf("打开文件N0:\\test\\test.txt失败\r\n");
	}
	
	/**********************************************************************************************************/
	/* 打开文件夹test中的文件test2.txt，如果没有子文件夹和txt文件会自动创建*/
	fout = fopen ("N0:\\test\\test2.txt", "w"); 
	if (fout != NULL) 
	{
		xprintf("打开文件N0:\\test\\test2.txt成功，如果没有子文件夹和txt文件会自动创建\r\n");
		
		/* 写数据 */
		bw = fprintf (fout, "%d %d %f\r\n", i, i*5, i*5.55f);
		
		/* 使用函数ferror检测是否发生过错误 */
		if (ferror(fout) != NULL)  
		{
			xprintf("写入内容失败\r\n");
		}
		else
		{
			xprintf("写入内容成功\r\n");	
		}
		
		/* 关闭文件 */
		fclose(fout);
	}
	else
	{
		xprintf("打开文件N0:\\test\\test.txt失败\r\n");
	}
	
	/**********************************************************************************************************/
	/* 打开文件夹test中的文件test3.txt，如果没有子文件夹和txt文件会自动创建*/
	fout = fopen ("N0:\\test\\test3.txt", "w"); 
	if (fout != NULL) 
	{
		xprintf("打开文件N0:\\test\\test3.txt成功，如果没有子文件夹和txt文件会自动创建\r\n");
		
		/* 写数据 */
		fputs((const char *)WriteText1, fout);
		
		/* 使用函数ferror检测是否发生过错误 */
		if (ferror(fout) != NULL)  
		{
			xprintf("写入内容失败\r\n");
		}
		else
		{
			xprintf("写入内容成功\r\n");	
		}
		
		/* 关闭文件 */
		fclose(fout);
	}
	else
	{
		xprintf("打开文件N0:\\test\\test.txt失败\r\n");
	}

access_fail:	
	/* 卸载NAND Flash */
	result = funinit("N0:");
	if(result != NULL)
	{
		xprintf("卸载文件系统失败\r\n");
	}
	else
	{
		xprintf("卸载文件系统成功\r\n");
	}

	xprintf("------------------------------------------------------------------\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: ReadFileData
*	功能说明: 分别使用fread，fscan和fgets读取三个不同的txt文件。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void ReadFileData(void)
{
	const uint8_t WriteText[] = {"STM32 Demo application\r\n2018-03-09\r\n"};
	uint8_t Readbuf[100];
	FILE *fin;
	uint32_t bw;
	uint32_t index1, index2;
	float  count = 0.0f;
	uint8_t result;

	/* 加载NAND Flash */
	result = finit("N0:");
	if(result != NULL)
	{
		/* 如果挂载失败，务必不要再调用FlashFS的其它API函数，防止进入硬件异常 */
		xprintf("挂载文件系统失败 (%s)\r\n", ReVal_Table[result]);
		goto access_fail;
	}
	else
	{
		xprintf("挂载文件系统成功 (%s)\r\n", ReVal_Table[result]);
	}
	xprintf("------------------------------------------------------------------\r\n");
	
	/**********************************************************************************************************/
	/* 打开文件夹test中的文件test1.txt */
	fin = fopen ("N0:\\test\\test1.txt", "r"); 
	if (fin != NULL) 
	{
		xprintf("<1>打开文件N0:\\test\\test1.txt成功\r\n");
		
		/* 防止警告 */
		(void) WriteText;
		
		/* 读数据 */
		bw = fread(Readbuf, sizeof(uint8_t), sizeof(WriteText)/sizeof(uint8_t), fin);
		if(bw == sizeof(WriteText)/sizeof(uint8_t))
		{
			Readbuf[bw] = NULL;
			xprintf("test1.txt 文件内容 : \r\n%s\r\n", Readbuf);
		}
		else
		{ 
			xprintf("读内容失败\r\n");
		}
		
		/* 关闭文件 */
		fclose(fin);
	}
	else
	{
		xprintf("打开文件N0:\\test\\test.txt失败, 可能文件不存在\r\n");
	}
	
	/**********************************************************************************************************/
	/* 打开文件夹test中的文件test2.txt */
	fin = fopen ("N0:\\test\\test2.txt", "r"); 
	if (fin != NULL) 
	{
		xprintf("\r\n<2>打开文件N0:\\test\\test2.txt成功\r\n");
		
		bw = fscanf(fin, "%d %d %f", &index1, &index2, &count);

		/* 3参数都读出来了 */
		if (bw == 3)  
		{
			xprintf("读出的数值\r\nindex1 = %d index2 = %d count = %f\r\n", index1, index2, count);
		}
		else
		{
			xprintf("读内容失败\r\n");	
		}
		
		/* 关闭文件 */
		fclose(fin);
	}
	else
	{
		xprintf("打开文件N0:\\test\\test.txt失败\r\n");
	}
	
	/**********************************************************************************************************/
	/* 打开文件夹test中的文件test3.txt，如果没有子文件夹和txt文件会自动创建*/
	fin = fopen ("N0:\\test\\test3.txt", "r"); 
	if (fin != NULL) 
	{
		xprintf("\r\n<3>打开文件N0:\\test\\test3.txt成功\r\n");
		
		/* 读数据 */
		if(fgets((char *)Readbuf, sizeof(Readbuf), fin) != NULL)
		{
			xprintf("test3.txt 文件内容 : \r\n%s\r\n", Readbuf);
		}
		else
		{
			xprintf("读内容失败\r\n");
		}
		
		/* 关闭文件 */
		fclose(fin);
	}
	else
	{
		xprintf("打开文件N0:\\test\\test.txt失败\r\n");
	}
	
access_fail:
	/* 卸载NAND Flash */
	result = funinit("N0:");
	if(result != NULL)
	{
		xprintf("卸载文件系统失败\r\n");
	}
	else
	{
		xprintf("卸载文件系统成功\r\n");
	}

	xprintf("------------------------------------------------------------------\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: SeekFileData
*	功能说明: 创建一个text文本，在指定位置对其进行读写操作。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void SeekFileData(void)
{
	const uint8_t WriteText[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"};
	FILE *fin, *fout;
	uint32_t bw;
	uint32_t uiPos;
	uint8_t ucChar;
	uint8_t result;

	/* 加载NAND Flash */
	result = finit("N0:");
	if(result != NULL)
	{
		/* 如果挂载失败，务必不要再调用FlashFS的其它API函数，防止进入硬件异常 */
		xprintf("挂载文件系统失败 (%s)\r\n", ReVal_Table[result]);
		goto access_fail;
	}
	else
	{
		xprintf("挂载文件系统成功 (%s)\r\n", ReVal_Table[result]);
	}
	xprintf("------------------------------------------------------------------\r\n");
	
	/**********************************************************************************************************/
	/* 打开文件夹test中的文件test1.txt，如果没有子文件夹和txt文件会自动创建*/
	fout = fopen ("N0:\\test.txt", "w"); 
	if (fout != NULL) 
	{
		xprintf("打开文件N0:\\test.txt成功，如果没有txt文件会自动创建\r\n");
		/* 写数据 */
		bw = fwrite (WriteText, sizeof(uint8_t), sizeof(WriteText)/sizeof(uint8_t), fout);
		if(bw == sizeof(WriteText)/sizeof(uint8_t))
		{
			xprintf("写入内容成功\r\n");
		}
		else
		{ 
			xprintf("写入内容失败\r\n");
		}
			
		/* 关闭文件 */
		fclose(fout);
	}
	else
	{
		xprintf("打开文件N0:\\test.txt失败\r\n");
	}
	
	/***********************************************/
	fin = fopen ("N0:\\test.txt","r");
	if (fin != NULL)  
	{
		xprintf("\r\n打开文件N0:\\test.txt成功\r\n");
		
		/* 读取文件test.txt的位置0的字符 */
		fseek (fin, 0L, SEEK_SET);
		uiPos = ftell(fin); 	
		ucChar = fgetc (fin);		
		xprintf("文件test.txt当前读取位置：%02d，字符：%c\r\n", uiPos, ucChar);
		
		/* 读取文件test.txt的位置1的字符 */
		fseek (fin, 1L, SEEK_SET);
		uiPos = ftell(fin); 	
		ucChar = fgetc (fin);		
		xprintf("文件test.txt当前读取位置：%02d，字符：%c\r\n", uiPos, ucChar);

		/* 读取文件test.txt的位置25的字符 */
		fseek (fin, 25L, SEEK_SET);
		uiPos = ftell(fin); 	
		ucChar = fgetc (fin);		
		xprintf("文件test.txt当前读取位置：%02d，字符：%c\r\n", uiPos, ucChar);
		
		/* 通过上面函数的操作，当前读写位置是26
		   下面函数是在当前位置的基础上后退2个位置，也就是24，调用函数fgetc后，位置就是25
		 */
		fseek (fin, -2L, SEEK_CUR);
		uiPos = ftell(fin); 	
		ucChar = fgetc (fin);		
		xprintf("文件test.txt当前读取位置：%02d，字符：%c\r\n", uiPos, ucChar);
		
		/* 读取文件test.txt的倒数第2个字符, 最后一个是'\0' */
		fseek (fin, -2L, SEEK_END); 
		uiPos = ftell(fin); 
		ucChar = fgetc (fin);
		xprintf("文件test.txt当前读取位置：%02d，字符：%c\r\n", uiPos, ucChar);
		
		/* 将读取位置重新定位到文件开头 */
		rewind(fin);  
		uiPos = ftell(fin); 
		ucChar = fgetc (fin);
		xprintf("文件test.txt当前读取位置：%02d，字符：%c\r\n", uiPos, ucChar);	
		
		/* 
		   这里是演示一下ungetc的作用，此函数就是将当前的读取位置偏移回一个字符，
		   而fgetc调用后位置增加一个字符。
		 */
		fseek (fin, 0L, SEEK_SET);
		ucChar = fgetc (fin);
		uiPos = ftell(fin); 
		xprintf("文件test.txt当前读取位置：%02d，字符：%c\r\n", uiPos, ucChar);
		ungetc(ucChar, fin); 
		uiPos = ftell(fin); 
		xprintf("文件test.txt当前读取位置：%02d，字符：%c\r\n", uiPos, ucChar);
		
		/* 关闭*/
		fclose (fin);
	}
	else
	{
		xprintf("打开文件N0:\\test.txt失败\r\n");
	}
	
	/***********************************************/
	fin = fopen ("N0:\\test.txt","r+");
	if (fin != NULL)  
	{
		xprintf("\r\n打开文件N0:\\test.txt成功\r\n");
		
		/* 文件test.txt的位置2插入新字符 '！' */
		fseek (fin, 2L, SEEK_SET);
		ucChar = fputc ('!', fin);
		/* 刷新数据到文件内 */
		fflush(fin);		

		fseek (fin, 2L, SEEK_SET);
		uiPos = ftell(fin); 
		ucChar = fgetc (fin);				
		xprintf("文件test.txt位置：%02d，已经插入新字符：%c\r\n", uiPos, ucChar);
	
		
		/* 文件test.txt的倒数第2个字符, 插入新字符 ‘&’ ，最后一个是'\0' */
		fseek (fin, -2L, SEEK_END); 
		ucChar = fputc ('&', fin);
		/* 刷新数据到文件内 */
		fflush(fin);	

		fseek (fin, -2L, SEEK_END); 
		uiPos = ftell(fin); 
		ucChar = fgetc (fin);	
		xprintf("文件test.txt位置：%02d，已经插入新字符：%c\r\n", uiPos, ucChar);		
		
		/* 关闭*/
		fclose (fin);
	}
	else
	{
		xprintf("打开文件N0:\\test.txt失败\r\n");
	}
	
access_fail:	
	/* 卸载NAND Flash */
	result = funinit("N0:");
	if(result != NULL)
	{
		xprintf("卸载文件系统失败\r\n");
	}
	else
	{
		xprintf("卸载文件系统成功\r\n");
	}

	xprintf("------------------------------------------------------------------\r\n");

}

/*
*********************************************************************************************************
*	函 数 名: DeleteDirFile
*	功能说明: 删除文件夹和文件
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DeleteDirFile(void)
{
	uint8_t result;
	uint8_t i;
	char FileName[50];
	
	/* 加载NAND Flash */
	result = finit("N0:");
	if(result != NULL)
	{
		/* 如果挂载失败，务必不要再调用FlashFS的其它API函数，防止进入硬件异常 */
		xprintf("挂载文件系统失败 (%s)\r\n", ReVal_Table[result]);
		goto access_fail;
	}
	else
	{
		xprintf("挂载文件系统成功 (%s)\r\n", ReVal_Table[result]);
	}
	xprintf("------------------------------------------------------------------\r\n");
	
	/* 删除文件 speed1.txt */
	for (i = 0; i < 20; i++)
	{
		xprintf(FileName, "N0:\\Speed%02d.txt", i);		/* 每写1次，序号递增 */
		result = fdelete (FileName);
		if (result != NULL) 
		{
			xprintf("%s文件不存在（返回值：%d）\r\n", FileName, result);
		}
		else
		{
			xprintf("删除%s文件成功\r\n", FileName);
		}
	}
	
	for(i = 1; i < 4; i++)
	{
		xprintf(FileName, "N0:\\test\\test%01d.txt", i);   /* 每写1次，序号递增 */
		result = fdelete (FileName);
		if (result != NULL) 
		{
			xprintf("%s文件不存在（返回值：%d）\r\n", FileName, result);
		}
		else
		{
			xprintf("删除%s文件成功\r\n", FileName);
		}	
	}
	
	/* 删除文件夹test */
	result = fdelete ("N0:\\test\\");
	if (result != NULL) 
	{
		xprintf("test文件夹非空或不存在（返回值：%d）\r\n", result);
	}
	else
	{
		xprintf("删除test文件夹成功\r\n");
	}
	
	/* 删除csv文件 */
	result = fdelete ("N0:\\record.csv");
	if (result != NULL) 
	{
		xprintf("record.csv文件不存在（返回值：%d）\r\n", result);
	}
	else
	{
		xprintf("删除record.csv件成功\r\n");
	}
	
access_fail:
	/* 卸载NAND Flash */
	result = funinit("N0:");
	if(result != NULL)
	{
		xprintf("卸载文件系统失败\r\n");
	}
	else
	{
		xprintf("卸载文件系统成功\r\n");
	}
		
	xprintf("------------------------------------------------------------------\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: WriteFileTest
*	功能说明: 测试文件读写速度
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void WriteFileTest(void)
{
	FILE *fout;
	uint32_t bw;
	uint32_t i,k;
	uint32_t runtime1,runtime2,timelen;
	uint8_t err = 0, result;
	char TestFileName[13];
	static uint8_t s_ucTestSn = 0;

	for (i = 0; i < sizeof(g_TestBuf); i++)
	{
		g_TestBuf[i] = (i / 512) + '0';
	}

	/* 加载NAND Flash -----------------------------------------*/
	result = finit("N0:");
	if(result != NULL)
	{
		/* 如果挂载失败，务必不要再调用FlashFS的其它API函数，防止进入硬件异常 */
		xprintf("挂载文件系统失败 (%s)\r\n", ReVal_Table[result]);
		goto access_fail;
	}
	else
	{
		xprintf("挂载文件系统成功 (%s)\r\n", ReVal_Table[result]);
	}
	
	xprintf("------------------------------------------------------------------\r\n");

	/* 打开文件 -----------------------------------------*/
	sprintf(TestFileName, "N0:\\Speed%02d.txt", s_ucTestSn++);		/* 每写1次，序号递增 */
	fout = fopen (TestFileName, "w");
	if (fout ==  NULL)
	{
		xprintf("文件: %s创建失败\r\n", TestFileName);
		goto access_fail;
	}
	
	/* 写一串数据 测试写速度-----------------------------------------*/
	xprintf("开始写文件%s %dKB ...\r\n", TestFileName, TEST_FILE_LEN / 1024);
	runtime1 = GetSystemTimer();	// = os_time_get ();	/* 读取系统运行时间 */
	for (i = 0; i < TEST_FILE_LEN / BUF_SIZE; i++)
	{
		bw = fwrite (g_TestBuf, sizeof(uint8_t), BUF_SIZE, fout);   
		if (bw == BUF_SIZE)
		{
			if (((i + 1) % 8) == 0)
			{
				xprintf(".");
			}
		}
		else
		{
			err = 1;
			xprintf("%s文件写失败\r\n", TestFileName);
			break;
		}
	}
	runtime2 = GetSystemTimer();	//= os_time_get ();	/* 读取系统运行时间 */

	if (err == 0)
	{
		timelen = (runtime2 - runtime1);
		xprintf("\r\n  写耗时 : %dms   平均写速度 : %dB/S (%dKB/S)\r\n",
			timelen,
			(TEST_FILE_LEN * 1000) / timelen,
			((TEST_FILE_LEN / 1024) * 1000) / timelen);
	}
	
	/* 使用函数ferror再次检测是否发生过错误 -----------------------*/
	if (ferror (fout))  
	{
		xprintf ("写数据过程中出现过错误\r\n\n");
    }
	else
	{
		xprintf ("写数据过程中未出现过错误\r\n\n");		
	}

	fclose (fout);	/* 关闭文件*/


	/* 开始读文件，测试读速度 ---------------------------------------*/
	fout = fopen (TestFileName, "r");
	if (fout ==  NULL)
	{
		xprintf("文件: %s读取失败\r\n", TestFileName);
		goto access_fail;
	}

	xprintf("开始读文件 %dKB ...\r\n", TEST_FILE_LEN / 1024);
	runtime1 = GetSystemTimer();	//= os_time_get ();	/* 读取系统运行时间 */
	for (i = 0; i < TEST_FILE_LEN / BUF_SIZE; i++)
	{
		bw = fread(g_TestBuf, sizeof(uint8_t), BUF_SIZE, fout);   
		if (bw == BUF_SIZE)
		{
			if (((i + 1) % 8) == 0)
			{
				xprintf(".");
			}

			/* 比较写入的数据是否正确，此语句会导致读卡速度结果降低 */
			for (k = 0; k < sizeof(g_TestBuf); k++)
			{
				if (g_TestBuf[k] != (k / 512) + '0')
				{
				  	err = 1;
					xprintf("Speed1.txt 文件读成功，但是数据出错\r\n");
					break;
				}
			}
			if (err == 1)
			{
				break;
			}
		}
		else
		{
			err = 1;
			xprintf("Speed1.txt 文件读失败\r\n");
			break;
		}
	}
	runtime2 = GetSystemTimer();	//= os_time_get ();	/* 读取系统运行时间 */

	if (err == 0)
	{
		timelen = (runtime2 - runtime1);
		xprintf("\r\n  读耗时 : %dms   平均读速度 : %dB/S (%dKB/S)\r\n", timelen,
			(TEST_FILE_LEN * 1000) / timelen, ((TEST_FILE_LEN / 1024) * 1000) / timelen);
	}
	
	/* 使用函数ferror再次检测是否发生过错误 -----------------------*/
	if (ferror (fout))  
	{
		xprintf ("读数据过程中出现过错误\r\n");
    }
	else
	{
		xprintf ("读数据过程中未出现过错误\r\n");		
	}

	fclose (fout);	/* 关闭文件*/
	
access_fail:
	/* 卸载NAND Flash */
	result = funinit("N0:");
	if(result != NULL)
	{
		xprintf("卸载文件系统失败\r\n");
	}
	else
	{
		xprintf("卸载文件系统成功\r\n");
	}
	
	xprintf("------------------------------------------------------------------\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: WriteCSVFile
*	功能说明: 写数据到CSV文件中
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void WriteCSVFile(void)
{
	const uint8_t WriteText[] = {"记录时间,标号,记录数值一,记录数值二\r\n"};
	FILE *fout;
	uint8_t result;
	static uint16_t i = 1;
	static uint8_t ucFirstRunFlag = 0;

	/* 加载SD卡 */
	result = finit("N0:");
	if(result != NULL)
	{
		/* 如果挂载失败，务必不要再调用FlashFS的其它API函数，防止进入硬件异常 */
		xprintf("挂载文件系统失败 (%s)\r\n", ReVal_Table[result]);
		goto access_fail;
	}
	else
	{
		xprintf("挂载文件系统成功 (%s)\r\n", ReVal_Table[result]);
	}
	xprintf("------------------------------------------------------------------\r\n");
	
	/**********************************************************************************************************/
	/* 
	  1. 打开文件record.csv，如果没有此文件会自动创建。
	  2. 第二个参数表示向此文件写数据都从尾部开始添加。
	*/
	fout = fopen ("N0:\\record.csv", "a"); 
	if (fout != NULL) 
	{
		xprintf("打开文件N0:\\record.csvt成功，如果没有此文件会自动创建\r\n");
		
		/* 写数据，如果是第一次写数据，先写CSV文件的表项的题目，以后写数据不再需要写此表项 */
		if(ucFirstRunFlag == 0)
		{
			fprintf(fout, (char *)WriteText);
			ucFirstRunFlag = 1;
		}
		
		/* 依次写5行数据 */
		fprintf (fout, "%d, %d, %f\r\n", i, i*5, i*5.55f);
		
		i++;
		fprintf (fout, "%d, %d, %f\r\n", i, i*5, i*5.55f);
		
		i++;
		fprintf (fout, "%d, %d, %f\r\n", i, i*5, i*5.55f);
		
		i++;
		fprintf (fout, "%d, %d, %f\r\n", i, i*5, i*5.55f);
		
		i++;
		fprintf (fout, "%d, %d, %f\r\n", i, i*5, i*5.55f);
		i++;
		
		/* 使用函数ferror检测是否发生过错误 */
		if (ferror(fout) != NULL)  
		{
			xprintf("写入内容失败\r\n");
		}
		else
		{
			xprintf("写入内容成功\r\n");	
		}
		
		/* 关闭文件 */
		fclose(fout);
	}
	else
	{
		xprintf("打开文件N0:\\record.csv失败\r\n");
	}

access_fail:	
	/* 卸载SD卡 */
	result = funinit("N0:");
	if(result != NULL)
	{
		xprintf("卸载文件系统失败\r\n");
	}
	else
	{
		xprintf("卸载文件系统成功\r\n");
	}

	xprintf("------------------------------------------------------------------\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: ReadCSVFileData
*	功能说明: 读取CSV文件中的数据。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void ReadCSVFileData(void)
{
	FILE *fin;
	uint32_t bw;
	uint8_t result;

	/* 加载SD卡 */
	result = finit("N0:");
	if(result != NULL)
	{
		/* 如果挂载失败，务必不要再调用FlashFS的其它API函数，防止进入硬件异常 */
		xprintf("挂载文件系统失败 (%s)\r\n", ReVal_Table[result]);
		goto access_fail;
	}
	else
	{
		xprintf("挂载文件系统成功 (%s)\r\n", ReVal_Table[result]);
	}
	xprintf("------------------------------------------------------------------\r\n");
	
	/*****************************************************.
	*****************************************************/
	/* 打开record.csv文件 */
	fin = fopen ("N0:\\record.csv", "r"); 
	if (fin != NULL) 
	{
		xprintf("打开文件N0:\\record.csv成功\r\n");
		/* 将record.csv文件中的所有数据都读取出来 */
        while(1)
		{
			bw = fread(g_TestBuf, sizeof(uint8_t), BUF_SIZE/2, fin);
			/* 末尾加上结束符，设置串口打印截止位置 */
			g_TestBuf[bw] = '\0';
			xprintf("%s", g_TestBuf);
			if(bw < BUF_SIZE/2) break;	
		}
		
		/* 使用函数ferror检测是否发生过错误 */
		if (ferror(fin) != NULL)  
		{
			xprintf("读出内容失败\r\n");
		}
		else
		{
			xprintf("record.csv里面的内容全部正确读出\r\n");	
		}
		
		/* 关闭文件 */
		fclose(fin);
	}
	else
	{
		xprintf("打开文件N0:\\record.csv失败，可能文件不存在，已经被删除\r\n");
	}
	
access_fail:
	/* 卸载SD卡 */
	result = funinit("N0:");
	if(result != NULL)
	{
		xprintf("卸载文件系统失败\r\n");
	}
	else
	{
		xprintf("卸载文件系统成功\r\n");
	}

	xprintf("------------------------------------------------------------------\r\n");
}


/*
*********************************************************************************************************
*	函 数 名: CreateNewDirFileTest
*	功能说明: 在NAND Flash创建三个text文本，分别使用fwrite，fprintf和fputs写数据。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void CreateNewDirFileTest(void)
{
	const uint8_t WriteText[] = {"STM32 Demo application\r\n2018-03-09\r\n"};
	const uint8_t WriteText1[] = {"STM32 Demo application Test...\r"};
	FILE *fout;
	uint32_t bw;
	uint32_t i = 2;
	uint8_t result;
	uint32_t rng_data,Filenum=0;
	char filename[100];
	
	rng_data = GetRandomNumber();
	
	/* 加载NAND Flash */
	result = finit("N0:");
	if(result != NULL)
	{
		/* 如果挂载失败，务必不要再调用FlashFS的其它API函数，防止进入硬件异常 */
		xprintf("挂载文件系统失败 (%s)\r\n", ReVal_Table[result]);
		goto access_fail;
	}
	else
	{
		xprintf("挂载文件系统成功 (%s)\r\n", ReVal_Table[result]);
	}
	xprintf("------------------------------------------------------------------\r\n");
	
	/**********************************************************************************************************/
	/* 打开文件夹test中的文件test1.txt，如果没有子文件夹和txt文件会自动创建*/
	while(Filenum++<20000)
	{
		//sprintf(filename, "N0:\\dir%d-%02d\\dir2-%02d\\dir3-%02d\\dir4-%02d\\Test%02d.txt", (uint8_t)rng_data,Filenum,Filenum,Filenum,Filenum,Filenum);
		sprintf(filename, "N0:\\Test%02d.txt", Filenum);		
		xprintf("Open file\r\n");
		xprintf(filename);
		DisplayTime(0);
		fout = fopen (filename, "w"); 
		DisplayTime(1);
		if (fout != NULL) 
		{
			/* 写数据 */
			bw = fwrite (WriteText, sizeof(uint8_t), sizeof(WriteText)/sizeof(uint8_t), fout);
			if(bw == sizeof(WriteText)/sizeof(uint8_t))
			{
				//xprintf("写入内容成功\r\n");
			}
			else
			{ 
				xprintf("写入内容失败\r\n");
				break;
			}
			
			/* 关闭文件 */
			fclose(fout);
		}
		else
		{
			xprintf("打开文件失败\r\n");
			break;
		}
	}

access_fail:	
	/* 卸载NAND Flash */
	result = funinit("N0:");
	if(result != NULL)
	{
		xprintf("卸载文件系统失败\r\n");
	}
	else
	{
		xprintf("卸载文件系统成功\r\n");
	}

	xprintf("------------------------------------------------------------------\r\n");
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
