

#ifndef _USB_BSP_USR_H
#define _USB_BSP_USR_H

#include "usbd_msc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usb_conf.h"
#include "usb_bsp.h"

void usbd_OpenMassStorage(void);
void usbd_CloseMassStorage(void);

extern USB_OTG_CORE_HANDLE     USB_OTG_dev;

#endif


