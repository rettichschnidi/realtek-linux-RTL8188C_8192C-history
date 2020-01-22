/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *                                        
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 
******************************************************************************/
#define _HCI_INTF_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <recv_osdep.h>
#include <xmit_osdep.h>
#include <hal_init.h>
#include <rtw_version.h>

#ifndef CONFIG_USB_HCI

#error "CONFIG_USB_HCI shall be on!\n"

#endif

#include <usb_vendor_req.h>
#include <usb_ops.h>
#include <usb_osintf.h>
#include <usb_hal.h>
#ifdef CONFIG_PLATFORM_RTK_DMP
#include <asm/io.h>
#endif

#if defined (PLATFORM_LINUX) && defined (PLATFORM_WINDOWS)

#error "Shall be Linux or Windows, but not both!\n"

#endif

#ifdef CONFIG_80211N_HT
extern int ht_enable;
extern int cbw40_enable;
extern int ampdu_enable;//for enable tx_ampdu
#endif

extern char* initmac;

static struct usb_interface *pintf;


extern int pm_netdev_open(struct net_device *pnetdev,u8 bnormal);
static int rtw_suspend(struct usb_interface *intf, pm_message_t message);
static int rtw_resume(struct usb_interface *intf);


static int rtw_drv_init(struct usb_interface *pusb_intf,const struct usb_device_id *pdid);
static void rtw_dev_remove(struct usb_interface *pusb_intf);

#define USB_VENDER_ID_REALTEK		0x0BDA

static struct usb_device_id rtw_usb_id_tbl[] ={
#ifdef CONFIG_RTL8192C
	//DID_USB_V60_20110425
	/*=== Realtek demoboard ===*/		
	{USB_DEVICE(0x0BDA, 0x8191)},//Default ID
	
	/****** 8188CUS ********/
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8176)},//8188cu 1*1 dongole 
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8170)},//8188CE-VAU USB minCard
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x817A)},//8188cu Slim Solo
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x817B)},//8188cu Slim Combo	
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x817D)},//8188RU High-power USB Dongle
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8754)},//8188 Combo for BC4
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x817F)},//8188RU
	
	/****** 8192CUS ********/
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8177)},//8191cu 1*2
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8178)},//8192cu 2*2
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x817C)},//8192CE-VAU USB minCard
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8186)},//8192CE-VAU USB minCard

	/*=== Customer ID ===*/	
	/****** 8188CUS Dongle********/
	{USB_DEVICE(0x07B8, 0x8189)},//Funai - Abocom
	{USB_DEVICE(0x2019, 0xED17)},//PCI - Edimax        
	{USB_DEVICE(0x0DF6, 0x0052)}, //Sitecom - Edimax    
	{USB_DEVICE(0x7392, 0x7811)},//Edimax - Edimax  
	{USB_DEVICE(0x07B8, 0x8189)},//Abocom - Abocom 
	{USB_DEVICE(0x0EB0, 0x9071)},//NO Brand - Etop  
	{USB_DEVICE(0x06F8, 0xE033)},//Hercules - Edimax 
	{USB_DEVICE(0x103C, 0x1629)},//HP - Lite-On ,8188CUS Slim Combo
	{USB_DEVICE(0x2001, 0x3308)},//D-Link - Alpha
	{USB_DEVICE(0x050D, 0x1102)},//Belkin - Edimax
	{USB_DEVICE(0x2019, 0xAB2A)},//Planex - Abocom
	{USB_DEVICE(0x20F4, 0x648B)},//TRENDnet - Cameo
	{USB_DEVICE(0x4855, 0x0090)},// 	- Feixun
	{USB_DEVICE(0x13D3, 0x3357)},	// -AzureWave
	{USB_DEVICE(0x0DF6, 0x005C)},//Sitecom-Edimax
	{USB_DEVICE(0x0BDA, 0x5088)},//Thinkware-CC&C
	{USB_DEVICE(0x4856, 0x0091)},//NetweeN-Feixun
	{USB_DEVICE(0x9846, 0x9041)},//Netgear,Cameo
	{USB_DEVICE(0x0846, 0x9041)},//Netgear,Cameo
	{USB_DEVICE(0x2019, 0x4902)},//Planex,Etop
	
	/****** 8188CE-VAU********/
	{USB_DEVICE(0x13D3, 0x3358)},// -Azwave 8188CE-VAU

	/****** 8188CUS Slim Solo********/
	{USB_DEVICE(0x04F2, 0xAFF7)},//XAVI-XAVI
	{USB_DEVICE(0x04F2, 0xAFF9)},//XAVI-XAVI
	{USB_DEVICE(0x04F2, 0xAFFA)},//XAVI-XAVI

	/****** 8188CUS Slim Combo ********/
	{USB_DEVICE(0x04F2, 0xAFF8)},//XAVI-XAVI
	{USB_DEVICE(0x04F2, 0xAFFB)},//XAVI-XAVI
	{USB_DEVICE(0x04F2, 0xAFFC)},//XAVI-XAVI
	
	
	/****** 8192CUS Dongle********/	
	{USB_DEVICE(0x07b8, 0x8178)},//Funai -Abocom
	{USB_DEVICE(0x2001, 0x3307)},//D-Link-Cameo   
	{USB_DEVICE(0x2001, 0x330A)},//D-Link-Alpha   
	{USB_DEVICE(0x2001, 0x3309)},//D-Link-Alpha   	
	{USB_DEVICE(0x0586, 0x341F)},//Zyxel -Abocom
	{USB_DEVICE(0x7392, 0x7822)},//Edimax -Edimax	
	{USB_DEVICE(0x2019, 0xAB2B)},//Planex -Abocom
	{USB_DEVICE(0x07B8, 0x8178)},//Abocom -Abocom	
	{USB_DEVICE(0x07AA, 0x0056)},//ATKK-Gemtek	
	{USB_DEVICE(0x4855, 0x0091)},// 	-Feixun
	{USB_DEVICE(0x050D, 0x2102)},//Belkin-Sercomm
	{USB_DEVICE(0x2001, 0x3307)},//D-Link-Cameo	
	{USB_DEVICE(0x050D, 0x2102)},//Belkin-Sercomm
	{USB_DEVICE(0x050D, 0x2103)},//Belkin-Edimax
	{USB_DEVICE(0x20F4, 0x624D)},//TRENDnet
