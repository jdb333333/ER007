
#if defined(TWO_SDCARDS)	

#include "stm32f2xx.h"
#include "typedef.h"
#include "ff.h"		
#include "demo_nand_fatfs.h"
#include "monitor.h"
#include "SysTick.h"

/* ���ڲ��Զ�д�ٶ� */
#define TEST_FILE_LEN			(2*1024*1024)	/* ���ڲ��Ե��ļ����� */
#define BUF_SIZE				(4*1024)		/* ÿ�ζ�дSD����������ݳ��� */
extern uint8_t g_TestBuf[BUF_SIZE];

/* �������ļ��ڵ��õĺ������� */
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
*	�� �� ��: DemoFatFS
*	����˵��: FatFS�ļ�ϵͳ��ʾ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DemoFatFS_Nand(void)
{
	char *ptr;
	FATFS fs;	
	FRESULT result;

	/* ��ӡ�����б��û�����ͨ�����ڲ���ָ�� */
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
				xprintf("��1 - ViewRootDir��\r\n");
				ViewRootDir();		/* ��ʾ��Ŀ¼�µ��ļ��� */
				break;

			case '2':
				xprintf("��2 - CreateNewFile��\r\n");
				CreateNewFile();		/* ����һ�����ļ�,д��һ���ַ��� */
				break;

			case '3':
				xprintf("��3 - ReadFileData��\r\n");
				ReadFileData();		/* ��ȡ��Ŀ¼��armfly.txt������ */
				break;

			case '4':
				xprintf("��4 - CreateDir��\r\n");
				CreateDir();		/* ����Ŀ¼ */
				break;

			case '5':
				xprintf("��5 - DeleteDirFile��\r\n");
				DeleteDirFile();	/* ɾ��Ŀ¼���ļ� */
				break;

			case '6':
				xprintf("��6 - TestSpeed��\r\n");
				WriteFileTest();	/* �ٶȲ��� */
				break;
			
			case '7':
				xprintf("��7 - CreateNewDirFileTest��\r\n");
				CreateNewDirFileTest();
				break;
			
			case '8':
				xprintf("��8 - ViewNandCapacity��\r\n");
				ViewNandCapacity();		/* ��ʾNAND Flash������ʣ������ */
				break;
			case '0':
				xprintf("Start Format(Low Level) NAND Flash......\r\n");
				result = FTL_Format();//NAND_Format();
				if(result)
					xprintf("NAND Flash Format Failed\r\n");
				else
					xprintf("NAND Flash Format Ok\r\n");
#if 1				
//f_mkfs��ʽ��ΪFAT��֧�ֳ���256���ļ���������Ҫ��PC�ϸ�ʽ��	
				f_mount(FS_NAND, &fs);	
				result = f_mkfs(FS_NAND,0,1024);
				if(result== FR_OK)		
					xprintf("��ʽ���ɹ� f_mkfs (%d)\r\n",result);
				else
					xprintf("��ʽ��ʧ�� f_mkfs (%d)\r\n",result);			
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
	xprintf("1 - ��ʾ��Ŀ¼�µ��ļ��б�\r\n");
	xprintf("2 - ����һ�����ļ�test.txt\r\n");
	xprintf("3 - ��test.txt�ļ�������\r\n");
	xprintf("4 - ����Ŀ¼\r\n");
	xprintf("5 - ɾ���ļ���Ŀ¼\r\n");
	xprintf("6 - ��д�ļ��ٶȲ���\r\n");
	xprintf("7 - �����ļ����ļ��д����ٶ�\r\n");
	xprintf("8 - ��ȡNAND����\r\n");	
	xprintf("0 - NAND Flash �ͼ���ʽ��\r\n");
	xprintf("Q - �˳� \r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: ViewRootDir
*	����˵��: ��ʾSD����Ŀ¼�µ��ļ���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void ViewRootDir(void)
{
	/* ������ʹ�õľֲ�����ռ�ý϶࣬���޸������ļ�����֤��ջ�ռ乻�� */
	FRESULT result;
	FATFS fs;
	DIR DirInf;
	FILINFO FileInf;
	uint32_t cnt = 0;
	char lfname[256];

 	/* �����ļ�ϵͳ */
	result = f_mount(FS_NAND, &fs);	/* Mount a logical drive */
	if (result != FR_OK)
	{
		xprintf("�����ļ�ϵͳʧ�� (%d)\r\n", result);
	}

	DirInf.fs = &fs;
	
	/* �򿪸��ļ��� */
	result = f_opendir(&DirInf, "2:/"); /* 2: ��ʾ�̷� */
	if (result != FR_OK)
	{
		xprintf("�򿪸�Ŀ¼ʧ�� (%d)\r\n", result);
		return;
	}

	/* ��ȡ��ǰ�ļ����µ��ļ���Ŀ¼ */
	FileInf.lfname = lfname;
	FileInf.lfsize = 256;

	xprintf("����        |  �ļ���С | ���ļ��� | ���ļ���\r\n");
	for (cnt = 0; ;cnt++)
	{
		result = f_readdir(&DirInf,&FileInf); 		/* ��ȡĿ¼��������Զ����� */
		if (result != FR_OK || FileInf.fname[0] == 0)
		{
			break;
		}

		if (FileInf.fname[0] == '.')
		{
			continue;
		}

		/* �ж����ļ�������Ŀ¼ */
		if (FileInf.fattrib & AM_DIR)
		{
			xprintf("(0x%02d)Ŀ¼  ", FileInf.fattrib);
		}
		else
		{
			xprintf("(0x%02d)�ļ�  ", FileInf.fattrib);
		}

		/* ��ӡ�ļ���С, ���4G */
		xprintf(" %10d", FileInf.fsize);

		xprintf("  %s |", FileInf.fname);	/* ���ļ��� */

		xprintf("  %s\r\n", (char *)FileInf.lfname);	/* ���ļ��� */
	}

	/* ж���ļ�ϵͳ */
	f_mount(FS_NAND, NULL);
}

/*
*********************************************************************************************************
*	�� �� ��: CreateNewFile
*	����˵��: ��SD������һ�����ļ����ļ�������д��0123456789��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void CreateNewFile(void)
{
	/* ������ʹ�õľֲ�����ռ�ý϶࣬���޸������ļ�����֤��ջ�ռ乻�� */
	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;
	uint32_t bw;

 	/* �����ļ�ϵͳ */
	result = f_mount(FS_NAND, &fs);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		xprintf("�����ļ�ϵͳʧ�� (%d)\r\n", result);
	}

	/* �򿪸��ļ��� */
	result = f_opendir(&DirInf, "2:/"); /* 2: ��ʾ�̷� */
	if (result != FR_OK)
	{
		xprintf("�򿪸�Ŀ¼ʧ�� (%d)\r\n", result);
		return;
	}

	/* ���ļ� */
	result = f_open(&file, "2:/test.txt", FA_CREATE_ALWAYS | FA_WRITE);

	/* дһ������ */
	result = f_write(&file, "FatFS Write Demo \r\n 0123456789 \r\n", 34, &bw);
	if (result == FR_OK)
	{
		xprintf("test.txt �ļ�д��ɹ�\r\n");
	}
	else
	{
		xprintf("test.txt �ļ�д��ʧ��\r\n");
	}

	/* �ر��ļ�*/
	f_close(&file);

	/* ж���ļ�ϵͳ */
	f_mount(FS_NAND, NULL);
}

