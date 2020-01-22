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
#define  _IOCTL_LINUX_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <wlan_bssdef.h>
#include <rtw_debug.h>
#include <wifi.h>
#include <rtw_mlme.h>
#include <rtw_mlme_ext.h>
#include <rtw_ioctl.h>
#include <rtw_ioctl_set.h>
#include <rtw_ioctl_query.h>

//#ifdef CONFIG_MP_INCLUDED
#include <rtw_mp_ioctl.h>
//#endif

#include <usb_ops.h>
#include <rtw_version.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27))
#define  iwe_stream_add_event(a, b, c, d, e)  iwe_stream_add_event(b, c, d, e)
#define  iwe_stream_add_point(a, b, c, d, e)  iwe_stream_add_point(b, c, d, e)
#endif


#define RTL_IOCTL_WPA_SUPPLICANT	SIOCIWFIRSTPRIV+30

#define SCAN_ITEM_SIZE 768
#define MAX_CUSTOM_LEN 64
#define RATE_COUNT 4

extern u8 key_2char2num(u8 hch, u8 lch);

u32 rtw_rates[] = {1000000,2000000,5500000,11000000,
	6000000,9000000,12000000,18000000,24000000,36000000,48000000,54000000};

const char * const iw_operation_mode[] = 
{ 
	"Auto", "Ad-Hoc", "Managed",  "Master", "Repeater", "Secondary", "Monitor" 
};

static int hex2num_i(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

static int hex2byte_i(const char *hex)
{
	int a, b;
	a = hex2num_i(*hex++);
	if (a < 0)
		return -1;
	b = hex2num_i(*hex++);
	if (b < 0)
		return -1;
	return (a << 4) | b;
}

/**
 * hwaddr_aton - Convert ASCII string to MAC address
 * @txt: MAC address as a string (e.g., "00:11:22:33:44:55")
 * @addr: Buffer for the MAC address (ETH_ALEN = 6 bytes)
 * Returns: 0 on success, -1 on failure (e.g., string not a MAC address)
 */
static int hwaddr_aton_i(const char *txt, u8 *addr)
{
	int i;

	for (i = 0; i < 6; i++) {
		int a, b;

		a = hex2num_i(*txt++);
		if (a < 0)
			return -1;
		b = hex2num_i(*txt++);
		if (b < 0)
			return -1;
		*addr++ = (a << 4) | b;
		if (i < 5 && *txt++ != ':')
			return -1;
	}

	return 0;
}

void request_wps_pbc_event(_adapter *padapter)
{
	u8 *buff, *p;
	union iwreq_data wrqu;


	buff = rtw_malloc(IW_CUSTOM_MAX);
	if(!buff)
		return;
		
	_memset(buff, 0, IW_CUSTOM_MAX);
		
	p=buff;
		
	p+=sprintf(p, "WPS_PBC_START.request=TRUE");
		
	_memset(&wrqu,0,sizeof(wrqu));
		
	wrqu.data.length = p-buff;
		
	wrqu.data.length = (wrqu.data.length<IW_CUSTOM_MAX) ? wrqu.data.length:IW_CUSTOM_MAX;

	printk("%s\n", __FUNCTION__);
		
	wireless_send_event(padapter->pnetdev, IWEVCUSTOM, &wrqu, buff);

	if(buff)
	{
		rtw_mfree(buff, IW_CUSTOM_MAX);
	}

}


void indicate_wx_scan_complete_event(_adapter *padapter)
{	
	union iwreq_data wrqu;
	struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;	

	_memset(&wrqu, 0, sizeof(union iwreq_data));

	//printk("+rtw_indicate_wx_scan_complete_event\n");
	wireless_send_event(padapter->pnetdev, SIOCGIWSCAN, &wrqu, NULL);
}


void indicate_wx_assoc_event(_adapter *padapter)
{	
	union iwreq_data wrqu;
	struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;	

	_memset(&wrqu, 0, sizeof(union iwreq_data));
	
	wrqu.ap_addr.sa_family = ARPHRD_ETHER;	
	
	_memcpy(wrqu.ap_addr.sa_data, pmlmepriv->cur_network.network.MacAddress, ETH_ALEN);

	//printk("+indicate_wx_assoc_event\n");
	wireless_send_event(padapter->pnetdev, SIOCGIWAP, &wrqu, NULL);
}

void indicate_wx_disassoc_event(_adapter *padapter)
{	
	union iwreq_data wrqu;

	_memset(&wrqu, 0, sizeof(union iwreq_data));

	wrqu.ap_addr.sa_family = ARPHRD_ETHER;
	_memset(wrqu.ap_addr.sa_data, 0, ETH_ALEN);
	
	//printk("+indicate_wx_disassoc_event\n");
	wireless_send_event(padapter->pnetdev, SIOCGIWAP, &wrqu, NULL);
}

/*
uint	is_cckrates_included(u8 *rate)
{	
		u32	i = 0;			

		while(rate[i]!=0)
		{		
			if  (  (((rate[i]) & 0x7f) == 2)	|| (((rate[i]) & 0x7f) == 4) ||		
			(((rate[i]) & 0x7f) == 11)  || (((rate[i]) & 0x7f) == 22) )		
			return _TRUE;	
			i++;
		}
		
		return _FALSE;
}

uint	is_cckratesonly_included(u8 *rate)
{
	u32 i = 0;

	while(rate[i]!=0)
	{
			if  (  (((rate[i]) & 0x7f) != 2) && (((rate[i]) & 0x7f) != 4) &&
				(((rate[i]) & 0x7f) != 11)  && (((rate[i]) & 0x7f) != 22) )
			return _FALSE;		
			i++;
	}
	
	return _TRUE;
}
*/

static char *translate_scan(_adapter *padapter, 
				struct iw_request_info* info, struct wlan_network *pnetwork,
				char *start, char *stop)
{
	struct iw_event iwe;
	u16 cap;
	u32 ht_ielen = 0;
	char custom[MAX_CUSTOM_LEN];
	char *p;
	u16 max_rate=0, rate, ht_cap=_FALSE;
	u32 i = 0;	
	char	*current_val;
	long rssi;
	u8 bw_40MHz=0, short_GI=0;
	u16 mcs_rate=0;
	struct registry_priv *pregpriv = &padapter->registrypriv;
	
#if ( P2P_INCLUDED == 1 )	
	struct wifidirect_info	*pwdinfo = &padapter->wdinfo;

	if ( pwdinfo->p2p_state == P2P_STATE_LISTEN )
	{
		u32	blnGotP2PIE = _FALSE;
		
		//	User is doing the P2P device discovery
		//	The SSID should be "DIRECT-" and the IE should contains the P2P IE.
		//	If not, the driver should ignore this AP and go to the next AP.

		//	Verifying the SSID
		if ( ( pnetwork->network.Ssid.SsidLength == P2P_WILDCARD_SSID_LEN ) && 
			( _memcmp( pnetwork->network.Ssid.Ssid, pwdinfo->p2p_wildcard_ssid, pnetwork->network.Ssid.SsidLength ) )
		   )
		{
			u32	p2pielen = 0;

			//	Verifying the P2P IE
			if ( get_p2p_ie( &pnetwork->network.IEs[12], pnetwork->network.IELength - 12, NULL, &p2pielen) )
			{
				blnGotP2PIE = _TRUE;
			}

		}

		if ( blnGotP2PIE == _FALSE )
		{
			return start;
		}
		
	}

#endif

	/*  AP MAC address  */
	iwe.cmd = SIOCGIWAP;
	iwe.u.ap_addr.sa_family = ARPHRD_ETHER;

	_memcpy(iwe.u.ap_addr.sa_data, pnetwork->network.MacAddress, ETH_ALEN);
	start = iwe_stream_add_event(info, start, stop, &iwe, IW_EV_ADDR_LEN);

	/* Add the ESSID */
#ifdef CONFIG_ANDROID	
	if(validate_ssid(&(pnetwork->network.Ssid))==_FALSE)
	{		
		RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_err_, ("translate_scan : validate_ssid==FALSE \n"));
	}	
	else
#endif
	{
		iwe.cmd = SIOCGIWESSID;
		iwe.u.data.flags = 1;	
		iwe.u.data.length = min((u16)pnetwork->network.Ssid.SsidLength, (u16)32);
		start = iwe_stream_add_point(info, start, stop, &iwe, pnetwork->network.Ssid.Ssid);
	}

	//parsing HT_CAP_IE
	p = get_ie(&pnetwork->network.IEs[12], _HT_CAPABILITY_IE_, &ht_ielen, pnetwork->network.IELength-12);
	if(p && ht_ielen>0)
	{
		struct ieee80211_ht_cap *pht_capie;
		ht_cap = _TRUE;			
		pht_capie = (struct ieee80211_ht_cap *)(p+2);		
		_memcpy(&mcs_rate , pht_capie->supp_mcs_set, 2);
		bw_40MHz = (pht_capie->cap_info&IEEE80211_HT_CAP_SUP_WIDTH) ? 1:0;
		short_GI = (pht_capie->cap_info&(IEEE80211_HT_CAP_SGI_20|IEEE80211_HT_CAP_SGI_40)) ? 1:0;
	}

	/* Add the protocol name */
	iwe.cmd = SIOCGIWNAME;
	if ((is_cckratesonly_included((u8*)&pnetwork->network.SupportedRates)) == _TRUE)		
	{
		if(ht_cap == _TRUE)
			snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11bn");
		else
		snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11b");
	}	
	else if ((is_cckrates_included((u8*)&pnetwork->network.SupportedRates)) == _TRUE)	
	{
		if(ht_cap == _TRUE)
			snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11bgn");
		else
		snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11bg");
	}	
	else
	{
		if(pnetwork->network.Configuration.DSConfig > 14)
		{
			if(ht_cap == _TRUE)
				snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11an");
			else
				snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11a");
		}
		else
		{
			if(ht_cap == _TRUE)
				snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11gn");
			else
				snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11g");
		}
	}	

	start = iwe_stream_add_event(info, start, stop, &iwe, IW_EV_CHAR_LEN);

	  /* Add mode */
        iwe.cmd = SIOCGIWMODE;
	_memcpy((u8 *)&cap, get_capability_from_ie(pnetwork->network.IEs), 2);

	cap = le16_to_cpu(cap);

	if(cap & (WLAN_CAPABILITY_IBSS |WLAN_CAPABILITY_BSS)){
		if (cap & WLAN_CAPABILITY_BSS)
			iwe.u.mode = IW_MODE_MASTER;
		else
			iwe.u.mode = IW_MODE_ADHOC;

		start = iwe_stream_add_event(info, start, stop, &iwe, IW_EV_UINT_LEN);
	}

	if(pnetwork->network.Configuration.DSConfig<1 /*|| pnetwork->network.Configuration.DSConfig>14*/)
		pnetwork->network.Configuration.DSConfig = 1;

	 /* Add frequency/channel */
	iwe.cmd = SIOCGIWFREQ;
	iwe.u.freq.m = ch2freq(pnetwork->network.Configuration.DSConfig) * 100000;
	iwe.u.freq.e = 1;
	iwe.u.freq.i = pnetwork->network.Configuration.DSConfig;
	start = iwe_stream_add_event(info, start, stop, &iwe, IW_EV_FREQ_LEN);

	/* Add encryption capability */
	iwe.cmd = SIOCGIWENCODE;
	if (cap & WLAN_CAPABILITY_PRIVACY)
		iwe.u.data.flags = IW_ENCODE_ENABLED | IW_ENCODE_NOKEY;
	else
		iwe.u.data.flags = IW_ENCODE_DISABLED;
	iwe.u.data.length = 0;
	start = iwe_stream_add_point(info, start, stop, &iwe, pnetwork->network.Ssid.Ssid);
	
	/*Add basic and extended rates */
	max_rate = 0;
	p = custom;
	p += snprintf(p, MAX_CUSTOM_LEN - (p - custom), " Rates (Mb/s): ");
	while(pnetwork->network.SupportedRates[i]!=0)
	{
		rate = pnetwork->network.SupportedRates[i]&0x7F; 
		if (rate > max_rate)
			max_rate = rate;
		p += snprintf(p, MAX_CUSTOM_LEN - (p - custom),
			      "%d%s ", rate >> 1, (rate & 1) ? ".5" : "");
		i++;
	}
	
	if(ht_cap == _TRUE)
	{
		if(mcs_rate&0x8000)//MCS15
		{
			max_rate = (bw_40MHz) ? ((short_GI)?300:270):((short_GI)?144:130);
			
		}
		else if(mcs_rate&0x0080)//MCS7
		{
			max_rate = (bw_40MHz) ? ((short_GI)?150:135):((short_GI)?72:65);
		}
		else//default MCS7
		{
			printk("wx_get_scan, mcs_rate_bitmap=0x%x\n", mcs_rate);
			max_rate = (bw_40MHz) ? ((short_GI)?150:135):((short_GI)?72:65);
		}

		max_rate = max_rate*2;//Mbps/2;		
	}

	iwe.cmd = SIOCGIWRATE;
	iwe.u.bitrate.fixed = iwe.u.bitrate.disabled = 0;
	iwe.u.bitrate.value = max_rate * 500000;
	start = iwe_stream_add_event(info, start, stop, &iwe, IW_EV_PARAM_LEN);
	
	//parsing WPA/WPA2 IE
	{
		u8 buf[MAX_WPA_IE_LEN];
		u8 wpa_ie[255],rsn_ie[255];
		u16 wpa_len=0,rsn_len=0;
		u8 *p;
		sint out_len=0;
		out_len=get_sec_ie(pnetwork->network.IEs ,pnetwork->network.IELength,rsn_ie,&rsn_len,wpa_ie,&wpa_len);
		RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_scan: ssid=%s\n",pnetwork->network.Ssid.Ssid));
		RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_scan: wpa_len=%d rsn_len=%d\n",wpa_len,rsn_len));

		if (wpa_len > 0)
		{
			p=buf;
			_memset(buf, 0, MAX_WPA_IE_LEN);
			p += sprintf(p, "wpa_ie=");
			for (i = 0; i < wpa_len; i++) {
				p += sprintf(p, "%02x", wpa_ie[i]);
			}
	
			_memset(&iwe, 0, sizeof(iwe));
			iwe.cmd = IWEVCUSTOM;
			iwe.u.data.length = strlen(buf);
			start = iwe_stream_add_point(info, start, stop, &iwe,buf);
			
			_memset(&iwe, 0, sizeof(iwe));
			iwe.cmd =IWEVGENIE;
			iwe.u.data.length = wpa_len;
			start = iwe_stream_add_point(info, start, stop, &iwe, wpa_ie);			
		}
		if (rsn_len > 0)
		{
			p = buf;
			_memset(buf, 0, MAX_WPA_IE_LEN);
			p += sprintf(p, "rsn_ie=");
			for (i = 0; i < rsn_len; i++) {
				p += sprintf(p, "%02x", rsn_ie[i]);
			}
			_memset(&iwe, 0, sizeof(iwe));
			iwe.cmd = IWEVCUSTOM;
			iwe.u.data.length = strlen(buf);
			start = iwe_stream_add_point(info, start, stop, &iwe,buf);
		
			_memset(&iwe, 0, sizeof(iwe));
			iwe.cmd =IWEVGENIE;
			iwe.u.data.length = rsn_len;
			start = iwe_stream_add_point(info, start, stop, &iwe, rsn_ie);		
		}
	}

	{//parsing WPS IE
		u8 wps_ie[512];
		uint wps_ielen;

		if(get_wps_ie(pnetwork->network.IEs, pnetwork->network.IELength, wps_ie, &wps_ielen)!=NULL)
		{
			if(wps_ielen>2)
			{				
				iwe.cmd =IWEVGENIE;
				iwe.u.data.length = (u16)wps_ielen;
				start = iwe_stream_add_point(info, start, stop, &iwe, wps_ie);
			}	
		}
	}

	/* Add quality statistics */
	iwe.cmd = IWEVQUAL;
	rssi = pnetwork->network.Rssi;//dBM
	
#ifdef CONFIG_RTL8711	
	rssi = (rssi*2) + 190;
	if(rssi>100) rssi = 100;
	if(rssi<0) rssi = 0;
#endif	
	
	//printk("RSSI=0x%X%%\n", rssi);

	// we only update signal_level (signal strength) that is rssi.
	iwe.u.qual.updated = IW_QUAL_QUAL_UPDATED | IW_QUAL_LEVEL_UPDATED | IW_QUAL_NOISE_INVALID
	#ifdef CONFIG_ANDROID
		| IW_QUAL_DBM
	#endif
	;

	#ifdef CONFIG_ANDROID
	iwe.u.qual.level = (u8)pnetwork->network.Rssi;//dbm
	#else
	iwe.u.qual.level = (u8)pnetwork->network.PhyInfo.SignalStrength;//%
	#endif
	iwe.u.qual.qual = (u8)pnetwork->network.PhyInfo.SignalStrength;   // signal quality
	iwe.u.qual.noise = 0; // noise level
	
	//printk("iqual=%d, ilevel=%d, inoise=%d, iupdated=%d\n", iwe.u.qual.qual, iwe.u.qual.level , iwe.u.qual.noise, iwe.u.qual.updated);

	start = iwe_stream_add_event(info, start, stop, &iwe, IW_EV_QUAL_LEN);
	
	//how to translate rssi to ?%
	//rssi = (iwe.u.qual.level*2) +  190;
	//if(rssi>100) rssi = 100;
	//if(rssi<0) rssi = 0;
	
	return start;	
}

static int wpa_set_auth_algs(struct net_device *dev, u32 value)
{	
	_adapter *padapter = netdev_priv(dev);
	int ret = 0;

	if ((value & AUTH_ALG_SHARED_KEY)&&(value & AUTH_ALG_OPEN_SYSTEM))
	{
		printk("wpa_set_auth_algs, AUTH_ALG_SHARED_KEY and  AUTH_ALG_OPEN_SYSTEM [value:0x%x]\n",value);
		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
		padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeAutoSwitch;
		padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Auto;
	} 
	else if (value & AUTH_ALG_SHARED_KEY)
	{
		printk("wpa_set_auth_algs, AUTH_ALG_SHARED_KEY  [value:0x%x]\n",value);
		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;

#ifdef CONFIG_PLATFORM_MT53XX
		padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeAutoSwitch;
		padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Auto;
#else
		padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeShared;
		padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Shared;
#endif
	} 
	else if(value & AUTH_ALG_OPEN_SYSTEM)
	{
		printk("wpa_set_auth_algs, AUTH_ALG_OPEN_SYSTEM\n");
		//padapter->securitypriv.ndisencryptstatus = Ndis802_11EncryptionDisabled;
		if(padapter->securitypriv.ndisauthtype < Ndis802_11AuthModeWPAPSK)
		{
#ifdef CONFIG_PLATFORM_MT53XX
			padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeAutoSwitch;
			padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Auto;
#else
			padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeOpen;
 			padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Open;
#endif
		}
		
	}
	else if(value & AUTH_ALG_LEAP)
	{
		printk("wpa_set_auth_algs, AUTH_ALG_LEAP\n");
	}
	else
	{
		printk("wpa_set_auth_algs, error!\n");
		ret = -EINVAL;
	}

	return ret;
	
}

static int wpa_set_encryption(struct net_device *dev, struct ieee_param *param, u32 param_len)
{
	int ret = 0;
	u32 wep_key_idx, wep_key_len;
	NDIS_802_11_WEP	 *pwep = NULL;	
	_adapter *padapter = netdev_priv(dev);
	struct mlme_priv 	*pmlmepriv = &padapter->mlmepriv;		
	struct security_priv *psecuritypriv = &padapter->securitypriv;
#if ( P2P_INCLUDED == 1 )
	struct wifidirect_info* pwdinfo = &padapter->wdinfo;
#endif

_func_enter_;

	param->u.crypt.err = 0;
	param->u.crypt.alg[IEEE_CRYPT_ALG_NAME_LEN - 1] = '\0';

	if (param_len < (u32) ((u8 *) param->u.crypt.key - (u8 *) param) + param->u.crypt.key_len)
	{
		ret =  -EINVAL;
		goto exit;
	}
	
	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff) 
	{
		if (param->u.crypt.idx >= WEP_KEYS)
		{
			ret = -EINVAL;
			goto exit;
		}
	} else {
		ret = -EINVAL;
		goto exit;
	}

	if (strcmp(param->u.crypt.alg, "WEP") == 0)
	{
		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("wpa_set_encryption, crypt.alg = WEP\n"));
		printk("wpa_set_encryption, crypt.alg = WEP\n");
		
		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
		padapter->securitypriv.dot11PrivacyAlgrthm=_WEP40_;
		padapter->securitypriv.dot118021XGrpPrivacy=_WEP40_;	

		wep_key_idx = param->u.crypt.idx;
		wep_key_len = param->u.crypt.key_len;
			
		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_,("(1)wep_key_idx=%d\n", wep_key_idx));
		printk("(1)wep_key_idx=%d\n", wep_key_idx);

		if (wep_key_idx > WEP_KEYS)
			return -EINVAL;

		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_,("(2)wep_key_idx=%d\n", wep_key_idx));

		if (wep_key_len > 0) 
		{			
		 	wep_key_len = wep_key_len <= 5 ? 5 : 13;

		 	pwep =(NDIS_802_11_WEP	 *) rtw_malloc(wep_key_len + FIELD_OFFSET(NDIS_802_11_WEP, KeyMaterial));
			if(pwep == NULL){
				RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,(" wpa_set_encryption: pwep allocate fail !!!\n"));
				goto exit;
			}
			
		 	_memset(pwep, 0, sizeof(NDIS_802_11_WEP));
		
		 	pwep->KeyLength = wep_key_len;
			pwep->Length = wep_key_len + FIELD_OFFSET(NDIS_802_11_WEP, KeyMaterial);

			if(wep_key_len==13)
			{
				padapter->securitypriv.dot11PrivacyAlgrthm=_WEP104_;
				padapter->securitypriv.dot118021XGrpPrivacy=_WEP104_;
			}			
		}
		else {		
			ret = -EINVAL;
			goto exit;
		}

		pwep->KeyIndex = wep_key_idx;
		pwep->KeyIndex |= 0x80000000; 

		_memcpy(pwep->KeyMaterial,  param->u.crypt.key, pwep->KeyLength);
	
		if(param->u.crypt.set_tx)
		{
			printk("wep, set_tx=1\n");
			
		if(set_802_11_add_wep(padapter, pwep) == (u8)_FAIL)
			{
				ret = -EOPNOTSUPP ;
			}	
		}
		else
		{
			printk("wep, set_tx=0\n");
			
			//don't update "psecuritypriv->dot11PrivacyAlgrthm" and 
			//"psecuritypriv->dot11PrivacyKeyIndex=keyid", but can set_key to fw/cam
			
			if (wep_key_idx >= WEP_KEYS) {
				ret = -EOPNOTSUPP ;
				goto exit;
			}				
			
		      _memcpy(&(psecuritypriv->dot11DefKey[wep_key_idx].skey[0]), pwep->KeyMaterial, pwep->KeyLength);
			psecuritypriv->dot11DefKeylen[wep_key_idx]=pwep->KeyLength;	
			set_key(padapter, psecuritypriv, wep_key_idx);			
		}

		goto exit;		
	}

	if(padapter->securitypriv.dot11AuthAlgrthm == dot11AuthAlgrthm_8021X) // 802_1x
	{
		struct sta_info * psta,*pbcmc_sta;
		struct sta_priv * pstapriv = &padapter->stapriv;

		if (check_fwstate(pmlmepriv, WIFI_STATION_STATE | WIFI_MP_STATE) == _TRUE) //sta mode
		{
			psta = get_stainfo(pstapriv, get_bssid(pmlmepriv));				
			if (psta == NULL) {
				//DEBUG_ERR( ("Set wpa_set_encryption: Obtain Sta_info fail \n"));
			}
			else
			{
				#ifdef WPA_SET_ENCRYPTION_REFINE
				if (strcmp(param->u.crypt.alg, "none") != 0) 
				#endif
					psta->ieee8021x_blocked = _FALSE;
				
				if((padapter->securitypriv.ndisencryptstatus == Ndis802_11Encryption2Enabled)||
						(padapter->securitypriv.ndisencryptstatus ==  Ndis802_11Encryption3Enabled))
				{
					psta->dot118021XPrivacy = padapter->securitypriv.dot11PrivacyAlgrthm;
				}		

				if(param->u.crypt.set_tx ==1)//pairwise key
				{ 
					_memcpy(psta->dot118021x_UncstKey.skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
					
					if(strcmp(param->u.crypt.alg, "TKIP") == 0)//set mic key
					{						
						//DEBUG_ERR(("\nset key length :param->u.crypt.key_len=%d\n", param->u.crypt.key_len));
						_memcpy(psta->dot11tkiptxmickey.skey, &(param->u.crypt.key[16]), 8);
						_memcpy(psta->dot11tkiprxmickey.skey, &(param->u.crypt.key[24]), 8);

						padapter->securitypriv.busetkipkey=_FALSE;
						_set_timer(&padapter->securitypriv.tkip_timer, 50);						
					}

					//DEBUG_ERR(("\n param->u.crypt.key_len=%d\n",param->u.crypt.key_len));
					//DEBUG_ERR(("\n ~~~~stastakey:unicastkey\n"));
					
					setstakey_cmd(padapter, (unsigned char *)psta, _TRUE);
				}
				else//group key
				{ 					
					_memcpy(padapter->securitypriv.dot118021XGrpKey[param->u.crypt.idx-1].skey,  param->u.crypt.key,(param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
					_memcpy(padapter->securitypriv.dot118021XGrptxmickey.skey,&(param->u.crypt.key[16]),8);
					_memcpy(padapter->securitypriv.dot118021XGrprxmickey.skey,&(param->u.crypt.key[24]),8);
                                        padapter->securitypriv.binstallGrpkey = _TRUE;	
					//DEBUG_ERR(("\n param->u.crypt.key_len=%d\n", param->u.crypt.key_len));
					//DEBUG_ERR(("\n ~~~~stastakey:groupkey\n"));
					set_key(padapter,&padapter->securitypriv,param->u.crypt.idx);
#if ( P2P_INCLUDED == 1 )
					if ( pwdinfo->p2p_state == P2P_STATE_PROVISIONING_ING )
					{
						pwdinfo->p2p_state == P2P_STATE_PROVISIONING_DONE;
					}
#endif
					
				}						
			}

			pbcmc_sta=get_bcmc_stainfo(padapter);
			if(pbcmc_sta==NULL)
			{
				//DEBUG_ERR( ("Set OID_802_11_ADD_KEY: bcmc stainfo is null \n"));
			}
			else
			{
				#ifdef WPA_SET_ENCRYPTION_REFINE
				if (strcmp(param->u.crypt.alg, "none") != 0) 
				#endif
					pbcmc_sta->ieee8021x_blocked = _FALSE;
				if((padapter->securitypriv.ndisencryptstatus == Ndis802_11Encryption2Enabled)||
						(padapter->securitypriv.ndisencryptstatus ==  Ndis802_11Encryption3Enabled))
				{							
					pbcmc_sta->dot118021XPrivacy = padapter->securitypriv.dot11PrivacyAlgrthm;
				}					
			}				
		}
		else if(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE)) //adhoc mode
		{		
		}			
	}

exit:
	
	if (pwep) {
		rtw_mfree((u8 *)pwep, wep_key_len + FIELD_OFFSET(NDIS_802_11_WEP, KeyMaterial));		
	}	
	
	_func_exit_;
	
	return ret;	
}

static int rtw_set_wpa_ie(_adapter *padapter, char *pie, unsigned short ielen)
{
	u8 *buf=NULL, *pos=NULL;	
	u32 left; 	
	int group_cipher = 0, pairwise_cipher = 0;
	int ret = 0;
#if ( P2P_INCLUDED == 1 )	
	struct wifidirect_info* pwdinfo = &padapter->wdinfo;
#endif

	if((ielen > MAX_WPA_IE_LEN) || (pie == NULL)){
		padapter->securitypriv.wps_phase = _FALSE;	
		if(pie == NULL)	
			return ret;
		else
			return -EINVAL;
	}

	if(ielen)
	{		
		buf = rtw_zmalloc(ielen);
		if (buf == NULL){
			ret =  -ENOMEM;
			goto exit;
		}
	
		_memcpy(buf, pie , ielen);

		//dump
		{
			int i;
			printk("\n wpa_ie(length:%d):\n", ielen);
			for(i=0;i<ielen;i=i+8)
				printk("0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x \n",buf[i],buf[i+1],buf[i+2],buf[i+3],buf[i+4],buf[i+5],buf[i+6],buf[i+7]);
		}
	
		pos = buf;
		if(ielen < RSN_HEADER_LEN){
			RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("Ie len too short %d\n", ielen));
			ret  = -1;
			goto exit;
		}

#if 0
		pos += RSN_HEADER_LEN;
		left  = ielen - RSN_HEADER_LEN;
		
		if (left >= RSN_SELECTOR_LEN){
			pos += RSN_SELECTOR_LEN;
			left -= RSN_SELECTOR_LEN;
		}		
		else if (left > 0){
			RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("Ie length mismatch, %u too much \n", left));
			ret =-1;
			goto exit;
		}
#endif		
		
		if(parse_wpa_ie(buf, ielen, &group_cipher, &pairwise_cipher) == _SUCCESS)
		{
			padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_8021X;
			padapter->securitypriv.ndisauthtype=Ndis802_11AuthModeWPAPSK;
		}
	
		if(parse_wpa2_ie(buf, ielen, &group_cipher, &pairwise_cipher) == _SUCCESS)
		{
			padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_8021X;
			padapter->securitypriv.ndisauthtype=Ndis802_11AuthModeWPA2PSK;		
		}
			
		switch(group_cipher)
		{
			case WPA_CIPHER_NONE:
				padapter->securitypriv.dot118021XGrpPrivacy=_NO_PRIVACY_;
				padapter->securitypriv.ndisencryptstatus=Ndis802_11EncryptionDisabled;
				break;
			case WPA_CIPHER_WEP40:
				padapter->securitypriv.dot118021XGrpPrivacy=_WEP40_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
				break;
			case WPA_CIPHER_TKIP:
				padapter->securitypriv.dot118021XGrpPrivacy=_TKIP_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption2Enabled;
				break;
			case WPA_CIPHER_CCMP:
				padapter->securitypriv.dot118021XGrpPrivacy=_AES_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption3Enabled;
				break;
			case WPA_CIPHER_WEP104:	
				padapter->securitypriv.dot118021XGrpPrivacy=_WEP104_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
				break;
		}

		switch(pairwise_cipher)
		{
			case WPA_CIPHER_NONE:
				padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
				padapter->securitypriv.ndisencryptstatus=Ndis802_11EncryptionDisabled;
				break;
			case WPA_CIPHER_WEP40:
				padapter->securitypriv.dot11PrivacyAlgrthm=_WEP40_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
				break;
			case WPA_CIPHER_TKIP:
				padapter->securitypriv.dot11PrivacyAlgrthm=_TKIP_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption2Enabled;
				break;
			case WPA_CIPHER_CCMP:
				padapter->securitypriv.dot11PrivacyAlgrthm=_AES_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption3Enabled;
				break;
			case WPA_CIPHER_WEP104:	
				padapter->securitypriv.dot11PrivacyAlgrthm=_WEP104_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
				break;
		}
		
		padapter->securitypriv.wps_phase = _FALSE;			
		{//set wps_ie	
			u16 cnt = 0;	
			u8 eid, wps_oui[4]={0x0,0x50,0xf2,0x04};
			 
			while( cnt < ielen )
			{
				eid = buf[cnt];
		
				if((eid==_VENDOR_SPECIFIC_IE_)&&(_memcmp(&buf[cnt+2], wps_oui, 4)==_TRUE))
				{
					printk("SET WPS_IE\n");

					padapter->securitypriv.wps_ie_len = ( (buf[cnt+1]+2) < (MAX_WPA_IE_LEN<<2)) ? (buf[cnt+1]+2):(MAX_WPA_IE_LEN<<2);
					
					_memcpy(padapter->securitypriv.wps_ie, &buf[cnt], padapter->securitypriv.wps_ie_len);
					
					padapter->securitypriv.wps_phase = _TRUE;
					
#if ( P2P_INCLUDED == 1 )					
					if ( pwdinfo->p2p_state == P2P_STATE_GONEGO_OK )
					{
						pwdinfo->p2p_state = P2P_STATE_PROVISIONING_ING;
					}
#endif
					printk("SET WPS_IE, wps_phase==_TRUE\n");

					cnt += buf[cnt+1]+2;
					
					break;
				} else {
					cnt += buf[cnt+1]+2; //goto next	
				}				
			}			
		}		
	}
	
	RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_info_,
		 ("rtw_set_wpa_ie: pairwise_cipher=0x%08x padapter->securitypriv.ndisencryptstatus=%d padapter->securitypriv.ndisauthtype=%d\n",
		  pairwise_cipher, padapter->securitypriv.ndisencryptstatus, padapter->securitypriv.ndisauthtype));
 	
