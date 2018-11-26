
#if defined(TWO_SDCARDS)	

#include "stm32f2xx.h"
#include "typedef.h"
#include "ff.h"		
#include "demo_nand_fatfs.h"
#include "monitor.h"
#include "SysTick.h"

/* 用于测试读写速度 */
#define TEST_FILE_LEN			(2*1024*1024)	/* 用于测试的文件长度 */
#define BUF_SIZE				(4*1024)		/* 每次读写SD卡的最大数据长度 */
extern uint8_t g_TestBuf[BUF_SIZE];

/* 仅允许本文件内调用的函数声明 */
static void DispMenu(void);
static void ViewRootDir(void);
static void CreateNewFile(void);
static void ReadFileData(void);
static void CreateDir(void);
static void DeleteDirFile(void);
static void WriteFileTest(void);
static void CreateNewDirFileTest(void);
static void ViewNandCapacity(void);

/*
*********************************************************************************************************
*	函 数 名: DemoFatFS
*	功能说明: FatFS文件系统演示主程序
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void DemoFatFS_Nand(void)
{
	char *ptr;
	FATFS fs;	
	FRESULT result;

	/* 打印命令列表，用户可以通过串口操作指令 */
	DispMenu();
	f_mount(FS_NAND, &fs);
	
	while(1)
	{
		ptr = ConsoleLine;
		ConsoleLine[0]=0;
		get_line(ptr, sizeof(ConsoleLine));

		switch (*ptr++)
		{
			case '1':
				xprintf("【1 - ViewRootDir】\r\n");
				ViewRootDir();		/* 显示根目录下的文件名 */
				break;

			case '2':
				xprintf("【2 - CreateNewFile】\r\n");
				CreateNewFile();		/* 创建一个新文件,写入一个字符串 */
				break;

			case '3':
				xprintf("【3 - ReadFileData】\r\n");
				ReadFileData();		/* 读取根目录下armfly.txt的内容 */
				break;

			case '4':
				xprintf("【4 - CreateDir】\r\n");
				CreateDir();		/* 创建目录 */
				break;

			case '5':
				xprintf("【5 - DeleteDirFile】\r\n");
				DeleteDirFile();	/* 删除目录和文件 */
				break;

			case '6':
				xprintf("【6 - TestSpeed】\r\n");
				WriteFileTest();	/* 速度测试 */
				break;
			
			case '7':
				xprintf("【7 - CreateNewDirFileTest】\r\n");
				CreateNewDirFileTest();
				break;
			
			case '8':
				xprintf("【8 - ViewNandCapacity】\r\n");
				ViewNandCapacity();		/* 显示NAND Flash容量和剩余容量 */
				break;
			case '0':
				xprintf("Start Format(Low Level) NAND Flash......\r\n");
				result = FTL_Format();//NAND_Format();
				if(result)
					xprintf("NAND Flash Format Failed\r\n");
				else
					xprintf("NAND Flash Format Ok\r\n");
#if 1				
//f_mkfs格式化为FAT不支持超过256个文件，所以需要在PC上格式化	
				f_mount(FS_NAND, &fs);	
				result = f_mkfs(FS_NAND,0,1024);
				if(result== FR_OK)		
					xprintf("格式化成功 f_mkfs (%d)\r\n",result);
				else
					xprintf("格式化失败 f_mkfs (%d)\r\n",result);			
#endif
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
	xprintf("1 - 显示根目录下的文件列表\r\n");
	xprintf("2 - 创建一个新文件test.txt\r\n");
	xprintf("3 - 读test.txt文件的内容\r\n");
	xprintf("4 - 创建目录\r\n");
	xprintf("5 - 删除文件和目录\r\n");
	xprintf("6 - 读写文件速度测试\r\n");
	xprintf("7 - 测试文件和文件夹创建速度\r\n");
	xprintf("8 - 获取NAND容量\r\n");	
	xprintf("0 - NAND Flash 低级格式化\r\n");
	xprintf("Q - 退出 \r\n");
}

/*
*********************************************************************************************************
*	函 数 名: ViewRootDir
*	功能说明: 显示SD卡根目录下的文件名
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void ViewRootDir(void)
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	DIR DirInf;
	FILINFO FileInf;
	uint32_t cnt = 0;
	char lfname[256];

 	/* 挂载文件系统 */
	result = f_mount(FS_NAND, &fs);	/* Mount a logical drive */
	if (result != FR_OK)
	{
		xprintf("挂载文件系统失败 (%d)\r\n", result);
	}

	DirInf.fs = &fs;
	
	/* 打开根文件夹 */
	result = f_opendir(&DirInf, "2:/"); /* 2: 表示盘符 */
	if (result != FR_OK)
	{
		xprintf("打开根目录失败 (%d)\r\n", result);
		return;
	}

	/* 读取当前文件夹下的文件和目录 */
	FileInf.lfname = lfname;
	FileInf.lfsize = 256;

	xprintf("属性        |  文件大小 | 短文件名 | 长文件名\r\n");
	for (cnt = 0; ;cnt++)
	{
		result = f_readdir(&DirInf,&FileInf); 		/* 读取目录项，索引会自动下移 */
		if (result != FR_OK || FileInf.fname[0] == 0)
		{
			break;
		}

		if (FileInf.fname[0] == '.')
		{
			continue;
		}

		/* 判断是文件还是子目录 */
		if (FileInf.fattrib & AM_DIR)
		{
			xprintf("(0x%02d)目录  ", FileInf.fattrib);
		}
		else
		{
			xprintf("(0x%02d)文件  ", FileInf.fattrib);
		}

		/* 打印文件大小, 最大4G */
		xprintf(" %10d", FileInf.fsize);

		xprintf("  %s |", FileInf.fname);	/* 短文件名 */

		xprintf("  %s\r\n", (char *)FileInf.lfname);	/* 长文件名 */
	}

	/* 卸载文件系统 */
	f_mount(FS_NAND, NULL);
}

