/**
  ******************************************************************************
  * @file    UDisk_demo.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-August-2013
  * @brief   This file contain the demo implementation
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
//#include <string.h>

#include "UDisk_demo.h"
#include "monitor.h"

/** @addtogroup USBH_USER
* @{
*/

/** @addtogroup USBH_DUAL_FUNCT_DEMO
* @{
*/

/** @defgroup USBH_DUAL_FUNCT_DEMO
* @brief    This file includes the usb host stack user callbacks
* @{
*/

/** @defgroup USBH_DUAL_FUNCT_DEMO_Private_TypesDefinitions
* @{
*/
/**
* @}
*/


/** @defgroup USBH_DUAL_FUNCT_DEMO_Private_Defines
* @{
*/
#  define IMAGE_BUFFER_SIZE  512

#  define BF_TYPE 0x4D42             /* "MB" */

#  define BI_RGB       0             /* No compression - straight BGR data */
#  define BI_RLE8      1             /* 8-bit run-length compression */
#  define BI_RLE4      2             /* 4-bit run-length compression */
#  define BI_BITFIELDS 3             /* RGB bitmap with RGB masks */

/**
* @}
*/


/** @defgroup USBH_DUAL_FUNCT_DEMO_Private_Macros
* @{
*/

/**
* @}
*/


/** @defgroup USBH_DUAL_FUNCT_DEMO_Private_Variables
* @{
*/


extern BYTE USBH_USR_ApplicationState;

BYTE Image_Buf[IMAGE_BUFFER_SIZE];

BYTE filenameString[15]  = {0};
FATFS fatfs;
static FIL file;
DWORD DiskCapacity;

DEMO_StateMachine             demo;
BYTE line_idx = 0;
BYTE wait_user_input = 0;
BYTE Device_EnumDone      = 0;
BYTE * DEMO_main_menu[] =
{
  "1 - Host Demo \n",
  "2 - (RESERVED)\n",
  "3 - Credits   \n",
};

BYTE * DEMO_HOST_menu[] =
{
  "1 - Explore UDisk content\n",
  "2 - Write File to disk\n",
  "3 - Read .CPP file \n",
  "4 - Disamount\n",
};

BYTE * DEMO_DEVICE_menu[] =
{
  "1 - Return               \n",
  "                         \n",
  "                         \n",
};

BYTE writeTextBuff[] = "STM32 Connectivity line Host Demo application using FAT_FS   ";
BYTE *writeFrom;//=(BYTE*)0x8000000;

/** @defgroup USBH_DUAL_FUNCT_DEMO_Private_Constants
* @{
*/
static void Demo_Application (void);
static void Demo_SelectItem (BYTE **menu , BYTE item, int items);
static BYTE Explore_Disk (char* path , BYTE recu_level);
static BYTE Image_Browser (char* path);
static void     Show_Image(void);
static void Toggle_Leds(void);
static BYTE Check_BMP_file(BYTE *buf);
/**
* @}
*/



/** @defgroup USBH_DUAL_FUNCT_DEMO_Private_FunctionPrototypes
* @{
*/
/**
* @}
*/


/** @defgroup USBH_DUAL_FUNCT_DEMO_Private_Functions
* @{
*/

/**
* @brief  Demo_Init
*         Demo initialization
* @param  None
* @retval None
*/

void Demo_Init (void)
{

  xputs("> Initializing demo....\n");

  Demo_SelectItem (DEMO_main_menu, 0,1);
  xputs("> Demo Initialized\n");
  xputs("> Use Keyboard to Select demo.\n");

}
/**
* @brief  Demo_PressAnyKey
          Waiting for any key pressed
* @param  None
* @retval None
*/

void    Demo_PressAnyKey(BYTE testUSB)
{
    //ccr>>>>>>>>>>
    wait_user_input = 1;
    xputs( "Press any key to continue...\n");

    while(wait_user_input != 2)
    {
      if (testUSB && !USB_DeviceConnected())
      {
          Device_EnumDone = 0;
          break;
      }
      Demo_ProbeKey();
      Toggle_Leds();
    }
    wait_user_input = 0;
    //<<<<<<<<<<<<<<<
}


/**
* @brief  Demo_ProbeKey
*         Probe the joystick state
* @param  state : joystick state
* @retval None
*/
//ER220 keypad
#define   KEY_NONE  0x08  //exit
#define   KEY_SEL   0x0d  //select
#define   KEY_DOWN  'b'  //'2'
#define   KEY_UP    'a'  //'8'