exit:

	if (buf) rtw_mfree(buf, ielen);
	
	return ret;	
}

static int rtw_wx_get_name(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)netdev_priv(dev);
	u16 cap;
	u32 ht_ielen = 0;
	char *p;
	u8 ht_cap=_FALSE;
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	WLAN_BSSID_EX  *pcur_bss = &pmlmepriv->cur_network.network;
	NDIS_802_11_RATES_EX* prates = NULL;

	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("cmd_code=%x\n", info->cmd));

	_func_enter_;	

	if (check_fwstate(pmlmepriv, _FW_LINKED|WIFI_ADHOC_MASTER_STATE) == _TRUE)
	{
		//parsing HT_CAP_IE
		p = get_ie(&pcur_bss->IEs[12], _HT_CAPABILITY_IE_, &ht_ielen, pcur_bss->IELength-12);
		if(p && ht_ielen>0)
		{
			ht_cap = _TRUE;
		}

		prates = &pcur_bss->SupportedRates;

		if (is_cckratesonly_included((u8*)prates) == _TRUE)
		{
			if(ht_cap == _TRUE)
				snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11bn");
			else
				snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11b");
		}
		else if ((is_cckrates_included((u8*)prates)) == _TRUE)
		{
			if(ht_cap == _TRUE)
				snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11bgn");
			else
				snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11bg");
		}
		else
		{
			if(pcur_bss->Configuration.DSConfig > 14)
			{
				if(ht_cap == _TRUE)
					snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11an");
				else
					snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11a");
			}
			else
			{
				if(ht_cap == _TRUE)
					snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11gn");
				else
					snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11g");
			}
		}
	}
	else
	{
		//prates = &padapter->registrypriv.dev_network.SupportedRates;
		//snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11g");
		snprintf(wrqu->name, IFNAMSIZ, "unassociated");
	}

	_func_exit_;

	return 0;
}

static int rtw_wx_set_freq(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{	
	_func_enter_;

	RT_TRACE(_module_rtl871x_mlme_c_, _drv_notice_, ("+rtw_wx_set_freq\n"));

	_func_exit_;
	
	return 0;
}

static int rtw_wx_get_freq(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)netdev_priv(dev);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	WLAN_BSSID_EX  *pcur_bss = &pmlmepriv->cur_network.network;
	
	if(check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE)
	{
		//wrqu->freq.m = ieee80211_wlan_frequencies[pcur_bss->Configuration.DSConfig-1] * 100000;
		wrqu->freq.m = ch2freq(pcur_bss->Configuration.DSConfig) * 100000;
		wrqu->freq.e = 1;
		wrqu->freq.i = pcur_bss->Configuration.DSConfig;

	}
	else{
		wrqu->freq.m = ch2freq(padapter->mlmeextpriv.cur_channel) * 100000;
		wrqu->freq.e = 1;
		wrqu->freq.i = padapter->mlmeextpriv.cur_channel;
	}

	return 0;
}

static int rtw_wx_set_mode(struct net_device *dev, struct iw_request_info *a,
			     union iwreq_data *wrqu, char *b)
{
	_adapter *padapter = (_adapter *)netdev_priv(dev);
	NDIS_802_11_NETWORK_INFRASTRUCTURE networkType ;
	int ret = 0;
	
	_func_enter_;
	
	switch(wrqu->mode)
	{
		case IW_MODE_AUTO:
			networkType = Ndis802_11AutoUnknown;
			printk("set_mode = IW_MODE_AUTO\n");	
			break;				
		case IW_MODE_ADHOC:		
			networkType = Ndis802_11IBSS;
			printk("set_mode = IW_MODE_ADHOC\n");			
			break;
		case IW_MODE_MASTER:		
			networkType = Ndis802_11APMode;
			printk("set_mode = IW_MODE_MASTER\n");
                        //setopmode_cmd(padapter, networkType);	
			break;				
		case IW_MODE_INFRA:
			networkType = Ndis802_11Infrastructure;
			printk("set_mode = IW_MODE_INFRA\n");			
			break;
	
		default :
			ret = -EINVAL;;
			RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("\n Mode: %s is not supported  \n", iw_operation_mode[wrqu->mode]));
			goto exit;
	}
	
/*	
	if(Ndis802_11APMode == networkType)
	{
		setopmode_cmd(padapter, networkType);
	}	
	else
	{
		setopmode_cmd(padapter, Ndis802_11AutoUnknown);	
	}
*/
	
	if (set_802_11_infrastructure_mode(padapter, networkType) ==_FALSE){

		ret = -1;
		goto exit;

	}

	setopmode_cmd(padapter, networkType);

exit:
	
	_func_exit_;
	
	return ret;
	
}

static int rtw_wx_get_mode(struct net_device *dev, struct iw_request_info *a,
			     union iwreq_data *wrqu, char *b)
{
	_adapter *padapter = (_adapter *)netdev_priv(dev);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	
	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,(" rtw_wx_get_mode \n"));

	_func_enter_;
	
	if (check_fwstate(pmlmepriv, WIFI_STATION_STATE) == _TRUE)
	{
		wrqu->mode = IW_MODE_INFRA;
	}
	else if  ((check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == _TRUE) ||
		       (check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE))
		
	{
		wrqu->mode = IW_MODE_ADHOC;
	}
	else if(check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE)
	{
		wrqu->mode = IW_MODE_MASTER;
	}
	else
	{
		wrqu->mode = IW_MODE_AUTO;
	}

	_func_exit_;
	
	return 0;
	
}


static int rtw_wx_set_pmkid(struct net_device *dev,
	                     struct iw_request_info *a,
			     union iwreq_data *wrqu, char *extra)
{

	_adapter    *padapter = (_adapter *)netdev_priv(dev);
	u8          j,blInserted = _FALSE;
	int         intReturn = _FALSE;
	struct mlme_priv  *pmlmepriv = &padapter->mlmepriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
        struct iw_pmksa*  pPMK = ( struct iw_pmksa* ) extra;
        u8     strZeroMacAddress[ ETH_ALEN ] = { 0x00 };
        u8     strIssueBssid[ ETH_ALEN ] = { 0x00 };
        
/*
        struct iw_pmksa
        {
            __u32   cmd;
            struct sockaddr bssid;
            __u8    pmkid[IW_PMKID_LEN];   //IW_PMKID_LEN=16
        }
        There are the BSSID information in the bssid.sa_data array.
        If cmd is IW_PMKSA_FLUSH, it means the wpa_suppplicant wants to clear all the PMKID information.
        If cmd is IW_PMKSA_ADD, it means the wpa_supplicant wants to add a PMKID/BSSID to driver.
        If cmd is IW_PMKSA_REMOVE, it means the wpa_supplicant wants to remove a PMKID/BSSID from driver.
        */

	_memcpy( strIssueBssid, pPMK->bssid.sa_data, ETH_ALEN);
        if ( pPMK->cmd == IW_PMKSA_ADD )
        {
                printk( "[rtw_wx_set_pmkid] IW_PMKSA_ADD!\n" );
                if ( _memcmp( strIssueBssid, strZeroMacAddress, ETH_ALEN ) == _TRUE )
                {
                    return( intReturn );
                }
                else
                {
                    intReturn = _TRUE;
                }
		blInserted = _FALSE;
		
		//overwrite PMKID
		for(j=0 ; j<NUM_PMKID_CACHE; j++)
		{
			if( _memcmp( psecuritypriv->PMKIDList[j].Bssid, strIssueBssid, ETH_ALEN) ==_TRUE )
			{ // BSSID is matched, the same AP => rewrite with new PMKID.
                                
                                printk( "[rtw_wx_set_pmkid] BSSID exists in the PMKList.\n" );

				_memcpy( psecuritypriv->PMKIDList[j].PMKID, pPMK->pmkid, IW_PMKID_LEN);
                                psecuritypriv->PMKIDList[ j ].bUsed = _TRUE;
				psecuritypriv->PMKIDIndex = j+1;
				blInserted = _TRUE;
				break;
			}	
	        }

	        if(!blInserted)
                {
		    // Find a new entry
                    printk( "[rtw_wx_set_pmkid] Use the new entry index = %d for this PMKID.\n",
                            psecuritypriv->PMKIDIndex );

	            _memcpy(psecuritypriv->PMKIDList[psecuritypriv->PMKIDIndex].Bssid, strIssueBssid, ETH_ALEN);
		    _memcpy(psecuritypriv->PMKIDList[psecuritypriv->PMKIDIndex].PMKID, pPMK->pmkid, IW_PMKID_LEN);

                    psecuritypriv->PMKIDList[ psecuritypriv->PMKIDIndex ].bUsed = _TRUE;
		    psecuritypriv->PMKIDIndex++ ;
		    if(psecuritypriv->PMKIDIndex==16)
                    {
		        psecuritypriv->PMKIDIndex =0;
                    }
		}
        }
        else if ( pPMK->cmd == IW_PMKSA_REMOVE )
        {
                printk( "[rtw_wx_set_pmkid] IW_PMKSA_REMOVE!\n" );
                intReturn = _TRUE;
		for(j=0 ; j<NUM_PMKID_CACHE; j++)
		{
			if( _memcmp( psecuritypriv->PMKIDList[j].Bssid, strIssueBssid, ETH_ALEN) ==_TRUE )
			{ // BSSID is matched, the same AP => Remove this PMKID information and reset it. 
                                _memset( psecuritypriv->PMKIDList[ j ].Bssid, 0x00, ETH_ALEN );
                                psecuritypriv->PMKIDList[ j ].bUsed = _FALSE;
				break;
			}	
	        }
        }
        else if ( pPMK->cmd == IW_PMKSA_FLUSH ) 
        {
            printk( "[rtw_wx_set_pmkid] IW_PMKSA_FLUSH!\n" );
            _memset( &psecuritypriv->PMKIDList[ 0 ], 0x00, sizeof( RT_PMKID_LIST ) * NUM_PMKID_CACHE );
            psecuritypriv->PMKIDIndex = 0;
            intReturn = _TRUE;
        }
    return( intReturn );
}

static int rtw_wx_get_sens(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	//_adapter *padapter = netdev_priv(dev);
	
	wrqu->sens.value = 0;
	wrqu->sens.fixed = 0;	/* no auto select */
	wrqu->sens.disabled = 1;
	
	return 0;

}

static int rtw_wx_get_range(struct net_device *dev, 
				struct iw_request_info *info, 
				union iwreq_data *wrqu, char *extra)
{
	struct iw_range *range = (struct iw_range *)extra;
	_adapter *padapter = (_adapter *)netdev_priv(dev);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;

	u16 val;
	int i;
	
	_func_enter_;
	
	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_range. cmd_code=%x\n", info->cmd));

	wrqu->data.length = sizeof(*range);
	_memset(range, 0, sizeof(*range));

	/* Let's try to keep this struct in the same order as in
	 * linux/include/wireless.h
	 */

	/* TODO: See what values we can set, and remove the ones we can't
	 * set, or fill them with some default data.
	 */

	/* ~5 Mb/s real (802.11b) */
	range->throughput = 5 * 1000 * 1000;     

	// TODO: Not used in 802.11b?
//	range->min_nwid;	/* Minimal NWID we are able to set */
	// TODO: Not used in 802.11b?
//	range->max_nwid;	/* Maximal NWID we are able to set */

        /* Old Frequency (backward compat - moved lower ) */
//	range->old_num_channels; 
//	range->old_num_frequency;
//	range->old_freq[6]; /* Filler to keep "version" at the same offset */

	/* signal level threshold range */

	//percent values between 0 and 100.
	range->max_qual.qual = 100;	
	range->max_qual.level = 100;
	range->max_qual.noise = 100;
	range->max_qual.updated = 7; /* Updated all three */


	range->avg_qual.qual = 92; /* > 8% missed beacons is 'bad' */
	/* TODO: Find real 'good' to 'bad' threshol value for RSSI */
	range->avg_qual.level = 20 + -98;
	range->avg_qual.noise = 0;
	range->avg_qual.updated = 7; /* Updated all three */

	range->num_bitrates = RATE_COUNT;

	for (i = 0; i < RATE_COUNT && i < IW_MAX_BITRATES; i++) {
		range->bitrate[i] = rtw_rates[i];
	}

	range->min_frag = MIN_FRAG_THRESHOLD;
	range->max_frag = MAX_FRAG_THRESHOLD;

	range->pm_capa = 0;

	range->we_version_compiled = WIRELESS_EXT;
	range->we_version_source = 16;

//	range->retry_capa;	/* What retry options are supported */
//	range->retry_flags;	/* How to decode max/min retry limit */
//	range->r_time_flags;	/* How to decode max/min retry life */
//	range->min_retry;	/* Minimal number of retries */
//	range->max_retry;	/* Maximal number of retries */
//	range->min_r_time;	/* Minimal retry lifetime */
//	range->max_r_time;	/* Maximal retry lifetime */

	for (i = 0, val = 0; i < NUM_CHANNELS; i++) {

		// Include only legal frequencies for some countries
		if(pmlmeext->channel_set[i].ChannelNum != 0)
		{
			range->freq[val].i = pmlmeext->channel_set[i].ChannelNum;
			range->freq[val].m = ch2freq(pmlmeext->channel_set[i].ChannelNum) * 100000;
			range->freq[val].e = 1;
			val++;
		}

		if (val == IW_MAX_FREQUENCIES)
			break;
	}

	range->num_channels = val;
	range->num_frequency = val;

// Commented by Albert 2009/10/13
// The following code will proivde the security capability to network manager.
// If the driver doesn't provide this capability to network manager,
// the WPA/WPA2 routers can't be choosen in the network manager.

/*
#define IW_SCAN_CAPA_NONE		0x00
#define IW_SCAN_CAPA_ESSID		0x01
#define IW_SCAN_CAPA_BSSID		0x02
#define IW_SCAN_CAPA_CHANNEL	0x04
#define IW_SCAN_CAPA_MODE		0x08
#define IW_SCAN_CAPA_RATE		0x10
#define IW_SCAN_CAPA_TYPE		0x20
#define IW_SCAN_CAPA_TIME		0x40
*/

#if WIRELESS_EXT > 17
	range->enc_capa = IW_ENC_CAPA_WPA|IW_ENC_CAPA_WPA2|
			  IW_ENC_CAPA_CIPHER_TKIP|IW_ENC_CAPA_CIPHER_CCMP;
#endif

#ifdef IW_SCAN_CAPA_ESSID //WIRELESS_EXT > 21
	range->scan_capa = IW_SCAN_CAPA_ESSID | IW_SCAN_CAPA_TYPE |IW_SCAN_CAPA_BSSID|
					IW_SCAN_CAPA_CHANNEL|IW_SCAN_CAPA_MODE|IW_SCAN_CAPA_RATE;
#endif


	_func_exit_;

	return 0;

}

//set bssid flow
//s1. set_802_11_infrastructure_mode()
//s2. set_802_11_authentication_mode()
//s3. set_802_11_encryption_mode()
//s4. set_802_11_bssid()
static int rtw_wx_set_wap(struct net_device *dev,
			 struct iw_request_info *info,
			 union iwreq_data *awrq,
			 char *extra)
{
	uint ret = 0;
	_adapter *padapter = netdev_priv(dev);
	struct sockaddr *temp = (struct sockaddr *)awrq;
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	_list	*phead;
	u8 *dst_bssid, *src_bssid;
	_queue	*queue	= &(pmlmepriv->scanned_queue);
	struct	wlan_network	*pnetwork = NULL;
	NDIS_802_11_AUTHENTICATION_MODE	authmode;

	_func_enter_;
	
	if(!padapter->bup){
		ret = -1;
		goto exit;
	}

	
	if (temp->sa_family != ARPHRD_ETHER){
		ret = -EINVAL;
		goto exit;
	}

	authmode = padapter->securitypriv.ndisauthtype;

       phead = get_list_head(queue);
       pmlmepriv->pscanned = get_next(phead);

	while (1)
	 {
			
		if ((end_of_queue_search(phead, pmlmepriv->pscanned)) == _TRUE)
		{
#if 0		
			ret = -EINVAL;
			goto exit;

			if(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE)
			{
	            		set_802_11_bssid(padapter, temp->sa_data);
	    			goto exit;                    
			}
			else
			{
				ret = -EINVAL;
				goto exit;
			}
#endif

			if (set_802_11_bssid(padapter, temp->sa_data) == _FALSE)
				ret = -1;

	    		goto exit;   			
		}
	
		pnetwork = LIST_CONTAINOR(pmlmepriv->pscanned, struct wlan_network, list);

		pmlmepriv->pscanned = get_next(pmlmepriv->pscanned);

		dst_bssid = pnetwork->network.MacAddress;

		src_bssid = temp->sa_data;

		if ((_memcmp(dst_bssid, src_bssid, ETH_ALEN)) == _TRUE)
		{			
			if(!set_802_11_infrastructure_mode(padapter, pnetwork->network.InfrastructureMode))
			{
				ret = -1;
				goto exit;
			}

				break;			
		}

	}		

	set_802_11_authentication_mode(padapter, authmode);

	//set_802_11_encryption_mode(padapter, padapter->securitypriv.ndisencryptstatus);
	
	if (set_802_11_bssid(padapter, temp->sa_data) == _FALSE) {
		ret = -1;
		goto exit;		
	}	
	
exit:
	
	_func_exit_;
	
	return ret;	
}

static int rtw_wx_get_wap(struct net_device *dev, 
			    struct iw_request_info *info, 
			    union iwreq_data *wrqu, char *extra)
{

	_adapter *padapter = (_adapter *)netdev_priv(dev);	
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	WLAN_BSSID_EX  *pcur_bss = &pmlmepriv->cur_network.network;	
	
	wrqu->ap_addr.sa_family = ARPHRD_ETHER;
	
	_memset(wrqu->ap_addr.sa_data, 0, ETH_ALEN);
	
	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_wap\n"));

	_func_enter_;

	if  ( ((check_fwstate(pmlmepriv, _FW_LINKED)) == _TRUE) || 
			((check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE)) == _TRUE) ||
			((check_fwstate(pmlmepriv, WIFI_AP_STATE)) == _TRUE) )
	{

		_memcpy(wrqu->ap_addr.sa_data, pcur_bss->MacAddress, ETH_ALEN);
	}
	else
	{
	 	_memset(wrqu->ap_addr.sa_data, 0, ETH_ALEN);
	}		

	_func_exit_;
	
	return 0;
	
}

static int rtw_wx_set_mlme(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
#if 0
/* SIOCSIWMLME data */
struct	iw_mlme
{
	__u16		cmd; /* IW_MLME_* */
	__u16		reason_code;
	struct sockaddr	addr;
};
#endif

	int ret=0;
	u16 reason;
	_adapter *padapter = (_adapter *)netdev_priv(dev);
	struct iw_mlme *mlme = (struct iw_mlme *) extra;
	

	if(mlme==NULL)
		return -1;

	reason = cpu_to_le16(mlme->reason_code);

	switch (mlme->cmd) 
	{
		case IW_MLME_DEAUTH:			
				if(!set_802_11_disassociate(padapter))
				ret = -1;						
				break;
				
		case IW_MLME_DISASSOC:			
				if(!set_802_11_disassociate(padapter))
						ret = -1;		
				
				break;
				
		default:
			return -EOPNOTSUPP;
	}
	
	return ret;
	
}

int rfpwrstate_check(_adapter *padapter)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct pwrctrl_priv *pwrctrlpriv = &padapter->pwrctrlpriv;
		
	if( pwrctrlpriv->power_mgnt == PS_MODE_ACTIVE )
		return _TRUE;		
	
	if(rf_off == pwrpriv->current_rfpwrstate )
	{		
#if (DEV_BUS_TYPE==DEV_BUS_USB_INTERFACE)
#ifdef CONFIG_AUTOSUSPEND
		 if(pwrpriv->brfoffbyhw==_TRUE)
		{
			printk("hw still in rf_off state ...........\n");
			return _FAIL;
		}
		else if(padapter->registrypriv.usbss_enable)
		{
			printk("\n %s call autoresume_enter....\n",__FUNCTION__);	
			if(_FAIL ==  autoresume_enter(padapter))
			{
				printk("======> autoresume fail.............\n");
				return _FAIL;			
			}	
		}
		
		else
#endif
#endif
		{
#ifdef CONFIG_IPS
			printk("\n %s call ips_leave....\n",__FUNCTION__);				
			if(_FAIL ==  ips_leave(padapter))
			{
				printk("======> ips_leave fail.............\n");
				return _FAIL;			
			}
#endif
		}
			
	}
	return _SUCCESS;

}

static int rtw_wx_set_scan(struct net_device *dev, struct iw_request_info *a,
			     union iwreq_data *wrqu, char *extra)
{
	u8 _status;
	int ret = 0;	
	_adapter *padapter = (_adapter *)netdev_priv(dev);		
	struct mlme_priv *pmlmepriv= &padapter->mlmepriv;
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
#if ( P2P_INCLUDED == 1 )	
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);	
#endif
	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_set_scan\n"));

_func_enter_;

#ifdef CONFIG_MP_INCLUDED
	if (check_fwstate(pmlmepriv, WIFI_MP_STATE) == _TRUE)
	{
		ret = -1;
		goto exit;
	}
#endif

	if(_FAIL == rfpwrstate_check(padapter))
	{
		ret= _FAIL;
		goto exit;
	}

	if(padapter->bDriverStopped){
           printk("bDriverStopped=%d\n", padapter->bDriverStopped);
		ret= -1;
		goto exit;
	}
	
	if(!padapter->bup){
		ret = -1;
		goto exit;
	}
	
	if (padapter->hw_init_completed==_FALSE){
		ret = -1;
		goto exit;
	}

	// When Busy Traffic, driver do not site survey. So driver return success.
	// wpa_supplicant will not issue SIOCSIWSCAN cmd again after scan timeout.
	// modify by thomas 2011-02-22.
	if (pmlmepriv->LinkDetectInfo.bBusyTraffic == _TRUE)
	{
		goto exit;
	} 

	if (check_fwstate(pmlmepriv, _FW_UNDER_SURVEY|_FW_UNDER_LINKING) == _TRUE)
	{
		goto exit;
	} 

//	Mareded by Albert 20101103
//	For the DMP WiFi Display project, the driver won't to scan because
//	the pmlmepriv->scan_interval is always equal to 3.
//	So, the wpa_supplicant won't find out the WPS SoftAP.

/*
	if(pmlmepriv->scan_interval>10)
		pmlmepriv->scan_interval = 0;

	if(pmlmepriv->scan_interval > 0)
	{
		printk("scan done\n");
		ret = 0;
		goto exit;
	}
		
*/
#if ( P2P_INCLUDED == 1 )
	if ( pwdinfo->p2p_state == P2P_STATE_LISTEN )
	{
		pwdinfo->p2p_state = P2P_STATE_FIND_PHASE_SEARCH;
		pwdinfo->find_phase_state_exchange_cnt = 0;
	}
#endif

#if WIRELESS_EXT >= 17
	if (wrqu->data.length == sizeof(struct iw_scan_req)) 
	{
		struct iw_scan_req *req = (struct iw_scan_req *)extra;
	
		if (wrqu->data.flags & IW_SCAN_THIS_ESSID)
		{
			NDIS_802_11_SSID ssid;
			_irqL	irqL;
			int len = min((int)req->essid_len, IW_ESSID_MAX_SIZE);

			_memset((unsigned char*)&ssid, 0, sizeof(NDIS_802_11_SSID));

			_memcpy(ssid.Ssid, req->essid, len);
			ssid.SsidLength = len;	

			printk("IW_SCAN_THIS_ESSID, ssid=%s, len=%d\n", req->essid, req->essid_len);
		
			_enter_critical_bh(&pmlmepriv->lock, &irqL);				
		
			_status = sitesurvey_cmd(padapter, &ssid);
		
			_exit_critical_bh(&pmlmepriv->lock, &irqL);
			
		}
		else if (req->scan_type == IW_SCAN_TYPE_PASSIVE)
		{
			printk("rtw_wx_set_scan, req->scan_type == IW_SCAN_TYPE_PASSIVE\n");
		}
		
	}
	else
#endif
	{
		_status = set_802_11_bssid_list_scan(padapter);
	}

	if(_status == _FALSE)
		ret = -1;

exit:

_func_exit_;

	return ret;	
}

static int rtw_wx_get_scan(struct net_device *dev, struct iw_request_info *a,
			     union iwreq_data *wrqu, char *extra)
{
	_irqL	irqL;
	_list					*plist, *phead;
	_adapter *padapter = (_adapter *)netdev_priv(dev);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);	
	_queue				*queue	= &(pmlmepriv->scanned_queue);	
	struct	wlan_network	*pnetwork = NULL;
	char *ev = extra;
	char *stop = ev + wrqu->data.length;
	u32 ret = 0;	
	u32 cnt=0;
	u32 wait_for_surveydone;
#if ( P2P_INCLUDED == 1 )
	struct	wifidirect_info*	pwdinfo = &padapter->wdinfo;
#endif
	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_scan\n"));
	RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_, (" Start of Query SIOCGIWSCAN .\n"));

	_func_enter_;
	
	if(padapter->pwrctrlpriv.brfoffbyhw && padapter->bDriverStopped)
	{
		return -EINVAL;
	}
  
  	// 20110214 Commented by Jeff: 
  	// In rockchip 2818 platforms with low-speed IO, the UI will not show scan list bause of this busy waiting
  	#ifndef CONFIG_ANDROID
	#if ( P2P_INCLUDED == 1 )
	if ( pwdinfo->p2p_state != P2P_STATE_NONE )
	{
		wait_for_surveydone = 100;
	}
	else
	{
		wait_for_surveydone = 200;
	}
	#else
	{
		wait_for_surveydone = 100;
	}
	#endif
 	while((check_fwstate(pmlmepriv, (_FW_UNDER_SURVEY|_FW_UNDER_LINKING))) == _TRUE)
	{	
		msleep_os(30);
		cnt++;
		if(cnt > wait_for_surveydone )
			break;
	}
	#endif

	_enter_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	phead = get_list_head(queue);
	plist = get_next(phead);
       
	while(1)
	{
		if (end_of_queue_search(phead,plist)== _TRUE)
			break;

		if((stop - ev) < SCAN_ITEM_SIZE) {
			ret = -E2BIG;
			break;
		}

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);
                
		ev=translate_scan(padapter, a, pnetwork, ev, stop);

		plist = get_next(plist);
	
	}        

	_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

       wrqu->data.length = ev-extra;
	wrqu->data.flags = 0;
	
exit:		
	
	_func_exit_;	
	
	return ret ;
	
}

//set ssid flow
//s1. set_802_11_infrastructure_mode()
//s2. set_802_11_authenticaion_mode()
//s3. set_802_11_encryption_mode()
//s4. set_802_11_ssid()
static int rtw_wx_set_essid(struct net_device *dev, 
			      struct iw_request_info *a,
			      union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	_queue *queue = &pmlmepriv->scanned_queue;
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	_list *phead;
	s8 status = _TRUE;
	struct wlan_network *pnetwork = NULL;

	NDIS_802_11_AUTHENTICATION_MODE authmode;	
	NDIS_802_11_SSID ndis_ssid;	
	u8 *dst_ssid, *src_ssid;

	uint ret = 0, len;

	_func_enter_;
	
	RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_info_,
		 ("+rtw_wx_set_essid: fw_state=0x%08x\n", get_fwstate(pmlmepriv)));
	if(_FAIL == rfpwrstate_check(padapter))
	{		
		return _FAIL;
	}

	if(!padapter->bup){
		ret = -1;
		goto exit;
	}