#endif
#ifdef CONFIG_RTL8192D
	//92DU
	// Realtek */
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8193)},
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8194)},

	/*=== Customer ID ===*/	
	{USB_DEVICE(0x2019, 0xAB2C)},//PCI-Abocm
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8111)}, // PCI-Abocom 5G dongle for WiFi Display 
#endif
	{}	/* Terminating entry */
};

int const rtw_usb_id_len = sizeof(rtw_usb_id_tbl) / sizeof(struct usb_device_id);

static struct specific_device_id specific_device_id_tbl[] = {
	{.idVendor=USB_VENDER_ID_REALTEK, .idProduct=0x8177, .flags=SPEC_DEV_ID_DISABLE_HT},//8188cu 1*1 dongole, (b/g mode only)
	{.idVendor=USB_VENDER_ID_REALTEK, .idProduct=0x817E, .flags=SPEC_DEV_ID_DISABLE_HT},//8188CE-VAU USB minCard (b/g mode only)
	{.idVendor=0x0b05, .idProduct=0x1791, .flags=SPEC_DEV_ID_DISABLE_HT},
	{.idVendor=0x13D3, .idProduct=0x3311, .flags=SPEC_DEV_ID_DISABLE_HT},
	{.idVendor=0x13D3, .idProduct=0x3359, .flags=SPEC_DEV_ID_DISABLE_HT},//Russian customer -Azwave (8188CE-VAU  g mode)	
#ifdef RTK_DMP_PLATFORM
	{.idVendor=USB_VENDER_ID_REALTEK, .idProduct=0x8111, .flags=SPEC_DEV_ID_ASSIGN_IFNAME}, // PCI-Abocom 5G dongle for WiFi Display
#endif /* RTK_DMP_PLATFORM */
	{}
};

typedef struct _driver_priv{

	struct usb_driver rtw_usb_drv;
	int drv_registered;

}drv_priv, *pdrv_priv;


static drv_priv drvpriv = {
	.rtw_usb_drv.name = (char*)DRV_NAME,
	.rtw_usb_drv.probe = rtw_drv_init,
	.rtw_usb_drv.disconnect = rtw_dev_remove,
	.rtw_usb_drv.id_table = rtw_usb_id_tbl,
	.rtw_usb_drv.suspend =  rtw_suspend,
	.rtw_usb_drv.resume = rtw_resume,
#ifdef CONFIG_AUTOSUSPEND	
	.rtw_usb_drv.supports_autosuspend = 1,	
#endif
};

MODULE_DEVICE_TABLE(usb, rtw_usb_id_tbl);


static inline int RT_usb_endpoint_dir_in(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN);
}

static inline int RT_usb_endpoint_dir_out(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT);
}

static inline int RT_usb_endpoint_xfer_int(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT);
}

static inline int RT_usb_endpoint_xfer_bulk(const struct usb_endpoint_descriptor *epd)
{
 	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK);
}

static inline int RT_usb_endpoint_is_bulk_in(const struct usb_endpoint_descriptor *epd)
{
	return (RT_usb_endpoint_xfer_bulk(epd) && RT_usb_endpoint_dir_in(epd));
}

static inline int RT_usb_endpoint_is_bulk_out(const struct usb_endpoint_descriptor *epd)
{
	return (RT_usb_endpoint_xfer_bulk(epd) && RT_usb_endpoint_dir_out(epd));
}

static inline int RT_usb_endpoint_is_int_in(const struct usb_endpoint_descriptor *epd)
{
	return (RT_usb_endpoint_xfer_int(epd) && RT_usb_endpoint_dir_in(epd));
}

static inline int RT_usb_endpoint_num(const struct usb_endpoint_descriptor *epd)
{
	return epd->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
}

