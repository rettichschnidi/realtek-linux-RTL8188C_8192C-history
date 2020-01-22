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
#define _OS_INTFS_C_

#include <drv_conf.h>

#if defined (PLATFORM_LINUX) && defined (PLATFORM_WINDOWS)

#error "Shall be Linux or Windows, but not both!\n"

#endif

#include <osdep_service.h>
#include <drv_types.h>
#include <xmit_osdep.h>
#include <recv_osdep.h>
#include <hal_init.h>
#include <rtw_ioctl.h>
#include <rtw_version.h>

#ifdef CONFIG_SDIO_HCI
#include <sdio_osintf.h>
#endif

#ifdef CONFIG_USB_HCI
#include <usb_osintf.h>
#endif

#ifdef CONFIG_PCI_HCI
#include <pci_osintf.h>
#endif

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Realtek Wireless Lan Driver");
MODULE_AUTHOR("Realtek Semiconductor Corp.");
MODULE_VERSION(DRIVERVERSION);

/* module param defaults */
int rtw_chip_version = 0x00;
int rtw_rfintfs = HWPI;
int rtw_lbkmode = 0;//RTL8712_AIR_TRX;


int rtw_network_mode = Ndis802_11IBSS;//Ndis802_11Infrastructure;//infra, ad-hoc, auto	  
//NDIS_802_11_SSID	ssid;
int rtw_channel = 1;//ad-hoc support requirement 
int rtw_wireless_mode = WIRELESS_11BG_24N;
int rtw_vrtl_carrier_sense = AUTO_VCS;
int rtw_vcs_type = RTS_CTS;//*
int rtw_rts_thresh = 2347;//*
int rtw_frag_thresh = 2346;//*
int rtw_preamble = PREAMBLE_LONG;//long, short, auto
int rtw_scan_mode = 1;//active, passive
int rtw_adhoc_tx_pwr = 1;
int rtw_soft_ap = 0;
//int smart_ps = 1;  
#ifdef CONFIG_POWER_SAVING
int rtw_power_mgnt = 1;
#else
int rtw_power_mgnt = PS_MODE_ACTIVE;
#endif	
int rtw_radio_enable = 1;
int rtw_long_retry_lmt = 7;
int rtw_short_retry_lmt = 7;
int rtw_busy_thresh = 40;
//int qos_enable = 0; //*
int rtw_ack_policy = NORMAL_ACK;
#ifdef CONFIG_MP_INCLUDED
int rtw_mp_mode = 1;
#else
int rtw_mp_mode = 0;
#endif
int rtw_software_encrypt = 0;
int rtw_software_decrypt = 0;	  
 
int rtw_wmm_enable = 1;// default is set to enable the wmm.
int rtw_uapsd_enable = 0;	  
int rtw_uapsd_max_sp = NO_LIMIT;
int rtw_uapsd_acbk_en = 0;
int rtw_uapsd_acbe_en = 0;
int rtw_uapsd_acvi_en = 0;
int rtw_uapsd_acvo_en = 0;
	
#ifdef CONFIG_80211N_HT
int rtw_ht_enable = 1;
int rtw_cbw40_enable = 1;
int rtw_ampdu_enable = 1;//for enable tx_ampdu
#endif
//int rf_config = RF_1T2R;  // 1T2R	
int rtw_rf_config = RF_819X_MAX_TYPE;  //auto
int rtw_low_power = 0;
#ifdef CONFIG_WIFI_TEST
int rtw_wifi_spec = 1;//for wifi test
#else
int rtw_wifi_spec = 0;
#endif
int rtw_channel_plan = RT_CHANNEL_DOMAIN_MAX;

#ifdef CONFIG_BT_COEXIST
int rtw_bt_iso = 2;// 0:Low, 1:High, 2:From Efuse
int rtw_bt_sco = 3;// 0:Idle, 1:None-SCO, 2:SCO, 3:From Counter, 4.Busy, 5.OtherBusy
int rtw_bt_ampdu =1 ;// 0:Disable BT control A-MPDU, 1:Enable BT control A-MPDU.
#endif
int rtw_AcceptAddbaReq = _TRUE;// 0:Reject AP's Add BA req, 1:Accept AP's Add BA req.

int  rtw_antdiv_cfg = 2; // 0:OFF , 1:ON, 2:decide by Efuse config

#ifdef CONFIG_USB_AUTOSUSPEND
int rtw_enusbss = 1;//0:disable,1:enable
#else
int rtw_enusbss = 0;//0:disable,1:enable
#endif

int rtw_hwpdn_mode=2;//0:disable,1:enable,2: by EFUSE config

#ifdef CONFIG_HW_PWRP_DETECTION
int rtw_hwpwrp_detect = 1; 
#else
int rtw_hwpwrp_detect = 0; //HW power  ping detect 0:disable , 1:enable
#endif

char* ifname = "wlan%d";

char* rtw_initmac = 0;  // temp mac address if users want to use instead of the mac address in Efuse