#if WIRELESS_EXT <= 20
	if ((wrqu->essid.length-1) > IW_ESSID_MAX_SIZE){
#else
	if (wrqu->essid.length > IW_ESSID_MAX_SIZE){
#endif
		ret= -E2BIG;
		goto exit;
	}
	
	if(check_fwstate(pmlmepriv, WIFI_AP_STATE)) {
		ret = -1;
		goto exit;
	}		
	
	authmode = padapter->securitypriv.ndisauthtype;
	printk("=>%s\n",__FUNCTION__);
	if (wrqu->essid.flags && wrqu->essid.length)
	{
		// Commented by Albert 20100519
		// We got the codes in "set_info" function of iwconfig source code.
		//	=========================================
		//	wrq.u.essid.length = strlen(essid) + 1;
	  	//	if(we_kernel_version > 20)
		//		wrq.u.essid.length--;
		//	=========================================
		//	That means, if the WIRELESS_EXT less than or equal to 20, the correct ssid len should subtract 1.
#if WIRELESS_EXT <= 20
		len = ((wrqu->essid.length-1) < IW_ESSID_MAX_SIZE) ? (wrqu->essid.length-1) : IW_ESSID_MAX_SIZE;
#else
		len = (wrqu->essid.length < IW_ESSID_MAX_SIZE) ? wrqu->essid.length : IW_ESSID_MAX_SIZE;
#endif

		printk("ssid=%s, len=%d\n", extra, wrqu->essid.length);

		_memset(&ndis_ssid, 0, sizeof(NDIS_802_11_SSID));
		ndis_ssid.SsidLength = len;
		_memcpy(ndis_ssid.Ssid, extra, len);		
		src_ssid = ndis_ssid.Ssid;
		
		RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_info_, ("rtw_wx_set_essid: ssid=[%s]\n", src_ssid));
		
	       phead = get_list_head(queue);
              pmlmepriv->pscanned = get_next(phead);

		while (1)
		{			
			if (end_of_queue_search(phead, pmlmepriv->pscanned) == _TRUE)
			{
#if 0			
				if(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE)
				{
	            			set_802_11_ssid(padapter, &ndis_ssid);

		    			goto exit;                    
				}
				else
				{
					RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_,("rtw_wx_set_ssid(): scanned_queue is empty\n"));
					ret = -EINVAL;
					goto exit;
				}
#endif			
			        RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_warning_,
					 ("rtw_wx_set_essid: scan_q is empty, set ssid to check if scanning again!\n"));

				break;
			}
	
			pnetwork = LIST_CONTAINOR(pmlmepriv->pscanned, struct wlan_network, list);

			pmlmepriv->pscanned = get_next(pmlmepriv->pscanned);

			dst_ssid = pnetwork->network.Ssid.Ssid;

			RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_info_,
				 ("rtw_wx_set_essid: dst_ssid=%s\n",
				  pnetwork->network.Ssid.Ssid));

			if ((_memcmp(dst_ssid, src_ssid, ndis_ssid.SsidLength) == _TRUE) &&
				(pnetwork->network.Ssid.SsidLength==ndis_ssid.SsidLength))
			{
				RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_info_,
					 ("rtw_wx_set_essid: find match, set infra mode\n"));
				
				if(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE)
				{
					if(pnetwork->network.InfrastructureMode != pmlmepriv->cur_network.network.InfrastructureMode)
						continue;
				}	
					
				if (set_802_11_infrastructure_mode(padapter, pnetwork->network.InfrastructureMode) == _FALSE)
				{
					ret = -1;
					goto exit;
				}

				break;			
			}
		}

		RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_info_,
			 ("set ssid: set_802_11_auth. mode=%d\n", authmode));
		set_802_11_authentication_mode(padapter, authmode);
		//set_802_11_encryption_mode(padapter, padapter->securitypriv.ndisencryptstatus);
		if (set_802_11_ssid(padapter, &ndis_ssid) == _FALSE) {
			ret = -1;
			goto exit;
		}	
	}			
	printk("<=%s\n",__FUNCTION__);
exit:
	
	_func_exit_;
	
	return ret;	
}

static int rtw_wx_get_essid(struct net_device *dev, 
			      struct iw_request_info *a,
			      union iwreq_data *wrqu, char *extra)
{
	u32 len,ret = 0;
	_adapter *padapter = (_adapter *)netdev_priv(dev);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	WLAN_BSSID_EX  *pcur_bss = &pmlmepriv->cur_network.network;

	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_essid\n"));

	_func_enter_;

	if ( (check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE) ||
	      (check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == _TRUE))
	{
		len = pcur_bss->Ssid.SsidLength;

		wrqu->essid.length = len;
			
		_memcpy(extra, pcur_bss->Ssid.Ssid, len);

		wrqu->essid.flags = 1;
	}
	else
	{
		ret = -1;
		goto exit;
	}

exit:
	
	_func_exit_;
	
	return ret;
	
}

static int rtw_wx_set_rate(struct net_device *dev, 
			      struct iw_request_info *a,
			      union iwreq_data *wrqu, char *extra)
{
	int	i, ret = 0;
	_adapter *padapter = (_adapter *)netdev_priv(dev);
	u8	datarates[NumRates];
	u32	target_rate = wrqu->bitrate.value;
	u32	fixed = wrqu->bitrate.fixed;
	u32	ratevalue = 0;
	 u8 mpdatarate[NumRates]={11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0xff};

_func_enter_;

	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,(" rtw_wx_set_rate \n"));
	RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_,("target_rate = %d, fixed = %d\n",target_rate,fixed));
	
	if(target_rate == -1){
		ratevalue = 11;
		goto set_rate;
	}
	target_rate = target_rate/100000;

	switch(target_rate){
		case 10:
			ratevalue = 0;
			break;
		case 20:
			ratevalue = 1;
			break;
		case 55:
			ratevalue = 2;
			break;
		case 60:
			ratevalue = 3;
			break;
		case 90:
			ratevalue = 4;
			break;
		case 110:
			ratevalue = 5;
			break;
		case 120:
			ratevalue = 6;
			break;
		case 180:
			ratevalue = 7;
			break;
		case 240:
			ratevalue = 8;
			break;
		case 360:
			ratevalue = 9;
			break;
		case 480:
			ratevalue = 10;
			break;
		case 540:
			ratevalue = 11;
			break;
		default:
			ratevalue = 11;
			break;
	}

set_rate:

	for(i=0; i<NumRates; i++)
	{
		if(ratevalue==mpdatarate[i])
		{
			datarates[i] = mpdatarate[i];
			if(fixed == 0)
				break;
		}
		else{
			datarates[i] = 0xff;
		}

		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_,("datarate_inx=%d\n",datarates[i]));
	}

	if( setdatarate_cmd(padapter, datarates) !=_SUCCESS){
		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("rtw_wx_set_rate Fail!!!\n"));
		ret = -1;
	}

_func_exit_;

	return ret;
}

static int rtw_wx_get_rate(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{	
	int i;
	u8 *p;
	u16 rate = 0, max_rate = 0, ht_cap=_FALSE;
	u32 ht_ielen = 0;	
	_adapter *padapter = (_adapter *)netdev_priv(dev);
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	WLAN_BSSID_EX  *pcur_bss = &pmlmepriv->cur_network.network;
	struct ieee80211_ht_cap *pht_capie;
	u8	bw_40MHz=0, short_GI=0;
	u16	mcs_rate=0;
	u8	rf_type = 0;
	struct registry_priv *pregpriv = &padapter->registrypriv;


	i=0;
#ifdef CONFIG_MP_INCLUDED
	if (check_fwstate(pmlmepriv, WIFI_MP_STATE) == _TRUE)
		return -1;
#endif
	if((check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE) || (check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == _TRUE))
	{
		p = get_ie(&pcur_bss->IEs[12], _HT_CAPABILITY_IE_, &ht_ielen, pcur_bss->IELength-12);
		if(p && ht_ielen>0)
		{
			ht_cap = _TRUE;	

			pht_capie = (struct ieee80211_ht_cap *)(p+2);
		
			_memcpy(&mcs_rate , pht_capie->supp_mcs_set, 2);

			bw_40MHz = (pht_capie->cap_info&IEEE80211_HT_CAP_SUP_WIDTH) ? 1:0;

			short_GI = (pht_capie->cap_info&(IEEE80211_HT_CAP_SGI_20|IEEE80211_HT_CAP_SGI_40)) ? 1:0;
		}

		while( (pcur_bss->SupportedRates[i]!=0) && (pcur_bss->SupportedRates[i]!=0xFF))
		{
			rate = pcur_bss->SupportedRates[i]&0x7F;
			if(rate>max_rate)
				max_rate = rate;

			wrqu->bitrate.fixed = 0;	/* no auto select */
			//wrqu->bitrate.disabled = 1/;
		
			i++;
		}
	
		if(ht_cap == _TRUE)
		{
#if 0 //have some issue,neet to debug - 20101008-georgia
			if(mcs_rate&0x8000)//MCS15
			{
				max_rate = (bw_40MHz) ? ((short_GI)?300:270):((short_GI)?144:130);
			
			}
			else if(mcs_rate&0x0080)//MCS7
			{
				max_rate = (bw_40MHz) ? ((short_GI)?150:135):((short_GI)?72:65);
			}
			else//default MCS7
			{
				//printk("wx_get_rate, mcs_rate_bitmap=0x%x\n", mcs_rate);
				max_rate = (bw_40MHz) ? ((short_GI)?150:135):((short_GI)?72:65);
			}
#else
			padapter->HalFunc.GetHwRegHandler(padapter, HW_VAR_RF_TYPE, (u8 *)(&rf_type));
			if(rf_type == RF_1T1R)
				max_rate = (bw_40MHz) ? ((short_GI)?150:135):((short_GI)?72:65);				
			else
				max_rate = (bw_40MHz) ? ((short_GI)?300:270):((short_GI)?144:130);
#endif
			max_rate = max_rate*2;//Mbps/2			
			wrqu->bitrate.value = max_rate*500000;
			
		}
		else
		{
			wrqu->bitrate.value = max_rate*500000;
		}

	}
	else
	{
		return -1;
	}

	return 0;
	
}

static int rtw_wx_get_rts(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)netdev_priv(dev);
	
	_func_enter_;
	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,(" rtw_wx_get_rts \n"));
	
	wrqu->rts.value = padapter->registrypriv.rts_thresh;
	wrqu->rts.fixed = 0;	/* no auto select */
	//wrqu->rts.disabled = (wrqu->rts.value == DEFAULT_RTS_THRESHOLD);
	
	_func_exit_;
	
	return 0;
}

static int rtw_wx_set_frag(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = netdev_priv(dev);

	_func_enter_;
	
	if (wrqu->frag.disabled)
		padapter->xmitpriv.frag_len = MAX_FRAG_THRESHOLD;
	else {
		if (wrqu->frag.value < MIN_FRAG_THRESHOLD ||
		    wrqu->frag.value > MAX_FRAG_THRESHOLD)
			return -EINVAL;
		
		padapter->xmitpriv.frag_len = wrqu->frag.value & ~0x1;
	}
	
	_func_exit_;
	
	return 0;
	
}


static int rtw_wx_get_frag(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = netdev_priv(dev);
	
	_func_enter_;
	
	wrqu->frag.value = padapter->xmitpriv.frag_len;
	wrqu->frag.fixed = 0;	/* no auto select */
	//wrqu->frag.disabled = (wrqu->frag.value == DEFAULT_FRAG_THRESHOLD);
	
	_func_exit_;
	
	return 0;
}

static int rtw_wx_get_retry(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	//_adapter *padapter = netdev_priv(dev);
	
	wrqu->retry.value = 7;
	wrqu->retry.fixed = 0;	/* no auto select */
	wrqu->retry.disabled = 1;
	
	return 0;

}	

#if 0
#define IW_ENCODE_INDEX		0x00FF	/* Token index (if needed) */
#define IW_ENCODE_FLAGS		0xFF00	/* Flags defined below */
#define IW_ENCODE_MODE		0xF000	/* Modes defined below */
#define IW_ENCODE_DISABLED	0x8000	/* Encoding disabled */
#define IW_ENCODE_ENABLED	0x0000	/* Encoding enabled */
#define IW_ENCODE_RESTRICTED	0x4000	/* Refuse non-encoded packets */
#define IW_ENCODE_OPEN		0x2000	/* Accept non-encoded packets */
#define IW_ENCODE_NOKEY		0x0800  /* Key is write only, so not present */
#define IW_ENCODE_TEMP		0x0400  /* Temporary key */
/*
iwconfig wlan0 key on -> flags = 0x6001 -> maybe it means auto
iwconfig wlan0 key off -> flags = 0x8800
iwconfig wlan0 key open -> flags = 0x2800
iwconfig wlan0 key open 1234567890 -> flags = 0x2000
iwconfig wlan0 key restricted -> flags = 0x4800
iwconfig wlan0 key open [3] 1234567890 -> flags = 0x2003
iwconfig wlan0 key restricted [2] 1234567890 -> flags = 0x4002
iwconfig wlan0 key open [3] -> flags = 0x2803
iwconfig wlan0 key restricted [2] -> flags = 0x4802
*/
#endif

static int rtw_wx_set_enc(struct net_device *dev, 
			    struct iw_request_info *info, 
			    union iwreq_data *wrqu, char *keybuf)
{	
	u32 key, ret = 0;
	u32 keyindex_provided;
	NDIS_802_11_WEP	 wep;	
	NDIS_802_11_AUTHENTICATION_MODE authmode;

	struct iw_point *erq = &(wrqu->encoding);
	_adapter *padapter = netdev_priv(dev);

	printk("+rtw_wx_set_enc, flags=0x%x\n", erq->flags);

	_memset(&wep, 0, sizeof(NDIS_802_11_WEP));
	
	key = erq->flags & IW_ENCODE_INDEX;
	
	_func_enter_;	

	if (erq->flags & IW_ENCODE_DISABLED)
	{
		printk("EncryptionDisabled\n");
		padapter->securitypriv.ndisencryptstatus = Ndis802_11EncryptionDisabled;
		padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
		padapter->securitypriv.dot118021XGrpPrivacy=_NO_PRIVACY_;
		padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_Open; //open system
  		authmode = Ndis802_11AuthModeOpen;
		padapter->securitypriv.ndisauthtype=authmode;
     		
		goto exit;
	}

	if (key) {
		if (key > WEP_KEYS)
			return -EINVAL;
		key--;
		keyindex_provided = 1;
	} 
	else
	{
		keyindex_provided = 0;
		key = padapter->securitypriv.dot11PrivacyKeyIndex;
		printk("rtw_wx_set_enc, key=%d\n", key);
	}
	
	//set authentication mode	
	if(erq->flags & IW_ENCODE_OPEN)
	{
		printk("rtw_wx_set_enc():IW_ENCODE_OPEN\n");
		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;//Ndis802_11EncryptionDisabled;

#ifdef CONFIG_PLATFORM_MT53XX
		padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Auto;
#else
		padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_Open;
#endif

		padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
		padapter->securitypriv.dot118021XGrpPrivacy=_NO_PRIVACY_;
  		authmode = Ndis802_11AuthModeOpen;
		padapter->securitypriv.ndisauthtype=authmode;
	}	
	else if(erq->flags & IW_ENCODE_RESTRICTED)
	{		
		printk("rtw_wx_set_enc():IW_ENCODE_RESTRICTED\n");
		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;

#ifdef CONFIG_PLATFORM_MT53XX
		padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Auto;
#else
		padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_Shared;
#endif

		padapter->securitypriv.dot11PrivacyAlgrthm=_WEP40_;
		padapter->securitypriv.dot118021XGrpPrivacy=_WEP40_;			
		authmode = Ndis802_11AuthModeShared;
		padapter->securitypriv.ndisauthtype=authmode;
	}
	else
	{
		printk("rtw_wx_set_enc():erq->flags=0x%x\n", erq->flags);

		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;//Ndis802_11EncryptionDisabled;
		padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_Open; //open system
		padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
		padapter->securitypriv.dot118021XGrpPrivacy=_NO_PRIVACY_;
  		authmode = Ndis802_11AuthModeOpen;
		padapter->securitypriv.ndisauthtype=authmode;
	}
	
	wep.KeyIndex = key;
	if (erq->length > 0)
	{
		wep.KeyLength = erq->length <= 5 ? 5 : 13;

		wep.Length = wep.KeyLength + FIELD_OFFSET(NDIS_802_11_WEP, KeyMaterial);
	}
	else
	{
		wep.KeyLength = 0 ;
		
		if(keyindex_provided == 1)// set key_id only, no given KeyMaterial(erq->length==0).
		{
			padapter->securitypriv.dot11PrivacyKeyIndex = key;

			printk("(keyindex_provided == 1), keyid=%d, key_len=%d\n", key, padapter->securitypriv.dot11DefKeylen[key]);

			switch(padapter->securitypriv.dot11DefKeylen[key])
			{
				case 5:
					padapter->securitypriv.dot11PrivacyAlgrthm=_WEP40_;					
					break;
				case 13:
					padapter->securitypriv.dot11PrivacyAlgrthm=_WEP104_;					
					break;
				default:
					padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;					
					break;
			}
				
			goto exit;
			
		}
		
	}

	wep.KeyIndex |= 0x80000000;

	_memcpy(wep.KeyMaterial, keybuf, wep.KeyLength);
	
	if (set_802_11_add_wep(padapter, &wep) == _FALSE) {
		ret = -EOPNOTSUPP;
		goto exit;
	}	

exit:
	
	_func_exit_;
	
	return ret;
	
}

static int rtw_wx_get_enc(struct net_device *dev, 
			    struct iw_request_info *info, 
			    union iwreq_data *wrqu, char *keybuf)
{
	uint key, ret =0;
	_adapter *padapter = netdev_priv(dev);
	struct iw_point *erq = &(wrqu->encoding);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);

	_func_enter_;
	
	if(check_fwstate(pmlmepriv, _FW_LINKED) != _TRUE)
	{
		 if(check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) != _TRUE)
		 {
		erq->length = 0;
		erq->flags |= IW_ENCODE_DISABLED;
		return 0;
	}	
	}	

	
	key = erq->flags & IW_ENCODE_INDEX;

	if (key) {
		if (key > WEP_KEYS)
			return -EINVAL;
		key--;
	} else
	{
		key = padapter->securitypriv.dot11PrivacyKeyIndex;
	}	

	erq->flags = key + 1;

	//if(padapter->securitypriv.ndisauthtype == Ndis802_11AuthModeOpen)
	//{
	//      erq->flags |= IW_ENCODE_OPEN;
	//}	  
	
	switch(padapter->securitypriv.ndisencryptstatus)
	{
		case Ndis802_11EncryptionNotSupported:
		case Ndis802_11EncryptionDisabled:

		erq->length = 0;
		erq->flags |= IW_ENCODE_DISABLED;
	
		break;
		
		case Ndis802_11Encryption1Enabled:					
		
		erq->length = padapter->securitypriv.dot11DefKeylen[key];		

		if(erq->length)
		{
			_memcpy(keybuf, padapter->securitypriv.dot11DefKey[key].skey, padapter->securitypriv.dot11DefKeylen[key]);
		
		erq->flags |= IW_ENCODE_ENABLED;

			if(padapter->securitypriv.ndisauthtype == Ndis802_11AuthModeOpen)
			{
	     			erq->flags |= IW_ENCODE_OPEN;
			}
			else if(padapter->securitypriv.ndisauthtype == Ndis802_11AuthModeShared)
			{
		erq->flags |= IW_ENCODE_RESTRICTED;
			}	
		}	
		else
		{
			erq->length = 0;
			erq->flags |= IW_ENCODE_DISABLED;
		}

		break;

		case Ndis802_11Encryption2Enabled:
		case Ndis802_11Encryption3Enabled:

		erq->length = 16;
		erq->flags |= (IW_ENCODE_ENABLED | IW_ENCODE_OPEN | IW_ENCODE_NOKEY);

		break;
	
		default:
		erq->length = 0;
		erq->flags |= IW_ENCODE_DISABLED;

		break;
		
	}
	
	_func_exit_;
	
	return ret;
	
}				     

static int rtw_wx_get_power(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	//_adapter *padapter = netdev_priv(dev);
	
	wrqu->power.value = 0;
	wrqu->power.fixed = 0;	/* no auto select */
	wrqu->power.disabled = 1;
	
	return 0;

}

static int rtw_wx_set_gen_ie(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	int ret;
	_adapter *padapter = netdev_priv(dev);
	
       ret = rtw_set_wpa_ie(padapter, extra, wrqu->data.length);
	   
	return ret;
}	

static int rtw_wx_set_auth(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = netdev_priv(dev);
	struct iw_param *param = (struct iw_param*)&(wrqu->param);
	int ret = 0;
	
	switch (param->flags & IW_AUTH_INDEX) {
	case IW_AUTH_WPA_VERSION:
		break;
	case IW_AUTH_CIPHER_PAIRWISE:
		
		break;
	case IW_AUTH_CIPHER_GROUP:
		
		break;
	case IW_AUTH_KEY_MGMT:
		/*
		 *  ??? does not use these parameters
		 */
		break;

	case IW_AUTH_TKIP_COUNTERMEASURES:
        {
	    if ( param->value )
            {  // wpa_supplicant is enabling the tkip countermeasure.
               padapter->securitypriv.btkip_countermeasure = _TRUE; 
            }
            else
            {  // wpa_supplicant is disabling the tkip countermeasure.
               padapter->securitypriv.btkip_countermeasure = _FALSE; 
            }
		break;
        }
	case IW_AUTH_DROP_UNENCRYPTED:
		{
			/* HACK:
			 *
			 * wpa_supplicant calls set_wpa_enabled when the driver
			 * is loaded and unloaded, regardless of if WPA is being
			 * used.  No other calls are made which can be used to
			 * determine if encryption will be used or not prior to
			 * association being expected.  If encryption is not being
			 * used, drop_unencrypted is set to false, else true -- we
			 * can use this to determine if the CAP_PRIVACY_ON bit should
			 * be set.
			 */

			if(padapter->securitypriv.ndisencryptstatus == Ndis802_11Encryption1Enabled)
			{
				break;//it means init value, or using wep, ndisencryptstatus = Ndis802_11Encryption1Enabled, 
						// then it needn't reset it;
			}
			
			if(param->value){
				padapter->securitypriv.ndisencryptstatus = Ndis802_11EncryptionDisabled;
				padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
				padapter->securitypriv.dot118021XGrpPrivacy=_NO_PRIVACY_;
				padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_Open; //open system
				padapter->securitypriv.ndisauthtype=Ndis802_11AuthModeOpen;
			}
			
			break;
		}

	case IW_AUTH_80211_AUTH_ALG:

		#if defined(CONFIG_ANDROID) //&& !defined(CONFIG_PLATFORM_ROCKCHIPS)
		/*
		 *  It's the starting point of a link layer connection using wpa_supplicant
		*/
		if(check_fwstate(&padapter->mlmepriv, _FW_LINKED)) {
			disassoc_cmd(padapter);
			DBG_871X("%s...call rtw_indicate_disconnect\n ",__FUNCTION__);
			indicate_disconnect(padapter);
			free_assoc_resources(padapter);
		}
		#endif


		ret = wpa_set_auth_algs(dev, (u32)param->value);		
	
		break;

	case IW_AUTH_WPA_ENABLED:

		//if(param->value)
		//	padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_8021X; //802.1x
		//else
		//	padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Open;//open system
		
		//_disassociate(priv);
		
		break;

	case IW_AUTH_RX_UNENCRYPTED_EAPOL:
		//ieee->ieee802_1x = param->value;
		break;

	case IW_AUTH_PRIVACY_INVOKED:
		//ieee->privacy_invoked = param->value;
		break;

	default:
		return -EOPNOTSUPP;
		
	}
	
	return ret;
	
}

static int rtw_wx_set_enc_ext(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	char *alg_name;
	u32 param_len;
	struct ieee_param *param = NULL;
	struct iw_point *pencoding = &wrqu->encoding;
 	struct iw_encode_ext *pext = (struct iw_encode_ext *)extra;
	int ret=0;

	param_len = sizeof(struct ieee_param) + pext->key_len;
	param = (struct ieee_param *)rtw_malloc(param_len);
	if (param == NULL)
		return -1;
	
	_memset(param, 0, param_len);

	param->cmd = IEEE_CMD_SET_ENCRYPTION;
	_memset(param->sta_addr, 0xff, ETH_ALEN);


	switch (pext->alg) {
	case IW_ENCODE_ALG_NONE:
		//todo: remove key 
		//remove = 1;	
		alg_name = "none";
		break;
	case IW_ENCODE_ALG_WEP:
		alg_name = "WEP";
		break;
	case IW_ENCODE_ALG_TKIP:
		alg_name = "TKIP";
		break;
	case IW_ENCODE_ALG_CCMP:
		alg_name = "CCMP";
		break;
	default:	
		return -1;
	}
	
	strncpy((char *)param->u.crypt.alg, alg_name, IEEE_CRYPT_ALG_NAME_LEN);

	
	if(pext->ext_flags & IW_ENCODE_EXT_GROUP_KEY)//?
	{
		param->u.crypt.set_tx = 0;
	}

	if (pext->ext_flags & IW_ENCODE_EXT_SET_TX_KEY)//?
	{
		param->u.crypt.set_tx = 1;
	}

	param->u.crypt.idx = (pencoding->flags&0x00FF) -1 ;
	
	if (pext->ext_flags & IW_ENCODE_EXT_RX_SEQ_VALID) 
	{	
		_memcpy(param->u.crypt.seq, pext->rx_seq, 8);
	}

	if(pext->key_len)
	{
		param->u.crypt.key_len = pext->key_len;
		//_memcpy(param + 1, pext + 1, pext->key_len);
		_memcpy(param->u.crypt.key, pext + 1, pext->key_len);
	}	

	
	if (pencoding->flags & IW_ENCODE_DISABLED)
	{		
		//todo: remove key 
		//remove = 1;		
	}	
	
	ret =  wpa_set_encryption(dev, param, param_len);	
	

	if(param)
	{
		rtw_mfree((u8*)param, param_len);
	}
		
	
	return ret;		

}


static int rtw_wx_get_nick(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{	
	// _adapter *padapter = netdev_priv(dev);
	 //struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	 //struct security_priv *psecuritypriv = &padapter->securitypriv;

	if(extra)
	{
		wrqu->data.length = 14;
		wrqu->data.flags = 1;
		_memcpy(extra, "<WIFI@REALTEK>", 14);
	}

	//kill_pid(find_vpid(pid), SIGUSR1, 1); //for test

	//dump debug info here	
/*
	u32 dot11AuthAlgrthm;		// 802.11 auth, could be open, shared, and 8021x
	u32 dot11PrivacyAlgrthm;	// This specify the privacy for shared auth. algorithm.
	u32 dot118021XGrpPrivacy;	// This specify the privacy algthm. used for Grp key 
	u32 ndisauthtype;
	u32 ndisencryptstatus;
*/

	//printk("auth_alg=0x%x, enc_alg=0x%x, auth_type=0x%x, enc_type=0x%x\n", 
	//		psecuritypriv->dot11AuthAlgrthm, psecuritypriv->dot11PrivacyAlgrthm,
	//		psecuritypriv->ndisauthtype, psecuritypriv->ndisencryptstatus);
	
	//printk("enc_alg=0x%x\n", psecuritypriv->dot11PrivacyAlgrthm);
	//printk("auth_type=0x%x\n", psecuritypriv->ndisauthtype);
	//printk("enc_type=0x%x\n", psecuritypriv->ndisencryptstatus);

#if 0
	printk("dbg(0x210)=0x%x\n", read32(padapter, 0x210));
	printk("dbg(0x608)=0x%x\n", read32(padapter, 0x608));
	printk("dbg(0x280)=0x%x\n", read32(padapter, 0x280));
	printk("dbg(0x284)=0x%x\n", read32(padapter, 0x284));
	printk("dbg(0x288)=0x%x\n", read32(padapter, 0x288));
	
	printk("dbg(0x664)=0x%x\n", read32(padapter, 0x664));


	printk("\n");

	printk("dbg(0x430)=0x%x\n", read32(padapter, 0x430));
	printk("dbg(0x438)=0x%x\n", read32(padapter, 0x438));

	printk("dbg(0x440)=0x%x\n", read32(padapter, 0x440));
	
	printk("dbg(0x458)=0x%x\n", read32(padapter, 0x458));
	
	printk("dbg(0x484)=0x%x\n", read32(padapter, 0x484));
	printk("dbg(0x488)=0x%x\n", read32(padapter, 0x488));
	
	printk("dbg(0x444)=0x%x\n", read32(padapter, 0x444));
	printk("dbg(0x448)=0x%x\n", read32(padapter, 0x448));
	printk("dbg(0x44c)=0x%x\n", read32(padapter, 0x44c));
	printk("dbg(0x450)=0x%x\n", read32(padapter, 0x450));
#endif
	
	return 0;

}

static int rtw_wx_read32(struct net_device *dev,
                            struct iw_request_info *info,
                            union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = netdev_priv(dev);

	u32 addr;
	u32 data32;


	addr = *(u32*)extra;
	data32 = read32(padapter, addr);
	sprintf(extra, "0x%08x", data32);

	return 0;
}

static int rtw_wx_write32(struct net_device *dev,
                            struct iw_request_info *info,
                            union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = netdev_priv(dev);

	u32 addr;
	u32 data32;


	addr = *(u32*)extra;
	data32 = *((u32*)extra + 1);
	write32(padapter, addr, data32);

	return 0;
}

static int rtw_wx_read_rf(struct net_device *dev,
                            struct iw_request_info *info,
                            union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = netdev_priv(dev);
	u32 path, addr, data32;


	path = *(u32*)extra;
	addr = *((u32*)extra + 1);
	data32 = padapter->HalFunc.read_rfreg(padapter, path, addr, 0xFFFFF);
//	printk("%s: path=%d addr=0x%02x data=0x%05x\n", __func__, path, addr, data32);
	/*
	 * IMPORTANT!!
	 * Only when wireless private ioctl is at odd order,
	 * "extra" would be copied to user space.
	 */
	sprintf(extra, "0x%05x", data32);

	return 0;
}

static int rtw_wx_write_rf(struct net_device *dev,
                            struct iw_request_info *info,
                            union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = netdev_priv(dev);
	u32 path, addr, data32;


	path = *(u32*)extra;
	addr = *((u32*)extra + 1);
	data32 = *((u32*)extra + 2);
//	printk("%s: path=%d addr=0x%02x data=0x%05x\n", __func__, path, addr, data32);
	padapter->HalFunc.write_rfreg(padapter, path, addr, 0xFFFFF, data32);

	return 0;
}

static int rtw_wx_priv_null(struct net_device *dev, struct iw_request_info *a,
		 union iwreq_data *wrqu, char *b)
{
	return -1;
}

static int dummy(struct net_device *dev, struct iw_request_info *a,
		 union iwreq_data *wrqu, char *b)
{
	//_adapter *padapter = (_adapter *)netdev_priv(dev);	
	//struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	//printk("cmd_code=%x, fwstate=0x%x\n", a->cmd, pmlmepriv->fw_state);
	
	return -1;
	
}

static int rtw_wx_set_channel_plan(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = netdev_priv(dev);
	struct registry_priv *pregistrypriv = &padapter->registrypriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	extern int channel_plan;
	channel_plan = (int)wrqu->data.pointer;
	pregistrypriv->channel_plan = channel_plan;
	pmlmepriv->ChannelPlan = pregistrypriv->channel_plan;
	printk("\n======== Set channel_plan = 0x%02X ========\n",channel_plan);

	return 0;
}

static int rtw_wx_set_mtk_wps_probe_ie(struct net_device *dev,
		struct iw_request_info *a,
		union iwreq_data *wrqu, char *b)
{
#ifdef CONFIG_PLATFORM_MT53XX
	_adapter *padapter = (_adapter *)netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_notice_,
		 ("WLAN IOCTL: cmd_code=%x, fwstate=0x%x\n",
		  a->cmd, pmlmepriv->fw_state));
#endif
	return 0;
}