static u32 usb_dvobj_init(_adapter *padapter)
{
	int	i;
	u8	val8;
	int	status = _SUCCESS;
	struct usb_device_descriptor 	*pdev_desc;
	struct usb_host_config			*phost_conf;
	struct usb_config_descriptor		*pconf_desc;
	struct usb_host_interface		*phost_iface;
	struct usb_interface_descriptor	*piface_desc;
	struct usb_host_endpoint		*phost_endp;
	struct usb_endpoint_descriptor	*pendp_desc;
	struct dvobj_priv	*pdvobjpriv = &padapter->dvobjpriv;
	struct usb_device	*pusbd = pdvobjpriv->pusbdev;

_func_enter_;

	pdvobjpriv->padapter = padapter;

	pdvobjpriv->RtNumInPipes = 0;
	pdvobjpriv->RtNumOutPipes = 0;

	//padapter->EepromAddressSize = 6;
	//pdvobjpriv->nr_endpoint = 6;

	pdev_desc = &pusbd->descriptor;

#if 0
	printk("\n8712_usb_device_descriptor:\n");
	printk("bLength=%x\n", pdev_desc->bLength);
	printk("bDescriptorType=%x\n", pdev_desc->bDescriptorType);
	printk("bcdUSB=%x\n", pdev_desc->bcdUSB);
	printk("bDeviceClass=%x\n", pdev_desc->bDeviceClass);
	printk("bDeviceSubClass=%x\n", pdev_desc->bDeviceSubClass);
	printk("bDeviceProtocol=%x\n", pdev_desc->bDeviceProtocol);
	printk("bMaxPacketSize0=%x\n", pdev_desc->bMaxPacketSize0);
	printk("idVendor=%x\n", pdev_desc->idVendor);
	printk("idProduct=%x\n", pdev_desc->idProduct);
	printk("bcdDevice=%x\n", pdev_desc->bcdDevice);
	printk("iManufacturer=%x\n", pdev_desc->iManufacturer);
	printk("iProduct=%x\n", pdev_desc->iProduct);
	printk("iSerialNumber=%x\n", pdev_desc->iSerialNumber);
	printk("bNumConfigurations=%x\n", pdev_desc->bNumConfigurations);
#endif

	phost_conf = pusbd->actconfig;
	pconf_desc = &phost_conf->desc;

#if 0
	printk("\n8712_usb_configuration_descriptor:\n");
	printk("bLength=%x\n", pconf_desc->bLength);
	printk("bDescriptorType=%x\n", pconf_desc->bDescriptorType);
	printk("wTotalLength=%x\n", pconf_desc->wTotalLength);
	printk("bNumInterfaces=%x\n", pconf_desc->bNumInterfaces);
	printk("bConfigurationValue=%x\n", pconf_desc->bConfigurationValue);
	printk("iConfiguration=%x\n", pconf_desc->iConfiguration);
	printk("bmAttributes=%x\n", pconf_desc->bmAttributes);
	printk("bMaxPower=%x\n", pconf_desc->bMaxPower);
#endif

	//printk("\n/****** num of altsetting = (%d) ******/\n", pintf->num_altsetting);

	phost_iface = &pintf->altsetting[0];
	piface_desc = &phost_iface->desc;

#if 0
	printk("\n8712_usb_interface_descriptor:\n");
	printk("bLength=%x\n", piface_desc->bLength);
	printk("bDescriptorType=%x\n", piface_desc->bDescriptorType);
	printk("bInterfaceNumber=%x\n", piface_desc->bInterfaceNumber);
	printk("bAlternateSetting=%x\n", piface_desc->bAlternateSetting);
	printk("bNumEndpoints=%x\n", piface_desc->bNumEndpoints);
	printk("bInterfaceClass=%x\n", piface_desc->bInterfaceClass);
	printk("bInterfaceSubClass=%x\n", piface_desc->bInterfaceSubClass);
	printk("bInterfaceProtocol=%x\n", piface_desc->bInterfaceProtocol);
	printk("iInterface=%x\n", piface_desc->iInterface);
#endif

	pdvobjpriv->NumInterfaces = pconf_desc->bNumInterfaces;
	pdvobjpriv->InterfaceNumber = piface_desc->bInterfaceNumber;
	pdvobjpriv->nr_endpoint = piface_desc->bNumEndpoints;

	//printk("\ndump usb_endpoint_descriptor:\n");

	for (i = 0; i < pdvobjpriv->nr_endpoint; i++)
	{
		phost_endp = phost_iface->endpoint + i;
		if (phost_endp)
		{
			pendp_desc = &phost_endp->desc;

			printk("\nusb_endpoint_descriptor(%d):\n", i);
			printk("bLength=%x\n",pendp_desc->bLength);
			printk("bDescriptorType=%x\n",pendp_desc->bDescriptorType);
			printk("bEndpointAddress=%x\n",pendp_desc->bEndpointAddress);
			//printk("bmAttributes=%x\n",pendp_desc->bmAttributes);
			//printk("wMaxPacketSize=%x\n",pendp_desc->wMaxPacketSize);
			printk("wMaxPacketSize=%x\n",le16_to_cpu(pendp_desc->wMaxPacketSize));
			printk("bInterval=%x\n",pendp_desc->bInterval);
			//printk("bRefresh=%x\n",pendp_desc->bRefresh);
			//printk("bSynchAddress=%x\n",pendp_desc->bSynchAddress);

			if (RT_usb_endpoint_is_bulk_in(pendp_desc))
			{
				printk("RT_usb_endpoint_is_bulk_in = %x\n", RT_usb_endpoint_num(pendp_desc));
				pdvobjpriv->RtNumInPipes++;
			}
			else if (RT_usb_endpoint_is_int_in(pendp_desc))
			{
				printk("RT_usb_endpoint_is_int_in = %x, Interval = %x\n", RT_usb_endpoint_num(pendp_desc),pendp_desc->bInterval);
				pdvobjpriv->RtNumInPipes++;
			}
			else if (RT_usb_endpoint_is_bulk_out(pendp_desc))
			{
				printk("RT_usb_endpoint_is_bulk_out = %x\n", RT_usb_endpoint_num(pendp_desc));
				pdvobjpriv->RtNumOutPipes++;
			}
			pdvobjpriv->ep_num[i] = RT_usb_endpoint_num(pendp_desc);
		}
	}
	
	printk("nr_endpoint=%d, in_num=%d, out_num=%d\n\n", pdvobjpriv->nr_endpoint, pdvobjpriv->RtNumInPipes, pdvobjpriv->RtNumOutPipes);

	if (pusbd->speed == USB_SPEED_HIGH)
	{
		pdvobjpriv->ishighspeed = _TRUE;
		printk("USB_SPEED_HIGH\n");
	}
	else
	{
		pdvobjpriv->ishighspeed = _FALSE;
		printk("NON USB_SPEED_HIGH\n");
	}

	//.2
	if ((init_io_priv(padapter)) == _FAIL)
	{
		RT_TRACE(_module_hci_intfs_c_,_drv_err_,(" \n Can't init io_reqs\n"));
		status = _FAIL;
	}

	//.3 misc
	_init_sema(&(padapter->dvobjpriv.usb_suspend_sema), 0);	

	intf_read_chip_version(padapter);

	//.4 usb endpoint mapping
	intf_chip_configure(padapter);

_func_exit_;

	return status;
}

