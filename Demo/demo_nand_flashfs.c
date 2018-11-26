/*
*********************************************************************************************************
*
*	ģ������ : FlashFS�ļ�ϵͳ��ʾģ�顣
*	�ļ����� : demo_nandflash_flashfs.c
*	��    �� : V1.0
*	˵    �� : ��������ֲFlashFS�ļ�ϵͳ.
*   ע������ : 1. FlashFS��֧�ֲ��������ļ������ļ�����֧����ʹ�ö��ļ�����FSN_CM3.lib�г�8.3��ʽ��������
*                 ����ʹ��fopen�������������ļ������ļ�ʱ�����
*              2. FlashFSֻ��MDK�Ŀ⣬û��IAR��GCC��
*
*	�޸ļ�¼ :
*		�汾��    ����         ����            ˵��
*       V1.0    2015-09-19   Eric2013    1. RL-FlashFS���ļ����汾V4.74
*
*	Copyright (C), 2015-2020, ���������� www.armfly.com
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



/* ���ڲ��Զ�д�ٶ� */
#define TEST_FILE_LEN			(2*1024*1024)	/* ���ڲ��Ե��ļ����� */
#define BUF_SIZE				(4*1024)		/* ÿ�ζ�дSD����������ݳ��� */
uint8_t g_TestBuf[BUF_SIZE];

/* �������ļ��ڵ��õĺ������� */
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

/* FlashFs API�ķ���ֵ */
static const char * ReVal_Table[]= 
{
	"0���ɹ�",				                        
	"1��IO����I/O������ʼ��ʧ�ܣ�����û�д洢�豸�������豸��ʼ��ʧ��",
	"2������󣬹���ʧ�ܣ�����FAT�ļ�ϵͳ��ζ����Ч��MBR��������¼���߷�FAT��ʽ",
	"3��FAT��־��ʼ��ʧ�ܣ�FAT��ʼ���ɹ��ˣ�������־��ʼ��ʧ��",
};

