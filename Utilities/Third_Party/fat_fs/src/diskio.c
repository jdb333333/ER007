/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2013        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control module to the FatFs module with a defined API.        */
/*-----------------------------------------------------------------------*/
#define SD_ONLY     0   //ccr2017-04-01,=1时,只支持SD卡

#include "diskio.h"			/* FatFs lower layer API */
#include "ff.h"
#ifdef SPI_SD
#include "spi_sd.h"
#else
#include "sdio_sd.h"
#endif

#if defined(NAND_DISK)
#include "nand.h"
#include "ftl.h"
#endif

#if _USE_LFN == 3
#include "malloc.h"
#endif

#include "usb_conf.h"
#include "usbh_msc_core.h"

static volatile DSTATUS Stat = STA_NOINIT;  /* Disk status */
extern USB_OTG_CORE_HANDLE          USB_OTG_Core;
extern USBH_HOST                     USB_Host;


#define SECTOR_SIZE		512


#if (defined(SPI_SD) && defined(NAND_DISK))
#error "defined(SPI_SD) and defined(NAND_DISK)"
#endif

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/
DSTATUS SD_disk_initialize(BYTE pdrv )
{

    SD_Error Status;


		switch (pdrv)
		{
		case FS_SD :
				Status = SD_Init();
				break;

#if defined(FS_SD_2ND)
		case FS_SD_2ND :
				Status = SD_Init_2nd();
				break;
#endif
		default:
				break;
		}

    if (Status == SD_OK)
    {
        return RES_OK;
    } else
    {
        return STA_NODISK;
    }
}