static void usb_dvobj_deinit(_adapter * padapter){

	struct dvobj_priv *pdvobjpriv=&padapter->dvobjpriv;

	_func_enter_;


	_func_exit_;
}

static void decide_chip_type_by_usb_device_id(_adapter *padapter, const struct usb_device_id *pdid)
{
	//u32	i;
	//u16	vid, pid;

	padapter->chip_type = NULL_CHIP_TYPE;

	//vid = pdid->idVendor;
	//pid = pdid->idProduct;

	//TODO: dynamic judge 92c or 92d according to usb vid and pid.
#ifdef CONFIG_RTL8192C
	padapter->chip_type = RTL8188C_8192C;
	padapter->HardwareType = HARDWARE_TYPE_RTL8192CU;
	DBG_8192C("CHIP TYPE: RTL8188C_8192C\n");
#endif

#ifdef CONFIG_RTL8192D
	padapter->chip_type = RTL8192D;
	padapter->HardwareType = HARDWARE_TYPE_RTL8192DU;
	DBG_8192C("CHIP TYPE: RTL8192D\n");
#endif

}

static void usb_intf_start(_adapter *padapter)
{

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("+usb_intf_start\n"));

	if(padapter->HalFunc.inirp_init == NULL)
	{
		RT_TRACE(_module_os_intfs_c_,_drv_err_,("Initialize dvobjpriv.inirp_init error!!!\n"));
	}
	else
	{	
		padapter->HalFunc.inirp_init(padapter);
	}			

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("-usb_intf_start\n"));

}

static void usb_intf_stop(_adapter *padapter)
{

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("+usb_intf_stop\n"));

	//disabel_hw_interrupt
	if(padapter->bSurpriseRemoved == _FALSE)
	{
		//device still exists, so driver can do i/o operation
		//TODO:
		RT_TRACE(_module_hci_intfs_c_,_drv_err_,("SurpriseRemoved==_FALSE\n"));
	}

	//cancel in irp
	if(padapter->HalFunc.inirp_deinit !=NULL)
	{
		padapter->HalFunc.inirp_deinit(padapter);
	}

	//cancel out irp
	write_port_cancel(padapter);

	//todo:cancel other irps

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("-usb_intf_stop\n"));

}

static void rtw_dev_unload(_adapter *padapter)
{
	struct net_device *pnetdev= (struct net_device*)padapter->pnetdev;
	u8 val8;
	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("+rtw_dev_unload\n"));

	if(padapter->bup == _TRUE)
	{
		printk("===> rtw_dev_unload\n");

		padapter->bDriverStopped = _TRUE;
		val8 = 0xFF;
		padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_TXPAUSE,&val8);
		//s3.
		if(padapter->intf_stop)
		{
			padapter->intf_stop(padapter);
		}

		//s4.
		if(!padapter->pwrctrlpriv.bInternalAutoSuspend )			
		stop_drv_threads(padapter);


		//s5.
		if(padapter->bSurpriseRemoved == _FALSE)
		{
			//printk("r871x_dev_unload()->rtl871x_hal_deinit()\n");
			#ifdef CONFIG_WOWLAN
			if(padapter->pwrctrlpriv.bSupportWakeOnWlan==_TRUE){
				printk("%s bSupportWakeOnWlan==_TRUE  do not run rtw_hal_deinit()\n",__FUNCTION__);
			}
			else
			#endif
			{
				rtw_hal_deinit(padapter);
			}
			padapter->bSurpriseRemoved = _TRUE;
		}

		//s6.
		if(padapter->dvobj_deinit)
		{
			padapter->dvobj_deinit(padapter);
		}
		else
		{
			RT_TRACE(_module_hci_intfs_c_,_drv_err_,("Initialize hcipriv.hci_priv_init error!!!\n"));
		}

		padapter->bup = _FALSE;

	}
	else
	{
		RT_TRACE(_module_hci_intfs_c_,_drv_err_,("r871x_dev_unload():padapter->bup == _FALSE\n" ));
	}

	printk("<=== rtw_dev_unload\n");

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("-rtw_dev_unload\n"));

}

static void process_spec_devid(const struct usb_device_id *pdid)
{
	u16 vid, pid;
	u32 flags;
	int i;
	int num = sizeof(specific_device_id_tbl)/sizeof(struct specific_device_id);

	for(i=0; i<num; i++)
	{
		vid = specific_device_id_tbl[i].idVendor;
		pid = specific_device_id_tbl[i].idProduct;
		flags = specific_device_id_tbl[i].flags;

#ifdef CONFIG_80211N_HT
		if((pdid->idVendor==vid) && (pdid->idProduct==pid) && (flags&SPEC_DEV_ID_DISABLE_HT))
		{
			 ht_enable = 0;
			 cbw40_enable = 0;
			 ampdu_enable = 0;
		}
#endif

#ifdef RTK_DMP_PLATFORM
		// Change the ifname to wlan10 when PC side WFD dongle plugin on DMP platform.
		// It is used to distinguish between normal and PC-side wifi dongle/module.
		if((pdid->idVendor==vid) && (pdid->idProduct==pid) && (flags&SPEC_DEV_ID_ASSIGN_IFNAME))
		{
			extern char* ifname;
			strncpy(ifname, "wlan10", 6); 
			//printk("%s()-%d: ifname=%s, vid=%04X, pid=%04X\n", __FUNCTION__, __LINE__, ifname, vid, pid);
		}
#endif /* RTK_DMP_PLATFORM */

	}
}

#ifdef SUPPORT_HW_RFOFF_DETECTED
extern u8 disconnect_hdl(_adapter *padapter, u8 *pbuf);
extern void os_indicate_disconnect( _adapter *adapter );