/*
*********************************************************************************************************
*	�� �� ��: DemoFlashFs
*	����˵��: FlashFs�ļ�ϵͳ��ʾ������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DemoFlashFs(void)
{
	char *ptr;

	/* ��ӡ�����б��û�����ͨ�����ڲ���ָ�� */
	DispMenu();
	
	while(1)
	{
		ptr = ConsoleLine;
		ConsoleLine[0]=0;
		get_line(ptr, sizeof(ConsoleLine));
		switch (*ptr++)
		{
			case '0':
				xprintf("��0 - ViewNandCapacity��\r\n");
				ViewNandCapacity();		/* �ͼ���ʽ������ʾNAND Flash������ʣ������ */
				break;

			case '1':
				xprintf("��1 - ViewRootDir��\r\n");
				ViewRootDir();		     /* ��ʾNANDĿ¼�µ��ļ��� */
				break;
			
			case '2':
				xprintf("��2 - CreateNewFile��\r\n");
				CreateNewFile();	    /* ��������text�ı���ʹ�ò�ͬ����д������ */
				break;
			
			case '3':
				xprintf("��3 - ReadFileData��\r\n");
				ReadFileData();	        /* ʹ��������ͬ������ȡ�ı�  */
				break;

			case '4':
				xprintf("��4 - SeekFileData��\r\n");
				SeekFileData();	        /* ����һ��text�ı�����ָ��λ�ö�����ж�д����  */
				break;
			
			case '5':
				xprintf("��5 - DeleteDirFile��\r\n");
				DeleteDirFile();	    /* ɾ���ļ��к��ļ�  */
				break;
			
			case '6':
				xprintf("��6 - WriteFileTest��\r\n");
				WriteFileTest();	    /* �����ļ���д�ٶ�  */
				break;

			case '7':
				xprintf("��7 - WriteCSVFile��\r\n");
				WriteCSVFile();	        /* д���ݵ�CSV�ļ���  */
				break;
			
			case '8':
				xprintf("��8 - ReadCSVFileData��\r\n");
				ReadCSVFileData();	    /* ��CSV�ļ��ж�����  */
				break;
			
			case '9':
				xprintf("��9 - CreateNewDirFileTest��\r\n");
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
*	�� �� ��: DispMenu
*	����˵��: ��ʾ������ʾ�˵�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispMenu(void)
{
	xprintf("\r\n------------------------------------------------\r\n");
	xprintf("��ѡ�񴮿ڲ���������Լ��̴�ӡ���ּ���:\r\n");
	xprintf("�״�ʹ�ã���ص��ô�������0������NAND Flash�ĵͼ���ʽ�����ļ�ϵͳ��ʽ��\r\n");	
	xprintf("0 - �ͼ���ʽ������ʾNAND Flash������ʣ������\r\n");
	xprintf("1 - ��ʾNAND Flash��Ŀ¼�µ��ļ�\r\n");
	xprintf("2 - ��������text�ı���ʹ�ò�ͬ����д������\r\n");
	xprintf("3 - ʹ��������ͬ������ȡ�ı�\r\n");
	xprintf("4 - ����һ��text�ı���ָ��һ��λ�ö�����ж�д����\r\n");
	xprintf("5 - ɾ���ļ��к��ļ�\r\n");
	xprintf("6 - �����ļ���д�ٶ�\r\n");
	xprintf("7 - д���ݵ�CSV�ļ���\r\n");
	xprintf("8 - ��CSV�ļ��ж�����\r\n");
	xprintf("9 - �����ļ����ļ��д����ٶ�\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: DotFormat
*	����˵��: �����ݹ淶����ʾ�������û��鿴
*             ����
*             2345678   ---->  2.345.678
*             334426783 ---->  334.426.783
*             ��������Ϊ��λ������ʾ
*	��    ��: _ullVal   ��Ҫ�淶��ʾ����ֵ
*             _sp       �淶��ʾ�����ݴ洢��buf��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DotFormat(uint64_t _ullVal, char *_sp) 
{
	/* ��ֵ���ڵ���10^9 */
	if (_ullVal >= (U64)1e9) 
	{
		_sp += sprintf (_sp, "%d.", (uint32_t)(_ullVal / (uint64_t)1e9));
		_ullVal %= (uint64_t)1e9;
		_sp += sprintf (_sp, "%03d.", (uint32_t)(_ullVal / (uint64_t)1e6));
		_ullVal %= (uint64_t)1e6;
		sprintf (_sp, "%03d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
		return;
	}
	
	/* ��ֵ���ڵ���10^6 */
	if (_ullVal >= (uint64_t)1e6) 
	{
		_sp += sprintf (_sp,"%d.", (uint32_t)(_ullVal / (uint64_t)1e6));
		_ullVal %= (uint64_t)1e6;
		sprintf (_sp,"%03d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
		return;
	}
	
	/* ��ֵ���ڵ���10^3 */
	if (_ullVal >= 1000) 
	{
		sprintf (_sp, "%d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
		return;
	}
	
	/* ������ֵ */
	sprintf (_sp,"%d",(U32)(_ullVal));
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
	uint8_t result;
	Media_INFO info;
	uint64_t ullNANDCapacity;
	FAT_VI *mc0;  
	uint8_t buf[15];
	
	/* 
	   �ͼ���ʽ����ע��һ��Ҫִ���˺���NAND_Init����ִ��NAND_Format()����Ϊ��
	   1. finit("N0:") �����NAND_Init����FlahFS��nandflash�Ľӿ��ļ�FS_NAND_FlashPrg.c
	   2. funinit("N0:") �����UnInit����FlahFS��nandflash�Ľӿ��ļ�FS_NAND_FlashPrg.c
	*/
	NAND_Init();
	xprintf("���ڽ��еͼ���ʽ����....\r\n");
	NAND_Format();
	xprintf("�ͼ���ʽ�����....\r\n");

	/* ����SD�� */
	result = finit("N0:");
	if(result != NULL)
	{
		xprintf("�����ļ�ϵͳʧ�� (%s)\r\n", ReVal_Table[result]);
		xprintf("����ʧ�ܣ�NAND Flash��Ҫ����FAT32��ʽ��\r\n");
	}
	else
	{
		xprintf("�����ļ�ϵͳ�ɹ� (%s)\r\n", ReVal_Table[result]);
	}
	
	xprintf("���ڽ���FAT32��ʽ����....\r\n");
	if (fformat ("N0:") != 0)  
	{            
		xprintf ("��ʽ��ʧ��\r\n");
	}
	else  
	{
		xprintf ("��ʽ���ɹ�\r\n");
	}
	
	xprintf("------------------------------------------------------------------\r\n");
	/* ��ȡvolume label */
	if (fvol ("N0:", (char *)buf) == 0) 
	{
		if (buf[0]) 
		{
			xprintf ("NAND Flash��volume label�� %s\r\n", buf);
		}
		else 
		{
			xprintf ("NAND Flashû��volume label\r\n");
		}
	}
	else 
	{
		xprintf ("Volume���ʴ���\r\n");
	}

	/* ��ȡNAND Flashʣ������ */
	ullNANDCapacity = ffree("N0:");
	DotFormat(ullNANDCapacity, (char *)buf);
	xprintf("NAND Flashʣ������ = %10s�ֽ�\r\n", buf);
	
	/* ж��NAND Flash */
	result = funinit("N0:");
	if(result != NULL)
	{
		xprintf("ж���ļ�ϵͳʧ��\r\n");
	}
	else
	{
		xprintf("ж���ļ�ϵͳ�ɹ�\r\n");
	}
	
	/* ��ȡ��Ӧ�洢�豸�ľ�� */
	mc0 = ioc_getcb("N0:");          
   
	/* ��ʼ��FAT�ļ�ϵͳ��ʽ�Ĵ洢�豸 */
	if (ioc_init (mc0) == 0) 
	{
		/* ��ȡ�洢�豸��������Ϣ */
		ioc_read_info (&info, mc0);

		/* �ܵ������� * ������С��NAND Flash��������С��512�ֽ� */
		ullNANDCapacity = (uint64_t)info.block_cnt << 9;
		DotFormat(ullNANDCapacity, (char *)buf);
		xprintf("NAND Flash������ = %10s�ֽ�\r\nNAND Flash���������� = %d \r\n", buf, info.block_cnt);
	
		xprintf("NAND Flash��������С = %d�ֽ�\r\n", info.read_blen);
		xprintf("NAND Flashд������С = %d�ֽ�\r\n", info.write_blen);
	}
	else 
	{
		xprintf("��ʼ��ʧ��\r\n");
	}
	
	/* ж��NAND Flash */
	if(ioc_uninit (mc0) != NULL)
	{
		xprintf("ж��ʧ��\r\n");		
	}
	else
	{
		xprintf("ж�سɹ�\r\n");	
	}
	xprintf("------------------------------------------------------------------\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: ViewRootDir
*	����˵��: ��ʾNAND FlashĿ¼�µ��ļ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void ViewRootDir(void)
{
	uint8_t result;
	FINFO info;
	uint64_t ullNANDCapacity;
	uint8_t buf[15];
	
    info.fileID = 0;                /* ÿ��ʹ��ffind����ǰ��info.fileID�����ʼ��Ϊ0 */

	/* ����NAND Flash */
	result = finit("N0:");
	if(result != NULL)
	{
		/* �������ʧ�ܣ���ز�Ҫ�ٵ���FlashFS������API��������ֹ����Ӳ���쳣 */
		xprintf("�����ļ�ϵͳʧ�� (%s)\r\n", ReVal_Table[result]);
		goto access_fail;
	}
	else
	{
		xprintf("�����ļ�ϵͳ�ɹ� (%s)\r\n", ReVal_Table[result]);
	}
	xprintf("------------------------------------------------------------------\r\n");
	xprintf("�ļ���                 |  �ļ���С     | �ļ�ID  | ����      |����\r\n");
	
	/* 
	   ����Ŀ¼�µ������ļ��г�����
	   1. "*" ���� "*.*" ����ָ��·���µ������ļ�
	   2. "abc*"         ����ָ��·������abc��ͷ�������ļ�
	   3. "*.jpg"        ����ָ��·������.jpg��β�������ļ�
	   4. "abc*.jpg"     ����ָ��·������abc��ͷ��.jpg��β�������ļ�
	
	   ������ʵ��������Ŀ¼�������ļ�
	*/
	while(ffind ("N0:*.*", &info) == 0)  
	{ 
		/* �����ļ���ʾ��С��ʽ */
		DotFormat(info.size, (char *)buf);
		
		/* ��ӡ��Ŀ¼�µ������ļ� */
		xprintf ("%-20s %12s bytes, ID: %04d  ",
				info.name,
				buf,
				info.fileID);
		
		/* �ж����ļ�������Ŀ¼ */
		if (info.attrib & ATTR_DIRECTORY)
		{
			xprintf("(0x%02x)Ŀ¼", info.attrib);
		}
		else
		{
			xprintf("(0x%02x)�ļ�", info.attrib);
		}
		
		/* ��ʾ�ļ����� */
		xprintf ("  %04d.%02d.%02d  %02d:%02d\r\n",
                 info.time.year, info.time.mon, info.time.day,
               info.time.hr, info.time.min);
    }
	
	if (info.fileID == 0)  
	{
		xprintf ("NAND Flash��û�д���ļ�\r\n");
	}
	
	/* ��ȡNAND Flashʣ������ */
	ullNANDCapacity = ffree("N0:");
	DotFormat(ullNANDCapacity, (char *)buf);
	xprintf("NAND FLASH������ = %10s�ֽ�\r\n", buf);

access_fail:
	/* ж��NAND Flash */
	result = funinit("N0:");
	if(result != NULL)
	{
		xprintf("ж���ļ�ϵͳʧ��\r\n");
	}
	else
	{
		xprintf("ж���ļ�ϵͳ�ɹ�\r\n");
	}
	
	xprintf("------------------------------------------------------------------\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: CreateNewFile
*	����˵��: ��NAND Flash��������text�ı����ֱ�ʹ��fwrite��fprintf��fputsд���ݡ�
*	��    ��: ��
*	�� �� ֵ: ��
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

	/* ����NAND Flash */
	result = finit("N0:");
	if(result != NULL)
	{
		/* �������ʧ�ܣ���ز�Ҫ�ٵ���FlashFS������API��������ֹ����Ӳ���쳣 */
		xprintf("�����ļ�ϵͳʧ�� (%s)\r\n", ReVal_Table[result]);
		goto access_fail;
	}
	else
	{
		xprintf("�����ļ�ϵͳ�ɹ� (%s)\r\n", ReVal_Table[result]);
	}
	xprintf("------------------------------------------------------------------\r\n");
	
	/**********************************************************************************************************/
	/* ���ļ���test�е��ļ�test1.txt�����û�����ļ��к�txt�ļ����Զ�����*/
	fout = fopen ("N0:\\test\\test1.txt", "w"); 
	if (fout != NULL) 
	{
		xprintf("���ļ�N0:\\test\\test1.txt�ɹ������û�����ļ��к�txt�ļ����Զ�����\r\n");
		/* д���� */
		bw = fwrite (WriteText, sizeof(uint8_t), sizeof(WriteText)/sizeof(uint8_t), fout);
		if(bw == sizeof(WriteText)/sizeof(uint8_t))
		{
			xprintf("д�����ݳɹ�\r\n");
		}
		else
		{ 
			xprintf("д������ʧ��\r\n");
		}
		
		/* �ر��ļ� */
		fclose(fout);
	}
	else
	{
		xprintf("���ļ�N0:\\test\\test.txtʧ��\r\n");
	}
	
	/**********************************************************************************************************/
	/* ���ļ���test�е��ļ�test2.txt�����û�����ļ��к�txt�ļ����Զ�����*/
	fout = fopen ("N0:\\test\\test2.txt", "w"); 
	if (fout != NULL) 
	{
		xprintf("���ļ�N0:\\test\\test2.txt�ɹ������û�����ļ��к�txt�ļ����Զ�����\r\n");
		
		/* д���� */
		bw = fprintf (fout, "%d %d %f\r\n", i, i*5, i*5.55f);
		
		/* ʹ�ú���ferror����Ƿ��������� */
		if (ferror(fout) != NULL)  
		{
			xprintf("д������ʧ��\r\n");
		}
		else
		{
			xprintf("д�����ݳɹ�\r\n");	
		}
		
		/* �ر��ļ� */
		fclose(fout);
	}
	else
	{
		xprintf("���ļ�N0:\\test\\test.txtʧ��\r\n");
	}
	
	/**********************************************************************************************************/
	/* ���ļ���test�е��ļ�test3.txt�����û�����ļ��к�txt�ļ����Զ�����*/
	fout = fopen ("N0:\\test\\test3.txt", "w"); 
	if (fout != NULL) 
	{
		xprintf("���ļ�N0:\\test\\test3.txt�ɹ������û�����ļ��к�txt�ļ����Զ�����\r\n");
		
		/* д���� */
		fputs((const char *)WriteText1, fout);
		
		/* ʹ�ú���ferror����Ƿ��������� */
		if (ferror(fout) != NULL)  
		{
			xprintf("д������ʧ��\r\n");
		}
		else
		{
			xprintf("д�����ݳɹ�\r\n");	
		}
		
		/* �ر��ļ� */
		fclose(fout);
	}
	else
	{
		xprintf("���ļ�N0:\\test\\test.txtʧ��\r\n");
	}

access_fail:	
	/* ж��NAND Flash */
	result = funinit("N0:");
	if(result != NULL)
	{
		xprintf("ж���ļ�ϵͳʧ��\r\n");
	}
	else
	{
		xprintf("ж���ļ�ϵͳ�ɹ�\r\n");
	}

	xprintf("------------------------------------------------------------------\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: ReadFileData
*	����˵��: �ֱ�ʹ��fread��fscan��fgets��ȡ������ͬ��txt�ļ���
*	��    ��: ��
*	�� �� ֵ: ��
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

	/* ����NAND Flash */
	result = finit("N0:");
	if(result != NULL)
	{
		/* �������ʧ�ܣ���ز�Ҫ�ٵ���FlashFS������API��������ֹ����Ӳ���쳣 */
		xprintf("�����ļ�ϵͳʧ�� (%s)\r\n", ReVal_Table[result]);
		goto access_fail;
	}
	else
	{
		xprintf("�����ļ�ϵͳ�ɹ� (%s)\r\n", ReVal_Table[result]);
	}
	xprintf("------------------------------------------------------------------\r\n");
	
	/**********************************************************************************************************/
	/* ���ļ���test�е��ļ�test1.txt */
	fin = fopen ("N0:\\test\\test1.txt", "r"); 
	if (fin != NULL) 
	{
		xprintf("<1>���ļ�N0:\\test\\test1.txt�ɹ�\r\n");
		
		/* ��ֹ���� */
		(void) WriteText;
		
		/* ������ */
		bw = fread(Readbuf, sizeof(uint8_t), sizeof(WriteText)/sizeof(uint8_t), fin);
		if(bw == sizeof(WriteText)/sizeof(uint8_t))
		{
			Readbuf[bw] = NULL;
			xprintf("test1.txt �ļ����� : \r\n%s\r\n", Readbuf);
		}
		else
		{ 
			xprintf("������ʧ��\r\n");
		}
		
		/* �ر��ļ� */
		fclose(fin);
	}
	else
	{
		xprintf("���ļ�N0:\\test\\test.txtʧ��, �����ļ�������\r\n");
	}
	
	/**********************************************************************************************************/
	/* ���ļ���test�е��ļ�test2.txt */
	fin = fopen ("N0:\\test\\test2.txt", "r"); 
	if (fin != NULL) 
	{
		xprintf("\r\n<2>���ļ�N0:\\test\\test2.txt�ɹ�\r\n");
		
		bw = fscanf(fin, "%d %d %f", &index1, &index2, &count);

		/* 3�������������� */
		if (bw == 3)  
		{
			xprintf("��������ֵ\r\nindex1 = %d index2 = %d count = %f\r\n", index1, index2, count);
		}
		else
		{
			xprintf("������ʧ��\r\n");	
		}
		
		/* �ر��ļ� */
		fclose(fin);
	}
	else
	{
		xprintf("���ļ�N0:\\test\\test.txtʧ��\r\n");
	}
	
	/**********************************************************************************************************/
	/* ���ļ���test�е��ļ�test3.txt�����û�����ļ��к�txt�ļ����Զ�����*/
	fin = fopen ("N0:\\test\\test3.txt", "r"); 
	if (fin != NULL) 
	{
		xprintf("\r\n<3>���ļ�N0:\\test\\test3.txt�ɹ�\r\n");
		
		/* ������ */
		if(fgets((char *)Readbuf, sizeof(Readbuf), fin) != NULL)
		{
			xprintf("test3.txt �ļ����� : \r\n%s\r\n", Readbuf);
		}
		else
		{
			xprintf("������ʧ��\r\n");
		}
		
		/* �ر��ļ� */
		fclose(fin);
	}
	else
	{
		xprintf("���ļ�N0:\\test\\test.txtʧ��\r\n");
	}
	
access_fail:
	/* ж��NAND Flash */
	result = funinit("N0:");
	if(result != NULL)
	{
		xprintf("ж���ļ�ϵͳʧ��\r\n");
	}
	else
	{
		xprintf("ж���ļ�ϵͳ�ɹ�\r\n");
	}

	xprintf("------------------------------------------------------------------\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: SeekFileData
*	����˵��: ����һ��text�ı�����ָ��λ�ö�����ж�д������
*	��    ��: ��
*	�� �� ֵ: ��
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

	/* ����NAND Flash */
	result = finit("N0:");
	if(result != NULL)
	{
		/* �������ʧ�ܣ���ز�Ҫ�ٵ���FlashFS������API��������ֹ����Ӳ���쳣 */
		xprintf("�����ļ�ϵͳʧ�� (%s)\r\n", ReVal_Table[result]);
		goto access_fail;
	}
	else
	{
		xprintf("�����ļ�ϵͳ�ɹ� (%s)\r\n", ReVal_Table[result]);
	}
	xprintf("------------------------------------------------------------------\r\n");
	
	/**********************************************************************************************************/
	/* ���ļ���test�е��ļ�test1.txt�����û�����ļ��к�txt�ļ����Զ�����*/
	fout = fopen ("N0:\\test.txt", "w"); 
	if (fout != NULL) 
	{
		xprintf("���ļ�N0:\\test.txt�ɹ������û��txt�ļ����Զ�����\r\n");
		/* д���� */
		bw = fwrite (WriteText, sizeof(uint8_t), sizeof(WriteText)/sizeof(uint8_t), fout);
		if(bw == sizeof(WriteText)/sizeof(uint8_t))
		{
			xprintf("д�����ݳɹ�\r\n");
		}
		else
		{ 
			xprintf("д������ʧ��\r\n");
		}
			
		/* �ر��ļ� */
		fclose(fout);
	}
	else
	{
		xprintf("���ļ�N0:\\test.txtʧ��\r\n");
	}
	
	/***********************************************/
	fin = fopen ("N0:\\test.txt","r");
	if (fin != NULL)  
	{
		xprintf("\r\n���ļ�N0:\\test.txt�ɹ�\r\n");
		
		/* ��ȡ�ļ�test.txt��λ��0���ַ� */
		fseek (fin, 0L, SEEK_SET);
		uiPos = ftell(fin); 	
		ucChar = fgetc (fin);		
		xprintf("�ļ�test.txt��ǰ��ȡλ�ã�%02d���ַ���%c\r\n", uiPos, ucChar);
		
		/* ��ȡ�ļ�test.txt��λ��1���ַ� */
		fseek (fin, 1L, SEEK_SET);
		uiPos = ftell(fin); 	
		ucChar = fgetc (fin);		
		xprintf("�ļ�test.txt��ǰ��ȡλ�ã�%02d���ַ���%c\r\n", uiPos, ucChar);

		/* ��ȡ�ļ�test.txt��λ��25���ַ� */
		fseek (fin, 25L, SEEK_SET);
		uiPos = ftell(fin); 	
		ucChar = fgetc (fin);		
		xprintf("�ļ�test.txt��ǰ��ȡλ�ã�%02d���ַ���%c\r\n", uiPos, ucChar);
		
		/* ͨ�����溯���Ĳ�������ǰ��дλ����26
		   ���溯�����ڵ�ǰλ�õĻ����Ϻ���2��λ�ã�Ҳ����24�����ú���fgetc��λ�þ���25
		 */
		fseek (fin, -2L, SEEK_CUR);
		uiPos = ftell(fin); 	
		ucChar = fgetc (fin);		
		xprintf("�ļ�test.txt��ǰ��ȡλ�ã�%02d���ַ���%c\r\n", uiPos, ucChar);
		
		/* ��ȡ�ļ�test.txt�ĵ�����2���ַ�, ���һ����'\0' */
		fseek (fin, -2L, SEEK_END); 
		uiPos = ftell(fin); 
		ucChar = fgetc (fin);
		xprintf("�ļ�test.txt��ǰ��ȡλ�ã�%02d���ַ���%c\r\n", uiPos, ucChar);
		
		/* ����ȡλ�����¶�λ���ļ���ͷ */
		rewind(fin);  
		uiPos = ftell(fin); 
		ucChar = fgetc (fin);
		xprintf("�ļ�test.txt��ǰ��ȡλ�ã�%02d���ַ���%c\r\n", uiPos, ucChar);	
		
		/* 
		   ��������ʾһ��ungetc�����ã��˺������ǽ���ǰ�Ķ�ȡλ��ƫ�ƻ�һ���ַ���
		   ��fgetc���ú�λ������һ���ַ���
		 */
		fseek (fin, 0L, SEEK_SET);
		ucChar = fgetc (fin);
		uiPos = ftell(fin); 
		xprintf("�ļ�test.txt��ǰ��ȡλ�ã�%02d���ַ���%c\r\n", uiPos, ucChar);
		ungetc(ucChar, fin); 
		uiPos = ftell(fin); 
		xprintf("�ļ�test.txt��ǰ��ȡλ�ã�%02d���ַ���%c\r\n", uiPos, ucChar);
		
		/* �ر�*/
		fclose (fin);
	}
	else
	{
		xprintf("���ļ�N0:\\test.txtʧ��\r\n");
	}
	
	/***********************************************/
	fin = fopen ("N0:\\test.txt","r+");
	if (fin != NULL)  
	{
		xprintf("\r\n���ļ�N0:\\test.txt�ɹ�\r\n");
		
		/* �ļ�test.txt��λ��2�������ַ� '��' */
		fseek (fin, 2L, SEEK_SET);
		ucChar = fputc ('!', fin);
		/* ˢ�����ݵ��ļ��� */
		fflush(fin);		

		fseek (fin, 2L, SEEK_SET);
		uiPos = ftell(fin); 
		ucChar = fgetc (fin);				
		xprintf("�ļ�test.txtλ�ã�%02d���Ѿ��������ַ���%c\r\n", uiPos, ucChar);
	
		
		/* �ļ�test.txt�ĵ�����2���ַ�, �������ַ� ��&�� �����һ����'\0' */
		fseek (fin, -2L, SEEK_END); 
		ucChar = fputc ('&', fin);
		/* ˢ�����ݵ��ļ��� */
		fflush(fin);	

		fseek (fin, -2L, SEEK_END); 
		uiPos = ftell(fin); 
		ucChar = fgetc (fin);	
		xprintf("�ļ�test.txtλ�ã�%02d���Ѿ��������ַ���%c\r\n", uiPos, ucChar);		
		
		/* �ر�*/
		fclose (fin);
	}
	else
	{
		xprintf("���ļ�N0:\\test.txtʧ��\r\n");
	}
	
access_fail:	
	/* ж��NAND Flash */
	result = funinit("N0:");
	if(result != NULL)
	{
		xprintf("ж���ļ�ϵͳʧ��\r\n");
	}
	else
	{
		xprintf("ж���ļ�ϵͳ�ɹ�\r\n");
	}

	xprintf("------------------------------------------------------------------\r\n");

}

/*
*********************************************************************************************************
*	�� �� ��: DeleteDirFile
*	����˵��: ɾ���ļ��к��ļ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DeleteDirFile(void)
{
	uint8_t result;
	uint8_t i;
	char FileName[50];
	
	/* ����NAND Flash */
	result = finit("N0:");
	if(result != NULL)
	{
		/* �������ʧ�ܣ���ز�Ҫ�ٵ���FlashFS������API��������ֹ����Ӳ���쳣 */
		xprintf("�����ļ�ϵͳʧ�� (%s)\r\n", ReVal_Table[result]);
		goto access_fail;
	}
	else
	{
		xprintf("�����ļ�ϵͳ�ɹ� (%s)\r\n", ReVal_Table[result]);
	}
	xprintf("------------------------------------------------------------------\r\n");
	
	/* ɾ���ļ� speed1.txt */
	for (i = 0; i < 20; i++)
	{
		xprintf(FileName, "N0:\\Speed%02d.txt", i);		/* ÿд1�Σ���ŵ��� */
		result = fdelete (FileName);
		if (result != NULL) 
		{
			xprintf("%s�ļ������ڣ�����ֵ��%d��\r\n", FileName, result);
		}
		else
		{
			xprintf("ɾ��%s�ļ��ɹ�\r\n", FileName);
		}
	}
	
	for(i = 1; i < 4; i++)
	{
		xprintf(FileName, "N0:\\test\\test%01d.txt", i);   /* ÿд1�Σ���ŵ��� */
		result = fdelete (FileName);
		if (result != NULL) 
		{
			xprintf("%s�ļ������ڣ�����ֵ��%d��\r\n", FileName, result);
		}
		else
		{
			xprintf("ɾ��%s�ļ��ɹ�\r\n", FileName);
		}	
	}
	
	/* ɾ���ļ���test */
	result = fdelete ("N0:\\test\\");
	if (result != NULL) 
	{
		xprintf("test�ļ��зǿջ򲻴��ڣ�����ֵ��%d��\r\n", result);
	}
	else
	{
		xprintf("ɾ��test�ļ��гɹ�\r\n");
	}
	
	/* ɾ��csv�ļ� */
	result = fdelete ("N0:\\record.csv");
	if (result != NULL) 
	{
		xprintf("record.csv�ļ������ڣ�����ֵ��%d��\r\n", result);
	}
	else
	{
		xprintf("ɾ��record.csv���ɹ�\r\n");
	}
	
access_fail:
	/* ж��NAND Flash */
	result = funinit("N0:");
	if(result != NULL)
	{
		xprintf("ж���ļ�ϵͳʧ��\r\n");
	}
	else
	{
		xprintf("ж���ļ�ϵͳ�ɹ�\r\n");
	}
		
	xprintf("------------------------------------------------------------------\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: WriteFileTest
*	����˵��: �����ļ���д�ٶ�
*	��    ��: ��
*	�� �� ֵ: ��
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

	/* ����NAND Flash -----------------------------------------*/
	result = finit("N0:");
	if(result != NULL)
	{
		/* �������ʧ�ܣ���ز�Ҫ�ٵ���FlashFS������API��������ֹ����Ӳ���쳣 */
		xprintf("�����ļ�ϵͳʧ�� (%s)\r\n", ReVal_Table[result]);
		goto access_fail;
	}
	else
	{
		xprintf("�����ļ�ϵͳ�ɹ� (%s)\r\n", ReVal_Table[result]);
	}
	
	xprintf("------------------------------------------------------------------\r\n");

	/* ���ļ� -----------------------------------------*/
	sprintf(TestFileName, "N0:\\Speed%02d.txt", s_ucTestSn++);		/* ÿд1�Σ���ŵ��� */
	fout = fopen (TestFileName, "w");
	if (fout ==  NULL)
	{
		xprintf("�ļ�: %s����ʧ��\r\n", TestFileName);
		goto access_fail;
	}
	
	/* дһ������ ����д�ٶ�-----------------------------------------*/
	xprintf("��ʼд�ļ�%s %dKB ...\r\n", TestFileName, TEST_FILE_LEN / 1024);
	runtime1 = GetSystemTimer();	// = os_time_get ();	/* ��ȡϵͳ����ʱ�� */
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
			xprintf("%s�ļ�дʧ��\r\n", TestFileName);
			break;
		}
	}
	runtime2 = GetSystemTimer();	//= os_time_get ();	/* ��ȡϵͳ����ʱ�� */

	if (err == 0)
	{
		timelen = (runtime2 - runtime1);
		xprintf("\r\n  д��ʱ : %dms   ƽ��д�ٶ� : %dB/S (%dKB/S)\r\n",
			timelen,
			(TEST_FILE_LEN * 1000) / timelen,
			((TEST_FILE_LEN / 1024) * 1000) / timelen);
	}
	
	/* ʹ�ú���ferror�ٴμ���Ƿ��������� -----------------------*/
	if (ferror (fout))  
	{
		xprintf ("д���ݹ����г��ֹ�����\r\n\n");
    }
	else
	{
		xprintf ("д���ݹ�����δ���ֹ�����\r\n\n");		
	}

	fclose (fout);	/* �ر��ļ�*/


	/* ��ʼ���ļ������Զ��ٶ� ---------------------------------------*/
	fout = fopen (TestFileName, "r");
	if (fout ==  NULL)
	{
		xprintf("�ļ�: %s��ȡʧ��\r\n", TestFileName);
		goto access_fail;
	}

	xprintf("��ʼ���ļ� %dKB ...\r\n", TEST_FILE_LEN / 1024);
	runtime1 = GetSystemTimer();	//= os_time_get ();	/* ��ȡϵͳ����ʱ�� */
	for (i = 0; i < TEST_FILE_LEN / BUF_SIZE; i++)
	{
		bw = fread(g_TestBuf, sizeof(uint8_t), BUF_SIZE, fout);   
		if (bw == BUF_SIZE)
		{
			if (((i + 1) % 8) == 0)
			{
				xprintf(".");
			}

			/* �Ƚ�д��������Ƿ���ȷ�������ᵼ�¶����ٶȽ������ */
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
	runtime2 = GetSystemTimer();	//= os_time_get ();	/* ��ȡϵͳ����ʱ�� */

	if (err == 0)
	{
		timelen = (runtime2 - runtime1);
		xprintf("\r\n  ����ʱ : %dms   ƽ�����ٶ� : %dB/S (%dKB/S)\r\n", timelen,
			(TEST_FILE_LEN * 1000) / timelen, ((TEST_FILE_LEN / 1024) * 1000) / timelen);
	}
	
	/* ʹ�ú���ferror�ٴμ���Ƿ��������� -----------------------*/
	if (ferror (fout))  
	{
		xprintf ("�����ݹ����г��ֹ�����\r\n");
    }
	else
	{
		xprintf ("�����ݹ�����δ���ֹ�����\r\n");		
	}

	fclose (fout);	/* �ر��ļ�*/
	
access_fail:
	/* ж��NAND Flash */
	result = funinit("N0:");
	if(result != NULL)
	{
		xprintf("ж���ļ�ϵͳʧ��\r\n");
	}
	else
	{
		xprintf("ж���ļ�ϵͳ�ɹ�\r\n");
	}
	
	xprintf("------------------------------------------------------------------\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: WriteCSVFile
*	����˵��: д���ݵ�CSV�ļ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void WriteCSVFile(void)
{
	const uint8_t WriteText[] = {"��¼ʱ��,���,��¼��ֵһ,��¼��ֵ��\r\n"};
	FILE *fout;
	uint8_t result;
	static uint16_t i = 1;
	static uint8_t ucFirstRunFlag = 0;

	/* ����SD�� */
	result = finit("N0:");
	if(result != NULL)
	{
		/* �������ʧ�ܣ���ز�Ҫ�ٵ���FlashFS������API��������ֹ����Ӳ���쳣 */
		xprintf("�����ļ�ϵͳʧ�� (%s)\r\n", ReVal_Table[result]);
		goto access_fail;
	}
	else
	{
		xprintf("�����ļ�ϵͳ�ɹ� (%s)\r\n", ReVal_Table[result]);
	}
	xprintf("------------------------------------------------------------------\r\n");
	
	/**********************************************************************************************************/
	/* 
	  1. ���ļ�record.csv�����û�д��ļ����Զ�������
	  2. �ڶ���������ʾ����ļ�д���ݶ���β����ʼ��ӡ�
	*/
	fout = fopen ("N0:\\record.csv", "a"); 
	if (fout != NULL) 
	{
		xprintf("���ļ�N0:\\record.csvt�ɹ������û�д��ļ����Զ�����\r\n");
		
		/* д���ݣ�����ǵ�һ��д���ݣ���дCSV�ļ��ı������Ŀ���Ժ�д���ݲ�����Ҫд�˱��� */
		if(ucFirstRunFlag == 0)
		{
			fprintf(fout, (char *)WriteText);
			ucFirstRunFlag = 1;
		}
		
		/* ����д5������ */
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
		
		/* ʹ�ú���ferror����Ƿ��������� */
		if (ferror(fout) != NULL)  
		{
			xprintf("д������ʧ��\r\n");
		}
		else
		{
			xprintf("д�����ݳɹ�\r\n");	
		}
		
		/* �ر��ļ� */
		fclose(fout);
	}
	else
	{
		xprintf("���ļ�N0:\\record.csvʧ��\r\n");
	}

access_fail:	
	/* ж��SD�� */
	result = funinit("N0:");
	if(result != NULL)
	{
		xprintf("ж���ļ�ϵͳʧ��\r\n");
	}
	else
	{
		xprintf("ж���ļ�ϵͳ�ɹ�\r\n");
	}

	xprintf("------------------------------------------------------------------\r\n");
}

/*
*********************************************************************************************************
*	�� �� ��: ReadCSVFileData
*	����˵��: ��ȡCSV�ļ��е����ݡ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void ReadCSVFileData(void)
{
	FILE *fin;
	uint32_t bw;
	uint8_t result;

	/* ����SD�� */
	result = finit("N0:");
	if(result != NULL)
	{
		/* �������ʧ�ܣ���ز�Ҫ�ٵ���FlashFS������API��������ֹ����Ӳ���쳣 */
		xprintf("�����ļ�ϵͳʧ�� (%s)\r\n", ReVal_Table[result]);
		goto access_fail;
	}
	else
	{
		xprintf("�����ļ�ϵͳ�ɹ� (%s)\r\n", ReVal_Table[result]);
	}
	xprintf("------------------------------------------------------------------\r\n");
	
	/*****************************************************.
	*****************************************************/
	/* ��record.csv�ļ� */
	fin = fopen ("N0:\\record.csv", "r"); 
	if (fin != NULL) 
	{
		xprintf("���ļ�N0:\\record.csv�ɹ�\r\n");
		/* ��record.csv�ļ��е��������ݶ���ȡ���� */
        while(1)
		{
			bw = fread(g_TestBuf, sizeof(uint8_t), BUF_SIZE/2, fin);
			/* ĩβ���Ͻ����������ô��ڴ�ӡ��ֹλ�� */
			g_TestBuf[bw] = '\0';
			xprintf("%s", g_TestBuf);
			if(bw < BUF_SIZE/2) break;	
		}
		
		/* ʹ�ú���ferror����Ƿ��������� */
		if (ferror(fin) != NULL)  
		{
			xprintf("��������ʧ��\r\n");
		}
		else
		{
			xprintf("record.csv���������ȫ����ȷ����\r\n");	
		}
		
		/* �ر��ļ� */
		fclose(fin);
	}
	else
	{
		xprintf("���ļ�N0:\\record.csvʧ�ܣ������ļ������ڣ��Ѿ���ɾ��\r\n");
	}
	
access_fail:
	/* ж��SD�� */
	result = funinit("N0:");
	if(result != NULL)
	{
		xprintf("ж���ļ�ϵͳʧ��\r\n");
	}
	else
	{
		xprintf("ж���ļ�ϵͳ�ɹ�\r\n");
	}

	xprintf("------------------------------------------------------------------\r\n");
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
	const uint8_t WriteText[] = {"STM32 Demo application\r\n2018-03-09\r\n"};
	const uint8_t WriteText1[] = {"STM32 Demo application Test...\r"};
	FILE *fout;
	uint32_t bw;
	uint32_t i = 2;
	uint8_t result;
	uint32_t rng_data,Filenum=0;
	char filename[100];
	
	rng_data = GetRandomNumber();
	
	/* ����NAND Flash */
	result = finit("N0:");
	if(result != NULL)
	{
		/* �������ʧ�ܣ���ز�Ҫ�ٵ���FlashFS������API��������ֹ����Ӳ���쳣 */
		xprintf("�����ļ�ϵͳʧ�� (%s)\r\n", ReVal_Table[result]);
		goto access_fail;
	}
	else
	{
		xprintf("�����ļ�ϵͳ�ɹ� (%s)\r\n", ReVal_Table[result]);
	}
	xprintf("------------------------------------------------------------------\r\n");
	
	/**********************************************************************************************************/
	/* ���ļ���test�е��ļ�test1.txt�����û�����ļ��к�txt�ļ����Զ�����*/
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
			/* д���� */
			bw = fwrite (WriteText, sizeof(uint8_t), sizeof(WriteText)/sizeof(uint8_t), fout);
			if(bw == sizeof(WriteText)/sizeof(uint8_t))
			{
				//xprintf("д�����ݳɹ�\r\n");
			}
			else
			{ 
				xprintf("д������ʧ��\r\n");
				break;
			}
			
			/* �ر��ļ� */
			fclose(fout);
		}
		else
		{
			xprintf("���ļ�ʧ��\r\n");
			break;
		}
	}

access_fail:	
	/* ж��NAND Flash */
	result = funinit("N0:");
	if(result != NULL)
	{
		xprintf("ж���ļ�ϵͳʧ��\r\n");
	}
	else
	{
		xprintf("ж���ļ�ϵͳ�ɹ�\r\n");
	}

	xprintf("------------------------------------------------------------------\r\n");
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