static int rtw_wx_get_sensitivity(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *buf)
{
#ifdef CONFIG_PLATFORM_MT53XX
	_adapter *padapter = netdev_priv(dev);

    //wrqu->qual.level = (u8)padapter->mlmepriv.cur_network.network.Rssi;

	wrqu->qual.level = padapter->recvpriv.fw_rssi;

    printk(" level = %u\n",  wrqu->qual.level );
#endif
	return 0;
}

static int rtw_wx_set_mtk_wps_ie(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
#ifdef CONFIG_PLATFORM_MT53XX
	_adapter *padapter = netdev_priv(dev);

	return rtw_set_wpa_ie(padapter, wrqu->data.pointer, wrqu->data.length);
#else
	return 0;
#endif
}

/*
typedef int (*iw_handler)(struct net_device *dev, struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra);
*/
/*
 *	For all data larger than 16 octets, we need to use a
 *	pointer to memory allocated in user space.
 */
static  int rtw_drvext_hdl(struct net_device *dev, struct iw_request_info *info,
						union iwreq_data *wrqu, char *extra)
{

 #if 0
struct	iw_point
{
  void __user	*pointer;	/* Pointer to the data  (in user space) */
  __u16		length;		/* number of fields or size in bytes */
  __u16		flags;		/* Optional params */
};
 #endif

#ifdef CONFIG_DRVEXT_MODULE
	u8 res;
	struct drvext_handler *phandler;	
	struct drvext_oidparam *poidparam;		
	int ret;
	u16 len;
	u8 *pparmbuf, bset;
	_adapter *padapter = netdev_priv(dev);
	struct iw_point *p = &wrqu->data;

	if( (!p->length) || (!p->pointer)){
		ret = -EINVAL;
		goto _rtw_drvext_hdl_exit;
	}
	
	
	bset = (u8)(p->flags&0xFFFF);
	len = p->length;
	pparmbuf = (u8*)rtw_malloc(len);
	if (pparmbuf == NULL){
		ret = -ENOMEM;
		goto _rtw_drvext_hdl_exit;
	}
	
	if(bset)//set info
	{
		if (copy_from_user(pparmbuf, p->pointer,len)) {
			rtw_mfree(pparmbuf, len);
			ret = -EFAULT;
			goto _rtw_drvext_hdl_exit;
		}		
	}
	else//query info
	{
	
	}

	
	//
	poidparam = (struct drvext_oidparam *)pparmbuf;	
	
	RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_,("drvext set oid subcode [%d], len[%d], InformationBufferLength[%d]\r\n",
        					 poidparam->subcode, poidparam->len, len));


	//check subcode	
	if ( poidparam->subcode >= MAX_DRVEXT_HANDLERS)
	{
		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("no matching drvext handlers\r\n"));		
		ret = -EINVAL;
		goto _rtw_drvext_hdl_exit;
	}


	if ( poidparam->subcode >= MAX_DRVEXT_OID_SUBCODES)
	{
		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("no matching drvext subcodes\r\n"));		
		ret = -EINVAL;
		goto _rtw_drvext_hdl_exit;
	}


	phandler = drvextoidhandlers + poidparam->subcode;

	if (poidparam->len != phandler->parmsize)
	{
		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("no matching drvext param size %d vs %d\r\n",			
						poidparam->len , phandler->parmsize));		
		ret = -EINVAL;		
		goto _rtw_drvext_hdl_exit;
	}


	res = phandler->handler(&padapter->drvextpriv, bset, poidparam->data);

	if(res==0)
	{
		ret = 0;
			
		if (bset == 0x00) {//query info
			//_memcpy(p->pointer, pparmbuf, len);
			if (copy_to_user(p->pointer, pparmbuf, len))
				ret = -EFAULT;
		}		
	}		
	else
		ret = -EFAULT;

	
_rtw_drvext_hdl_exit:	
	
	return ret;	
	
#endif

	return 0;

}

static void rtw_dbg_mode_hdl(_adapter *padapter, u32 id, u8 *pdata, u32 len)
{
	pRW_Reg 	RegRWStruct;
	struct rf_reg_param *prfreg;
	u8 path;
	u8 offset;
	u32 value;

	printk("%s\n", __FUNCTION__);

	switch(id)
	{
		case GEN_MP_IOCTL_SUBCODE(MP_START):
			printk("871x_driver is only for normal mode, can't enter mp mode\n");
			break;
		case GEN_MP_IOCTL_SUBCODE(READ_REG):
			RegRWStruct = (pRW_Reg)pdata;
			switch (RegRWStruct->width)
			{
				case 1:
					RegRWStruct->value = read8(padapter, RegRWStruct->offset);
					break;
				case 2:
					RegRWStruct->value = read16(padapter, RegRWStruct->offset);
					break;
				case 4:
					RegRWStruct->value = read32(padapter, RegRWStruct->offset);
					break;
				default:
					break;
			}
		
			break;
		case GEN_MP_IOCTL_SUBCODE(WRITE_REG):
			RegRWStruct = (pRW_Reg)pdata;
			switch (RegRWStruct->width)
			{
				case 1:
					write8(padapter, RegRWStruct->offset, (u8)RegRWStruct->value);
					break;
				case 2:
					write16(padapter, RegRWStruct->offset, (u16)RegRWStruct->value);
					break;
				case 4:
					write32(padapter, RegRWStruct->offset, (u32)RegRWStruct->value);
					break;
				default:					
				break;
			}
				
			break;
		case GEN_MP_IOCTL_SUBCODE(READ_RF_REG):

			prfreg = (struct rf_reg_param *)pdata;

			path = (u8)prfreg->path;		
			offset = (u8)prfreg->offset;	

			value = padapter->HalFunc.read_rfreg(padapter, path, offset, 0xffffffff);

			prfreg->value = value;

			break;			
		case GEN_MP_IOCTL_SUBCODE(WRITE_RF_REG):

			prfreg = (struct rf_reg_param *)pdata;

			path = (u8)prfreg->path;
			offset = (u8)prfreg->offset;	
			value = prfreg->value;

			padapter->HalFunc.write_rfreg(padapter, path, offset, 0xffffffff, value);
			
			break;			
                case GEN_MP_IOCTL_SUBCODE(TRIGGER_GPIO):
			printk("==> trigger gpio 0\n");
			padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_TRIGGER_GPIO_0, 0);
			break;	
#ifdef CONFIG_BT_COEXIST
		case GEN_MP_IOCTL_SUBCODE(SET_DM_BT):			
			printk("==> set dm_bt_coexist:%x\n",*(u8 *)pdata);
			padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_BT_SET_COEXIST, pdata);
			break;
		case GEN_MP_IOCTL_SUBCODE(DEL_BA):
			printk("==> delete ba:%x\n",*(u8 *)pdata);
			padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_BT_ISSUE_DELBA, pdata);
			break;
#endif
#ifdef SILENT_RESET_FOR_SPECIFIC_PLATFOM
		case GEN_MP_IOCTL_SUBCODE(GET_WIFI_STATUS):	
			if(padapter->HalFunc.sreset_get_wifi_status)				
				*pdata = padapter->HalFunc.sreset_get_wifi_status(padapter);                   
			break;
#endif
	
		default:
			break;
	}
	
}

static int rtw_mp_ioctl_hdl(struct net_device *dev, struct iw_request_info *info,
						union iwreq_data *wrqu, char *extra)
{
	int ret = 0;
	unsigned long BytesRead, BytesWritten, BytesNeeded;
	struct oid_par_priv	oid_par;
	struct mp_ioctl_handler	*phandler;
	struct mp_ioctl_param	*poidparam;
	uint status=0;
	u16 len;
	u8 *pparmbuf = NULL, bset;
	_adapter *padapter = netdev_priv(dev);
	struct iw_point *p = &wrqu->data;

	//printk("+rtw_mp_ioctl_hdl\n");

	//mutex_lock(&ioctl_mutex);

	if ((!p->length) || (!p->pointer)) {
		ret = -EINVAL;
		goto _rtw_mp_ioctl_hdl_exit;
	}

	pparmbuf = NULL;
	bset = (u8)(p->flags & 0xFFFF);
	len = p->length;
	pparmbuf = (u8*)rtw_malloc(len);
	if (pparmbuf == NULL){
		ret = -ENOMEM;
		goto _rtw_mp_ioctl_hdl_exit;
	}

	if (copy_from_user(pparmbuf, p->pointer, len)) {
		ret = -EFAULT;
		goto _rtw_mp_ioctl_hdl_exit;
	}

	poidparam = (struct mp_ioctl_param *)pparmbuf;
	RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_info_,
		 ("rtw_mp_ioctl_hdl: subcode [%d], len[%d], buffer_len[%d]\r\n",
		  poidparam->subcode, poidparam->len, len));

	if (poidparam->subcode >= MAX_MP_IOCTL_SUBCODE) {
		RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_err_, ("no matching drvext subcodes\r\n"));
		ret = -EINVAL;
		goto _rtw_mp_ioctl_hdl_exit;
	}

	//printk("%s: %d\n", __func__, poidparam->subcode);

#ifdef CONFIG_MP_INCLUDED 
	phandler = mp_ioctl_hdl + poidparam->subcode;

	if ((phandler->paramsize != 0) && (poidparam->len < phandler->paramsize))
	{
		RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_err_,
			 ("no matching drvext param size %d vs %d\r\n",
			  poidparam->len, phandler->paramsize));
		ret = -EINVAL;
		goto _rtw_mp_ioctl_hdl_exit;
	}

	if (phandler->handler)
	{
		oid_par.adapter_context = padapter;
		oid_par.oid = phandler->oid;
		oid_par.information_buf = poidparam->data;
		oid_par.information_buf_len = poidparam->len;
		oid_par.dbg = 0;

		BytesWritten = 0;
		BytesNeeded = 0;

		if (bset) {
			oid_par.bytes_rw = &BytesRead;
			oid_par.bytes_needed = &BytesNeeded;
			oid_par.type_of_oid = SET_OID;
		} else {
			oid_par.bytes_rw = &BytesWritten;
			oid_par.bytes_needed = &BytesNeeded;
			oid_par.type_of_oid = QUERY_OID;
		}

		status = phandler->handler(&oid_par);

		//todo:check status, BytesNeeded, etc.
	}
	else {
		printk("rtw_mp_ioctl_hdl(): err!, subcode=%d, oid=%d, handler=%p\n", 
			poidparam->subcode, phandler->oid, phandler->handler);
		ret = -EFAULT;
		goto _rtw_mp_ioctl_hdl_exit;
	}
#else

	rtw_dbg_mode_hdl(padapter, poidparam->subcode, poidparam->data, poidparam->len);
	
#endif

	if (bset == 0x00) {//query info
		if (copy_to_user(p->pointer, pparmbuf, len))
			ret = -EFAULT;
	}

	if (status) {
		ret = -EFAULT;
		goto _rtw_mp_ioctl_hdl_exit;
	}

_rtw_mp_ioctl_hdl_exit:

	if (pparmbuf)
		rtw_mfree(pparmbuf, len);

	//mutex_unlock(&ioctl_mutex);

	return ret;
}

static int rtw_get_ap_info(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	int bssid_match, ret = 0;
	u32 cnt=0, wpa_ielen;
	_irqL	irqL;
	_list	*plist, *phead;
	unsigned char *pbuf;
	u8 bssid[ETH_ALEN];
	char data[32];
	struct wlan_network *pnetwork = NULL;
	_adapter *padapter = netdev_priv(dev);	
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);	
	_queue *queue = &(pmlmepriv->scanned_queue);
	struct iw_point *pdata = &wrqu->data;	

	printk("+rtw_get_aplist_info\n");

	if((padapter->bDriverStopped) || (pdata==NULL))
	{                
		ret= -EINVAL;
		goto exit;
	}		
  
 	while((check_fwstate(pmlmepriv, (_FW_UNDER_SURVEY|_FW_UNDER_LINKING))) == _TRUE)
	{	
		msleep_os(30);
		cnt++;
		if(cnt > 100)
			break;
	}
	

	//pdata->length = 0;//?	
	pdata->flags = 0;
	if(pdata->length>=32)
	{
		if(copy_from_user(data, pdata->pointer, 32))
		{
			ret= -EINVAL;
			goto exit;
		}
	}	
	else
	{
		ret= -EINVAL;
		goto exit;
	}	

	_enter_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);
	
	phead = get_list_head(queue);
	plist = get_next(phead);
       
	while(1)
	{
		if (end_of_queue_search(phead,plist)== _TRUE)
			break;


		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);

		//if(hwaddr_aton_i(pdata->pointer, bssid)) 
		if(hwaddr_aton_i(data, bssid)) 
		{			
			printk("Invalid BSSID '%s'.\n", (u8*)data);
			return -EINVAL;
		}		
		
	
		if(_memcmp(bssid, pnetwork->network.MacAddress, ETH_ALEN) == _TRUE)//BSSID match, then check if supporting wpa/wpa2
		{
			printk("BSSID:" MACSTR "\n", MAC2STR(bssid));
			
			pbuf = get_wpa_ie(&pnetwork->network.IEs[12], &wpa_ielen, pnetwork->network.IELength-12);				
			if(pbuf && (wpa_ielen>0))
			{
				pdata->flags = 1;
				break;
			}

			pbuf = get_wpa2_ie(&pnetwork->network.IEs[12], &wpa_ielen, pnetwork->network.IELength-12);
			if(pbuf && (wpa_ielen>0))
			{
				pdata->flags = 2;
				break;
			}
			
		}

		plist = get_next(plist);		
	
	}        

	_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	if(pdata->length>=34)
	{
		if(copy_to_user((u8*)pdata->pointer+32, (u8*)&pdata->flags, 1))
		{
			ret= -EINVAL;
			goto exit;
		}
	}	
	
exit:
	
	return ret;
		
}

static int rtw_set_pid(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	
	int ret = 0;	
	_adapter *padapter = netdev_priv(dev);	
	struct iw_point *pdata = &wrqu->data;	

	printk("+rtw_set_pid\n");

	if((padapter->bDriverStopped) || (pdata==NULL))
	{                
		ret= -EINVAL;
		goto exit;
	}		
  
 	//pdata->length = 0;
	//pdata->flags = 0;

	//_memcpy(&padapter->pid, pdata->pointer, sizeof(int));
	if(copy_from_user(&padapter->pid, pdata->pointer, sizeof(int)))
	{
		ret= -EINVAL;
		goto exit;
	}

	printk("got pid=%d\n", padapter->pid);

exit:
	
	return ret;
		
}

static int rtw_wps_start(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	
	int ret = 0;	
	_adapter *padapter = netdev_priv(dev);	
	struct iw_point *pdata = &wrqu->data;
	u32   u32wps_start = 0;
        unsigned int uintRet = 0;

        uintRet = copy_from_user( ( void* ) &u32wps_start, pdata->pointer, 4 );

	if((padapter->bDriverStopped) || (pdata==NULL))
	{                
		ret= -EINVAL;
		goto exit;
	}		

       if ( u32wps_start == 0 )
       {
           u32wps_start = *extra;
       }

       printk( "[%s] wps_start = %d\n", __FUNCTION__, u32wps_start );

       if ( u32wps_start == 1 ) // WPS Start
       {
           padapter->ledpriv.LedControlHandler(padapter, LED_CTL_START_WPS);
       }
	else if ( u32wps_start == 2 ) // WPS Stop because of wps success
	{
           padapter->ledpriv.LedControlHandler(padapter, LED_CTL_STOP_WPS);
	}
	else if ( u32wps_start == 3 ) // WPS Stop because of wps fail
	{
           padapter->ledpriv.LedControlHandler(padapter, LED_CTL_STOP_WPS_FAIL);
	}
exit:
	
	return ret;
		
}

#if ( P2P_INCLUDED == 1 )

void enter_listen_state_workitem_process(struct work_struct *work)
{
	struct wifidirect_info  *pwdinfo = container_of(work, struct wifidirect_info, enter_listen_state_workitem);
	_adapter *padapter = pwdinfo->padapter;
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;

_func_enter_;

	pwdinfo->p2p_state = P2P_STATE_LISTEN;
	set_channel_bwmode( padapter, pwdinfo->listen_channel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, HT_CHANNEL_WIDTH_20);

_func_exit_;
}

void enter_listen_state_timer_process (void *FunctionContext)
{
	_adapter *adapter = (_adapter *)FunctionContext;
	struct	wifidirect_info		*pwdinfo = &adapter->wdinfo;

	_set_workitem( &pwdinfo->enter_listen_state_workitem );
}

void find_phase_workitem_process(struct work_struct *work)
{
	struct wifidirect_info  *pwdinfo = container_of(work, struct wifidirect_info, find_phase_workitem);
	_adapter *padapter = pwdinfo->padapter;
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	NDIS_802_11_SSID 	ssid;
	_irqL				irqL;
	u8					_status = 0;

_func_enter_;

	_memset((unsigned char*)&ssid, 0, sizeof(NDIS_802_11_SSID));
	_memcpy(ssid.Ssid, pwdinfo->p2p_wildcard_ssid, P2P_WILDCARD_SSID_LEN );
	ssid.SsidLength = P2P_WILDCARD_SSID_LEN;

	pwdinfo->p2p_state = P2P_STATE_FIND_PHASE_SEARCH;
		
	_enter_critical_bh(&pmlmepriv->lock, &irqL);
	_status = sitesurvey_cmd(padapter, &ssid);
	_exit_critical_bh(&pmlmepriv->lock, &irqL);


_func_exit_;
}

void find_phase_timer_process (void *FunctionContext)
{
	_adapter *adapter = (_adapter *)FunctionContext;

	adapter->wdinfo.find_phase_state_exchange_cnt++;

	_set_workitem( &adapter->wdinfo.find_phase_workitem );
}

static void init_wifidirect_info( _adapter*	padapter, u8* pinitValue)
{
	struct wifidirect_info	*pwdinfo;

	pwdinfo = &padapter->wdinfo;

	pwdinfo->padapter = padapter;

	//init device&interface address
	_memcpy(pwdinfo->device_addr, myid(&(padapter->eeprompriv)), ETH_ALEN);
	_memcpy(pwdinfo->interface_addr, myid(&(padapter->eeprompriv)), ETH_ALEN);
	
	//	1, 6, 11 are the social channel defined in the WiFi Direct specification.
	pwdinfo->social_chan[0] = 1;
	pwdinfo->social_chan[1] = 6;
	pwdinfo->social_chan[2] = 11;
	
	//	Use the channel 11 as the listen channel
	pwdinfo->listen_channel = 11;

	if ( *pinitValue == 1 )
	{
		pwdinfo->role = P2P_ROLE_DEVICE;
		pwdinfo->p2p_state = P2P_STATE_LISTEN;
		pwdinfo->intent = 1;
	}
	else if ( *pinitValue == 2 )
	{
		pwdinfo->role = P2P_ROLE_GO;
		pwdinfo->p2p_state = P2P_STATE_GONEGO_OK;
		pwdinfo->intent = 15;
	}
	
//	Use the OFDM rate in the P2P probe response frame. ( 6(B), 9(B), 12(B), 24(B), 36, 48, 54 )	
	pwdinfo->support_rate[0] = 0x8c;
	pwdinfo->support_rate[1] = 0x92;
	pwdinfo->support_rate[2] = 0x98;
	pwdinfo->support_rate[3] = 0xB0;
	pwdinfo->support_rate[4] = 0x48;
	pwdinfo->support_rate[5] = 0x60;
	pwdinfo->support_rate[6] = 0x6c;


	_memcpy( ( void* ) pwdinfo->p2p_wildcard_ssid, "DIRECT-", 7 );

	_memset( pwdinfo->device_name, 0x00, WPS_MAX_DEVICE_NAME_LEN );
	_memcpy( pwdinfo->device_name, "Realtek DMP Device", 18 );
	pwdinfo->device_name_len = 18;

	_memset( &pwdinfo->invitereq_info, 0x00, sizeof( struct tx_invite_req_info ) );
	pwdinfo->invitereq_info.token = 3;	//	Token used for P2P invitation request frame.
	pwdinfo->invitereq_info.peer_operation_ch = pwdinfo->listen_channel;
	
	_memset( &pwdinfo->inviteresp_info, 0x00, sizeof( struct tx_invite_resp_info ) );
	pwdinfo->inviteresp_info.token = 0;

	pwdinfo->profileindex = 0;
	_memset( &pwdinfo->profileinfo[ 0 ], 0x00, sizeof( struct profile_info ) * P2P_MAX_PERSISTENT_GROUP_NUM );

	_init_workitem( &pwdinfo->find_phase_workitem, find_phase_workitem_process, padapter );
	_init_timer( &pwdinfo->find_phase_timer, padapter->pnetdev, find_phase_timer_process, padapter );

	pwdinfo->find_phase_state_exchange_cnt = 0;

	pwdinfo->listen_dwell = ( u8 ) (( get_current_time() % 3 ) + 1);
	printk( "[%s] listen_dwell time is %d00ms\n", __FUNCTION__, pwdinfo->listen_dwell );

	pwdinfo->wps_config_method_request = WPS_CM_NONE;

	_init_workitem( &pwdinfo->enter_listen_state_workitem, enter_listen_state_workitem_process, padapter );
	_init_timer( &pwdinfo->enter_listen_state_timer, padapter->pnetdev, enter_listen_state_timer_process, padapter );

	pwdinfo->device_password_id_for_nego = WPS_DPID_PBC;
	pwdinfo->negotiation_dialog_token = 1;

	_memset( pwdinfo->nego_ssid, 0x00, WLAN_SSID_MAXLEN );
	pwdinfo->nego_ssidlen = 0;
}

#endif

static int rtw_p2p_enable(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	
	int ret = 0;	
	
#if ( P2P_INCLUDED == 1 )

	_adapter *padapter = netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);	
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;

	if ( ( *extra == 1 ) || ( *extra == 2 ) )
	{
		u8 channel, ch_offset;
		u16 bwmode;

		//	Enable P2P function
		init_wifidirect_info(padapter,  extra );
		
		
		if(pwdinfo->p2p_state == P2P_STATE_LISTEN)
		{
			//	Stay at the listen state and wait for discovery.
			channel = pwdinfo->listen_channel;
			ch_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
			bwmode = HT_CHANNEL_WIDTH_20;			
		}
		else
		{
			pwdinfo->operating_channel = pmlmeext->cur_channel;
		
			channel = pwdinfo->operating_channel;
			ch_offset = pmlmeext->cur_ch_offset;
			bwmode = pmlmeext->cur_bwmode;						
		}

		set_channel_bwmode(padapter, channel, ch_offset, bwmode);
	
	}
	else if ( *extra == 0 )
	{	//	Disable P2P Listen State
		if ( pwdinfo->p2p_state != P2P_STATE_NONE )
		{
			_memset( pwdinfo, 0x00, sizeof( struct wifidirect_info) );
			pwdinfo->p2p_state = P2P_STATE_NONE;
		}
	}
	
#endif

exit:
	
	return ret;
		
}

static int rtw_p2p_set_go_nego_ssid(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	
	int ret = 0;	
#if ( P2P_INCLUDED == 1 )	
	_adapter *padapter = netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);	
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);

	printk( "[%s] ssid = %s, len = %d\n", __FUNCTION__, extra, strlen( extra ) );
	_memcpy( pwdinfo->nego_ssid, extra, strlen( extra ) );
	pwdinfo->nego_ssidlen = strlen( extra );
#endif
exit:
	
	return ret;
		
}


static int rtw_p2p_set_intent(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	
	int ret = 0;	
#if ( P2P_INCLUDED == 1 )	
	_adapter *padapter = netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);	
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);

	pwdinfo->intent= *((u8*)extra);
	printk( "[%s] intent = %d\n", __FUNCTION__, pwdinfo->intent);
#endif
exit:
	
	return ret;
		
}

static int rtw_p2p_profilefound(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	
	int ret = 0;	
#if ( P2P_INCLUDED == 1 )	
	_adapter *padapter = netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);	
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);

	//	Comment by Albert 2010/10/13
	//	Input data format:
	//	Ex:  0
	//	Ex:  1XXXXXXXXXXXXYYSSID
	//	0 => Reflush the profile record list.
	//	1 => Add the profile list
	//	XXXXXXXXXXXX => peer's MAC Address ( 00:E0:4C:00:00:01 => 00E04C000001 )
	//	YY => SSID Length
	//	SSID => SSID for persistence group

	printk( "[%s] In value = %s, len = %d \n", __FUNCTION__, extra, wrqu->data.length -1);

	
	//	The upper application should pass the SSID to driver by using this rtw_p2p_profilefound function.
	if ( pwdinfo->p2p_state != P2P_STATE_NONE )
	{
		if ( extra[ 0 ] == '0' )
		{
			//	Remove all the profile information of wifidirect_info structure.
			_memset( &pwdinfo->profileinfo[ 0 ], 0x00, sizeof( struct profile_info ) * P2P_MAX_PERSISTENT_GROUP_NUM );
			pwdinfo->profileindex = 0;
		}
		else
		{
			if ( pwdinfo->profileindex >= P2P_MAX_PERSISTENT_GROUP_NUM )
		{
				ret = -1;
		}
		else
		{
				int jj, kk;
				
				//	Add this profile information into pwdinfo->profileinfo
				//	Ex:  1XXXXXXXXXXXXYYSSID
				for( jj = 0, kk = 1; jj < ETH_ALEN; jj++, kk += 2 )
				{
					pwdinfo->profileinfo[ pwdinfo->profileindex ].peermac[ jj ] = key_2char2num(extra[ kk ], extra[ kk+ 1 ]);
				}

				pwdinfo->profileinfo[ pwdinfo->profileindex ].ssidlen = ( extra[13] - '0' ) * 10 + ( extra[ 14 ] - '0' );
				_memcpy( pwdinfo->profileinfo[ pwdinfo->profileindex ].ssid, &extra[ 15 ], pwdinfo->profileinfo[ pwdinfo->profileindex ].ssidlen );
				pwdinfo->profileindex++;
			}
		}
	}	
#endif
exit:
	
	return ret;
		
}

static int rtw_p2p_setDN(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	
	int ret = 0;	
#if ( P2P_INCLUDED == 1 )
	_adapter *padapter = netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);	
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);


	printk( "[%s] %s %d\n", __FUNCTION__, extra, wrqu->data.length -1  );
	pwdinfo->device_name_len = wrqu->data.length - 1;
	_memset( pwdinfo->device_name, 0x00, WPS_MAX_DEVICE_NAME_LEN );
	_memcpy( pwdinfo->device_name, extra, pwdinfo->device_name_len );
#endif
exit:
	
	return ret;
		
}


static int rtw_p2p_get_status(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	
	int ret = 0;	
#if ( P2P_INCLUDED == 1 )
	_adapter *padapter = netdev_priv(dev);	
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	
	printk( "[%s] Role = %d, Status = %d, peer addr = %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", __FUNCTION__, pwdinfo->role, pwdinfo->p2p_state,
			pwdinfo->p2p_peer_interface_addr[ 0 ], pwdinfo->p2p_peer_interface_addr[ 1 ], pwdinfo->p2p_peer_interface_addr[ 2 ],
			pwdinfo->p2p_peer_interface_addr[ 3 ], pwdinfo->p2p_peer_interface_addr[ 4 ], pwdinfo->p2p_peer_interface_addr[ 5 ]);

	//	Commented by Albert 2010/10/12
	//	Because of the output size limitation, I had removed the "Role" information.
	//	About the "Role" information, we will use the new private IOCTL to get the "Role" information.
	sprintf( extra, "\n\nStatus=%.2d\n", pwdinfo->p2p_state );

	if ( pwdinfo->p2p_state == P2P_STATE_LISTEN )
	{
		//	Stay at the listen state and wait for discovery.			
		set_channel_bwmode(padapter, pwdinfo->listen_channel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, HT_CHANNEL_WIDTH_20);
	}
#endif
exit:
	
	return ret;
		
}