module_param(ifname, charp, 0644);
module_param(rtw_initmac, charp, 0644);
module_param(rtw_channel_plan, int, 0644);
module_param(rtw_chip_version, int, 0644);
module_param(rtw_rfintfs, int, 0644);
module_param(rtw_lbkmode, int, 0644);
module_param(rtw_network_mode, int, 0644);
module_param(rtw_channel, int, 0644);
module_param(rtw_mp_mode, int, 0644);
module_param(rtw_wmm_enable, int, 0644);
module_param(rtw_vrtl_carrier_sense, int, 0644);
module_param(rtw_vcs_type, int, 0644);
module_param(rtw_busy_thresh, int, 0644);
#ifdef CONFIG_80211N_HT
module_param(rtw_ht_enable, int, 0644);
module_param(rtw_cbw40_enable, int, 0644);
module_param(rtw_ampdu_enable, int, 0644);
#endif
module_param(rtw_rf_config, int, 0644);
module_param(rtw_power_mgnt, int, 0644);
module_param(rtw_low_power, int, 0644);
module_param(rtw_wifi_spec, int, 0644);

module_param(rtw_antdiv_cfg, int, 0644);


module_param(rtw_enusbss, int, 0644);
module_param(rtw_hwpdn_mode, int, 0644);
module_param(rtw_hwpwrp_detect, int, 0644);

#ifdef CONFIG_ADAPTOR_INFO_CACHING_FILE
char *adaptor_info_caching_file_path= "/data/misc/wifi/rtw_cache";
module_param(adaptor_info_caching_file_path, charp, 0644);
#endif

static uint loadparam( _adapter *padapter,  _nic_hdl	pnetdev);
static int netdev_open (struct net_device *pnetdev);
static int netdev_close (struct net_device *pnetdev);

//#ifdef RTK_DMP_PLATFORM
#ifdef CONFIG_PROC_DEBUG
#define RTL8192C_PROC_NAME "rtl819xC"
#define RTL8192D_PROC_NAME "rtl819xD"
static char rtw_proc_name[IFNAMSIZ];
static struct proc_dir_entry *rtw_proc = NULL;
static int	rtw_proc_cnt = 0;

void rtw_proc_init_one(struct net_device *dev)
{
	struct proc_dir_entry *dir_dev = NULL;
	struct proc_dir_entry *entry=NULL;
	_adapter	*padapter = rtw_netdev_priv(dev);

	if(rtw_proc == NULL)
	{
		if(padapter->chip_type == RTL8188C_8192C)
		{
			_rtw_memcpy(rtw_proc_name, RTL8192C_PROC_NAME, sizeof(RTL8192C_PROC_NAME));
		}
		else if(padapter->chip_type == RTL8192D)
		{
			_rtw_memcpy(rtw_proc_name, RTL8192D_PROC_NAME, sizeof(RTL8192D_PROC_NAME));
		}

#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
		rtw_proc=create_proc_entry(rtw_proc_name, S_IFDIR, proc_net);
#else
		rtw_proc=create_proc_entry(rtw_proc_name, S_IFDIR, init_net.proc_net);
#endif
		if (rtw_proc == NULL) {
			DBG_8192C(KERN_ERR "Unable to create rtw_proc directory\n");
			return;
		}
	}

	if(padapter->dir_dev == NULL)
	{
		padapter->dir_dev = create_proc_entry(dev->name, 
					  S_IFDIR | S_IRUGO | S_IXUGO, 
					  rtw_proc);

		dir_dev = padapter->dir_dev;

		if(dir_dev==NULL)
		{
			if(rtw_proc_cnt == 0)
			{
				if(rtw_proc){
#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
					remove_proc_entry(rtw_proc_name, proc_net);
#else
					remove_proc_entry(rtw_proc_name, init_net.proc_net);
#endif		
					rtw_proc = NULL;
				}
			}

			DBG_8192C("Unable to create dir_dev directory\n");
			return;
		}
	}
	else
	{
		return;
	}

	rtw_proc_cnt++;

	entry = create_proc_read_entry("write_reg", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_write_reg, dev);				   
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n"); 
		return;
	}
	entry->write_proc = proc_set_write_reg;

	entry = create_proc_read_entry("read_reg", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_read_reg, dev);				   
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n"); 
		return;
	}
	entry->write_proc = proc_set_read_reg;

	
	entry = create_proc_read_entry("fwstate", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_fwstate, dev);				   
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n"); 
		return;
	}


	entry = create_proc_read_entry("sec_info", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_sec_info, dev);				   
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n"); 
		return;
	}


	entry = create_proc_read_entry("mlmext_state", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_mlmext_state, dev);				   
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n"); 
		return;
	}


	entry = create_proc_read_entry("qos_option", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_qos_option, dev);				   
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n"); 
		return;
	}

	entry = create_proc_read_entry("ht_option", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_ht_option, dev);				   
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n"); 
		return;
	}

	entry = create_proc_read_entry("rf_info", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_rf_info, dev);				   
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n"); 
		return;
	}
	
	entry = create_proc_read_entry("ap_info", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_ap_info, dev);				   
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n"); 
		return;
	}

	entry = create_proc_read_entry("adapter_state", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_adapter_state, dev);				   
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n"); 
		return;
	}

	entry = create_proc_read_entry("trx_info", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_trx_info, dev);				   
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n"); 
		return;
	}

#ifdef CONFIG_AP_MODE

	entry = create_proc_read_entry("all_sta_info", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_all_sta_info, dev);				   
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n"); 
		return;
	}
#endif

#ifdef MEMORY_LEAK_DEBUG
	entry = create_proc_read_entry("_malloc_cnt", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_malloc_cnt, dev);				   
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n"); 
		return;
	}
#endif

#ifdef CONFIG_FIND_BEST_CHANNEL
	entry = create_proc_read_entry("best_channel", S_IFREG | S_IRUGO,
				   dir_dev, proc_get_best_channel, dev);				   
	if (!entry) {
		DBG_871X("Unable to create_proc_read_entry!\n"); 
		return;
	}