int rtw_hw_suspend(_adapter *padapter )
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct usb_interface *pusb_intf = padapter->dvobjpriv.pusbintf;	
	struct net_device *pnetdev=usb_get_intfdata(pusb_intf);
	
	_func_enter_;

	if((!padapter->bup) || (padapter->bDriverStopped)||(padapter->bSurpriseRemoved))
	{
		printk("padapter->bup=%d bDriverStopped=%d bSurpriseRemoved = %d\n",
			padapter->bup, padapter->bDriverStopped,padapter->bSurpriseRemoved);		
		goto error_exit;
	}
	
	if(padapter)//system suspend
	{		
		LeaveAllPowerSaveMode(padapter);
		
		printk("==> rtw_hw_suspend\n");	
		_enter_pwrlock(&pwrpriv->lock);
		pwrpriv->bips_processing = _TRUE;
		padapter->net_closed = _TRUE;
		//s1.
		if(pnetdev)
		{
			netif_carrier_off(pnetdev);
			netif_stop_queue(pnetdev);
		}

		//s2.
		//s2-1.  issue rtw_disassoc_cmd to fw
		//rtw_disassoc_cmd(padapter);//donnot enqueue cmd
		disconnect_hdl(padapter, NULL);
		
		//s2-2.  indicate disconnect to os
		//rtw_indicate_disconnect(padapter);
		{
			struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;		

			if((pmlmepriv->fw_state & _FW_LINKED))
			{
			        pmlmepriv->fw_state ^= _FW_LINKED;

				padapter->ledpriv.LedControlHandler(padapter, LED_CTL_NO_LINK);

				os_indicate_disconnect(padapter);
				
#ifdef CONFIG_LPS
				//donnot enqueue cmd
				lps_ctrl_wk_cmd(padapter, LPS_CTRL_DISCONNECT, 0);
#endif
			}

		}
		//s2-3.
		free_assoc_resources(padapter);

		//s2-4.
		free_network_queue(padapter,_TRUE);

		ips_dev_unload(padapter);			

		pwrpriv->current_rfpwrstate = rf_off;
		pwrpriv->bips_processing = _FALSE;		

		_exit_pwrlock(&pwrpriv->lock);
	}
	else
		goto error_exit;
	
	_func_exit_;
	return 0;
	
error_exit:
	printk("%s, failed \n",__FUNCTION__);
	return (-1);

}

int rtw_hw_resume(_adapter *padapter)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct usb_interface *pusb_intf = padapter->dvobjpriv.pusbintf;
	struct net_device *pnetdev=usb_get_intfdata(pusb_intf);	

	_func_enter_;

	if(padapter)//system resume
	{	
		printk("==> rtw_hw_resume\n");
		_enter_pwrlock(&pwrpriv->lock);
		pwrpriv->bips_processing = _TRUE;
		reset_drv_sw(padapter);
	
		if(pm_netdev_open(pnetdev,_FALSE) != 0)
		{
			_exit_pwrlock(&pwrpriv->lock);
			goto error_exit;
		}

		netif_device_attach(pnetdev);	
		netif_carrier_on(pnetdev);

		if(!netif_queue_stopped(pnetdev))
      			netif_start_queue(pnetdev);
		else
			netif_wake_queue(pnetdev);
		
		pwrpriv->bkeepfwalive = _FALSE;
		pwrpriv->brfoffbyhw = _FALSE;
		
		pwrpriv->current_rfpwrstate = rf_on;
		pwrpriv->bips_processing = _FALSE;	
	
		_exit_pwrlock(&pwrpriv->lock);
	}
	else
	{
		goto error_exit;	
	}

	_func_exit_;
	
	return 0;
error_exit:
	printk("%s, Open net dev failed \n",__FUNCTION__);
	return (-1);
}
#endif

static int rtw_suspend(struct usb_interface *pusb_intf, pm_message_t message)
{
	struct net_device *pnetdev=usb_get_intfdata(pusb_intf);
	_adapter *padapter = (_adapter*)netdev_priv(pnetdev);
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct usb_device *usb_dev = interface_to_usbdev(pusb_intf);
	
	_func_enter_;

	if((!padapter->bup) || (padapter->bDriverStopped)||(padapter->bSurpriseRemoved))
	{
		printk("padapter->bup=%d bDriverStopped=%d bSurpriseRemoved = %d\n",
			padapter->bup, padapter->bDriverStopped,padapter->bSurpriseRemoved);		
		//goto error_exit;
		return 0;
	}
	

	printk("###########  rtw_suspend  #################\n");
	
	if(padapter)//system suspend
	{
		LeaveAllPowerSaveMode(padapter);

		_enter_pwrlock(&pwrpriv->lock);
		padapter->net_closed = _TRUE;
		//s1.
		if(pnetdev)
		{
			netif_carrier_off(pnetdev);
			netif_stop_queue(pnetdev);
		}
#ifdef CONFIG_WOWLAN
		padapter->pwrctrlpriv.bSupportWakeOnWlan=_TRUE;
#else		
		//s2.
		//s2-1.  issue rtw_disassoc_cmd to fw
		disassoc_cmd(padapter);
#endif
		//s2-2.  indicate disconnect to os
		indicate_disconnect(padapter);
		//s2-3.
		free_assoc_resources(padapter);
#ifdef CONFIG_AUTOSUSPEND
		if(!pwrpriv->bInternalAutoSuspend )
#endif
		//s2-4.
		free_network_queue(padapter, _TRUE);

		rtw_dev_unload(padapter);
#ifdef CONFIG_AUTOSUSPEND
		pwrpriv->current_rfpwrstate = rf_off;
		pwrpriv->bips_processing = _FALSE;
#endif
		_exit_pwrlock(&pwrpriv->lock);
	}
	else
		goto error_exit;
	
	_func_exit_;
	return 0;
	
error_exit:
	printk("%s, failed \n",__FUNCTION__);
	return (-1);

}