/*
*********************************************************************************************************
*	函 数 名: CreateNewFile
*	功能说明: 在SD卡创建一个新文件，文件内容填写“0123456789”
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void CreateNewFile(void)
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;
	uint32_t bw;

 	/* 挂载文件系统 */
	result = f_mount(FS_NAND, &fs);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		xprintf("挂载文件系统失败 (%d)\r\n", result);
	}

	/* 打开根文件夹 */
	result = f_opendir(&DirInf, "2:/"); /* 2: 表示盘符 */
	if (result != FR_OK)
	{
		xprintf("打开根目录失败 (%d)\r\n", result);
		return;
	}

	/* 打开文件 */
	result = f_open(&file, "2:/test.txt", FA_CREATE_ALWAYS | FA_WRITE);

	/* 写一串数据 */
	result = f_write(&file, "FatFS Write Demo \r\n 0123456789 \r\n", 34, &bw);
	if (result == FR_OK)
	{
		xprintf("test.txt 文件写入成功\r\n");
	}
	else
	{
		xprintf("test.txt 文件写入失败\r\n");
	}

	/* 关闭文件*/
	f_close(&file);

	/* 卸载文件系统 */
	f_mount(FS_NAND, NULL);
}

/*
*********************************************************************************************************
*	函 数 名: ReadFileData
*	功能说明: 读取文件armfly.txt前128个字符，并打印到串口
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void ReadFileData(void)
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;
	uint32_t bw;
	char buf[128];

 	/* 挂载文件系统 */
	result = f_mount(FS_NAND, &fs);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		xprintf("挂载文件系统失败(%d)\r\n", result);
	}

	/* 打开根文件夹 */
	result = f_opendir(&DirInf, "2:/"); /* 2: 表示盘符 */
	if (result != FR_OK)
	{
		xprintf("打开根目录失败(%d)\r\n", result);
		return;
	}

	/* 打开文件 */
	result = f_open(&file, "2:/test.txt", FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)
	{
		xprintf("Don't Find File : test.txt\r\n");
		return;
	}

	/* 读取文件 */
	result = f_read(&file, &buf, sizeof(buf) - 1, &bw);
	if (bw > 0)
	{
		buf[bw] = 0;
		xprintf("\r\ntest.txt 文件内容 : \r\n%s\r\n", buf);
	}
	else
	{
		xprintf("\r\ntest.txt 文件内容 : \r\n");
	}

	/* 关闭文件*/
	f_close(&file);

	/* 卸载文件系统 */
	f_mount(FS_NAND, NULL);
}