static int rtw_p2p_get_peer_ifaddr(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	
	int ret = 0;	
#if ( P2P_INCLUDED == 1 )
	_adapter *padapter = netdev_priv(dev);	
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );


	printk( "[%s] Role = %d, Status = %d, peer addr = %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", __FUNCTION__, pwdinfo->role, pwdinfo->p2p_state,
			pwdinfo->p2p_peer_interface_addr[ 0 ], pwdinfo->p2p_peer_interface_addr[ 1 ], pwdinfo->p2p_peer_interface_addr[ 2 ],
			pwdinfo->p2p_peer_interface_addr[ 3 ], pwdinfo->p2p_peer_interface_addr[ 4 ], pwdinfo->p2p_peer_interface_addr[ 5 ]);
	sprintf( extra, "\n%.2X%.2X%.2X%.2X%.2X%.2X",
			pwdinfo->p2p_peer_interface_addr[ 0 ], pwdinfo->p2p_peer_interface_addr[ 1 ], pwdinfo->p2p_peer_interface_addr[ 2 ],
			pwdinfo->p2p_peer_interface_addr[ 3 ], pwdinfo->p2p_peer_interface_addr[ 4 ], pwdinfo->p2p_peer_interface_addr[ 5 ]);
#endif
exit:
	
	return ret;
		
}

static int rtw_p2p_get_wps_configmethod(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	
	int ret = 0;	
#if ( P2P_INCLUDED == 1 )
	_adapter 				*padapter = netdev_priv(dev);	
	struct iw_point 		*pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );
	u8					peerMAC[ ETH_ALEN ] = { 0x00 };
	int 					jj,kk;
	u8   					peerMACStr[ ETH_ALEN * 2 ] = { 0x00 };
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	_irqL				irqL;
	_list					*plist, *phead;
	_queue				*queue	= &(pmlmepriv->scanned_queue);
	struct	wlan_network	*pnetwork = NULL;
	u8					blnMatch = 0;


	//	Commented by Albert 20110226
	//	The input data is the MAC address which the application wants to know its WPS config method.
	//	After knowing its WPS config method, the application can decide the config method for provisioning discovery.
	//	Format: iwpriv wlanx p2p_get_wpsCM 00E04C000005

	printk( "[%s] data = %s\n", __FUNCTION__, ( char* ) wrqu->data.pointer );
	_memcpy( peerMACStr , wrqu->data.pointer , wrqu->data.length );


	for( jj = 0, kk = 0; jj < ETH_ALEN; jj++, kk += 2 )
	{
		peerMAC[ jj ] = key_2char2num( peerMACStr[kk], peerMACStr[kk+ 1] );
	}

	_enter_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	phead = get_list_head(queue);
	plist = get_next(phead);
       
	while(1)
	{
		if (end_of_queue_search(phead,plist)== _TRUE)
			break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);
		if ( _memcmp( pnetwork->network.MacAddress, peerMAC, ETH_ALEN ) )
		{
			u8	wpsie[ 100 ] = { 0x00 };
			uint	wpsie_len = 0;
			u16	attr_content = 0;
			uint	attr_contentlen = 0;
			
              	//	The mac address is matched.

			if ( get_wps_ie_p2p( &pnetwork->network.IEs[ 12 ], pnetwork->network.IELength - 12, wpsie,  &wpsie_len ) )
			{
				get_wps_attr_content( wpsie, wpsie_len, WPS_ATTR_CONF_METHOD, ( u8* ) &attr_content, &attr_contentlen);
				if ( attr_contentlen )
				{
					attr_content = be16_to_cpu( attr_content );
					sprintf( extra, "%.4d", attr_content );
					_memcpy( wrqu->data.pointer, extra, 4 );
					*( (u8*) wrqu->data.pointer + 4 ) = 0x00;
					blnMatch = 1;
				}
			}
			
			break;
              }

		plist = get_next(plist);
	
	}        

	_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	if ( !blnMatch )
	{
		sprintf( extra, "0000" );	
		_memcpy( wrqu->data.pointer, extra, 4 );
	}
#endif
exit:
	
	return ret;
		
}

static int rtw_p2p_connect(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	
	int ret = 0;	
#if ( P2P_INCLUDED == 1 )	
	_adapter 				*padapter = netdev_priv(dev);	
	struct iw_point 		*pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );
	u8					peerMAC[ ETH_ALEN ] = { 0x00 };
	int 					jj,kk;
	u8   					peerMACStr[ ETH_ALEN * 2 ] = { 0x00 };
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	_irqL				irqL;
	_list					*plist, *phead;
	_queue				*queue	= &(pmlmepriv->scanned_queue);
	struct	wlan_network	*pnetwork = NULL;
	uint					uintPeerChannel = 0;
	


	//	Commented by Albert 20110304
	//	The input data contains two informations.
	//	1. First information is the MAC address which wants to formate with
	//	2. Second information is the WPS PINCode or "pbc" string for push button method
	//	Format: iwpriv wlanx p2p_connect 00E04C000005_pbc
	//	Format: iwpriv wlanx p2p_connect 00E04C000005_pin

	printk( "[%s] data = %s\n", __FUNCTION__, extra );


	for( jj = 0, kk = 0; jj < ETH_ALEN; jj++, kk += 2 )
	{
		peerMAC[ jj ] = key_2char2num( extra[kk], extra[kk+ 1] );
	}

	if ( _memcmp( &extra[ 13 ], "pin", 3 ) )
	{
		pwdinfo->device_password_id_for_nego = WPS_DPID_PIN;
	}
	else if ( _memcmp( &extra[ 13 ], "pbc", 3 ) )
	{
		pwdinfo->wps_config_method_request = WPS_DPID_PBC;
	}
	else
	{
		return -1;
	}

	_enter_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	phead = get_list_head(queue);
	plist = get_next(phead);
       
	while(1)
	{
		if (end_of_queue_search(phead,plist)== _TRUE)
			break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);
		if ( _memcmp( pnetwork->network.MacAddress, peerMAC, ETH_ALEN ) )
		{
			uintPeerChannel = pnetwork->network.Configuration.DSConfig;
			break;
              }

		plist = get_next(plist);
	
	}        

	_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	set_channel_bwmode(padapter, uintPeerChannel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, HT_CHANNEL_WIDTH_20);
	issue_p2p_GO_request( padapter, pnetwork->network.MacAddress);

	_set_timer( &pwdinfo->enter_listen_state_timer, P2P_GO_NEGO_TIMEOUT );
#endif
exit:
	
	return ret;
		
}

static int rtw_p2p_prov_disc(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	
	int ret = 0;	
#if ( P2P_INCLUDED == 1 )	
	_adapter 				*padapter = netdev_priv(dev);	
	struct iw_point 		*pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );
	u8					peerMAC[ ETH_ALEN ] = { 0x00 };
	int 					jj,kk;
	u8   					peerMACStr[ ETH_ALEN * 2 ] = { 0x00 };
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	_irqL				irqL;
	_list					*plist, *phead;
	_queue				*queue	= &(pmlmepriv->scanned_queue);
	struct	wlan_network	*pnetwork = NULL;
	uint					uintPeerChannel = 0;
	


	//	Commented by Albert 20110301
	//	The input data contains two informations.
	//	1. First information is the MAC address which wants to issue the provisioning discovery request frame.
	//	2. Second information is the WPS configuration method which wants to discovery
	//	Format: iwpriv wlanx p2p_prov_disc 00E04C000005_display
	//	Format: iwpriv wlanx p2p_prov_disc 00E04C000005_keypad
	//	Format: iwpriv wlanx p2p_prov_disc 00E04C000005_pbc

	printk( "[%s] data = %s\n", __FUNCTION__, extra );


	for( jj = 0, kk = 0; jj < ETH_ALEN; jj++, kk += 2 )
	{
		peerMAC[ jj ] = key_2char2num( extra[kk], extra[kk+ 1] );
	}

	if ( _memcmp( &extra[ 13 ], "display", 7 ) )
	{
		pwdinfo->wps_config_method_request = WPS_CM_DISPLYA;
	}
	else if ( _memcmp( &extra[ 13 ], "keypad", 7 ) )
	{
		pwdinfo->wps_config_method_request = WPS_CM_KEYPAD;
	}
	else if ( _memcmp( &extra[ 13 ], "pbc", 3 ) )
	{
		pwdinfo->wps_config_method_request = WPS_CM_PUSH_BUTTON;
	}

	_enter_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	phead = get_list_head(queue);
	plist = get_next(phead);
       
	while(1)
	{
		if (end_of_queue_search(phead,plist)== _TRUE)
			break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);
		if ( _memcmp( pnetwork->network.MacAddress, peerMAC, ETH_ALEN ) )
		{
			uintPeerChannel = pnetwork->network.Configuration.DSConfig;
			break;
              }

		plist = get_next(plist);
	
	}        

	_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	set_channel_bwmode(padapter, uintPeerChannel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, HT_CHANNEL_WIDTH_20);
	issue_p2p_provision_request( padapter, pnetwork->network.MacAddress);

	_set_timer( &pwdinfo->enter_listen_state_timer, P2P_PROVISION_TIMEOUT );
#endif
exit:
	
	return ret;
		
}

#if 0
void mac_reg_dump(_adapter *padapter)
{
	int i,j=1;		
	printk("\n======= MAC REG =======\n");
	for(i=0x0;i<0x300;i+=4)
	{	
		if(j%4==1)	printk("0x%02x",i);
		printk(" 0x%08x ",read32(padapter,i));		
		if((j++)%4 == 0)	printk("\n");	
	}
	for(i=0x400;i<0x800;i+=4)
	{	
		if(j%4==1)	printk("0x%02x",i);
		printk(" 0x%08x ",read32(padapter,i));		
		if((j++)%4 == 0)	printk("\n");	
	}									
}
void bb_reg_dump(_adapter *padapter)
{
	int i,j=1;		
	printk("\n======= BB REG =======\n");
	for(i=0x800;i<0x1000;i+=4)
	{
		if(j%4==1) printk("0x%02x",i);
				
		printk(" 0x%08x ",read32(padapter,i));		
		if((j++)%4 == 0)	printk("\n");	
	}		
}
void rf_reg_dump(_adapter *padapter)
{	
	int i,j=1,path;
	u32 value;			
	printk("\n======= RF REG =======\n");
	for(path=0;path<2;path++)
	{
		printk("\nRF_Path(%x)\n",path);
		for(i=0;i<0x100;i++)
		{								
			value = PHY_QueryRFReg(padapter, (RF90_RADIO_PATH_E)path,i, bMaskDWord);
			if(j%4==1)	printk("0x%02x ",i);
			printk(" 0x%08x ",value);
			if((j++)%4==0)	printk("\n");	
		}	
	}
}

#endif

void mac_reg_dump(_adapter *padapter)
{
	int i,j=1;		
	printk("\n======= MAC REG =======\n");
	for(i=0x0;i<0x300;i+=4)
	{	
		if(j%4==1)	printk("0x%02x",i);
		printk(" 0x%08x ",read32(padapter,i));		
		if((j++)%4 == 0)	printk("\n");	
	}
	for(i=0x400;i<0x800;i+=4)
	{	
		if(j%4==1)	printk("0x%02x",i);
		printk(" 0x%08x ",read32(padapter,i));		
		if((j++)%4 == 0)	printk("\n");	
	}									
}
void bb_reg_dump(_adapter *padapter)
{
	int i,j=1;		
	printk("\n======= BB REG =======\n");
	for(i=0x800;i<0x1000;i+=4)
	{
		if(j%4==1) printk("0x%02x",i);
				
		printk(" 0x%08x ",read32(padapter,i));		
		if((j++)%4 == 0)	printk("\n");	
	}		
}
void rf_reg_dump(_adapter *padapter)
{	
	int i,j=1,path;
	u32 value;	
	u8 rf_type,path_nums = 0;
	padapter->HalFunc.GetHwRegHandler(padapter, HW_VAR_RF_TYPE, (u8 *)(&rf_type));
		
	printk("\n======= RF REG =======\n");
	if((RF_1T2R == rf_type) ||(RF_1T1R ==rf_type ))	
		path_nums = 1;
	else	
		path_nums = 2;
		
	for(path=0;path<path_nums;path++)
	{
		printk("\nRF_Path(%x)\n",path);
		for(i=0;i<0x100;i++)
		{								
			//value = PHY_QueryRFReg(padapter, (RF90_RADIO_PATH_E)path,i, bMaskDWord);
			value =padapter->HalFunc.read_rfreg(padapter, path, i, 0xffffffff);
			if(j%4==1)	printk("0x%02x ",i);
			printk(" 0x%08x ",value);
			if((j++)%4==0)	printk("\n");	
		}	
	}
}

static int rtw_dbg_port(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{	
	_irqL irqL;
	int ret = 0;
	u8 major_cmd, minor_cmd;
	u16 arg;
	u32 extra_arg, *pdata, val32;
	struct sta_info *psta;						
	_adapter *padapter = netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	struct wlan_network *cur_network = &(pmlmepriv->cur_network);
	struct sta_priv *pstapriv = &padapter->stapriv;
	

	pdata = (u32*)&wrqu->data;	

	val32 = *pdata;
	arg = (u16)(val32&0x0000ffff);
	major_cmd = (u8)(val32>>24);
	minor_cmd = (u8)((val32>>16)&0x00ff);

	extra_arg = *(pdata+1);
	
	switch(major_cmd)
	{
		case 0x70://read_reg
			switch(minor_cmd)
			{
				case 1:
					printk("read8(0x%x)=0x%x\n", arg, read8(padapter, arg));
					break;
				case 2:
					printk("read16(0x%x)=0x%x\n", arg, read16(padapter, arg));
					break;
				case 4:
					printk("read32(0x%x)=0x%x\n", arg, read32(padapter, arg));
					break;
			}			
			break;
		case 0x71://write_reg
			switch(minor_cmd)
			{
				case 1:
					write8(padapter, arg, extra_arg);
					printk("write8(0x%x)=0x%x\n", arg, read8(padapter, arg));
					break;
				case 2:
					write16(padapter, arg, extra_arg);
					printk("write16(0x%x)=0x%x\n", arg, read16(padapter, arg));
					break;
				case 4:
					write32(padapter, arg, extra_arg);
					printk("write32(0x%x)=0x%x\n", arg, read32(padapter, arg));
					break;
			}			
			break;
		case 0x72://read_bb
			printk("read_bbreg(0x%x)=0x%x\n", arg, padapter->HalFunc.read_bbreg(padapter, arg, 0xffffffff));
			break;
		case 0x73://write_bb
			padapter->HalFunc.write_bbreg(padapter, arg, 0xffffffff, extra_arg);
			printk("write_bbreg(0x%x)=0x%x\n", arg, padapter->HalFunc.read_bbreg(padapter, arg, 0xffffffff));
			break;
		case 0x74://read_rf
			printk("read RF_reg path(0x%02x),offset(0x%x),value(0x%08x)\n",minor_cmd,arg,padapter->HalFunc.read_rfreg(padapter, minor_cmd, arg, 0xffffffff));	
			break;
		case 0x75://write_rf
			padapter->HalFunc.write_rfreg(padapter, minor_cmd, arg, 0xffffffff, extra_arg);
			printk("write RF_reg path(0x%02x),offset(0x%x),value(0x%08x)\n",minor_cmd,arg, padapter->HalFunc.read_rfreg(padapter, minor_cmd, arg, 0xffffffff));
			break;	

		case 0x7F:
			switch(minor_cmd)
			{
				case 0x0:
					printk("fwstate=0x%x\n", pmlmepriv->fw_state);
					break;
				case 0x01:
					printk("auth_alg=0x%x, enc_alg=0x%x, auth_type=0x%x, enc_type=0x%x\n", 
						psecuritypriv->dot11AuthAlgrthm, psecuritypriv->dot11PrivacyAlgrthm,
						psecuritypriv->ndisauthtype, psecuritypriv->ndisencryptstatus);
					break;
				case 0x02:
					printk("pmlmeinfo->state=0x%x\n", pmlmeinfo->state);
					break;
				case 0x03:
					printk("qos_option=%d\n", pmlmepriv->qospriv.qos_option);
					printk("ht_option=%d\n", pmlmepriv->htpriv.ht_option);
					break;
				case 0x04:
					printk("cur_ch=%d\n", pmlmeext->cur_channel);
					printk("cur_bw=%d\n", pmlmeext->cur_bwmode);
					printk("cur_ch_off=%d\n", pmlmeext->cur_ch_offset);
					break;
				case 0x05:
					psta = get_stainfo(pstapriv, cur_network->network.MacAddress);
					if(psta)
					{
						int i;
						struct recv_reorder_ctrl *preorder_ctrl;
					
						printk("sta's macaddr:" MACSTR "\n", MAC2STR(psta->hwaddr));
						printk("rtsen=%d, cts2slef=%d\n", psta->rtsen, psta->cts2self);
						printk("qos_en=%d, ht_en=%d, init_rate=%d\n", psta->qos_option, psta->htpriv.ht_option, psta->init_rate);	
						printk("state=0x%x, aid=%d, macid=%d, raid=%d\n", psta->state, psta->aid, psta->mac_id, psta->raid);	
						printk("bwmode=%d, ch_offset=%d, sgi=%d\n", psta->htpriv.bwmode, psta->htpriv.ch_offset, psta->htpriv.sgi);						
						printk("ampdu_enable = %d\n", psta->htpriv.ampdu_enable);	
						printk("agg_enable_bitmap=%x, candidate_tid_bitmap=%x\n", psta->htpriv.agg_enable_bitmap, psta->htpriv.candidate_tid_bitmap);
						
						for(i=0;i<16;i++)
						{							
							preorder_ctrl = &psta->recvreorder_ctrl[i];
							if(preorder_ctrl->enable)
							{
								printk("tid=%d, indicate_seq=%d\n", i, preorder_ctrl->indicate_seq);
							}
						}	
							
					}
					else
					{							
						printk("can't get sta's macaddr, cur_network's macaddr:" MACSTR "\n", MAC2STR(cur_network->network.MacAddress));
					}					
					break;
				case 0x06:
					{
						u8	DMFlag;
						padapter->HalFunc.GetHwRegHandler(padapter, HW_VAR_DM_FLAG, (u8 *)(&DMFlag));
						printk("(B)DMFlag=0x%x, arg=0x%x\n", DMFlag, arg);
						DMFlag = (u8)(0x0f&arg);
						printk("(A)DMFlag=0x%x\n", DMFlag);
						padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_DM_FLAG, (u8 *)(&DMFlag));
					}
					break;
				case 0x07:
					printk("bSurpriseRemoved=%d, bDriverStopped=%d\n", 
						padapter->bSurpriseRemoved, padapter->bDriverStopped);
					break;
                                case 0x08:
					{
						struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
						struct recv_priv  *precvpriv = &padapter->recvpriv;
						
						printk("free_xmitbuf_cnt=%d, free_xmitframe_cnt=%d\n", 
							pxmitpriv->free_xmitbuf_cnt, pxmitpriv->free_xmitframe_cnt);
						#ifdef CONFIG_USB_HCI
						printk("rx_urb_pending_cn=%d\n", precvpriv->rx_pending_cnt);
						#endif
					}
					break;	
				case 0x09:
					{
						int i, j;
						_list	*plist, *phead;
						struct recv_reorder_ctrl *preorder_ctrl;
						
#ifdef CONFIG_AP_MODE
						printk("sta_dz_bitmap=0x%x, tim_bitmap=0x%x\n", pstapriv->sta_dz_bitmap, pstapriv->tim_bitmap);
#endif						
						_enter_critical_bh(&pstapriv->sta_hash_lock, &irqL);

						for(i=0; i< NUM_STA; i++)
						{
							phead = &(pstapriv->sta_hash[i]);
							plist = get_next(phead);
		
							while ((end_of_queue_search(phead, plist)) == _FALSE)
							{
								psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);

								plist = get_next(plist);

								if(extra_arg == psta->aid)
								{
									printk("sta's macaddr:" MACSTR "\n", MAC2STR(psta->hwaddr));
									printk("rtsen=%d, cts2slef=%d\n", psta->rtsen, psta->cts2self);
									printk("qos_en=%d, ht_en=%d, init_rate=%d\n", psta->qos_option, psta->htpriv.ht_option, psta->init_rate);	
									printk("state=0x%x, aid=%d, macid=%d, raid=%d\n", psta->state, psta->aid, psta->mac_id, psta->raid);	
									printk("bwmode=%d, ch_offset=%d, sgi=%d\n", psta->htpriv.bwmode, psta->htpriv.ch_offset, psta->htpriv.sgi);						
									printk("ampdu_enable = %d\n", psta->htpriv.ampdu_enable);									
									printk("agg_enable_bitmap=%x, candidate_tid_bitmap=%x\n", psta->htpriv.agg_enable_bitmap, psta->htpriv.candidate_tid_bitmap);
									printk("sleepq_len=%d\n", psta->sleepq_len);
						
									for(j=0;j<16;j++)
									{							
										preorder_ctrl = &psta->recvreorder_ctrl[j];
										if(preorder_ctrl->enable)
										{
											printk("tid=%d, indicate_seq=%d\n", j, preorder_ctrl->indicate_seq);
										}
									}		
									
								}							
			
							}
						}
	
						_exit_critical_bh(&pstapriv->sta_hash_lock, &irqL);

					}
					break;

                                case 0x0c://dump rx packet
					{
						printk("dump rx packet (%d)\n",extra_arg);						
						//pHalData->bDumpRxPkt =extra_arg;						
						padapter->HalFunc.SetHalDefVarHandler(padapter, HAL_DEF_DBG_DUMP_RXPKT, &(extra_arg));
					}
					break;
#if 0				
					case 0x0d://dump cam
					{
						//u8 entry = (u8) extra_arg;
						u8 entry=0;
						//dump cam
						for(entry=0;entry<32;entry++)
							read_cam(padapter,entry);
					}				
					break;
#endif
		#ifdef SILENT_RESET_FOR_SPECIFIC_PLATFOM
				case 0x0f:
						{
							if(extra_arg == 0){	
								printk("###### silent reset test.......#####\n");
								if(padapter->HalFunc.silentreset)
									padapter->HalFunc.silentreset(padapter);						
							}
							
						}
				break;
				case 0x12:
					{
						struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;	
						printk("==>silent resete cnts:%d\n",pwrpriv->ips_enter_cnts);
					}
					break;	
					
		#endif	

				case 0x10:// driver version display
					printk("rtw driver version=%s\n", DRIVERVERSION);		
					break;
				case 0x11:
					{
						printk("turn %s Rx RSSI display function\n",(extra_arg==1)?"on":"off");
						padapter->bRxRSSIDisplay = extra_arg ;						
					}
					break;
#if 1
	                        case 0xdd://registers dump , 0 for mac reg,1 for bb reg, 2 for rf reg
						{						
						if(extra_arg==0){
							mac_reg_dump(padapter);
						}
						else if(extra_arg==1){
							bb_reg_dump(padapter);
						}
						else if(extra_arg==2){
							rf_reg_dump(padapter);
						}
																				
					}
					break;		
#endif
				case 0xee://turn on/off dynamic funcs
					{
						u8 dm_flag;

						if(0xf==extra_arg){
							padapter->HalFunc.GetHalDefVarHandler(padapter, HAL_DEF_DBG_DM_FUNC,&dm_flag);							
							printk(" === DMFlag(0x%02x) === \n",dm_flag);
							printk("extra_arg = 0  - disable all dynamic func \n");
							printk("extra_arg = 1  - disable DIG- BIT(0)\n");
							printk("extra_arg = 2  - disable High power - BIT(1)\n");
							printk("extra_arg = 3  - disable tx power tracking - BIT(2)\n");
							printk("extra_arg = 4  - disable BT coexistence - BIT(3)\n");
							printk("extra_arg = 5  - disable antenna diversity - BIT(4)\n");
							printk("extra_arg = 6  - enable all dynamic func \n");							
						}
						else{
							/*	extra_arg = 0  - disable all dynamic func
								extra_arg = 1  - disable DIG
								extra_arg = 2  - disable tx power tracking
								extra_arg = 3  - turn on all dynamic func
							*/			
							padapter->HalFunc.SetHalDefVarHandler(padapter, HAL_DEF_DBG_DM_FUNC, &(extra_arg));
							padapter->HalFunc.GetHalDefVarHandler(padapter, HAL_DEF_DBG_DM_FUNC,&dm_flag);							
							printk(" === DMFlag(0x%02x) === \n",dm_flag);
						}
					}
					break;

				case 0xfd:
					write8(padapter, 0xc50, arg);
					printk("wr(0xc50)=0x%x\n", read8(padapter, 0xc50));
					write8(padapter, 0xc58, arg);
					printk("wr(0xc58)=0x%x\n", read8(padapter, 0xc58));
					break;
				case 0xfe:
					printk("rd(0xc50)=0x%x\n", read8(padapter, 0xc50));
					printk("rd(0xc58)=0x%x\n", read8(padapter, 0xc58));
					break;
				case 0xff:
					{
						printk("dbg(0x210)=0x%x\n", read32(padapter, 0x210));
						printk("dbg(0x608)=0x%x\n", read32(padapter, 0x608));
						printk("dbg(0x280)=0x%x\n", read32(padapter, 0x280));
						printk("dbg(0x284)=0x%x\n", read32(padapter, 0x284));
						printk("dbg(0x288)=0x%x\n", read32(padapter, 0x288));
	
						printk("dbg(0x664)=0x%x\n", read32(padapter, 0x664));


						printk("\n");
		
						printk("dbg(0x430)=0x%x\n", read32(padapter, 0x430));
						printk("dbg(0x438)=0x%x\n", read32(padapter, 0x438));

						printk("dbg(0x440)=0x%x\n", read32(padapter, 0x440));
	
						printk("dbg(0x458)=0x%x\n", read32(padapter, 0x458));
	
						printk("dbg(0x484)=0x%x\n", read32(padapter, 0x484));
						printk("dbg(0x488)=0x%x\n", read32(padapter, 0x488));
	
						printk("dbg(0x444)=0x%x\n", read32(padapter, 0x444));
						printk("dbg(0x448)=0x%x\n", read32(padapter, 0x448));
						printk("dbg(0x44c)=0x%x\n", read32(padapter, 0x44c));
						printk("dbg(0x450)=0x%x\n", read32(padapter, 0x450));
					}
					break;
			}			
			break;
		default:
			printk("error dbg cmd!\n");
			break;	
	}
	

	return ret;

}

static int wpa_set_param(struct net_device *dev, u8 name, u32 value)
{
	uint ret=0;
	u32 flags;
	_adapter *padapter = netdev_priv(dev);
	
	switch (name){
	case IEEE_PARAM_WPA_ENABLED:

		padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_8021X; //802.1x
		
		//ret = ieee80211_wpa_enable(ieee, value);
		
		switch((value)&0xff)
		{
			case 1 : //WPA
			padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeWPAPSK; //WPA_PSK
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption2Enabled;
				break;
			case 2: //WPA2
			padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeWPA2PSK; //WPA2_PSK
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption3Enabled;
				break;
		}
		
		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_,("wpa_set_param:padapter->securitypriv.ndisauthtype=%d\n", padapter->securitypriv.ndisauthtype));
		
		break;

	case IEEE_PARAM_TKIP_COUNTERMEASURES:
		//ieee->tkip_countermeasures=value;
		break;

	case IEEE_PARAM_DROP_UNENCRYPTED: 
	{
		/* HACK:
		 *
		 * wpa_supplicant calls set_wpa_enabled when the driver
		 * is loaded and unloaded, regardless of if WPA is being
		 * used.  No other calls are made which can be used to
		 * determine if encryption will be used or not prior to
		 * association being expected.  If encryption is not being
		 * used, drop_unencrypted is set to false, else true -- we
		 * can use this to determine if the CAP_PRIVACY_ON bit should
		 * be set.
		 */
		 
#if 0	 
		struct ieee80211_security sec = {
			.flags = SEC_ENABLED,
			.enabled = value,
		};
 		ieee->drop_unencrypted = value;
		/* We only change SEC_LEVEL for open mode. Others
		 * are set by ipw_wpa_set_encryption.
		 */
		if (!value) {
			sec.flags |= SEC_LEVEL;
			sec.level = SEC_LEVEL_0;
		}
		else {
			sec.flags |= SEC_LEVEL;
			sec.level = SEC_LEVEL_1;
		}
		if (ieee->set_security)
			ieee->set_security(ieee->dev, &sec);
#endif		
		break;

	}
	case IEEE_PARAM_PRIVACY_INVOKED:	
		
		//ieee->privacy_invoked=value;
		
		break;

	case IEEE_PARAM_AUTH_ALGS:
		
		ret = wpa_set_auth_algs(dev, value);
		
		break;

	case IEEE_PARAM_IEEE_802_1X:
		
		//ieee->ieee802_1x=value;		
		
		break;
		
	case IEEE_PARAM_WPAX_SELECT:
		
		// added for WPA2 mixed mode
		//printk(KERN_WARNING "------------------------>wpax value = %x\n", value);
		/*
		spin_lock_irqsave(&ieee->wpax_suitlist_lock,flags);
		ieee->wpax_type_set = 1;
		ieee->wpax_type_notify = value;
		spin_unlock_irqrestore(&ieee->wpax_suitlist_lock,flags);
		*/
		
		break;

	default:		


		
		ret = -EOPNOTSUPP;

		
		break;
	
	}

	return ret;
	
}

static int wpa_mlme(struct net_device *dev, u32 command, u32 reason)
{	
	int ret = 0;
	_adapter *padapter = netdev_priv(dev);

	switch (command)
	{
		case IEEE_MLME_STA_DEAUTH:

			if(!set_802_11_disassociate(padapter))
				ret = -1;		
			
			break;

		case IEEE_MLME_STA_DISASSOC:
		
			if(!set_802_11_disassociate(padapter))
				ret = -1;		
	
			break;

		default:
			ret = -EOPNOTSUPP;
			break;
	}

	return ret;
	
}