static int rtw_resume(struct usb_interface *pusb_intf)
{
	struct net_device *pnetdev=usb_get_intfdata(pusb_intf);
	_adapter *padapter = (_adapter*)netdev_priv(pnetdev);
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct usb_device *usb_dev = interface_to_usbdev(pusb_intf);
	
	_func_enter_;

	printk("###########  rtw_resume  #################\n");
	printk("bkeepfwalive(%x)\n",pwrpriv->bkeepfwalive);
	
	if(padapter)//system resume
	{
		_enter_pwrlock(&pwrpriv->lock);
		reset_drv_sw(padapter);

		if(pm_netdev_open(pnetdev,_TRUE) != 0)
			goto error_exit;

		netif_device_attach(pnetdev);	
		netif_carrier_on(pnetdev);
#ifdef CONFIG_AUTOSUSPEND
		if(pwrpriv->bInternalAutoSuspend )
		{			
			pwrpriv->current_rfpwrstate = rf_on;	
			pwrpriv->bkeepfwalive = _FALSE;
			pwrpriv->bInternalAutoSuspend = _FALSE;
			pwrpriv->brfoffbyhw = _FALSE;
			{
				printk("enc_algorithm(%x),wepkeymask(%x)\n",
					padapter->securitypriv.dot11PrivacyAlgrthm,pwrpriv->wepkeymask);
				if(	(_WEP40_ == padapter->securitypriv.dot11PrivacyAlgrthm) ||
					(_WEP104_ == padapter->securitypriv.dot11PrivacyAlgrthm))
				{
					sint keyid;	
			
					for(keyid=0;keyid<4;keyid++){				
						if(pwrpriv->wepkeymask & BIT(keyid))
							set_key(padapter,&padapter->securitypriv, keyid);	
					}
				}
			}
		}
#endif
		_exit_pwrlock(&pwrpriv->lock);
	}
	else
	{
		goto error_exit;
	}

	_func_exit_;
	
	return 0;
error_exit:
	printk("%s, Open net dev failed \n",__FUNCTION__);
	return (-1);
}

#ifdef CONFIG_AUTOSUSPEND
void autosuspend_enter(_adapter* padapter)	
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	pwrpriv->bInternalAutoSuspend = _TRUE;
	pwrpriv->bips_processing = _TRUE;	
	
	printk("==>autosuspend_enter...........\n");	
	
	if(rf_off == pwrpriv->change_rfpwrstate )
	{	
		#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
		usb_enable_autosuspend(padapter->dvobjpriv.pusbdev);
		#else
		padapter->dvobjpriv.pusbdev->autosuspend_disabled = 0;//autosuspend disabled by the user	
		#endif
	
		#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,33))
			usb_autopm_put_interface(padapter->dvobjpriv.pusbintf);	
		#elif (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,20))		
			usb_autopm_enable(padapter->dvobjpriv.pusbintf);
		#else
			usb_autosuspend_device(padapter->dvobjpriv.pusbdev, 1);
		#endif
	}
	#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,32))
	printk("...pm_usage_cnt(%d).....\n",atomic_read(&(padapter->dvobjpriv.pusbintf->pm_usage_cnt)));
	#else
	printk("...pm_usage_cnt(%d).....\n",padapter->dvobjpriv.pusbintf->pm_usage_cnt);
	#endif
	
}
int autoresume_enter(_adapter* padapter)
{
	int result = _SUCCESS;
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct security_priv* psecuritypriv=&(padapter->securitypriv);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	
	
	printk("====> autoresume_enter \n");
	
	if(rf_off == pwrpriv->current_rfpwrstate )
	{			
		#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,33))				
			if (usb_autopm_get_interface( padapter->dvobjpriv.pusbintf) < 0) 
			{
				printk( "can't get autopm: %d\n", result);
				result = _FAIL;
				goto error_exit;
			}			
		#elif (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,20))				
			usb_autopm_disable(padapter->dvobjpriv.pusbintf);
		#else
			usb_autoresume_device(padapter->dvobjpriv.pusbdev, 1);
		#endif

		#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,32))
		printk("...pm_usage_cnt(%d).....\n",atomic_read(&(padapter->dvobjpriv.pusbintf->pm_usage_cnt)));
		#else
		printk("...pm_usage_cnt(%d).....\n",padapter->dvobjpriv.pusbintf->pm_usage_cnt);
		#endif	
	}
	printk("<==== autoresume_enter \n");
error_exit:	

	return result;
}
#endif

u8 key_char2num(u8 ch)
{
    if((ch>='0')&&(ch<='9'))
        return ch - '0';
    else if ((ch>='a')&&(ch<='f'))
        return ch - 'a' + 10;
    else if ((ch>='A')&&(ch<='F'))
        return ch - 'A' + 10;
    else
	 return 0xff;
}

u8 key_2char2num(u8 hch, u8 lch)
{
    return ((key_char2num(hch) << 4) | key_char2num(lch));
}