#endif


}

void rtw_proc_remove_one(struct net_device *dev)
{
	struct proc_dir_entry *dir_dev = NULL;
	_adapter	*padapter = rtw_netdev_priv(dev);


	dir_dev = padapter->dir_dev;
	padapter->dir_dev = NULL;

	if (dir_dev) {

		remove_proc_entry("write_reg", dir_dev);
		remove_proc_entry("read_reg", dir_dev);
		remove_proc_entry("fwstate", dir_dev);
		remove_proc_entry("sec_info", dir_dev);
		remove_proc_entry("mlmext_state", dir_dev);
		remove_proc_entry("qos_option", dir_dev);
		remove_proc_entry("ht_option", dir_dev);
		remove_proc_entry("rf_info", dir_dev);		
		remove_proc_entry("ap_info", dir_dev);
		remove_proc_entry("adapter_state", dir_dev);
		remove_proc_entry("trx_info", dir_dev);

#ifdef CONFIG_AP_MODE	
		remove_proc_entry("all_sta_info", dir_dev);
#endif		

#ifdef MEMORY_LEAK_DEBUG
		remove_proc_entry("_malloc_cnt", dir_dev);
#endif 

#ifdef CONFIG_FIND_BEST_CHANNEL
		remove_proc_entry("best_channel", dir_dev);
#endif 
		remove_proc_entry(dev->name, rtw_proc);
		dir_dev = NULL;
		
	}
	else
	{
		return;
	}

	rtw_proc_cnt--;

	if(rtw_proc_cnt == 0)
	{
		if(rtw_proc){
#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
			remove_proc_entry(rtw_proc_name, proc_net);
#else
			remove_proc_entry(rtw_proc_name, init_net.proc_net);
#endif		
			rtw_proc = NULL;
		}
	}
}
#endif

uint loadparam( _adapter *padapter,  _nic_hdl	pnetdev)
{
       
	uint status = _SUCCESS;
	struct registry_priv  *registry_par = &padapter->registrypriv;

_func_enter_;

	registry_par->chip_version = (u8)rtw_chip_version;
	registry_par->rfintfs = (u8)rtw_rfintfs;
	registry_par->lbkmode = (u8)rtw_lbkmode;	
	//registry_par->hci = (u8)hci;
	registry_par->network_mode  = (u8)rtw_network_mode;	

     	_rtw_memcpy(registry_par->ssid.Ssid, "ANY", 3);
	registry_par->ssid.SsidLength = 3;
	
	registry_par->channel = (u8)rtw_channel;
	registry_par->wireless_mode = (u8)rtw_wireless_mode;
	registry_par->vrtl_carrier_sense = (u8)rtw_vrtl_carrier_sense ;
	registry_par->vcs_type = (u8)rtw_vcs_type;
	registry_par->rts_thresh=(u16)rtw_rts_thresh;
	registry_par->frag_thresh=(u16)rtw_frag_thresh;
	registry_par->preamble = (u8)rtw_preamble;
	registry_par->scan_mode = (u8)rtw_scan_mode;
	registry_par->adhoc_tx_pwr = (u8)rtw_adhoc_tx_pwr;
	registry_par->soft_ap=  (u8)rtw_soft_ap;
	//registry_par->smart_ps =  (u8)rtw_smart_ps;  
	registry_par->power_mgnt = (u8)rtw_power_mgnt;
	registry_par->radio_enable = (u8)rtw_radio_enable;
	registry_par->long_retry_lmt = (u8)rtw_long_retry_lmt;
	registry_par->short_retry_lmt = (u8)rtw_short_retry_lmt;
  	registry_par->busy_thresh = (u16)rtw_busy_thresh;
  	//registry_par->qos_enable = (u8)rtw_qos_enable;
    	registry_par->ack_policy = (u8)rtw_ack_policy;
	registry_par->mp_mode = (u8)rtw_mp_mode;	
	registry_par->software_encrypt = (u8)rtw_software_encrypt;
	registry_par->software_decrypt = (u8)rtw_software_decrypt;	  

	 //UAPSD
	registry_par->wmm_enable = (u8)rtw_wmm_enable;
	registry_par->uapsd_enable = (u8)rtw_uapsd_enable;	  
	registry_par->uapsd_max_sp = (u8)rtw_uapsd_max_sp;
	registry_par->uapsd_acbk_en = (u8)rtw_uapsd_acbk_en;
	registry_par->uapsd_acbe_en = (u8)rtw_uapsd_acbe_en;
	registry_par->uapsd_acvi_en = (u8)rtw_uapsd_acvi_en;
	registry_par->uapsd_acvo_en = (u8)rtw_uapsd_acvo_en;

#ifdef CONFIG_80211N_HT
	registry_par->ht_enable = (u8)rtw_ht_enable;
	registry_par->cbw40_enable = (u8)rtw_cbw40_enable;
	registry_par->ampdu_enable = (u8)rtw_ampdu_enable;
#endif

	registry_par->rf_config = (u8)rtw_rf_config;
	registry_par->low_power = (u8)rtw_low_power;

	
	registry_par->wifi_spec = (u8)rtw_wifi_spec;

	registry_par->channel_plan = (u8)rtw_channel_plan;

#ifdef CONFIG_BT_COEXIST
	registry_par->bt_iso = (u8)rtw_bt_iso;
	registry_par->bt_sco = (u8)rtw_bt_sco;
	registry_par->bt_ampdu = (u8)rtw_bt_ampdu;
#endif
	registry_par->bAcceptAddbaReq = (u8)rtw_AcceptAddbaReq;

	registry_par->antdiv_cfg = (u8)rtw_antdiv_cfg;

#ifdef CONFIG_AUTOSUSPEND
	registry_par->usbss_enable = (u8)rtw_enusbss;//0:disable,1:enable
#endif
#ifdef SUPPORT_HW_RFOFF_DETECTED
	registry_par->hwpdn_mode = (u8)rtw_hwpdn_mode;//0:disable,1:enable,2:by EFUSE config
	registry_par->hwpwrp_detect = (u8)rtw_hwpwrp_detect;//0:disable,1:enable
#endif

#ifdef CONFIG_ADAPTOR_INFO_CACHING_FILE
	snprintf(registry_par->adaptor_info_caching_file_path, PATH_LENGTH_MAX, "%s",adaptor_info_caching_file_path);
	registry_par->adaptor_info_caching_file_path[PATH_LENGTH_MAX-1]=0;
#endif

_func_exit_;

	return status;

}