DSTATUS disk_initialize (
                        BYTE pdrv               /* Physical drive nmuber (0..) */
                        )
{
    DSTATUS stat;

    switch (pdrv)
    {
#if defined(NAND_DISK)
	case FS_NAND :
		stat = FTL_Init();//=NAND_Init()
		if (stat != RES_OK)
		{
			/* 如果初始化失败，请执行低级格式化 */
			xprintf("FS_NAND Error!  \r\n");
			stat = RES_ERROR;
		}
		return stat;
#elif  defined(FS_SD_2ND)
	case FS_SD_2ND :
#endif
    case FS_SD :
		stat = SD_disk_initialize(pdrv);
		return stat;

    case FS_USB :
        if (HCD_IsDeviceConnected(&USB_OTG_Core))
        {
            Stat &= ~STA_NOINIT;
        }
        return stat;

    }
    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
                    BYTE pdrv       /* Physical drive nmuber (0..) */
                    )
{
    DSTATUS stat;

    switch (pdrv)
    {
    case FS_SD :
#if defined(NAND_DISK)
	case FS_NAND :
			stat = 0;
		return stat;
#elif  defined(FS_SD_2ND)
    case FS_SD_2ND :
#endif
        stat = 0;
        return stat;
    case FS_USB :
        return stat;
    }
    return STA_NOINIT;

}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
                  BYTE pdrv,      /* Physical drive nmuber (0..) */
                  BYTE *buff,     /* Data buffer to store read data */
                  DWORD sector,   /* Sector address (LBA) */
                  BYTE count      /* Number of sectors to read (1..128) */
                  )
{
    DRESULT res;

	  SD_Error Status = SD_OK;

	BYTE ustatus = USBH_MSC_OK;

    switch (pdrv)
    {
    case FS_SD :
    {
        if (count == 1)
        {
            Status = SD_ReadBlock(buff, sector << 9 , SECTOR_SIZE);
        } else
        {
            Status = SD_ReadMultiBlocks(buff, sector << 9 , SECTOR_SIZE, count);
        }
        if (Status != SD_OK)
        {
            return RES_ERROR;
        }
        return RES_OK;
     }

#if defined(NAND_DISK)
    case FS_NAND :
    {
        //ustatus = NAND_ReadMultiSectors(buff, sector, 512, count);
        ustatus = FTL_ReadSectors(buff,sector,512,count);
        if(ustatus == 0)
            return RES_OK;
        else
            return RES_ERROR;
    }

#elif  defined(FS_SD_2ND)
    case FS_SD_2ND :
    {
        if (count == 1)
        {
            Status = SD_ReadBlock_2nd(buff, sector << 9 , SECTOR_SIZE);
        } else
        {
            Status = SD_ReadMultiBlocks_2nd(buff, sector << 9 , SECTOR_SIZE, count);
        }
        if (Status != SD_OK)
        {
            return RES_ERROR;
        }

        return RES_OK;
     }
#endif

    case FS_USB :
		{
        if (pdrv || !count) return RES_PARERR;
        if (Stat & STA_NOINIT) return RES_NOTRDY;

        if (HCD_IsDeviceConnected(&USB_OTG_Core))
        {

            do
            {
                ustatus = USBH_MSC_Read10(&USB_OTG_Core, buff,sector,512 * count);
                USBH_MSC_HandleBOTXfer(&USB_OTG_Core ,&USB_Host);

                if (!HCD_IsDeviceConnected(&USB_OTG_Core))
                {
                    return RES_ERROR;
                }
            }
            while (ustatus == USBH_MSC_BUSY );
        }

        if (ustatus == USBH_MSC_OK)
            return RES_OK;
		}
		break;
    }
    return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
#if _USE_WRITE
DRESULT disk_write (
                   BYTE pdrv,          /* Physical drive nmuber (0..) */
                   const BYTE *buff,   /* Data to be written */
                   DWORD sector,       /* Sector address (LBA) */
                   BYTE count          /* Number of sectors to write (1..128) */
                   )
{
    DRESULT res;

		BYTE ustatus = USBH_MSC_OK;
	  SD_Error Status = SD_OK;

    switch (pdrv)
    {
    case FS_SD :
    {
        if (count == 1)
        {
            Status = SD_WriteBlock((uint8_t *)buff, sector << 9 ,SECTOR_SIZE);
            if (Status != SD_OK)
            {
                return RES_ERROR;
            }
            return RES_OK;
        }else
        {
            /* 此处存在疑问： 扇区个数如果写 count ，将导致最后1个block无法写入 */
#if 1 //ouhs 20170608
            Status = SD_WriteMultiBlocks((uint8_t *)buff, sector << 9 ,SECTOR_SIZE, count);
#else
            Status = SD_WriteMultiBlocks((uint8_t *)buff, sector << 9 ,SECTOR_SIZE, count + 1);
#endif
            if (Status != SD_OK)
            {
                return RES_ERROR;
            }
            return RES_OK;
        }
     }
#if defined(NAND_DISK)
	case FS_NAND:
        //ustatus =NAND_WriteMultiSectors((uint8_t *)buff, sector, 512, count);
        ustatus =FTL_WriteSectors((u8*)buff,sector,512,count);
        if(ustatus == 0)
            return RES_OK;
        else
            return RES_ERROR;

#elif  defined(FS_SD_2ND)
	case FS_SD_2ND :
    {
        if (count == 1)
        {
            Status = SD_WriteBlock_2nd((uint8_t *)buff, sector << 9 ,SECTOR_SIZE);
            if (Status != SD_OK)
            {
                return RES_ERROR;
            }
            return RES_OK;
        }else
        {
            /* 此处存在疑问： 扇区个数如果写 count ，将导致最后1个block无法写入 */
#if 1 //ouhs 20170608
            Status = SD_WriteMultiBlocks_2nd((uint8_t *)buff, sector << 9 ,SECTOR_SIZE, count);
#else
            Status = SD_WriteMultiBlocks_2nd((uint8_t *)buff, sector << 9 ,SECTOR_SIZE, count + 1);
#endif
            if (Status != SD_OK)
            {
                return RES_ERROR;
            }
            return RES_OK;
        }
     }
#endif
    case FS_USB :
		{
        if (pdrv || !count) return RES_PARERR;
        if (Stat & STA_NOINIT) return RES_NOTRDY;
        if (Stat & STA_PROTECT) return RES_WRPRT;


        if (HCD_IsDeviceConnected(&USB_OTG_Core))
        {
            do
            {
                ustatus = USBH_MSC_Write10(&USB_OTG_Core,(BYTE*)buff,sector,512 * count);
                USBH_MSC_HandleBOTXfer(&USB_OTG_Core, &USB_Host);

                if (!HCD_IsDeviceConnected(&USB_OTG_Core))
                {
                    return RES_ERROR;
                }
            }

            while (ustatus == USBH_MSC_BUSY );

        }

        if (ustatus == USBH_MSC_OK)
            return RES_OK;
		}
		break;
    }
    return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
                   BYTE pdrv,      /* Physical drive nmuber (0..) */
                   BYTE cmd,       /* Control code */
                   void *buff      /* Buffer to send/receive control data */
                   )
{
    DRESULT res = RES_OK;

    switch (pdrv)
    {
    case FS_SD :
				switch(cmd)
				{
				case CTRL_SYNC:
						break;

				case GET_BLOCK_SIZE:
					//if (CardType & 1 )
					//{/* SDC ver 1.XX */
					//}
					//else
					//{/* SDC ver 2.00 or MMC */
					//}
					break;

				case GET_SECTOR_COUNT:
					break;

				case GET_SECTOR_SIZE:
					*(WORD*)buff = SECTOR_SIZE;
					res = RES_OK;
					break;

				default:
					res = RES_PARERR;
					break;
				}
			return res;

#if defined(NAND_DISK)
		case FS_NAND :
			switch(cmd){
			case CTRL_SYNC:
				res = RES_OK;
				break;

			case GET_BLOCK_SIZE:
				*(DWORD*)buff = nand_dev.page_mainsize/512;//block大小,定义成一个page的大小
				res = RES_OK;
				break;

			case GET_SECTOR_COUNT:
				*(DWORD*)buff = nand_dev.valid_blocknum*nand_dev.block_pagenum*nand_dev.page_mainsize/512;//NAND FLASH的总扇区大小
				res = RES_OK;
				break;

			case GET_SECTOR_SIZE:
				*(WORD*)buff = 512;	//NAND FLASH扇区强制为512字节大小
				res = RES_OK;
				break;

			default:
				res = RES_PARERR;
				break;
			}

			return res;

#elif  defined(FS_SD_2ND)
        case FS_SD_2ND :
				switch(cmd)
				{
				case CTRL_SYNC:
						break;

				case GET_BLOCK_SIZE:
					//if (CardType & 1 )
					//{/* SDC ver 1.XX */
					//}
					//else
					//{/* SDC ver 2.00 or MMC */
					//}
					break;

				case GET_SECTOR_COUNT:
					break;

				case GET_SECTOR_SIZE:
					*(WORD*)buff = SECTOR_SIZE;
					res = RES_OK;
					break;

				default:
					res = RES_PARERR;
					break;
				}
			return res;
#endif

    case FS_USB :
				switch (cmd) {
				case CTRL_SYNC :		/* Make sure that no pending write process */
					res = RES_OK;
					break;

				case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
					*(DWORD*)buff = (DWORD) USBH_MSC_Param.MSCapacity;
					res = RES_OK;
					break;

				case GET_SECTOR_SIZE :	/* Get R/W sector size (WORD) */
					*(WORD*)buff = 512;
					res = RES_OK;
					break;

				case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
					*(DWORD*)buff = 512;
					res = RES_OK;
					break;

				default:
					res = RES_PARERR;
				}
        return res;

    }
    return RES_PARERR;
}
#endif

/*
*********************************************************************************************************
*	函 数 名: get_fattime
*	功能说明: 获得系统时间，用于改写文件的创建和修改时间。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
#if 0
DWORD get_fattime (void)
{
    /* 如果有全局时钟，可按下面的格式进行时钟转换. 这个例子是2013-01-01 00:00:00 */

    return((DWORD)(2013 - 1980) << 25)  /* Year = 2013 */
    | ((DWORD)1 << 21)              /* Month = 1 */
    | ((DWORD)1 << 16)              /* Day_m = 1*/
    | ((DWORD)0 << 11)              /* Hour = 0 */
    | ((DWORD)0 << 5)               /* Min = 0 */
    | ((DWORD)0 >> 1);              /* Sec = 0 */
}
#endif

#if _USE_LFN == 3

//动态分配内存
void *ff_memalloc (UINT size)
{
	return (void*)mymalloc(SRAMIN,size);
}
//释放内存
void ff_memfree (void* mf)
{
	myfree(SRAMIN,mf);
}

#endif