/*
*********************************************************************************************************
*	�� �� ��: ReadFileData
*	����˵��: ��ȡ�ļ�armfly.txtǰ128���ַ�������ӡ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void ReadFileData(void)
{
	/* ������ʹ�õľֲ�����ռ�ý϶࣬���޸������ļ�����֤��ջ�ռ乻�� */
	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;
	uint32_t bw;
	char buf[128];

 	/* �����ļ�ϵͳ */
	result = f_mount(FS_NAND, &fs);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		xprintf("�����ļ�ϵͳʧ��(%d)\r\n", result);
	}

	/* �򿪸��ļ��� */
	result = f_opendir(&DirInf, "2:/"); /* 2: ��ʾ�̷� */
	if (result != FR_OK)
	{
		xprintf("�򿪸�Ŀ¼ʧ��(%d)\r\n", result);
		return;
	}

	/* ���ļ� */
	result = f_open(&file, "2:/test.txt", FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)
	{
		xprintf("Don't Find File : test.txt\r\n");
		return;
	}

	/* ��ȡ�ļ� */
	result = f_read(&file, &buf, sizeof(buf) - 1, &bw);
	if (bw > 0)
	{
		buf[bw] = 0;
		xprintf("\r\ntest.txt �ļ����� : \r\n%s\r\n", buf);
	}
	else
	{
		xprintf("\r\ntest.txt �ļ����� : \r\n");
	}

	/* �ر��ļ�*/
	f_close(&file);

	/* ж���ļ�ϵͳ */
	f_mount(FS_NAND, NULL);
}