static int rtw_net_set_mac_address(struct net_device *pnetdev, void *p)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);
	struct sockaddr *addr = p;
	
	if(padapter->bup == _FALSE)
	{
		//DBG_8192C("r8711_net_set_mac_address(), MAC=%x:%x:%x:%x:%x:%x\n", addr->sa_data[0], addr->sa_data[1], addr->sa_data[2], addr->sa_data[3],
		//addr->sa_data[4], addr->sa_data[5]);
		_rtw_memcpy(padapter->eeprompriv.mac_addr, addr->sa_data, ETH_ALEN);
		//_rtw_memcpy(pnetdev->dev_addr, addr->sa_data, ETH_ALEN);
		//padapter->bset_hwaddr = _TRUE;
	}

	return 0;
}

static struct net_device_stats *rtw_net_get_stats(struct net_device *pnetdev)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);
	struct xmit_priv *pxmitpriv = &(padapter->xmitpriv);
	struct recv_priv *precvpriv = &(padapter->recvpriv);

	padapter->stats.tx_packets = pxmitpriv->tx_pkts;//pxmitpriv->tx_pkts++;
	padapter->stats.rx_packets = precvpriv->rx_pkts;//precvpriv->rx_pkts++; 		
	padapter->stats.tx_dropped = pxmitpriv->tx_drop;
	padapter->stats.rx_dropped = precvpriv->rx_drop;
	padapter->stats.tx_bytes = pxmitpriv->tx_bytes;
	padapter->stats.rx_bytes = precvpriv->rx_bytes;
	
	return &padapter->stats;	
}

#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,29))
static const struct net_device_ops rtw_netdev_ops = {
	.ndo_open = netdev_open,
        .ndo_stop = netdev_close,
        .ndo_start_xmit = rtw_xmit_entry,
        .ndo_set_mac_address = rtw_net_set_mac_address,
        .ndo_get_stats = rtw_net_get_stats,
        .ndo_do_ioctl = rtw_ioctl,
};
#endif

int rtw_init_netdev_name(struct net_device *pnetdev)
{
	_adapter *padapter = rtw_netdev_priv(pnetdev);

#ifdef CONFIG_EASY_REPLACEMENT
	struct net_device	*TargetNetdev = NULL;
	_adapter			*TargetAdapter = NULL;
	struct net 		*devnet = NULL;

	if(padapter->bDongle == 1)
	{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
		TargetNetdev = dev_get_by_name("wlan0");
#else
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))
		devnet = pnetdev->nd_net;
	#else
		devnet = dev_net(pnetdev);
	#endif
		TargetNetdev = dev_get_by_name(devnet, "wlan0");
#endif
		if(TargetNetdev) {
			DBG_8192C("Force onboard module driver disappear !!!\n");
			TargetAdapter = rtw_netdev_priv(TargetNetdev);
			TargetAdapter->DriverState = DRIVER_DISAPPEAR;
			if(TargetAdapter->pid != 0)
				padapter->pid = TargetAdapter->pid;
			dev_put(TargetNetdev);
			unregister_netdev(TargetNetdev);
#ifdef CONFIG_PROC_DEBUG
			if(TargetAdapter->chip_type == padapter->chip_type)
				rtw_proc_remove_one(TargetNetdev);
#endif
			padapter->DriverState = DRIVER_REPLACE_DONGLE;
		}
	}
#endif

	if(dev_alloc_name(pnetdev, ifname) < 0)
	{
		RT_TRACE(_module_os_intfs_c_,_drv_err_,("dev_alloc_name, fail! \n"));
	}

	netif_carrier_off(pnetdev);
	//netif_stop_queue(pnetdev);

	return 0;
}

struct net_device *rtw_init_netdev(void)	
{
	_adapter *padapter;
	struct net_device *pnetdev;

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("+init_net_dev\n"));

	//pnetdev = alloc_netdev(sizeof(_adapter), "wlan%d", ether_setup);
	pnetdev = rtw_alloc_etherdev(sizeof(_adapter));
	if (!pnetdev)
		return NULL;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
	SET_MODULE_OWNER(pnetdev);
#endif
	
	padapter = rtw_netdev_priv(pnetdev);
	padapter->pnetdev = pnetdev;	
	
	//pnetdev->init = NULL;
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,29))

	DBG_8192C("register rtw_netdev_ops to netdev_ops\n");
	pnetdev->netdev_ops = &rtw_netdev_ops;

