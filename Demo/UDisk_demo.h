/**
  ******************************************************************************
  * @file    usbh_usr.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   This file is the header file for usb usr file
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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USR_DEMO_H__
#define __USR_DEMO_H__


/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "include.h"

#include "usb_bsp.h"
#include "usbd_usr.h"


typedef enum {
  DEMO_IDLE   = 0,
  DEMO_WAIT,
  DEMO_DEVICE,
  DEMO_HOST,
}Demo_State;

typedef enum {
  DEMO_HOST_IDLE   = 0,
  DEMO_HOST_WAIT,
}Demo_HOST_State;

typedef enum {
  DEMO_DEVICE_IDLE   = 0,
  DEMO_DEVICE_WAIT,
}Demo_DEVICE_State;

typedef struct _DemoStateMachine
{

  Demo_State           state;
  Demo_HOST_State       Host_state;
  Demo_DEVICE_State     Device_state;
  BYTE              select;
  BYTE              lock;

}DEMO_StateMachine;


/**
  * @}
  */


/** @defgroup USBH_DUAL_FUNCT_DEMO_Exported_FunctionsPrototype
  * @{
  */

void Demo_Init(void);
void Demo_Process(void);
void Demo_ProbeKey(void);
void USB_Test(void);

#endif /* __USR_DEMO_H__ */
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