/*
*********************************************************************************************************
*	函 数 名: CreateDir
*	功能说明: 在SD卡根目录创建Dir1和Dir2目录，在Dir1目录下创建子目录Dir1_1
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void CreateDir(void)
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;

 	/* 挂载文件系统 */
	result = f_mount(FS_NAND, &fs);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		xprintf("挂载文件系统失败 (%d)\r\n", result);
	}

	/* 创建目录/Dir1 */
	result = f_mkdir("2:/Dir1");
	if (result == FR_OK)
	{
		xprintf("f_mkdir Dir1 Ok\r\n");
	}
	else if (result == FR_EXIST)
	{
		xprintf("Dir1 目录已经存在(%d)\r\n", result);
	}
	else
	{
		xprintf("f_mkdir Dir1 失败 (%d)\r\n", result);
		return;
	}

	/* 创建目录/Dir2 */
	result = f_mkdir("2:/Dir2");
	if (result == FR_OK)
	{
		xprintf("f_mkdir Dir2 Ok\r\n");
	}
	else if (result == FR_EXIST)
	{
		xprintf("Dir2 目录已经存在(%d)\r\n", result);
	}
	else
	{
		xprintf("f_mkdir Dir2 失败 (%d)\r\n", result);
		return;
	}

	/* 创建子目录 /Dir1/Dir1_1	   注意：创建子目录Dir1_1时，必须先创建好Dir1 */
	result = f_mkdir("2:/Dir1/Dir1_1"); /* */
	if (result == FR_OK)
	{
		xprintf("f_mkdir Dir1_1 成功\r\n");
	}
	else if (result == FR_EXIST)
	{
		xprintf("Dir1_1 目录已经存在 (%d)\r\n", result);
	}
	else
	{
		xprintf("f_mkdir Dir1_1 失败 (%d)\r\n", result);
		return;
	}

	/* 卸载文件系统 */
	f_mount(FS_NAND, NULL);
}