#else
	pnetdev->open = netdev_open;
	pnetdev->stop = netdev_close;	
	
	pnetdev->hard_start_xmit = rtw_xmit_entry;

	pnetdev->set_mac_address = rtw_net_set_mac_address;
	pnetdev->get_stats = rtw_net_get_stats;

	pnetdev->do_ioctl = rtw_ioctl;

#endif


#ifdef CONFIG_TCP_CSUM_OFFLOAD_TX
	pnetdev->features |= NETIF_F_IP_CSUM;
#endif	
	//pnetdev->tx_timeout = NULL;
	pnetdev->watchdog_timeo = HZ*3; /* 3 second timeout */
	
	pnetdev->wireless_handlers = (struct iw_handler_def *)&rtw_handlers_def;  
	
#ifdef WIRELESS_SPY
	//priv->wireless_data.spy_data = &priv->spy_data;
	//pnetdev->wireless_data = &priv->wireless_data;
#endif

	//step 2.
   	loadparam(padapter, pnetdev);
	
	return pnetdev;

}

u32 rtw_start_drv_threads(_adapter *padapter)
{

	u32 _status = _SUCCESS;

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("+rtw_start_drv_threads\n"));

#ifdef CONFIG_SDIO_HCI
	padapter->xmitThread = kernel_thread(rtw_xmit_thread, padapter, CLONE_FS|CLONE_FILES);
	if(padapter->xmitThread < 0)
		_status = _FAIL;
#endif

#ifdef CONFIG_RECV_THREAD_MODE
	padapter->recvThread = kernel_thread(recv_thread, padapter, CLONE_FS|CLONE_FILES);
	if(padapter->recvThread < 0)
		_status = _FAIL;	
#endif

	padapter->cmdThread = kernel_thread(rtw_cmd_thread, padapter, CLONE_FS|CLONE_FILES);
	if(padapter->cmdThread < 0)
		_status = _FAIL;		

#ifdef CONFIG_EVENT_THREAD_MODE
	padapter->evtThread = kernel_thread(event_thread, padapter, CLONE_FS|CLONE_FILES);
	if(padapter->evtThread < 0)
		_status = _FAIL;		
#endif

	return _status;

}

void rtw_stop_drv_threads (_adapter *padapter)
{
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("+rtw_stop_drv_threads\n"));	

	//Below is to termindate rtw_cmd_thread & event_thread...
	_rtw_up_sema(&padapter->cmdpriv.cmd_queue_sema);
	//_rtw_up_sema(&padapter->cmdpriv.cmd_done_sema);
	if(padapter->cmdThread){
		_rtw_down_sema(&padapter->cmdpriv.terminate_cmdthread_sema);
	}

#ifdef CONFIG_EVENT_THREAD_MODE
        _rtw_up_sema(&padapter->evtpriv.evt_notify);
	if(padapter->evtThread){
		_rtw_down_sema(&padapter->evtpriv.terminate_evtthread_sema);
	}
#endif

#ifdef CONFIG_XMIT_THREAD_MODE
	// Below is to termindate tx_thread...
	_rtw_up_sema(&padapter->xmitpriv.xmit_sema);	
	_rtw_down_sema(&padapter->xmitpriv.terminate_xmitthread_sema);
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("\n drv_halt: rtw_xmit_thread can be terminated ! \n"));
#endif
	 
#ifdef CONFIG_RECV_THREAD_MODE	
	// Below is to termindate rx_thread...
	_rtw_up_sema(&padapter->recvpriv.recv_sema);
	_rtw_down_sema(&padapter->recvpriv.terminate_recvthread_sema);
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("\n drv_halt:recv_thread can be terminated! \n"));
#endif


}

u8 rtw_init_default_value(_adapter *padapter)
{
	u8 ret  = _SUCCESS;
	struct registry_priv* pregistrypriv = &padapter->registrypriv;
	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;
	struct recv_priv	*precvpriv = &padapter->recvpriv;
	struct mlme_priv *pmlmepriv= &padapter->mlmepriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;

	//xmit_priv
	pxmitpriv->vcs_setting = pregistrypriv->vrtl_carrier_sense;
	pxmitpriv->vcs = pregistrypriv->vcs_type;
	pxmitpriv->vcs_type = pregistrypriv->vcs_type;
	//pxmitpriv->rts_thresh = pregistrypriv->rts_thresh;
	pxmitpriv->frag_len = pregistrypriv->frag_thresh;
	
		

	//recv_priv
	

	//mlme_priv
	pmlmepriv->scan_interval = SCAN_INTERVAL;// 30*2 sec = 60sec
	pmlmepriv->scan_mode = SCAN_ACTIVE;
	
	//qos_priv
	//pmlmepriv->qospriv.qos_option = pregistrypriv->wmm_enable;
	
	//ht_priv
#ifdef CONFIG_80211N_HT		
	pmlmepriv->htpriv.ampdu_enable = _FALSE;//set to disabled
#endif	

	//security_priv
	//rtw_get_encrypt_decrypt_from_registrypriv(padapter);
	psecuritypriv->binstallGrpkey = _FAIL;
	psecuritypriv->sw_encrypt=pregistrypriv->software_encrypt;
	psecuritypriv->sw_decrypt=pregistrypriv->software_decrypt;
	
	psecuritypriv->dot11AuthAlgrthm = dot11AuthAlgrthm_Open; //open system
	psecuritypriv->dot11PrivacyAlgrthm = _NO_PRIVACY_;

	psecuritypriv->dot11PrivacyKeyIndex = 0;

	psecuritypriv->dot118021XGrpPrivacy = _NO_PRIVACY_;
	psecuritypriv->dot118021XGrpKeyid = 1;

	psecuritypriv->ndisauthtype = Ndis802_11AuthModeOpen;
	psecuritypriv->ndisencryptstatus = Ndis802_11WEPDisabled;
	

	//pwrctrl_priv


	//registry_priv
	rtw_init_registrypriv_dev_network(padapter);		
	rtw_update_registrypriv_dev_network(padapter);


	//hal_priv
	padapter->HalFunc.init_default_value(padapter);

	//misc.
	padapter->bReadPortCancel = _FALSE;
	padapter->bWritePortCancel = _FALSE;
	padapter->bRxRSSIDisplay = 0;
	
	return ret;
}