void Demo_ProbeKey(void)
{
    BYTE state=0;

    if (CheckKeyboard())
        state = Getch();
    //else if (!UART_GetChar_nb (&state))
		//		return;
		else 
		{
			state = UART_GetChar_nb();
			if (UART_GetChar_nb()==0xff)
        return;
		}


  /*Explorer running */
  if((wait_user_input == 1))
  {
    wait_user_input = 2;
    return;
  }
  if (state>='1' && state<='9')
  {
    demo.select = (state & 0x0f) -1;
  }
  else if ((state == KEY_UP)&& (demo.select > 0))
  {
    demo.select--;
  }
  else if (((state == KEY_DOWN) && (demo.select < 2)) ||
           ((demo.state == DEMO_HOST) && (state == KEY_DOWN) &&(demo.select < 3)))
  {

    if (!((demo.state == DEMO_DEVICE) && demo.select == 0))
    {
      demo.select++;
    }
  }
  else if (state == KEY_SEL)
  {
    demo.select  |= 0x80;
  }
}

/**
* @brief  Demo_SelectItem
*         manage the menu on the screen
* @param  menu : menu table
*         item : selected item to be highlighted
* @retval None
*/

static void Demo_SelectItem (BYTE **menu , BYTE item, int items)
{

	int i;

	//mClearScreen();
	//Bios0(BiosCmd_OPDisp_ClearSCR);
	xputs(menu [item]);xputc('\n');
	/*
	for (i=0;i<items;i++)
	{
		if (i==item) mSetInvAttr();
		mDispStringXY(menu [i],0,i);
		if (i==item) mClearInvAttr();
	}
	*/

}

/**
* @brief  Demo_Application
*         Demo state machine
* @param  None
* @retval None
*/
/*接入U盘串口调试信息
>Device Attached
>Reset device
>FS Device Found
 VID: 1F75h
 PID: 0902h
>Device Address Assigned
>UDisk/SD Found
>Manufacturer: Innostor
>Product: USB3.0 Drive
>SN:0902000000000000000000000103
>Device Ready.
> USB Host High speed initialized.
> File System initialized.
> Disk capacity : 3837787648 Bytes
Press any key to continue...

*/