static int wpa_supplicant_ioctl(struct net_device *dev, struct iw_point *p)
{
	struct ieee_param *param;
	uint ret=0;

	//down(&ieee->wx_sem);	

	if (p->length < sizeof(struct ieee_param) || !p->pointer){
		ret = -EINVAL;
		goto out;
	}
	
	param = (struct ieee_param *)rtw_malloc(p->length);
	if (param == NULL)
	{
		ret = -ENOMEM;
		goto out;
	}
	
	if (copy_from_user(param, p->pointer, p->length))
	{
		rtw_mfree((u8*)param, p->length);
		ret = -EFAULT;
		goto out;
	}

	switch (param->cmd) {

	case IEEE_CMD_SET_WPA_PARAM:
		ret = wpa_set_param(dev, param->u.wpa_param.name, param->u.wpa_param.value);
		break;

	case IEEE_CMD_SET_WPA_IE:
		//ret = wpa_set_wpa_ie(dev, param, p->length);
		ret =  rtw_set_wpa_ie((_adapter *)netdev_priv(dev), (char*)param->u.wpa_ie.data, (u16)param->u.wpa_ie.len);
		break;

	case IEEE_CMD_SET_ENCRYPTION:
		ret = wpa_set_encryption(dev, param, p->length);
		break;

	case IEEE_CMD_MLME:
		ret = wpa_mlme(dev, param->u.mlme.command, param->u.mlme.reason_code);
		break;

	default:
		printk("Unknown WPA supplicant request: %d\n", param->cmd);
		ret = -EOPNOTSUPP;
		break;
		
	}

	if (ret == 0 && copy_to_user(p->pointer, param, p->length))
		ret = -EFAULT;

	rtw_mfree((u8 *)param, p->length);
	
out:
	
	//up(&ieee->wx_sem);
	
	return ret;
	
}

#ifdef CONFIG_AP_MODE
static u8 set_pairwise_key(_adapter *padapter, struct sta_info *psta)
{
	struct cmd_obj*			ph2c;
	struct set_stakey_parm	*psetstakey_para;
	struct cmd_priv 			*pcmdpriv=&padapter->cmdpriv;	
	u8	res=_SUCCESS;

	ph2c = (struct cmd_obj*)rtw_zmalloc(sizeof(struct cmd_obj));
	if ( ph2c == NULL){
		res= _FAIL;
		goto exit;
	}

	psetstakey_para = (struct set_stakey_parm*)rtw_zmalloc(sizeof(struct set_stakey_parm));
	if(psetstakey_para==NULL){
		rtw_mfree((u8 *) ph2c, sizeof(struct cmd_obj));
		res=_FAIL;
		goto exit;
	}

	init_h2fwcmd_w_parm_no_rsp(ph2c, psetstakey_para, _SetStaKey_CMD_);


	psetstakey_para->algorithm = (u8)psta->dot118021XPrivacy;

	_memcpy(psetstakey_para->addr, psta->hwaddr, ETH_ALEN);	
	
	_memcpy(psetstakey_para->key, &psta->dot118021x_UncstKey, 16);

	
	enqueue_cmd(pcmdpriv, ph2c);	

exit:

	return res;
	
}

static int set_group_key(_adapter *padapter, u8 *key, u8 alg, int keyid)
{
	u8 keylen;
	struct cmd_obj* pcmd;
	struct setkey_parm *psetkeyparm;
	struct cmd_priv	*pcmdpriv=&(padapter->cmdpriv);	
	int res=_SUCCESS;

	printk("%s\n", __FUNCTION__);
	
	pcmd = (struct cmd_obj*)rtw_zmalloc(sizeof(struct	cmd_obj));
	if(pcmd==NULL){
		res= _FAIL;
		goto exit;
	}
	psetkeyparm=(struct setkey_parm*)rtw_zmalloc(sizeof(struct setkey_parm));
	if(psetkeyparm==NULL){
		rtw_mfree((unsigned char *)pcmd, sizeof(struct cmd_obj));
		res= _FAIL;
		goto exit;
	}

	_memset(psetkeyparm, 0, sizeof(struct setkey_parm));
		
	psetkeyparm->keyid=(u8)keyid;

	psetkeyparm->algorithm = alg;

	switch(alg)
	{
		case _WEP40_:					
			keylen = 5;
			break;
		case _WEP104_:
			keylen = 13;			
			break;
		case _TKIP_:
		case _TKIP_WTMIC_:		
		case _AES_:
			keylen = 16;		
		default:
			keylen = 16;		
	}

	_memcpy(&(psetkeyparm->key[0]), key, keylen);
	
	pcmd->cmdcode = _SetKey_CMD_;
	pcmd->parmbuf = (u8 *)psetkeyparm;   
	pcmd->cmdsz =  (sizeof(struct setkey_parm));  
	pcmd->rsp = NULL;
	pcmd->rspsz = 0;


	_init_listhead(&pcmd->list);

	enqueue_cmd(pcmdpriv, pcmd);

exit:

	return _SUCCESS;
	

}

static int set_wep_key(_adapter *padapter, u8 *key, u8 keylen, int keyid)
{	
	u8 alg;

	switch(keylen)
	{
		case 5:
			alg =_WEP40_;			
			break;
		case 13:
			alg =_WEP104_;			
			break;
		default:
			alg =_NO_PRIVACY_;			
	}

	return set_group_key(padapter, key, alg, keyid);

}


static int rtw_set_encryption(struct net_device *dev, struct ieee_param *param, u32 param_len)
{
	int ret = 0;
	u32 wep_key_idx, wep_key_len;
	NDIS_802_11_WEP	 *pwep = NULL;
	struct sta_info *psta = NULL, *pbcmc_sta = NULL;	
	_adapter *padapter = (_adapter *)netdev_priv(dev);
	struct mlme_priv 	*pmlmepriv = &padapter->mlmepriv;
	struct security_priv* psecuritypriv=&(padapter->securitypriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	printk("%s\n", __FUNCTION__);

	param->u.crypt.err = 0;
	param->u.crypt.alg[IEEE_CRYPT_ALG_NAME_LEN - 1] = '\0';

	//sizeof(struct ieee_param) = 64 bytes;
	//if (param_len !=  (u32) ((u8 *) param->u.crypt.key - (u8 *) param) + param->u.crypt.key_len)
	if (param_len !=  sizeof(struct ieee_param) + param->u.crypt.key_len)
	{
		ret =  -EINVAL;
		goto exit;
	}

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff) 
	{
		if (param->u.crypt.idx >= WEP_KEYS)
		{
			ret = -EINVAL;
			goto exit;
		}	
	}
	else 
	{		
		psta = get_stainfo(pstapriv, param->sta_addr);
		if(!psta)
		{
			//ret = -EINVAL;
			printk("rtw_set_encryption(), sta has already been removed or never been added\n");
			goto exit;
		}			
	}

	if (strcmp(param->u.crypt.alg, "none") == 0 && (psta==NULL))
	{
		//todo:clear default encryption keys

		printk("clear default encryption keys, keyid=%d\n", param->u.crypt.idx);
		
		goto exit;
	}


	if (strcmp(param->u.crypt.alg, "WEP") == 0 && (psta==NULL))
	{		
		printk("r871x_set_encryption, crypt.alg = WEP\n");
		
		wep_key_idx = param->u.crypt.idx;
		wep_key_len = param->u.crypt.key_len;
					
		printk("r871x_set_encryption, wep_key_idx=%d, len=%d\n", wep_key_idx, wep_key_len);

		if((wep_key_idx >= WEP_KEYS) || (wep_key_len<=0))
		{
			ret = -EINVAL;
			goto exit;
		}
			

		if (wep_key_len > 0) 
		{			
		 	wep_key_len = wep_key_len <= 5 ? 5 : 13;

		 	pwep =(NDIS_802_11_WEP *)rtw_malloc(wep_key_len + FIELD_OFFSET(NDIS_802_11_WEP, KeyMaterial));
			if(pwep == NULL){
				printk(" r871x_set_encryption: pwep allocate fail !!!\n");
				goto exit;
			}
			
		 	_memset(pwep, 0, sizeof(NDIS_802_11_WEP));
		
		 	pwep->KeyLength = wep_key_len;
			pwep->Length = wep_key_len + FIELD_OFFSET(NDIS_802_11_WEP, KeyMaterial);
			
		}
		
		pwep->KeyIndex = wep_key_idx;

		_memcpy(pwep->KeyMaterial,  param->u.crypt.key, pwep->KeyLength);

		if(param->u.crypt.set_tx)
		{
			printk("wep, set_tx=1\n");

			psecuritypriv->ndisencryptstatus = Ndis802_11Encryption1Enabled;
			psecuritypriv->dot11PrivacyAlgrthm=_WEP40_;
			psecuritypriv->dot118021XGrpPrivacy=_WEP40_;
			
			if(pwep->KeyLength==13)
			{
				psecuritypriv->dot11PrivacyAlgrthm=_WEP104_;
				psecuritypriv->dot118021XGrpPrivacy=_WEP104_;
			}

		
			psecuritypriv->dot11PrivacyKeyIndex = wep_key_idx;
			
			_memcpy(&(psecuritypriv->dot11DefKey[wep_key_idx].skey[0]), pwep->KeyMaterial, pwep->KeyLength);

			psecuritypriv->dot11DefKeylen[wep_key_idx]=pwep->KeyLength;

			set_wep_key(padapter, pwep->KeyMaterial, pwep->KeyLength, wep_key_idx);		

			
		}
		else
		{
			printk("wep, set_tx=0\n");
			
			//don't update "psecuritypriv->dot11PrivacyAlgrthm" and 
			//"psecuritypriv->dot11PrivacyKeyIndex=keyid", but can set_key to cam
					
		      _memcpy(&(psecuritypriv->dot11DefKey[wep_key_idx].skey[0]), pwep->KeyMaterial, pwep->KeyLength);

			psecuritypriv->dot11DefKeylen[wep_key_idx] = pwep->KeyLength;			

			set_wep_key(padapter, pwep->KeyMaterial, pwep->KeyLength, wep_key_idx);
			
		}

		goto exit;
		
	}

	
	if(!psta && check_fwstate(pmlmepriv, WIFI_AP_STATE)) // //group key
	{
		if(param->u.crypt.set_tx ==1)
		{
			if(strcmp(param->u.crypt.alg, "WEP") == 0)
			{
				printk("%s, set group_key, WEP\n", __FUNCTION__);
				
				_memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
					
				psecuritypriv->dot118021XGrpPrivacy = _WEP40_;
				if(param->u.crypt.key_len==13)
				{						
						psecuritypriv->dot118021XGrpPrivacy = _WEP104_;
				}
				
			}
			else if(strcmp(param->u.crypt.alg, "TKIP") == 0)
			{						
				printk("%s, set group_key, TKIP\n", __FUNCTION__);
				
				psecuritypriv->dot118021XGrpPrivacy = _TKIP_;

				_memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx-1].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
				
				//DEBUG_ERR("set key length :param->u.crypt.key_len=%d\n", param->u.crypt.key_len);
				//set mic key
				_memcpy(psecuritypriv->dot118021XGrptxmickey.skey, &(param->u.crypt.key[16]), 8);
				_memcpy(psecuritypriv->dot118021XGrprxmickey.skey, &(param->u.crypt.key[24]), 8);

				psecuritypriv->busetkipkey = _TRUE;
											
			}
			else if(strcmp(param->u.crypt.alg, "CCMP") == 0)
			{
				printk("%s, set group_key, CCMP\n", __FUNCTION__);
			
				psecuritypriv->dot118021XGrpPrivacy = _AES_;

				_memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx-1].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
			}
			else
			{
				printk("%s, set group_key, none\n", __FUNCTION__);
				
				psecuritypriv->dot118021XGrpPrivacy = _NO_PRIVACY_;
			}

			psecuritypriv->dot118021XGrpKeyid = param->u.crypt.idx;

			psecuritypriv->binstallGrpkey = _TRUE;

			psecuritypriv->dot11PrivacyAlgrthm = psecuritypriv->dot118021XGrpPrivacy;//!!!
								
			set_group_key(padapter, param->u.crypt.key, psecuritypriv->dot118021XGrpPrivacy, param->u.crypt.idx);
			
			pbcmc_sta=get_bcmc_stainfo(padapter);
			if(pbcmc_sta)
			{
				pbcmc_sta->ieee8021x_blocked = _FALSE;
				pbcmc_sta->dot118021XPrivacy= psecuritypriv->dot118021XGrpPrivacy;//rx will use bmc_sta's dot118021XPrivacy			
			}	
						
		}

		goto exit;
		
	}	

	if(psecuritypriv->dot11AuthAlgrthm == dot11AuthAlgrthm_8021X && psta) // psk/802_1x
	{
		if(check_fwstate(pmlmepriv, WIFI_AP_STATE))
		{
			if(param->u.crypt.set_tx ==1)
			{ 
				_memcpy(psta->dot118021x_UncstKey.skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
				
				if(strcmp(param->u.crypt.alg, "WEP") == 0)
				{
					printk("%s, set pairwise key, WEP\n", __FUNCTION__);
					
					psta->dot118021XPrivacy = _WEP40_;
					if(param->u.crypt.key_len==13)
					{						
						psta->dot118021XPrivacy = _WEP104_;
					}
				}
				else if(strcmp(param->u.crypt.alg, "TKIP") == 0)
				{						
					printk("%s, set pairwise key, TKIP\n", __FUNCTION__);
					
					psta->dot118021XPrivacy = _TKIP_;
				
					//DEBUG_ERR("set key length :param->u.crypt.key_len=%d\n", param->u.crypt.key_len);
					//set mic key
					_memcpy(psta->dot11tkiptxmickey.skey, &(param->u.crypt.key[16]), 8);
					_memcpy(psta->dot11tkiprxmickey.skey, &(param->u.crypt.key[24]), 8);

					psecuritypriv->busetkipkey = _TRUE;
											
				}
				else if(strcmp(param->u.crypt.alg, "CCMP") == 0)
				{

					printk("%s, set pairwise key, CCMP\n", __FUNCTION__);
					
					psta->dot118021XPrivacy = _AES_;
				}
				else
				{
					printk("%s, set pairwise key, none\n", __FUNCTION__);
					
					psta->dot118021XPrivacy = _NO_PRIVACY_;
				}
						
				set_pairwise_key(padapter, psta);
					
				psta->ieee8021x_blocked = _FALSE;
					
			}			
			else//group key???
			{ 
				if(strcmp(param->u.crypt.alg, "WEP") == 0)
				{
					_memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
					
					psecuritypriv->dot118021XGrpPrivacy = _WEP40_;
					if(param->u.crypt.key_len==13)
					{						
						psecuritypriv->dot118021XGrpPrivacy = _WEP104_;
					}
				}
				else if(strcmp(param->u.crypt.alg, "TKIP") == 0)
				{						
					psecuritypriv->dot118021XGrpPrivacy = _TKIP_;

					_memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx-1].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
				
					//DEBUG_ERR("set key length :param->u.crypt.key_len=%d\n", param->u.crypt.key_len);
					//set mic key
					_memcpy(psecuritypriv->dot118021XGrptxmickey.skey, &(param->u.crypt.key[16]), 8);
					_memcpy(psecuritypriv->dot118021XGrprxmickey.skey, &(param->u.crypt.key[24]), 8);

					psecuritypriv->busetkipkey = _TRUE;
											
				}
				else if(strcmp(param->u.crypt.alg, "CCMP") == 0)
				{
					psecuritypriv->dot118021XGrpPrivacy = _AES_;

					_memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx-1].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
				}
				else
				{
					psecuritypriv->dot118021XGrpPrivacy = _NO_PRIVACY_;
				}

				psecuritypriv->dot118021XGrpKeyid = param->u.crypt.idx;

				psecuritypriv->binstallGrpkey = _TRUE;	
								
				psecuritypriv->dot11PrivacyAlgrthm = psecuritypriv->dot118021XGrpPrivacy;//!!!
								
				set_group_key(padapter, param->u.crypt.key, psecuritypriv->dot118021XGrpPrivacy, param->u.crypt.idx);
			
				pbcmc_sta=get_bcmc_stainfo(padapter);
				if(pbcmc_sta)
				{
					pbcmc_sta->ieee8021x_blocked = _FALSE;
					pbcmc_sta->dot118021XPrivacy= psecuritypriv->dot118021XGrpPrivacy;//rx will use bmc_sta's dot118021XPrivacy			
				}					

			}
			
		}
				
	}

exit:

	if(pwep)
	{
		rtw_mfree((u8 *)pwep, wep_key_len + FIELD_OFFSET(NDIS_802_11_WEP, KeyMaterial));		
	}	
	
	return ret;
	
}

static int rtw_set_beacon(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	unsigned char *p;
	struct sta_info *psta = NULL;
	unsigned short cap, ht_cap=_FALSE;
	unsigned int ie_len = 0;
	int group_cipher, pairwise_cipher;	
	unsigned char	channel, network_type, supportRate[NDIS_802_11_LENGTH_RATES_EX];
	int supportRateNum = 0;
	unsigned char OUI1[] = {0x00, 0x50, 0xf2,0x01};
	unsigned char wps_oui[4]={0x0,0x50,0xf2,0x04};
	unsigned char WMM_PARA_IE[] = {0x00, 0x50, 0xf2, 0x02, 0x01, 0x01};	
	_adapter *padapter = (_adapter *)netdev_priv(dev);
	struct registry_priv *pregistrypriv = &padapter->registrypriv;	
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	WLAN_BSSID_EX *pbss_network = (WLAN_BSSID_EX *)&pmlmepriv->cur_network.network;	
	struct sta_priv *pstapriv = &padapter->stapriv;
	unsigned char *ie = pbss_network->IEs;
	unsigned char *pbuf = param->u.bcn_ie.buf;

	/* SSID */
	/* Supported rates */
	/* DS Params */
	/* WLAN_EID_COUNTRY */
	/* ERP Information element */
	/* Extended supported rates */
	/* WPA/WPA2 */
	/* Wi-Fi Wireless Multimedia Extensions */
	/* ht_capab, ht_oper */
	/* WPS IE */


	printk("%s, len=%d\n", __FUNCTION__, len);

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) != _TRUE)
		return -EINVAL;


	_memcpy(&pstapriv->max_num_sta, param->u.bcn_ie.reserved, 2);

	if((pstapriv->max_num_sta>NUM_STA) || (pstapriv->max_num_sta<=0))
		pstapriv->max_num_sta = NUM_STA;
	
	pbss_network->IELength = len-12-2;// 12 = param header, 2:no packed

	if(pbss_network->IELength>MAX_IE_SZ)
		return -ENOMEM;
	

	_memset(ie, 0, MAX_IE_SZ);
	
	_memcpy(ie, pbuf, pbss_network->IELength);


	if(pbss_network->InfrastructureMode!=Ndis802_11APMode)
		return -EINVAL;

	pbss_network->Rssi = 0;

	_memcpy(pbss_network->MacAddress, myid(&(padapter->eeprompriv)), ETH_ALEN);
	
	//beacon interval
	p = get_beacon_interval_from_ie(ie);//ie + 8;	// 8: TimeStamp, 2: Beacon Interval 2:Capability
	pbss_network->Configuration.BeaconPeriod = *(unsigned short*)p;
	
	//capability
	cap = *(unsigned short *)get_capability_from_ie(ie);
	cap = le16_to_cpu(cap);

	//SSID
	p = get_ie(ie + _BEACON_IE_OFFSET_, _SSID_IE_, &ie_len, (pbss_network->IELength -_BEACON_IE_OFFSET_));
	if(p && ie_len>0)
	{
		_memset(&pbss_network->Ssid, 0, sizeof(NDIS_802_11_SSID));
		_memcpy(pbss_network->Ssid.Ssid, (p + 2), ie_len);
		pbss_network->Ssid.SsidLength = ie_len;
	}	

	//chnnel
	channel = 0;
	pbss_network->Configuration.Length = 0;
	p = get_ie(ie + _BEACON_IE_OFFSET_, _DSSET_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
	if(p && ie_len>0)
		channel = *(p + 2);

	pbss_network->Configuration.DSConfig = channel;

	
	_memset(supportRate, 0, NDIS_802_11_LENGTH_RATES_EX);
	// get supported rates
	p = get_ie(ie + _BEACON_IE_OFFSET_, _SUPPORTEDRATES_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));	
	if (p !=  NULL) 
	{
		_memcpy(supportRate, p+2, ie_len);	
		supportRateNum = ie_len;
	}
	
	//get ext_supported rates
	p = get_ie(ie + _BEACON_IE_OFFSET_, _EXT_SUPPORTEDRATES_IE_, &ie_len, pbss_network->IELength - _BEACON_IE_OFFSET_);	
	if (p !=  NULL)
	{
		_memcpy(supportRate+supportRateNum, p+2, ie_len);
		supportRateNum += ie_len;
	
	}

	network_type = check_network_type(supportRate, supportRateNum, channel);

	set_supported_rate(pbss_network->SupportedRates, network_type);


	//parsing ERP_IE
	p = get_ie(ie + _BEACON_IE_OFFSET_, _ERPINFO_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
	if(p && ie_len>0)
	{
		ERP_IE_handler(padapter, (PNDIS_802_11_VARIABLE_IEs)p);
	}

	//parsing HT_CAP_IE
	p = get_ie(ie + _BEACON_IE_OFFSET_, _HT_CAPABILITY_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
	if(p && ie_len>0)
	{
		u8 rf_type;
		ht_cap = _TRUE;
		network_type |= WIRELESS_11_24N;

		padapter->HalFunc.GetHwRegHandler(padapter, HW_VAR_RF_TYPE, (u8 *)(&rf_type));

		if(rf_type == RF_1T1R)
		{
			struct ieee80211_ht_cap *pht_cap = (struct ieee80211_ht_cap *)(p+2);
			
			pht_cap->supp_mcs_set[0] = 0xff;
			pht_cap->supp_mcs_set[1] = 0x0;				
		}			
		
		_memcpy(&pmlmepriv->htpriv.ht_cap, p+2, ie_len);		
	}

	switch(network_type)
	{
		case WIRELESS_11B:
			pbss_network->NetworkTypeInUse = Ndis802_11DS;
			break;	
		case WIRELESS_11G:
		case WIRELESS_11BG:
             case WIRELESS_11G_24N:
		case WIRELESS_11BG_24N:
			pbss_network->NetworkTypeInUse = Ndis802_11OFDM24;
			break;
		case WIRELESS_11A:
			pbss_network->NetworkTypeInUse = Ndis802_11OFDM5;
			break;
		default :
			pbss_network->NetworkTypeInUse = Ndis802_11OFDM24;
			break;
	}
	
	pmlmepriv->cur_network.network_type = network_type;

	//update privacy/security
	if (cap & BIT(4))
		pbss_network->Privacy = 1;
	else
		pbss_network->Privacy = 0;

	psecuritypriv->wpa_psk = 0;

	//wpa2
	group_cipher = 0; pairwise_cipher = 0;
	psecuritypriv->wpa2_group_cipher = _NO_PRIVACY_;
	psecuritypriv->wpa2_pairwise_cipher = _NO_PRIVACY_;	
	p = get_ie(ie + _BEACON_IE_OFFSET_, _RSN_IE_2_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));		
	if(p && ie_len>0)
	{
		if(parse_wpa2_ie(p, ie_len+2, &group_cipher, &pairwise_cipher) == _SUCCESS)
		{
			psecuritypriv->dot11AuthAlgrthm= dot11AuthAlgrthm_8021X;
			
			psecuritypriv->dot8021xalg = 1;//psk,  todo:802.1x
			psecuritypriv->wpa_psk |= BIT(1);

			psecuritypriv->wpa2_group_cipher = group_cipher;
			psecuritypriv->wpa2_pairwise_cipher = pairwise_cipher;
#if 0
			switch(group_cipher)
			{
				case WPA_CIPHER_NONE:				
				psecuritypriv->wpa2_group_cipher = _NO_PRIVACY_;
				break;
				case WPA_CIPHER_WEP40:				
				psecuritypriv->wpa2_group_cipher = _WEP40_;
				break;
				case WPA_CIPHER_TKIP:				
				psecuritypriv->wpa2_group_cipher = _TKIP_;
				break;
				case WPA_CIPHER_CCMP:				
				psecuritypriv->wpa2_group_cipher = _AES_;				
				break;
				case WPA_CIPHER_WEP104:					
				psecuritypriv->wpa2_group_cipher = _WEP104_;
				break;
			}

			switch(pairwise_cipher)
			{
				case WPA_CIPHER_NONE:			
				psecuritypriv->wpa2_pairwise_cipher = _NO_PRIVACY_;
				break;
				case WPA_CIPHER_WEP40:			
				psecuritypriv->wpa2_pairwise_cipher = _WEP40_;
				break;
				case WPA_CIPHER_TKIP:				
				psecuritypriv->wpa2_pairwise_cipher = _TKIP_;
				break;
				case WPA_CIPHER_CCMP:			
				psecuritypriv->wpa2_pairwise_cipher = _AES_;
				break;
				case WPA_CIPHER_WEP104:					
				psecuritypriv->wpa2_pairwise_cipher = _WEP104_;
				break;
			}
#endif			
		}
		
	}

	//wpa
	ie_len = 0;
	group_cipher = 0; pairwise_cipher = 0;
	psecuritypriv->wpa_group_cipher = _NO_PRIVACY_;
	psecuritypriv->wpa_pairwise_cipher = _NO_PRIVACY_;	
	for (p = ie + _BEACON_IE_OFFSET_; ;p += (ie_len + 2))
	{
		p = get_ie(p, _SSN_IE_1_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_ - (ie_len + 2)));		
		if ((p) && (_memcmp(p+2, OUI1, 4)))
		{
			if(parse_wpa_ie(p, ie_len+2, &group_cipher, &pairwise_cipher) == _SUCCESS)
			{
				psecuritypriv->dot11AuthAlgrthm= dot11AuthAlgrthm_8021X;
				
				psecuritypriv->dot8021xalg = 1;//psk,  todo:802.1x

				psecuritypriv->wpa_psk |= BIT(0);

				psecuritypriv->wpa_group_cipher = group_cipher;
				psecuritypriv->wpa_pairwise_cipher = pairwise_cipher;

#if 0
				switch(group_cipher)
				{
					case WPA_CIPHER_NONE:					
					psecuritypriv->wpa_group_cipher = _NO_PRIVACY_;
					break;
					case WPA_CIPHER_WEP40:					
					psecuritypriv->wpa_group_cipher = _WEP40_;
					break;
					case WPA_CIPHER_TKIP:					
					psecuritypriv->wpa_group_cipher = _TKIP_;
					break;
					case WPA_CIPHER_CCMP:					
					psecuritypriv->wpa_group_cipher = _AES_;				
					break;
					case WPA_CIPHER_WEP104:					
					psecuritypriv->wpa_group_cipher = _WEP104_;
					break;
				}

				switch(pairwise_cipher)
				{
					case WPA_CIPHER_NONE:					
					psecuritypriv->wpa_pairwise_cipher = _NO_PRIVACY_;
					break;
					case WPA_CIPHER_WEP40:					
					psecuritypriv->wpa_pairwise_cipher = _WEP40_;
					break;
					case WPA_CIPHER_TKIP:					
					psecuritypriv->wpa_pairwise_cipher = _TKIP_;
					break;
					case WPA_CIPHER_CCMP:					
					psecuritypriv->wpa_pairwise_cipher = _AES_;
					break;
					case WPA_CIPHER_WEP104:					
					psecuritypriv->wpa_pairwise_cipher = _WEP104_;
					break;
				}
#endif				
			}

			break;
			
		}
			
		if ((p == NULL) || (ie_len == 0))
		{
				break;
		}
		
	}

	//wmm
	ie_len = 0;
	pmlmepriv->qospriv.qos_option = 0;
	if(pregistrypriv->wmm_enable)
	{
		for (p = ie + _BEACON_IE_OFFSET_; ;p += (ie_len + 2))
		{			
			p = get_ie(p, _VENDOR_SPECIFIC_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_ - (ie_len + 2)));	
			if((p) && _memcmp(p+2, WMM_PARA_IE, 6)) 
			{
				pmlmepriv->qospriv.qos_option = 1;	

				*(p+8) |= BIT(7);//QoS Info, support U-APSD
				
				break;				
			}
			
			if ((p == NULL) || (ie_len == 0))
			{
				break;
			}			
		}		
	}

	pmlmepriv->htpriv.ht_option = _FALSE;
#ifdef CONFIG_80211N_HT
	if( (psecuritypriv->wpa2_pairwise_cipher&WPA_CIPHER_TKIP) ||
		      (psecuritypriv->wpa_pairwise_cipher&WPA_CIPHER_TKIP))
	{	
                //todo:
		//ht_cap = _FALSE;
	}
		      
	//ht_cap	
	if(pregistrypriv->ht_enable && ht_cap==_TRUE)
	{		
		pmlmepriv->htpriv.ht_option = _TRUE;
		pmlmepriv->qospriv.qos_option = 1;

		if(pregistrypriv->ampdu_enable==1)
		{
			pmlmepriv->htpriv.ampdu_enable = _TRUE;
		}
	}
#endif


	pbss_network->Length = get_WLAN_BSSID_EX_sz((WLAN_BSSID_EX  *)pbss_network);

	//issue beacon to start bss network
	start_bss_network(padapter, (u8*)pbss_network);
			

	//alloc sta_info for ap itself
	psta = get_stainfo(&padapter->stapriv, pbss_network->MacAddress);
	if(!psta)
	{
		psta = alloc_stainfo(&padapter->stapriv, pbss_network->MacAddress);
		if (psta == NULL) 
		{ 
			return -EINVAL;
		}	
	}	
			
	indicate_connect( padapter);

	pmlmepriv->cur_network.join_res = _TRUE;//for check if already set beacon
		
	//update bc/mc sta_info
	//update_bmc_sta(padapter);

	return ret;
	
}