u8 rtw_reset_drv_sw(_adapter *padapter)
{
	u8	ret8=_SUCCESS;	
	struct mlme_priv *pmlmepriv= &padapter->mlmepriv;
	struct pwrctrl_priv *pwrctrlpriv = &padapter->pwrctrlpriv;

	//hal_priv
	padapter->HalFunc.init_default_value(padapter);
	padapter->bReadPortCancel = _FALSE;
	padapter->bWritePortCancel = _FALSE;
	padapter->bRxRSSIDisplay = 0;
	pmlmepriv->scan_interval = SCAN_INTERVAL;// 30*2 sec = 60sec
	pmlmepriv->scan_mode = SCAN_ACTIVE; // 1: active scan ,0 passive scan

	pwrctrlpriv->bips_processing = _FALSE;		

	padapter->xmitpriv.tx_pkts = 0;
	padapter->recvpriv.rx_pkts = 0;

	pmlmepriv->LinkDetectInfo.bBusyTraffic = _FALSE;

	if(pmlmepriv->fw_state & _FW_UNDER_SURVEY)			
		pmlmepriv->fw_state ^= _FW_UNDER_SURVEY;
	
	if(pmlmepriv->fw_state & _FW_UNDER_LINKING) 		
		pmlmepriv->fw_state ^= _FW_UNDER_LINKING;

#ifdef CONFIG_AUTOSUSPEND	
	#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,22) && LINUX_VERSION_CODE<=KERNEL_VERSION(2,6,34))
		padapter->dvobjpriv.pusbdev->autosuspend_disabled = 1;//autosuspend disabled by the user
	#endif
#endif

#ifdef SILENT_RESET_FOR_SPECIFIC_PLATFOM
	if(padapter->HalFunc.sreset_reset_value)
		padapter->HalFunc.sreset_reset_value(padapter);
#endif
	pwrctrlpriv->pwr_state_check_cnts = 0;

	return ret8;
}


u8 rtw_init_drv_sw(_adapter *padapter)
{

	u8	ret8=_SUCCESS;

_func_enter_;

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("+rtw_init_drv_sw\n"));

	if ((rtw_init_cmd_priv(&padapter->cmdpriv)) == _FAIL)
	{
		RT_TRACE(_module_os_intfs_c_,_drv_err_,("\n Can't init cmd_priv\n"));
		ret8=_FAIL;
		goto exit;
	}
	
	padapter->cmdpriv.padapter=padapter;
	
	if ((rtw_init_evt_priv(&padapter->evtpriv)) == _FAIL)
	{
		RT_TRACE(_module_os_intfs_c_,_drv_err_,("\n Can't init evt_priv\n"));
		ret8=_FAIL;
		goto exit;
	}
	
	
	if (rtw_init_mlme_priv(padapter) == _FAIL)
	{
		RT_TRACE(_module_os_intfs_c_,_drv_err_,("\n Can't init mlme_priv\n"));
		ret8=_FAIL;
		goto exit;
	}
		
	if(_rtw_init_xmit_priv(&padapter->xmitpriv, padapter) == _FAIL)
	{
		DBG_871X("Can't _rtw_init_xmit_priv\n");
		ret8=_FAIL;
		goto exit;
	}
		
	if(_rtw_init_recv_priv(&padapter->recvpriv, padapter) == _FAIL)
	{
		DBG_871X("Can't _rtw_init_recv_priv\n");
		ret8=_FAIL;
		goto exit;
	}

	_rtw_memset((unsigned char *)&padapter->securitypriv, 0, sizeof (struct security_priv));	
	_init_timer(&(padapter->securitypriv.tkip_timer), padapter->pnetdev, rtw_use_tkipkey_handler, padapter);

	if(_rtw_init_sta_priv(&padapter->stapriv) == _FAIL)
	{
		DBG_871X("Can't _rtw_init_sta_priv\n");
		ret8=_FAIL;
		goto exit;
	}
	
	padapter->stapriv.padapter = padapter;	

	rtw_init_bcmc_stainfo(padapter);

	rtw_init_pwrctrl_priv(padapter);	

	//_rtw_memset((u8 *)&padapter->qospriv, 0, sizeof (struct qos_priv));//move to mlme_priv
	
#ifdef CONFIG_MP_INCLUDED
	if (init_mp_priv(padapter) == _FAIL) {
		ERR_8192C("%s: initialize MP private data Fail!\n", __func__);
	}
#endif

	ret8 = rtw_init_default_value(padapter);

	rtw_dm_init(padapter);

	rtw_led_init(padapter);