/*
*********************************************************************************************************
*	�� �� ��: CreateDir
*	����˵��: ��SD����Ŀ¼����Dir1��Dir2Ŀ¼����Dir1Ŀ¼�´�����Ŀ¼Dir1_1
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void CreateDir(void)
{
	/* ������ʹ�õľֲ�����ռ�ý϶࣬���޸������ļ�����֤��ջ�ռ乻�� */
	FRESULT result;
	FATFS fs;

 	/* �����ļ�ϵͳ */
	result = f_mount(FS_NAND, &fs);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		xprintf("�����ļ�ϵͳʧ�� (%d)\r\n", result);
	}

	/* ����Ŀ¼/Dir1 */
	result = f_mkdir("2:/Dir1");
	if (result == FR_OK)
	{
		xprintf("f_mkdir Dir1 Ok\r\n");
	}
	else if (result == FR_EXIST)
	{
		xprintf("Dir1 Ŀ¼�Ѿ�����(%d)\r\n", result);
	}
	else
	{
		xprintf("f_mkdir Dir1 ʧ�� (%d)\r\n", result);
		return;
	}

	/* ����Ŀ¼/Dir2 */
	result = f_mkdir("2:/Dir2");
	if (result == FR_OK)
	{
		xprintf("f_mkdir Dir2 Ok\r\n");
	}
	else if (result == FR_EXIST)
	{
		xprintf("Dir2 Ŀ¼�Ѿ�����(%d)\r\n", result);
	}
	else
	{
		xprintf("f_mkdir Dir2 ʧ�� (%d)\r\n", result);
		return;
	}

	/* ������Ŀ¼ /Dir1/Dir1_1	   ע�⣺������Ŀ¼Dir1_1ʱ�������ȴ�����Dir1 */
	result = f_mkdir("2:/Dir1/Dir1_1"); /* */
	if (result == FR_OK)
	{
		xprintf("f_mkdir Dir1_1 �ɹ�\r\n");
	}
	else if (result == FR_EXIST)
	{
		xprintf("Dir1_1 Ŀ¼�Ѿ����� (%d)\r\n", result);
	}
	else
	{
		xprintf("f_mkdir Dir1_1 ʧ�� (%d)\r\n", result);
		return;
	}

	/* ж���ļ�ϵͳ */
	f_mount(FS_NAND, NULL);
}