static void Demo_Application (void)
{
  static BYTE prev_select = 0;
  WORD bytesWritten, bytesToWrite,i;
  BYTE ch;

  FRESULT res;

  DIR dir;
		
  switch (demo.state)
  {
  case  DEMO_IDLE:
    Demo_SelectItem (DEMO_main_menu, 0,3);
    demo.state = DEMO_WAIT;
    demo.select = 0;
    break;
  case  DEMO_WAIT:
    if(demo.select != prev_select)
    {
      prev_select = demo.select ;
      Demo_SelectItem (DEMO_main_menu, demo.select & 0x7F,3);

      /* Handle select item */
      if(demo.select & 0x80)
      {
        demo.select &= 0x7F;

        switch (demo.select)
        {
        case  0:
          demo.state = DEMO_HOST;
          demo.Host_state = DEMO_HOST_IDLE;
          break;

        case 1:
          demo.state = DEMO_DEVICE;
          demo.Device_state = DEMO_DEVICE_IDLE;
          break;

        case 2:
          xputs("\nSystem Information :\n");
          xputs("_________________________\n\n");

#if defined (USE_STM322xG_EVAL)
          xputs("Board : STM322xG-Eval.\n");
          xputs("Device: STM32F2xx.\n");
          xputs("USB Host Library v2.1.0.\n");
          xputs("USB Device Library v1.1.0.\n");
          xputs("USB OTG Driver v2.1.0\n");
          xputs("STM32 Std Library v1.1.1.\n");
#elif defined (USE_STM324xG_EVAL)
          xputs("Board : STM324xG-Eval.\n");
          xputs("Device: STM32F4xx.\n");
          xputs("USB Host Library v2.1.0.\n");
          xputs("USB Device Library v1.1.0.\n");
          xputs("USB OTG Driver v2.1.0\n");
          xputs("STM32 Std Library v1.0.1.\n");
#elif defined (USE_STM3210C_EVAL)
          xputs("Board : STM3210C-Eval.\n");
          xputs("Device: STM32F10x.\n");
          xputs("USB Host Library v2.1.0.\n");
          xputs("USB Device Library v1.1.0.\n");
          xputs("USB OTG Driver v2.1.0\n");
          xputs("STM32 Std Library V3.6.0.\n");
#else
 #error "Missing define: Evaluation board (ie. USE_STM322xG_EVAL)"
#endif  /* USE_STM322xG_EVAL */

          demo.state = DEMO_IDLE;
          break;
        default:
          break;
        }
      }
    }
    break;
  case  DEMO_HOST:
    switch (demo.Host_state)
    {
    case  DEMO_HOST_IDLE:
      demo.Host_state = DEMO_HOST_WAIT;
      break;
    case  DEMO_HOST_WAIT:
      if(Device_EnumDone==0 && USB_EnumDone())
      {
#ifdef USE_USB_OTG_HS
        xputs("> USB Host High speed initialized.\n");
#else
        xputs("> USB Host Full speed initialized.\n");
#endif

        /* Initialises the File System*/
        if ( f_mount( 0, &fatfs ) != FR_OK )
        {
          /* efs initialisation fails*/
          xputs("> Cannot initialize File System.\n");
        }

        xputs("> File System initialized.\n");
				
				if (USB_DeviceType()==USB_DISK && USB_EnumDone())
        {
	        DiskCapacity = USB_DiskCapacity();
				}
        xprintf("> Disk capacity : %lu Bytes\n",DiskCapacity);
        Demo_PressAnyKey(1);//ccr
        Demo_SelectItem (DEMO_HOST_menu, 0,4);
        demo.select = 0;
        Device_EnumDone = 1;
      }

      if(Device_EnumDone == 1)
      {
        if(demo.select != prev_select)
        {
          prev_select = demo.select ;
          //ccr USB_OTG_DisableGlobalInt(&USB_OTG_Core);
          Demo_SelectItem (DEMO_HOST_menu, demo.select & 0x7F,4);
          //ccr USB_OTG_EnableGlobalInt(&USB_OTG_Core);

          /* Handle select item */
          if(demo.select & 0x80)
          {
            demo.select &= 0x7F;
            switch (demo.select)
            {
            case  0:
              Explore_Disk("0:/", 1);
              line_idx = 0;
              break;
            case 1:
              /* Writes a text file, STM32.TXT in the disk*/
              xputs("> Writing File to disk flash ...\n");
              if(USB_DiskProtected())
              {
                xputs ( "> Disk flash is write protected \n");
                break;
              }
              /* Register work area for logical drives */
              f_mount(0, &fatfs);

              //ccr>>>>>>>>>>>>>>>>
              res = f_opendir(&dir, "0:CCR");
              if (res != FR_OK)
              {
                  f_mkdir("0:CCR");
                  f_chdir("0:CCR");//ccr test
              }
              else
              {
                  f_chdir("0:CCR");//ccr test
              }
              //<<<<<<<<<<<<<<<<<<<
              if(f_open(&file, "STM32_0.TXT",FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
              {
                /* Write buffer to file */
                bytesToWrite = sizeof(writeTextBuff);
                for (i=0;i<bytesToWrite;i++)
                {
                    writeTextBuff[i] = ((*writeFrom) & 0x1f) +' ';
                    writeFrom++;
                    xputc(writeTextBuff[i]);
                }

                res= f_write (&file, writeTextBuff, bytesToWrite, (void *)&bytesWritten);

                for (i=0;i<bytesToWrite;i++)
                {
                    writeTextBuff[i] = ((*writeFrom) & 0x1f) +' ';
                    writeFrom++;
                    xputc(writeTextBuff[i]);
                }

                res= f_write (&file, writeTextBuff, bytesToWrite, (void *)&bytesWritten);

                if((bytesWritten == 0) || (res != FR_OK)) /*EOF or Error*/
                {
                  xputs("> STM32_0.TXT CANNOT be writen.\n");
                }
                else
                {
                  xputs("> 'STM32_0.TXT' file created\n");
                }

                /*close file and filesystem*/
                f_close(&file);
              }
              if(f_open(&file, "STM32_1.TXT",FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
              {
                /* Write buffer to file */
                bytesToWrite = sizeof(writeTextBuff);
                for (i=0;i<bytesToWrite;i++)
                {
                    writeTextBuff[i] = ((*writeFrom) & 0x1f) +' ';
                    writeFrom++;
                    xputc(writeTextBuff[i]);
                }

                res= f_write (&file, writeTextBuff, bytesToWrite, (void *)&bytesWritten);

                for (i=0;i<bytesToWrite;i++)
                {
                    writeTextBuff[i] = ((*writeFrom) & 0x1f) +' ';
                    writeFrom++;
                    xputc(writeTextBuff[i]);
                }

                res= f_write (&file, writeTextBuff, bytesToWrite, (void *)&bytesWritten);

                if((bytesWritten == 0) || (res != FR_OK)) /*EOF or Error*/
                {
                  xputs("> STM32_1.TXT CANNOT be writen.\n");
                }
                else
                {
                  xputs("> 'STM32_1.TXT' file created\n");
                }

                /*close file and filesystem*/
                f_close(&file);

                Demo_PressAnyKey(1);//ccr
              }
              break;
            case  2:
              if (f_mount( 0, &fatfs ) != FR_OK )
              {
                /* fat_fs initialisation fails*/
                break;
              }
              Image_Browser("0:/");

              /*Clear windows */
              xputs("> Slide show application closed.\n");
              Demo_PressAnyKey(1);//ccr

              break;

            case 3://
              f_mount(0, NULL);
              USB_Dismount();
              demo.state = DEMO_IDLE;
              xputs("> Use Key to Select demo.\n");
              break;
            default:
              break;
            }
          }
        }
      }
      break;
    default:
      break;
    }
    break;
  case  DEMO_DEVICE:
    switch (demo.Device_state)
    {
    case  DEMO_DEVICE_IDLE:
/*ccr>>>>>>>>>>
      USBD_Init(&USB_OTG_Core,
#ifdef USE_USB_OTG_FS
                USB_OTG_FS_CORE_ID,
#elif defined USE_USB_OTG_HS
                USB_OTG_HS_CORE_ID,
#endif
                &USR_desc,
                &USBD_MSC_cb,
                &USR_cb);

<<<<<<<<<<*/
      demo.Device_state = DEMO_DEVICE_WAIT;
      demo.select = 0;
      break;

    case  DEMO_DEVICE_WAIT:

      if(demo.select != prev_select)
      {
        prev_select = demo.select ;
        Demo_SelectItem (DEMO_DEVICE_menu, demo.select & 0x7F,3);

        /* Handle select item */
        if(demo.select & 0x80)
        {
          demo.select &= 0x7F;
          switch (demo.select)
          {
          case  0:
            demo.state = DEMO_IDLE;
            demo.select = 0;
            xputs("> Device application closed.\n");
//ccr            DCD_DevDisconnect (&USB_OTG_Core);
//ccr            USB_OTG_StopDevice(&USB_OTG_Core);
            break;
          default:
            break;
          }
        }
      }
      break;

    default:
      break;
    }
    break;
  default:
    break;
  }

}


/**
* @brief  Explore_Disk
*         Displays disk content
* @param  path: pointer to root path
* @retval None
*/
static BYTE Explore_Disk (char* path , BYTE recu_level)
{

  FRESULT res;
  FILINFO fno;
  DIR dir;
  char *fn;
  char tmp[14];


  res = f_opendir(&dir, path);
  if (res == FR_OK) {
    while(1)
    {
      if (!USB_DeviceConnected())
      {
          Device_EnumDone = 0;
          break;
      }
      res = f_readdir(&dir, &fno);
      if (res != FR_OK || fno.fname[0] == 0)
      {
        break;
      }
      if (fno.fname[0] == '.')
      {
        continue;
      }
      fn = fno.fname;
      strcpy(tmp, fn);
      line_idx++;
      wait_user_input = 0;

      if(recu_level == 1)
      {
        xputs("   |__");
      }
      else if(recu_level == 2)
      {
        xputs("   |   |__");
      }
      if((fno.fattrib & AM_MASK) == AM_DIR)
      {
        strcat(tmp, "\n");
        xputs((void *)tmp);
      }
      else
      {
        strcat(tmp, "\n");
        xputs((void *)tmp);
      }

      if(((fno.fattrib & AM_MASK) == AM_DIR)&&(recu_level == 1))
      {
        Explore_Disk(fn, 2);
      }
    }
  }
  return res;
}

/**
* @brief  Toggle_Leds
*         Toggle leds to shows user input state
* @param  None
* @retval None
*/
static void Toggle_Leds(void)
{
}

/**
* @brief  Show_Image
*         Displays BMP image
* @param  None
* @retval None
*/
static void Show_Image(void)
{

  WORD i = 0;
  WORD numOfReadBytes = 0;
  FRESULT res;

/*ccr>>>>>>>>>>>>
  if(Check_BMP_file(Image_Buf) > 0)
  {
    xputs("Bad BMP Format.\n");
    return;
  }
<<<<<<<<<<<<<<<<*/
  while (1)
  {
      if (!USB_DeviceConnected())
      {
          Device_EnumDone = 0;
          break;
      }

    res = f_read(&file, Image_Buf, IMAGE_BUFFER_SIZE, (void *)&numOfReadBytes);
    if((numOfReadBytes == 0) || (res != FR_OK)) /*EOF or Error*/
    {
      break;
    }
    for(i = 0 ; i < IMAGE_BUFFER_SIZE; i+= 2)
    {
        xputc(Image_Buf[i]);    xputc(Image_Buf[i+1]);
    }
  }

}


/**
* @brief  Show_Image
*         launch the Image browser
* @param  path string
* @retval status
*/
static BYTE Image_Browser (char* path)
{
  FRESULT res;
  BYTE ret = 1;
  FILINFO fno;
  DIR dir;
  char *fn;

  res = f_opendir(&dir, path);//打开文件夹
  if (res == FR_OK) {

    for (;;) {
      res = f_readdir(&dir, &fno);//枚举文件夹中的文件
      if (res != FR_OK || fno.fname[0] == 0)
      {
        wait_user_input = 0;
        break;
      }

      if (fno.fname[0] == '.') continue;

      fn = fno.fname;

      if (fno.fattrib & AM_DIR)
      {
        continue;
      }
      else
      {
//ccr        if((strstr(fn, "bmp")) || (strstr(fn, "BMP")))
        if (strstr(fn, "cpp") || strstr(fn, "CPP"))
        {

          res = f_open(&file, fn, FA_OPEN_EXISTING | FA_READ);//打开文件
          Show_Image();
          Demo_PressAnyKey(1);
          f_close(&file);//关闭文件

        }
      }
    }
  }
  return ret;
}

/**
* @brief  Check_BMP_file
*         Displays BMP image
* @param  None
* @retval None
*/
static BYTE Check_BMP_file(BYTE *buf)
{
  WORD             numOfReadBytes = 0;
  WORD  Type;
  DWORD  Width;
  DWORD  Height;
  WORD  Bpp;
  DWORD  Compression ;

  if(f_read(&file, buf, 54, (void *)&numOfReadBytes) != FR_OK) /*Error*/
  {
    return 1;
  }

  Type        = *(__packed WORD *)(buf + 0 );
  Width       = *(__packed DWORD *)(buf + 18 );
  Height      = *(__packed DWORD *)(buf + 22 );
  Bpp         = *(__packed WORD *)(buf + 28 );
  Compression = *(__packed DWORD *)(buf + 30 );

  if(( Type != BF_TYPE)||
     ( Width != 320)||
       ( Height != 240)||
         ( Bpp != 16)||
           ( Compression != BI_BITFIELDS))
  {
    return 1;
  }

  return 0;
}


/*****************************************************************************/


BYTE USB_Keyboard_Test(void)
{
    if (CheckUSBKey())
    {
        xputc(GetUSBKey(0));
    }
    return USB_GetHostState();
}

void  UDisk_Test()
{	
	Demo_ProbeKey();
	Demo_Application();//ccr    Demo_Process();		
}

/**
* @}
*/
void USB_Test()
{
      /* Init HS Core  : Demo start in host mode*/
  BYTE ctrl=1;

  USB_Start();

  demo.state = DEMO_HOST;
  while (1)
  {
		switch (USB_Detect())
		{
		case USB_KEYBOARD:
				USB_Keyboard_Test();
				ctrl = 0;
				break;
		case USB_DISK:
				UDisk_Test();
				ctrl = 0;
				break;
		default:
				demo.state = DEMO_HOST;
				if (ctrl==0)
				{
					xputs("> Device Removed.\n");
					return;
				}
				ctrl = 1;
				break;;
		}
  }
}

/**
* @}
*/

/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