/*
*********************************************************************************************************
*	函 数 名: DeleteDirFile
*	功能说明: 删除SD卡根目录下的 armfly.txt 文件和 Dir1，Dir2 目录
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DeleteDirFile(void)
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	char FileName[13];
	uint8_t i;

 	/* 挂载文件系统 */
	result = f_mount(FS_NAND, &fs);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		xprintf("挂载文件系统失败 (%d)\r\n", result);
	}

	#if 0
	/* 打开根文件夹 */
	result = f_opendir(&DirInf, "2:/"); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK)
	{
		xprintf("打开根目录失败(%d)\r\n", result);
		return;
	}
	#endif

	/* 删除目录/Dir1 【因为还存在目录非空（存在子目录)，所以这次删除会失败】*/
	result = f_unlink("2:/Dir1");
	if (result == FR_OK)
	{
		xprintf("删除目录Dir1成功\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		xprintf("没有发现文件或目录 :%s\r\n", "/Dir1");
	}
	else
	{
		xprintf("删除Dir1失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
	}

	/* 先删除目录/Dir1/Dir1_1 */
	result = f_unlink("2:/Dir1/Dir1_1");
	if (result == FR_OK)
	{
		xprintf("删除子目录/Dir1/Dir1_1成功\r\n");
	}
	else if ((result == FR_NO_FILE) || (result == FR_NO_PATH))
	{
		xprintf("没有发现文件或目录 :%s\r\n", "/Dir1/Dir1_1");
	}
	else
	{
		xprintf("删除子目录/Dir1/Dir1_1失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
	}

	/* 先删除目录/Dir1 */
	result = f_unlink("2:/Dir1");
	if (result == FR_OK)
	{
		xprintf("删除目录Dir1成功\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		xprintf("没有发现文件或目录 :%s\r\n", "/Dir1");
	}
	else
	{
		xprintf("删除Dir1失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
	}

	/* 删除目录/Dir2 */
	result = f_unlink("2:/Dir2");
	if (result == FR_OK)
	{
		xprintf("删除目录 Dir2 成功\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		xprintf("没有发现文件或目录 :%s\r\n", "/Dir2");
	}
	else
	{
		xprintf("删除Dir2 失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
	}

	/* 删除文件 armfly.txt */
	result = f_unlink("2:/armfly.txt");
	if (result == FR_OK)
	{
		xprintf("删除文件 armfly.txt 成功\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		xprintf("没有发现文件或目录 :%s\r\n", "armfly.txt");
	}
	else
	{
		xprintf("删除armfly.txt失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
	}

	/* 删除文件 speed1.txt */
	for (i = 0; i < 20; i++)
	{
		sprintf(FileName, "2:/Speed%02d.txt", i);		/* 每写1次，序号递增 */
		result = f_unlink(FileName);
		if (result == FR_OK)
		{
			xprintf("删除文件%s成功\r\n", FileName);
		}
		else if (result == FR_NO_FILE)
		{
			xprintf("没有发现文件:%s\r\n", FileName);
		}
		else
		{
			xprintf("删除%s文件失败(错误代码 = %d) 文件只读或目录非空\r\n", FileName, result);
		}
	}

	/* 卸载文件系统 */
	f_mount(FS_NAND, NULL);
}

/*
*********************************************************************************************************
*	函 数 名: WriteFileTest
*	功能说明: 测试文件读写速度
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void WriteFileTest(void)
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;
	uint32_t bw;
	uint32_t i,k;
	uint32_t runtime1,runtime2,timelen;
	uint8_t err = 0;
	char TestFileName[13];
	static uint8_t s_ucTestSn = 0;

	for (i = 0; i < sizeof(g_TestBuf); i++)
	{
		g_TestBuf[i] = (i / 512) + '0';
	}

  	/* 挂载文件系统 */
	result = f_mount(FS_NAND, &fs);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		xprintf("挂载文件系统失败 (%d)\r\n", result);
	}

	/* 打开根文件夹 */
	result = f_opendir(&DirInf, "2:/"); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK)
	{
		xprintf("打开根目录失败 (%d)\r\n", result);
		return;
	}

	/* 打开文件 */
	sprintf(TestFileName, "2:/Speed%02d.txt", s_ucTestSn++);		/* 每写1次，序号递增 */
	result = f_open(&file, TestFileName, FA_CREATE_ALWAYS | FA_WRITE);

	/* 写一串数据 */
	xprintf("开始写文件%s %dKB ...\r\n", TestFileName, TEST_FILE_LEN / 1024);
	runtime1 = GetSystemTimer();//bsp_GetRunTime();	/* 读取系统运行时间 */
	for (i = 0; i < TEST_FILE_LEN / BUF_SIZE; i++)
	{
		result = f_write(&file, g_TestBuf, sizeof(g_TestBuf), &bw);
		if (result == FR_OK)
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
	runtime2 = GetSystemTimer();//bsp_GetRunTime();	/* 读取系统运行时间 */

	if (err == 0)
	{
		timelen = (runtime2 - runtime1);
		xprintf("\r\n  写耗时 : %dms   平均写速度 : %dB/S (%dKB/S)\r\n",
			timelen,
			(TEST_FILE_LEN * 1000) / timelen,
			((TEST_FILE_LEN / 1024) * 1000) / timelen);
	}

	f_close(&file);		/* 关闭文件*/


	/* 开始读文件测试 */
	result = f_open(&file, TestFileName, FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)
	{
		xprintf("没有找到文件: %s\r\n", TestFileName);
		return;
	}

	xprintf("开始读文件 %dKB ...\r\n", TEST_FILE_LEN / 1024);
	runtime1 = GetSystemTimer();//bsp_GetRunTime();	/* 读取系统运行时间 */
	for (i = 0; i < TEST_FILE_LEN / BUF_SIZE; i++)
	{
		result = f_read(&file, g_TestBuf, sizeof(g_TestBuf), &bw);
		if (result == FR_OK)
		{
			if (((i + 1) % 8) == 0)
			{
				xprintf(".");
			}

			/* 比较写入的数据是否正确，此语句会导致读卡速度结果降低到 3.5MBytes/S */
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
	runtime2 = GetSystemTimer();//bsp_GetRunTime();	/* 读取系统运行时间 */

	if (err == 0)
	{
		timelen = (runtime2 - runtime1);
		xprintf("\r\n  读耗时 : %dms   平均读速度 : %dB/S (%dKB/S)\r\n", timelen,
			(TEST_FILE_LEN * 1000) / timelen, ((TEST_FILE_LEN / 1024) * 1000) / timelen);
	}

	/* 关闭文件*/
	f_close(&file);

	/* 卸载文件系统 */
	f_mount(FS_NAND, NULL);
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
	const uint8_t WriteText[] = {"STM32 Demo application"};
	WORD bytesWritten, bytesToWrite;
	FRESULT res;
	FIL file;
  FILINFO finfo;
  DIR dirs;
	FATFS fs;
  char path[100]={""};  
	char TestFileName[100];
	char DirName1[50],DirName2[50],DirName3[50],DirName4[50];
	uint32_t rng_data,Filenum=0;
	
	rng_data = GetRandomNumber();
	
	res = f_mount(FS_NAND, &fs);
	if(res == FR_OK)
		xprintf("\r\n %d:f_mount OK", res);
	else
		xprintf("\r\n %d:f_mount err", res);
	
	dirs.fs = &fs;
	
	if (f_opendir(&dirs, "2:/") == FR_OK)        	//读取该磁盘的根目录   
	{
		while(Filenum++<20000)
		{			
			sprintf(DirName1, "2:dir%d-%02d", (uint8_t)rng_data,Filenum);
			sprintf(DirName2, "2:dir%d-%02d/dir2-%02d", (uint8_t)rng_data,Filenum,Filenum);
			sprintf(DirName3, "2:dir%d-%02d/dir2-%02d/dir3-%02d", (uint8_t)rng_data,Filenum,Filenum,Filenum);
			sprintf(DirName4, "2:dir%d-%02d/dir2-%02d/dir3-%02d/dir4-%02d", (uint8_t)rng_data,Filenum,Filenum,Filenum,Filenum);
			sprintf(TestFileName, "2:/dir%d-%02d/dir2-%02d/dir3-%02d/dir4-%02d/test%02d.txt", (uint8_t)rng_data,Filenum,Filenum,Filenum,Filenum,Filenum);      /* 每写1次，序号递增 */
			
			xprintf("Create dir...\r\n");
			DisplayTime(0);	f_mkdir(DirName1);DisplayTime(1);
			DisplayTime(0);	f_mkdir(DirName2);DisplayTime(1);
			DisplayTime(0);	f_mkdir(DirName3);DisplayTime(1);
			DisplayTime(0);	res = f_mkdir(DirName4);DisplayTime(1);

			if(res != FR_OK)
			{
					xprintf("Fail to create dir, Err Code:%d\n",res);
					break;				
			}
			xprintf("Open file\r\n");				
			DisplayTime(0);
			res = f_open(&file, TestFileName, FA_CREATE_ALWAYS | FA_WRITE);		
			DisplayTime(1);
			
			if(res == FR_OK)
			{ 
				//xprintf("Write to file\r\n");
				bytesToWrite = sizeof(WriteText);
				//DisplayTime(0);
				res= f_write (&file, WriteText, bytesToWrite, (void *)&bytesWritten);				
				//DisplayTime(1);
				if((bytesWritten == 0) || (res != FR_OK)) 
				{
					xprintf("\r\n%s CANNOT be writen", TestFileName);
					break;
				}
				else
				{
					xprintf("\r\n%s file created", TestFileName);
				}	
			}
			else
			{
				xprintf("\r\n f_open:%d", res);
				break;
			}
			
			res = f_close(&file);							
			if(res != FR_OK)
			{
				xprintf("\r\n f_close:%d", res);
				break;
			}		
		}
	}	
	else
	{
		xprintf("\n\r err: f_opendir"); 
	}		
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
	DWORD free_clust, tot_sect, fre_sect,total, free;
	FATFS fs;
	FATFS *pfs;
	FRESULT res;
	
	res = f_mount(FS_NAND, &fs);
	if(res == FR_OK)
		xprintf("f_mount OK (%d)\r\n ", res);
	else
		xprintf("f_mount err (%d)\r\n ", res);
	
	//读取SD卡容量及剩余容量
	DisplayTime(0);
	if(f_getfree("2:/", &free_clust, &pfs)==FR_OK)
	{
		tot_sect=(pfs->n_fatent-2)*pfs->csize; 
		fre_sect=free_clust*pfs->csize;   

		total=tot_sect>>1; 
		free=fre_sect>>1; 
		xprintf("\r\n Capacity is ..%d M /%d M \r\n", free/1024, total/1024);
	}
	else
	{
		xputs("\r\n f_getfree err");
	}	
	DisplayTime(1);	
}

#endif //(TWO_SDCARDS)	