static int rtw_hostapd_sta_flush(struct net_device *dev)
{
	//_irqL irqL;
	//_list	*phead, *plist;
	int ret=0;	
	//struct sta_info *psta = NULL;
	_adapter *padapter = (_adapter *)netdev_priv(dev);	
	//struct sta_priv *pstapriv = &padapter->stapriv;

	printk("%s\n", __FUNCTION__);

	flush_all_cam_entry(padapter);	//clear CAM

#if 0
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);
	
	//free sta asoc_queue
	while ((end_of_queue_search(phead, plist)) == _FALSE)	
	{		
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		
		plist = get_next(plist);

		list_delete(&psta->asoc_list);		

		//tear down Rx AMPDU
		send_delba(padapter, 0, psta->hwaddr);// recipient
	
		//tear down TX AMPDU
		send_delba(padapter, 1, psta->hwaddr);// // originator
		psta->htpriv.agg_enable_bitmap = 0x0;//reset
		psta->htpriv.candidate_tid_bitmap = 0x0;//reset

		issue_deauth(padapter, psta->hwaddr, WLAN_REASON_DEAUTH_LEAVING);

		_enter_critical_bh(&(pstapriv->sta_hash_lock), &irqL);					
		free_stainfo(padapter, psta);
		_exit_critical_bh(&(pstapriv->sta_hash_lock), &irqL);
		
	}
#endif

	ret = rtw_sta_flush(padapter);	

	return ret;

}

static int rtw_add_sta(struct net_device *dev, struct ieee_param *param)
{
	_irqL irqL;
	int ret=0;	
	struct sta_info *psta = NULL;
	_adapter *padapter = (_adapter *)netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	printk("rtw_add_sta(aid=%d)=" MACSTR "\n", param->u.add_sta.aid, MAC2STR(param->sta_addr));
	
	if(check_fwstate(pmlmepriv, (_FW_LINKED|WIFI_AP_STATE)) != _TRUE)	
	{
		return -EINVAL;		
	}

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff) 
	{
		return -EINVAL;	
	}

/*
	psta = get_stainfo(pstapriv, param->sta_addr);
	if(psta)
	{
		printk("rtw_add_sta(), free has been added psta=%p\n", psta);
		_enter_critical_bh(&(pstapriv->sta_hash_lock), &irqL);		
		free_stainfo(padapter,  psta);		
		_exit_critical_bh(&(pstapriv->sta_hash_lock), &irqL);

		psta = NULL;
	}	
*/
	//psta = alloc_stainfo(pstapriv, param->sta_addr);
	psta = get_stainfo(pstapriv, param->sta_addr);
	if(psta)
	{
		int flags = param->u.add_sta.flags;			
		
		//printk("rtw_add_sta(), init sta's variables, psta=%p\n", psta);
		
		psta->aid = param->u.add_sta.aid;//aid=1~2007

		_memcpy(psta->bssrateset, param->u.add_sta.tx_supp_rates, 16);
		
		
		//check wmm cap.
		if(WLAN_STA_WME&flags)
			psta->qos_option = 1;
		else
			psta->qos_option = 0;

		if(pmlmepriv->qospriv.qos_option == 0)	
			psta->qos_option = 0;

		
#ifdef CONFIG_80211N_HT		
		//chec 802.11n ht cap.
		if(WLAN_STA_HT&flags)
		{
			psta->htpriv.ht_option = _TRUE;
			psta->qos_option = 1;
			_memcpy((void*)&psta->htpriv.ht_cap, (void*)&param->u.add_sta.ht_cap, sizeof(struct ieee80211_ht_cap));
		}
		else		
		{
			psta->htpriv.ht_option = _FALSE;
		}
		
		if(pmlmepriv->htpriv.ht_option == _FALSE)	
			psta->htpriv.ht_option = _FALSE;
#endif		


		update_sta_info_apmode(padapter, psta);
		
		
	}
	else
	{
		ret = -ENOMEM;
	}	
	
	return ret;
	
}

static int rtw_del_sta(struct net_device *dev, struct ieee_param *param)
{
	_irqL irqL;
	int ret=0;	
	struct sta_info *psta = NULL;
	_adapter *padapter = (_adapter *)netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	printk("rtw_del_sta=" MACSTR "\n", MAC2STR(param->sta_addr));
		
	if(check_fwstate(pmlmepriv, (_FW_LINKED|WIFI_AP_STATE)) != _TRUE)		
	{
		return -EINVAL;		
	}

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff) 
	{
		return -EINVAL;	
	}

	psta = get_stainfo(pstapriv, param->sta_addr);
	if(psta)
	{
		//printk("free psta=%p, aid=%d\n", psta, psta->aid);
		
#if 0		
		//tear down Rx AMPDU
		send_delba(padapter, 0, psta->hwaddr);// recipient
	
		//tear down TX AMPDU
		send_delba(padapter, 1, psta->hwaddr);// // originator
		psta->htpriv.agg_enable_bitmap = 0x0;//reset
		psta->htpriv.candidate_tid_bitmap = 0x0;//reset
		
		issue_deauth(padapter, psta->hwaddr, WLAN_REASON_DEAUTH_LEAVING);
		
		_enter_critical_bh(&(pstapriv->sta_hash_lock), &irqL);		
		free_stainfo(padapter,  psta);		
		_exit_critical_bh(&(pstapriv->sta_hash_lock), &irqL);

		pstapriv->sta_dz_bitmap &=~BIT(psta->aid);
		pstapriv->tim_bitmap &=~BIT(psta->aid);		
#endif

		ap_free_sta(padapter, psta);

		psta = NULL;
		
	}
	else
	{
		printk("rtw_del_sta(), sta has already been removed or never been added\n");
		
		//ret = -1;
	}
	
	
	return ret;
	
}

static int rtw_get_sta_wpaie(struct net_device *dev, struct ieee_param *param)
{
	int ret=0;	
	struct sta_info *psta = NULL;
	_adapter *padapter = (_adapter *)netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	printk("rtw_get_sta_wpaie, sta_addr: " MACSTR "\n", MAC2STR(param->sta_addr));

	if(check_fwstate(pmlmepriv, (_FW_LINKED|WIFI_AP_STATE)) != _TRUE)		
	{
		return -EINVAL;		
	}

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff) 
	{
		return -EINVAL;	
	}

	psta = get_stainfo(pstapriv, param->sta_addr);
	if(psta)
	{
		if((psta->wpa_ie[0] == WLAN_EID_RSN) || (psta->wpa_ie[0] == WLAN_EID_GENERIC))
		{
			int wpa_ie_len;
			int copy_len;

			wpa_ie_len = psta->wpa_ie[1];
			
			copy_len = ((wpa_ie_len+2) > sizeof(psta->wpa_ie)) ? (sizeof(psta->wpa_ie)):(wpa_ie_len+2);
				
			param->u.wpa_ie.len = copy_len;

			_memcpy(param->u.wpa_ie.reserved, psta->wpa_ie, copy_len);
		}
		else
		{
			//ret = -1;
			printk("sta's wpa_ie is NONE\n");
		}		
	}
	else
	{
		ret = -1;
	}

	return ret;

}

static int rtw_set_wps_beacon(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	unsigned char wps_oui[4]={0x0,0x50,0xf2,0x04};
	_adapter *padapter = (_adapter *)netdev_priv(dev);	
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	int ie_len;

	printk("%s, len=%d\n", __FUNCTION__, len);

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) != _TRUE)
		return -EINVAL;

	ie_len = len-12-2;// 12 = param header, 2:no packed


	if(pmlmepriv->wps_beacon_ie)
	{
		rtw_mfree(pmlmepriv->wps_beacon_ie, pmlmepriv->wps_beacon_ie_len);
		pmlmepriv->wps_beacon_ie = NULL;			
	}	

	if(ie_len>0)
	{
		pmlmepriv->wps_beacon_ie = rtw_malloc(ie_len);
		pmlmepriv->wps_beacon_ie_len = ie_len;
		if ( pmlmepriv->wps_beacon_ie == NULL) {
			printk("%s()-%d: rtw_malloc() ERROR!\n", __FUNCTION__, __LINE__);
			return -EINVAL;
		}

		_memcpy(pmlmepriv->wps_beacon_ie, param->u.bcn_ie.buf, ie_len);

		update_beacon(padapter, _VENDOR_SPECIFIC_IE_, wps_oui, _TRUE);
	}
	
	
	return ret;		

}

static int rtw_set_wps_probe_resp(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	_adapter *padapter = (_adapter *)netdev_priv(dev);	
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	int ie_len;

	printk("%s, len=%d\n", __FUNCTION__, len);

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) != _TRUE)
		return -EINVAL;

	ie_len = len-12-2;// 12 = param header, 2:no packed


	if(pmlmepriv->wps_probe_resp_ie)
	{
		rtw_mfree(pmlmepriv->wps_probe_resp_ie, pmlmepriv->wps_probe_resp_ie_len);
		pmlmepriv->wps_probe_resp_ie = NULL;			
	}	

	if(ie_len>0)
	{
		pmlmepriv->wps_probe_resp_ie = rtw_malloc(ie_len);
		pmlmepriv->wps_probe_resp_ie_len = ie_len;
		if ( pmlmepriv->wps_probe_resp_ie == NULL) {
			printk("%s()-%d: rtw_malloc() ERROR!\n", __FUNCTION__, __LINE__);
			return -EINVAL;
		}
		_memcpy(pmlmepriv->wps_probe_resp_ie, param->u.bcn_ie.buf, ie_len);		
	}
	
	
	return ret;

}

static int rtw_hostapd_ioctl(struct net_device *dev, struct iw_point *p)
{
	struct ieee_param *param;
	int ret=0;

	//printk("%s\n", __FUNCTION__);

	//if (p->length < sizeof(struct ieee_param) || !p->pointer){
	if(!p->pointer){
		ret = -EINVAL;
		goto out;
	}
	
	param = (struct ieee_param *)rtw_malloc(p->length);
	if (param == NULL)
	{
		ret = -ENOMEM;
		goto out;
	}
	
	if (copy_from_user(param, p->pointer, p->length))
	{
		rtw_mfree((u8*)param, p->length);
		ret = -EFAULT;
		goto out;
	}

	//printk("%s, cmd=%d\n", __FUNCTION__, param->cmd);

	switch (param->cmd) 
	{	
		case RTL871X_HOSTAPD_FLUSH:

			ret = rtw_hostapd_sta_flush(dev);

			break;
	
		case RTL871X_HOSTAPD_ADD_STA:	
			
			ret = rtw_add_sta(dev, param);					
			
			break;

		case RTL871X_HOSTAPD_REMOVE_STA:

			ret = rtw_del_sta(dev, param);

			break;
	
		case RTL871X_HOSTAPD_SET_BEACON:

			ret = rtw_set_beacon(dev, param, p->length);

			break;
			
		case RTL871X_SET_ENCRYPTION:

			ret = rtw_set_encryption(dev, param, p->length);
			
			break;
			
		case RTL871X_HOSTAPD_GET_WPAIE_STA:

			ret = rtw_get_sta_wpaie(dev, param);
	
			break;
			
		case RTL871X_HOSTAPD_SET_WPS_BEACON:

			ret = rtw_set_wps_beacon(dev, param, p->length);

			break;

		case RTL871X_HOSTAPD_SET_WPS_PROBE_RESP:

			ret = rtw_set_wps_probe_resp(dev, param, p->length);
			
	 		break;
			
		default:
			printk("Unknown hostapd request: %d\n", param->cmd);
			ret = -EOPNOTSUPP;
			break;
		
	}

	if (ret == 0 && copy_to_user(p->pointer, param, p->length))
		ret = -EFAULT;


	rtw_mfree((u8 *)param, p->length);
	
out:
		
	return ret;
	
}
#endif

static int rtw_wx_set_priv(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *awrq,
				char *extra)
{

#ifdef CONFIG_DEBUG_RTW_WX_SET_PRIV
	char *ext_dbg;
#endif

	int ret = 0;
#ifdef CONFIG_ANDROID
	int len = 0;
	char *ext;

	_adapter *padapter = netdev_priv(dev);
	struct iw_point *dwrq = (struct iw_point*)awrq;

	//RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_notice_, ("+rtw_wx_set_priv\n"));

	len = dwrq->length;
	if (!(ext = rtw_malloc(len)))
		return -ENOMEM;

	if (copy_from_user(ext, dwrq->pointer, len)) {
		rtw_mfree(ext, len);
		return -EFAULT;
	}

	//RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_notice_,
	//	 ("rtw_wx_set_priv: %s req=%s\n",
	//	  dev->name, ext));

	#ifdef CONFIG_DEBUG_RTW_WX_SET_PRIV	
	if (!(ext_dbg = rtw_malloc(len)))
	{
		rtw_mfree(ext, len);
		return -ENOMEM;
	}	
	
	_memcpy(ext_dbg, ext, len);
	#endif

	//DBG_871X("rtw_wx_set_priv: %s req=%s\n", dev->name, ext);

	if(0 == strcasecmp(ext,"START")){
		//Turn on Wi-Fi hardware
		//OK if successful
		ret=-1;
		//sprintf(ext, "OK");
		goto FREE_EXT;
		
	}else if(0 == strcasecmp(ext,"STOP")){
		//Turn off Wi-Fi hardwoare
		//OK if successful
		ret=-1;
		//sprintf(ext, "OK");
		goto FREE_EXT;
		
	}else if(0 == strcasecmp(ext,"RSSI")){
		//Return received signal strength indicator in -db for current AP
		//<ssid> Rssi xx 
		struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);	
		struct	wlan_network	*pcur_network = &pmlmepriv->cur_network;

		if(check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE) {
			sprintf(ext, "%s rssi %d",
				pcur_network->network.Ssid.Ssid,
				padapter->recvpriv.rssi);
				
		} else {
			sprintf(ext, "OK");
		}
		
	}else if(0 == strcasecmp(ext,"LINKSPEED")){
		//Return link speed in MBPS
		//LinkSpeed xx 
		union iwreq_data wrqd;
		int ret_inner;
		int mbps;
		
		if( 0!=(ret_inner=rtw_wx_get_rate(dev, info, &wrqd, extra)) ){
			//DBG_8192C("rtw_wx_set_priv: (SIOCSIWPRIV) %s req=%s rtw_wx_get_rate return %d\n", 
			//dev->name, ext, ret);
			//goto FREE_EXT;
			mbps=0;
		} else {
			mbps=wrqd.bitrate.value / 1000000;
		}
		
		sprintf(ext, "LINKSPEED %d", mbps);
		
		
	}else if(0 == strcasecmp(ext,"MACADDR")){
		//Return mac address of the station
		//Macaddr = xx.xx.xx.xx.xx.xx 
		sprintf(ext,
			"MACADDR = %02x.%02x.%02x.%02x.%02x.%02x",
			*(dev->dev_addr),*(dev->dev_addr+1),*(dev->dev_addr+2),
			*(dev->dev_addr+3),*(dev->dev_addr+4),*(dev->dev_addr+5));

	}else if(0 == strcasecmp(ext,"SCAN-ACTIVE")){
		//Set scan type to active
		//OK if successful
		struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
		pmlmepriv->scan_mode=SCAN_ACTIVE;
		sprintf(ext, "OK");
		
	}else if(0 == strcasecmp(ext,"SCAN-PASSIVE")){
		//Set scan type to passive
		//OK if successfu
		struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
		pmlmepriv->scan_mode=SCAN_PASSIVE;
		sprintf(ext, "OK");
		
	}else{
		#ifdef  CONFIG_DEBUG_RTW_WX_SET_PRIV
		DBG_8192C("rtw_wx_set_priv: %s unknowned req=%s\n", 
		dev->name, ext_dbg);
		#endif
		goto FREE_EXT;
		
	}

	if (copy_to_user(dwrq->pointer, ext, min(dwrq->length, (u16)(strlen(ext)+1)) ) )
		ret = -EFAULT;

	//#if 1
	//DBG_8192C("rtw_wx_set_priv: %s rep=%s :strlen(ext):%d\n", 
	//	dev->name, ext ,strlen(ext));
	//#endif
	#ifdef CONFIG_DEBUG_RTW_WX_SET_PRIV
	DBG_8192C("rtw_wx_set_priv: %s req=%s rep=%s\n", 
	dev->name, ext_dbg ,ext);
	#endif

FREE_EXT:

	rtw_mfree(ext, len);
	#ifdef CONFIG_DEBUG_RTW_WX_SET_PRIV
	rtw_mfree(ext_dbg, len);
	#endif

	//DBG_8192C("rtw_wx_set_priv: (SIOCSIWPRIV) %s ret=%d\n", 
	//		dev->name, ret);
#endif
	return ret;
	
}

#if defined(CONFIG_MP_INCLUDED) && defined(MP_IWPRIV_SUPPORT)

/*
 * Input Format: %s,%d,%d
 *	%s is width, could be
 *		"b" for 1 byte
 *		"w" for WORD (2 bytes)
 *		"dw" for DWORD (4 bytes)
 *	1st %d is address(offset)
 *	2st %d is data to write
 */
static int rtw_mp_write_reg(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	char *pch, *pnext, *ptmp;
	char *width_str;
	char width;
	u32 addr, data;
	int ret;
	PADAPTER padapter = netdev_priv(dev);


	pch = extra;
	pnext = strpbrk(pch, " ,.-");
	if (pnext == NULL) return -EINVAL;
	*pnext = 0;
	width_str = pch;

	pch = pnext + 1;
	pnext = strpbrk(pch, " ,.-");
	if (pnext == NULL) return -EINVAL;
	*pnext = 0;
	addr = simple_strtoul(pch, &ptmp, 16);
	if (addr > 0x3FFF) return -EINVAL;

	pch = pnext + 1;
	if ((pch - extra) >= wrqu->data.length) return -EINVAL;
	data = simple_strtoul(pch, &ptmp, 16);

	ret = 0;
	width = width_str[0];
	switch (width) {
		case 'b':
			// 1 byte
			if (data > 0xFF) {
				ret = -EINVAL;
				break;
			}
			write8(padapter, addr, data);
			break;
		case 'w':
			// 2 bytes
			if (data > 0xFFFF) {
				ret = -EINVAL;
				break;
			}
			write16(padapter, addr, data);
			break;
		case 'd':
			// 4 bytes
			write32(padapter, addr, data);
			break;
		default:
			ret = -EINVAL;
			break;
	}

	return ret;
}

/*
 * Input Format: %s,%d
 *	%s is width, could be
 *		"b" for 1 byte
 *		"w" for WORD (2 bytes)
 *		"dw" for DWORD (4 bytes)
 *	%d is address(offset)
 *
 * Return:
 *	%d for data readed
 */