#ifdef SILENT_RESET_FOR_SPECIFIC_PLATFOM
	rtw_sreset_init(padapter);
#endif	

exit:
	
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("-rtw_init_drv_sw\n"));

	_func_exit_;	
	
	return ret8;
	
}

void rtw_cancel_all_timer(_adapter *padapter)
{
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("+rtw_cancel_all_timer\n"));

	_cancel_timer_ex(&padapter->mlmepriv.assoc_timer);
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("rtw_cancel_all_timer:cancel association timer complete! \n"));

	_cancel_timer_ex(&padapter->securitypriv.tkip_timer);
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("rtw_cancel_all_timer:cancel tkip_timer! \n"));

	_cancel_timer_ex(&padapter->mlmepriv.scan_to_timer);
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("rtw_cancel_all_timer:cancel scan_to_timer! \n"));	
	
	_cancel_timer_ex(&padapter->mlmepriv.dynamic_chk_timer);
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("rtw_cancel_all_timer:cancel dynamic_chk_timer! \n"));

	// cancel sw led timer
	padapter->HalFunc.DeInitSwLeds(padapter);
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("rtw_cancel_all_timer:cancel DeInitSwLeds! \n"));

#ifdef CONFIG_IPS
	// cancel ips timer
	_cancel_timer_ex(&padapter->pwrctrlpriv.pwr_state_check_timer);
#endif

	// cancel dm  timer
	padapter->HalFunc.dm_deinit(padapter);

}

u8 rtw_free_drv_sw(_adapter *padapter)
{


	struct net_device *pnetdev = (struct net_device*)padapter->pnetdev;

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("==>rtw_free_drv_sw"));	

	free_mlme_ext_priv(&padapter->mlmeextpriv);
	
	rtw_free_cmd_priv(&padapter->cmdpriv);
	
	rtw_free_evt_priv(&padapter->evtpriv);

	rtw_free_mlme_priv(&padapter->mlmepriv);
	
	//free_io_queue(padapter);
	
	_rtw_free_xmit_priv(&padapter->xmitpriv);
	
	_rtw_free_sta_priv(&padapter->stapriv); //will free bcmc_stainfo here
	
	_rtw_free_recv_priv(&padapter->recvpriv);	

	//rtw_mfree((void *)padapter, sizeof (padapter));

#ifdef CONFIG_DRVEXT_MODULE
	free_drvext(&padapter->drvextpriv);
#endif

	padapter->HalFunc.free_hal_data(padapter);

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("<==rtw_free_drv_sw\n"));

	if(pnetdev)
	{
		rtw_free_netdev(pnetdev);
	}

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("-rtw_free_drv_sw\n"));

	return _SUCCESS;
	
}

static int netdev_open(struct net_device *pnetdev)
{
	uint status;	
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);
	struct pwrctrl_priv *pwrctrlpriv = &padapter->pwrctrlpriv;

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("+871x_drv - dev_open\n"));
	DBG_8192C("+871x_drv - drv_open, bup=%d\n", padapter->bup);

       if(padapter->bup == _FALSE)
    	{    
		padapter->bDriverStopped = _FALSE;
	 	padapter->bSurpriseRemoved = _FALSE;	 
		padapter->bCardDisableWOHSM = _FALSE;        	
	
		status = rtw_hal_init(padapter);		
		if (status ==_FAIL)
		{			
			RT_TRACE(_module_os_intfs_c_,_drv_err_,("rtl871x_hal_init(): Can't init h/w!\n"));
			goto netdev_open_error;
		}
		
		DBG_8192C("MAC Address = %x-%x-%x-%x-%x-%x\n", 
				 pnetdev->dev_addr[0], pnetdev->dev_addr[1], pnetdev->dev_addr[2], pnetdev->dev_addr[3], pnetdev->dev_addr[4], pnetdev->dev_addr[5]);		

		
		status=rtw_start_drv_threads(padapter);
		if(status ==_FAIL)
		{			
			RT_TRACE(_module_os_intfs_c_,_drv_err_,("Initialize driver software resource Failed!\n"));			
			goto netdev_open_error;			
		}


		if (init_mlme_ext_priv(padapter) == _FAIL)
		{
			RT_TRACE(_module_os_intfs_c_,_drv_err_,("can't init mlme_ext_priv\n"));
			goto netdev_open_error;
		}


#ifdef CONFIG_DRVEXT_MODULE
		init_drvext(padapter);
#endif

		if(padapter->intf_start)
		{
			padapter->intf_start(padapter);
		}

#ifdef CONFIG_PROC_DEBUG
#ifndef RTK_DMP_PLATFORM
		rtw_proc_init_one(pnetdev);
#endif
#endif
		padapter->bup = _TRUE;
	}
	padapter->net_closed = _FALSE;

	_set_timer(&padapter->mlmepriv.dynamic_chk_timer, 2000);

	if(( pwrctrlpriv->power_mgnt != PS_MODE_ACTIVE ) ||(padapter->pwrctrlpriv.bHWPwrPindetect))
	{
		padapter->pwrctrlpriv.bips_processing = _FALSE;	
		_set_timer(&padapter->pwrctrlpriv.pwr_state_check_timer, padapter->pwrctrlpriv.pwr_state_check_inverval);
 	}

	//netif_carrier_on(pnetdev);//call this func when rtw_joinbss_event_callback return success       
 	if(!netif_queue_stopped(pnetdev))
      		netif_start_queue(pnetdev);
	else
		netif_wake_queue(pnetdev);

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("-871x_drv - dev_open\n"));
	DBG_8192C("-871x_drv - drv_open, bup=%d\n", padapter->bup);
		
	 return 0;
	