/*
 * drv_init() - a device potentially for us
 *
 * notes: drv_init() is called when the bus driver has located a card for us to support.
 *        We accept the new device by returning 0.
*/
static int rtw_drv_init(struct usb_interface *pusb_intf, const struct usb_device_id *pdid)
{
	int i;
	u8 mac[ETH_ALEN];
	uint status;
	_adapter *padapter = NULL;
	struct dvobj_priv *pdvobjpriv;
	struct net_device *pnetdev;

	RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("+rtw_drv_init\n"));
	//printk("+rtw_drv_init\n");

	//2009.8.13, by Thomas
	// In this probe function, O.S. will provide the usb interface pointer to driver.
	// We have to increase the reference count of the usb device structure by using the usb_get_dev function.
	usb_get_dev(interface_to_usbdev(pusb_intf));

	pintf = pusb_intf;	

	//step 0.
	process_spec_devid(pdid);

	//step 1. set USB interface data
	// init data
	pnetdev = init_netdev();
	if (!pnetdev) 
		goto error;
	
	SET_NETDEV_DEV(pnetdev, &pusb_intf->dev);

	padapter = netdev_priv(pnetdev);
	pdvobjpriv = &padapter->dvobjpriv;
	pdvobjpriv->padapter = padapter;
	pdvobjpriv->pusbintf = pusb_intf ;
	pdvobjpriv->pusbdev = interface_to_usbdev(pusb_intf);

	// set data
	usb_set_intfdata(pusb_intf, pnetdev);

	//set interface_type to usb
	padapter->interface_type = RTW_USB;

	//step 1-1., decide the chip_type via vid/pid
	decide_chip_type_by_usb_device_id(padapter, pdid);

	//step 2.	
	if(padapter->chip_type == RTL8188C_8192C)
	{
#ifdef CONFIG_RTL8192C
		rtl8192cu_set_hal_ops(padapter);
#endif
	}
	else if(padapter->chip_type == RTL8192D)
	{
#ifdef CONFIG_RTL8192D
		rtl8192du_set_hal_ops(padapter);
#endif
	}
	else
	{
		DBG_8192C("Detect NULL_CHIP_TYPE\n");
		status = _FAIL;
		goto error;
	}

	padapter->dvobj_init=&usb_dvobj_init;
	padapter->dvobj_deinit=&usb_dvobj_deinit;
	padapter->intf_start=&usb_intf_start;
	padapter->intf_stop=&usb_intf_stop;

	//step 3.
	//initialize the dvobj_priv 		
	if (padapter->dvobj_init == NULL){
		RT_TRACE(_module_hci_intfs_c_,_drv_err_,("\n Initialize dvobjpriv.dvobj_init error!!!\n"));
		goto error;
	}

	status = padapter->dvobj_init(padapter);	
	if (status != _SUCCESS) {
		RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("initialize device object priv Failed!\n"));
		goto error;
	}


	//step 4.
	status = init_drv_sw(padapter);
	if(status ==_FAIL){
		RT_TRACE(_module_hci_intfs_c_,_drv_err_,("Initialize driver software resource Failed!\n"));
		goto error;
	}


	//step 5. read efuse/eeprom data and get mac_addr
	intf_read_chip_info(padapter);	
#ifdef CONFIG_PM
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18))
	if(padapter->pwrctrlpriv.bSupportRemoteWakeup)
	{
		pdvobjpriv->pusbdev->do_remote_wakeup=1;
		pusb_intf->needs_remote_wakeup = 1;		
		device_init_wakeup(&pusb_intf->dev, 1);
		printk("\n  padapter->pwrctrlpriv.bSupportRemoteWakeup~~~~~~\n");
		printk("\n  padapter->pwrctrlpriv.bSupportRemoteWakeup~~~[%d]~~~\n",device_may_wakeup(&pusb_intf->dev));
	}
#endif
#endif

#ifdef CONFIG_AUTOSUSPEND
	if( padapter->registrypriv.power_mgnt != PS_MODE_ACTIVE )
	{
		if(padapter->registrypriv.usbss_enable ){ 	/* autosuspend (2s delay) */
			#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,38))
			pdvobjpriv->pusbdev->dev.power.autosuspend_delay = 0 * HZ;//15 * HZ; idle-delay time		
			#else
			pdvobjpriv->pusbdev->autosuspend_delay = 0 * HZ;//15 * HZ; idle-delay time		 	
			#endif

			#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
			usb_enable_autosuspend(padapter->dvobjpriv.pusbdev);
			#elif  (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,22) && LINUX_VERSION_CODE<=KERNEL_VERSION(2,6,34))
			padapter->bDisableAutosuspend = padapter->dvobjpriv.pusbdev->autosuspend_disabled ;
			padapter->dvobjpriv.pusbdev->autosuspend_disabled = 0;//autosuspend disabled by the user
			#endif

			usb_autopm_get_interface(padapter->dvobjpriv.pusbintf );//init pm_usage_cnt ,let it start from 1

			#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,32))
			printk("%s...pm_usage_cnt(%d).....\n",__FUNCTION__,atomic_read(&(pdvobjpriv->pusbintf ->pm_usage_cnt)));
			#else
			printk("%s...pm_usage_cnt(%d).....\n",__FUNCTION__,pdvobjpriv->pusbintf ->pm_usage_cnt);
			#endif							
		}
	}	
#endif
	// alloc dev name after read efuse.
	init_netdev_name(pnetdev);

	if ( initmac )
	{	//	Users specify the mac address
		int jj,kk;

		for( jj = 0, kk = 0; jj < ETH_ALEN; jj++, kk += 3 )
		{
			mac[jj] = key_2char2num(initmac[kk], initmac[kk+ 1]);
		}
	}
	else
	{	//	Use the mac address stored in the Efuse
		_memcpy(mac, padapter->eeprompriv.mac_addr, ETH_ALEN);
	}

	if (((mac[0]==0xff) &&(mac[1]==0xff) && (mac[2]==0xff) &&
	     (mac[3]==0xff) && (mac[4]==0xff) &&(mac[5]==0xff)) ||
	    ((mac[0]==0x0) && (mac[1]==0x0) && (mac[2]==0x0) &&
	     (mac[3]==0x0) && (mac[4]==0x0) &&(mac[5]==0x0)))
	{
		mac[0] = 0x00;
		mac[1] = 0xe0;
		mac[2] = 0x4c;
		mac[3] = 0x87;
		mac[4] = 0x00;
		mac[5] = 0x00;
	}
	_memcpy(pnetdev->dev_addr, mac, ETH_ALEN);
	printk("MAC Address from efuse= %02x:%02x:%02x:%02x:%02x:%02x\n",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);


	//step 6.
	/* Tell the network stack we exist */
	if (register_netdev(pnetdev) != 0) {
		RT_TRACE(_module_hci_intfs_c_,_drv_err_,("register_netdev() failed\n"));
		goto error;
	}

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("-drv_init - Adapter->bDriverStopped=%d, Adapter->bSurpriseRemoved=%d\n",padapter->bDriverStopped, padapter->bSurpriseRemoved));
	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("-871x_drv - drv_init, success!\n"));
	//printk("-871x_drv - drv_init, success!\n");