/*
*********************************************************************************************************
*	�� �� ��: DeleteDirFile
*	����˵��: ɾ��SD����Ŀ¼�µ� armfly.txt �ļ��� Dir1��Dir2 Ŀ¼
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DeleteDirFile(void)
{
	/* ������ʹ�õľֲ�����ռ�ý϶࣬���޸������ļ�����֤��ջ�ռ乻�� */
	FRESULT result;
	FATFS fs;
	char FileName[13];
	uint8_t i;

 	/* �����ļ�ϵͳ */
	result = f_mount(FS_NAND, &fs);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		xprintf("�����ļ�ϵͳʧ�� (%d)\r\n", result);
	}

	#if 0
	/* �򿪸��ļ��� */
	result = f_opendir(&DirInf, "2:/"); /* ���������������ӵ�ǰĿ¼��ʼ */
	if (result != FR_OK)
	{
		xprintf("�򿪸�Ŀ¼ʧ��(%d)\r\n", result);
		return;
	}
	#endif

	/* ɾ��Ŀ¼/Dir1 ����Ϊ������Ŀ¼�ǿգ�������Ŀ¼)���������ɾ����ʧ�ܡ�*/
	result = f_unlink("2:/Dir1");
	if (result == FR_OK)
	{
		xprintf("ɾ��Ŀ¼Dir1�ɹ�\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		xprintf("û�з����ļ���Ŀ¼ :%s\r\n", "/Dir1");
	}
	else
	{
		xprintf("ɾ��Dir1ʧ��(������� = %d) �ļ�ֻ����Ŀ¼�ǿ�\r\n", result);
	}

	/* ��ɾ��Ŀ¼/Dir1/Dir1_1 */
	result = f_unlink("2:/Dir1/Dir1_1");
	if (result == FR_OK)
	{
		xprintf("ɾ����Ŀ¼/Dir1/Dir1_1�ɹ�\r\n");
	}
	else if ((result == FR_NO_FILE) || (result == FR_NO_PATH))
	{
		xprintf("û�з����ļ���Ŀ¼ :%s\r\n", "/Dir1/Dir1_1");
	}
	else
	{
		xprintf("ɾ����Ŀ¼/Dir1/Dir1_1ʧ��(������� = %d) �ļ�ֻ����Ŀ¼�ǿ�\r\n", result);
	}

	/* ��ɾ��Ŀ¼/Dir1 */
	result = f_unlink("2:/Dir1");
	if (result == FR_OK)
	{
		xprintf("ɾ��Ŀ¼Dir1�ɹ�\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		xprintf("û�з����ļ���Ŀ¼ :%s\r\n", "/Dir1");
	}
	else
	{
		xprintf("ɾ��Dir1ʧ��(������� = %d) �ļ�ֻ����Ŀ¼�ǿ�\r\n", result);
	}

	/* ɾ��Ŀ¼/Dir2 */
	result = f_unlink("2:/Dir2");
	if (result == FR_OK)
	{
		xprintf("ɾ��Ŀ¼ Dir2 �ɹ�\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		xprintf("û�з����ļ���Ŀ¼ :%s\r\n", "/Dir2");
	}
	else
	{
		xprintf("ɾ��Dir2 ʧ��(������� = %d) �ļ�ֻ����Ŀ¼�ǿ�\r\n", result);
	}

	/* ɾ���ļ� armfly.txt */
	result = f_unlink("2:/armfly.txt");
	if (result == FR_OK)
	{
		xprintf("ɾ���ļ� armfly.txt �ɹ�\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		xprintf("û�з����ļ���Ŀ¼ :%s\r\n", "armfly.txt");
	}
	else
	{
		xprintf("ɾ��armfly.txtʧ��(������� = %d) �ļ�ֻ����Ŀ¼�ǿ�\r\n", result);
	}

	/* ɾ���ļ� speed1.txt */
	for (i = 0; i < 20; i++)
	{
		sprintf(FileName, "2:/Speed%02d.txt", i);		/* ÿд1�Σ���ŵ��� */
		result = f_unlink(FileName);
		if (result == FR_OK)
		{
			xprintf("ɾ���ļ�%s�ɹ�\r\n", FileName);
		}
		else if (result == FR_NO_FILE)
		{
			xprintf("û�з����ļ�:%s\r\n", FileName);
		}
		else
		{
			xprintf("ɾ��%s�ļ�ʧ��(������� = %d) �ļ�ֻ����Ŀ¼�ǿ�\r\n", FileName, result);
		}
	}

	/* ж���ļ�ϵͳ */
	f_mount(FS_NAND, NULL);
}

/*
*********************************************************************************************************
*	�� �� ��: WriteFileTest
*	����˵��: �����ļ���д�ٶ�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void WriteFileTest(void)
{
	/* ������ʹ�õľֲ�����ռ�ý϶࣬���޸������ļ�����֤��ջ�ռ乻�� */
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

  	/* �����ļ�ϵͳ */
	result = f_mount(FS_NAND, &fs);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		xprintf("�����ļ�ϵͳʧ�� (%d)\r\n", result);
	}

	/* �򿪸��ļ��� */
	result = f_opendir(&DirInf, "2:/"); /* ���������������ӵ�ǰĿ¼��ʼ */
	if (result != FR_OK)
	{
		xprintf("�򿪸�Ŀ¼ʧ�� (%d)\r\n", result);
		return;
	}

	/* ���ļ� */
	sprintf(TestFileName, "2:/Speed%02d.txt", s_ucTestSn++);		/* ÿд1�Σ���ŵ��� */
	result = f_open(&file, TestFileName, FA_CREATE_ALWAYS | FA_WRITE);

	/* дһ������ */
	xprintf("��ʼд�ļ�%s %dKB ...\r\n", TestFileName, TEST_FILE_LEN / 1024);
	runtime1 = GetSystemTimer();//bsp_GetRunTime();	/* ��ȡϵͳ����ʱ�� */
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
			xprintf("%s�ļ�дʧ��\r\n", TestFileName);
			break;
		}
	}
	runtime2 = GetSystemTimer();//bsp_GetRunTime();	/* ��ȡϵͳ����ʱ�� */

	if (err == 0)
	{
		timelen = (runtime2 - runtime1);
		xprintf("\r\n  д��ʱ : %dms   ƽ��д�ٶ� : %dB/S (%dKB/S)\r\n",
			timelen,
			(TEST_FILE_LEN * 1000) / timelen,
			((TEST_FILE_LEN / 1024) * 1000) / timelen);
	}

	f_close(&file);		/* �ر��ļ�*/


	/* ��ʼ���ļ����� */
	result = f_open(&file, TestFileName, FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)
	{
		xprintf("û���ҵ��ļ�: %s\r\n", TestFileName);
		return;
	}

	xprintf("��ʼ���ļ� %dKB ...\r\n", TEST_FILE_LEN / 1024);
	runtime1 = GetSystemTimer();//bsp_GetRunTime();	/* ��ȡϵͳ����ʱ�� */
	for (i = 0; i < TEST_FILE_LEN / BUF_SIZE; i++)
	{
		result = f_read(&file, g_TestBuf, sizeof(g_TestBuf), &bw);
		if (result == FR_OK)
		{
			if (((i + 1) % 8) == 0)
			{
				xprintf(".");
			}

			/* �Ƚ�д��������Ƿ���ȷ�������ᵼ�¶����ٶȽ�����͵� 3.5MBytes/S */
			for (k = 0; k < sizeof(g_TestBuf); k++)
			{
				if (g_TestBuf[k] != (k / 512) + '0')
				{
				  	err = 1;
					xprintf("Speed1.txt �ļ����ɹ����������ݳ���\r\n");
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
			xprintf("Speed1.txt �ļ���ʧ��\r\n");
			break;
		}
	}
	runtime2 = GetSystemTimer();//bsp_GetRunTime();	/* ��ȡϵͳ����ʱ�� */

	if (err == 0)
	{
		timelen = (runtime2 - runtime1);
		xprintf("\r\n  ����ʱ : %dms   ƽ�����ٶ� : %dB/S (%dKB/S)\r\n", timelen,
			(TEST_FILE_LEN * 1000) / timelen, ((TEST_FILE_LEN / 1024) * 1000) / timelen);
	}

	/* �ر��ļ�*/
	f_close(&file);

	/* ж���ļ�ϵͳ */
	f_mount(FS_NAND, NULL);
}

/*
*********************************************************************************************************
*	�� �� ��: CreateNewDirFileTest
*	����˵��: ��NAND Flash��������text�ı����ֱ�ʹ��fwrite��fprintf��fputsд���ݡ�
*	��    ��: ��
*	�� �� ֵ: ��
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
	
	if (f_opendir(&dirs, "2:/") == FR_OK)        	//��ȡ�ô��̵ĸ�Ŀ¼   
	{
		while(Filenum++<20000)
		{			
			sprintf(DirName1, "2:dir%d-%02d", (uint8_t)rng_data,Filenum);
			sprintf(DirName2, "2:dir%d-%02d/dir2-%02d", (uint8_t)rng_data,Filenum,Filenum);
			sprintf(DirName3, "2:dir%d-%02d/dir2-%02d/dir3-%02d", (uint8_t)rng_data,Filenum,Filenum,Filenum);
			sprintf(DirName4, "2:dir%d-%02d/dir2-%02d/dir3-%02d/dir4-%02d", (uint8_t)rng_data,Filenum,Filenum,Filenum,Filenum);
			sprintf(TestFileName, "2:/dir%d-%02d/dir2-%02d/dir3-%02d/dir4-%02d/test%02d.txt", (uint8_t)rng_data,Filenum,Filenum,Filenum,Filenum,Filenum);      /* ÿд1�Σ���ŵ��� */
			
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
*	�� �� ��: ViewNandCapacity
*	����˵��: NAND Flash�ͼ���ʽ�������أ�ж�ؼ���������ʾ
*	��    ��: ��
*	�� �� ֵ: ��
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
	
	//��ȡSD��������ʣ������
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