netdev_open_error:

	padapter->bup = _FALSE;
	
	netif_carrier_off(pnetdev);	
	netif_stop_queue(pnetdev);
	
	RT_TRACE(_module_os_intfs_c_,_drv_err_,("-871x_drv - dev_open, fail!\n"));
	DBG_8192C("-871x_drv - drv_open fail, bup=%d\n", padapter->bup);
	
	return (-1);
	
}



#ifdef CONFIG_IPS
int  ips_netdrv_open(_adapter *padapter)
{
	int status = _SUCCESS;
	padapter->net_closed = _FALSE;
	DBG_8192C("===> %s.........\n",__FUNCTION__);


	padapter->bDriverStopped = _FALSE;
	padapter->bSurpriseRemoved = _FALSE;
	padapter->bCardDisableWOHSM = _FALSE;
	padapter->bup = _TRUE;

	status = rtw_hal_init(padapter);
	if (status ==_FAIL)
	{
		RT_TRACE(_module_os_intfs_c_,_drv_err_,("ips_netdrv_open(): Can't init h/w!\n"));
		goto netdev_open_error;
	}

	if(padapter->intf_start)
	{
		padapter->intf_start(padapter);
	}

	_set_timer(&padapter->pwrctrlpriv.pwr_state_check_timer, padapter->pwrctrlpriv.pwr_state_check_inverval);
  	_set_timer(&padapter->mlmepriv.dynamic_chk_timer,5000);

	 return _SUCCESS;

netdev_open_error:
	padapter->bup = _FALSE;
	DBG_8192C("-ips_netdrv_open - drv_open failure, bup=%d\n", padapter->bup);

	return _FAIL;
}

void rtw_ips_dev_unload(_adapter *padapter)
{
	struct net_device *pnetdev= (struct net_device*)padapter->pnetdev;
	struct xmit_priv	*pxmitpriv = &(padapter->xmitpriv);
	DBG_8192C("====> %s...\n",__FUNCTION__);

	padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_FIFO_CLEARN_UP, 0);

	if(padapter->intf_stop)
	{
		padapter->intf_stop(padapter);
	}

	//s5.
	if(padapter->bSurpriseRemoved == _FALSE)
	{
		rtw_hal_deinit(padapter);
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

}

int rtw_ips_pwr_up(_adapter *padapter)
{	
	int result;
	DBG_8192C("===>  rtw_ips_pwr_up..............\n");
	rtw_reset_drv_sw(padapter);
	result = ips_netdrv_open(padapter);
 	DBG_8192C("<===  rtw_ips_pwr_up..............\n");
	return result;

}

void rtw_ips_pwr_down(_adapter *padapter)
{
	DBG_8192C("===> rtw_ips_pwr_down...................\n");

	padapter->bCardDisableWOHSM = _TRUE;
	padapter->net_closed = _TRUE;

	padapter->ledpriv.LedControlHandler(padapter, LED_CTL_NO_LINK);
	
	rtw_ips_dev_unload(padapter);
	padapter->bCardDisableWOHSM = _FALSE;
	DBG_8192C("<=== rtw_ips_pwr_down.....................\n");
}
#endif

int pm_netdev_open(struct net_device *pnetdev,u8 bnormal)
{
	int status;
	if(bnormal)
		status = netdev_open(pnetdev);
#ifdef CONFIG_IPS
	else
		status =  (_SUCCESS == ips_netdrv_open((_adapter *)rtw_netdev_priv(pnetdev)))?(0):(-1);
#endif

	return status;
}
extern int rfpwrstate_check(_adapter *padapter);
static int netdev_close(struct net_device *pnetdev)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("+871x_drv - drv_close\n"));	

	if(padapter->pwrctrlpriv.bInternalAutoSuspend == _TRUE)
	{
		rfpwrstate_check(padapter);
	}
	padapter->net_closed = _TRUE;

/*	if(!padapter->hw_init_completed)
	{
		DBG_8192C("(1)871x_drv - drv_close, bup=%d, hw_init_completed=%d\n", padapter->bup, padapter->hw_init_completed);

		padapter->bDriverStopped = _TRUE;

		rtw_dev_unload(padapter);
	}
	else*/
	{
		DBG_8192C("(2)871x_drv - drv_close, bup=%d, hw_init_completed=%d\n", padapter->bup, padapter->hw_init_completed);

		//s1.
		if(pnetdev)   
     		{
			if (!netif_queue_stopped(pnetdev))
				netif_stop_queue(pnetdev);
     		}

#ifndef CONFIG_ANDROID

		//s2.	
		//s2-1.  issue rtw_disassoc_cmd to fw
		rtw_disassoc_cmd(padapter);	
		//s2-2.  indicate disconnect to os
		rtw_indicate_disconnect(padapter);
		//s2-3. 
	       rtw_free_assoc_resources(padapter);	
		//s2-4.
		rtw_free_network_queue(padapter,_TRUE);
#endif
	}

	// Close LED
	padapter->ledpriv.LedControlHandler(padapter, LED_CTL_POWER_OFF);

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("-871x_drv - drv_close\n"));
	DBG_8192C("-871x_drv - drv_close, bup=%d\n", padapter->bup);
	   
	return 0;
	
}