#ifdef CONFIG_PROC_DEBUG
#ifdef RTK_DMP_PLATFORM
	rtw_proc_init_one(pnetdev);
#endif
#endif

#ifdef CONFIG_HOSTAPD_MLME
	hostapd_mode_init(padapter);
#endif

#ifdef CONFIG_PLATFORM_RTD2880B
	printk("wlan link up\n");
	rtd2885_wlan_netlink_sendMsg("linkup", "8712");
#endif

	return 0;

error:

	usb_put_dev(interface_to_usbdev(pusb_intf));//decrease the reference count of the usb device structure if driver fail on initialzation

	usb_set_intfdata(pusb_intf, NULL);

	usb_dvobj_deinit(padapter);
	
	if (pnetdev)
	{
		//unregister_netdev(pnetdev);
		free_netdev(pnetdev);
	}

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("-871x_usb - drv_init, fail!\n"));
	//printk("-871x_usb - drv_init, fail!\n");

	return -ENODEV;
}

/*
 * dev_remove() - our device is being removed
*/
//rmmod module & unplug(SurpriseRemoved) will call r871xu_dev_remove() => how to recognize both
static void rtw_dev_remove(struct usb_interface *pusb_intf)
{
	struct net_device *pnetdev=usb_get_intfdata(pusb_intf);
	_adapter *padapter = (_adapter*)netdev_priv(pnetdev);
	struct dvobj_priv *pdvobjpriv = &padapter->dvobjpriv;

_func_exit_;

	usb_set_intfdata(pusb_intf, NULL);

	if(padapter)
	{
		printk("+rtw_dev_remove\n");
		RT_TRACE(_module_hci_intfs_c_,_drv_err_,("+dev_remove()\n"));

#ifdef CONFIG_HOSTAPD_MLME
		hostapd_mode_unload(padapter);
#endif
		LeaveAllPowerSaveMode(padapter);
		if(drvpriv.drv_registered == _TRUE)
		{
			//printk("r871xu_dev_remove():padapter->bSurpriseRemoved == _TRUE\n");
			padapter->bSurpriseRemoved = _TRUE;
		}
		/*else
		{
			//printk("r871xu_dev_remove():module removed\n");
			padapter->hw_init_completed = _FALSE;
		}*/

		if(padapter->DriverState != DRIVER_DISAPPEAR)
		{
			if(pnetdev) {
				unregister_netdev(pnetdev); //will call netdev_close()
#ifdef CONFIG_PROC_DEBUG
				rtw_proc_remove_one(pnetdev);
#endif
			}
		}

		cancel_all_timer(padapter);

		rtw_dev_unload(padapter);

		printk("+r871xu_dev_remove, hw_init_completed=%d\n", padapter->hw_init_completed);

		free_drv_sw(padapter);

		//after free_drv_sw(), padapter has beed freed, don't refer to it.
		
	}

	usb_put_dev(interface_to_usbdev(pusb_intf));//decrease the reference count of the usb device structure when disconnect

	//If we didn't unplug usb dongle and remove/insert modlue, driver fails on sitesurvey for the first time when device is up . 
	//Reset usb port for sitesurvey fail issue. 2009.8.13, by Thomas
	//Modify condition for 92DU DMDP 2010.11.18, by Thomas
	if((pdvobjpriv->NumInterfaces != 2) || (pdvobjpriv->InterfaceNumber == 1))
	{
		if(interface_to_usbdev(pusb_intf)->state != USB_STATE_NOTATTACHED)
		{
			printk("usb attached..., try to reset usb device\n");
			usb_reset_device(interface_to_usbdev(pusb_intf));
		}
	}
	
	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("-dev_remove()\n"));
	printk("-r871xu_dev_remove, done\n");

#ifdef CONFIG_PLATFORM_RTD2880B
	printk("wlan link down\n");
	rtd2885_wlan_netlink_sendMsg("linkdown", "8712");
#endif


_func_exit_;

	return;

}


static int __init rtw_drv_entry(void)
{
#ifdef CONFIG_PLATFORM_RTK_DMP
	u32 tmp;
	tmp=readl((volatile unsigned int*)0xb801a608);
	tmp &= 0xffffff00;
	tmp |= 0x55;
	writel(tmp,(volatile unsigned int*)0xb801a608);//write dummy register for 1055
#endif

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("+rtw_drv_entry\n"));

	printk("\nrtw driver version=%s\n", DRIVERVERSION);		
	drvpriv.drv_registered = _TRUE;
	return usb_register(&drvpriv.rtw_usb_drv);
}

static void __exit rtw_drv_halt(void)
{
	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("+rtw_drv_halt\n"));
	printk("+rtw_drv_halt\n");
	drvpriv.drv_registered = _FALSE;
	usb_deregister(&drvpriv.rtw_usb_drv);
	printk("-rtw_drv_halt\n");
}


module_init(rtw_drv_entry);
module_exit(rtw_drv_halt);


/*
init (driver module)-> r8712u_drv_entry
probe (sd device)-> r871xu_drv_init(dev_init)
open (net_device) ->netdev_open
close (net_device) ->netdev_close
remove (sd device) ->r871xu_dev_remove
exit (driver module)-> r8712u_drv_halt
*/


/*
r8711s_drv_entry()
r8711u_drv_entry()
r8712s_drv_entry()
r8712u_drv_entry()
*/