static int rtw_mp_read_reg(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	char input[128];
	char *pch, *pnext, *ptmp;
	char *width_str;
	char width;
	u32 addr;
	u32 *data = (u32*)extra;
	int ret;
	PADAPTER padapter = netdev_priv(dev);


	if (wrqu->data.length > 128) return -EFAULT;
	if (copy_from_user(input, wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;

	pch = input;
	pnext = strpbrk(pch, " ,.-");
	if (pnext == NULL) return -EINVAL;
	*pnext = 0;
	width_str = pch;

	pch = pnext + 1;
	if ((pch - input) >= wrqu->data.length) return -EINVAL;
	addr = simple_strtoul(pch, &ptmp, 16);
	if (addr > 0x3FFF) return -EINVAL;

	ret = 0;
	width = width_str[0];
	switch (width) {
		case 'b':
			// 1 byte
			*(u8*)data = read8(padapter, addr);
			wrqu->data.length = 1;
			break;
		case 'w':
			// 2 bytes
			*(u16*)data = read16(padapter, addr);
			wrqu->data.length = 2;
			break;
		case 'd':
			// 4 bytes
			*data = read32(padapter, addr);
			wrqu->data.length = 4;
			break;
		default:
			wrqu->data.length = 0;
			ret = -EINVAL;
			break;
	}

	return ret;
}

/*
 * Input Format: %d,%x,%x
 *	%d is RF path, should be smaller than MAX_RF_PATH_NUMS
 *	1st %x is address(offset)
 *	2st %x is data to write
 */
static int rtw_mp_write_rf(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u32 path, addr, data;
	int ret;
	PADAPTER padapter = netdev_priv(dev);


	ret = sscanf(extra, "%d,%x,%x", &path, &addr, &data);
	if (ret < 3) return -EINVAL;

	if (path >= MAX_RF_PATH_NUMS) return -EINVAL;
	if (addr > 0xFF) return -EINVAL;
	if (data > 0xFFFFF) return -EINVAL;

	write_rfreg(padapter, path, addr, data);

	return 0;
}

/*
 * Input Format: %d,%x
 *	%d is RF path, should be smaller than MAX_RF_PATH_NUMS
 *	%x is address(offset)
 *
 * Return:
 *	%d for data readed
 */
static int rtw_mp_read_rf(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	char input[128];
	u32 path, addr;
	u32 *data = (u32*)extra;
	int ret;
	PADAPTER padapter = netdev_priv(dev);


	if (wrqu->data.length > 128) return -EFAULT;
	if (copy_from_user(input, wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;

	ret = sscanf(input, "%d,%x", &path, &addr);
	if (ret < 2) return -EINVAL;

	if (path >= MAX_RF_PATH_NUMS) return -EINVAL;
	if (addr > 0xFF) return -EINVAL;

	*data = read_rfreg(padapter, path, addr);
	wrqu->data.length = 4;

	return 0;
}

static int rtw_mp_start(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u8 val8;
	PADAPTER padapter = netdev_priv(dev);


	if (padapter->registrypriv.mp_mode == 0)
		return -EPERM;

	if (padapter->mppriv.mode == MP_OFF) {
		if (mp_start_test(padapter) == _FAIL)
			return -EPERM;
		padapter->mppriv.mode = MP_ON;
	}

	return 0;
}

static int rtw_mp_stop(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	PADAPTER padapter = netdev_priv(dev);


	if (padapter->mppriv.mode != MP_OFF) {
		mp_stop_test(padapter);
		padapter->mppriv.mode = MP_OFF;
	}

	return 0;
}

extern int wifirate2_ratetbl_inx(unsigned char rate);

static int rtw_mp_rate(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u32 rate = MPT_RATE_1M;
	PADAPTER padapter = netdev_priv(dev);

	rate = *(u32*)extra;
		
	if(rate <= 0x7f)
		rate = wifirate2_ratetbl_inx( (u8)rate);	
	else 
		rate =(rate-0x80+MPT_RATE_MCS0);

	//printk("%s: rate=%d\n", __func__, rate);
	
	if (rate >= MPT_RATE_LAST )	
	return -EINVAL;

	padapter->mppriv.rateidx = rate;
	SetDataRate(padapter);

	return 0;
}

static int rtw_mp_channel(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u32 channel = 1;
	PADAPTER padapter = netdev_priv(dev);

	channel = *(u32*)extra;
	//printk("%s: channel=%d\n", __func__, channel);
	
	if (channel > 14)
		return -EINVAL;

	padapter->mppriv.channel = channel;
	SetChannel(padapter);

	return 0;
}

static int rtw_mp_bandwidth(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u32 bandwidth=0, sg=0;
	u8 buffer[40];
	PADAPTER padapter = netdev_priv(dev);
	if (copy_from_user(buffer, (void*)wrqu->data.pointer, wrqu->data.length))
                return -EFAULT;
	//printk("%s:iwpriv in=%s\n", __func__, extra);
	
	sscanf(buffer, "40M=%d, shortGI=%d", &bandwidth, &sg);
	
	if (bandwidth != HT_CHANNEL_WIDTH_40)
		bandwidth = HT_CHANNEL_WIDTH_20;

	//printk("%s: bw=%d sg=%d \n", __func__, bandwidth , sg);
	padapter->mppriv.bandwidth = (u8)bandwidth;
	padapter->mppriv.preamble = sg;
	
	SetBandwidth(padapter);

	return 0;
}

static int rtw_mp_txpower(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u8 buffer[40];
	u32 idx_a,idx_b;


	PADAPTER padapter = netdev_priv(dev);
	if (copy_from_user(buffer, (void*)wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;

	sscanf(buffer,"patha=%d,pathb=%d",&idx_a,&idx_b);
	//printk("%s: tx_pwr_idx_a=%x b=%x\n", __func__, idx_a, idx_b);

	padapter->mppriv.txpoweridx = (u8)idx_a;
	padapter->mppriv.txpoweridx_b = (u8)idx_b;
	
	SetAntennaPathPower(padapter);
	
	return 0;
}

static int rtw_mp_ant_tx(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u8 i;
	u16 antenna = 0;
	PADAPTER padapter = netdev_priv(dev);

	//printk("%s: extra=%s\n", __func__, extra);
	for (i=0; i < (wrqu->data.length-1); i++){
		switch(extra[i])
			{
				case 'a' :
								antenna|=ANTENNA_A;
								break;
				case 'b':
								antenna|=ANTENNA_B;
								break;
			}
	}
	//antenna |= BIT(extra[i]-'a');

	//printk("%s: antenna=0x%x\n", __func__, antenna);		
	padapter->mppriv.antenna_tx = antenna;
	//printk("%s:mppriv.antenna_rx=%d\n", __func__, padapter->mppriv.antenna_tx);
	
	SetAntenna(padapter);
	return 0;
}

static int rtw_mp_ant_rx(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u8 i;
	u16 antenna = 0;
	u8 buffer[16];
	PADAPTER padapter = netdev_priv(dev);

	if (copy_from_user(buffer, (void*)wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;
	//printk("%s: extra=%s\n", __func__, buffer);

	for (i=0; i < (wrqu->data.length-1); i++) {
		switch(extra[i])
			{
				case 'a' :
								antenna|=ANTENNA_A;
								break;
				case 'b':
								antenna|=ANTENNA_B;
								break;
			}
	}
	
	//printk("%s: antenna=0x%x\n", __func__, antenna);		
	padapter->mppriv.antenna_rx = antenna;
	//printk("%s:mppriv.antenna_rx=%d\n", __func__, padapter->mppriv.antenna_rx);

	SetAntenna(padapter);
	return 0;
}

static int rtw_mp_ctx(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u32 pkTx = 1, countPkTx = 1, cotuTx = 1, CarrSprTx = 1, scTx = 1, sgleTx = 1, stop = 1;
	u32 bStartTest = 1;
	u32 count = 0;
	u8 buffer[40];
	struct mp_priv *pmp_priv;
	struct pkt_attrib *pattrib;

	PADAPTER padapter = netdev_priv(dev);


	pmp_priv = &padapter->mppriv;

	if (copy_from_user(buffer, wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;
	printk("%s: in=%s\n", __func__, buffer);

	countPkTx = strncmp(buffer, "count=", 5); // strncmp TRUE is 0
	cotuTx = strncmp(buffer, "background", 20);
	CarrSprTx = strncmp(buffer, "background,cs", 20);
	scTx = strncmp(buffer, "background,sc", 20);
	sgleTx = strncmp(buffer, "background,stone", 20);
	pkTx = strncmp(buffer, "background,pkt", 20);
	stop = strncmp(buffer, "stop", 5);
	sscanf(buffer, "count=%d,pkt", &count);
	//printk("%s: count=%d countPkTx=%d cotuTx=%d CarrSprTx=%d scTx=%d sgleTx=%d pkTx=%d stop=%d\n", __func__, count, countPkTx, cotuTx, CarrSprTx, pkTx, sgleTx, scTx, stop);

	if (stop == 0) {
		bStartTest = 0; // To set Stop
	} else {
		bStartTest = 1;
		if (pmp_priv->mode != MP_ON) {
			if (pmp_priv->tx.stop != 1) {
				printk("%s: MP_MODE != ON %d\n", __func__, pmp_priv->mode);
				return  -EFAULT;
			}
		}
	}

	if (pkTx == 0 || countPkTx == 0)
		pmp_priv->mode = MP_PACKET_TX;
	if (sgleTx == 0)
		pmp_priv->mode = MP_SINGLE_TONE_TX;
	if (cotuTx == 0)
		pmp_priv->mode = MP_CONTINUOUS_TX;
	if (CarrSprTx == 0)
		pmp_priv->mode = MP_CARRIER_SUPPRISSION_TX;
	if (scTx == 0)
		pmp_priv->mode = MP_SINGLE_CARRIER_TX;

	switch (pmp_priv->mode)
	{
		case MP_PACKET_TX:
			//printk("%s:pkTx %d\n", __func__,bStartTest);
			if (bStartTest == 0) {
				pmp_priv->tx.stop = 1;
				pmp_priv->mode = MP_ON;
			} else if (pmp_priv->tx.stop == 1) {
				//printk("%s:countPkTx %d\n", __func__,count);
				pmp_priv->tx.stop = 0;
				pmp_priv->tx.count = count;
				pmp_priv->tx.payload = 2;
				pattrib = &pmp_priv->tx.xmitframe.attrib;
				pattrib->pktlen = 1000;
				_memset(pattrib->dst, 0xFF, ETH_ALEN);
				SetPacketTx(padapter);
			} else {
				//printk("%s: pkTx not stop\n", __func__);
				return -EFAULT;
			}
			return 0;

		case MP_SINGLE_TONE_TX:
			//printk("%s: sgleTx %d \n", __func__, bStartTest);
			SetSingleToneTx(padapter, (u8)bStartTest);
			break;

		case MP_CONTINUOUS_TX:
			//printk("%s: cotuTx %d\n", __func__, bStartTest);
			SetContinuousTx(padapter, (u8)bStartTest);
			break;

		case MP_CARRIER_SUPPRISSION_TX:
			//printk("%s: CarrSprTx %d\n", __func__, bStartTest);
			SetCarrierSuppressionTx(padapter, (u8)bStartTest);
			break;

		case MP_SINGLE_CARRIER_TX:
			//printk("%s: scTx %d\n", __func__, bStartTest);
			SetSingleCarrierTx(padapter, (u8)bStartTest);
			break;

		default:
			//printk("%s:No Match MP_MODE\n", __func__);
			return -EFAULT;
	}

	if (bStartTest) {
		struct mp_priv *pmp_priv = &padapter->mppriv;
		if (pmp_priv->tx.stop == 0) {
			pmp_priv->tx.stop = 1;
			//printk("%s: pkt tx is running...\n", __func__);
			msleep_os(5);
		}
		pmp_priv->tx.stop = 0;
		pmp_priv->tx.count = 1;
		SetPacketTx(padapter);
	} else {
		pmp_priv->mode = MP_ON;
	}

	return 0;
}

static int rtw_mp_arx(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	PADAPTER padapter = netdev_priv(dev);

	ResetPhyRxPktCount(padapter);

	return 0;
}

static int rtw_mp_trx_query(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	
	u32 txok,txfail,rxok,rxfail;
	PADAPTER padapter = netdev_priv(dev);
	if (copy_from_user(extra, wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;
	txok=padapter->mppriv.tx.sended;
	txfail=0;
	rxok = GetPhyRxPktReceived(padapter);
	rxfail = GetPhyRxPktCRC32Error(padapter);
	
	_memset(extra, '\0', 128);

	sprintf(extra, "Tx OK:%d, Tx Fail:%d, Rx OK:%d, CRC error:%d ", txok, txfail,rxok,rxfail);

	wrqu->data.length=strlen(extra)+1;

	return 0;
}

static int rtw_mp_pwrtrk(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u8 enable;
	u32 thermal;
	s32 ret;
	PADAPTER padapter = netdev_priv(dev);


	enable = 1;
	if (wrqu->data.length > 1) { // not empty string
		if (strncmp(extra, "stop", 4) == 0)
			enable = 0;
		else {
			if (sscanf(extra, "ther=%d", &thermal)) {
				ret = SetThermalMeter(padapter, (u8)thermal);
				if (ret == _FAIL) return -EPERM;
			} else
				return -EINVAL;
		}
	}

	ret = SetPowerTracking(padapter, enable);
	if (ret == _FAIL) return -EPERM;

	return 0;
}

static int rtw_mp_psd(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	PADAPTER padapter = netdev_priv(dev);


	if (copy_from_user(extra, wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;
	
	wrqu->data.length = mp_query_psd(padapter, extra);

	return 0;
}

static int rtw_mp_thermal(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u8 val;
	PADAPTER padapter = netdev_priv(dev);


	GetThermalMeter(padapter, &val);
	*(u8*)extra = val;
	wrqu->data.length = 1;

	return 0;
}

#endif

static int rtw_mp_efuse_get(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	PADAPTER padapter = netdev_priv(dev);
	struct mp_priv *pmp_priv;	
	
	int i,j;
	u8 data[EFUSE_MAP_SIZE];
	u8 rawdata[EFUSE_MAX_SIZE];
	u16	mapLen=0;
	char *pch, *ptmp, *token, *tmp[3];
	u16 addr = 0, cnts = 0, max_available_size = 0,raw_cursize = 0 ,raw_maxsize = 0;
	
	_memset(data, '\0', sizeof(data));
	_memset(rawdata, '\0', sizeof(rawdata));
	
	if (copy_from_user(extra, wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;

	pch = extra;
	printk("%s: in=%s\n", __func__, extra);
	
	i=0;
	//mac 16 "00e04c871200"
	while ( (token = strsep (&pch,",") )!=NULL )
	{
		      	tmp[i] = token;		  
			i++;
	}
	
	if ( strcmp(tmp[0],"realmap") == 0 ) {
		
		printk("strcmp OK =  %s \n" ,tmp[0]);

		mapLen = EFUSE_MAP_SIZE;
		 
			 
			 if (efuse_map_read(padapter, 0, mapLen, data) == _SUCCESS){
		 			printk("\t  efuse_map_read \n"); 
			  }else {
					printk("\t  efuse_map_read : Fail \n");
					return -EFAULT;
			 } 
			  _memset(extra, '\0', sizeof(extra));
				printk("\tOFFSET\tVALUE(hex)\n");
				sprintf(extra, "%s \n", extra);
				for ( i = 0; i < EFUSE_MAP_SIZE; i += 16 )
				{
					printk("\t0x%02x\t", i);
					sprintf(extra, "%s \t0x%02x\t", extra,i);
					for (j = 0; j < 8; j++)
					{	  
						printk("%02X ", data[i+j]);
						sprintf(extra, "%s %02X", extra, data[i+j]);
					}
						printk("\t");
						sprintf(extra,"%s\t",extra);
					for (; j < 16; j++){
						printk("%02X ", data[i+j]);
						sprintf(extra, "%s %02X", extra, data[i+j]);
					}
						printk("\n");
						sprintf(extra,"%s\n",extra);	
				}
				printk("\n");
				wrqu->data.length = strlen(extra);
				
			return 0;
	}
	else if ( strcmp(tmp[0],"rmap") == 0 ) {
					// rmap addr cnts
					addr = simple_strtoul(tmp[1], &ptmp, 16);
			
					printk("addr = %x \n" ,addr);
					
					cnts=simple_strtoul(tmp[2], &ptmp,32);
					printk("cnts = %d \n" ,cnts);
					//_memset(extra, '\0', wrqu->data.length);

				EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);
				if ((addr + cnts) > max_available_size) {
						printk("(addr + cnts parameter error \n");
						return -EFAULT;
					}
				
				if (efuse_map_read(padapter, addr, cnts, data) == _FAIL) 
					printk("efuse_access error \n");   		
				 else{
						printk("efuse_access ok \n");
				}	

				_memset(extra, '\0', sizeof(extra));	 
				for ( i = 0; i < cnts; i ++) {
						printk("0x%02x", data[i]);
						sprintf(extra, "%s 0x%02X", extra, data[i]);
						printk(" ");
						sprintf(extra,"%s ",extra);
				}
				
				wrqu->data.length = strlen(extra)+1;
				
				printk("extra = %s ", extra);
				
		         return 0;	
	}
	else if ( strcmp(tmp[0],"realraw") == 0 ) {
			addr=0;
			mapLen = EFUSE_MAX_SIZE;

				if (efuse_access(padapter, _FALSE, addr, mapLen, rawdata) == _FAIL)
				{
							printk("\t  efuse_map_read : Fail \n");
							return -EFAULT;
				} else
					printk("\t  efuse_access raw ok \n"); 	
				
				_memset(extra, '\0', sizeof(extra));
				for ( i=0; i<mapLen; i++ ) {
					printk(" %02x", rawdata[i]);
					sprintf(extra, "%s %02x", extra, rawdata[i] );

					if ((i & 0xF) == 0xF){ 
							printk("\n\t");
							sprintf(extra, "%s\n\t", extra);
						}
					else if ((i & 0x7) == 0x7){ 
							printk("\t");
							sprintf(extra, "%s\t", extra);
				}
				
				}
				wrqu->data.length = strlen(extra);
				return 0;
	}
	else if ( strcmp(tmp[0],"mac") == 0 ) {
			#ifdef CONFIG_RTL8192C
				addr = 0x16;
				cnts = 6;
			#endif
			#ifdef CONFIG_RTL8192D
				addr = 0x19;
				cnts = 6;
			#endif
				EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);
				if ((addr + mapLen) > max_available_size) {
						printk("(addr + cnts parameter error \n");
						return -EFAULT;
					}
				if (efuse_map_read(padapter, addr, cnts, data) == _FAIL) 
					printk("efuse_access error \n");   		
				 else{
						printk("efuse_access ok \n");
				}	
				 _memset(extra, '\0', sizeof(extra));		 
				for ( i = 0; i < cnts; i ++) {
						printk("0x%02x", data[i]);
						sprintf(extra, "%s 0x%02X", extra, data[i+j]);
						printk(" ");
						sprintf(extra,"%s ",extra);
				}
				wrqu->data.length = strlen(extra);
				return 0;
	}
	else if ( strcmp(tmp[0],"vidpid") == 0 ) {
				#ifdef CONFIG_RTL8192C
				       addr=0x0a;
				#endif
				#ifdef CONFIG_RTL8192D
					addr = 0x0c;
				#endif
				cnts = 4;
				EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);
				if ((addr + mapLen) > max_available_size) {
						printk("(addr + cnts parameter error \n");
						return -EFAULT;
					}
				if (efuse_map_read(padapter, addr, cnts, data) == _FAIL) 
					printk("efuse_access error \n");   		
				 else{
						printk("efuse_access ok \n");
				}	
				 _memset(extra, '\0', sizeof(extra));		 
				for ( i = 0; i < cnts; i ++) {
						printk("0x%02x", data[i]);
						sprintf(extra, "%s 0x%02X", extra, data[i+j]);
						printk(" ");
						sprintf(extra,"%s ",extra);
				}
				wrqu->data.length = strlen(extra);
				return 0;
	}
	else if ( strcmp(tmp[0],"ableraw") == 0 ) {
		
			efuse_GetCurrentSize(padapter,&raw_cursize);
			raw_maxsize = efuse_GetMaxSize(padapter);
			sprintf(extra, "%s : [ available raw size] = %d",extra,raw_maxsize-raw_cursize);
			wrqu->data.length = strlen(extra);
			
			return 0;
		}
	return 0;
}

static int rtw_mp_efuse_set(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	PADAPTER padapter = netdev_priv(dev);
	
	u8 buffer[40];
	u32 i,jj,kk;
	u8 setdata[EFUSE_MAP_SIZE];
	u8 setrawdata[EFUSE_MAX_SIZE];
	char *pch, *ptmp, *token, *edata,*tmp[5];

	u16 addr = 0, max_available_size = 0;
	u32  cnts = 0;
	
	pch = extra;
	printk("%s: in=%s\n", __func__, extra);
	
	i=0;
	while ( (token = strsep (&pch,",") )!=NULL )
	{
		      	tmp[i] = token;
			i++;
	}
	// tmp[0],[1],[2]
	// wmap,addr,00e04c871200
	if ( strcmp(tmp[0],"wmap") == 0 ) {
      if ( ! strlen( tmp[2] )/2 > 1 ) return -EFAULT;             
			addr = simple_strtoul( tmp[1], &ptmp, 16 );
			addr = addr & 0xFF;
			printk("addr = %x \n" ,addr);
					
			cnts = strlen( tmp[2] )/2;	
			if ( cnts == 0) return -EFAULT;
					
			printk("cnts = %d \n" ,cnts);
			printk("target data = %s \n" ,tmp[2]);
					
			for( jj = 0, kk = 0; jj < cnts; jj++, kk += 2 )
			{
				setdata[jj] = key_2char2num( tmp[2][kk], tmp[2][kk+ 1] );
			}
	
			EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);
			
			if ((addr + cnts) > max_available_size) {
						printk("parameter error \n");
						return -EFAULT;
			}	
			if (efuse_map_write(padapter, addr, cnts, setdata) == _FAIL) {			
					printk("efuse_map_write error \n");
					return -EFAULT;
			} else
			   printk("efuse_map_write ok \n");
		
		return 0;
	}
	else if ( strcmp(tmp[0],"wraw") == 0 ) {
			 if ( ! strlen( tmp[2] )/2 > 1 ) return -EFAULT;             
			addr = simple_strtoul( tmp[1], &ptmp, 16 );
			addr = addr & 0xFF;
			printk("addr = %x \n" ,addr);
				
			cnts=strlen( tmp[2] )/2;
			if ( cnts == 0) return -EFAULT;

			printk(" cnts = %d \n" ,cnts );		
			printk("target data = %s \n" ,tmp[2] );
			
			for( jj = 0, kk = 0; jj < cnts; jj++, kk += 2 )
			{
					setrawdata[jj] = key_2char2num( tmp[2][kk], tmp[2][kk+ 1] );
			}
					
			if ( efuse_access( padapter, _TRUE, addr, cnts, setrawdata ) == _FAIL ){
					printk("\t  efuse_map_read : Fail \n");
						return -EFAULT;
			} else
			  printk("\t  efuse_access raw ok \n"); 	
			
					return 0;
		}
	else if ( strcmp(tmp[0],"mac") == 0 ) { 
			//mac,00e04c871200
			#ifdef CONFIG_RTL8192C
				addr = 0x16;
			#endif
			#ifdef CONFIG_RTL8192D
				addr = 0x19;
			#endif
				cnts = strlen( tmp[1] )/2;
				if ( cnts == 0) return -EFAULT;
				if ( cnts > 6 ){
						printk("error data for mac addr = %s \n" ,tmp[1]);
						return -EFAULT;
				}
				
				printk("target data = %s \n" ,tmp[1]);
				
				for( jj = 0, kk = 0; jj < cnts; jj++, kk += 2 )
				{
					setdata[jj] = key_2char2num(tmp[1][kk], tmp[1][kk+ 1]);
				}
				
				EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);
		
				if ((addr + cnts) > max_available_size) {
						printk("parameter error \n");
						return -EFAULT;
					}	
				if ( efuse_map_write(padapter, addr, cnts, setdata) == _FAIL ) {
					printk("efuse_map_write error \n");
					return -EFAULT;
				} else
					printk("efuse_map_write ok \n");
				
			return 0;
		}
		else if ( strcmp(tmp[0],"vidpid") == 0 ) { 
				// pidvid,da0b7881
				#ifdef CONFIG_RTL8192C
				       addr=0x0a;
				#endif
				#ifdef CONFIG_RTL8192D
					addr = 0x0c;
				#endif
				
				cnts=strlen( tmp[1] )/2;
				if ( cnts == 0) return -EFAULT;
				printk("target data = %s \n" ,tmp[1]);
				
				for( jj = 0, kk = 0; jj < cnts; jj++, kk += 2 )
				{
					setdata[jj] = key_2char2num(tmp[1][kk], tmp[1][kk+ 1]);
				}

				EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);
				
				if ((addr + cnts) > max_available_size) {
						printk("parameter error \n");
						return -EFAULT;
					}	
				
				if ( efuse_map_write(padapter, addr, cnts, setdata) == _FAIL ) {
					printk("efuse_map_write error \n");
					return -EFAULT;
				} else
					printk("efuse_map_write ok \n");
			
				return 0;
		}
		
	  return 0;
}






//based on "driver_ipw" and for hostapd
int rtw_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	//_adapter *padapter = netdev_priv(dev);
	struct iwreq *wrq = (struct iwreq *)rq;
	int ret=0;

	//down(&priv->wx_sem);

	switch (cmd)
	{
	    case RTL_IOCTL_WPA_SUPPLICANT:	
			ret = wpa_supplicant_ioctl(dev, &wrq->u.data);
			break;
#ifdef CONFIG_AP_MODE
		case RTL_IOCTL_HOSTAPD:
			ret = rtw_hostapd_ioctl(dev, &wrq->u.data);			
			break;
#endif
	    default:
			ret = -EOPNOTSUPP;
			break;
	}

	//up(&priv->wx_sem);
	
	return ret;
	
}

static iw_handler rtw_handlers[] =
{
        NULL,                     			/* SIOCSIWCOMMIT */
        rtw_wx_get_name,   	  	/* SIOCGIWNAME */
        dummy,                    			/* SIOCSIWNWID */
        dummy,                   			 /* SIOCGIWNWID */
	 rtw_wx_set_freq,			/* SIOCSIWFREQ */
        rtw_wx_get_freq,        	/* SIOCGIWFREQ */
        rtw_wx_set_mode,       	 /* SIOCSIWMODE */
        rtw_wx_get_mode,       	 /* SIOCGIWMODE */
        dummy,//rtw_wx_set_sens,       /* SIOCSIWSENS */
	 rtw_wx_get_sens,        		/* SIOCGIWSENS */
        NULL,                     			/* SIOCSIWRANGE */
        rtw_wx_get_range,	  	/* SIOCGIWRANGE */
        rtw_wx_set_priv,			/* SIOCSIWPRIV */
        NULL,                     			/* SIOCGIWPRIV */
        NULL,                     			/* SIOCSIWSTATS */
        NULL,                    			 /* SIOCGIWSTATS */
        dummy,                   			 /* SIOCSIWSPY */
        dummy,                   			 /* SIOCGIWSPY */
        NULL,                    			 /* SIOCGIWTHRSPY */
        NULL,                     			/* SIOCWIWTHRSPY */
        rtw_wx_set_wap,      	  	/* SIOCSIWAP */
        rtw_wx_get_wap,        		 /* SIOCGIWAP */
        rtw_wx_set_mlme,                  /* request MLME operation; uses struct iw_mlme */
        dummy,                     		/* SIOCGIWAPLIST -- depricated */
        rtw_wx_set_scan,        	/* SIOCSIWSCAN */
        rtw_wx_get_scan,        	/* SIOCGIWSCAN */
        rtw_wx_set_essid,       	/* SIOCSIWESSID */
        rtw_wx_get_essid,       	/* SIOCGIWESSID */
        dummy,                    			/* SIOCSIWNICKN */
        rtw_wx_get_nick,             		/* SIOCGIWNICKN */
        NULL,                     			/* -- hole -- */
        NULL,                    			 /* -- hole -- */
	 rtw_wx_set_rate,			/* SIOCSIWRATE */
        rtw_wx_get_rate,       		 /* SIOCGIWRATE */
        dummy,                    			/* SIOCSIWRTS */
        rtw_wx_get_rts,                    /* SIOCGIWRTS */
        rtw_wx_set_frag,        		/* SIOCSIWFRAG */
        rtw_wx_get_frag,       		 /* SIOCGIWFRAG */
        dummy,                   			 /* SIOCSIWTXPOW */
        dummy,                   			 /* SIOCGIWTXPOW */
        dummy,//rtw_wx_set_retry,       /* SIOCSIWRETRY */
        rtw_wx_get_retry,//          	/* SIOCGIWRETRY */
        rtw_wx_set_enc,         		/* SIOCSIWENCODE */
        rtw_wx_get_enc,         	/* SIOCGIWENCODE */
        dummy,                    			/* SIOCSIWPOWER */
        rtw_wx_get_power,            /* SIOCGIWPOWER */
        NULL,			/*---hole---*/
	 NULL, 			/*---hole---*/
	 rtw_wx_set_gen_ie, 		/* SIOCSIWGENIE */
	 NULL, 						/* SIOCGWGENIE */
	 rtw_wx_set_auth,			/* SIOCSIWAUTH */
	 NULL,						/* SIOCGIWAUTH */
	 rtw_wx_set_enc_ext, 		/* SIOCSIWENCODEEXT */
	 NULL,						/* SIOCGIWENCODEEXT */
	 rtw_wx_set_pmkid, 						/* SIOCSIWPMKSA */
	 NULL, 			 			/*---hole---*/
		
}; 

#if defined(CONFIG_MP_INCLUDED) && defined(MP_IWPRIV_SUPPORT)

static const struct iw_priv_args rtw_private_args[] =
{
	{SIOCIWFIRSTPRIV + 0x00, IW_PRIV_TYPE_CHAR | 128, 0, "write_reg"},
	{SIOCIWFIRSTPRIV + 0x01, IW_PRIV_TYPE_CHAR | 128, IW_PRIV_TYPE_BYTE | 4, "read_reg"},
	{SIOCIWFIRSTPRIV + 0x02, IW_PRIV_TYPE_CHAR | 128, 0, "write_rf" },
	{SIOCIWFIRSTPRIV + 0x03, IW_PRIV_TYPE_CHAR | 128, IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED | 4, "read_rf" },
	{SIOCIWFIRSTPRIV + 0x04, IW_PRIV_TYPE_NONE, 0, "mp_start"},
	{SIOCIWFIRSTPRIV + 0x05, IW_PRIV_TYPE_NONE, 0, "mp_stop"},
	{SIOCIWFIRSTPRIV + 0x06, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "mp_rate"},
	{SIOCIWFIRSTPRIV + 0x07, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "mp_channel"},
	{SIOCIWFIRSTPRIV + 0x08, IW_PRIV_TYPE_CHAR | 40, 0, "mp_bandwidth"},
	{SIOCIWFIRSTPRIV + 0x09, IW_PRIV_TYPE_CHAR | 40, 0, "mp_txpower"},
	{SIOCIWFIRSTPRIV + 0x0a, IW_PRIV_TYPE_CHAR | IFNAMSIZ, 0, "mp_ant_tx"},
	{SIOCIWFIRSTPRIV + 0x0b, IW_PRIV_TYPE_CHAR | IFNAMSIZ, 0, "mp_ant_rx"},
	{SIOCIWFIRSTPRIV + 0x0c, IW_PRIV_TYPE_CHAR | 128, 0, "mp_ctx"},
	{SIOCIWFIRSTPRIV + 0x0d, 0, IW_PRIV_TYPE_CHAR | 128, "mp_query"},
	{SIOCIWFIRSTPRIV + 0x0e, IW_PRIV_TYPE_CHAR | 40, 0, "mp_arx"},
	{SIOCIWFIRSTPRIV + 0x0f, IW_PRIV_TYPE_CHAR | 40, IW_PRIV_TYPE_CHAR | 0x7FF, "mp_psd"}, 
	{SIOCIWFIRSTPRIV + 0x10, IW_PRIV_TYPE_CHAR | 40, 0, "mp_pwrtrk"},
	{SIOCIWFIRSTPRIV + 0x11, 0, IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED | 1, "mp_ther"},
	{SIOCIWFIRSTPRIV + 0x12, 0, 0, "mp_ioctl"}, // mp_ioctl
	{SIOCIWFIRSTPRIV + 0x13, IW_PRIV_TYPE_CHAR | 128, IW_PRIV_TYPE_CHAR |IW_PRIV_SIZE_FIXED |0x700 ,"efuse_get"},
	{SIOCIWFIRSTPRIV + 0x14, IW_PRIV_TYPE_CHAR | 128, 0, "efuse_set"},
};

static iw_handler rtw_private_handler[] = 
{
	rtw_mp_write_reg,	// set, 0x00 = 0
	rtw_mp_read_reg,	// get, 0x01 = 1
	rtw_mp_write_rf,	// set, 0x02 = 2
	rtw_mp_read_rf,		// get, 0x03 = 3
	rtw_mp_start,
	rtw_mp_stop,
	rtw_mp_rate,
	rtw_mp_channel,
	rtw_mp_bandwidth,
	rtw_mp_txpower,
	rtw_mp_ant_tx,
	rtw_mp_ant_rx,
	rtw_mp_ctx,
	rtw_mp_trx_query,	// get, 0x0d = 13
	rtw_mp_arx,
	rtw_mp_psd,		// get, 0x0f = 15
	rtw_mp_pwrtrk,		// set, 0x10 = 16
	rtw_mp_thermal,		// get, 0x11 = 17
	rtw_mp_ioctl_hdl,
	rtw_mp_efuse_get,
	rtw_mp_efuse_set,
};

#else // not inlucde MP

static const struct iw_priv_args rtw_private_args[] = {
	{
		SIOCIWFIRSTPRIV + 0x0,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "write32"
	},
	{
		SIOCIWFIRSTPRIV + 0x1,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | IFNAMSIZ, "read32"
	},
	{
		SIOCIWFIRSTPRIV + 0x2, 0, 0, "driver_ext"
	},
	{
		SIOCIWFIRSTPRIV + 0x3, 0, 0, "" // mp_ioctl
	},
	{
		SIOCIWFIRSTPRIV + 0x4,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "apinfo"
	},
	{
		SIOCIWFIRSTPRIV + 0x5,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "setpid"
	},
	{
		SIOCIWFIRSTPRIV + 0x6,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "wps_start"
	},
//#ifdef CONFIG_PLATFORM_MT53XX	
	{
		SIOCIWFIRSTPRIV + 0x7,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "get_sensitivity"
	},
	{
		SIOCIWFIRSTPRIV + 0x8,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "wps_prob_req_ie"
	},
	{
		SIOCIWFIRSTPRIV + 0x9,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "wps_assoc_req_ie"
	},
//endif

//#ifdef RTK_DMP_PLATFORM	
	{
		SIOCIWFIRSTPRIV + 0xA,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "channel_plan"
	},
//#endif
	{
		SIOCIWFIRSTPRIV + 0xB,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "dbg"
	},	
	{
		SIOCIWFIRSTPRIV + 0xC,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3, 0, "rfw"
	},
	{
		SIOCIWFIRSTPRIV + 0xD,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
		IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | IFNAMSIZ, "rfr"
	},
#if 0
	{
		SIOCIWFIRSTPRIV + 0xE,0,0, "wowlan_ctrl"
	},
#endif
	{
		SIOCIWFIRSTPRIV + 0x10,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "p2p_enable"
	},
	{
		SIOCIWFIRSTPRIV + 0x11,
		0, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | IFNAMSIZ , "p2p_get_status"
	},
	{
		SIOCIWFIRSTPRIV + 0x12,
		IW_PRIV_TYPE_CHAR | WPS_MAX_DEVICE_NAME_LEN, 0, "p2p_setDN"
	},
	{
		SIOCIWFIRSTPRIV + 0x13,
		0, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | IFNAMSIZ , "p2p_get_peerifa"
	},
	{
		SIOCIWFIRSTPRIV + 0x15,
		IW_PRIV_TYPE_CHAR | 12, IW_PRIV_TYPE_CHAR | IFNAMSIZ , "p2p_get_wpsCM"
	},
	{
		//	15: 1 for operation, 12 for MAC address, 2 for the length of Ssid
		SIOCIWFIRSTPRIV + 0x16,
		IW_PRIV_TYPE_CHAR | ( 15 + IW_ESSID_MAX_SIZE ), 0, "p2p_profilefound"
	},
	{
		//	20: 12 for MAC address, 1 for underline,  7 for configuration method description
		SIOCIWFIRSTPRIV + 0x18,
		IW_PRIV_TYPE_CHAR | 20, 0, "p2p_prov_disc"
	},	
#if 0
	{
		//	20: 12 for MAC address, 1 for underline,  8 PINCode or "pbc" string.
		SIOCIWFIRSTPRIV + 0x1A,
		IW_PRIV_TYPE_CHAR | 21, 0, "p2p_connect"
	},
	{
		SIOCIWFIRSTPRIV + 0x1B,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "p2p_set_intent"
	},
	{
		SIOCIWFIRSTPRIV + 0x1C,
		IW_PRIV_TYPE_CHAR |WLAN_SSID_MAXLEN, 0, "p2p_set_ssid"
	},
#endif
	{SIOCIWFIRSTPRIV + 0x1A, IW_PRIV_TYPE_CHAR | 128, 0, "efuse_set"},
	{SIOCIWFIRSTPRIV + 0x1B, IW_PRIV_TYPE_CHAR | 128, IW_PRIV_TYPE_CHAR |IW_PRIV_SIZE_FIXED |0x700 ,"efuse_get"},

};

static iw_handler rtw_private_handler[] = 
{
	rtw_wx_write32,
	rtw_wx_read32,
	rtw_drvext_hdl,
	rtw_mp_ioctl_hdl,
	rtw_get_ap_info, /*for MM DTV platform*/
	rtw_set_pid,
	rtw_wps_start,
//#ifdef CONFIG_PLATFORM_MT53XX
	rtw_wx_get_sensitivity,			//0x7
	rtw_wx_set_mtk_wps_probe_ie,	//0x8
	rtw_wx_set_mtk_wps_ie,			//0x9
//#endif
//#ifdef RTK_DMP_PLATFORM
	rtw_wx_set_channel_plan, //Set Channel depend on the country code, 0xa
//#endif
	rtw_dbg_port,				//0x0b
	rtw_wx_write_rf,				//0x0c
	rtw_wx_read_rf,				//0x0d
#if 0
	rtw_wowlan_ctrl,				//0x0e
#else
	rtw_wx_priv_null,			//0x0e
#endif
	rtw_wx_priv_null,			//0x0f
	rtw_p2p_enable,					//0x10
	rtw_p2p_get_status,				//0x11
	rtw_p2p_setDN,					//0x12
	rtw_p2p_get_peer_ifaddr,		//0x13
	rtw_wx_priv_null,				//0x14
	rtw_p2p_get_wps_configmethod,	//0x15
	rtw_p2p_profilefound,			//0x16
	rtw_wx_priv_null,				//0x17	
	rtw_p2p_prov_disc,				//0x18
	rtw_wx_priv_null,				//0x19
	//rtw_p2p_connect,				//0x1A
	//rtw_p2p_set_intent,				//0x1B
	//rtw_p2p_set_go_nego_ssid,		//0x1C
	rtw_mp_efuse_set,				//0x1A
	rtw_mp_efuse_get,				//0x1B
	// 0x1C is reserved for hostapd

};

#endif // #if defined(CONFIG_MP_INCLUDED) && defined(MP_IWPRIV_SUPPORT)

#if WIRELESS_EXT >= 17	
static struct iw_statistics *rtw_get_wireless_stats(struct net_device *dev)
{
       _adapter *padapter = netdev_priv(dev);
	   struct iw_statistics *piwstats=&padapter->iwstats;
	int tmp_level = 0;
	int tmp_qual = 0;
	int tmp_noise = 0;

	if (check_fwstate(&padapter->mlmepriv, _FW_LINKED) != _TRUE)
	{
		piwstats->qual.qual = 0;
		piwstats->qual.level = 0;
		piwstats->qual.noise = 0;
		//printk("No link  level:%d, qual:%d, noise:%d\n", tmp_level, tmp_qual, tmp_noise);
	}
	else{
		tmp_level =padapter->recvpriv.signal_strength;//padapter->recvpriv.rssi; 
		tmp_qual =padapter->recvpriv.signal_strength; //padapter->recvpriv.signal_qual;
		tmp_noise =padapter->recvpriv.noise;		
		//printk("level:%d, qual:%d, noise:%d, rssi (%d)\n", tmp_level, tmp_qual, tmp_noise,padapter->recvpriv.rssi);

		piwstats->qual.level = tmp_level;
		piwstats->qual.qual = tmp_qual;
		piwstats->qual.noise = tmp_noise;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,14))
	piwstats->qual.updated = IW_QUAL_ALL_UPDATED ;//|IW_QUAL_DBM;
#else
#ifdef RTK_DMP_PLATFORM
	//IW_QUAL_DBM= 0x8, if driver use this flag, wireless extension will show value of dbm.
	//remove this flag for show percentage 0~100
	piwstats->qual.updated = 0x07;
#else
	piwstats->qual.updated = 0x0f;
#endif
#endif

	return &padapter->iwstats;
}
#endif

struct iw_handler_def rtw_handlers_def =
{
	.standard = rtw_handlers,
	.num_standard = sizeof(rtw_handlers) / sizeof(iw_handler),
	.private = rtw_private_handler,
	.private_args = (struct iw_priv_args *)rtw_private_args,
	.num_private = sizeof(rtw_private_handler) / sizeof(iw_handler),
 	.num_private_args = sizeof(rtw_private_args) / sizeof(struct iw_priv_args),
#if WIRELESS_EXT >= 17
	.get_wireless_stats = rtw_get_wireless_stats,
#endif
};

