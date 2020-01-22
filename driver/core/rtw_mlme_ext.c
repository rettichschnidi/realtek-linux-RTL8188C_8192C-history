/******************************************************************************
* rtw_mlme_ext.c                                                                                                                                 *
*                                                                                                                                          *
* Description :                                                                                                                       *
*                                                                                                                                           *
* Author :                                                                                                                       *
*                                                                                                                                         *
* History :
*
*
*                                                                                                                                       *
* Copyright 2009, Realtek Corp.                                                                                                  *
*                                                                                                                                        *
* The contents of this file is the sole property of Realtek Corp.  It can not be                                     *
* be used, copied or modified without written permission from Realtek Corp.                                         *
*                                                                                                                                          *
*******************************************************************************/
#define _RTW_MLME_EXT_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <wifi.h>
#include <rtw_mlme_ext.h>
#include <wlan_bssdef.h>
#include <mlme_osdep.h>
#include <recv_osdep.h>

struct mlme_handler mlme_sta_tbl[]={
	{WIFI_ASSOCREQ,		"OnAssocReq",	&OnAssocReq},
	{WIFI_ASSOCRSP,		"OnAssocRsp",	&OnAssocRsp},
	{WIFI_REASSOCREQ,	"OnReAssocReq",	&OnAssocReq},
	{WIFI_REASSOCRSP,	"OnReAssocRsp",	&OnAssocRsp},
	{WIFI_PROBEREQ,		"OnProbeReq",	&OnProbeReq},
	{WIFI_PROBERSP,		"OnProbeRsp",		&OnProbeRsp},

	/*----------------------------------------------------------
					below 2 are reserved
	-----------------------------------------------------------*/
	{0,					"DoReserved",		&DoReserved},
	{0,					"DoReserved",		&DoReserved},
	{WIFI_BEACON,		"OnBeacon",		&OnBeacon},
	{WIFI_ATIM,			"OnATIM",		&OnAtim},
	{WIFI_DISASSOC,		"OnDisassoc",		&OnDisassoc},
	{WIFI_AUTH,			"OnAuth",		&OnAuthClient},
	{WIFI_DEAUTH,		"OnDeAuth",		&OnDeAuth},
	{WIFI_ACTION,		"OnAction",		&OnAction},
};

#ifdef _CONFIG_NATIVEAP_MLME_
struct mlme_handler mlme_ap_tbl[]={
	{WIFI_ASSOCREQ,		"OnAssocReq",	&OnAssocReq},
	{WIFI_ASSOCRSP,		"OnAssocRsp",	&OnAssocRsp},
	{WIFI_REASSOCREQ,	"OnReAssocReq",	&OnAssocReq},
	{WIFI_REASSOCRSP,	"OnReAssocRsp",	&OnAssocRsp},
	{WIFI_PROBEREQ,		"OnProbeReq",	&OnProbeReq},
	{WIFI_PROBERSP,		"OnProbeRsp",		&OnProbeRsp},

	/*----------------------------------------------------------
					below 2 are reserved
	-----------------------------------------------------------*/
	{0,					"DoReserved",		&DoReserved},
	{0,					"DoReserved",		&DoReserved},
	{WIFI_BEACON,		"OnBeacon",		&OnBeacon},
	{WIFI_ATIM,			"OnATIM",		&OnAtim},
	{WIFI_DISASSOC,		"OnDisassoc",		&OnDisassoc},
	{WIFI_AUTH,			"OnAuth",		&OnAuth},
	{WIFI_DEAUTH,		"OnDeAuth",		&OnDeAuth},
	{WIFI_ACTION,		"OnAction",		&OnAction},
};
#endif

struct action_handler OnAction_tbl[]={
	{WLAN_CATEGORY_SPECTRUM_MGMT,	 "ACTION_SPECTRUM_MGMT", &DoReserved},
	{WLAN_CATEGORY_QOS, "ACTION_QOS", &OnAction_qos},
	{WLAN_CATEGORY_DLS, "ACTION_DLS", &OnAction_dls},
	{WLAN_CATEGORY_BACK, "ACTION_BACK", &OnAction_back},
	{WLAN_CATEGORY_PUBLIC, "ACTION_PUBLIC", &OnAction_public},
	{WLAN_CATEGORY_RADIO_MEASUREMENT, "ACTION_RADIO_MEASUREMENT", &DoReserved},
	{WLAN_CATEGORY_FT, "ACTION_FT",	&DoReserved},
	{WLAN_CATEGORY_HT,	"ACTION_HT",	&OnAction_ht},
	{WLAN_CATEGORY_SA_QUERY, "ACTION_SA_QUERY", &DoReserved},
	{WLAN_CATEGORY_WMM, "ACTION_WMM", &OnAction_wmm},	
};


/**************************************************
OUI definitions for the vendor specific IE
***************************************************/
unsigned char	WPA_OUI[] = {0x00, 0x50, 0xf2, 0x01};
unsigned char WMM_OUI[] = {0x00, 0x50, 0xf2, 0x02};
unsigned char	WPS_OUI[] = {0x00, 0x50, 0xf2, 0x04};

unsigned char	WMM_INFO_OUI[] = {0x00, 0x50, 0xf2, 0x02, 0x00, 0x01};
unsigned char	WMM_PARA_OUI[] = {0x00, 0x50, 0xf2, 0x02, 0x01, 0x01};

unsigned char WPA_TKIP_CIPHER[4] = {0x00, 0x50, 0xf2, 0x02};
unsigned char RSN_TKIP_CIPHER[4] = {0x00, 0x0f, 0xac, 0x02};

extern unsigned char REALTEK_96B_IE[];

/********************************************************
MCS rate definitions
*********************************************************/

unsigned char	MCS_rate_2R[16] = {0xff, 0xff, 0x0, 0x0, 0x01, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
unsigned char	MCS_rate_1R[16] = {0xff, 0x00, 0x0, 0x0, 0x01, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

/********************************************************
ChannelPlan definitions
*********************************************************/
static RT_CHANNEL_PLAN	DefaultChannelPlan[RT_CHANNEL_DOMAIN_MAX] = {
							{{1,2,3,4,5,6,7,8,9,10,11,36,40,44,48,52,56,60,64,149,153,157,161,165},24},	// RT_CHANNEL_DOMAIN_FCC
							{{1,2,3,4,5,6,7,8,9,10,11},11},												// RT_CHANNEL_DOMAIN_IC
							{{1,2,3,4,5,6,7,8,9,10,11,12,13,36,40,44,48,52,56,60,64},21},				// RT_CHANNEL_DOMAIN_ETSI
							{{1,2,3,4,5,6,7,8,9,10,11,12,13},13},										// RT_CHANNEL_DOMAIN_SPAIN
							{{1,2,3,4,5,6,7,8,9,10,11,12,13},13},										// RT_CHANNEL_DOMAIN_FRANCE
							{{1,2,3,4,5,6,7,8,9,10,11,12,13,14,36,40,44,48,52,56,60,64},22},			// RT_CHANNEL_DOMAIN_MKK
							{{1,2,3,4,5,6,7,8,9,10,11,12,13,14,36,40,44,48,52,56,60,64},22},			// RT_CHANNEL_DOMAIN_MKK1
							{{1,2,3,4,5,6,7,8,9,10,11,12,13},13},										// RT_CHANNEL_DOMAIN_ISRAEL
							{{1,2,3,4,5,6,7,8,9,10,11,12,13,14,36,40,44,48,52,56,60,64},22},			// RT_CHANNEL_DOMAIN_TELEC
							{{1,2,3,4,5,6,7,8,9,10,11,12,13,36,40,44,48,52,56,60,64},21},				// RT_CHANNEL_DOMAIN_MIC
							{{1,2,3,4,5,6,7,8,9,10,11,12,13,14},14},									// RT_CHANNEL_DOMAIN_GLOBAL_DOAMIN
							{{1,2,3,4,5,6,7,8,9,10,11,12,13},13},										// RT_CHANNEL_DOMAIN_WORLD_WIDE_13
							{{1,2,3,4,5,6,7,8,9,10,11,12,13,36,40,44,48,52,56,60,64},21},				// RT_CHANNEL_DOMAIN_TELEC_NETGEAR
							{{1,2,3,4,5,6,7,8,9,10,11,36,40,44,48,52,56,60,64,149,153,157,161,165},24},	// RT_CHANNEL_DOMAIN_NCC													
							};

/****************************************************************************

Following are the initialization functions for WiFi MLME

*****************************************************************************/

static int init_hw_mlme_ext(_adapter *padapter)
{
	struct	mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;

	//set_opmode_cmd(padapter, infra_client_with_mlme);//removed

	set_channel_bwmode(padapter, pmlmeext->cur_channel, pmlmeext->cur_ch_offset, pmlmeext->cur_bwmode);

	return _SUCCESS;
}

static void init_mlme_ext_priv_value(_adapter* padapter)
{
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	//unsigned char default_channel_set[NUM_CHANNELS] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0, 0};
	unsigned char	mixed_datarate[NumRates] = {_1M_RATE_, _2M_RATE_, _5M_RATE_, _11M_RATE_, _6M_RATE_,_9M_RATE_, _12M_RATE_, _18M_RATE_, _24M_RATE_, _36M_RATE_, _48M_RATE_, _54M_RATE_, 0xff};
	unsigned char	mixed_basicrate[NumRates] ={_1M_RATE_, _2M_RATE_, _5M_RATE_, _11M_RATE_, 0xff,};

	pmlmeext->event_seq = 0;
	pmlmeext->mgnt_seq = 0;//reset to zero when disconnect at client mode

	pmlmeext->cur_channel = padapter->registrypriv.channel;
	pmlmeext->cur_bwmode = HT_CHANNEL_WIDTH_20;
	pmlmeext->cur_ch_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
	pmlmeext->retry = 0;

	pmlmeext->cur_wireless_mode = padapter->registrypriv.wireless_mode;

	//_memcpy(pmlmeext->channel_set, DefaultChannelPlan[padapter->mlmepriv.ChannelPlan].Channel, DefaultChannelPlan[padapter->mlmepriv.ChannelPlan].Len);
	//_memcpy(pmlmeext->channel_set, default_channel_set, NUM_CHANNELS);
	_memcpy(pmlmeext->datarate, mixed_datarate, NumRates);
	_memcpy(pmlmeext->basicrate, mixed_basicrate, NumRates);

	pmlmeext->sitesurvey_res.state = SCAN_DISABLE;
	pmlmeext->sitesurvey_res.channel_idx = 0;
	pmlmeext->sitesurvey_res.bss_cnt = 0;

	pmlmeinfo->state = _FALSE;
	pmlmeinfo->reauth_count = 0;
	pmlmeinfo->reassoc_count = 0;
	pmlmeinfo->link_count = 0;
	pmlmeinfo->auth_seq = 0;
	pmlmeinfo->auth_algo = dot11AuthAlgrthm_Open;
	pmlmeinfo->key_index = 0;
	pmlmeinfo->iv = 0;

	pmlmeinfo->enc_algo = _NO_PRIVACY_;
	pmlmeinfo->authModeToggle = 0;

	_memset(pmlmeinfo->chg_txt, 0, 128);

	pmlmeinfo->slotTime = SHORT_SLOT_TIME;
	pmlmeinfo->preamble_mode = PREAMBLE_AUTO;

	pmlmeinfo->dialogToken = 0;
}

static u8 init_channel_set(_adapter* padapter, u8 ChannelPlan, RT_CHANNEL_INFO *channel_set)
{
	u8	index,chanset_size = 0;
	u8	b5GBand = _FALSE, b2_4GBand = _FALSE;

	_memset(channel_set, 0, sizeof(RT_CHANNEL_INFO)*NUM_CHANNELS);

	if(ChannelPlan >= RT_CHANNEL_DOMAIN_MAX)
	{
		DBG_8192C("channel plan id error \n");
		return chanset_size;
	}

	if(padapter->registrypriv.wireless_mode & WIRELESS_11G)
	{
		b2_4GBand = _TRUE;
	}

	if(padapter->registrypriv.wireless_mode & WIRELESS_11A)
	{
		b5GBand = _TRUE;
	}

	for(index=0;index<DefaultChannelPlan[ChannelPlan].Len;index++)
	{
		if((DefaultChannelPlan[ChannelPlan].Channel[index]  <= 14) && (b2_4GBand))
		{
			channel_set[chanset_size].ChannelNum = DefaultChannelPlan[ChannelPlan].Channel[index];

		if(RT_CHANNEL_DOMAIN_GLOBAL_DOAMIN == ChannelPlan) //Channel 1~11 is active, and 12~14 is passive
		{
				if(channel_set[chanset_size].ChannelNum >= 1 && channel_set[chanset_size].ChannelNum <= 11)
					channel_set[chanset_size].ScanType = SCAN_ACTIVE;
				else if((channel_set[chanset_size].ChannelNum  >= 12 && channel_set[chanset_size].ChannelNum  <= 14))
					channel_set[chanset_size].ScanType  = SCAN_PASSIVE;			
		}
		else if(RT_CHANNEL_DOMAIN_WORLD_WIDE_13 == ChannelPlan)// channel 12~13, passive scan
		{
				if(channel_set[chanset_size].ChannelNum <= 11)
					channel_set[chanset_size].ScanType = SCAN_ACTIVE;
			else
					channel_set[chanset_size].ScanType = SCAN_PASSIVE;
		}
		else
		{
				channel_set[chanset_size].ScanType = SCAN_ACTIVE;
			}

			chanset_size++;
		}
		else if((DefaultChannelPlan[ChannelPlan].Channel[index]  >= 36) && (b5GBand))
		{
			channel_set[chanset_size].ChannelNum = DefaultChannelPlan[ChannelPlan].Channel[index];
			channel_set[chanset_size].ScanType = SCAN_ACTIVE;
			chanset_size++;
		}
	}
		
	return chanset_size;
}

int	init_mlme_ext_priv(_adapter* padapter)
{
	int	res = _SUCCESS;
	struct registry_priv* pregistrypriv = &padapter->registrypriv;
	struct	mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	_memset((u8 *)pmlmeext, 0, sizeof(struct mlme_ext_priv));
	pmlmeext->padapter = padapter;

	//fill_fwpriv(padapter, &(pmlmeext->fwpriv));

	init_mlme_ext_priv_value(padapter);
	pmlmeinfo->bAcceptAddbaReq = pregistrypriv->bAcceptAddbaReq;
	
	init_mlme_ext_timer(padapter);

#ifdef CONFIG_AP_MODE
	init_mlme_ap_info(padapter);	
#endif

	res = init_hw_mlme_ext(padapter);

	pmlmeext->max_chan_nums = init_channel_set(padapter, pmlmepriv->ChannelPlan,pmlmeext->channel_set);

	pmlmeext->chan_scan_time = SURVEY_TO;
	pmlmeext->mlmeext_init = _TRUE;

	return res;

}

void free_mlme_ext_priv (struct mlme_ext_priv *pmlmeext)
{
	_adapter *padapter = pmlmeext->padapter;

	if (!padapter)
		return;

	if (padapter->bDriverStopped == _TRUE)
	{
		_cancel_timer_ex(&pmlmeext->survey_timer);
		_cancel_timer_ex(&pmlmeext->link_timer);
		//_cancel_timer_ex(&pmlmeext->ADDBA_timer);
	}
	
	
}

static void UpdateBrateTbl(
	IN PADAPTER		Adapter,
	IN u8			*mBratesOS
)
{
	u8	i;
	u8	rate;

	// 1M, 2M, 5.5M, 11M, 6M, 12M, 24M are mandatory.
	for(i=0;i<NDIS_802_11_LENGTH_RATES_EX;i++)
	{
		rate = mBratesOS[i] & 0x7f;
		switch(rate)
		{
			case IEEE80211_CCK_RATE_1MB:
			case IEEE80211_CCK_RATE_2MB:
			case IEEE80211_CCK_RATE_5MB:
			case IEEE80211_CCK_RATE_11MB:
			case IEEE80211_OFDM_RATE_6MB:
			case IEEE80211_OFDM_RATE_12MB:
			case IEEE80211_OFDM_RATE_24MB:
				mBratesOS[i] |= IEEE80211_BASIC_RATE_MASK;
				break;
		}
	}

}

static void _mgt_dispatcher(_adapter *padapter, struct mlme_handler *ptable, union recv_frame *precv_frame)
{
	u8 bc_addr[ETH_ALEN] = {0xff,0xff,0xff,0xff,0xff,0xff};
	u8 *pframe = precv_frame->u.hdr.rx_data; 
	uint len = precv_frame->u.hdr.len;

	  if(ptable->func)
        {
       	 //receive the frames that ra(a1) is my address or ra(a1) is bc address.
		if (!_memcmp(GetAddr1Ptr(pframe), myid(&padapter->eeprompriv), ETH_ALEN) &&
			!_memcmp(GetAddr1Ptr(pframe), bc_addr, ETH_ALEN)) 
		{
			return;
		}
		
		ptable->func(padapter, precv_frame);
        }
	
}

void mgt_dispatcher(_adapter *padapter, union recv_frame *precv_frame)
{
	int index;
	struct mlme_handler *ptable;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	u8 bc_addr[ETH_ALEN] = {0xff,0xff,0xff,0xff,0xff,0xff};
	u8 *pframe = precv_frame->u.hdr.rx_data;
	uint len = precv_frame->u.hdr.len;

	RT_TRACE(_module_rtl871x_mlme_c_, _drv_info_,
		 ("+mgt_dispatcher: type(0x%x) subtype(0x%x)\n",
		  GetFrameType(pframe), GetFrameSubType(pframe)));

#if 0
	{
		u8 *pbuf;
		pbuf = GetAddr1Ptr(pframe);
		printk("A1-%x:%x:%x:%x:%x:%x\n", *pbuf, *(pbuf+1), *(pbuf+2), *(pbuf+3), *(pbuf+4), *(pbuf+5));
		pbuf = GetAddr2Ptr(pframe);
		printk("A2-%x:%x:%x:%x:%x:%x\n", *pbuf, *(pbuf+1), *(pbuf+2), *(pbuf+3), *(pbuf+4), *(pbuf+5));
		pbuf = GetAddr3Ptr(pframe);
		printk("A3-%x:%x:%x:%x:%x:%x\n", *pbuf, *(pbuf+1), *(pbuf+2), *(pbuf+3), *(pbuf+4), *(pbuf+5));
	}
#endif

	if (GetFrameType(pframe) != WIFI_MGT_TYPE)
	{
		RT_TRACE(_module_rtl871x_mlme_c_, _drv_err_, ("mgt_dispatcher: type(0x%x) error!\n", GetFrameType(pframe)));
		return;
	}

	//receive the frames that ra(a1) is my address or ra(a1) is bc address.
	if (!_memcmp(GetAddr1Ptr(pframe), myid(&padapter->eeprompriv), ETH_ALEN) &&
		!_memcmp(GetAddr1Ptr(pframe), bc_addr, ETH_ALEN))
	{
		return;
	}

	ptable = mlme_sta_tbl;

	index = GetFrameSubType(pframe) >> 4;

	if (index > 13)
	{
		RT_TRACE(_module_rtl871x_mlme_c_,_drv_err_,("Currently we do not support reserved sub-fr-type=%d\n", index));
		return;
	}
	ptable += index;

#if 0//gtest
	sa = get_sa(pframe);
	psta = search_assoc_sta(sa, padapter);
	// only check last cache seq number for management frame
	if (psta != NULL) {
		if (GetRetry(pframe)) {
			if (GetTupleCache(pframe) == psta->rxcache->nonqos_seq){
				RT_TRACE(_module_rtl871x_mlme_c_,_drv_err_,("drop due to decache!\n"));
				return;
			}
		}
		psta->rxcache->nonqos_seq = GetTupleCache(pframe);
	}
#else

	if(GetRetry(pframe))
	{
		//RT_TRACE(_module_rtl871x_mlme_c_,_drv_err_,("drop due to decache!\n"));
		//return;
	}
#endif

#ifdef CONFIG_AP_MODE
	switch (GetFrameSubType(pframe)) 
	{
		case WIFI_AUTH:
			if(check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE)
				ptable->func = &OnAuth;
			else
				ptable->func = &OnAuthClient;
			//pass through
		case WIFI_ASSOCREQ:
		case WIFI_REASSOCREQ:
			_mgt_dispatcher(padapter, ptable, precv_frame);	
#ifdef CONFIG_HOSTAPD_MLME				
			if(check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE)
				hostapd_mlme_rx(padapter, precv_frame);
#endif			
			break;
		case WIFI_PROBEREQ:
			if(check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE)
			{
#ifdef CONFIG_HOSTAPD_MLME		
				hostapd_mlme_rx(padapter, precv_frame);
#else
				_mgt_dispatcher(padapter, ptable, precv_frame);
#endif
			}
			else
				_mgt_dispatcher(padapter, ptable, precv_frame);
			break;
		case WIFI_BEACON:			
			_mgt_dispatcher(padapter, ptable, precv_frame);
			break;
		case WIFI_ACTION:
			//if(check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE)
			_mgt_dispatcher(padapter, ptable, precv_frame);		
			break;
		default:
			_mgt_dispatcher(padapter, ptable, precv_frame);	
			if(check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE)
				hostapd_mlme_rx(padapter, precv_frame);			
			break;
	}
#else

	_mgt_dispatcher(padapter, ptable, precv_frame);	
	
#endif

}

u32 p2p_listen_state_process(_adapter *padapter, unsigned char *da)
{
	issue_probersp_p2p( padapter, da);
	return _SUCCESS;
}


/****************************************************************************

Following are the callback functions for each subtype of the management frames

*****************************************************************************/

unsigned int OnProbeReq(_adapter *padapter, union recv_frame *precv_frame)
{
	unsigned int	ielen;
	unsigned char	*p;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);
	WLAN_BSSID_EX 	*cur = &(pmlmeinfo->network);
	u8 *pframe = precv_frame->u.hdr.rx_data;
	uint len = precv_frame->u.hdr.len;
	u8 is_valid_p2p_probereq = _FALSE;

	if ( ( pwdinfo->p2p_state != P2P_STATE_NONE ) && 
		( pwdinfo->p2p_state != P2P_STATE_IDLE ) && 
		( pwdinfo->role != P2P_ROLE_CLIENT ) &&
		( pwdinfo->p2p_state != P2P_STATE_FIND_PHASE_SEARCH ) &&
		(pwdinfo->p2p_state != P2P_STATE_SCAN )
	   )
	{		
		if((is_valid_p2p_probereq = process_probe_req_p2p_ie(pwdinfo, pframe, len)) == _TRUE)
		{
			if(pwdinfo->role == P2P_ROLE_DEVICE)
			{
				p2p_listen_state_process( padapter,  get_sa(pframe));
				
				return _SUCCESS;	
			}

			if(pwdinfo->role == P2P_ROLE_GO)
			{
				goto _continue;
			}
		}		
	}

_continue:

	if(check_fwstate(pmlmepriv, WIFI_STATION_STATE))
	{
		return _SUCCESS;
	}	

	if(check_fwstate(pmlmepriv, _FW_LINKED) == _FALSE && 
		check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE|WIFI_AP_STATE)==_FALSE)
	{
		return _SUCCESS;
	}	


	//DBG_871X("+OnProbeReq\n");
		
	p = get_ie(pframe + WLAN_HDR_A3_LEN + _PROBEREQ_IE_OFFSET_, _SSID_IE_, (int *)&ielen,
			len - WLAN_HDR_A3_LEN - _PROBEREQ_IE_OFFSET_);


	//check (wildcard) SSID 
	if (p != NULL)
	{
		if(is_valid_p2p_probereq == _TRUE)
		{
			goto _issue_probersp;
		}	

		if ((ielen != 0) && (!_memcmp((void *)(p+2), (void *)cur->Ssid.Ssid, le32_to_cpu(cur->Ssid.SsidLength))))
		{
			return _SUCCESS;
		}
		
_issue_probersp:
	
		issue_probersp(padapter, get_sa(pframe), is_valid_p2p_probereq);		
		
	}

	return _SUCCESS;
	
}

unsigned int OnProbeRsp(_adapter *padapter, union recv_frame *precv_frame)
{
	struct sta_info	*psta;	
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct sta_priv	*pstapriv = &padapter->stapriv;
	u8 *pframe = precv_frame->u.hdr.rx_data; 
	//uint len = precv_frame->u.hdr.len;

	report_survey_event(padapter, precv_frame);

	if (pmlmeext->sitesurvey_res.state == SCAN_PROCESS)
	{
		return _SUCCESS;
	}

	if (_memcmp(GetAddr3Ptr(pframe), get_my_bssid(&pmlmeinfo->network), ETH_ALEN))
	{	
		if (pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS)
		{
			if ((psta = get_stainfo(pstapriv, GetAddr2Ptr(pframe))) != NULL)
			{
				psta->sta_stats.rx_pkts++;
			}
		}
	}
	
	return _SUCCESS;
	
}

unsigned int OnBeacon(_adapter *padapter, union recv_frame *precv_frame)
{
	int cam_idx;
	struct sta_info	*psta;	
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct sta_priv	*pstapriv = &padapter->stapriv;
	u8 *pframe = precv_frame->u.hdr.rx_data;
	uint len = precv_frame->u.hdr.len;

	if (pmlmeext->sitesurvey_res.state == SCAN_PROCESS)
	{
		report_survey_event(padapter, precv_frame);	

		return _SUCCESS;
	}

	if (_memcmp(GetAddr3Ptr(pframe), get_my_bssid(&pmlmeinfo->network), ETH_ALEN))
	{
		if (pmlmeinfo->state & WIFI_FW_AUTH_NULL)
		{
			//check the vendor of the assoc AP
			pmlmeinfo->assoc_AP_vendor = check_assoc_AP(pframe+sizeof(struct ieee80211_hdr_3addr), len-sizeof(struct ieee80211_hdr_3addr));				

			//update TSF Value
			update_TSF(pmlmeext, pframe, len);

			//start auth
			start_clnt_auth(padapter);

			return _SUCCESS;
		}

		if(((pmlmeinfo->state&0x03) == WIFI_FW_STATION_STATE) && (pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS))
		{
			if ((psta = get_stainfo(pstapriv, GetAddr2Ptr(pframe))) != NULL)
			{
				//update WMM, ERP in the beacon
				//todo: the timer is used instead of the number of the beacon received
				if ((psta->sta_stats.rx_pkts & 0xf) == 0)
				{
					//DBG_871X("update_bcn_info\n");
					update_beacon_info(padapter, pframe, len, psta);
				}
				psta->sta_stats.rx_pkts++;
			}		
		}
		else if((pmlmeinfo->state&0x03) == WIFI_FW_ADHOC_STATE)
		{
			if ((psta = get_stainfo(pstapriv, GetAddr2Ptr(pframe))) != NULL)
			{
				//update WMM, ERP in the beacon
				//todo: the timer is used instead of the number of the beacon received
				if ((psta->sta_stats.rx_pkts & 0xf) == 0)
				{
					//DBG_871X("update_bcn_info\n");
					update_beacon_info(padapter, pframe, len, psta);
				}
				psta->sta_stats.rx_pkts++;
			}
			else
			{					
				//allocate a new CAM entry for IBSS station
				if ((cam_idx = allocate_cam_entry(padapter)) == NUM_STA)
				{
					goto _END_ONBEACON_;
				}

				//get supported rate
				if (update_sta_support_rate(padapter, (pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_), (len - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_), cam_idx) == _FAIL)
				{
					pmlmeinfo->FW_sta_info[cam_idx].status = 0;
					goto _END_ONBEACON_;
				}
		
				//update TSF Value
				update_TSF(pmlmeext, pframe, len);			

				//report sta add event
				report_add_sta_event(padapter, GetAddr2Ptr(pframe), cam_idx);

				//pmlmeext->linked_to = LINKED_TO;				
			}
		}
	}

_END_ONBEACON_:

	return _SUCCESS;

}

unsigned int OnAuth(_adapter *padapter, union recv_frame *precv_frame)
{
#ifdef CONFIG_AP_MODE
	unsigned int	auth_mode, seq, ie_len;	
	unsigned char	*sa, *p;	
	u16	algorithm;
	int	status;
	static struct sta_info stat;	
	struct	sta_info	*pstat=NULL;	
	struct	sta_priv *pstapriv = &padapter->stapriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	u8 *pframe = precv_frame->u.hdr.rx_data; 
	uint len = precv_frame->u.hdr.len;

	if((pmlmeinfo->state&0x03) != WIFI_FW_AP_STATE)
		return _FAIL;

	DBG_871X("+OnAuth\n");
	
	sa = GetAddr2Ptr(pframe);
	
	auth_mode = psecuritypriv->dot11AuthAlgrthm;
	seq = cpu_to_le16(*(unsigned short *)((unsigned int)pframe + WLAN_HDR_A3_LEN + 2));
	algorithm = cpu_to_le16(*(unsigned short *)((unsigned int)pframe + WLAN_HDR_A3_LEN));

	if (GetPrivacy(pframe))
	{	
#if 0 //TODO: SW wep_decrypt
		if (SWCRYPTO)
		{
			status = wep_decrypt(priv, pframe, pfrinfo->pktlen,
				priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm);
			if (status == FALSE)
			{
				SAVE_INT_AND_CLI(flags);
				RT_TRACE(_module_rtl871x_mlme_c_,_drv_err_,"wep-decrypt a Auth frame error!\n");
				status = _STATS_CHALLENGE_FAIL_;
				goto auth_fail;
			}
		}

		seq = cpu_to_le16(*(unsigned short *)((unsigned int)pframe + WLAN_HDR_A3_LEN + 4 + 2));
		algorithm = cpu_to_le16(*(unsigned short *)((unsigned int)pframe + WLAN_HDR_A3_LEN + 4));
#endif
	}


	DBG_871X("auth alg=%x, seq=%X\n", algorithm, seq);

	if (auth_mode == 2 &&
			psecuritypriv->dot11PrivacyAlgrthm != _WEP40_ &&
			psecuritypriv->dot11PrivacyAlgrthm != _WEP104_)
		auth_mode = 0;

	if ((algorithm > 0 && auth_mode == 0) ||	// rx a shared-key auth but shared not enabled
		(algorithm == 0 && auth_mode == 1) )	// rx a open-system auth but shared-key is enabled
	{		
		DBG_871X("auth rejected due to bad alg [alg=%d, auth_mib=%d] %02X%02X%02X%02X%02X%02X\n",
			algorithm, auth_mode, sa[0], sa[1], sa[2], sa[3], sa[4], sa[5]);
		
		status = _STATS_NO_SUPP_ALG_;
		
		goto auth_fail;
	}
	
#if 0 //TODO:ACL control	
	phead = &priv->wlan_acl_list;
	plist = phead->next;
	//check sa
	if (acl_mode == 1)		// 1: positive check, only those on acl_list can be connected.
		res = FAIL;
	else
		res = SUCCESS;

	while(plist != phead)
	{
		paclnode = list_entry(plist, struct wlan_acl_node, list);
		plist = plist->next;
		if (!memcmp((void *)sa, paclnode->addr, 6)) {
			if (paclnode->mode & 2) { // deny
				res = FAIL;
				break;
			}
			else {
				res = SUCCESS;
				break;
			}
		}
	}

	if (res != SUCCESS) {
		RT_TRACE(_module_rtl871x_mlme_c_,_drv_err_,"auth abort because ACL!\n");
		return FAIL;
	}
#endif

	pstat = get_stainfo(pstapriv, sa);
	if (pstat == NULL)
	{
		// allocate a new one
		DBG_871X("going to alloc stainfo for sa=%02X%02X%02X%02X%02X%02X\n",  sa[0],sa[1],sa[2],sa[3],sa[4],sa[5]);
		pstat = alloc_stainfo(pstapriv, sa);
		if (pstat == NULL)
		{
			DBG_871X(" Exceed the upper limit of supported clients...\n");
			status = _STATS_UNABLE_HANDLE_STA_;
			goto auth_fail;
		}
		
		pstat->state = WIFI_FW_AUTH_NULL;
		pstat->auth_seq = 0;

		//pstat->flags = 0;
		//pstat->capability = 0;
	}
	else
	{		
		if(is_list_empty(&pstat->asoc_list)==_FALSE)
		{			
			list_delete(&pstat->asoc_list);
			if (pstat->expire_to > 0)
			{
				//TODO: STA re_auth within expire_to
			}
		}
		if (seq==1) {
			//TODO: STA re_auth and auth timeout 
		}
	}

	if (is_list_empty(&pstat->auth_list))
	{		
		list_insert_tail(&pstat->auth_list, &pstapriv->auth_list);
	}	
		

	if (pstat->auth_seq == 0)
		pstat->expire_to = pstapriv->auth_to;

	if ((pstat->auth_seq + 1) != seq)
	{
		DBG_871X("(1)auth rejected because out of seq [rx_seq=%d, exp_seq=%d]!\n",
			seq, pstat->auth_seq+1);
		status = _STATS_OUT_OF_AUTH_SEQ_;
		goto auth_fail;
	}

	if (algorithm==0 && (auth_mode == 0 || auth_mode == 2))
	{
		if (seq == 1)
		{
			pstat->state &= ~WIFI_FW_AUTH_NULL;
			pstat->state |= WIFI_FW_AUTH_SUCCESS;
			pstat->expire_to = pstapriv->assoc_to;
			pstat->authalg = algorithm;
		}
		else
		{
			DBG_871X("(2)auth rejected because out of seq [rx_seq=%d, exp_seq=%d]!\n",
				seq, pstat->auth_seq+1);
			status = _STATS_OUT_OF_AUTH_SEQ_;
			goto auth_fail;
		}
	}
	else // shared system or auto authentication
	{
		if (seq == 1)
		{
			//prepare for the challenging txt...
			
			//get_random_bytes((void *)pstat->chg_txt, 128);//TODO:
			
			pstat->state &= ~WIFI_FW_AUTH_NULL;
			pstat->state |= WIFI_FW_AUTH_STATE;
			pstat->authalg = algorithm;
			pstat->auth_seq = 2;
		}
		else if (seq == 3)
		{
			//checking for challenging txt...
			DBG_871X("checking for challenging txt...\n");
			
			p = get_ie(pframe + WLAN_HDR_A3_LEN + 4 + _AUTH_IE_OFFSET_ , _CHLGETXT_IE_, (int *)&ie_len,
					len - WLAN_HDR_A3_LEN - _AUTH_IE_OFFSET_ - 4);

			if((p==NULL) || (ie_len<=0))
			{
				DBG_871X("auth rejected because challenge failure!(1)\n");
				status = _STATS_CHALLENGE_FAIL_;
				goto auth_fail;
			}
			
			if (_memcmp((void *)(p + 2), pstat->chg_txt, 128))
			{
				pstat->state &= (~WIFI_FW_AUTH_STATE);
				pstat->state |= WIFI_FW_AUTH_SUCCESS;
				// challenging txt is correct...
				pstat->expire_to =  pstapriv->assoc_to;
			}
			else
			{
				DBG_871X("auth rejected because challenge failure!\n");
				status = _STATS_CHALLENGE_FAIL_;
				goto auth_fail;
			}
		}
		else
		{
			DBG_871X("(3)auth rejected because out of seq [rx_seq=%d, exp_seq=%d]!\n",
				seq, pstat->auth_seq+1);
			status = _STATS_OUT_OF_AUTH_SEQ_;
			goto auth_fail;
		}
	}


	// Now, we are going to issue_auth...
	pstat->auth_seq = seq + 1;	
	
#ifdef CONFIG_NATIVEAP_MLME
	issue_auth(padapter, pstat, (unsigned short)(_STATS_SUCCESSFUL_));
#endif

	if (pstat->state & WIFI_FW_AUTH_SUCCESS)
		pstat->auth_seq = 0;

		
	return _SUCCESS;

auth_fail:

	if (pstat) 
	{
		pstat = &stat;
		_memset((char *)pstat, '\0', sizeof(stat));
		pstat->auth_seq = 2;
		_memcpy(pstat->hwaddr, sa, 6);
	}
	
#ifdef CONFIG_NATIVEAP_MLME
	issue_auth(padapter, pstat, (unsigned short)status);	
#endif

#endif
	return _FAIL;

}

unsigned int OnAuthClient(_adapter *padapter, union recv_frame *precv_frame)
{
	unsigned int	seq, len, status, algthm, offset;
	unsigned char	*p;
	unsigned int	go2asoc = 0;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	u8 *pframe = precv_frame->u.hdr.rx_data;
	uint pkt_len = precv_frame->u.hdr.len;

	DBG_871X("%s\n", __FUNCTION__);

	//check A1 matches or not
	if (!_memcmp(myid(&(padapter->eeprompriv)), get_da(pframe), ETH_ALEN))
		return _SUCCESS;

	if (!(pmlmeinfo->state & WIFI_FW_AUTH_STATE))
		return _SUCCESS;

	offset = (GetPrivacy(pframe))? 4: 0;

	algthm 	= le16_to_cpu(*(unsigned short *)((SIZE_PTR)pframe + WLAN_HDR_A3_LEN + offset));
	seq 	= le16_to_cpu(*(unsigned short *)((SIZE_PTR)pframe + WLAN_HDR_A3_LEN + offset + 2));
	status 	= le16_to_cpu(*(unsigned short *)((SIZE_PTR)pframe + WLAN_HDR_A3_LEN + offset + 4));

	if (status != 0)
	{
		DBG_871X("clnt auth fail, status: %d\n", status);
		goto authclnt_fail;
	}

	if (seq == 2)
	{
		if (pmlmeinfo->auth_algo == dot11AuthAlgrthm_Shared)
		{
			 // legendary shared system
			p = get_ie(pframe + WLAN_HDR_A3_LEN + _AUTH_IE_OFFSET_, _CHLGETXT_IE_, (int *)&len,
				pkt_len - WLAN_HDR_A3_LEN - _AUTH_IE_OFFSET_);

			if (p == NULL)
			{
				//printk("marc: no challenge text?\n");
				goto authclnt_fail;
			}

			_memcpy((void *)(pmlmeinfo->chg_txt), (void *)(p + 2), len);
			pmlmeinfo->auth_seq = 3;
			issue_auth(padapter, NULL, 0);

			return _SUCCESS;
		}
		else
		{
			// open system
			go2asoc = 1;
		}
	}
	else if (seq == 4)
	{
		if (pmlmeinfo->auth_algo == dot11AuthAlgrthm_Shared)
		{
			go2asoc = 1;
		}
		else
		{
			goto authclnt_fail;
		}
	}
	else
	{
		// this is also illegal
		//printk("marc: clnt auth failed due to illegal seq=%x\n", seq);
		goto authclnt_fail;
	}

	if (go2asoc)
	{
		start_clnt_assoc(padapter);
		return _SUCCESS;
	}

authclnt_fail:

	//pmlmeinfo->state &= ~(WIFI_FW_AUTH_STATE);

	return _FAIL;

}

unsigned int OnAssocReq(_adapter *padapter, union recv_frame *precv_frame)
{
#ifdef CONFIG_AP_MODE
	u16 capab_info, listen_interval;
	struct ieee802_11_elems elems;	
	struct sta_info	*pstat;
	unsigned char		reassoc, *p, *pos, *wpa_ie;
	unsigned char		rsnie_hdr[4]={0x00, 0x50, 0xf2, 0x01};
	unsigned char WMM_IE[] = {0x00, 0x50, 0xf2, 0x02, 0x00, 0x01};
	int		i, ie_len, wpa_ie_len, left;
	unsigned long		flags;
	unsigned char		supportRate[16];
	int					supportRateNum;
	unsigned short		status = _STATS_SUCCESSFUL_;
	unsigned short		frame_type, ie_offset=0;	
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);	
	WLAN_BSSID_EX 	*cur = &(pmlmeinfo->network);
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);
	u8 *pframe = precv_frame->u.hdr.rx_data;
	uint pkt_len = precv_frame->u.hdr.len;
	u8 p2p_status_code = P2P_STATUS_SUCCESS;
	u8 p2pie[ MAX_P2P_IE_LEN] = { 0x00 };
	u32 p2pielen = 0;


	if((pmlmeinfo->state&0x03) != WIFI_FW_AP_STATE)
		return _FAIL;
	
	frame_type = GetFrameSubType(pframe);
	if (frame_type == WIFI_ASSOCREQ)
	{
		reassoc = 0;
		ie_offset = _ASOCREQ_IE_OFFSET_;
	}	
	else // WIFI_REASSOCREQ
	{
		reassoc = 1;
		ie_offset = _REASOCREQ_IE_OFFSET_;
	}
	

	if (pkt_len < IEEE80211_3ADDR_LEN + ie_offset) {
		DBG_871X("handle_assoc(reassoc=%d) - too short payload (len=%lu)"
		       "\n", reassoc, (unsigned long)pkt_len);
		return _FAIL;
	}
	
	pstat = get_stainfo(pstapriv, GetAddr2Ptr(pframe));
	if (pstat == (struct sta_info *)NULL)
	{
		status = _RSON_CLS2_;
		goto asoc_class2_error;
	}

	capab_info = le16_to_cpu(*(unsigned short *)(pframe + WLAN_HDR_A3_LEN));
	listen_interval = le16_to_cpu(*(unsigned short *)(pframe + WLAN_HDR_A3_LEN+2));

	left = pkt_len - (IEEE80211_3ADDR_LEN + ie_offset);
	pos = pframe + (IEEE80211_3ADDR_LEN + ie_offset);
	

	DBG_871X("%s\n", __FUNCTION__);


	// check if this stat has been successfully authenticated/assocated
	if (!((pstat->state) & WIFI_FW_AUTH_SUCCESS))
	{
		status = _RSON_CLS2_;
		goto asoc_class2_error;
	}

#if 0// todo:tkip_countermeasures
	if (hapd->tkip_countermeasures) {
		resp = WLAN_REASON_MICHAEL_MIC_FAILURE;
		goto fail;
	}
#endif

	pstat->capability = capab_info;

#if 0//todo:
	//check listen_interval
	if (listen_interval > hapd->conf->max_listen_interval) {
		hostapd_logger(hapd, mgmt->sa, HOSTAPD_MODULE_IEEE80211,
			       HOSTAPD_LEVEL_DEBUG,
			       "Too large Listen Interval (%d)",
			       listen_interval);
		resp = WLAN_STATUS_ASSOC_DENIED_LISTEN_INT_TOO_LARGE;
		goto fail;
	}
	
	pstat->listen_interval = listen_interval;
#endif

	//now parse all ieee802_11 ie to point to elems
	if (rtw_ieee802_11_parse_elems(pos, left, &elems, 1) == ParseFailed ||
	    !elems.ssid) {
		DBG_871X("STA " MACSTR " sent invalid association request\n",
		       MAC2STR(pstat->hwaddr));
		status = _STATS_FAILURE_;		
		goto OnAssocReqFail;
	}


	// now we should check all the fields...
	// checking SSID
	p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, _SSID_IE_, &ie_len,
		pkt_len - WLAN_HDR_A3_LEN - ie_offset);
	if (p == NULL)
	{
		status = _STATS_FAILURE_;		
	}

	if (ie_len == 0) // broadcast ssid, however it is not allowed in assocreq
		status = _STATS_FAILURE_;
	else
	{
		// check if ssid match
		if (!_memcmp((void *)(p+2), cur->Ssid.Ssid, cur->Ssid.SsidLength))
			status = _STATS_FAILURE_;

		if (ie_len != cur->Ssid.SsidLength)
			status = _STATS_FAILURE_;
	}

	if(_STATS_SUCCESSFUL_ != status)
		goto OnAssocReqFail;

	// check if the supported rate is ok
	p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, _SUPPORTEDRATES_IE_, &ie_len, pkt_len - WLAN_HDR_A3_LEN - ie_offset);
	if (p == NULL) {
		DBG_871X("Rx a sta assoc-req which supported rate is empty!\n");
		// use our own rate set as statoin used
		//_memcpy(supportRate, AP_BSSRATE, AP_BSSRATE_LEN);
		//supportRateNum = AP_BSSRATE_LEN;
		
		status = _STATS_FAILURE_;
		goto OnAssocReqFail;
	}
	else {
		_memcpy(supportRate, p+2, ie_len);
		supportRateNum = ie_len;

		p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, _EXT_SUPPORTEDRATES_IE_ , &ie_len,
				pkt_len - WLAN_HDR_A3_LEN - ie_offset);
		if (p !=  NULL) {
			
			if(supportRateNum<=sizeof(supportRate))
			{
				_memcpy(supportRate+supportRateNum, p+2, ie_len);
				supportRateNum += ie_len;
			}			
		}
	}

	//todo: mask supportRate between AP & STA -> move to update raid
	//get_matched_rate(pmlmeext, supportRate, &supportRateNum, 0);

	//update station supportRate	
	pstat->bssratelen = supportRateNum;
	_memcpy(pstat->bssrateset, supportRate, supportRateNum);


	//check RSN/WPA/WPS
	pstat->dot8021xalg = 0;
      	pstat->wpa_psk = 0;
	pstat->wpa_group_cipher = 0;
	pstat->wpa2_group_cipher = 0;
	pstat->wpa_pairwise_cipher = 0;
	pstat->wpa2_pairwise_cipher = 0;
	_memset(pstat->wpa_ie, 0, sizeof(pstat->wpa_ie));
	if((psecuritypriv->wpa_psk & BIT(1)) && elems.rsn_ie) {

		int group_cipher=0, pairwise_cipher=0;	
		
		wpa_ie = elems.rsn_ie;
		wpa_ie_len = elems.rsn_ie_len;

		if(parse_wpa2_ie(wpa_ie-2, wpa_ie_len+2, &group_cipher, &pairwise_cipher) == _SUCCESS)
		{
			pstat->dot8021xalg = 1;//psk,  todo:802.1x						
			pstat->wpa_psk |= BIT(1);

			pstat->wpa2_group_cipher = group_cipher&psecuritypriv->wpa2_group_cipher;				
			pstat->wpa2_pairwise_cipher = pairwise_cipher&psecuritypriv->wpa2_pairwise_cipher;
			
			if(!pstat->wpa2_group_cipher)
				status = WLAN_STATUS_GROUP_CIPHER_NOT_VALID;

			if(!pstat->wpa2_pairwise_cipher)
				status = WLAN_STATUS_PAIRWISE_CIPHER_NOT_VALID;
		}
		else
		{
			status = WLAN_STATUS_INVALID_IE;
		}	
			
	} else if ((psecuritypriv->wpa_psk & BIT(0)) && elems.wpa_ie) {

		int group_cipher=0, pairwise_cipher=0;	
		
		wpa_ie = elems.wpa_ie;
		wpa_ie_len = elems.wpa_ie_len;

		if(parse_wpa_ie(wpa_ie-2, wpa_ie_len+2, &group_cipher, &pairwise_cipher) == _SUCCESS)
		{
			pstat->dot8021xalg = 1;//psk,  todo:802.1x						
			pstat->wpa_psk |= BIT(0);

			pstat->wpa_group_cipher = group_cipher&psecuritypriv->wpa_group_cipher;				
			pstat->wpa_pairwise_cipher = pairwise_cipher&psecuritypriv->wpa_pairwise_cipher;
			
			if(!pstat->wpa_group_cipher)
				status = WLAN_STATUS_GROUP_CIPHER_NOT_VALID;

			if(!pstat->wpa_pairwise_cipher)
				status = WLAN_STATUS_PAIRWISE_CIPHER_NOT_VALID;
		
		}
		else
		{
			status = WLAN_STATUS_INVALID_IE;
		}
		
	} else {
		wpa_ie = NULL;
		wpa_ie_len = 0;
	}

	if(_STATS_SUCCESSFUL_ != status)
		goto OnAssocReqFail;

	pstat->flags &= ~(WLAN_STA_WPS | WLAN_STA_MAYBE_WPS);
	//if (hapd->conf->wps_state && wpa_ie == NULL) { //todo: to check ap if supporting WPS
	if(wpa_ie == NULL) {
		if (elems.wps_ie) {
			DBG_871X("STA included WPS IE in "
				   "(Re)Association Request - assume WPS is "
				   "used\n");
			pstat->flags |= WLAN_STA_WPS;
			//wpabuf_free(sta->wps_ie);
			//sta->wps_ie = wpabuf_alloc_copy(elems.wps_ie + 4,
			//				elems.wps_ie_len - 4);
		} else {
			DBG_871X("STA did not include WPA/RSN IE "
				   "in (Re)Association Request - possible WPS "
				   "use\n");
			pstat->flags |= WLAN_STA_MAYBE_WPS;
		}
	} else if (psecuritypriv->wpa_psk && wpa_ie == NULL) {
		DBG_871X("STA " MACSTR ": No WPA/RSN IE in association "
		       "request\n", MAC2STR(pstat->hwaddr));
		status = WLAN_STATUS_INVALID_IE;
		goto OnAssocReqFail;
	}
	else
	{
		int copy_len;

		copy_len = ((wpa_ie_len+2) > sizeof(pstat->wpa_ie)) ? (sizeof(pstat->wpa_ie)):(wpa_ie_len+2);

		_memcpy(pstat->wpa_ie, wpa_ie-2, copy_len);
	}


	// check if there is WMM IE & support WWM-PS
	pstat->flags &= ~WLAN_STA_WME;
	pstat->qos_option = 0;
	pstat->qos_info = 0;
	pstat->has_legacy_ac = _TRUE;
	pstat->uapsd_vo = 0;
	pstat->uapsd_vi = 0;
	pstat->uapsd_be = 0;
	pstat->uapsd_bk = 0;
	if (pmlmepriv->qospriv.qos_option) 
	{
		p = pframe + WLAN_HDR_A3_LEN + ie_offset; ie_len = 0;
		for (;;) 
		{
			p = get_ie(p, _VENDOR_SPECIFIC_IE_, &ie_len, pkt_len - WLAN_HDR_A3_LEN - ie_offset);
			if (p != NULL) {
				if (_memcmp(p+2, WMM_IE, 6)) {

					pstat->flags |= WLAN_STA_WME;
					
					pstat->qos_option = 1;				
					pstat->qos_info = *(p+8);
					
					pstat->max_sp_len = (pstat->qos_info>>5)&0x3;

					if((pstat->qos_info&0xf) !=0xf)
						pstat->has_legacy_ac = _TRUE;
					else
						pstat->has_legacy_ac = _FALSE;
					
					if(pstat->qos_info&0xf)
					{
						if(pstat->qos_info&BIT(0))
							pstat->uapsd_vo = BIT(0)|BIT(1);
						else
							pstat->uapsd_vo = 0;
		
						if(pstat->qos_info&BIT(1))
							pstat->uapsd_vi = BIT(0)|BIT(1);
						else
							pstat->uapsd_vi = 0;
			
						if(pstat->qos_info&BIT(2))
							pstat->uapsd_bk = BIT(0)|BIT(1);
						else
							pstat->uapsd_bk = 0;
			
						if(pstat->qos_info&BIT(3))			
							pstat->uapsd_be = BIT(0)|BIT(1);
						else
							pstat->uapsd_be = 0;
		
					}
	
					break;
				}
			}
			else {				
				break;
			}
			p = p + ie_len + 2;
		}
	}


#ifdef CONFIG_80211N_HT
	/* save HT capabilities in the sta object */
	_memset(&pstat->htpriv.ht_cap, 0, sizeof(struct ieee80211_ht_cap));
	if (elems.ht_capabilities && elems.ht_capabilities_len >= sizeof(struct ieee80211_ht_cap)) 
	{
		pstat->flags |= WLAN_STA_HT;
		
		pstat->flags |= WLAN_STA_WME;
		
		_memcpy(&pstat->htpriv.ht_cap, elems.ht_capabilities, sizeof(struct ieee80211_ht_cap));			
		
	} else
		pstat->flags &= ~WLAN_STA_HT;

	
	if((pmlmepriv->htpriv.ht_option == 0) && (pstat->flags&WLAN_STA_HT))
	{
		status = _STATS_FAILURE_;
		goto OnAssocReqFail;
	}
		

	if ((pstat->flags & WLAN_STA_HT) &&
		    ((pstat->wpa2_pairwise_cipher&WPA_CIPHER_TKIP) ||
		      (pstat->wpa_pairwise_cipher&WPA_CIPHER_TKIP)))
	{		    
		DBG_871X("HT: " MACSTR " tried to "
				   "use TKIP with HT association\n", MAC2STR(pstat->hwaddr));
		
		//status = WLAN_STATUS_CIPHER_REJECTED_PER_POLICY;
		//goto OnAssocReqFail;
	}
#endif /* CONFIG_80211N_HT */

       //
       //if (hapd->iface->current_mode->mode == HOSTAPD_MODE_IEEE80211G)//?
	pstat->flags |= WLAN_STA_NONERP;	
	for (i = 0; i < pstat->bssratelen; i++) {
		if ((pstat->bssrateset[i] & 0x7f) > 22) {
			pstat->flags &= ~WLAN_STA_NONERP;
			break;
		}
	}

	if (pstat->capability & WLAN_CAPABILITY_SHORT_PREAMBLE)
		pstat->flags |= WLAN_STA_SHORT_PREAMBLE;
	else
		pstat->flags &= ~WLAN_STA_SHORT_PREAMBLE;

	
	
	if (status != _STATS_SUCCESSFUL_)
		goto OnAssocReqFail;


	pstat->is_p2p_device = _FALSE;
	if(get_p2p_ie(pframe + WLAN_HDR_A3_LEN + ie_offset , pkt_len - WLAN_HDR_A3_LEN - ie_offset , p2pie, &p2pielen))
	{
		pstat->is_p2p_device = _TRUE;
		if((p2p_status_code=(u8)process_assoc_req_p2p_ie(pwdinfo, p2pie, p2pielen, pstat))>0)
		{
			pstat->p2p_status_code = p2p_status_code;
			goto OnAssocReqFail;
		}
	}	
	pstat->p2p_status_code = p2p_status_code;
		

	//TODO: identify_proprietary_vendor_ie();
	// Realtek proprietary IE
	// identify if this is Broadcom sta
	// identify if this is ralink sta
	// Customer proprietary IE

	

	/* get a unique AID */
	if (pstat->aid > 0) {
		DBG_871X("  old AID %d\n", pstat->aid);
	} else {
		for (pstat->aid = 1; pstat->aid <= NUM_STA; pstat->aid++)
			if (pstapriv->sta_aid[pstat->aid - 1] == NULL)
				break;
				
		//if (pstat->aid > NUM_STA) {
		if (pstat->aid > pstapriv->max_num_sta) {
				
			pstat->aid = 0;
				
			DBG_871X("  no room for more AIDs\n");

			status = WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA;
				
			goto OnAssocReqFail;
				
			
		} else {
			pstapriv->sta_aid[pstat->aid - 1] = pstat;
			DBG_871X("allocate new AID = (%d)\n", pstat->aid);
		}	
	}

	pstat->state |= WIFI_ASOC_STATE;
	
	if (!is_list_empty(&pstat->auth_list))
	{
		list_delete(&pstat->auth_list);
	}
	if (is_list_empty(&pstat->asoc_list))
	{
		pstat->expire_to = pstapriv->expire_to;
		list_insert_tail(&pstat->asoc_list, &pstapriv->asoc_list);		
	}
	

	// now the station is qualified to join our BSS...	
	if(pstat && (pstat->state & WIFI_ASOC_STATE) && (_STATS_SUCCESSFUL_==status))
	{
#ifdef CONFIG_NATIVEAP_MLME
		//.1 bss_cap_update
		//bss_cap_update(padapter, pstat);


		//.2 -
		DBG_871X("indicate_sta_join_event to upper layer - hostapd\n");		 
		indicate_sta_assoc_event(padapter, pstat);
		
	
		//.3-(1) report sta add event
		report_add_sta_event(padapter, pstat->hwaddr, pstat->aid);
		
		//.3 -(2)
		//sta_info_update(padapter, pstat);		
		
		if (frame_type == WIFI_ASSOCREQ)
			issue_asocrsp(padapter, status, pstat, WIFI_ASSOCRSP);
		else
			issue_asocrsp(padapter, status, pstat, WIFI_REASSOCRSP);
	
#endif
	}

	return _SUCCESS;

asoc_class2_error:

#ifdef CONFIG_NATIVEAP_MLME
	issue_deauth(padapter, (void *)GetAddr2Ptr(pframe), status);
#endif

	return _FAIL;

OnAssocReqFail:


#ifdef CONFIG_NATIVEAP_MLME
	pstat->aid = 0;
	if (frame_type == WIFI_ASSOCREQ)
		issue_asocrsp(padapter, status, pstat, WIFI_ASSOCRSP);
	else
		issue_asocrsp(padapter, status, pstat, WIFI_REASSOCRSP);
#endif


#endif /* CONFIG_AP_MODE */

	return _FAIL;		

}

unsigned int OnAssocRsp(_adapter *padapter, union recv_frame *precv_frame)
{
	uint i;
	int res;
	unsigned short	status;
	PNDIS_802_11_VARIABLE_IEs	pIE;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	//WLAN_BSSID_EX 		*cur_network = &(pmlmeinfo->network);
	u8 *pframe = precv_frame->u.hdr.rx_data;
	uint pkt_len = precv_frame->u.hdr.len;

	DBG_871X("%s\n", __FUNCTION__);
	
	//check A1 matches or not
	if (!_memcmp(myid(&(padapter->eeprompriv)), get_da(pframe), ETH_ALEN))
		return _SUCCESS;

	if (!(pmlmeinfo->state & (WIFI_FW_AUTH_SUCCESS | WIFI_FW_ASSOC_STATE)))
		return _SUCCESS;
	
	 if(pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS)
	 	return _SUCCESS;
	 
	_cancel_timer_ex(&pmlmeext->link_timer);	

	//status
	if ((status = le16_to_cpu(*(unsigned short *)(pframe + WLAN_HDR_A3_LEN + 2))) > 0)
	{
		DBG_871X("assoc reject, status: %x\n", status);
		res = -1;
		goto report_assoc_result;
	}

	//get capabilities
	pmlmeinfo->capability = le16_to_cpu(*(unsigned short *)(pframe + WLAN_HDR_A3_LEN));

	//set slot time
	pmlmeinfo->slotTime = (pmlmeinfo->capability & BIT(10))? 9: 20;

	//AID
	res = pmlmeinfo->aid = (int)(le16_to_cpu(*(unsigned short *)(pframe + WLAN_HDR_A3_LEN + 4))&0x3fff);

	//following are moved to join event callback function
	//to handle HT, WMM, rate adaptive, update MAC reg
	//for not to handle the synchronous IO in the tasklet
	for (i = (6 + WLAN_HDR_A3_LEN); i < pkt_len;)
	{
		pIE = (PNDIS_802_11_VARIABLE_IEs)(pframe + i);

		switch (pIE->ElementID)
		{
			case _VENDOR_SPECIFIC_IE_:
				if (_memcmp(pIE->data, WMM_PARA_OUI, 6))	//WMM
				{
					WMM_param_handler(padapter, pIE);
				}
				break;

			case _HT_CAPABILITY_IE_:	//HT caps
				HT_caps_handler(padapter, pIE);
				break;

			case _HT_EXTRA_INFO_IE_:	//HT info
				HT_info_handler(padapter, pIE);
				break;

			case _ERPINFO_IE_:
				ERP_IE_handler(padapter, pIE);

			default:
				break;
		}

		i += (pIE->Length + 2);
	}

	pmlmeinfo->state &= (~WIFI_FW_ASSOC_STATE);
	pmlmeinfo->state |= WIFI_FW_ASSOC_SUCCESS;

	//Update Basic Rate Table for spec, 2010-12-28 , by thomas
	UpdateBrateTbl(padapter, pmlmeinfo->network.SupportedRates);

report_assoc_result:

	report_join_res(padapter, res);

	return _SUCCESS;
}

unsigned int OnDeAuth(_adapter *padapter, union recv_frame *precv_frame)
{	
	unsigned short	reason;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	u8 *pframe = precv_frame->u.hdr.rx_data;
	uint pktlen = precv_frame->u.hdr.len;

	//check A3
	if (!(_memcmp(GetAddr3Ptr(pframe), get_my_bssid(&pmlmeinfo->network), ETH_ALEN)))
		return _SUCCESS;
	
	reason = le16_to_cpu(*(unsigned short *)(pframe + WLAN_HDR_A3_LEN));

        DBG_871X("%s Reason code(%d)\n", __FUNCTION__,reason);

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE)
	{		
#if 0	
		_irqL irqL;
		struct sta_info *psta;
		struct sta_priv *pstapriv = &padapter->stapriv;
		
		psta = get_stainfo(pstapriv, GetAddr2Ptr(pframe));
		_enter_critical_bh(&(pstapriv->sta_hash_lock), &irqL);		
		free_stainfo(padapter, psta);
		_exit_critical_bh(&(pstapriv->sta_hash_lock), &irqL);		
#endif
		ap_free_sta(padapter, get_stainfo(&padapter->stapriv, GetAddr2Ptr(pframe)));
				
		return _SUCCESS;
	}	
	else
	{
		receive_disconnect(padapter, GetAddr3Ptr(pframe));
	}	
	
	return _SUCCESS;
	
}

unsigned int OnDisassoc(_adapter *padapter, union recv_frame *precv_frame)
{
	unsigned short	reason;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	u8 *pframe = precv_frame->u.hdr.rx_data;
	uint pktlen = precv_frame->u.hdr.len;

	//check A3
	if (!(_memcmp(GetAddr3Ptr(pframe), get_my_bssid(&pmlmeinfo->network), ETH_ALEN)))
		return _SUCCESS;
	
	reason = le16_to_cpu(*(unsigned short *)(pframe + WLAN_HDR_A3_LEN));

        DBG_871X("%s Reason code(%d)\n", __FUNCTION__,reason);

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE)
	{	
#if 0	
		_irqL irqL;
		struct sta_info *psta;
		struct sta_priv *pstapriv = &padapter->stapriv;
		
		psta = get_stainfo(pstapriv, GetAddr2Ptr(pframe));
		_enter_critical_bh(&(pstapriv->sta_hash_lock), &irqL);	
		free_stainfo(padapter, psta);
		_exit_critical_bh(&(pstapriv->sta_hash_lock), &irqL);		
#endif

		ap_free_sta(padapter, get_stainfo(&padapter->stapriv, GetAddr2Ptr(pframe)));
			
		return _SUCCESS;
	}	
	else
	{
		receive_disconnect(padapter, GetAddr3Ptr(pframe));
	}	
	
	return _SUCCESS;
	
}

unsigned int OnAtim(_adapter *padapter, union recv_frame *precv_frame)
{
	DBG_871X("%s\n", __FUNCTION__);
	return _SUCCESS;
}

unsigned int OnAction_qos(_adapter *padapter, union recv_frame *precv_frame)
{
	return _SUCCESS;
}

unsigned int OnAction_dls(_adapter *padapter, union recv_frame *precv_frame)
{
	return _SUCCESS;
}

unsigned int OnAction_back(_adapter *padapter, union recv_frame *precv_frame)
{
	u8 *addr;
	struct sta_info *psta=NULL;
	struct recv_reorder_ctrl *preorder_ctrl;
	unsigned char		*frame_body;
	unsigned char		category, action;
	unsigned short	tid, status, reason_code;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	u8 *pframe = precv_frame->u.hdr.rx_data;
	struct sta_priv *pstapriv = &padapter->stapriv;
					
	uint len = precv_frame->u.hdr.len;

	//check RA matches or not	
	if (!_memcmp(myid(&(padapter->eeprompriv)), GetAddr1Ptr(pframe), ETH_ALEN))//for if1, sta/ap mode
		return _SUCCESS;

/*
	//check A1 matches or not
	if (!_memcmp(myid(&(padapter->eeprompriv)), get_da(pframe), ETH_ALEN))
		return _SUCCESS;
*/
	DBG_871X("%s\n", __FUNCTION__);

	if((pmlmeinfo->state&0x03) != WIFI_FW_AP_STATE)	
		if (!(pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS))
			return _SUCCESS;

	addr = GetAddr2Ptr(pframe);
	psta = get_stainfo(pstapriv, addr);

	if(psta==NULL)
		return _SUCCESS;

	frame_body = (unsigned char *)(pframe + sizeof(struct ieee80211_hdr_3addr));

	category = frame_body[0];
	if (category == WLAN_CATEGORY_BACK)// representing Block Ack
	{
		if (!pmlmeinfo->HT_enable)
		{
			return _SUCCESS;
		}

		action = frame_body[1];
		DBG_871X("%s, action=%d\n", __FUNCTION__, action);
		switch (action)
		{
			case WLAN_ACTION_ADDBA_REQ: //ADDBA request

				_memcpy(&(pmlmeinfo->ADDBA_req), &(frame_body[2]), sizeof(struct ADDBA_request));
				//process_addba_req(padapter, (u8*)&(pmlmeinfo->ADDBA_req), GetAddr3Ptr(pframe));
				process_addba_req(padapter, (u8*)&(pmlmeinfo->ADDBA_req), addr);
				
				if(pmlmeinfo->bAcceptAddbaReq == _TRUE)
				{
					issue_action_BA(padapter, addr, WLAN_ACTION_ADDBA_RESP, 0);
				}
				else
				{
					issue_action_BA(padapter, addr, WLAN_ACTION_ADDBA_RESP, 37);//reject ADDBA Req
				}
								
				break;

			case WLAN_ACTION_ADDBA_RESP: //ADDBA response

				status = frame_body[3] | (frame_body[4] << 8); //endian issue
				tid = ((frame_body[5] >> 2) & 0x7);

				if (status == 0)
				{	//successful					
					psta->htpriv.agg_enable_bitmap |= 1 << tid;					
					psta->htpriv.candidate_tid_bitmap &= ~BIT(tid);				
				}
				else
				{					
					psta->htpriv.agg_enable_bitmap &= ~BIT(tid);					
				}

				//printk("marc: ADDBA RSP: %x\n", pmlmeinfo->agg_enable_bitmap);
				break;

			case WLAN_ACTION_DELBA: //DELBA
				if ((frame_body[3] & BIT(3)) == 0)
				{
					psta->htpriv.agg_enable_bitmap &= ~(1 << ((frame_body[3] >> 4) & 0xf));
					psta->htpriv.candidate_tid_bitmap &= ~(1 << ((frame_body[3] >> 4) & 0xf));
					
					reason_code = frame_body[4] | (frame_body[5] << 8);
				}
				else if((frame_body[3] & BIT(3)) == 1)
				{						
					tid = (frame_body[3] >> 4) & 0x0F;
				
					preorder_ctrl =  &psta->recvreorder_ctrl[tid];
					preorder_ctrl->enable = _FALSE;
					preorder_ctrl->indicate_seq = 0xffff;
				}
				
				printk("%s(): DELBA: %x(%x)\n", __FUNCTION__,pmlmeinfo->agg_enable_bitmap, reason_code);
				//todo: how to notify the host while receiving DELETE BA
				break;

			default:
				break;
		}
	}

	return _SUCCESS;
}

void issue_p2p_GO_response(_adapter *padapter, u8* raddr, u8* frame_body)
{

	unsigned char category = WLAN_CATEGORY_PUBLIC;
	u8			action = P2P_PUB_ACTION_ACTION;
	u8			dialogToken = frame_body[7];	//	The Dialog Token of provisioning discovery request frame.
	u32			p2poui = cpu_to_be32(P2POUI);
	u8			oui_subtype = P2P_GO_NEGO_RESP;
	u8			wpsie[ 255 ] = { 0x00 }, p2pie[ 255 ] = { 0x00 };
	u8			wpsielen = 0, p2pielen = 0;
	
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct ieee80211_hdr	*pwlanhdr;
	unsigned short				*fctrl;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo);


	if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
	{
		return;
	}

	//update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	_memcpy(pwlanhdr->addr1, raddr, ETH_ALEN);
	_memcpy(pwlanhdr->addr2, myid(&(padapter->eeprompriv)), ETH_ALEN);
	_memcpy(pwlanhdr->addr3, myid(&(padapter->eeprompriv)), ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	SetFrameSubType(pframe, WIFI_ACTION);

	pframe += sizeof(struct ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct ieee80211_hdr_3addr);

	pframe = set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
	pframe = set_fixed_ie(pframe, 1, &(action), &(pattrib->pktlen));
	pframe = set_fixed_ie(pframe, 4, (unsigned char *) &(p2poui), &(pattrib->pktlen));
	pframe = set_fixed_ie(pframe, 1, &(oui_subtype), &(pattrib->pktlen));	
	pframe = set_fixed_ie(pframe, 1, &(dialogToken), &(pattrib->pktlen));		

	//	WPS Section
	wpsielen = 0;
	//	WPS OUI
	*(u32*) ( wpsie ) = cpu_to_be32( WPSOUI );
	wpsielen += 4;

	//	WPS version
	//	Type:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_ATTR_VER1 );
	wpsielen += 2;

	//	Length:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( 0x0001 );
	wpsielen += 2;

	//	Value:
	wpsie[wpsielen++] = WPS_VERSION_1;	//	Version 1.0

	//	Device Password ID
	//	Type:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_ATTR_DEVICE_PWID );
	wpsielen += 2;

	//	Length:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( 0x0002 );
	wpsielen += 2;

	//	Value:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_DPID_PIN );
	wpsielen += 2;

	pframe = set_ie(pframe, _VENDOR_SPECIFIC_IE_, wpsielen, (unsigned char *) wpsie, &pattrib->pktlen );


	//	P2P IE Section.

	//	P2P OUI
	p2pielen = 0;
	p2pie[ p2pielen++ ] = 0x50;
	p2pie[ p2pielen++ ] = 0x6F;
	p2pie[ p2pielen++ ] = 0x9A;
	p2pie[ p2pielen++ ] = 0x09;	//	WFA P2P v1.0

	//	Commented by Albert 20100908
	//	According to the P2P Specification, the group negoitation response frame should contain 9 P2P attributes
	//	1. Status
	//	2. P2P Capability
	//	3. Group Owner Intent
	//	4. Configuration Timeout
	//	5. Operating Channel
	//	6. Intended P2P Interface Address
	//	7. Channel List
	//	8. Device Info
	//	9. Group ID	( Only GO )


	//	ToDo:

	//	P2P Status
	//	Type:
	p2pie[ p2pielen++ ] = P2P_ATTR_STATUS;

	//	Length:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0x0001 );
	p2pielen += 2;

	//	Value:
	p2pie[ p2pielen++ ] = P2P_STATUS_SUCCESS;
	
	//	P2P Capability
	//	Type:
	p2pie[ p2pielen++ ] = P2P_ATTR_CAPABILITY;

	//	Length:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0x0002 );
	p2pielen += 2;

	//	Value:
	//	Device Capability Bitmap, 1 byte
	//	Be able to participate in additional P2P Groups and
	//	support the P2P Invitation Procedure
	p2pie[ p2pielen++ ] = P2P_DEVCAP_INVITATION_PROC;
	
	//	Group Capability Bitmap, 1 byte
	p2pie[ p2pielen++ ] = 0x00;

	//	Group Owner Intent
	//	Type:
	p2pie[ p2pielen++ ] = P2P_ATTR_GO_INTENT;

	//	Length:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0x0001 );
	p2pielen += 2;

	//	Value:
	//	Todo the tie breaker bit.
	p2pie[ p2pielen++ ] = ( ( pwdinfo->intent << 1 ) | BIT(0) );

	//	Configuration Timeout
	//	Type:
	p2pie[ p2pielen++ ] = P2P_ATTR_CONF_TIMEOUT;

	//	Length:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0x0002 );
	p2pielen += 2;

	//	Value:
	p2pie[ p2pielen++ ] = 200;	//	2 seconds needed to be the P2P GO
	p2pie[ p2pielen++ ] = 200;	//	2 seconds needed to be the P2P Client

	//	Operating Channel
	//	Type:
	p2pie[ p2pielen++ ] = P2P_ATTR_OPERATING_CH;

	//	Length:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0x0005 );
	p2pielen += 2;

	//	Value:
	//	Country String
	p2pie[ p2pielen++ ] = 'U';
	p2pie[ p2pielen++ ] = 'S';
	
	//	The third byte should be set to 0x04.
	//	Described in the "Operating Channel Attribute" section.
	p2pie[ p2pielen++ ] = 0x04;

	//	Operating Class
	p2pie[ p2pielen++ ] = 0x51;	//	Copy from SD7
	
	//	Channel Number
	p2pie[ p2pielen++ ] = 11;	//	operating channel number

	//	Intended P2P Interface Address	
	//	Type:
	p2pie[ p2pielen++ ] = P2P_ATTR_INTENTED_IF_ADDR;

	//	Length:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( ETH_ALEN );
	p2pielen += 2;

	//	Value:
	_memcpy( p2pie + p2pielen, myid( &padapter->eeprompriv ), ETH_ALEN );
	p2pielen += ETH_ALEN;

	//	Channel List
	//	Type:
	p2pie[ p2pielen++ ] = P2P_ATTR_CH_LIST;

	//	Length:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0x0010 );
	p2pielen += 2;

	//	Value:
	//	Country String
	p2pie[ p2pielen++ ] = 'U';
	p2pie[ p2pielen++ ] = 'S';
	
	//	The third byte should be set to 0x04.
	//	Described in the "Operating Channel Attribute" section.
	p2pie[ p2pielen++ ] = 0x04;

	//	Channel Entry List
	//	Operating Class
	p2pie[ p2pielen++ ] = 0x51;	//	Copy from SD7

	//	Number of Channels
	p2pie[ p2pielen++ ] = 0x0B;	//	support channel 1 - 11

	//	Channel List
	p2pie[ p2pielen++ ] = 0x01;
	p2pie[ p2pielen++ ] = 0x02;
	p2pie[ p2pielen++ ] = 0x03;
	p2pie[ p2pielen++ ] = 0x04;
	p2pie[ p2pielen++ ] = 0x05;
	p2pie[ p2pielen++ ] = 0x06;
	p2pie[ p2pielen++ ] = 0x07;
	p2pie[ p2pielen++ ] = 0x08;
	p2pie[ p2pielen++ ] = 0x09;
	p2pie[ p2pielen++ ] = 0x0A;
	p2pie[ p2pielen++ ] = 0x0B;	
	
	//	Device Info
	//	Type:
	p2pie[ p2pielen++ ] = P2P_ATTR_DEVICE_INFO;

	//	Length:
	//	21 -> P2P Device Address (6bytes) + Config Methods (2bytes) + Primary Device Type (8bytes) 
	//	+ NumofSecondDevType (1byte) + WPS Device Name ID field (2bytes) + WPS Device Name Len field (2bytes)
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 21 + pwdinfo->device_name_len );
	p2pielen += 2;

	//	Value:
	//	P2P Device Address
	_memcpy( p2pie + p2pielen, myid( &padapter->eeprompriv ), ETH_ALEN );
	p2pielen += ETH_ALEN;

	//	Config Method
	//	This field should be big endian. Noted by P2P specification.
	*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( WPS_CONFIG_METHOD_DISPLAY );
	p2pielen += 2;

	//	Primary Device Type
	//	Category ID
	*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( WPS_PDT_CID_RTK_WIDI );
	p2pielen += 2;

	//	OUI
	*(u32*) ( p2pie + p2pielen ) = cpu_to_be32( WPSOUI );
	p2pielen += 4;

	//	Sub Category ID
	*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( WPS_PDT_SCID_RTK_DMP );
	p2pielen += 2;

	//	Number of Secondary Device Types
	p2pie[ p2pielen++ ] = 0x00;	//	No Secondary Device Type List

	//	Device Name
	//	Type:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( WPS_ATTR_DEVICE_NAME );
	p2pielen += 2;

	//	Length:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( pwdinfo->device_name_len );
	p2pielen += 2;

	//	Value:
	_memcpy( p2pie + p2pielen, pwdinfo->device_name , pwdinfo->device_name_len );
	p2pielen += pwdinfo->device_name_len;	
	
	pframe = set_ie(pframe, _VENDOR_SPECIFIC_IE_, p2pielen, (unsigned char *) p2pie, &pattrib->pktlen );	
	

	pattrib->last_txcmdsz = pattrib->pktlen;

	dump_mgntframe(padapter, pmgntframe);

	return;

}

void issue_p2p_invitation_request(_adapter *padapter, u8* raddr )
{

	unsigned char category = WLAN_CATEGORY_PUBLIC;
	u8			action = P2P_PUB_ACTION_ACTION;
	u32			p2poui = cpu_to_be32(P2POUI);
	u8			oui_subtype = P2P_INVIT_REQ;
	u8			p2pie[ 255 ] = { 0x00 };
	u8			p2pielen = 0;
	u8			dialogToken = 3;
	
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct ieee80211_hdr	*pwlanhdr;
	unsigned short				*fctrl;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo);


	if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
	{
		return;
	}

	//update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	_memcpy(pwlanhdr->addr1, raddr, ETH_ALEN);
	_memcpy(pwlanhdr->addr2, myid(&(padapter->eeprompriv)), ETH_ALEN);
	_memcpy(pwlanhdr->addr3, raddr,  ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	SetFrameSubType(pframe, WIFI_ACTION);

	pframe += sizeof(struct ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct ieee80211_hdr_3addr);

	pframe = set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
	pframe = set_fixed_ie(pframe, 1, &(action), &(pattrib->pktlen));
	pframe = set_fixed_ie(pframe, 4, (unsigned char *) &(p2poui), &(pattrib->pktlen));
	pframe = set_fixed_ie(pframe, 1, &(oui_subtype), &(pattrib->pktlen));	
	pframe = set_fixed_ie(pframe, 1, &(dialogToken), &(pattrib->pktlen));

	//	P2P IE Section.

	//	P2P OUI
	p2pielen = 0;
	p2pie[ p2pielen++ ] = 0x50;
	p2pie[ p2pielen++ ] = 0x6F;
	p2pie[ p2pielen++ ] = 0x9A;
	p2pie[ p2pielen++ ] = 0x09;	//	WFA P2P v1.0

	//	Commented by Albert 20101011
	//	According to the P2P Specification, the P2P Invitation request frame should contain 7 P2P attributes
	//	1. Configuration Timeout
	//	2. Invitation Flags
	//	3. Operating Channel	( Only GO )
	//	4. P2P Group BSSID	( Only GO )
	//	5. Channel List
	//	6. P2P Group ID
	//	7. P2P Device Info

	//	Configuration Timeout
	//	Type:
	p2pie[ p2pielen++ ] = P2P_ATTR_CONF_TIMEOUT;

	//	Length:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0x0002 );
	p2pielen += 2;

	//	Value:
	p2pie[ p2pielen++ ] = 200;	//	2 seconds needed to be the P2P GO
	p2pie[ p2pielen++ ] = 200;	//	2 seconds needed to be the P2P Client

	//	Invitation Flags
	//	Type:
	p2pie[ p2pielen++ ] = P2P_ATTR_INVITATION_FLAGS;

	//	Length:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0x0001 );
	p2pielen += 2;

	//	Value:
	p2pie[ p2pielen++ ] = P2P_INVITATION_FLAGS_PERSISTENT;


	//	Channel List
	//	Type:
	p2pie[ p2pielen++ ] = P2P_ATTR_CH_LIST;

	//	Length:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0x0010 );
	p2pielen += 2;

	//	Value:
	//	Country String
	p2pie[ p2pielen++ ] = 'U';
	p2pie[ p2pielen++ ] = 'S';
	
	//	The third byte should be set to 0x04.
	//	Described in the "Operating Channel Attribute" section.
	p2pie[ p2pielen++ ] = 0x04;

	//	Channel Entry List
	//	Operating Class
	p2pie[ p2pielen++ ] = 0x51;	//	Copy from SD7

	//	Number of Channels
	p2pie[ p2pielen++ ] = 0x0B;	//	support channel 1 - 11

	//	Channel List
	p2pie[ p2pielen++ ] = 0x01;
	p2pie[ p2pielen++ ] = 0x02;
	p2pie[ p2pielen++ ] = 0x03;
	p2pie[ p2pielen++ ] = 0x04;
	p2pie[ p2pielen++ ] = 0x05;
	p2pie[ p2pielen++ ] = 0x06;
	p2pie[ p2pielen++ ] = 0x07;
	p2pie[ p2pielen++ ] = 0x08;
	p2pie[ p2pielen++ ] = 0x09;
	p2pie[ p2pielen++ ] = 0x0A;
	p2pie[ p2pielen++ ] = 0x0B;	

	//	P2P Group ID
	//	Type:
	p2pie[ p2pielen++ ] = P2P_ATTR_GROUP_ID;

	//	Length:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 6 + pwdinfo->invitereq_info.ssidlen );
	p2pielen += 2;

	//	Value:
	//	P2P Device Address for GO
	_memcpy( p2pie + p2pielen, raddr, ETH_ALEN );
	p2pielen += ETH_ALEN;

	//	SSID
	_memcpy( p2pie + p2pielen, pwdinfo->invitereq_info.ssid, pwdinfo->invitereq_info.ssidlen );
	p2pielen += pwdinfo->invitereq_info.ssidlen;
	

	//	Device Info
	//	Type:
	p2pie[ p2pielen++ ] = P2P_ATTR_DEVICE_INFO;

	//	Length:
	//	21 -> P2P Device Address (6bytes) + Config Methods (2bytes) + Primary Device Type (8bytes) 
	//	+ NumofSecondDevType (1byte) + WPS Device Name ID field (2bytes) + WPS Device Name Len field (2bytes)
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 21 + pwdinfo->device_name_len );
	p2pielen += 2;
	
	//	Value:
	//	P2P Device Address
	_memcpy( p2pie + p2pielen, myid( &padapter->eeprompriv ), ETH_ALEN );
	p2pielen += ETH_ALEN;

	//	Config Method
	//	This field should be big endian. Noted by P2P specification.
	*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( WPS_CONFIG_METHOD_DISPLAY );
	p2pielen += 2;

	//	Primary Device Type
	//	Category ID
	*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( WPS_PDT_CID_RTK_WIDI );
	p2pielen += 2;

	//	OUI
	*(u32*) ( p2pie + p2pielen ) = cpu_to_be32( WPSOUI );
	p2pielen += 4;

	//	Sub Category ID
	*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( WPS_PDT_SCID_RTK_DMP );
	p2pielen += 2;

	//	Number of Secondary Device Types
	p2pie[ p2pielen++ ] = 0x00;	//	No Secondary Device Type List

	//	Device Name
	//	Type:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( WPS_ATTR_DEVICE_NAME );
	p2pielen += 2;

	//	Length:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( pwdinfo->device_name_len );
	p2pielen += 2;

	//	Value:
	_memcpy( p2pie + p2pielen, pwdinfo->device_name, pwdinfo->device_name_len );
	p2pielen += pwdinfo->device_name_len;
		
	pframe = set_ie(pframe, _VENDOR_SPECIFIC_IE_, p2pielen, (unsigned char *) p2pie, &pattrib->pktlen );	
	

	pattrib->last_txcmdsz = pattrib->pktlen;

	dump_mgntframe(padapter, pmgntframe);

	return;

}



void issue_p2p_invitation_response(_adapter *padapter, u8* raddr, u8 dialogToken, u8 success)
{

	unsigned char category = WLAN_CATEGORY_PUBLIC;
	u8			action = P2P_PUB_ACTION_ACTION;
	u32			p2poui = cpu_to_be32(P2POUI);
	u8			oui_subtype = P2P_INVIT_RESP;
	u8			p2pie[ 255 ] = { 0x00 };
	u8			p2pielen = 0;
	
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct ieee80211_hdr	*pwlanhdr;
	unsigned short				*fctrl;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo);


	if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
	{
		return;
	}

	//update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	_memcpy(pwlanhdr->addr1, raddr, ETH_ALEN);
	_memcpy(pwlanhdr->addr2, myid(&(padapter->eeprompriv)), ETH_ALEN);
	_memcpy(pwlanhdr->addr3, raddr,  ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	SetFrameSubType(pframe, WIFI_ACTION);

	pframe += sizeof(struct ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct ieee80211_hdr_3addr);

	pframe = set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
	pframe = set_fixed_ie(pframe, 1, &(action), &(pattrib->pktlen));
	pframe = set_fixed_ie(pframe, 4, (unsigned char *) &(p2poui), &(pattrib->pktlen));
	pframe = set_fixed_ie(pframe, 1, &(oui_subtype), &(pattrib->pktlen));	
	pframe = set_fixed_ie(pframe, 1, &(dialogToken), &(pattrib->pktlen));

	//	P2P IE Section.

	//	P2P OUI
	p2pielen = 0;
	p2pie[ p2pielen++ ] = 0x50;
	p2pie[ p2pielen++ ] = 0x6F;
	p2pie[ p2pielen++ ] = 0x9A;
	p2pie[ p2pielen++ ] = 0x09;	//	WFA P2P v1.0

	//	Commented by Albert 20101005
	//	According to the P2P Specification, the P2P Invitation response frame should contain 5 P2P attributes
	//	1. Status
	//	2. Configuration Timeout
	//	3. Operating Channel	( Only GO )
	//	4. P2P Group BSSID	( Only GO )
	//	5. Channel List

	//	P2P Status
	//	Type:
	p2pie[ p2pielen++ ] = P2P_ATTR_STATUS;

	//	Length:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0x0001 );
	p2pielen += 2;

	//	Value:
	if ( success )
	{
		p2pie[ p2pielen++ ] = P2P_STATUS_SUCCESS;
	}
	else
	{
		//	Sent the event receiving the P2P Invitation Req frame to DMP UI.
		//	DMP had to compare the MAC address to find out the profile.
		//	So, the WiFi driver will send the P2P_STATUS_FAIL_INFO_UNAVAILABLE to NB.
		//	If the UI found the corresponding profile, the WiFi driver sends the P2P Invitation Req
		//	to NB to rebuild the persistent group.
		p2pie[ p2pielen++ ] = P2P_STATUS_FAIL_INFO_UNAVAILABLE;
	}
	
	//	Configuration Timeout
	//	Type:
	p2pie[ p2pielen++ ] = P2P_ATTR_CONF_TIMEOUT;

	//	Length:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0x0002 );
	p2pielen += 2;

	//	Value:
	p2pie[ p2pielen++ ] = 200;	//	2 seconds needed to be the P2P GO
	p2pie[ p2pielen++ ] = 200;	//	2 seconds needed to be the P2P Client


	if ( success )
	{
		//	Channel List
		//	Type:
		p2pie[ p2pielen++ ] = P2P_ATTR_CH_LIST;

		//	Length:
		*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0x0010 );
		p2pielen += 2;

		//	Value:
		//	Country String
		p2pie[ p2pielen++ ] = 'U';
		p2pie[ p2pielen++ ] = 'S';
	
		//	The third byte should be set to 0x04.
		//	Described in the "Operating Channel Attribute" section.
		p2pie[ p2pielen++ ] = 0x04;

		//	Channel Entry List
		//	Operating Class
		p2pie[ p2pielen++ ] = 0x51;	//	Copy from SD7

		//	Number of Channels
		p2pie[ p2pielen++ ] = 0x0B;	//	support channel 1 - 11

		//	Channel List
		p2pie[ p2pielen++ ] = 0x01;
		p2pie[ p2pielen++ ] = 0x02;
		p2pie[ p2pielen++ ] = 0x03;
		p2pie[ p2pielen++ ] = 0x04;
		p2pie[ p2pielen++ ] = 0x05;
		p2pie[ p2pielen++ ] = 0x06;
		p2pie[ p2pielen++ ] = 0x07;
		p2pie[ p2pielen++ ] = 0x08;
		p2pie[ p2pielen++ ] = 0x09;
		p2pie[ p2pielen++ ] = 0x0A;
		p2pie[ p2pielen++ ] = 0x0B;	
	}
		
	pframe = set_ie(pframe, _VENDOR_SPECIFIC_IE_, p2pielen, (unsigned char *) p2pie, &pattrib->pktlen );	
	

	pattrib->last_txcmdsz = pattrib->pktlen;

	dump_mgntframe(padapter, pmgntframe);

	return;

}

void issue_p2p_provision_response(_adapter *padapter, u8* raddr, u8* frame_body)
{

	unsigned char category = WLAN_CATEGORY_PUBLIC;
	u8			action = P2P_PUB_ACTION_ACTION;
	u8			dialogToken = frame_body[7];	//	The Dialog Token of provisioning discovery request frame.
	u32			p2poui = cpu_to_be32(P2POUI);
	u8			oui_subtype = P2P_PROVISION_DISC_RESP;
	u8			wpsie[ 100 ] = { 0x00 };
	u8			wpsielen = 0;
	
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct ieee80211_hdr	*pwlanhdr;
	unsigned short				*fctrl;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);


	if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
	{
		return;
	}

	//update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	_memcpy(pwlanhdr->addr1, raddr, ETH_ALEN);
	_memcpy(pwlanhdr->addr2, myid(&(padapter->eeprompriv)), ETH_ALEN);
	_memcpy(pwlanhdr->addr3, myid(&(padapter->eeprompriv)), ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	SetFrameSubType(pframe, WIFI_ACTION);

	pframe += sizeof(struct ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct ieee80211_hdr_3addr);

	pframe = set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
	pframe = set_fixed_ie(pframe, 1, &(action), &(pattrib->pktlen));
	pframe = set_fixed_ie(pframe, 4, (unsigned char *) &(p2poui), &(pattrib->pktlen));
	pframe = set_fixed_ie(pframe, 1, &(oui_subtype), &(pattrib->pktlen));	
	pframe = set_fixed_ie(pframe, 1, &(dialogToken), &(pattrib->pktlen));		

	wpsielen = 0;
	//	WPS OUI
	*(u32*) ( wpsie ) = cpu_to_be32( WPSOUI );
	wpsielen += 4;

	//	WPS version
	//	Type:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_ATTR_VER1 );
	wpsielen += 2;

	//	Length:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( 0x0001 );
	wpsielen += 2;

	//	Value:
	wpsie[wpsielen++] = WPS_VERSION_1;	//	Version 1.0

	//	Config Method
	//	Type:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_ATTR_CONF_METHOD );
	wpsielen += 2;

	//	Length:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( 0x0002 );
	wpsielen += 2;

	//	Value:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_CONFIG_METHOD_DISPLAY );
	wpsielen += 2;

	pframe = set_ie(pframe, _VENDOR_SPECIFIC_IE_, wpsielen, (unsigned char *) wpsie, &pattrib->pktlen );	

	pattrib->last_txcmdsz = pattrib->pktlen;

	dump_mgntframe(padapter, pmgntframe);

	return;

}

u8 is_matched_in_profilelist( u8* peermacaddr, struct profile_info* profileinfo )
{
	u8 i, match_result = 0;

	printk( "[%s] peermac = %.2X %.2X %.2X %.2X %.2X %.2X\n", __FUNCTION__,
	  	    peermacaddr[0], peermacaddr[1],peermacaddr[2],peermacaddr[3],peermacaddr[4],peermacaddr[5]);
	
	for( i = 0; i < P2P_MAX_PERSISTENT_GROUP_NUM; i++, profileinfo++ )
	{
	       printk( "[%s] profileinfo_mac = %.2X %.2X %.2X %.2X %.2X %.2X\n", __FUNCTION__,
		   	    profileinfo->peermac[0], profileinfo->peermac[1],profileinfo->peermac[2],profileinfo->peermac[3],profileinfo->peermac[4],profileinfo->peermac[5]);		   
		if ( _memcmp( peermacaddr, profileinfo->peermac, ETH_ALEN ) )
		{
			match_result = 1;
			printk( "[%s] Match!\n", __FUNCTION__ );
			break;
		}
	}
	
	return (match_result );
}

unsigned int OnAction_public(_adapter *padapter, union recv_frame *precv_frame)
{
	unsigned char		*frame_body;
	unsigned char		category, action;
	u8 *pframe = precv_frame->u.hdr.rx_data;
	uint len = precv_frame->u.hdr.len;
	u8	p2p_ie[ 255 ];
	u32	p2p_ielen;
	struct	wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	//check RA matches or not
	if (!_memcmp(myid(&(padapter->eeprompriv)), GetAddr1Ptr(pframe), ETH_ALEN))//for if1, sta/ap mode
		return _SUCCESS;

	frame_body = (unsigned char *)(pframe + sizeof(struct ieee80211_hdr_3addr));

	category = frame_body[0];
	if(category != WLAN_CATEGORY_PUBLIC)
		return _SUCCESS;

		action = frame_body[ 1 ];
	if ( action == ACT_PUBLIC_P2P )	//	IEEE 802.11 P2P Public Action usage.
	{		
		//	Commented by Albert 20100908
		//	Low byte -> High byte is 0x50, 0x6F, 0x9A, 0x09 for P2P OUI.
		//	But the P2POUT is defined as 0x506F9A09 -> should use the cpu_to_be32
		if ( cpu_to_be32( *( ( u32* ) ( frame_body + 2 ) ) ) == P2POUI )
		{
			_memset( p2p_ie, 0x00, 255 );
			p2p_ielen = 0;
			
			switch( frame_body[ 6 ] )//OUI Subtype
			{
				case P2P_GO_NEGO_REQ:
				{
					if ( get_p2p_ie( frame_body + _PUBLIC_ACTION_IE_OFFSET_, len - _PUBLIC_ACTION_IE_OFFSET_, p2p_ie, &p2p_ielen ) )
					{
						u8	attr_content = 0x00;
						u32	attr_contentlen = 0;						

						pwdinfo->p2p_state = P2P_STATE_GONEGO_ING;
						get_p2p_attr_content(p2p_ie, p2p_ielen, P2P_ATTR_GO_INTENT , &attr_content, &attr_contentlen);
						printk( "[%s] Got GO Nego Req Frame, GO Intent = %d, tie = %d\n", __FUNCTION__, attr_content >> 1, attr_content & 0x01 );
						pwdinfo->peer_intent = attr_content;	//	include both intent and tie breaker values.
						attr_contentlen = 0;
						get_p2p_attr_content( p2p_ie, p2p_ielen, P2P_ATTR_INTENTED_IF_ADDR, pwdinfo->p2p_peer_interface_addr, &attr_contentlen );
						
						if ( attr_contentlen != ETH_ALEN )
						{
							_memset( pwdinfo->p2p_peer_interface_addr, 0x00, ETH_ALEN );
						}
						
						issue_p2p_GO_response( padapter, GetAddr2Ptr(pframe), frame_body);
					}
					break;					
				}
				case P2P_GO_NEGO_RESP:
				{

					break;
				}
				case P2P_GO_NEGO_CONF:
				{
					if ( get_p2p_ie( frame_body + _PUBLIC_ACTION_IE_OFFSET_, len - _PUBLIC_ACTION_IE_OFFSET_, p2p_ie, &p2p_ielen ) )
					{
						u8	attr_content = 0x00, operatingch_info[5] = { 0x00 };
						u8	groupid[ 38 ] = { 0x00 };
						u32	attr_contentlen = 0;

						get_p2p_attr_content(p2p_ie, p2p_ielen, P2P_ATTR_STATUS, &attr_content, &attr_contentlen);
						if ( attr_contentlen == 1 )
						{
							printk( "[%s] Status = %d\n", __FUNCTION__, attr_content );
							if ( attr_content == P2P_STATUS_SUCCESS )
							{
								//	Commented by Albert 20100911
								//	Todo: Need to handle the case which both Intents are the same.
								pwdinfo->p2p_state = P2P_STATE_GONEGO_OK;
								if ( ( pwdinfo->intent ) > ( pwdinfo->peer_intent >> 1 ) )
								{
									pwdinfo->role = P2P_ROLE_GO;
								}
								else if ( ( pwdinfo->intent ) < ( pwdinfo->peer_intent >> 1 ) )
								{
									pwdinfo->role = P2P_ROLE_CLIENT;
								}
								else
								{
									//	Have to compare the Tie Breaker
									if ( pwdinfo->peer_intent & 0x01 )
									{
										pwdinfo->role = P2P_ROLE_CLIENT;
									}
									else
									{
										pwdinfo->role = P2P_ROLE_GO;
									}
								}								
								get_p2p_attr_content( p2p_ie, p2p_ielen, P2P_ATTR_GROUP_ID, groupid, &attr_contentlen);
								printk( "[%s] Ssid = %s, ssidlen = %d\n", __FUNCTION__, &groupid[ETH_ALEN], strlen(&groupid[ETH_ALEN]) );

								attr_contentlen = 0;
								get_p2p_attr_content(p2p_ie, p2p_ielen, P2P_ATTR_OPERATING_CH, operatingch_info, &attr_contentlen);
								printk( "[%s] Peer's operating channel = %d\n", __FUNCTION__, operatingch_info[4] );
								pwdinfo->peer_operating_ch = operatingch_info[4];
							}
							else
							{
								pwdinfo->role = P2P_ROLE_DEVICE;
								pwdinfo->p2p_state = P2P_STATE_GONEGO_FAIL;
							}
						}
					}
					break;
				}
				case P2P_INVIT_REQ:
				{
					//	Added by Albert 2010/10/05
					//	Received the P2P Invite Request frame.				
					
					printk( "[%s] Got invite request frame!\n", __FUNCTION__ );
					if ( get_p2p_ie( frame_body + _PUBLIC_ACTION_IE_OFFSET_, len - _PUBLIC_ACTION_IE_OFFSET_, p2p_ie, &p2p_ielen ) )
					{
						//	Parse the necessary information from the P2P Invitation Request frame.
						//	For example: The MAC address of sending this P2P Invitation Request frame.
						u8	groupid[ 38 ] = { 0x00 };
						u32	attr_contentlen = 0;
						u8	match_result = 0;						

						get_p2p_attr_content( p2p_ie, p2p_ielen, P2P_ATTR_GROUP_ID, groupid, &attr_contentlen);
						_memcpy( pwdinfo->p2p_peer_interface_addr, groupid, ETH_ALEN );
						pwdinfo->p2p_state = P2P_STATE_RECV_INVITE_REQ;
						printk( "[%s] peer address %.2X %.2X %.2X %.2X %.2X %.2X\n", __FUNCTION__,
								groupid[0], groupid[1], groupid[2], groupid[3], groupid[4], groupid[5] );

						if ( is_matched_in_profilelist( pwdinfo->p2p_peer_interface_addr, &pwdinfo->profileinfo[ 0 ] ) )
						{
							match_result = 1;
						}
						else
						{
							match_result = 0;
						}

						printk( "[%s] match_result = %d\n", __FUNCTION__, match_result );
						
						pwdinfo->inviteresp_info.token = frame_body[ 7 ];
						issue_p2p_invitation_response( padapter, pwdinfo->p2p_peer_interface_addr, pwdinfo->inviteresp_info.token, match_result );
					}

					break;
				}
				case P2P_INVIT_RESP:
				{
					u8	attr_content = 0x00;
					u32	attr_contentlen = 0;
					
					printk( "[%s] Got invite response frame!\n", __FUNCTION__ );
					if ( get_p2p_ie( frame_body + _PUBLIC_ACTION_IE_OFFSET_, len - _PUBLIC_ACTION_IE_OFFSET_, p2p_ie, &p2p_ielen ) )
					{
						get_p2p_attr_content(p2p_ie, p2p_ielen, P2P_ATTR_STATUS, &attr_content, &attr_contentlen);
					
						if ( attr_contentlen == 1 )
						{
							printk( "[%s] Status = %d\n", __FUNCTION__, attr_content );
							if ( attr_content == P2P_STATUS_SUCCESS )
							{
								pwdinfo->role = P2P_ROLE_CLIENT;
							}
							else
							{
								pwdinfo->role = P2P_ROLE_DEVICE;
							}
						}
						else
						{
								pwdinfo->role = P2P_ROLE_DEVICE;
						}
					}
					break;
				}
				case P2P_DEVDISC_REQ:
				{
					process_p2p_devdisc_req(pwdinfo, pframe, len);
					
					break;
				}
				case P2P_DEVDISC_RESP:
				{
					process_p2p_devdisc_resp(pwdinfo, pframe, len);
					
					break;
				}
				case P2P_PROVISION_DISC_REQ:
				{
					printk( "[%s] Got Provisioning Discovery Request Frame\n", __FUNCTION__ );
					pwdinfo->p2p_state = P2P_STATE_PROVISION_DIS;
					issue_p2p_provision_response( padapter, GetAddr2Ptr(pframe), frame_body);
					break;
				}
				case P2P_PROVISION_DISC_RESP:
				{

					break;
				}				
			}

		}		
	}
	
	return _SUCCESS;
}

unsigned int OnAction_ht(_adapter *padapter, union recv_frame *precv_frame)
{
	return _SUCCESS;
}

unsigned int OnAction_wmm(_adapter *padapter, union recv_frame *precv_frame)
{
	return _SUCCESS;
}

unsigned int OnAction(_adapter *padapter, union recv_frame *precv_frame)
{
	int i;
	unsigned char	category;
	struct action_handler *ptable;
	unsigned char	*frame_body;
	u8 *pframe = precv_frame->u.hdr.rx_data; 

	frame_body = (unsigned char *)(pframe + sizeof(struct ieee80211_hdr_3addr));
	
	category = frame_body[0];
	
	for(i = 0; i < sizeof(OnAction_tbl)/sizeof(struct action_handler); i++)	
	{
		ptable = &OnAction_tbl[i];
		
		if(category == ptable->num)
			ptable->func(padapter, precv_frame);
	
	}

	return _SUCCESS;

}

unsigned int DoReserved(_adapter *padapter, union recv_frame *precv_frame)
{
	u8 *pframe = precv_frame->u.hdr.rx_data;
	uint len = precv_frame->u.hdr.len;

	//DBG_871X("rcvd mgt frame(%x, %x)\n", (GetFrameSubType(pframe) >> 4), *(unsigned int *)GetAddr1Ptr(pframe));
	return _SUCCESS;
}

struct xmit_frame *alloc_mgtxmitframe(struct xmit_priv *pxmitpriv)
{
	struct xmit_frame			*pmgntframe;
	struct xmit_buf				*pxmitbuf;

	if ((pmgntframe = alloc_xmitframe(pxmitpriv)) == NULL)
	{
		return NULL;
	}

	if ((pxmitbuf = alloc_xmitbuf(pxmitpriv))	== NULL)
	{
		//dynamic allocate xmitbuf for mgt frame, if driver do not have xmitbuf.
		if((pxmitbuf = alloc_xmitbuf_dynamic(pxmitpriv, 2048)) == NULL)
		{
			free_xmitframe_ex(pxmitpriv, pmgntframe);
			return NULL;
		}
	}

	pmgntframe->frame_tag = MGNT_FRAMETAG;

	pmgntframe->pxmitbuf = pxmitbuf;

	pmgntframe->buf_addr = pxmitbuf->pbuf;

	pxmitbuf->priv_data = pmgntframe;

	return pmgntframe;

}


/****************************************************************************

Following are some TX fuctions for WiFi MLME

*****************************************************************************/

void update_mgntframe_attrib(_adapter *padapter, struct pkt_attrib *pattrib)
{
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);

	_memset((u8 *)(pattrib), 0, sizeof(struct pkt_attrib));

	pattrib->hdrlen = 24;
	pattrib->nr_frags = 1;
	pattrib->priority = 7;
	pattrib->mac_id = 0;
	pattrib->qsel = 0x12;

	pattrib->pktlen = 0;

	pattrib->raid = 6;//b mode

	pattrib->encrypt = _NO_PRIVACY_;
	pattrib->bswenc = _FALSE;	

	pattrib->qos_en = _FALSE;
	pattrib->ht_en = _FALSE;
	pattrib->bwmode = HT_CHANNEL_WIDTH_20;
	pattrib->ch_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
	pattrib->sgi = _FALSE;

	pattrib->seqnum = pmlmeext->mgnt_seq;

}

void dump_mgntframe(_adapter *padapter, struct xmit_frame *pmgntframe)
{
	padapter->HalFunc.mgnt_xmit(padapter, pmgntframe);
}

void issue_beacon(_adapter *padapter)
{
	struct xmit_frame	*pmgntframe;
	struct pkt_attrib	*pattrib;
	unsigned char	*pframe;
	struct ieee80211_hdr *pwlanhdr;
	unsigned short *fctrl;
	unsigned int	rate_len;
	struct xmit_priv	*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX 		*cur_network = &(pmlmeinfo->network);
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);
	u8	bc_addr[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};


	//DBG_871X("%s\n", __FUNCTION__);

	if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
	{
		return;
	}

	//update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);
	pattrib->qsel = 0x10;
	
	_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);
		
	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct ieee80211_hdr *)pframe;	
	
	
	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;
	
	_memcpy(pwlanhdr->addr1, bc_addr, ETH_ALEN);
	_memcpy(pwlanhdr->addr2, myid(&(padapter->eeprompriv)), ETH_ALEN);
	_memcpy(pwlanhdr->addr3, get_my_bssid(cur_network), ETH_ALEN);

	SetSeqNum(pwlanhdr, 0/*pmlmeext->mgnt_seq*/);
	//pmlmeext->mgnt_seq++;
	SetFrameSubType(pframe, WIFI_BEACON);
	
	pframe += sizeof(struct ieee80211_hdr_3addr);	
	pattrib->pktlen = sizeof (struct ieee80211_hdr_3addr);
	
	//timestamp will be inserted by hardware
	pframe += 8;
	pattrib->pktlen += 8;

	// beacon interval: 2 bytes
	_memcpy(pframe, (unsigned char *)(get_beacon_interval_from_ie(cur_network->IEs)), 2); 
	pframe += 2;
	pattrib->pktlen += 2;

	// capability info: 2 bytes
	_memcpy(pframe, (unsigned char *)(get_capability_from_ie(cur_network->IEs)), 2);
	pframe += 2;
	pattrib->pktlen += 2;


	if( (pmlmeinfo->state&0x03) == WIFI_FW_AP_STATE)
	{
		//DBG_871X("ie len=%d\n", cur_network->IELength);
		pattrib->pktlen += cur_network->IELength - sizeof(NDIS_802_11_FIXED_IEs);
		_memcpy(pframe, cur_network->IEs+sizeof(NDIS_802_11_FIXED_IEs), pattrib->pktlen);
		
		pframe += pattrib->pktlen;

		if(pwdinfo->role == P2P_ROLE_GO)
		{
			u32 len;
		
			len = build_beacon_p2p_ie(pwdinfo, pframe);
		
			pframe += len;
			pattrib->pktlen += len;
		}
		
		goto _issue_bcn;
	}

	//below for ad-hoc mode

	// SSID
	pframe = set_ie(pframe, _SSID_IE_, cur_network->Ssid.SsidLength, cur_network->Ssid.Ssid, &pattrib->pktlen);

	// supported rates...
	rate_len = get_rateset_len(cur_network->SupportedRates);
	pframe = set_ie(pframe, _SUPPORTEDRATES_IE_, ((rate_len > 8)? 8: rate_len), cur_network->SupportedRates, &pattrib->pktlen);

	// DS parameter set
	pframe = set_ie(pframe, _DSSET_IE_, 1, (unsigned char *)&(cur_network->Configuration.DSConfig), &pattrib->pktlen);

	if( (pmlmeinfo->state&0x03) == WIFI_FW_ADHOC_STATE)
	{
		u32 ATIMWindow;
		// IBSS Parameter Set...
		//ATIMWindow = cur->Configuration.ATIMWindow;
		ATIMWindow = 0;
		pframe = set_ie(pframe, _IBSS_PARA_IE_, 2, (unsigned char *)(&ATIMWindow), &pattrib->pktlen);
	}	


	//todo: ERP IE
	
	
	// EXTERNDED SUPPORTED RATE
	if (rate_len > 8)
	{
		pframe = set_ie(pframe, _EXT_SUPPORTEDRATES_IE_, (rate_len - 8), (cur_network->SupportedRates + 8), &pattrib->pktlen);
	}


	//todo:HT for adhoc

_issue_bcn:

	if ((pattrib->pktlen + TXDESC_SIZE) > 512)
	{
		DBG_871X("beacon frame too large\n");
		return;
	}
	
	pattrib->last_txcmdsz = pattrib->pktlen;

	//DBG_871X("issue bcn_sz=%d\n", pattrib->last_txcmdsz);

	dump_mgntframe(padapter, pmgntframe);

}

void issue_probersp_p2p(_adapter *padapter, unsigned char *da)
{
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct ieee80211_hdr	*pwlanhdr;
	unsigned short				*fctrl;	
	unsigned char					*mac;
	struct xmit_priv	*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	//WLAN_BSSID_EX 		*cur_network = &(pmlmeinfo->network);
	u16					beacon_interval = 100;
	u16					capInfo = 0;
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);
	u8					wpsie[255] = { 0x00 };
	u32					wpsielen = 0, p2pielen = 0;
	
	
	DBG_871X("%s\n", __FUNCTION__);
	
	if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
	{
		return;
	}
	
	//update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);	
	
	_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);
		
	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct ieee80211_hdr *)pframe;	
	
	mac = myid(&(padapter->eeprompriv));
	
	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;
	_memcpy(pwlanhdr->addr1, da, ETH_ALEN);
	_memcpy(pwlanhdr->addr2, mac, ETH_ALEN);
	
	//	Use the device address for BSSID field.	
	_memcpy(pwlanhdr->addr3, mac, ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	SetFrameSubType(fctrl, WIFI_PROBERSP);
	
	pattrib->hdrlen = sizeof(struct ieee80211_hdr_3addr);
	pattrib->pktlen = pattrib->hdrlen;
	pframe += pattrib->hdrlen;

	//timestamp will be inserted by hardware
	pframe += 8;
	pattrib->pktlen += 8;

	// beacon interval: 2 bytes
	_memcpy(pframe, (unsigned char *) &beacon_interval, 2); 
	pframe += 2;
	pattrib->pktlen += 2;

	//	capability info: 2 bytes
	//	ESS and IBSS bits must be 0 (defined in the 3.1.2.1.1 of WiFi Direct Spec)
	capInfo |= cap_ShortPremble;
	capInfo |= cap_ShortSlot;
	
	_memcpy(pframe, (unsigned char *) &capInfo, 2);
	pframe += 2;
	pattrib->pktlen += 2;


	// SSID
	pframe = set_ie(pframe, _SSID_IE_, 7, pwdinfo->p2p_wildcard_ssid, &pattrib->pktlen);

	// supported rates...
	//	Use the OFDM rate in the P2P probe response frame. ( 6(B), 9(B), 12(B), 24(B), 36, 48, 54 )
	pframe = set_ie(pframe, _SUPPORTEDRATES_IE_, 7, pwdinfo->support_rate, &pattrib->pktlen);

	// DS parameter set
	pframe = set_ie(pframe, _DSSET_IE_, 1, (unsigned char *)&pwdinfo->listen_channel, &pattrib->pktlen);

	//	Todo: WPS IE
	//	Noted by Albert 20100907
	//	According to the WPS specification, all the WPS attribute is presented by Big Endian.

	wpsielen = 0;
	//	WPS OUI
	*(u32*) ( wpsie ) = cpu_to_be32( WPSOUI );
	wpsielen += 4;

	//	WPS version
	//	Type:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_ATTR_VER1 );
	wpsielen += 2;

	//	Length:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( 0x0001 );
	wpsielen += 2;

	//	Value:
	wpsie[wpsielen++] = WPS_VERSION_1;	//	Version 1.0

	//	WiFi Simple Config State
	//	Type:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_ATTR_SIMPLE_CONF_STATE );
	wpsielen += 2;

	//	Length:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( 0x0001 );
	wpsielen += 2;

	//	Value:
	wpsie[wpsielen++] = WPS_WSC_STATE_NOT_CONFIG;	//	Not Configured.

	//	Response Type
	//	Type:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_ATTR_RESP_TYPE );
	wpsielen += 2;

	//	Length:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( 0x0001 );
	wpsielen += 2;

	//	Value:
	wpsie[wpsielen++] = WPS_RESPONSE_TYPE_8021X;

	//	UUID-E
	//	Type:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_ATTR_UUID_E );
	wpsielen += 2;

	//	Length:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( 0x0010 );
	wpsielen += 2;

	//	Value:
	_memcpy( wpsie + wpsielen, myid( &padapter->eeprompriv ), ETH_ALEN );
	wpsielen += 0x10;

	//	Manufacturer
	//	Type:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_ATTR_MANUFACTURER );
	wpsielen += 2;

	//	Length:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( 0x0007 );
	wpsielen += 2;

	//	Value:
	_memcpy( wpsie + wpsielen, "Realtek", 7 );
	wpsielen += 7;

	//	Model Name
	//	Type:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_ATTR_MODEL_NAME );
	wpsielen += 2;

	//	Length:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( 0x0006 );
	wpsielen += 2;	

	//	Value:
	_memcpy( wpsie + wpsielen, "8192CU", 6 );
	wpsielen += 6;

	//	Model Number
	//	Type:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_ATTR_MODEL_NUMBER );
	wpsielen += 2;

	//	Length:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( 0x0001 );
	wpsielen += 2;

	//	Value:
	wpsie[ wpsielen++ ] = 0x31;		//	character 1

	//	Serial Number
	//	Type:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_ATTR_SERIAL_NUMBER );
	wpsielen += 2;

	//	Length:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( ETH_ALEN );
	wpsielen += 2;

	//	Value:
	_memcpy( wpsie + wpsielen, "123456" , ETH_ALEN );
	wpsielen += ETH_ALEN;

	//	Primary Device Type
	//	Type:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_ATTR_PRIMARY_DEV_TYPE );
	wpsielen += 2;

	//	Length:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( 0x0008 );
	wpsielen += 2;

	//	Value:
	//	Category ID
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_PDT_CID_RTK_WIDI );
	wpsielen += 2;

	//	OUI
	*(u32*) ( wpsie + wpsielen ) = cpu_to_be32( WPSOUI );
	wpsielen += 4;

	//	Sub Category ID
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_PDT_SCID_RTK_DMP );
	wpsielen += 2;

	//	Device Name
	//	Type:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_ATTR_DEVICE_NAME );
	wpsielen += 2;

	//	Length:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( pwdinfo->device_name_len );
	wpsielen += 2;

	//	Value:
	_memcpy( wpsie + wpsielen, pwdinfo->device_name, pwdinfo->device_name_len );
	wpsielen += pwdinfo->device_name_len;

	//	Config Method
	//	Type:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_ATTR_CONF_METHOD );
	wpsielen += 2;

	//	Length:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( 0x0002 );
	wpsielen += 2;

	//	Value:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_CONFIG_METHOD_DISPLAY );
	wpsielen += 2;
	

	pframe = set_ie(pframe, _VENDOR_SPECIFIC_IE_, wpsielen, (unsigned char *) wpsie, &pattrib->pktlen );
	

	p2pielen = build_probe_resp_p2p_ie(pwdinfo, pframe);
	pframe += p2pielen;
	pattrib->pktlen += p2pielen;
	

	pattrib->last_txcmdsz = pattrib->pktlen;
	

	dump_mgntframe(padapter, pmgntframe);
	
	return;

}


void issue_probersp(_adapter *padapter, unsigned char *da, u8 is_valid_p2p_probereq)
{
	u8 *pwps_ie;
	uint wps_ielen;
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct ieee80211_hdr	*pwlanhdr;
	unsigned short				*fctrl;	
	unsigned char					*mac, *bssid;
	struct xmit_priv	*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX 		*cur_network = &(pmlmeinfo->network);
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);
	
	//DBG_871X("%s\n", __FUNCTION__);
	
	if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
	{
		return;
	}
	

	//update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);	
	
	_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);
		
	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct ieee80211_hdr *)pframe;	
	
	mac = myid(&(padapter->eeprompriv));
	bssid = cur_network->MacAddress;
	
	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;
	_memcpy(pwlanhdr->addr1, da, ETH_ALEN);
	_memcpy(pwlanhdr->addr2, mac, ETH_ALEN);
	_memcpy(pwlanhdr->addr3, bssid, ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	SetFrameSubType(fctrl, WIFI_PROBERSP);
	
	pattrib->hdrlen = sizeof(struct ieee80211_hdr_3addr);
	pattrib->pktlen = pattrib->hdrlen;
	pframe += pattrib->hdrlen;


	if(cur_network->IELength>MAX_IE_SZ)
		return;

#if defined (CONFIG_AP_MODE) && defined (CONFIG_NATIVEAP_MLME)
	pwps_ie = get_wps_ie(cur_network->IEs, cur_network->IELength, NULL, &wps_ielen);
	
	//inerset & update wps_probe_resp_ie
	if((pmlmepriv->wps_probe_resp_ie!=NULL) && pwps_ie && (wps_ielen>0))
	{
		uint wps_offset, remainder_ielen;
		u8 *premainder_ie, *pbackup_remainder_ie=NULL;	
		
		wps_offset = (uint)(pwps_ie - cur_network->IEs);

		premainder_ie = pwps_ie + wps_ielen;

		remainder_ielen = cur_network->IELength - wps_offset - wps_ielen;

		_memcpy(pframe, cur_network->IEs, wps_offset);		
		pframe += wps_offset;		
		pattrib->pktlen += wps_offset;		

		wps_ielen = (uint)pmlmepriv->wps_probe_resp_ie[1];//to get ie data len
		if((wps_offset+wps_ielen+2)<=MAX_IE_SZ)
		{
			_memcpy(pframe, pmlmepriv->wps_probe_resp_ie, wps_ielen+2);
			pframe += wps_ielen+2;		
			pattrib->pktlen += wps_ielen+2;	
		}

		if((wps_offset+wps_ielen+2+remainder_ielen)<=MAX_IE_SZ)
		{
			_memcpy(pframe, premainder_ie, remainder_ielen);
			pframe += remainder_ielen;		
			pattrib->pktlen += remainder_ielen;	
		}
	}
	else
#endif		
	{
		_memcpy(pframe, cur_network->IEs, cur_network->IELength);
		pframe += cur_network->IELength;
		pattrib->pktlen += cur_network->IELength;
	}	
	

	if(pwdinfo->role == P2P_ROLE_GO && is_valid_p2p_probereq)
	{
		u32 len;
		
		len = build_probe_resp_p2p_ie(pwdinfo, pframe);
		
		pframe += len;
		pattrib->pktlen += len;
	}
	

	pattrib->last_txcmdsz = pattrib->pktlen;
	

	dump_mgntframe(padapter, pmgntframe);
	
	return;

}

void issue_probereq_p2p(_adapter *padapter)
{
	struct xmit_frame		*pmgntframe;
	struct pkt_attrib		*pattrib;
	unsigned char			*pframe;
	struct ieee80211_hdr	*pwlanhdr;
	unsigned short		*fctrl;
	unsigned char			*mac;
	unsigned char			bssrate[NumRates];
	struct xmit_priv		*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	int	bssrate_len = 0;
	u8	bc_addr[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);	
	u8					wpsie[255] = { 0x00 }, p2pie[ 255 ] = { 0x00 };
	u16					wpsielen = 0, p2pielen = 0;

	if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
	{
		return;
	}

	//update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);


	_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct ieee80211_hdr *)pframe;

	mac = myid(&(padapter->eeprompriv));

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	//	broadcast probe request frame
	_memcpy(pwlanhdr->addr1, bc_addr, ETH_ALEN);
	_memcpy(pwlanhdr->addr3, bc_addr, ETH_ALEN);
	_memcpy(pwlanhdr->addr2, mac, ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	SetFrameSubType(pframe, WIFI_PROBEREQ);

	pframe += sizeof (struct ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof (struct ieee80211_hdr_3addr);

	pframe = set_ie(pframe, _SSID_IE_, P2P_WILDCARD_SSID_LEN, pwdinfo->p2p_wildcard_ssid, &(pattrib->pktlen));

	get_rate_set(padapter, bssrate, &bssrate_len);

	if (bssrate_len > 8)
	{
		pframe = set_ie(pframe, _SUPPORTEDRATES_IE_ , 8, bssrate, &(pattrib->pktlen));
		pframe = set_ie(pframe, _EXT_SUPPORTEDRATES_IE_ , (bssrate_len - 8), (bssrate + 8), &(pattrib->pktlen));
	}
	else
	{
		pframe = set_ie(pframe, _SUPPORTEDRATES_IE_ , bssrate_len , bssrate, &(pattrib->pktlen));
	}

	//	WPS IE
	//	Noted by Albert 20110221
	//	According to the WPS specification, all the WPS attribute is presented by Big Endian.

	wpsielen = 0;
	//	WPS OUI
	*(u32*) ( wpsie ) = cpu_to_be32( WPSOUI );
	wpsielen += 4;

	//	WPS version
	//	Type:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_ATTR_VER1 );
	wpsielen += 2;

	//	Length:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( 0x0001 );
	wpsielen += 2;

	//	Value:
	wpsie[wpsielen++] = WPS_VERSION_1;	//	Version 1.0

	//	Device Name
	//	Type:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_ATTR_DEVICE_NAME );
	wpsielen += 2;

	//	Length:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( pwdinfo->device_name_len );
	wpsielen += 2;

	//	Value:
	_memcpy( wpsie + wpsielen, pwdinfo->device_name, pwdinfo->device_name_len );
	wpsielen += pwdinfo->device_name_len;

	//	Primary Device Type
	//	Type:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_ATTR_PRIMARY_DEV_TYPE );
	wpsielen += 2;

	//	Length:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( 0x0008 );
	wpsielen += 2;

	//	Value:
	//	Category ID
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_PDT_CID_RTK_WIDI );
	wpsielen += 2;

	//	OUI
	*(u32*) ( wpsie + wpsielen ) = cpu_to_be32( WPSOUI );
	wpsielen += 4;

	//	Sub Category ID
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_PDT_SCID_RTK_DMP );
	wpsielen += 2;	

	pframe = set_ie(pframe, _VENDOR_SPECIFIC_IE_, wpsielen, (unsigned char *) wpsie, &pattrib->pktlen );
	
	//	P2P OUI
	p2pielen = 0;
	p2pie[ p2pielen++ ] = 0x50;
	p2pie[ p2pielen++ ] = 0x6F;
	p2pie[ p2pielen++ ] = 0x9A;
	p2pie[ p2pielen++ ] = 0x09;	//	WFA P2P v1.0

	//	Commented by Albert 20110221
	//	According to the P2P Specification, the probe request frame should contain 5 P2P attributes
	//	1. P2P Capability
	//	2. P2P Device ID if this probe request wants to find the specific P2P device
	//	3. Listen Channel
	//	4. Extended Listen Timing
	//	5. Operating Channel if this WiFi is working as the group owner now

	//	P2P Capability
	//	Type:
	p2pie[ p2pielen++ ] = P2P_ATTR_CAPABILITY;

	//	Length:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0x0002 );
	p2pielen += 2;

	//	Value:
	//	Device Capability Bitmap, 1 byte
	//	Be able to participate in additional P2P Groups and
	//	support the P2P Invitation Procedure
	p2pie[ p2pielen++ ] = P2P_DEVCAP_INVITATION_PROC;
	
	//	Group Capability Bitmap, 1 byte
	p2pie[ p2pielen++ ] = 0x00;

	//	Listen Channel
	//	Type:
	p2pie[ p2pielen++ ] = P2P_ATTR_LISTEN_CH;

	//	Length:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0x0005 );
	p2pielen += 2;

	//	Value:
	//	Country String
	p2pie[ p2pielen++ ] = 'U';
	p2pie[ p2pielen++ ] = 'S';
	
	//	The third byte should be set to 0x04.
	//	Described in the "Operating Channel Attribute" section.
	p2pie[ p2pielen++ ] = 0x04;

	//	Operating Class
	p2pie[ p2pielen++ ] = 0x51;	//	Copy from SD7
	
	//	Channel Number
	p2pie[ p2pielen++ ] = pwdinfo->listen_channel;	//	listen channel
	

	//	Extended Listen Timing
	//	Type:
	p2pie[ p2pielen++ ] = P2P_ATTR_EX_LISTEN_TIMING;

	//	Length:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0x0004 );
	p2pielen += 2;

	//	Value:
	//	Availability Period
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0xFFFF );
	p2pielen += 2;

	//	Availability Interval
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0xFFFF );
	p2pielen += 2;

	if ( pwdinfo->role == P2P_ROLE_GO )
	{
		//	Operating Channel (if this WiFi is working as the group owner now)
		//	Type:
		p2pie[ p2pielen++ ] = P2P_ATTR_OPERATING_CH;

		//	Length:
		*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0x0005 );
		p2pielen += 2;

		//	Value:
		//	Country String
		p2pie[ p2pielen++ ] = 'U';
		p2pie[ p2pielen++ ] = 'S';
	
		//	The third byte should be set to 0x04.
		//	Described in the "Operating Channel Attribute" section.
		p2pie[ p2pielen++ ] = 0x04;

		//	Operating Class
		p2pie[ p2pielen++ ] = 0x51;	//	Copy from SD7
	
		//	Channel Number
		p2pie[ p2pielen++ ] = 11;	//	operating channel number
		
	}
	
	pframe = set_ie(pframe, _VENDOR_SPECIFIC_IE_, p2pielen, (unsigned char *) p2pie, &pattrib->pktlen );	

	pattrib->last_txcmdsz = pattrib->pktlen;

	RT_TRACE(_module_rtl871x_mlme_c_,_drv_err_,("issuing probe_req, tx_len=%d\n", pattrib->last_txcmdsz));

	dump_mgntframe(padapter, pmgntframe);

	return;
}

void issue_probereq(_adapter *padapter, u8 blnbc)
{
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct ieee80211_hdr	*pwlanhdr;
	unsigned short				*fctrl;
	unsigned char					*mac;
	unsigned char					bssrate[NumRates];
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	int	bssrate_len = 0;
	u8	bc_addr[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	RT_TRACE(_module_rtl871x_mlme_c_,_drv_err_,("+issue_probereq\n"));

	if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
	{
		return;
	}

	//update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);


	_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct ieee80211_hdr *)pframe;

	mac = myid(&(padapter->eeprompriv));

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;
	
	if ( 0 == blnbc )
	{
		//	unicast probe request frame
		_memcpy(pwlanhdr->addr1, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);
		_memcpy(pwlanhdr->addr3, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);
	}
	else
	{
		//	broadcast probe request frame
	_memcpy(pwlanhdr->addr1, bc_addr, ETH_ALEN);
	_memcpy(pwlanhdr->addr3, bc_addr, ETH_ALEN);
	}

	_memcpy(pwlanhdr->addr2, mac, ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	SetFrameSubType(pframe, WIFI_PROBEREQ);

	pframe += sizeof (struct ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof (struct ieee80211_hdr_3addr);

	pframe = set_ie(pframe, _SSID_IE_, pmlmeext->sitesurvey_res.ss_ssidlen, pmlmeext->sitesurvey_res.ss_ssid, &(pattrib->pktlen));

	get_rate_set(padapter, bssrate, &bssrate_len);

	if (bssrate_len > 8)
	{
		pframe = set_ie(pframe, _SUPPORTEDRATES_IE_ , 8, bssrate, &(pattrib->pktlen));
		pframe = set_ie(pframe, _EXT_SUPPORTEDRATES_IE_ , (bssrate_len - 8), (bssrate + 8), &(pattrib->pktlen));
	}
	else
	{
		pframe = set_ie(pframe, _SUPPORTEDRATES_IE_ , bssrate_len , bssrate, &(pattrib->pktlen));
	}

	pattrib->last_txcmdsz = pattrib->pktlen;

	RT_TRACE(_module_rtl871x_mlme_c_,_drv_err_,("issuing probe_req, tx_len=%d\n", pattrib->last_txcmdsz));

	dump_mgntframe(padapter, pmgntframe);

	return;
}

// if psta == NULL, indiate we are station(client) now...
void issue_auth(_adapter *padapter, struct sta_info *psta, unsigned short status)
{
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct ieee80211_hdr	*pwlanhdr;
	unsigned short				*fctrl;
	unsigned int					val32;
	unsigned short				val16;
	int use_shared_key = 0;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
	{
		return;
	}

	//update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	SetFrameSubType(pframe, WIFI_AUTH);

	pframe += sizeof(struct ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct ieee80211_hdr_3addr);


	if(psta)// for AP mode
	{
#ifdef CONFIG_NATIVEAP_MLME

		_memcpy(pwlanhdr->addr1, psta->hwaddr, ETH_ALEN);		
		_memcpy(pwlanhdr->addr2, myid(&(padapter->eeprompriv)), ETH_ALEN);
		_memcpy(pwlanhdr->addr3, myid(&(padapter->eeprompriv)), ETH_ALEN);


		// setting auth algo number	
		val16 = (u16)psta->authalg;
		
		if(status != _STATS_SUCCESSFUL_)
			val16 = 0;

		if (val16)	{
			val16 = cpu_to_le16(val16);	
			use_shared_key = 1;
		}
		
		pframe = set_fixed_ie(pframe, _AUTH_ALGM_NUM_, (unsigned char *)&val16, &(pattrib->pktlen));
		
		// setting auth seq number
		val16 =(u16)psta->auth_seq;
		val16 = cpu_to_le16(val16);	
		pframe = set_fixed_ie(pframe, _AUTH_SEQ_NUM_, (unsigned char *)&val16, &(pattrib->pktlen));

		// setting status code...
		val16 = status;
		val16 = cpu_to_le16(val16);	
		pframe = set_fixed_ie(pframe, _STATUS_CODE_, (unsigned char *)&val16, &(pattrib->pktlen));

		// added challenging text...
		if ((psta->auth_seq == 2) && (psta->state & WIFI_FW_AUTH_STATE) && (use_shared_key==1))
		{
			pframe = set_ie(pframe, _CHLGETXT_IE_, 128, psta->chg_txt, &(pattrib->pktlen));			
		}
#endif
	}
	else
	{		
		_memcpy(pwlanhdr->addr1, get_my_bssid(&pmlmeinfo->network), ETH_ALEN);
		_memcpy(pwlanhdr->addr2, myid(&padapter->eeprompriv), ETH_ALEN);
		_memcpy(pwlanhdr->addr3, get_my_bssid(&pmlmeinfo->network), ETH_ALEN);
	
		// setting auth algo number		
		val16 = (pmlmeinfo->auth_algo == dot11AuthAlgrthm_Shared)? 1: 0;// 0:OPEN System, 1:Shared key
		if (val16)	{
			val16 = cpu_to_le16(val16);	
			use_shared_key = 1;
		}	
		//printk("%s auth_algo= %s auth_seq=%d\n",__FUNCTION__,(pmlmeinfo->auth_algo==0)?"OPEN":"SHARED",pmlmeinfo->auth_seq);
		
		//setting IV for auth seq #3
		if ((pmlmeinfo->auth_seq == 3) && (pmlmeinfo->state & WIFI_FW_AUTH_STATE) && (use_shared_key==1))
		{
			//printk("==> iv(%d),key_index(%d)\n",pmlmeinfo->iv,pmlmeinfo->key_index);
			val32 = ((pmlmeinfo->iv++) | (pmlmeinfo->key_index << 30));
			val32 = cpu_to_le32(val32);
			pframe = set_fixed_ie(pframe, 4, (unsigned char *)&val32, &(pattrib->pktlen));

			pattrib->iv_len = 4;
		}
	
		pframe = set_fixed_ie(pframe, _AUTH_ALGM_NUM_, (unsigned char *)&val16, &(pattrib->pktlen));
		
		// setting auth seq number
		val16 = pmlmeinfo->auth_seq;
		val16 = cpu_to_le16(val16);	
		pframe = set_fixed_ie(pframe, _AUTH_SEQ_NUM_, (unsigned char *)&val16, &(pattrib->pktlen));

		
		// setting status code...
		val16 = status;
		val16 = cpu_to_le16(val16);	
		pframe = set_fixed_ie(pframe, _STATUS_CODE_, (unsigned char *)&val16, &(pattrib->pktlen));

		// then checking to see if sending challenging text...
		if ((pmlmeinfo->auth_seq == 3) && (pmlmeinfo->state & WIFI_FW_AUTH_STATE) && (use_shared_key==1))
		{
			pframe = set_ie(pframe, _CHLGETXT_IE_, 128, pmlmeinfo->chg_txt, &(pattrib->pktlen));

			SetPrivacy(fctrl);
			
			pattrib->hdrlen = sizeof(struct ieee80211_hdr_3addr);			
			
			pattrib->encrypt = _WEP40_;

			pattrib->icv_len = 4;
			
			pattrib->pktlen += pattrib->icv_len;			
			
		}
		
	}

	pattrib->last_txcmdsz = pattrib->pktlen;

	wep_encrypt(padapter, (u8 *)pmgntframe);

	dump_mgntframe(padapter, pmgntframe);

	return;
}


void issue_asocrsp(_adapter *padapter, unsigned short status, struct sta_info *pstat, int pkt_type)
{
#ifdef CONFIG_AP_MODE
	struct xmit_frame	*pmgntframe;
	struct ieee80211_hdr	*pwlanhdr;
	struct pkt_attrib *pattrib;
	unsigned char	*pbuf, *pframe;
	unsigned short val;		
	unsigned short *fctrl;
	struct xmit_priv *pxmitpriv = &(padapter->xmitpriv);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);	
	WLAN_BSSID_EX *pnetwork = &(pmlmeinfo->network);
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);
	u8 *ie = pnetwork->IEs; 

	DBG_871X("%s\n", __FUNCTION__);

	if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
	{
		return;
	}

	//update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);


	_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	_memcpy((void *)GetAddr1Ptr(pwlanhdr), pstat->hwaddr, ETH_ALEN);
	_memcpy((void *)GetAddr2Ptr(pwlanhdr), myid(&(padapter->eeprompriv)), ETH_ALEN);
	_memcpy((void *)GetAddr3Ptr(pwlanhdr), get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);


	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	if ((pkt_type == WIFI_ASSOCRSP) || (pkt_type == WIFI_REASSOCRSP))
		SetFrameSubType(pwlanhdr, pkt_type);		
	else
		return;

	pattrib->hdrlen = sizeof(struct ieee80211_hdr_3addr);
	pattrib->pktlen += pattrib->hdrlen;
	pframe += pattrib->hdrlen;

	//capability
	val = *(unsigned short *)get_capability_from_ie(ie);

	pframe = set_fixed_ie(pframe, _CAPABILITY_ , (unsigned char *)&val, &(pattrib->pktlen));

	status = cpu_to_le16(status);
	pframe = set_fixed_ie(pframe , _STATUS_CODE_ , (unsigned char *)&status, &(pattrib->pktlen));
	
	val = cpu_to_le16(pstat->aid | BIT(14) | BIT(15));
	pframe = set_fixed_ie(pframe, _ASOC_ID_ , (unsigned char *)&val, &(pattrib->pktlen));

	if (pstat->bssratelen <= 8)
	{
		pframe = set_ie(pframe, _SUPPORTEDRATES_IE_, pstat->bssratelen, pstat->bssrateset, &(pattrib->pktlen));
	}	
	else 
	{
		pframe = set_ie(pframe, _SUPPORTEDRATES_IE_, 8, pstat->bssrateset, &(pattrib->pktlen));
		pframe = set_ie(pframe, _EXT_SUPPORTEDRATES_IE_, (pstat->bssratelen-8), pstat->bssrateset+8, &(pattrib->pktlen));
	}

	//FILL WMM IE
	if ((pstat->flags & WLAN_STA_WME) && (pmlmepriv->qospriv.qos_option))
	{
		uint ie_len=0;
		unsigned char WMM_PARA_IE[] = {0x00, 0x50, 0xf2, 0x02, 0x01, 0x01};	
		
		for (pbuf = ie + _BEACON_IE_OFFSET_; ;pbuf+= (ie_len + 2))
		{			
			pbuf = get_ie(pbuf, _VENDOR_SPECIFIC_IE_, &ie_len, (pnetwork->IELength - _BEACON_IE_OFFSET_ - (ie_len + 2)));	
			if(pbuf && _memcmp(pbuf+2, WMM_PARA_IE, 6)) 
			{				
				_memcpy(pframe, pbuf, ie_len+2);
				pframe += (ie_len+2);
				pattrib->pktlen +=(ie_len+2);
				
				break;				
			}
			
			if ((pbuf == NULL) || (ie_len == 0))
			{
				break;
			}			
		}
		
	}

#ifdef CONFIG_80211N_HT
	if ((pstat->flags & WLAN_STA_HT) && (pmlmepriv->htpriv.ht_option))
	{
		uint ie_len=0;
		
		//FILL HT CAP INFO IE
		//p = hostapd_eid_ht_capabilities_info(hapd, p);
		pbuf = get_ie(ie + _BEACON_IE_OFFSET_, _HT_CAPABILITY_IE_, &ie_len, (pnetwork->IELength - _BEACON_IE_OFFSET_));
		if(pbuf && ie_len>0)
		{
			_memcpy(pframe, pbuf, ie_len+2);
			pframe += (ie_len+2);
			pattrib->pktlen +=(ie_len+2);
		}

		//FILL HT ADD INFO IE
		//p = hostapd_eid_ht_operation(hapd, p);
		pbuf = get_ie(ie + _BEACON_IE_OFFSET_, _HT_ADD_INFO_IE_, &ie_len, (pnetwork->IELength - _BEACON_IE_OFFSET_));
		if(pbuf && ie_len>0)
		{
			_memcpy(pframe, pbuf, ie_len+2);
			pframe += (ie_len+2);
			pattrib->pktlen +=(ie_len+2);
		}
		
	}	
#endif


	if (pmlmeinfo->assoc_AP_vendor == realtekAP)
	{
		pframe = set_ie(pframe, _VENDOR_SPECIFIC_IE_, 6 , REALTEK_96B_IE, &(pattrib->pktlen));
	}

	if(pwdinfo->role == P2P_ROLE_GO)
	{
		u32 len;
		
		len = build_assoc_resp_p2p_ie(pwdinfo, pframe, pstat->p2p_status_code);
		
		pframe += len;
		pattrib->pktlen += len;
	}

	pattrib->last_txcmdsz = pattrib->pktlen;
	
	dump_mgntframe(padapter, pmgntframe);
	
#endif
}

void issue_assocreq(_adapter *padapter)
{
	struct xmit_frame					*pmgntframe;
	struct pkt_attrib					*pattrib;
	unsigned char							*pframe, *p;
	struct ieee80211_hdr			*pwlanhdr;
	unsigned short						*fctrl;
	unsigned short						val16;
	unsigned int							i, ie_len;
	unsigned char					rf_type, bssrate[NumRates];
	PNDIS_802_11_VARIABLE_IEs	pIE;
	struct registry_priv	 *pregpriv = &padapter->registrypriv;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	int	bssrate_len = 0;
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);
	u8					p2pie[ 255 ] = { 0x00 };
	u16					p2pielen = 0;	

	if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
	{
		return;
	}

	//update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);


	_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;
	_memcpy(pwlanhdr->addr1, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);
	_memcpy(pwlanhdr->addr2, myid(&(padapter->eeprompriv)), ETH_ALEN);
	_memcpy(pwlanhdr->addr3, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	SetFrameSubType(pframe, WIFI_ASSOCREQ);

	pframe += sizeof(struct ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct ieee80211_hdr_3addr);

	//caps
	_memcpy(pframe, get_capability_from_ie(pmlmeinfo->network.IEs), 2);
	pframe += 2;
	pattrib->pktlen += 2;

	//listen interval
	//todo: listen interval for power saving
	val16 = cpu_to_le16(3);
	_memcpy(pframe ,(unsigned char *)&val16, 2);
	pframe += 2;
	pattrib->pktlen += 2;

	//SSID
	pframe = set_ie(pframe, _SSID_IE_,  pmlmeinfo->network.Ssid.SsidLength, pmlmeinfo->network.Ssid.Ssid, &(pattrib->pktlen));

	//supported rate & extended supported rate
#if 0
	get_rate_set(padapter, bssrate, &bssrate_len);
#else
	for (bssrate_len = 0; bssrate_len < NumRates; bssrate_len++) {
		if (pmlmeinfo->network.SupportedRates[bssrate_len] == 0) break;
		bssrate[bssrate_len] = pmlmeinfo->network.SupportedRates[bssrate_len];
	}
#endif

	if (bssrate_len > 8)
	{
		pframe = set_ie(pframe, _SUPPORTEDRATES_IE_ , 8, bssrate, &(pattrib->pktlen));
		pframe = set_ie(pframe, _EXT_SUPPORTEDRATES_IE_ , (bssrate_len - 8), (bssrate + 8), &(pattrib->pktlen));
	}
	else
	{
		pframe = set_ie(pframe, _SUPPORTEDRATES_IE_ , bssrate_len , bssrate, &(pattrib->pktlen));
	}

	//RSN
	p = get_ie((pmlmeinfo->network.IEs + sizeof(NDIS_802_11_FIXED_IEs)), _RSN_IE_2_, &ie_len, (pmlmeinfo->network.IELength - sizeof(NDIS_802_11_FIXED_IEs)));
	if (p != NULL)
	{
		pframe = set_ie(pframe, _RSN_IE_2_, ie_len, (p + 2), &(pattrib->pktlen));
	}

	//HT caps
	p = get_ie((pmlmeinfo->network.IEs + sizeof(NDIS_802_11_FIXED_IEs)), _HT_CAPABILITY_IE_, &ie_len, (pmlmeinfo->network.IELength - sizeof(NDIS_802_11_FIXED_IEs)));
	if ((p != NULL) && (!(is_ap_in_tkip(padapter))))
	{
		_memcpy(&(pmlmeinfo->HT_caps), (p + 2), sizeof(struct HT_caps_element));

		//to disable 40M Hz support while gd_bw_40MHz_en = 0
		if (pregpriv->cbw40_enable == 0)
		{
			pmlmeinfo->HT_caps.HT_cap_element.HT_caps_info &= (~(BIT(6) | BIT(1)));
		}
		else
		{
			pmlmeinfo->HT_caps.HT_cap_element.HT_caps_info |= BIT(1);
		}

		//todo: disable SM power save mode
		pmlmeinfo->HT_caps.HT_cap_element.HT_caps_info |= 0x000c;

		padapter->HalFunc.GetHwRegHandler(padapter, HW_VAR_RF_TYPE, (u8 *)(&rf_type));
		//switch (pregpriv->rf_config)
		switch(rf_type)
		{
			case RF_1T1R:
				pmlmeinfo->HT_caps.HT_cap_element.HT_caps_info |= 0x0100;//RX STBC One spatial stream
				_memcpy(pmlmeinfo->HT_caps.HT_cap_element.MCS_rate, MCS_rate_1R, 16);
				break;

			case RF_2T2R:
			case RF_1T2R:
			default:
				pmlmeinfo->HT_caps.HT_cap_element.HT_caps_info |= 0x0200;//RX STBC two spatial stream
				_memcpy(pmlmeinfo->HT_caps.HT_cap_element.MCS_rate, MCS_rate_2R, 16);
				break;
		}

		pmlmeinfo->HT_caps.HT_cap_element.HT_caps_info = cpu_to_le16(pmlmeinfo->HT_caps.HT_cap_element.HT_caps_info);
		pframe = set_ie(pframe, _HT_CAPABILITY_IE_, ie_len , (u8 *)(&(pmlmeinfo->HT_caps)), &(pattrib->pktlen));
		
	}

	//vendor specific IE, such as WPA, WMM, WPS
	for (i = sizeof(NDIS_802_11_FIXED_IEs); i < pmlmeinfo->network.IELength;)
	{
		pIE = (PNDIS_802_11_VARIABLE_IEs)(pmlmeinfo->network.IEs + i);

		switch (pIE->ElementID)
		{
			case _VENDOR_SPECIFIC_IE_:
				if ((_memcmp(pIE->data, WPA_OUI, 4)) ||
						(_memcmp(pIE->data, WMM_OUI, 4)) ||
						(_memcmp(pIE->data, WPS_OUI, 4)))
				{
					pframe = set_ie(pframe, _VENDOR_SPECIFIC_IE_, pIE->Length, pIE->data, &(pattrib->pktlen));
				}
				break;

			default:
				break;
		}

		i += (pIE->Length + 2);
	}

	if (pmlmeinfo->assoc_AP_vendor == realtekAP)
	{
		pframe = set_ie(pframe, _VENDOR_SPECIFIC_IE_, 6 , REALTEK_96B_IE, &(pattrib->pktlen));
	}

	if ( pwdinfo->p2p_state == P2P_STATE_RECV_INVITE_REQ )
	{
		//	Should add the P2P IE in the association request frame.	
		//	P2P OUI
		
		p2pielen = 0;
		p2pie[ p2pielen++ ] = 0x50;
		p2pie[ p2pielen++ ] = 0x6F;
		p2pie[ p2pielen++ ] = 0x9A;
		p2pie[ p2pielen++ ] = 0x09;	//	WFA P2P v1.0

		//	Commented by Albert 20101109
		//	According to the P2P Specification, the association request frame should contain 3 P2P attributes
		//	1. P2P Capability
		//	2. Extended Listen Timing
		//	3. Device Info

		//	P2P Capability
		//	Type:
		p2pie[ p2pielen++ ] = P2P_ATTR_CAPABILITY;

		//	Length:
		*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0x0002 );
		p2pielen += 2;

		//	Value:
		//	Device Capability Bitmap, 1 byte
		//	Be able to participate in additional P2P Groups and
		//	support the P2P Invitation Procedure
		p2pie[ p2pielen++ ] = P2P_DEVCAP_INVITATION_PROC;
	
		//	Group Capability Bitmap, 1 byte
		p2pie[ p2pielen++ ] = 0x00;

		//	Extended Listen Timing
		//	Type:
		p2pie[ p2pielen++ ] = P2P_ATTR_EX_LISTEN_TIMING;

		//	Length:
		*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0x0004 );
		p2pielen += 2;

		//	Value:
		//	Availability Period
		*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0xFFFF );
		p2pielen += 2;

		//	Availability Interval
		*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0xFFFF );
		p2pielen += 2;

		//	Device Info
		//	Type:
		p2pie[ p2pielen++ ] = P2P_ATTR_DEVICE_INFO;

		//	Length:
		//	21 -> P2P Device Address (6bytes) + Config Methods (2bytes) + Primary Device Type (8bytes) 
		//	+ NumofSecondDevType (1byte) + WPS Device Name ID field (2bytes) + WPS Device Name Len field (2bytes)
		*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 21 + pwdinfo->device_name_len );
		p2pielen += 2;

		//	Value:
		//	P2P Device Address
		_memcpy( p2pie + p2pielen, myid( &padapter->eeprompriv ), ETH_ALEN );
		p2pielen += ETH_ALEN;

		//	Config Method
		//	This field should be big endian. Noted by P2P specification.
		*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( WPS_CONFIG_METHOD_DISPLAY );
		p2pielen += 2;

		//	Primary Device Type
		//	Category ID
		*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( WPS_PDT_CID_RTK_WIDI );
		p2pielen += 2;

		//	OUI
		*(u32*) ( p2pie + p2pielen ) = cpu_to_be32( WPSOUI );
		p2pielen += 4;

		//	Sub Category ID
		*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( WPS_PDT_SCID_RTK_DMP );
		p2pielen += 2;

		//	Number of Secondary Device Types
		p2pie[ p2pielen++ ] = 0x00;	//	No Secondary Device Type List

		//	Device Name
		//	Type:
		*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( WPS_ATTR_DEVICE_NAME );
		p2pielen += 2;

		//	Length:
		*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( pwdinfo->device_name_len );
		p2pielen += 2;

		//	Value:
		_memcpy( p2pie + p2pielen, pwdinfo->device_name, pwdinfo->device_name_len );
		p2pielen += pwdinfo->device_name_len;
	
		pframe = set_ie(pframe, _VENDOR_SPECIFIC_IE_, p2pielen, (unsigned char *) p2pie, &pattrib->pktlen );
		
	}

	pattrib->last_txcmdsz = pattrib->pktlen;
	dump_mgntframe(padapter, pmgntframe);

	return;
}

void issue_nulldata(_adapter *padapter, unsigned int power_mode)
{
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct ieee80211_hdr	*pwlanhdr;
	unsigned short				*fctrl;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	//DBG_871X("%s:%d\n", __FUNCTION__, power_mode);

	if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
	{
		return;
	}

	//update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;
	SetToDs(fctrl);
	if (power_mode)
	{
		SetPwrMgt(fctrl);
	}

	_memcpy(pwlanhdr->addr1, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);
	_memcpy(pwlanhdr->addr2, myid(&(padapter->eeprompriv)), ETH_ALEN);
	_memcpy(pwlanhdr->addr3, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	SetFrameSubType(pframe, WIFI_DATA_NULL);

	pframe += sizeof(struct ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct ieee80211_hdr_3addr);

	pattrib->last_txcmdsz = pattrib->pktlen;
	dump_mgntframe(padapter, pmgntframe);

	return;
}


void issue_qos_nulldata(_adapter *padapter, unsigned char *da, u16 tid)
{
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct ieee80211_hdr	*pwlanhdr;
	unsigned short				*fctrl, *qc;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	DBG_871X("%s\n", __FUNCTION__);

	if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
	{
		return;
	}

	//update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	pattrib->hdrlen +=2;
	pattrib->qos_en = _TRUE;
	pattrib->eosp = 1;
	pattrib->ack_policy = 0;
	pattrib->mdata = 0;

	_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;
	SetToDs(fctrl);

	if(pattrib->mdata)
		SetMData(fctrl);

	qc = (unsigned short *)(pframe + pattrib->hdrlen - 2);
	
	SetPriority(qc, tid);

	SetEOSP(qc, pattrib->eosp);

	SetAckpolicy(qc, pattrib->ack_policy);

	_memcpy(pwlanhdr->addr1, da, ETH_ALEN);
	_memcpy(pwlanhdr->addr2, myid(&(padapter->eeprompriv)), ETH_ALEN);
	_memcpy(pwlanhdr->addr3, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	SetFrameSubType(pframe, WIFI_QOS_DATA_NULL);

	pframe += sizeof(struct ieee80211_hdr_3addr_qos);
	pattrib->pktlen = sizeof(struct ieee80211_hdr_3addr_qos);

	pattrib->last_txcmdsz = pattrib->pktlen;
	dump_mgntframe(padapter, pmgntframe);
	
}

void issue_deauth(_adapter *padapter, unsigned char *da, unsigned short reason)
{
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct ieee80211_hdr	*pwlanhdr;
	unsigned short				*fctrl;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
	{
		return;
	}

	//update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	_memcpy(pwlanhdr->addr1, da, ETH_ALEN);
	_memcpy(pwlanhdr->addr2, myid(&(padapter->eeprompriv)), ETH_ALEN);
	_memcpy(pwlanhdr->addr3, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	SetFrameSubType(pframe, WIFI_DEAUTH);

	pframe += sizeof(struct ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct ieee80211_hdr_3addr);

	reason = cpu_to_le16(reason);
	pframe = set_fixed_ie(pframe, _RSON_CODE_ , (unsigned char *)&reason, &(pattrib->pktlen));

	pattrib->last_txcmdsz = pattrib->pktlen;

	dump_mgntframe(padapter, pmgntframe);
}

void issue_action_BA(_adapter *padapter, unsigned char *raddr, unsigned char action, unsigned short status)
{	
	u8 category = WLAN_CATEGORY_BACK;
	u16	start_seq;
	u16	BA_para_set;
	u16	reason_code;
	u16	BA_timeout_value;
	u16	BA_starting_seqctrl;
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	u8					*pframe;
	struct ieee80211_hdr	*pwlanhdr;
	u16				*fctrl;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct sta_info				*psta;
	struct sta_priv				*pstapriv = &padapter->stapriv;


	DBG_871X("%s, category=%d, action=%d, status=%d\n", __FUNCTION__, category, action, status);

	if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
	{
		return;
	}

	//update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	//_memcpy(pwlanhdr->addr1, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);
	_memcpy(pwlanhdr->addr1, raddr, ETH_ALEN);
	_memcpy(pwlanhdr->addr2, myid(&(padapter->eeprompriv)), ETH_ALEN);
	_memcpy(pwlanhdr->addr3, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	SetFrameSubType(pframe, WIFI_ACTION);

	pframe += sizeof(struct ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct ieee80211_hdr_3addr);

	pframe = set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
	pframe = set_fixed_ie(pframe, 1, &(action), &(pattrib->pktlen));

      status = cpu_to_le16(status);
	

	if (category == 3)
	{
		switch (action)
		{
			case 0: //ADDBA req
				do {
					pmlmeinfo->dialogToken++;
				} while (pmlmeinfo->dialogToken == 0);
				pframe = set_fixed_ie(pframe, 1, &(pmlmeinfo->dialogToken), &(pattrib->pktlen));

				BA_para_set = (0x0803 | ((status & 0xf) << 2)); //immediate ack & 64 buffer size
				//sys_mib.BA_para_set = 0x0802;
				BA_para_set = cpu_to_le16(BA_para_set);
				pframe = set_fixed_ie(pframe, 2, (unsigned char *)(&(BA_para_set)), &(pattrib->pktlen));

				BA_timeout_value = 0;
				BA_timeout_value = cpu_to_le16(BA_timeout_value);
				pframe = set_fixed_ie(pframe, 2, (unsigned char *)(&(BA_timeout_value)), &(pattrib->pktlen));

				//if ((psta = get_stainfo(pstapriv, pmlmeinfo->network.MacAddress)) != NULL)
				if ((psta = get_stainfo(pstapriv, raddr)) != NULL)
				{
					start_seq = (psta->sta_xmitpriv.txseq_tid[status & 0x07]&0xfff) + 1;
					BA_starting_seqctrl = start_seq << 4;
				}
				BA_starting_seqctrl = cpu_to_le16(BA_starting_seqctrl);
				pframe = set_fixed_ie(pframe, 2, (unsigned char *)(&(BA_starting_seqctrl)), &(pattrib->pktlen));
				break;

			case 1: //ADDBA rsp
				pframe = set_fixed_ie(pframe, 1, &(pmlmeinfo->ADDBA_req.dialog_token), &(pattrib->pktlen));
				pframe = set_fixed_ie(pframe, 2, (unsigned char *)(&status), &(pattrib->pktlen));
				BA_para_set = cpu_to_le16((le16_to_cpu(pmlmeinfo->ADDBA_req.BA_para_set) & 0x3f) | 0x0800);
				pframe = set_fixed_ie(pframe, 2, (unsigned char *)(&(pmlmeinfo->ADDBA_req.BA_para_set)), &(pattrib->pktlen));
				pframe = set_fixed_ie(pframe, 2, (unsigned char *)(&(pmlmeinfo->ADDBA_req.BA_timeout_value)), &(pattrib->pktlen));
				break;
			case 2://DELBA
				BA_para_set = (status & 0x1F) << 3;
				BA_para_set = cpu_to_le16(BA_para_set);				
				pframe = set_fixed_ie(pframe, 2, (unsigned char *)(&(BA_para_set)), &(pattrib->pktlen));

				reason_code = 37;//Requested from peer STA as it does not want to use the mechanism
				reason_code = cpu_to_le16(reason_code);
				pframe = set_fixed_ie(pframe, 2, (unsigned char *)(&(reason_code)), &(pattrib->pktlen));
				break;
			default:
				break;
		}
	}

	pattrib->last_txcmdsz = pattrib->pktlen;

	dump_mgntframe(padapter, pmgntframe);
}

static void issue_action_BSSCoexistPacket(_adapter *padapter)
{	
	_irqL	irqL;
	_list		*plist, *phead;
	unsigned char category, action;
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char				*pframe;
	struct ieee80211_hdr	*pwlanhdr;
	unsigned short			*fctrl;
	struct	wlan_network	*pnetwork = NULL;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	_queue		*queue	= &(pmlmepriv->scanned_queue);
	u8 InfoContent[16] = {0};
	u8 ICS[8][15];
	
	if((pmlmepriv->num_FortyMHzIntolerant==0) || (pmlmepriv->num_sta_no_ht==0))
		return;

	if(_TRUE == pmlmeinfo->bwmode_updated)
		return;
	

	DBG_871X("%s\n", __FUNCTION__);


	category = WLAN_CATEGORY_PUBLIC;
	action = ACT_PUBLIC_BSSCOEXIST;

	if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
	{
		return;
	}

	//update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	_memcpy(pwlanhdr->addr1, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);
	_memcpy(pwlanhdr->addr2, myid(&(padapter->eeprompriv)), ETH_ALEN);
	_memcpy(pwlanhdr->addr3, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	SetFrameSubType(pframe, WIFI_ACTION);

	pframe += sizeof(struct ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct ieee80211_hdr_3addr);

	pframe = set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
	pframe = set_fixed_ie(pframe, 1, &(action), &(pattrib->pktlen));


	//
	if(pmlmepriv->num_FortyMHzIntolerant>0)
	{
		u8 iedata=0;
		
		iedata |= BIT(2);//20 MHz BSS Width Request

		pframe = set_ie(pframe, EID_BSSCoexistence,  1, &iedata, &(pattrib->pktlen));
		
	}
	

	//
	_memset(ICS, 0, sizeof(ICS));
	if(pmlmepriv->num_sta_no_ht>0)
	{	
		int i;
	
		_enter_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

		phead = get_list_head(queue);
		plist = get_next(phead);
       
		while(1)
		{
			int len;
			u8 *p;
			WLAN_BSSID_EX *pbss_network;
	
			if (end_of_queue_search(phead,plist)== _TRUE)
				break;		

			pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);      
		
			plist = get_next(plist);

			pbss_network = (WLAN_BSSID_EX *)&pnetwork->network;

			p = get_ie(pbss_network->IEs + _FIXED_IE_LENGTH_, _HT_CAPABILITY_IE_, &len, pbss_network->IELength - _FIXED_IE_LENGTH_);
			if((p==NULL) || (len==0))//non-HT
			{
				if((pbss_network->Configuration.DSConfig<=0) || (pbss_network->Configuration.DSConfig>14))
					continue;
				
				ICS[0][pbss_network->Configuration.DSConfig]=1;
				
				if(ICS[0][0] == 0)
					ICS[0][0] = 1;		
			}		
	
		}        

		_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);


		for(i= 0;i<8;i++)
		{
			if(ICS[i][0] == 1)
			{
				int j, k = 0;
				
				InfoContent[k] = i;				
				//SET_BSS_INTOLERANT_ELE_REG_CLASS(InfoContent,i);
				k++;
				
				for(j=1;j<=14;j++)
				{
					if(ICS[i][j]==1)
					{
						if(k<16)
						{
							InfoContent[k] = j; //channel number
							//SET_BSS_INTOLERANT_ELE_CHANNEL(InfoContent+k, j);
							k++;
						}	
					}	
				}	

				pframe = set_ie(pframe, EID_BSSIntolerantChlReport, k, InfoContent, &(pattrib->pktlen));
				
			}
			
		}
		

	}
		

	pattrib->last_txcmdsz = pattrib->pktlen;

	dump_mgntframe(padapter, pmgntframe);

}

unsigned int send_delba(_adapter *padapter, u8 initiator, u8 *addr)
{
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *psta = NULL;
//	struct recv_reorder_ctrl *preorder_ctrl;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	u16 tid;

	if((pmlmeinfo->state&0x03) != WIFI_FW_AP_STATE)	
		if (!(pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS))
			return _SUCCESS;
	
	psta = get_stainfo(pstapriv, addr);
	if(psta==NULL)
		return _SUCCESS;

	//printk("%s:%s\n", __FUNCTION__, (initiator==0)?"RX_DIR":"TX_DIR");
	
	if(initiator==0) // recipient
	{
		for(tid = 0;tid<MAXTID;tid++)
		{
			if(psta->recvreorder_ctrl[tid].enable == _TRUE)
			{
				printk("rx agg disable tid(%d)\n",tid);
				issue_action_BA(padapter, addr, WLAN_ACTION_DELBA, (((tid <<1) |initiator)&0x1F));
				psta->recvreorder_ctrl[tid].enable = _FALSE;
				psta->recvreorder_ctrl[tid].indicate_seq = 0xffff;
			}		
		}
	}
	else if(initiator == 1)// originator
	{
		//printk("tx agg_enable_bitmap(0x%08x)\n", psta->htpriv.agg_enable_bitmap);
		for(tid = 0;tid<MAXTID;tid++)
		{
			if(psta->htpriv.agg_enable_bitmap & BIT(tid))
			{
				printk("tx agg disable tid(%d)\n",tid);
				issue_action_BA(padapter, addr, WLAN_ACTION_DELBA, (((tid <<1) |initiator)&0x1F) );
				psta->htpriv.agg_enable_bitmap &= ~BIT(tid);
				psta->htpriv.candidate_tid_bitmap &= ~BIT(tid);
				
			}			
		}
	}
	
	return _SUCCESS;
	
}

unsigned int send_beacon(_adapter *padapter)
{
	u8	bxmitok = _FALSE;
	int	retry=0;

	do{

		issue_beacon(padapter);

		padapter->HalFunc.GetHwRegHandler(padapter, HW_VAR_TX_BCN_DONE, (u8 *)(&bxmitok));

	}while((_FALSE == bxmitok) &&((retry++)<100 ));

	if(retry == 100)
	{
		DBG_871X("send_beacon, fail!\n");
		return _FAIL;
	}
	else
	{
		return _SUCCESS;
	}
	
}

/****************************************************************************

Following are some utitity fuctions for WiFi MLME

*****************************************************************************/

BOOLEAN IsLegal5GChannel(
	IN PADAPTER			Adapter,
	IN u8			channel)
{
	
	int i=0;
	u8 Channel_5G[45] = {36,38,40,42,44,46,48,50,52,54,56,58,
		60,62,64,100,102,104,106,108,110,112,114,116,118,120,122,
		124,126,128,130,132,134,136,138,140,149,151,153,155,157,159,
		161,163,165};
	for(i=0;i<sizeof(Channel_5G);i++)
		if(channel == Channel_5G[i])
			return _TRUE;
	return _FALSE;
}

void site_survey(_adapter *padapter)
{
	unsigned char		survey_channel, val8;
	RT_SCAN_TYPE	ScanType;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);
	static unsigned char  prev_survey_channel = 0;
	static unsigned int p2p_scan_count = 0;

	if ( pwdinfo->p2p_state == P2P_STATE_GONEGO_OK )
	{
		p2p_scan_count++;
		printk( "[%s] p2p_state = P2P_STATE_GONEGO_OK, p2p_scan_count = %d\n", __FUNCTION__,  p2p_scan_count);
		if ( ( prev_survey_channel == pwdinfo->peer_operating_ch ) && ( p2p_scan_count > P2P_PROVISIONING_SCAN_CNT ) )
		{
			survey_channel = 0;
			p2p_scan_count = 0;
		}
		else
		{
			survey_channel = pwdinfo->peer_operating_ch;
		}
		
		prev_survey_channel = survey_channel;
		ScanType = SCAN_ACTIVE;
		printk( "[%s] survey_channel = %d\n", __FUNCTION__, survey_channel );
	}
	else
	{
		survey_channel = pmlmeext->channel_set[pmlmeext->sitesurvey_res.channel_idx].ChannelNum;
		ScanType = pmlmeext->channel_set[pmlmeext->sitesurvey_res.channel_idx].ScanType;
	}  

	if(survey_channel != 0)
	{		
		//PAUSE 4-AC Queue when site_survey
		padapter->HalFunc.GetHwRegHandler(padapter, HW_VAR_TXPAUSE, (u8 *)(&val8));
		val8 |= 0x0f;
		padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_TXPAUSE, (u8 *)(&val8));
		
		if(pmlmeext->sitesurvey_res.channel_idx == 0)
		{
			set_channel_bwmode(padapter, survey_channel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, HT_CHANNEL_WIDTH_20);
		}
		else
		{
			SelectChannel(padapter, survey_channel);
		}

		if((pmlmeext->sitesurvey_res.scan_mode == SCAN_ACTIVE) && (ScanType == SCAN_ACTIVE))
		{
			if ( ( pwdinfo->p2p_state == P2P_STATE_SCAN ) || ( pwdinfo->p2p_state == P2P_STATE_FIND_PHASE_SEARCH ) )
			{
				issue_probereq_p2p(padapter);				
			}
			else
			{
				//todo: to issue two probe req???
				issue_probereq(padapter, 1);
				//msleep_os(SURVEY_TO>>1);
				issue_probereq(padapter, 1);
			}
		}

		_set_timer(&pmlmeext->survey_timer, pmlmeext->chan_scan_time);

	}
	else
	{
		//	channel number is 0 or this channel is not valid.
		if ( ( pwdinfo->p2p_state == P2P_STATE_SCAN ) || ( pwdinfo->p2p_state == P2P_STATE_FIND_PHASE_SEARCH ) )
		{
			printk( "[%s] find phase exchange cnt = %d\n", __FUNCTION__, pwdinfo->find_phase_state_exchange_cnt );
		}
		
		if ( ( ( pwdinfo->p2p_state == P2P_STATE_SCAN ) || ( pwdinfo->p2p_state == P2P_STATE_FIND_PHASE_SEARCH ) ) && 
			( pwdinfo->find_phase_state_exchange_cnt < P2P_FINDPHASE_EX_CNT ) )
		{
			//	Set the P2P State to the listen state of find phase and set the current channel to the listen channel
			pwdinfo->p2p_state = P2P_STATE_FIND_PHASE_LISTEN;
			pmlmeext->sitesurvey_res.state = SCAN_DISABLE;			
			set_channel_bwmode(padapter, pwdinfo->listen_channel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, HT_CHANNEL_WIDTH_20);
			_set_timer( &pwdinfo->find_phase_timer, ( u32 ) ( ( u32 ) ( pwdinfo->listen_dwell ) * 100 ) );
		}
		else
		{

			if ( ( pwdinfo->p2p_state == P2P_STATE_SCAN ) || ( pwdinfo->p2p_state == P2P_STATE_FIND_PHASE_SEARCH ) )
			{
				pwdinfo->p2p_state = P2P_STATE_LISTEN;
			}
			
			pmlmeext->sitesurvey_res.state = SCAN_COMPLETE;
	
#ifdef CONFIG_ANTENNA_DIVERSITY
			// 20100721:Interrupt scan operation here.
			// For SW antenna diversity before link, it needs to switch to another antenna and scan again.
			// It compares the scan result and select beter one to do connection.
			if(padapter->HalFunc.SwAntDivBeforeLinkHandler(padapter))
			{				
				pmlmeext->sitesurvey_res.bss_cnt = 0;
				pmlmeext->sitesurvey_res.channel_idx = -1;
				pmlmeext->chan_scan_time = SURVEY_TO /2;			
				_set_timer(&pmlmeext->survey_timer, pmlmeext->chan_scan_time);
				return;
			}
#endif
			//switch back to the original channel
			//SelectChannel(padapter, pmlmeext->cur_channel, pmlmeext->cur_ch_offset);

			if ( pwdinfo->p2p_state == P2P_STATE_GONEGO_OK )
			{
				printk( "[%s] In P2P WPS mode, stay in the peer operating channel = %d\n", __FUNCTION__,  pwdinfo->peer_operating_ch );
				set_channel_bwmode(padapter, pwdinfo->peer_operating_ch, pmlmeext->cur_ch_offset, pmlmeext->cur_bwmode);
			}
			else
			{
				set_channel_bwmode(padapter, pmlmeext->cur_channel, pmlmeext->cur_ch_offset, pmlmeext->cur_bwmode);
			}

			//flush 4-AC Queue after site_survey
			val8 = 0;
			padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_TXPAUSE, (u8 *)(&val8));

			val8 = 0;
			padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_MLME_SITESURVEY, (u8 *)(&val8));

			//config MSR
			Set_NETYPE0_MSR(padapter, (pmlmeinfo->state & 0x3));

			//turn on dynamic functions
			Restore_DM_Func_Flag(padapter);
			//Switch_DM_Func(padapter, DYNAMIC_FUNC_DIG|DYNAMIC_FUNC_HP|DYNAMIC_FUNC_SS, _TRUE);

			if (is_client_associated_to_ap(padapter) == _TRUE)
			{
				//issue null data 
				issue_nulldata(padapter, 0);
			}

			report_surveydone_event(padapter);

			pmlmeext->chan_scan_time = SURVEY_TO;
			pmlmeext->sitesurvey_res.state = SCAN_DISABLE;

#ifdef PLATFORM_LINUX
			if(txframes_pending(padapter))	
			{
				struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
				tasklet_hi_schedule(&pxmitpriv->xmit_tasklet);
			}
#endif

			issue_action_BSSCoexistPacket(padapter);

		}
		
	}

	return;

}

//collect bss info from Beacon and Probe response frames.
u8 collect_bss_info(_adapter *padapter, union recv_frame *precv_frame, WLAN_BSSID_EX *bssid)
{
	int							i;
	u32	len;
	u8	*p;
	u16	val16, subtype;
	u8 *pframe = precv_frame->u.hdr.rx_data;
	u32	packet_len = precv_frame->u.hdr.len;
	struct registry_priv 	*pregistrypriv = &padapter->registrypriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	len = packet_len - sizeof(struct ieee80211_hdr_3addr);

	if (len > MAX_IE_SZ)
	{
		//printk("IE too long for survey event\n");
		return _FAIL;
	}

	_memset(bssid, 0, sizeof(WLAN_BSSID_EX));

	subtype = GetFrameSubType(pframe) >> 4;

	if(subtype==WIFI_BEACON)
		bssid->Reserved[0] = 1;
		
	bssid->Length = sizeof(WLAN_BSSID_EX) - MAX_IE_SZ + len;

	//below is to copy the information element
	bssid->IELength = len;
	_memcpy(bssid->IEs, (pframe + sizeof(struct ieee80211_hdr_3addr)), bssid->IELength);

	//get the signal strength
	//bssid->Rssi = precv_frame->u.hdr.attrib.signal_strength; // 0-100 index.	
	bssid->Rssi = precv_frame->u.hdr.attrib.RecvSignalPower; // in dBM.raw data	
	bssid->PhyInfo.SignalQuality = precv_frame->u.hdr.attrib.signal_qual;//in percentage 
	bssid->PhyInfo.SignalStrength = precv_frame->u.hdr.attrib.signal_strength;//in percentage
#ifdef CONFIG_ANTENNA_DIVERSITY
	padapter->HalFunc.GetHwRegHandler(padapter, HW_VAR_CURRENT_ANTENNA, (u8 *)(&bssid->PhyInfo.Optimum_antenna));
#endif

	// checking SSID
	if ((p = get_ie(bssid->IEs + _FIXED_IE_LENGTH_, _SSID_IE_, &len, bssid->IELength - _FIXED_IE_LENGTH_)) == NULL)
	{
		DBG_871X("marc: cannot find SSID for survey event\n");
		return _FAIL;
	}

	if (*(p + 1))
	{
		_memcpy(bssid->Ssid.Ssid, (p + 2), *(p + 1));
		bssid->Ssid.SsidLength = *(p + 1);
	}
	else
	{
		bssid->Ssid.SsidLength = 0;
	}

	_memset(bssid->SupportedRates, 0, NDIS_802_11_LENGTH_RATES_EX);

	//checking rate info...
	i = 0;
	p = get_ie(bssid->IEs + _FIXED_IE_LENGTH_, _SUPPORTEDRATES_IE_, &len, bssid->IELength - _FIXED_IE_LENGTH_);
	if (p != NULL)
	{
		_memcpy(bssid->SupportedRates, (p + 2), len);
		i = len;
	}

	p = get_ie(bssid->IEs + _FIXED_IE_LENGTH_, _EXT_SUPPORTEDRATES_IE_, &len, bssid->IELength - _FIXED_IE_LENGTH_);
	if (p != NULL)
	{
		_memcpy(bssid->SupportedRates + i, (p + 2), len);
	}

	//todo:
#if 0
	if (judge_network_type(bssid->SupportedRates, (len + i)) == WIRELESS_11B)
	{
		bssid->NetworkTypeInUse = Ndis802_11DS;
	}
	else
#endif
	{
		bssid->NetworkTypeInUse = Ndis802_11OFDM24;
	}

	// Checking for DSConfig
	p = get_ie(bssid->IEs + _FIXED_IE_LENGTH_, _DSSET_IE_, &len, bssid->IELength - _FIXED_IE_LENGTH_);

	bssid->Configuration.DSConfig = 0;
	bssid->Configuration.Length = 0;

	if (p)
	{
		bssid->Configuration.DSConfig = *(p + 2);
	}
	else
	{// In 5G, some ap do not have DSSET IE
		// checking HT info for channel
		p = get_ie(bssid->IEs + _FIXED_IE_LENGTH_, _HT_ADD_INFO_IE_, &len, bssid->IELength - _FIXED_IE_LENGTH_);
		if(p)
		{
			struct HT_info_element *HT_info = (struct HT_info_element *)(p + 2);
			bssid->Configuration.DSConfig = HT_info->primary_channel;
		}
		else
		{ // use current channel
			if (padapter->mlmeextpriv.sitesurvey_res.state == SCAN_PROCESS)
				bssid->Configuration.DSConfig = padapter->mlmeextpriv.channel_set[padapter->mlmeextpriv.sitesurvey_res.channel_idx].ChannelNum;
			else
				bssid->Configuration.DSConfig = padapter->mlmeextpriv.cur_channel;
		}
	}

	_memcpy(&bssid->Configuration.BeaconPeriod, get_beacon_interval_from_ie(bssid->IEs), 2);

	bssid->Configuration.BeaconPeriod = le32_to_cpu(bssid->Configuration.BeaconPeriod);

	val16 = get_capability((WLAN_BSSID_EX *)bssid);

	if (val16 & BIT(0))
	{
		bssid->InfrastructureMode = Ndis802_11Infrastructure;
		_memcpy(bssid->MacAddress, GetAddr2Ptr(pframe), ETH_ALEN);
	}
	else
	{
		bssid->InfrastructureMode = Ndis802_11IBSS;
		_memcpy(bssid->MacAddress, GetAddr3Ptr(pframe), ETH_ALEN);
	}

	if (val16 & BIT(4))
		bssid->Privacy = 1;
	else
		bssid->Privacy = 0;

	bssid->Configuration.ATIMWindow = 0;

	//20/40 BSS Coexistence check
	if((pregistrypriv->wifi_spec==1) && (_FALSE == pmlmeinfo->bwmode_updated))
	{	
		struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
		
		p = get_ie(bssid->IEs + _FIXED_IE_LENGTH_, _HT_CAPABILITY_IE_, &len, bssid->IELength - _FIXED_IE_LENGTH_);
		if(p && len>0)
		{
			struct HT_caps_element	*pHT_caps;
			pHT_caps = (struct HT_caps_element	*)(p + 2);
			
			if(pHT_caps->HT_cap_element.HT_caps_info&BIT(14))
			{				
				pmlmepriv->num_FortyMHzIntolerant++;
			}
		}
		else
		{
			pmlmepriv->num_sta_no_ht++;
		}
		
	}

	return _SUCCESS;

}

void start_create_ibss(_adapter* padapter)
{
	unsigned short	caps;
	u32	val32;
	u8	val8;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX		*pnetwork = (WLAN_BSSID_EX*)(&(pmlmeinfo->network));
	pmlmeext->cur_channel = (u8)pnetwork->Configuration.DSConfig;
	pmlmeinfo->bcn_interval = get_beacon_interval(pnetwork);

	//update wireless mode
	update_wireless_mode(padapter);

	//udpate capability
	caps = get_capability((WLAN_BSSID_EX *)pnetwork);
	update_capinfo(padapter, caps);
	if(caps&cap_IBSS)//adhoc master
	{		
		//set_opmode_cmd(padapter, adhoc);//removed

		val8 = 0xcf;
		padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_SEC_CFG, (u8 *)(&val8));

		//switch channel
		//SelectChannel(padapter, pmlmeext->cur_channel, HAL_PRIME_CHNL_OFFSET_DONT_CARE);
		set_channel_bwmode(padapter, pmlmeext->cur_channel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, HT_CHANNEL_WIDTH_20);

		beacon_timing_control(padapter);

		//set msr to WIFI_FW_ADHOC_STATE
		Set_NETYPE0_MSR(padapter, (pmlmeinfo->state & 0x3));

		//issue beacon
		if(send_beacon(padapter)==_FAIL)
		{
			RT_TRACE(_module_rtl871x_mlme_c_,_drv_err_,("issuing beacon frame fail....\n"));
			
			report_join_res(padapter, -1);
			//pmlmeinfo->state ^= WIFI_FW_ADHOC_STATE;
		}
		else
		{
			val8 = 0;
			padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_CHECK_BSSID, (u8 *)(&val8));
					
			report_join_res(padapter, 1);
			pmlmeinfo->state = WIFI_FW_ADHOC_STATE;
			pmlmeinfo->state |= WIFI_FW_ASSOC_SUCCESS;
		}
	}
	else
	{
		DBG_871X("start_create_ibss, invalid cap:%x\n", caps);
		return;
	}

}

void start_clnt_join(_adapter* padapter)
{
	unsigned short	caps;	
	u8	val8;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX		*pnetwork = (WLAN_BSSID_EX*)(&(pmlmeinfo->network));


	pmlmeext->cur_channel = (u8)pnetwork->Configuration.DSConfig;
	pmlmeinfo->bcn_interval = get_beacon_interval(pnetwork);

	//update wireless mode
	update_wireless_mode(padapter);

	//udpate capability
	caps = get_capability((WLAN_BSSID_EX *)pnetwork);
	update_capinfo(padapter, caps);
	if (caps&cap_ESS)
	{
		pmlmeinfo->state = WIFI_FW_AUTH_NULL | WIFI_FW_STATION_STATE;

		//set_opmode_cmd(padapter, infra_client_with_mlme);//removed

		val8 = (pmlmeinfo->auth_algo == dot11AuthAlgrthm_8021X)? 0xcc: 0xcf;
		padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_SEC_CFG, (u8 *)(&val8));

		//switch channel	
		set_channel_bwmode(padapter, pmlmeext->cur_channel, pmlmeext->cur_ch_offset, pmlmeext->cur_bwmode);

		//here wait for receiving the beacon to start auth
		//and enable a timer
		_set_timer(&(pmlmeext->link_timer), decide_wait_for_beacon_timeout(pmlmeinfo->bcn_interval));
	}
	else if (caps&cap_IBSS) //adhoc client
	{
		//set_opmode_cmd(padapter, adhoc);//removed

		val8 = 0xcf;
		padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_SEC_CFG, (u8 *)(&val8));

		//switch channel		
		set_channel_bwmode(padapter, pmlmeext->cur_channel, pmlmeext->cur_ch_offset, pmlmeext->cur_bwmode);

		beacon_timing_control(padapter);

		//set msr to WIFI_FW_ADHOC_STATE
		Set_NETYPE0_MSR(padapter, (pmlmeinfo->state & 0x3));

		report_join_res(padapter, 1);		
		pmlmeinfo->state = WIFI_FW_ADHOC_STATE;
	}
	else
	{
		//printk("marc: invalid cap:%x\n", caps);
		return;
	}

}

void start_clnt_auth(_adapter* padapter)
{
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	_cancel_timer_ex(&pmlmeext->link_timer);

	pmlmeinfo->state &= (~WIFI_FW_AUTH_NULL);
	pmlmeinfo->state |= WIFI_FW_AUTH_STATE;

	pmlmeinfo->auth_seq = 1;
	pmlmeinfo->reauth_count = 0;
	pmlmeinfo->reassoc_count = 0;
	pmlmeinfo->link_count = 0;

	issue_auth(padapter, NULL, 0);

	_set_timer(&pmlmeext->link_timer, REAUTH_TO);

}


void start_clnt_assoc(_adapter* padapter)
{
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	_cancel_timer_ex(&pmlmeext->link_timer);

	pmlmeinfo->state &= (~(WIFI_FW_AUTH_NULL | WIFI_FW_AUTH_STATE));
	pmlmeinfo->state |= (WIFI_FW_AUTH_SUCCESS | WIFI_FW_ASSOC_STATE);

	issue_assocreq(padapter);

	_set_timer(&pmlmeext->link_timer, REASSOC_TO);
}

unsigned int receive_disconnect(_adapter *padapter, unsigned char *MacAddr)
{	
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	//check A3
	if (!(_memcmp(MacAddr, get_my_bssid(&pmlmeinfo->network), ETH_ALEN)))
		return _SUCCESS;

	DBG_871X("%s\n", __FUNCTION__);
	
        if((pmlmeinfo->state&0x03) == WIFI_FW_STATION_STATE)
	{
		if (pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS)
		{
			report_del_sta_event(padapter, MacAddr);
		}
		else if (pmlmeinfo->state & WIFI_FW_LINKING_STATE)
		{
			report_join_res(padapter, -2);
		}
	}

	return _SUCCESS;
}

/****************************************************************************

Following are the functions to report events

*****************************************************************************/

void report_survey_event(_adapter *padapter, union recv_frame *precv_frame)
{
	struct cmd_obj *pcmd_obj;
	u8	*pevtcmd;
	u32 cmdsz;
	struct survey_event	*psurvey_evt;
	struct C2HEvent_Header *pc2h_evt_hdr;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;
	//u8 *pframe = precv_frame->u.hdr.rx_data;
	//uint len = precv_frame->u.hdr.len;

	if ((pcmd_obj = (struct cmd_obj*)_malloc(sizeof(struct cmd_obj))) == NULL)
	{
		return;
	}

	cmdsz = (sizeof(struct survey_event) + sizeof(struct C2HEvent_Header));
	if ((pevtcmd = (u8*)_malloc(cmdsz)) == NULL)
	{
		_mfree((u8 *)pcmd_obj, sizeof(struct cmd_obj));
		return;
	}

	_init_listhead(&pcmd_obj->list);

	pcmd_obj->cmdcode = GEN_CMD_CODE(_Set_MLME_EVT);
	pcmd_obj->cmdsz = cmdsz;
	pcmd_obj->parmbuf = pevtcmd;

	pcmd_obj->rsp = NULL;
	pcmd_obj->rspsz  = 0;

	pc2h_evt_hdr = (struct C2HEvent_Header*)(pevtcmd);
	pc2h_evt_hdr->len = sizeof(struct survey_event);
	pc2h_evt_hdr->ID = GEN_EVT_CODE(_Survey);
	pc2h_evt_hdr->seq = pmlmeext->event_seq++;

	psurvey_evt = (struct survey_event*)(pevtcmd + sizeof(struct C2HEvent_Header));
	if (collect_bss_info(padapter, precv_frame, (WLAN_BSSID_EX *)&psurvey_evt->bss) == _FAIL)
	{
		_mfree((u8 *)pcmd_obj, sizeof(struct cmd_obj));
		_mfree((u8 *)pevtcmd, cmdsz);
		return;
	}

	enqueue_cmd(pcmdpriv, pcmd_obj);

	pmlmeext->sitesurvey_res.bss_cnt++;

	return;

}

void report_surveydone_event(_adapter *padapter)
{
	struct cmd_obj *pcmd_obj;
	u8	*pevtcmd;
	u32 cmdsz;
	struct surveydone_event *psurveydone_evt;
	struct C2HEvent_Header	*pc2h_evt_hdr;
	struct mlme_ext_priv		*pmlmeext = &padapter->mlmeextpriv;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;

	if ((pcmd_obj = (struct cmd_obj*)_malloc(sizeof(struct cmd_obj))) == NULL)
	{
		return;
	}

	cmdsz = (sizeof(struct surveydone_event) + sizeof(struct C2HEvent_Header));
	if ((pevtcmd = (u8*)_malloc(cmdsz)) == NULL)
	{
		_mfree((u8 *)pcmd_obj, sizeof(struct cmd_obj));
		return;
	}

	_init_listhead(&pcmd_obj->list);

	pcmd_obj->cmdcode = GEN_CMD_CODE(_Set_MLME_EVT);
	pcmd_obj->cmdsz = cmdsz;
	pcmd_obj->parmbuf = pevtcmd;

	pcmd_obj->rsp = NULL;
	pcmd_obj->rspsz  = 0;

	pc2h_evt_hdr = (struct C2HEvent_Header*)(pevtcmd);
	pc2h_evt_hdr->len = sizeof(struct surveydone_event);
	pc2h_evt_hdr->ID = GEN_EVT_CODE(_SurveyDone);
	pc2h_evt_hdr->seq = pmlmeext->event_seq++;

	psurveydone_evt = (struct surveydone_event*)(pevtcmd + sizeof(struct C2HEvent_Header));
	psurveydone_evt->bss_cnt = pmlmeext->sitesurvey_res.bss_cnt;

	DBG_871X("survey done event(%x)\n", psurveydone_evt->bss_cnt);

	enqueue_cmd(pcmdpriv, pcmd_obj);

	return;

}

void report_join_res(_adapter *padapter, int res)
{
	struct cmd_obj *pcmd_obj;
	u8	*pevtcmd;
	u32 cmdsz;
	struct joinbss_event		*pjoinbss_evt;
	struct C2HEvent_Header	*pc2h_evt_hdr;
	struct mlme_ext_priv		*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;

	if ((pcmd_obj = (struct cmd_obj*)_malloc(sizeof(struct cmd_obj))) == NULL)
	{
		return;
	}

	cmdsz = (sizeof(struct joinbss_event) + sizeof(struct C2HEvent_Header));
	if ((pevtcmd = (u8*)_malloc(cmdsz)) == NULL)
	{
		_mfree((u8 *)pcmd_obj, sizeof(struct cmd_obj));
		return;
	}

	_init_listhead(&pcmd_obj->list);

	pcmd_obj->cmdcode = GEN_CMD_CODE(_Set_MLME_EVT);
	pcmd_obj->cmdsz = cmdsz;
	pcmd_obj->parmbuf = pevtcmd;

	pcmd_obj->rsp = NULL;
	pcmd_obj->rspsz  = 0;

	pc2h_evt_hdr = (struct C2HEvent_Header*)(pevtcmd);
	pc2h_evt_hdr->len = sizeof(struct joinbss_event);
	pc2h_evt_hdr->ID = GEN_EVT_CODE(_JoinBss);
	pc2h_evt_hdr->seq = pmlmeext->event_seq++;

	pjoinbss_evt = (struct joinbss_event*)(pevtcmd + sizeof(struct C2HEvent_Header));
	_memcpy((unsigned char *)(&(pjoinbss_evt->network.network)), &(pmlmeinfo->network), sizeof(WLAN_BSSID_EX));
	pjoinbss_evt->network.join_res 	= pjoinbss_evt->network.aid = res;

	DBG_871X("report_join_res(%x)\n", res);

	enqueue_cmd(pcmdpriv, pcmd_obj);

	return;

}

void report_del_sta_event(_adapter *padapter, unsigned char* MacAddr)
{
	struct cmd_obj *pcmd_obj;
	u8	*pevtcmd;
	u32 cmdsz;
	struct stadel_event			*pdel_sta_evt;
	struct C2HEvent_Header	*pc2h_evt_hdr;
	struct mlme_ext_priv		*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;

	if ((pcmd_obj = (struct cmd_obj*)_malloc(sizeof(struct cmd_obj))) == NULL)
	{
		return;
	}

	cmdsz = (sizeof(struct stadel_event) + sizeof(struct C2HEvent_Header));
	if ((pevtcmd = (u8*)_malloc(cmdsz)) == NULL)
	{
		_mfree((u8 *)pcmd_obj, sizeof(struct cmd_obj));
		return;
	}

	_init_listhead(&pcmd_obj->list);

	pcmd_obj->cmdcode = GEN_CMD_CODE(_Set_MLME_EVT);
	pcmd_obj->cmdsz = cmdsz;
	pcmd_obj->parmbuf = pevtcmd;

	pcmd_obj->rsp = NULL;
	pcmd_obj->rspsz  = 0;

	pc2h_evt_hdr = (struct C2HEvent_Header*)(pevtcmd);
	pc2h_evt_hdr->len = sizeof(struct stadel_event);
	pc2h_evt_hdr->ID = GEN_EVT_CODE(_DelSTA);
	pc2h_evt_hdr->seq = pmlmeext->event_seq++;

	pdel_sta_evt = (struct stadel_event*)(pevtcmd + sizeof(struct C2HEvent_Header));
	_memcpy((unsigned char *)(&(pdel_sta_evt->macaddr)), MacAddr, ETH_ALEN);

	DBG_871X("marc: delete STA\n");

	enqueue_cmd(pcmdpriv, pcmd_obj);

	return;
}

void report_add_sta_event(_adapter *padapter, unsigned char* MacAddr, int cam_idx)
{
	struct cmd_obj *pcmd_obj;
	u8	*pevtcmd;
	u32 cmdsz;
	struct stassoc_event		*padd_sta_evt;
	struct C2HEvent_Header	*pc2h_evt_hdr;
	struct mlme_ext_priv		*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;

	if ((pcmd_obj = (struct cmd_obj*)_malloc(sizeof(struct cmd_obj))) == NULL)
	{
		return;
	}

	cmdsz = (sizeof(struct stassoc_event) + sizeof(struct C2HEvent_Header));
	if ((pevtcmd = (u8*)_malloc(cmdsz)) == NULL)
	{
		_mfree((u8 *)pcmd_obj, sizeof(struct cmd_obj));
		return;
	}

	_init_listhead(&pcmd_obj->list);

	pcmd_obj->cmdcode = GEN_CMD_CODE(_Set_MLME_EVT);
	pcmd_obj->cmdsz = cmdsz;
	pcmd_obj->parmbuf = pevtcmd;

	pcmd_obj->rsp = NULL;
	pcmd_obj->rspsz  = 0;

	pc2h_evt_hdr = (struct C2HEvent_Header*)(pevtcmd);
	pc2h_evt_hdr->len = sizeof(struct stassoc_event);
	pc2h_evt_hdr->ID = GEN_EVT_CODE(_AddSTA);
	pc2h_evt_hdr->seq = pmlmeext->event_seq++;

	padd_sta_evt = (struct stassoc_event*)(pevtcmd + sizeof(struct C2HEvent_Header));
	_memcpy((unsigned char *)(&(padd_sta_evt->macaddr)), MacAddr, ETH_ALEN);
	padd_sta_evt->cam_id = cam_idx;

	DBG_871X("report_add_sta_event: add STA\n");

	enqueue_cmd(pcmdpriv, pcmd_obj);

	return;
}


/****************************************************************************

Following are the event callback functions

*****************************************************************************/

//for sta/adhoc mode
static void update_sta_info(_adapter *padapter, struct sta_info *psta)
{
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	//ERP
	VCS_update(padapter, psta);


	//HT
	if(pmlmepriv->htpriv.ht_option)
	{
		psta->htpriv.ht_option = _TRUE;

		psta->htpriv.ampdu_enable = pmlmepriv->htpriv.ampdu_enable;

		if (support_short_GI(padapter, &(pmlmeinfo->HT_caps)))
			psta->htpriv.sgi = _TRUE;

		psta->qos_option = _TRUE;
		
	}
	else
	{
		psta->htpriv.ht_option = _FALSE;

		psta->htpriv.ampdu_enable = _FALSE;
		
		psta->htpriv.sgi = _FALSE;

		psta->qos_option = _FALSE;//?

	}
	
	psta->htpriv.bwmode = pmlmeext->cur_bwmode;
	psta->htpriv.ch_offset = pmlmeext->cur_ch_offset;
	
	psta->htpriv.agg_enable_bitmap = 0x0;//reset
	psta->htpriv.candidate_tid_bitmap = 0x0;//reset
	

	//QoS
	if(pmlmepriv->qospriv.qos_option)
		psta->qos_option = _TRUE;
	

	psta->state = _FW_LINKED;

}

void mlmeext_joinbss_event_callback(_adapter *padapter)
{
	struct sta_info *psta, *psta_bmc;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX 		*cur_network = &(pmlmeinfo->network);
	struct sta_priv	*pstapriv = &padapter->stapriv;
	u8	join_type, init_rts_rate;


	//for bc/mc
	psta_bmc = get_bcmc_stainfo(padapter);
	if(psta_bmc)
	{
		pmlmeinfo->FW_sta_info[4].psta = psta_bmc;

		//update sta_info for bc/mc
		psta_bmc->mac_id = 4;

	}

	psta = get_stainfo(pstapriv, cur_network->MacAddress);
	if (psta)//only for infra. mode
	{
		pmlmeinfo->FW_sta_info[psta->mac_id].psta = psta;//psta->mac_id=5 for infra. mode
	}

	//udpate capability
	update_capinfo(padapter, pmlmeinfo->capability);

	// update IOT-releated issue
	update_IOT_info(padapter);

	//WMM, Update EDCA param
	WMMOnAssocRsp(padapter);

	//HT
	HTOnAssocRsp(padapter);

        //update sta_info
	if (psta) //only for infra. mode
	{
		//DBG_871X("set_sta_rate & update_sta_info\n");
	
		//set per sta rate after updating HT cap.
		set_sta_rate(padapter);
		
		update_sta_info(padapter, psta);
	}	


	//SET MSR to _HW_STATE_STATION_
	Set_NETYPE0_MSR(padapter, (pmlmeinfo->state & 0x3));

        padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_BASIC_RATE, cur_network->SupportedRates);

	padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_BSSID, cur_network->MacAddress);

	//BCN interval
	padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_BEACON_INTERVAL, (u8 *)(&pmlmeinfo->bcn_interval));
	
	join_type = 1;
	padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_MLME_JOIN, (u8 *)(&join_type));

	if((pmlmeinfo->state&0x03) == WIFI_FW_STATION_STATE)
	{	
		// correcting TSF
		correct_TSF(padapter, pmlmeext);
	
		//_set_timer(&pmlmeext->link_timer, DISCONNECT_TO);
		pmlmeext->linked_to = LINKED_TO;
	}	

	//turn on dynamic functions
	Switch_DM_Func(padapter, DYNAMIC_FUNC_DIG|DYNAMIC_FUNC_HP|DYNAMIC_FUNC_SS, _TRUE);

	
	DBG_871X("=>%s\n", __FUNCTION__);

}

void mlmeext_sta_add_event_callback(_adapter *padapter, struct sta_info *psta)
{
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	u8	join_type;

	DBG_871X("%s\n", __FUNCTION__);

	if((pmlmeinfo->state&0x03) == WIFI_FW_ADHOC_STATE)
	{
		if(pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS)//adhoc master or sta_count>1
		{
			//nothing to do
		}
		else//adhoc client
		{			
			//update TSF Value
			//update_TSF(pmlmeext, pframe, len);			

			// correcting TSF
			correct_TSF(padapter, pmlmeext);

			//start beacon
			if(send_beacon(padapter)==_FAIL)
			{
				pmlmeinfo->FW_sta_info[psta->mac_id].status = 0;
					
				pmlmeinfo->state ^= WIFI_FW_ADHOC_STATE;
					
				return;			
			}

			pmlmeinfo->state |= WIFI_FW_ASSOC_SUCCESS;
				
		}

		join_type = 2;
		padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_MLME_JOIN, (u8 *)(&join_type));
	}

	pmlmeinfo->FW_sta_info[psta->mac_id].psta = psta;

	//rate radaptive
	Update_RA_Entry(padapter, psta->mac_id);

	//for bc/mc rate
	Update_RA_Entry(padapter, 4);

	//update adhoc sta_info
	update_sta_info(padapter, psta);

	pmlmeext->linked_to = LINKED_TO;
	
}

void mlmeext_sta_del_event_callback(_adapter *padapter)
{
	struct mlme_ext_priv		*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	if (is_client_associated_to_ap(padapter) || is_IBSS_empty(padapter))
	{
		//set_opmode_cmd(padapter, infra_client_with_mlme);		

		//switch to the 20M Hz mode after disconnect
		pmlmeext->cur_bwmode = HT_CHANNEL_WIDTH_20;
		pmlmeext->cur_ch_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
		
		padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_MLME_DISCONNECT, 0);

		//SelectChannel(padapter, pmlmeext->cur_channel, pmlmeext->cur_ch_offset);
		set_channel_bwmode(padapter, pmlmeext->cur_channel, pmlmeext->cur_ch_offset, pmlmeext->cur_bwmode);
		flush_all_cam_entry(padapter);

		pmlmeinfo->state = WIFI_FW_NULL_STATE;
		
		//set MSR to no link state
		Set_NETYPE0_MSR(padapter, _HW_STATE_NOLINK_);
		
		pmlmeext->linked_to = 0;
		_cancel_timer_ex(&pmlmeext->link_timer);

	}

}

/****************************************************************************

Following are the functions for the timer handlers

*****************************************************************************/

void _linked_rx_signal_strehgth_display(_adapter *padapter)
{
	int	UndecoratedSmoothedPWDB;
	printk("============ linked status check ===================\n");
	printk("pathA Rx SNRdb:%d\n",padapter->recvpriv.RxSNRdB[0]);
	padapter->HalFunc.GetHalDefVarHandler(padapter, HAL_DEF_UNDERCORATEDSMOOTHEDPWDB, &UndecoratedSmoothedPWDB);
	printk("UndecoratedSmoothedPWDB:%d\n",UndecoratedSmoothedPWDB);
	printk("Rx RSSI:%d\n",padapter->recvpriv.rssi);
	printk("Rx Signal_strength:%d\n",padapter->recvpriv.signal_strength);
	printk("Rx Signal_qual:%d \n",padapter->recvpriv.signal_qual);
	printk("============ linked status check ===================\n");
}

void linked_status_chk(_adapter *padapter)
{
	u32	i;
	struct sta_info	*psta;
	struct xmit_priv	*pxmitpriv = &(padapter->xmitpriv);
	struct recv_priv	*precvpriv = &(padapter->recvpriv);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct sta_priv		*pstapriv = &padapter->stapriv;
	struct registry_priv* pregistrypriv = &padapter->registrypriv;

	if (is_client_associated_to_ap(padapter))
	{
		if(padapter->bRxRSSIDisplay)
		 	_linked_rx_signal_strehgth_display(padapter);
	
		//linked infrastructure client mode
		if ((psta = get_stainfo(pstapriv, pmlmeinfo->network.MacAddress)) != NULL)
		{
			/*to monitor whether the AP is alive or not*/
			if (psta->sta_stats.last_rx_pkts == psta->sta_stats.rx_pkts)
			{
				//	Commented by Albert 2010/07/21
				//	In this case, there is no any rx packet received by driver.
				
				if(pmlmeext->retry<3)
				{
					if(pmlmeext->retry==0)
					{
						_memcpy(pmlmeext->sitesurvey_res.ss_ssid, pmlmeinfo->network.Ssid.Ssid, pmlmeinfo->network.Ssid.SsidLength);
						pmlmeext->sitesurvey_res.ss_ssidlen = pmlmeinfo->network.Ssid.SsidLength;
						pmlmeext->sitesurvey_res.scan_mode = SCAN_ACTIVE;
						pmlmeext->sitesurvey_res.state = SCAN_DISABLE;

						//DBG_871X("issue_probereq to check if ap alive, retry=%d\n", pmlmeext->retry);
					
						//	In order to know the AP's current state, try to send the probe request 
						//	to trigger the AP to send the probe response.
						issue_probereq(padapter, 0);
						issue_probereq(padapter, 0);
						issue_probereq(padapter, 0);
					}
					
					pmlmeext->retry++;
					pmlmeext->linked_to = LINKED_TO;					
				}				
				else					
				{				
					pmlmeext->retry = 0;
                                	DBG_871X("no beacon to call receive_disconnect()\n");
					receive_disconnect(padapter, pmlmeinfo->network.MacAddress);
					pmlmeinfo->link_count = 0;
					return;
			       }				
			}
			else
			{
				pmlmeext->retry = 0;
				psta->sta_stats.last_rx_pkts = psta->sta_stats.rx_pkts;
				//_set_timer(&pmlmeext->link_timer, DISCONNECT_TO);
				pmlmeext->linked_to = LINKED_TO;
			}

			/*to send the AP a nulldata if no frame is xmitted in order to keep alive*/
			if (pmlmeinfo->link_count++ == 0)
			{
				pxmitpriv->last_tx_pkts = pxmitpriv->tx_pkts;
			}
			else if ((pmlmeinfo->link_count & 0xf) == 0)
			{
				if ( pxmitpriv->last_tx_pkts == pxmitpriv->tx_pkts)
				{
					//DBG_871X("(Interface %d)issue nulldata to keep alive\n",padapter->dvobjpriv.InterfaceNumber);
					issue_nulldata(padapter, 0);
				}

				pxmitpriv->last_tx_pkts = pxmitpriv->tx_pkts;
			}

		} //end of if ((psta = get_stainfo(pstapriv, passoc_res->network.MacAddress)) != NULL)
	}
	else if (is_client_associated_to_ibss(padapter))
	{
		//linked IBSS mode
		//for each assoc list entry to check the rx pkt counter
		for (i = 6; i < NUM_STA; i++)
		{
			if (pmlmeinfo->FW_sta_info[i].status == 1)
			{
				psta = pmlmeinfo->FW_sta_info[i].psta;
				
				if(NULL==psta) continue;

				if (pmlmeinfo->FW_sta_info[i].rx_pkt == psta->sta_stats.rx_pkts)
				{

					if(pmlmeinfo->FW_sta_info[i].retry<3)
					{
						pmlmeinfo->FW_sta_info[i].retry++;
					}
					else
					{
						pmlmeinfo->FW_sta_info[i].retry = 0;
						pmlmeinfo->FW_sta_info[i].status = 0;
						report_del_sta_event(padapter, psta->hwaddr);
					}	
				}
				else
				{
					pmlmeinfo->FW_sta_info[i].retry = 0;
					pmlmeinfo->FW_sta_info[i].rx_pkt = (u32)psta->sta_stats.rx_pkts;
				}				
			}
		}

		//_set_timer(&pmlmeext->link_timer, DISCONNECT_TO);
		pmlmeext->linked_to = LINKED_TO;

	}

}

void survey_timer_hdl(_adapter *padapter)
{
	struct cmd_obj	*ph2c;
	struct sitesurvey_parm	*psurveyPara;	
	struct cmd_priv					*pcmdpriv=&padapter->cmdpriv;
	struct mlme_priv				*pmlmepriv = &padapter->mlmepriv;
	struct mlme_ext_priv 		*pmlmeext = &padapter->mlmeextpriv;

	//printk("marc: survey timer\n");

	//issue sitesurvey_cmd
	if (pmlmeext->sitesurvey_res.state > SCAN_START)
	{
		if(pmlmeext->sitesurvey_res.state ==  SCAN_PROCESS)
		        pmlmeext->sitesurvey_res.channel_idx++;

		if ((ph2c = (struct cmd_obj*)_malloc(sizeof(struct cmd_obj))) == NULL)
		{
			goto exit_survey_timer_hdl;
		}

		if ((psurveyPara = (struct sitesurvey_parm*)_malloc(sizeof(struct sitesurvey_parm))) == NULL)
		{
			_mfree((unsigned char *)ph2c, sizeof(struct cmd_obj));
			goto exit_survey_timer_hdl;
		}

		init_h2fwcmd_w_parm_no_rsp(ph2c, psurveyPara, GEN_CMD_CODE(_SiteSurvey));
		enqueue_cmd(pcmdpriv, ph2c);
	}
	

exit_survey_timer_hdl:

	return;
}

void link_timer_hdl(_adapter *padapter)
{
	static unsigned int		rx_pkt = 0;
	static u64						tx_cnt = 0;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct sta_priv		*pstapriv = &padapter->stapriv;

	if (pmlmeinfo->state & WIFI_FW_AUTH_NULL)
	{
		DBG_871X("link_timer_hdl:no beacon while connecting\n");
		pmlmeinfo->state = 0;
		report_join_res(padapter, -3);
	}
	else if (pmlmeinfo->state & WIFI_FW_AUTH_STATE)
	{
		//re-auth timer
		if (++pmlmeinfo->reauth_count > REAUTH_LIMIT)
		{
			if (pmlmeinfo->auth_algo != dot11AuthAlgrthm_Auto)
			{
				pmlmeinfo->state = 0;
				report_join_res(padapter, -1);
				return;
			}
			else
			{
				pmlmeinfo->auth_algo = dot11AuthAlgrthm_Shared;
				pmlmeinfo->reauth_count = 0;
			}
		}

		DBG_871X("link_timer_hdl: auth timeout and try again\n");
		pmlmeinfo->auth_seq = 1;
		issue_auth(padapter, NULL, 0);
		_set_timer(&pmlmeext->link_timer, REAUTH_TO);
	}
	else if (pmlmeinfo->state & WIFI_FW_ASSOC_STATE)
	{
		//re-assoc timer
		if (++pmlmeinfo->reassoc_count > REASSOC_LIMIT)
		{
			pmlmeinfo->state = 0;
			report_join_res(padapter, -2);
			return;
		}

		DBG_871X("link_timer_hdl: assoc timeout and try again\n");
		issue_assocreq(padapter);
		_set_timer(&pmlmeext->link_timer, REASSOC_TO);
	}
#if 0
	else if (is_client_associated_to_ap(padapter))
	{
		//linked infrastructure client mode
		if ((psta = get_stainfo(pstapriv, pmlmeinfo->network.MacAddress)) != NULL)
		{
			/*to monitor whether the AP is alive or not*/
			if (rx_pkt == psta->sta_stats.rx_pkts)
			{
				receive_disconnect(padapter, pmlmeinfo->network.MacAddress);
				return;
			}
			else
			{
				rx_pkt = psta->sta_stats.rx_pkts;
				_set_timer(&pmlmeext->link_timer, DISCONNECT_TO);
			}

			//update the EDCA paramter according to the Tx/RX mode
			update_EDCA_param(padapter);

			/*to send the AP a nulldata if no frame is xmitted in order to keep alive*/
			if (pmlmeinfo->link_count++ == 0)
			{
				tx_cnt = pxmitpriv->tx_pkts;
			}
			else if ((pmlmeinfo->link_count & 0xf) == 0)
			{
				if (tx_cnt == pxmitpriv->tx_pkts)
				{
					issue_nulldata(padapter, 0);
				}

				tx_cnt = pxmitpriv->tx_pkts;
			}
		} //end of if ((psta = get_stainfo(pstapriv, passoc_res->network.MacAddress)) != NULL)
	}
	else if (is_client_associated_to_ibss(padapter))
	{
		//linked IBSS mode
		//for each assoc list entry to check the rx pkt counter
		for (i = 6; i < NUM_STA; i++)
		{
			if (pmlmeinfo->FW_sta_info[i].status == 1)
			{
				psta = pmlmeinfo->FW_sta_info[i].psta;

				if (pmlmeinfo->FW_sta_info[i].rx_pkt == psta->sta_stats.rx_pkts)
				{
					pmlmeinfo->FW_sta_info[i].status = 0;
					report_del_sta_event(padapter, psta->hwaddr);
				}
				else
				{
					pmlmeinfo->FW_sta_info[i].rx_pkt = psta->sta_stats.rx_pkts;
				}
			}
		}

		_set_timer(&pmlmeext->link_timer, DISCONNECT_TO);
	}
#endif

	return;
}

void addba_timer_hdl(struct sta_info *psta)
{
	u8 bitmap;
	u16 tid;
	struct ht_priv	*phtpriv;

	if(!psta)
		return;
	
	phtpriv = &psta->htpriv;

	if((phtpriv->ht_option==1) && (phtpriv->ampdu_enable==_TRUE)) 
	{
		if(phtpriv->candidate_tid_bitmap)
			phtpriv->candidate_tid_bitmap=0x0;
		
	}
	
}

u8 NULL_hdl(_adapter *padapter, u8 *pbuf)
{
	return H2C_SUCCESS;
}

u8 setopmode_hdl(_adapter *padapter, u8 *pbuf)
{
	u8	type;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct setopmode_parm *psetop = (struct setopmode_parm *)pbuf;

	if(psetop->mode == Ndis802_11APMode)
	{
		pmlmeinfo->state = WIFI_FW_AP_STATE;
		type = _HW_STATE_AP_;
#ifdef CONFIG_NATIVEAP_MLME
		//start_ap_mode(padapter);
#endif
	}
	else if(psetop->mode == Ndis802_11Infrastructure)
	{
		type = _HW_STATE_STATION_;
	}
	else if(psetop->mode == Ndis802_11IBSS)
	{
		type = _HW_STATE_ADHOC_;
	}
	else
	{
		type = _HW_STATE_NOLINK_;
	}

	padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_SET_OPMODE, (u8 *)(&type));
	//Set_NETYPE0_MSR(padapter, type);

	return H2C_SUCCESS;
	
}

u8 createbss_hdl(_adapter *padapter, u8 *pbuf)
{
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX	*pnetwork = (WLAN_BSSID_EX*)(&(pmlmeinfo->network));
	struct joinbss_parm *pparm = (struct joinbss_parm *)pbuf;
	u32	initialgain;

	
	if(pparm->network.InfrastructureMode == Ndis802_11APMode)
	{
#ifdef CONFIG_AP_MODE
	
		if(pmlmeinfo->state == WIFI_FW_AP_STATE)
		{		
			//todo:
			return H2C_SUCCESS;		
		}		
#endif
	}

	//below is for ad-hoc master
	if(pparm->network.InfrastructureMode == Ndis802_11IBSS)
	{
		joinbss_reset(padapter);

		pmlmeext->linked_to = 0;
	
		pmlmeext->cur_bwmode = HT_CHANNEL_WIDTH_20;
		pmlmeext->cur_ch_offset= HAL_PRIME_CHNL_OFFSET_DONT_CARE;	
		pmlmeinfo->ERP_enable = 0;
		pmlmeinfo->WMM_enable = 0;
		pmlmeinfo->HT_enable = 0;
		pmlmeinfo->HT_caps_enable = 0;
		pmlmeinfo->HT_info_enable = 0;
		pmlmeinfo->agg_enable_bitmap = 0;
		pmlmeinfo->candidate_tid_bitmap = 0;

		//disable dynamic functions, such as high power, DIG
		Save_DM_Func_Flag(padapter);
		Switch_DM_Func(padapter, DYNAMIC_FUNC_DISABLE, _FALSE);

		//config the initial gain under linking, need to write the BB registers
		initialgain = 0x30;
		padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_INITIAL_GAIN, (u8 *)(&initialgain));

		//cancel link timer 
		_cancel_timer_ex(&pmlmeext->link_timer);

		//clear CAM
		flush_all_cam_entry(padapter);	

		_memcpy(pnetwork, pbuf, FIELD_OFFSET(WLAN_BSSID_EX, IELength)); 
		pnetwork->IELength = ((WLAN_BSSID_EX *)pbuf)->IELength;

		if(pnetwork->IELength>MAX_IE_SZ)//Check pbuf->IELength
			return H2C_PARAMETERS_ERROR;

		_memcpy(pnetwork->IEs, ((WLAN_BSSID_EX *)pbuf)->IEs, pnetwork->IELength);
	
		start_create_ibss(padapter);

	}	

	return H2C_SUCCESS;

}

u8 join_cmd_hdl(_adapter *padapter, u8 *pbuf)
{
	u8	join_type;
	PNDIS_802_11_VARIABLE_IEs	pIE;
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX	*pnetwork = (WLAN_BSSID_EX*)(&(pmlmeinfo->network));
	struct joinbss_parm *pparm = (struct joinbss_parm *)pbuf;
	u32	acparm, initialgain, i;

	//check already connecting to AP or not
	if (pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS)
	{
		if (pmlmeinfo->state & WIFI_FW_STATION_STATE)
		{
			issue_deauth(padapter, pnetwork->MacAddress, WLAN_REASON_DEAUTH_LEAVING);
		}

		pmlmeinfo->state = WIFI_FW_NULL_STATE;
		
		//clear CAM
		flush_all_cam_entry(padapter);		
		
		_cancel_timer_ex(&pmlmeext->link_timer);
		
		//set MSR to nolink		
		Set_NETYPE0_MSR(padapter, _HW_STATE_NOLINK_);	

		padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_MLME_DISCONNECT, 0);
	}

#ifdef CONFIG_ANTENNA_DIVERSITY
	padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_ANTENNA_DIVERSITY_JOIN, (u8 *)(&pparm->network.PhyInfo.Optimum_antenna));
#endif

	joinbss_reset(padapter);

	pmlmeext->linked_to = 0;
	
	pmlmeext->cur_bwmode = HT_CHANNEL_WIDTH_20;
	pmlmeext->cur_ch_offset= HAL_PRIME_CHNL_OFFSET_DONT_CARE;	
	pmlmeinfo->ERP_enable = 0;
	pmlmeinfo->WMM_enable = 0;
	pmlmeinfo->HT_enable = 0;
	pmlmeinfo->HT_caps_enable = 0;
	pmlmeinfo->HT_info_enable = 0;
	pmlmeinfo->agg_enable_bitmap = 0;
	pmlmeinfo->candidate_tid_bitmap = 0;	
	pmlmeinfo->bwmode_updated = _FALSE;
	//pmlmeinfo->assoc_AP_vendor = maxAP;

       	_memcpy(pnetwork, pbuf, FIELD_OFFSET(WLAN_BSSID_EX, IELength)); 
	pnetwork->IELength = ((WLAN_BSSID_EX *)pbuf)->IELength;
	
	if(pnetwork->IELength>MAX_IE_SZ)//Check pbuf->IELength
		return H2C_PARAMETERS_ERROR;	
		
	_memcpy(pnetwork->IEs, ((WLAN_BSSID_EX *)pbuf)->IEs, pnetwork->IELength); 

	//Check AP vendor
	pmlmeinfo->assoc_AP_vendor = check_assoc_AP(pnetwork->IEs, pnetwork->IELength);

	for (i = sizeof(NDIS_802_11_FIXED_IEs); i < pnetwork->IELength;)
	{
		pIE = (PNDIS_802_11_VARIABLE_IEs)(pnetwork->IEs + i);

		switch (pIE->ElementID)
		{
			case _VENDOR_SPECIFIC_IE_://Get WMM IE.
				if ( _memcmp(pIE->data, WMM_OUI, 4) )
				{
					pmlmeinfo->WMM_enable = 1;
				}
				break;

			case _HT_CAPABILITY_IE_:	//Get HT Cap IE.
				pmlmeinfo->HT_caps_enable = 1;
				break;

			case _HT_EXTRA_INFO_IE_:	//Get HT Info IE.
				pmlmeinfo->HT_info_enable = 1;

				//spec case only for cisco's ap because cisco's ap issue assoc rsp using mcs rate @40MHz or @20MHz	
				if(pmlmeinfo->assoc_AP_vendor == ciscoAP)
				{
					struct HT_info_element *pht_info = (struct HT_info_element *)(pIE->data);
							
					if ((pregpriv->cbw40_enable) &&	 (pht_info->infos[0] & BIT(2)))
					{
						//switch to the 40M Hz mode according to the AP
						pmlmeext->cur_bwmode = HT_CHANNEL_WIDTH_40;
						switch (pht_info->infos[0] & 0x3)
						{
							case 1:
								pmlmeext->cur_ch_offset = HAL_PRIME_CHNL_OFFSET_LOWER;
								break;
			
							case 3:
								pmlmeext->cur_ch_offset = HAL_PRIME_CHNL_OFFSET_UPPER;
								break;
				
							default:
								pmlmeext->cur_ch_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
								break;
						}
								
						DBG_871X("set ch/bw for cisco's ap before connected\n");
					}
				}
				break;

			default:
				break;
		}

		i += (pIE->Length + 2);
	}

	if (padapter->registrypriv.wifi_spec) {
		// for WiFi test, follow WMM test plan spec
		acparm = 0x002F431C; // VO
		padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_AC_PARAM_VO, (u8 *)(&acparm));
		acparm = 0x005E541C; // VI
		padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_AC_PARAM_VI, (u8 *)(&acparm));
		acparm = 0x0000A525; // BE
		padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_AC_PARAM_BE, (u8 *)(&acparm));
		acparm = 0x0000A549; // BK
		padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_AC_PARAM_BK, (u8 *)(&acparm));
	
		// for WiFi test, mixed mode with intel STA under bg mode throughput issue
		if (padapter->mlmepriv.htpriv.ht_option == 0){
			acparm = 0x00004320;
			padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_AC_PARAM_BE, (u8 *)(&acparm));
		}
	}
	else {
		acparm = 0x002F3217; // VO
		padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_AC_PARAM_VO, (u8 *)(&acparm));
		acparm = 0x005E4317; // VI
		padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_AC_PARAM_VI, (u8 *)(&acparm));
		acparm = 0x00105320; // BE
		padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_AC_PARAM_BE, (u8 *)(&acparm));
		acparm = 0x0000A444; // BK
		padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_AC_PARAM_BK, (u8 *)(&acparm));
	}
	
	//disable dynamic functions, such as high power, DIG
	//Switch_DM_Func(padapter, DYNAMIC_FUNC_DISABLE, _FALSE);

	//config the initial gain under linking, need to write the BB registers
	initialgain = 0x32;
	padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_INITIAL_GAIN, (u8 *)(&initialgain));

	//set MSR to nolink		
	Set_NETYPE0_MSR(padapter, _HW_STATE_NOLINK_);		

	join_type = 0;
	padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_MLME_JOIN, (u8 *)(&join_type));
	
	//cancel link timer 
	_cancel_timer_ex(&pmlmeext->link_timer);
	
	start_clnt_join(padapter);

	return H2C_SUCCESS;
	
}

u8 disconnect_hdl(_adapter *padapter, unsigned char *pbuf)
{
	struct setauth_parm	*pparm = (struct setauth_parm *)pbuf;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX		*pnetwork = (WLAN_BSSID_EX*)(&(pmlmeinfo->network));
	u8	val8;
	
	if (is_client_associated_to_ap(padapter))
	{
		issue_deauth(padapter, pnetwork->MacAddress, WLAN_REASON_DEAUTH_LEAVING);
	}

	//set_opmode_cmd(padapter, infra_client_with_mlme);

	pmlmeinfo->state = WIFI_FW_NULL_STATE;
	
	//switch to the 20M Hz mode after disconnect
	pmlmeext->cur_bwmode = HT_CHANNEL_WIDTH_20;
	pmlmeext->cur_ch_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;	
		
	//set MSR to no link state
	Set_NETYPE0_MSR(padapter, _HW_STATE_NOLINK_);		

	padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_MLME_DISCONNECT, 0);
	
	if(((pmlmeinfo->state&0x03) == WIFI_FW_ADHOC_STATE) || ((pmlmeinfo->state&0x03) == WIFI_FW_AP_STATE))
	{
		//Stop BCN
		val8 = 0;
		padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_BCN_FUNC, (u8 *)(&val8));
	}

	pmlmeinfo->state = WIFI_FW_NULL_STATE;
	
	set_channel_bwmode(padapter, pmlmeext->cur_channel, pmlmeext->cur_ch_offset, pmlmeext->cur_bwmode);

	flush_all_cam_entry(padapter);
		
	_cancel_timer_ex(&pmlmeext->link_timer);
	pmlmeext->linked_to = 0;
	
	return 	H2C_SUCCESS;
}

u8 sitesurvey_cmd_hdl(_adapter *padapter, u8 *pbuf)
{
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct sitesurvey_parm	*pparm = (struct sitesurvey_parm *)pbuf;
	u8	val8;
	u32	initialgain;
		
	if (pmlmeext->sitesurvey_res.state == SCAN_DISABLE)
	{
		//for first time sitesurvey_cmd
		pmlmeext->sitesurvey_res.state = SCAN_START;
		pmlmeext->sitesurvey_res.bss_cnt = 0;
		pmlmeext->sitesurvey_res.channel_idx = 0;
		
		if (le32_to_cpu(pparm->ss_ssidlen))
		{
			_memcpy(pmlmeext->sitesurvey_res.ss_ssid, pparm->ss_ssid, le32_to_cpu(pparm->ss_ssidlen));
		}	
		else
		{
			_memset(pmlmeext->sitesurvey_res.ss_ssid, 0, (IW_ESSID_MAX_SIZE + 1));
		}	
		
		pmlmeext->sitesurvey_res.ss_ssidlen = le32_to_cpu(pparm->ss_ssidlen);
	
		pmlmeext->sitesurvey_res.scan_mode = le32_to_cpu(pparm->scan_mode);

		//issue null data if associating to the AP
		if (is_client_associated_to_ap(padapter) == _TRUE)
		{
			pmlmeext->sitesurvey_res.state = SCAN_TXNULL;

			issue_nulldata(padapter, 1);

			//delay 20ms to protect nulldata(1).
			_set_timer(&pmlmeext->survey_timer, 20);

			return H2C_SUCCESS;
		}
	}

	if ((pmlmeext->sitesurvey_res.state == SCAN_START) || (pmlmeext->sitesurvey_res.state == SCAN_TXNULL))
	{
		//disable dynamic functions, such as high power, DIG
		Save_DM_Func_Flag(padapter);
		Switch_DM_Func(padapter, DYNAMIC_FUNC_DISABLE, _FALSE);

		//config the initial gain under scaning, need to write the BB registers
		initialgain = 0x20;
		padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_INITIAL_GAIN, (u8 *)(&initialgain));		
		
		//set MSR to no link state
		Set_NETYPE0_MSR(padapter, _HW_STATE_NOLINK_);		

		val8 = 1; //before site survey
		padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_MLME_SITESURVEY, (u8 *)(&val8));

		pmlmeext->sitesurvey_res.state = SCAN_PROCESS;
	}
		
	site_survey(padapter);

	return H2C_SUCCESS;
	
}

u8 setauth_hdl(_adapter *padapter, unsigned char *pbuf)
{
	struct setauth_parm		*pparm = (struct setauth_parm *)pbuf;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	
	if (pparm->mode < 4)
	{
		pmlmeinfo->auth_algo = pparm->mode;
	}

	return 	H2C_SUCCESS;
}

u8 setkey_hdl(_adapter *padapter, u8 *pbuf)
{
	unsigned short				ctrl;
	struct setkey_parm		*pparm = (struct setkey_parm *)pbuf;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	unsigned char					null_sta[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	pmlmeinfo->key_index = pparm->keyid;
	
	//write cam
	ctrl = BIT(15) | ((pparm->algorithm) << 2) | pparm->keyid;	
	
	write_cam(padapter, pparm->keyid, ctrl, null_sta, pparm->key);
	
	return H2C_SUCCESS;
}

u8 set_stakey_hdl(_adapter *padapter, u8 *pbuf)
{
	unsigned short ctrl=0;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct set_stakey_parm	*pparm = (struct set_stakey_parm *)pbuf;
	
	if((pmlmeinfo->state&0x03) == WIFI_FW_AP_STATE)
	{
		unsigned char cam_id;//cam_entry
		struct sta_info *psta;
		struct sta_priv *pstapriv = &padapter->stapriv;
		
		psta = get_stainfo(pstapriv, pparm->addr);
		if(psta)
		{			
			ctrl = (BIT(15) | ((pparm->algorithm) << 2));

			DBG_8192C("r871x_set_stakey_hdl(): enc_algorithm=%d\n", pparm->algorithm);

			if((psta->mac_id<1) || (psta->mac_id>(NUM_STA-4)))
			{
				DBG_8192C("r871x_set_stakey_hdl():set_stakey failed, mac_id(aid)=%d\n", psta->mac_id);
				return H2C_REJECTED;
			}	
				 
			cam_id = (psta->mac_id + 3);//0~3 for default key, cmd_id=aid/macid + 3

			DBG_8192C("Write CAM, mac_addr=%x:%x:%x:%x:%x:%x, cam_entry=%d\n", pparm->addr[0], 
						pparm->addr[1], pparm->addr[2], pparm->addr[3], pparm->addr[4],
						pparm->addr[5], cam_id);

			write_cam(padapter, cam_id, ctrl, pparm->addr, pparm->key);
	
			return H2C_SUCCESS_RSP;
		
		}
		else
		{
			DBG_8192C("r871x_set_stakey_hdl(): sta has been free\n");
			return H2C_REJECTED;
		}
		
	}

	//below for sta mode
	
	ctrl = BIT(15) | ((pparm->algorithm) << 2);	

	write_cam(padapter, 5, ctrl, pparm->addr, pparm->key);

	pmlmeinfo->enc_algo = pparm->algorithm;
	
	return H2C_SUCCESS;
}

u8 add_ba_hdl(_adapter *padapter, unsigned char *pbuf)
{
	struct addBaReq_parm 	*pparm = (struct addBaReq_parm *)pbuf;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	struct sta_info *psta = get_stainfo(&padapter->stapriv, pparm->addr);
	
	if(!psta)
		return 	H2C_SUCCESS;
		

	if (((pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS) && (pmlmeinfo->HT_enable)) ||
		((pmlmeinfo->state&0x03) == WIFI_FW_AP_STATE))
	{
		//pmlmeinfo->ADDBA_retry_count = 0;
		//pmlmeinfo->candidate_tid_bitmap |= (0x1 << pparm->tid);		
		//psta->htpriv.candidate_tid_bitmap |= BIT(pparm->tid);
		issue_action_BA(padapter, pparm->addr, WLAN_ACTION_ADDBA_REQ, (u16)pparm->tid);		
		//_set_timer(&pmlmeext->ADDBA_timer, ADDBA_TO);
		_set_timer(&psta->addba_retry_timer, ADDBA_TO);
	}
	else
	{		
		psta->htpriv.candidate_tid_bitmap &= ~BIT(pparm->tid);		
	}
	
	return 	H2C_SUCCESS;
}

u8 set_tx_beacon_cmd(_adapter* padapter)
{
	struct cmd_obj	*ph2c;
	struct Tx_Beacon_param 	*ptxBeacon_parm;	
	struct cmd_priv	*pcmdpriv = &(padapter->cmdpriv);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	u8	res = _SUCCESS;
	
_func_enter_;	
	
	if ((ph2c = (struct cmd_obj*)_malloc(sizeof(struct cmd_obj))) == NULL)
	{
		res= _FAIL;
		goto exit;
	}
	
	if ((ptxBeacon_parm = (struct Tx_Beacon_param *)_malloc(sizeof(struct Tx_Beacon_param))) == NULL)
	{
		_mfree((unsigned char *)ph2c, sizeof(struct	cmd_obj));
		res= _FAIL;
		goto exit;
	}

	_memcpy(&(ptxBeacon_parm->network), &(pmlmeinfo->network), sizeof(WLAN_BSSID_EX));
	init_h2fwcmd_w_parm_no_rsp(ph2c, ptxBeacon_parm, GEN_CMD_CODE(_TX_Beacon));

	enqueue_cmd_ex(pcmdpriv, ph2c);

	
exit:
	
_func_exit_;

	return res;
}


u8 mlme_evt_hdl(_adapter *padapter, unsigned char *pbuf)
{
	u8 evt_code, evt_seq;
	u16 evt_sz;
	uint 	*peventbuf;
	void (*event_callback)(_adapter *dev, u8 *pbuf);
	struct evt_priv *pevt_priv = &(padapter->evtpriv);

	peventbuf = (uint*)pbuf;
	evt_sz = (u16)(*peventbuf&0xffff);
	evt_seq = (u8)((*peventbuf>>24)&0x7f);
	evt_code = (u8)((*peventbuf>>16)&0xff);
	
		
	// checking event sequence...		
	if ((evt_seq & 0x7f) != pevt_priv->event_seq)
	{
		RT_TRACE(_module_rtl871x_cmd_c_,_drv_info_,("Evetn Seq Error! %d vs %d\n", (evt_seq & 0x7f), pevt_priv->event_seq));
	
		pevt_priv->event_seq = (evt_seq+1)&0x7f;

		goto _abort_event_;
	}

	// checking if event code is valid
	if (evt_code >= MAX_C2HEVT)
	{
		RT_TRACE(_module_rtl871x_cmd_c_,_drv_err_,("\nEvent Code(%d) mismatch!\n", evt_code));
		goto _abort_event_;
	}

	// checking if event size match the event parm size	
	if ((wlanevents[evt_code].parmsize != 0) && 
			(wlanevents[evt_code].parmsize != evt_sz))
	{
			
		RT_TRACE(_module_rtl871x_cmd_c_,_drv_err_,("\nEvent(%d) Parm Size mismatch (%d vs %d)!\n", 
			evt_code, wlanevents[evt_code].parmsize, evt_sz));
		goto _abort_event_;	
			
	}

	pevt_priv->event_seq = (pevt_priv->event_seq+1)&0x7f;//update evt_seq

	peventbuf += 2;
				
	if(peventbuf)
	{
		event_callback = wlanevents[evt_code].event_callback;
		event_callback(padapter, (u8*)peventbuf);

		pevt_priv->evt_done_cnt++;
	}


_abort_event_:


	return H2C_SUCCESS;
		
}

u8 h2c_msg_hdl(_adapter *padapter, unsigned char *pbuf)
{
	if(!pbuf)
		return H2C_PARAMETERS_ERROR;

	return H2C_SUCCESS;
}


u8 tx_beacon_hdl(_adapter *padapter, unsigned char *pbuf)
{
	if(send_beacon(padapter)==_FAIL)
	{
		DBG_871X("issue_beacon, fail!\n");
		return H2C_PARAMETERS_ERROR;
	}	
#ifdef CONFIG_AP_MODE
	else //tx bc/mc frames after update TIM 
	{	
		_irqL irqL;
		struct sta_info *psta_bmc;
		_list	*xmitframe_plist, *xmitframe_phead;
		struct xmit_frame *pxmitframe=NULL;
		struct sta_priv  *pstapriv = &padapter->stapriv;
		
		//for BC/MC Frames
		psta_bmc = get_bcmc_stainfo(padapter);
		if(!psta_bmc)
			return H2C_SUCCESS;
	
		if((pstapriv->tim_bitmap&BIT(0)) && (psta_bmc->sleepq_len>0))
		{				
			_enter_critical_bh(&psta_bmc->sleep_q.lock, &irqL);	

			xmitframe_phead = get_list_head(&psta_bmc->sleep_q);
			xmitframe_plist = get_next(xmitframe_phead);

			while ((end_of_queue_search(xmitframe_phead, xmitframe_plist)) == _FALSE)
			{			
				pxmitframe = LIST_CONTAINOR(xmitframe_plist, struct xmit_frame, list);

				xmitframe_plist = get_next(xmitframe_plist);

				list_delete(&pxmitframe->list);

				psta_bmc->sleepq_len--;

				pxmitframe->attrib.triggered=1;

				if(padapter->HalFunc.hal_xmit(padapter, pxmitframe) == _TRUE)
				{		
					os_xmit_complete(padapter, pxmitframe);
				}

				//pstapriv->tim_bitmap &= ~BIT(0);				
		
			}	
	
			_exit_critical_bh(&psta_bmc->sleep_q.lock, &irqL);	

		}

	}
#endif

	return H2C_SUCCESS;
	
}

#ifdef CONFIG_AP_MODE

void init_mlme_ap_info(_adapter *padapter)
{
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;

	pmlmeext->bstart_bss = _FALSE;
}

static void update_BCNTIM(_adapter *padapter)
{
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	WLAN_BSSID_EX *pnetwork = (WLAN_BSSID_EX *)&pmlmepriv->cur_network.network;
	struct mlme_ext_priv *pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX *pnetwork_mlmeext = &(pmlmeinfo->network);
	unsigned char *src_ie = pnetwork->IEs;
	unsigned int src_ielen = pnetwork->IELength;
	unsigned char *dst_ie = pnetwork_mlmeext->IEs;

	
	//update TIM IE
	if(pstapriv->tim_bitmap)
	{
		u8 *p, ie_len;
		u16 tim_bitmap_le;
		u32 tmp_len, head_len=0;

		tim_bitmap_le = cpu_to_le16(pstapriv->tim_bitmap);
	
		//calucate head_len		
		head_len = _FIXED_IE_LENGTH_;
		head_len += pnetwork->Ssid.SsidLength + 2;

		// get supported rates len
		p = get_ie(src_ie + _BEACON_IE_OFFSET_, _SUPPORTEDRATES_IE_, &tmp_len, (pnetwork->IELength - _BEACON_IE_OFFSET_));	
		if (p !=  NULL) 
		{			
			head_len += tmp_len+2;
		}

		//DS Parameter Set IE, len=3	
		head_len += 3;

		//copy head offset
		_memcpy(dst_ie, src_ie, head_len);
		

		//append TIM IE from head_len offset
		dst_ie+=head_len;

		*dst_ie++=_TIM_IE_;

		if((pstapriv->tim_bitmap&0xff00) && (pstapriv->tim_bitmap&0x00fc))			
			ie_len = 5;
		else
			ie_len = 4;

		*dst_ie++= ie_len;
		
		*dst_ie++=0;//DTIM count
		*dst_ie++=1;//DTIM peroid
		
		if(pstapriv->tim_bitmap&BIT(0))//for bc/mc frames
			*dst_ie++ = BIT(0);//bitmap ctrl 
		else
			*dst_ie++ = 0;

		if(ie_len==4)
		{
			*dst_ie++ = *(u8*)&tim_bitmap_le;
		}	
		else if(ie_len==5)
		{
			_memcpy(dst_ie, &tim_bitmap_le, 2);
			dst_ie+=2;				
		}	
		
		//copy remainder IE
		_memcpy(dst_ie, src_ie+head_len, src_ielen-head_len);

		//pnetwork_mlmeext->Length += ie_len+2;
		//pnetwork_mlmeext->IELength += ie_len+2;
		pnetwork_mlmeext->Length = pnetwork->Length+ie_len+2;
		pnetwork_mlmeext->IELength = src_ielen+ie_len+2;
		
	}
	else
	{
		_memcpy(dst_ie, src_ie, src_ielen);
		pnetwork_mlmeext->Length = pnetwork->Length;
		pnetwork_mlmeext->IELength = src_ielen;
	}


	set_tx_beacon_cmd(padapter);


/*
	if(send_beacon(padapter)==_FAIL)
	{
		DBG_871X("issue_beacon, fail!\n");
	}
*/

}

u8 chk_sta_is_alive(struct sta_info *psta)
{	
	struct stainfo_stats *pstats;

	pstats = &psta->sta_stats;

	if(pstats->rx_pkts == pstats->last_rx_pkts)
	{
		if(psta->state&WIFI_SLEEP_STATE)
			return _TRUE;
		else
			return _FALSE;
	}
	else
	{
		pstats->last_rx_pkts = pstats->rx_pkts;
		
		return _TRUE;
	}
	
}

void	expire_timeout_chk(_adapter *padapter)
{
	_list	*phead, *plist;
	struct sta_info *psta=NULL;	
	struct sta_priv *pstapriv = &padapter->stapriv;	

	phead = &pstapriv->auth_list;
	plist = get_next(phead);

	//check auth_queue
	while ((end_of_queue_search(phead, plist)) == _FALSE)	
	{
		psta = LIST_CONTAINOR(plist, struct sta_info, auth_list);
		
		plist = get_next(plist);
	
		if(psta->expire_to>0)
		{
			psta->expire_to--;
			if (psta->expire_to == 0)
			{				
				_irqL irqL;
				
				list_delete(&psta->auth_list);
				
				DBG_871X("auth expire %02X%02X%02X%02X%02X%02X\n",
					psta->hwaddr[0],psta->hwaddr[1],psta->hwaddr[2],psta->hwaddr[3],psta->hwaddr[4],psta->hwaddr[5]);
				
				_enter_critical_bh(&(pstapriv->sta_hash_lock), &irqL);	
				free_stainfo(padapter, psta);
				_exit_critical_bh(&(pstapriv->sta_hash_lock), &irqL);	
			}	
		}		
		
	}


	psta = NULL;
	
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);

	//check asoc_queue
	while ((end_of_queue_search(phead, plist)) == _FALSE)	
	{
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		
		plist = get_next(plist);
	
		if(chk_sta_is_alive(psta))
		{
			psta->expire_to = pstapriv->expire_to;
		}		
	
		if(psta->expire_to>0)
		{
			psta->expire_to--;
			if (psta->expire_to == 0)
			{				
				_irqL irqL;
			
				list_delete(&psta->asoc_list);
				
				DBG_871X("asoc expire %02X%02X%02X%02X%02X%02X\n",
					psta->hwaddr[0],psta->hwaddr[1],psta->hwaddr[2],psta->hwaddr[3],psta->hwaddr[4],psta->hwaddr[5]);
#if 0
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
#endif
				ap_free_sta(padapter, psta);
				
			}			
		}	
		
	}

}


static void add_RATid(_adapter *padapter, struct sta_info *psta)
{	
	int i;
	u8 rf_type;
	u32 init_rate=0;
	unsigned char	 sta_band, raid, shortGIrate = _FALSE;
	unsigned char limit;	
	unsigned int tx_ra_bitmap=0;
	struct ht_priv	*psta_ht = NULL;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	
	if(psta)
		psta_ht = &psta->htpriv;
	else
		return;
	
	//b/g mode ra_bitmap  
	for (i=0; i<sizeof(psta->bssrateset); i++)
	{
		if (psta->bssrateset[i])
			tx_ra_bitmap |= get_bit_value_from_ieee_value(psta->bssrateset[i]&0x7f);
	}

	//n mode ra_bitmap
	if(psta_ht->ht_option) 
	{
		padapter->HalFunc.GetHwRegHandler(padapter, HW_VAR_RF_TYPE, (u8 *)(&rf_type));
		if(rf_type == RF_2T2R)
			limit=16;// 2R
		else
			limit=8;//  1R

		for (i=0; i<limit; i++) {
			if (psta_ht->ht_cap.supp_mcs_set[i/8] & BIT(i%8))
				tx_ra_bitmap |= BIT(i+12);
		}

		//max short GI rate
		shortGIrate = psta_ht->sgi;
	}


#if 0//gtest
	if(get_rf_mimo_mode(padapter) == RTL8712_RF_2T2R)
	{
		//is this a 2r STA?
		if((pstat->tx_ra_bitmap & 0x0ff00000) != 0 && !(priv->pshare->has_2r_sta & BIT(pstat->aid)))
		{
			priv->pshare->has_2r_sta |= BIT(pstat->aid);
			if(read16(padapter, 0x102501f6) != 0xffff)
			{
				write16(padapter, 0x102501f6, 0xffff);
				reset_1r_sta_RA(priv, 0xffff);
				Switch_1SS_Antenna(priv, 3);
			}
		}
		else// bg or 1R STA? 
		{ 
			if((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && pstat->ht_cap_len && priv->pshare->has_2r_sta == 0)
			{
				if(read16(padapter, 0x102501f6) != 0x7777)
				{ // MCS7 SGI
					write16(padapter, 0x102501f6,0x7777);
					reset_1r_sta_RA(priv, 0x7777);
					Switch_1SS_Antenna(priv, 2);
				}
			}
		}
		
	}

	if ((pstat->rssi_level < 1) || (pstat->rssi_level > 3)) 
	{
		if (pstat->rssi >= priv->pshare->rf_ft_var.raGoDownUpper)
			pstat->rssi_level = 1;
		else if ((pstat->rssi >= priv->pshare->rf_ft_var.raGoDown20MLower) ||
			((priv->pshare->is_40m_bw) && (pstat->ht_cap_len) &&
			(pstat->rssi >= priv->pshare->rf_ft_var.raGoDown40MLower) &&
			(pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))))
			pstat->rssi_level = 2;
		else
			pstat->rssi_level = 3;
	}

	// rate adaptive by rssi
	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && pstat->ht_cap_len)
	{
		if ((get_rf_mimo_mode(priv) == MIMO_1T2R) || (get_rf_mimo_mode(priv) == MIMO_1T1R))
		{
			switch (pstat->rssi_level) {
				case 1:
					pstat->tx_ra_bitmap &= 0x100f0000;
					break;
				case 2:
					pstat->tx_ra_bitmap &= 0x100ff000;
					break;
				case 3:
					if (priv->pshare->is_40m_bw)
						pstat->tx_ra_bitmap &= 0x100ff005;
					else
						pstat->tx_ra_bitmap &= 0x100ff001;

					break;
			}
		}
		else 
		{
			switch (pstat->rssi_level) {
				case 1:
					pstat->tx_ra_bitmap &= 0x1f0f0000;
					break;
				case 2:
					pstat->tx_ra_bitmap &= 0x1f0ff000;
					break;
				case 3:
					if (priv->pshare->is_40m_bw)
						pstat->tx_ra_bitmap &= 0x000ff005;
					else
						pstat->tx_ra_bitmap &= 0x000ff001;

					break;
			}

			// Don't need to mask high rates due to new rate adaptive parameters
			//if (pstat->is_broadcom_sta)		// use MCS12 as the highest rate vs. Broadcom sta
			//	pstat->tx_ra_bitmap &= 0x81ffffff;

			// NIC driver will report not supporting MCS15 and MCS14 in asoc req
			//if (pstat->is_rtl8190_sta && !pstat->is_2t_mimo_sta)
			//	pstat->tx_ra_bitmap &= 0x83ffffff;		// if Realtek 1x2 sta, don't use MCS15 and MCS14
		}
	}
	else if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) && isErpSta(pstat))
	{
		switch (pstat->rssi_level) {
			case 1:
				pstat->tx_ra_bitmap &= 0x00000f00;
				break;
			case 2:
				pstat->tx_ra_bitmap &= 0x00000ff0;
				break;
			case 3:
				pstat->tx_ra_bitmap &= 0x00000ff5;
				break;
		}
	}
	else 
	{
		pstat->tx_ra_bitmap &= 0x0000000d;
	}

	// disable tx short GI when station cannot rx MCS15(AP is 2T2R)
	// disable tx short GI when station cannot rx MCS7 (AP is 1T2R or 1T1R)
	// if there is only 1r STA and we are 2T2R, DO NOT mask SGI rate
	if ((!(pstat->tx_ra_bitmap & 0x8000000) && (priv->pshare->has_2r_sta > 0) && (get_rf_mimo_mode(padapter) == RTL8712_RF_2T2R)) ||
		 (!(pstat->tx_ra_bitmap & 0x80000) && (get_rf_mimo_mode(padapter) != RTL8712_RF_2T2R)))
	{
		pstat->tx_ra_bitmap &= ~BIT(28);	
	}
#endif

	
	if (tx_ra_bitmap & 0xffff000)
		sta_band |= WIRELESS_11_24N | WIRELESS_11G | WIRELESS_11B;
	else if (tx_ra_bitmap & 0xff0)
		sta_band |= WIRELESS_11G |WIRELESS_11B;
	else
		sta_band |= WIRELESS_11B;

	raid = networktype_to_raid(sta_band);	
	init_rate = get_highest_rate_idx(tx_ra_bitmap&0x0fffffff)&0x3f;
	
	if (psta->aid < NUM_STA) 
	{
		u8 arg = 0;

		arg = psta->aid&0x1f;
		
		arg |= BIT(7);
		
		if (shortGIrate==_TRUE)
			arg |= BIT(5);

		tx_ra_bitmap |= ((raid<<28)&0xf0000000);

		DBG_871X("update raid entry, bitmap=0x%x, arg=0x%x\n", tx_ra_bitmap, arg);

		//bitmap[0:27] = tx_rate_bitmap
		//bitmap[28:31]= Rate Adaptive id
		//arg[0:4] = macid
		//arg[5] = Short GI
		padapter->HalFunc.Add_RateATid(padapter, tx_ra_bitmap, arg);

		if (shortGIrate==_TRUE)
			init_rate |= BIT(6);
		
		//set ra_id, init_rate
		psta->raid = raid;
		psta->init_rate = init_rate;
		
	}
	else 
	{
		DBG_871X("station aid %d exceed the max number\n", psta->aid);
	}

}

static void update_bmc_sta(_adapter *padapter)
{
	_irqL	irqL;
	u32 init_rate=0;
	unsigned char	network_type, raid;
	unsigned short para16;
	int i, supportRateNum = 0;	
	unsigned int tx_ra_bitmap=0;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	WLAN_BSSID_EX *pcur_network = (WLAN_BSSID_EX *)&pmlmepriv->cur_network.network;	
	struct sta_info *psta = get_bcmc_stainfo(padapter);

	if(psta)
	{
		psta->aid = 0;//default set to 0
		//psta->mac_id = psta->aid+4;	
		psta->mac_id = psta->aid;

		psta->qos_option = 0;		
		psta->htpriv.ht_option = 0;

		psta->ieee8021x_blocked = 0;

		_memset((void*)&psta->sta_stats, 0, sizeof(struct stainfo_stats));

		//psta->dot118021XPrivacy = _NO_PRIVACY_;//!!! remove it, because it has been set before this.



		//prepare for add_RATid		
		supportRateNum = get_rateset_len((u8*)&pcur_network->SupportedRates);
		network_type = check_network_type((u8*)&pcur_network->SupportedRates, supportRateNum, 1);
		
		_memcpy(psta->bssrateset, &pcur_network->SupportedRates, supportRateNum);
		psta->bssratelen = supportRateNum;

		//b/g mode ra_bitmap  
		for (i=0; i<supportRateNum; i++)
		{	
			if (psta->bssrateset[i])
				tx_ra_bitmap |= get_bit_value_from_ieee_value(psta->bssrateset[i]&0x7f);
		}

		//force to b mode 
		network_type = WIRELESS_11B;
		tx_ra_bitmap = 0xf;
		//tx_ra_bitmap = update_basic_rate(pcur_network->SupportedRates, supportRateNum);

		raid = networktype_to_raid(network_type);
		init_rate = get_highest_rate_idx(tx_ra_bitmap&0x0fffffff)&0x3f;
				
		//DBG_871X("Add id %d val %08x to ratr for bmc sta\n", psta->aid, tx_ra_bitmap);

		//if(pHalData->fw_ractrl == _TRUE)
		{
			u8 arg = 0;

			arg = psta->aid&0x1f;
		
			arg |= BIT(7);
		
			//if (shortGIrate==_TRUE)
			//	arg |= BIT(5);
			
			tx_ra_bitmap |= ((raid<<28)&0xf0000000);			

			DBG_871X("update_bmc_sta, mask=0x%x, arg=0x%x\n", tx_ra_bitmap, arg);

			//bitmap[0:27] = tx_rate_bitmap
			//bitmap[28:31]= Rate Adaptive id
			//arg[0:4] = macid
			//arg[5] = Short GI
			padapter->HalFunc.Add_RateATid(padapter, tx_ra_bitmap, arg);			
		
		}

		//set ra_id, init_rate
		psta->raid = raid;
		psta->init_rate = init_rate;
	 
		_enter_critical_bh(&psta->lock, &irqL);
		psta->state = _FW_LINKED;
		_exit_critical_bh(&psta->lock, &irqL);

	}
	else
	{
		DBG_871X("add_RATid_bmc_sta error!\n");
	}
		
}

void update_sta_info_apmode(_adapter *padapter, struct sta_info *psta)
{	
	_irqL	irqL;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct ht_priv	*phtpriv_ap = &pmlmepriv->htpriv;
	struct ht_priv	*phtpriv_sta = &psta->htpriv;

	//set intf_tag to if1
	//psta->intf_tag = 0;
	
	//psta->mac_id = psta->aid+4;
	psta->mac_id = psta->aid;
	
	if(psecuritypriv->dot11AuthAlgrthm==dot11AuthAlgrthm_8021X)
		psta->ieee8021x_blocked = _TRUE;
	else
		psta->ieee8021x_blocked = _FALSE;
	

	//update sta's cap
	
	//ERP
	VCS_update(padapter, psta);
		
	//HT related cap
	if(phtpriv_sta->ht_option)
	{
		//check if sta supports rx ampdu
		phtpriv_sta->ampdu_enable = phtpriv_ap->ampdu_enable;

		//check if sta support s Short GI
		if((phtpriv_sta->ht_cap.cap_info & phtpriv_ap->ht_cap.cap_info) & cpu_to_le16(IEEE80211_HT_CAP_SGI_20|IEEE80211_HT_CAP_SGI_40))
		{
			phtpriv_sta->sgi = _TRUE;
		}

		// bwmode
		if((phtpriv_sta->ht_cap.cap_info & phtpriv_ap->ht_cap.cap_info) & cpu_to_le16(IEEE80211_HT_CAP_SUP_WIDTH))
		{
			//phtpriv_sta->bwmode = HT_CHANNEL_WIDTH_40;
			phtpriv_sta->bwmode = pmlmeext->cur_bwmode;
			phtpriv_sta->ch_offset = pmlmeext->cur_ch_offset;
			
		}		

		psta->qos_option = _TRUE;
		
	}
	else
	{
		phtpriv_sta->ampdu_enable = _FALSE;
		
		phtpriv_sta->sgi = _FALSE;
		phtpriv_sta->bwmode = HT_CHANNEL_WIDTH_20;
		phtpriv_sta->ch_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
	}

	//Rx AMPDU
	send_delba(padapter, 0, psta->hwaddr);// recipient
	
	//TX AMPDU
	send_delba(padapter, 1, psta->hwaddr);// // originator
	phtpriv_sta->agg_enable_bitmap = 0x0;//reset
	phtpriv_sta->candidate_tid_bitmap = 0x0;//reset
	

	//todo: init other variables
	
	_memset((void*)&psta->sta_stats, 0, sizeof(struct stainfo_stats));


	//add ratid
	add_RATid(padapter, psta);


	_enter_critical_bh(&psta->lock, &irqL);
	psta->state = _FW_LINKED;
	_exit_critical_bh(&psta->lock, &irqL);
	

}

void start_bss_network(_adapter *padapter, u8 *pbuf)
{
	u8 *p;
	u8 val8, cur_channel, cur_bwmode, cur_ch_offset;
	u16 bcn_interval;
	u32	acparm;	
	int	ie_len;	
	struct registry_priv	 *pregpriv = &padapter->registrypriv;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct security_priv* psecuritypriv=&(padapter->securitypriv);	
	WLAN_BSSID_EX *pnetwork = (WLAN_BSSID_EX *)&pmlmepriv->cur_network.network;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX *pnetwork_mlmeext = &(pmlmeinfo->network);
	
	
	//printk("%s\n", __FUNCTION__);

	bcn_interval = (u16)pnetwork->Configuration.BeaconPeriod;	
	cur_channel = pnetwork->Configuration.DSConfig;
	cur_bwmode = HT_CHANNEL_WIDTH_20;;
	cur_ch_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
	
	//todo: update wmm, ht cap
	//pmlmeinfo->WMM_enable;
	//pmlmeinfo->HT_enable;
	if(pmlmepriv->qospriv.qos_option)
		pmlmeinfo->WMM_enable = _TRUE;

	if(pmlmepriv->htpriv.ht_option)
	{
		pmlmeinfo->WMM_enable = _TRUE;
		pmlmeinfo->HT_enable = _TRUE;
	}
	

	if(pmlmepriv->cur_network.join_res != _TRUE) //setting only at  first time
	{		
		flush_all_cam_entry(padapter);	//clear CAM
	}	

	//set MSR to AP_Mode		
	Set_NETYPE0_MSR(padapter, _HW_STATE_AP_);	
		
	//Set BSSID REG
	padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_BSSID, pnetwork->MacAddress);

	//Set EDCA param reg
	acparm = 0x002F3217; // VO
	padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_AC_PARAM_VO, (u8 *)(&acparm));
	acparm = 0x005E4317; // VI
	padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_AC_PARAM_VI, (u8 *)(&acparm));
	acparm = 0x00105320; // BE
	padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_AC_PARAM_BE, (u8 *)(&acparm));
	acparm = 0x0000A444; // BK
	padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_AC_PARAM_BK, (u8 *)(&acparm));

	//Set Security
	val8 = (psecuritypriv->dot11AuthAlgrthm == dot11AuthAlgrthm_8021X)? 0xcc: 0xcf;
	padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_SEC_CFG, (u8 *)(&val8));

	//Beacon Control related register
	padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_BEACON_INTERVAL, (u8 *)(&bcn_interval));

	if(pmlmepriv->cur_network.join_res != _TRUE) //setting only at  first time
	{
		u32 initialgain;

		//disable dynamic functions, such as high power, DIG
		Save_DM_Func_Flag(padapter);
		Switch_DM_Func(padapter, DYNAMIC_FUNC_DISABLE, _FALSE);

		initialgain = 0x30;
		padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_INITIAL_GAIN, (u8 *)(&initialgain));
	
	}

	//set channel, bwmode	
	p = get_ie((pnetwork->IEs + sizeof(NDIS_802_11_FIXED_IEs)), _HT_ADD_INFO_IE_, &ie_len, (pnetwork->IELength - sizeof(NDIS_802_11_FIXED_IEs)));
	if( p && ie_len)
	{
		struct HT_info_element *pht_info = (struct HT_info_element *)(p+2);
					
		if ((pregpriv->cbw40_enable) &&	 (pht_info->infos[0] & BIT(2)))
		{
			//switch to the 40M Hz mode
			//pmlmeext->cur_bwmode = HT_CHANNEL_WIDTH_40;
			cur_bwmode = HT_CHANNEL_WIDTH_40;
			switch (pht_info->infos[0] & 0x3)
			{
				case 1:
					//pmlmeext->cur_ch_offset = HAL_PRIME_CHNL_OFFSET_LOWER;
					cur_ch_offset = HAL_PRIME_CHNL_OFFSET_LOWER;
					break;
			
				case 3:
					//pmlmeext->cur_ch_offset = HAL_PRIME_CHNL_OFFSET_UPPER;
					cur_ch_offset = HAL_PRIME_CHNL_OFFSET_UPPER;					
					break;
				
				default:
					//pmlmeext->cur_ch_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
					cur_ch_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
					break;
			}		
						
		}
					
	}

	//TODO: need to judge the phy parameters on concurrent mode for single phy
	//set_channel_bwmode(padapter, pmlmeext->cur_channel, pmlmeext->cur_ch_offset, pmlmeext->cur_bwmode);
	set_channel_bwmode(padapter, cur_channel, cur_ch_offset, cur_bwmode);

	DBG_871X("CH=%d, BW=%d, offset=%d\n", cur_channel, cur_bwmode, cur_ch_offset);

	//
	pmlmeext->cur_channel = cur_channel;	
	pmlmeext->cur_bwmode = cur_bwmode;
	pmlmeext->cur_ch_offset = cur_ch_offset;	
	pmlmeext->cur_wireless_mode = pmlmepriv->cur_network.network_type;

	//let pnetwork_mlmeext == pnetwork_mlme.
	_memcpy(pnetwork_mlmeext, pnetwork, pnetwork->Length);

	//issue beacon frame
	if(send_beacon(padapter)==_FAIL)
	{
		DBG_871X("issue_beacon, fail!\n");
	}


	//update bc/mc sta_info
	update_bmc_sta(padapter);
	
}

#ifdef CONFIG_NATIVEAP_MLME

static void update_bcn_fixed_ie(_adapter *padapter)
{
	DBG_871X("%s\n", __FUNCTION__);

}

static void update_bcn_erpinfo_ie(_adapter *padapter)
{
	DBG_871X("%s\n", __FUNCTION__);

}

static void update_bcn_htcap_ie(_adapter *padapter)
{
	DBG_871X("%s\n", __FUNCTION__);

}

static void update_bcn_htinfo_ie(_adapter *padapter)
{	
	DBG_871X("%s\n", __FUNCTION__);

}

static void update_bcn_rsn_ie(_adapter *padapter)
{
	DBG_871X("%s\n", __FUNCTION__);

}

static void update_bcn_wpa_ie(_adapter *padapter)
{
	DBG_871X("%s\n", __FUNCTION__);

}

static void update_bcn_wmm_ie(_adapter *padapter)
{
	DBG_871X("%s\n", __FUNCTION__);
	
}

static void update_bcn_wps_ie(_adapter *padapter)
{
	int match;
	u8 *pwps_ie, *pwps_ie_src, *premainder_ie, *pbackup_remainder_ie=NULL;
	uint wps_ielen, wps_offset, remainder_ielen;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX *pnetwork = &(pmlmeinfo->network);
	unsigned char *ie = pnetwork->IEs;
	u32 ielen = pnetwork->IELength;


	DBG_871X("%s\n", __FUNCTION__);

	pwps_ie = get_wps_ie(ie, ielen, NULL, &wps_ielen);
	
	if(pwps_ie==NULL || wps_ielen==0)
		return;

	wps_offset = (uint)(pwps_ie-ie);

	premainder_ie = pwps_ie + wps_ielen;

	remainder_ielen = ielen - wps_offset - wps_ielen;

	if(remainder_ielen>0)
	{
		pbackup_remainder_ie = _malloc(remainder_ielen);
		_memcpy(pbackup_remainder_ie, premainder_ie, remainder_ielen);
	}


	pwps_ie_src = pmlmepriv->wps_beacon_ie;
	if(pwps_ie_src == NULL)
		return;


	wps_ielen = (uint)pwps_ie_src[1];//to get ie data len
	if((wps_offset+wps_ielen+2+remainder_ielen)<=MAX_IE_SZ)
	{
		_memcpy(pwps_ie, pwps_ie_src, wps_ielen+2);
		pwps_ie += (wps_ielen+2);

		if(pbackup_remainder_ie)
			_memcpy(pwps_ie, pbackup_remainder_ie, remainder_ielen);

		//update IELength
		pnetwork->IELength = wps_offset + (wps_ielen+2) + remainder_ielen;
	}

}

static void update_bcn_vendor_spec_ie(_adapter *padapter, u8*oui)
{
	DBG_871X("%s\n", __FUNCTION__);

	if(_memcmp(WPA_OUI, oui, 4))
	{
		update_bcn_wpa_ie(padapter);
	}
	else if(_memcmp(WMM_OUI, oui, 4))
	{
		update_bcn_wmm_ie(padapter);
	}
	else if(_memcmp(WPS_OUI, oui, 4))
	{
		update_bcn_wps_ie(padapter);
	}
	else
	{
		DBG_871X("unknown OUI type!\n");
 	}
	
	
}

void update_beacon(_adapter *padapter, u8 ie_id, u8 *oui, u8 tx)
{
	//DBG_871X("%s\n", __FUNCTION__);

	switch(ie_id)
	{
		case 0xFF:

			update_bcn_fixed_ie(padapter);//8: TimeStamp, 2: Beacon Interval 2:Capability
			
			break;
	
		case _TIM_IE_:
			
			update_BCNTIM(padapter);
			
			break;

		case _ERPINFO_IE_:

			update_bcn_erpinfo_ie(padapter);

			break;
			
		case _HT_CAPABILITY_IE_:

			update_bcn_htcap_ie(padapter);
			
			break;

		case _RSN_IE_2_:

			update_bcn_rsn_ie(padapter);

			break;
			
		case _HT_ADD_INFO_IE_:

			update_bcn_htinfo_ie(padapter);
			
			break;
	
		case _VENDOR_SPECIFIC_IE_:

			update_bcn_vendor_spec_ie(padapter, oui);
			
			break;
			
		default:
			break;
	}

	if(tx)
	{
		//send_beacon(padapter);//send_beacon must execute on TSR level
		set_tx_beacon_cmd(padapter);

	}

}

#ifdef CONFIG_80211N_HT

/*
op_mode
Set to 0 (HT pure) under the followign conditions
	- all STAs in the BSS are 20/40 MHz HT in 20/40 MHz BSS or
	- all STAs in the BSS are 20 MHz HT in 20 MHz BSS
Set to 1 (HT non-member protection) if there may be non-HT STAs
	in both the primary and the secondary channel
Set to 2 if only HT STAs are associated in BSS,
	however and at least one 20 MHz HT STA is associated
Set to 3 (HT mixed mode) when one or more non-HT STAs are associated
	(currently non-GF HT station is considered as non-HT STA also)
*/
static int rtw_ht_operation_update(_adapter *padapter)
{
	u16 cur_op_mode, new_op_mode;
	int op_mode_changes = 0;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct ht_priv	*phtpriv_ap = &pmlmepriv->htpriv;

	if(pmlmepriv->htpriv.ht_option == _TRUE) 
		return 0;
	
	//if (!iface->conf->ieee80211n || iface->conf->ht_op_mode_fixed)
	//	return 0;

	DBG_871X("%s current operation mode=0x%X\n",
		   __FUNCTION__, pmlmepriv->ht_op_mode);

	if (!(pmlmepriv->ht_op_mode & HT_INFO_OPERATION_MODE_NON_GF_DEVS_PRESENT)
	    && pmlmepriv->num_sta_ht_no_gf) {
		pmlmepriv->ht_op_mode |=
			HT_INFO_OPERATION_MODE_NON_GF_DEVS_PRESENT;
		op_mode_changes++;
	} else if ((pmlmepriv->ht_op_mode &
		    HT_INFO_OPERATION_MODE_NON_GF_DEVS_PRESENT) &&
		   pmlmepriv->num_sta_ht_no_gf == 0) {
		pmlmepriv->ht_op_mode &=
			~HT_INFO_OPERATION_MODE_NON_GF_DEVS_PRESENT;
		op_mode_changes++;
	}

	if (!(pmlmepriv->ht_op_mode & HT_INFO_OPERATION_MODE_NON_HT_STA_PRESENT) &&
	    (pmlmepriv->num_sta_no_ht || pmlmepriv->olbc_ht)) {
		pmlmepriv->ht_op_mode |= HT_INFO_OPERATION_MODE_NON_HT_STA_PRESENT;
		op_mode_changes++;
	} else if ((pmlmepriv->ht_op_mode &
		    HT_INFO_OPERATION_MODE_NON_HT_STA_PRESENT) &&
		   (pmlmepriv->num_sta_no_ht == 0 && !pmlmepriv->olbc_ht)) {
		pmlmepriv->ht_op_mode &=
			~HT_INFO_OPERATION_MODE_NON_HT_STA_PRESENT;
		op_mode_changes++;
	}

	/* Note: currently we switch to the MIXED op mode if HT non-greenfield
	 * station is associated. Probably it's a theoretical case, since
	 * it looks like all known HT STAs support greenfield.
	 */
	new_op_mode = 0;
	if (pmlmepriv->num_sta_no_ht ||
	    (pmlmepriv->ht_op_mode & HT_INFO_OPERATION_MODE_NON_GF_DEVS_PRESENT))
		new_op_mode = OP_MODE_MIXED;
	else if ((phtpriv_ap->ht_cap.cap_info & IEEE80211_HT_CAP_SUP_WIDTH)
		 && pmlmepriv->num_sta_ht_20mhz)
		new_op_mode = OP_MODE_20MHZ_HT_STA_ASSOCED;
	else if (pmlmepriv->olbc_ht)
		new_op_mode = OP_MODE_MAY_BE_LEGACY_STAS;
	else
		new_op_mode = OP_MODE_PURE;

	cur_op_mode = pmlmepriv->ht_op_mode & HT_INFO_OPERATION_MODE_OP_MODE_MASK;
	if (cur_op_mode != new_op_mode) {
		pmlmepriv->ht_op_mode &= ~HT_INFO_OPERATION_MODE_OP_MODE_MASK;
		pmlmepriv->ht_op_mode |= new_op_mode;
		op_mode_changes++;
	}

	DBG_871X("%s new operation mode=0x%X changes=%d\n",
		   __FUNCTION__, pmlmepriv->ht_op_mode, op_mode_changes);

	return op_mode_changes;
	
}

#endif /* CONFIG_80211N_HT */


void bss_cap_update(_adapter *padapter, struct sta_info *psta)
{
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);

#if 0
	if (psta->flags & WLAN_STA_NONERP && !psta->nonerp_set) {
		psta->nonerp_set = 1;
		pmlmepriv->num_sta_non_erp++;
		if (pmlmepriv->num_sta_non_erp == 1)
			ieee802_11_set_beacons(hapd->iface);
	}
#endif

	if(psta->flags & WLAN_STA_NONERP)
	{
		if(!psta->nonerp_set)
		{
			psta->nonerp_set = 1;
			
			pmlmepriv->num_sta_non_erp++;
			
			if (pmlmepriv->num_sta_non_erp == 1)
				update_beacon(padapter, _ERPINFO_IE_, NULL, _TRUE);
		}
		
	}
	else
	{
		if(psta->nonerp_set)
		{
			psta->nonerp_set = 0;
			
			pmlmepriv->num_sta_non_erp--;
			
			if (pmlmepriv->num_sta_non_erp == 0)
				update_beacon(padapter, _ERPINFO_IE_, NULL, _TRUE);
		}
		
	}


#if 0
	if (!(psta->capability & WLAN_CAPABILITY_SHORT_SLOT) &&
	    !psta->no_short_slot_time_set) {
		psta->no_short_slot_time_set = 1;
		pmlmepriv->num_sta_no_short_slot_time++;
		if ((pmlmeext->cur_wireless_mode > WIRELESS_11B) &&
		    (pmlmepriv->num_sta_no_short_slot_time == 1))
			ieee802_11_set_beacons(hapd->iface);
	}
#endif

	if(!(psta->capability & WLAN_CAPABILITY_SHORT_SLOT))
	{
		if(!psta->no_short_slot_time_set)
		{
			psta->no_short_slot_time_set = 1;
			
			pmlmepriv->num_sta_no_short_slot_time++;
			
			if ((pmlmeext->cur_wireless_mode > WIRELESS_11B) &&
		   		 (pmlmepriv->num_sta_no_short_slot_time == 1))
						update_beacon(padapter, 0xFF, NULL, _TRUE);
			
		}
	}
	else
	{
		if(psta->no_short_slot_time_set)
		{
			psta->no_short_slot_time_set = 0;
			
			pmlmepriv->num_sta_no_short_slot_time--;
			
			if ((pmlmeext->cur_wireless_mode > WIRELESS_11B) &&
		   		 (pmlmepriv->num_sta_no_short_slot_time == 0))
						update_beacon(padapter, 0xFF, NULL, _TRUE);
		}
	}
		
	
#if 0
	if (!(psta->capability & WLAN_CAPABILITY_SHORT_PREAMBLE) &&
	    !psta->no_short_preamble_set) {
		psta->no_short_preamble_set = 1;
		pmlmepriv->num_sta_no_short_preamble++;
		if ((pmlmeext->cur_wireless_mode > WIRELESS_11B) && 
		     (pmlmepriv->num_sta_no_short_preamble == 1))
			ieee802_11_set_beacons(hapd->iface);
	}
#endif


	if(!(psta->flags & WLAN_STA_SHORT_PREAMBLE))	
	{
		if(!psta->no_short_preamble_set)
		{
			psta->no_short_preamble_set = 1;
			
			pmlmepriv->num_sta_no_short_preamble++;
			
			if ((pmlmeext->cur_wireless_mode > WIRELESS_11B) && 
		     		(pmlmepriv->num_sta_no_short_preamble == 1))
					update_beacon(padapter, 0xFF, NULL, _TRUE);
			
		}
	}
	else
	{
		if(psta->no_short_preamble_set)
		{
			psta->no_short_preamble_set = 0;
			
			pmlmepriv->num_sta_no_short_preamble--;
			
			if ((pmlmeext->cur_wireless_mode > WIRELESS_11B) && 
		     		(pmlmepriv->num_sta_no_short_preamble == 0))
					update_beacon(padapter, 0xFF, NULL, _TRUE);
			
		}
	}


#ifdef CONFIG_80211N_HT

	if (psta->flags & WLAN_STA_HT) 
	{
		u16 ht_capab = le16_to_cpu(psta->htpriv.ht_cap.cap_info);
			
		DBG_871X("HT: STA " MACSTR " HT Capabilities "
			   "Info: 0x%04x\n", MAC2STR(psta->hwaddr), ht_capab);

		if (psta->no_ht_set) {
			psta->no_ht_set = 0;
			pmlmepriv->num_sta_no_ht--;
		}
		
		if ((ht_capab & IEEE80211_HT_CAP_GRN_FLD) == 0) {
			if (!psta->no_ht_gf_set) {
				psta->no_ht_gf_set = 1;
				pmlmepriv->num_sta_ht_no_gf++;
			}
			DBG_871X("%s STA " MACSTR " - no "
				   "greenfield, num of non-gf stations %d\n",
				   __FUNCTION__, MAC2STR(psta->hwaddr),
				   pmlmepriv->num_sta_ht_no_gf);
		}
		
		if ((ht_capab & IEEE80211_HT_CAP_SUP_WIDTH) == 0) {
			if (!psta->ht_20mhz_set) {
				psta->ht_20mhz_set = 1;
				pmlmepriv->num_sta_ht_20mhz++;
			}
			DBG_871X("%s STA " MACSTR " - 20 MHz HT, "
				   "num of 20MHz HT STAs %d\n",
				   __FUNCTION__, MAC2STR(psta->hwaddr),
				   pmlmepriv->num_sta_ht_20mhz);
		}
		
	} 
	else 
	{
		if (!psta->no_ht_set) {
			psta->no_ht_set = 1;
			pmlmepriv->num_sta_no_ht++;
		}
		if(pmlmepriv->htpriv.ht_option == _TRUE) {		
			DBG_871X("%s STA " MACSTR
				   " - no HT, num of non-HT stations %d\n",
				   __FUNCTION__, MAC2STR(psta->hwaddr),
				   pmlmepriv->num_sta_no_ht);
		}
	}

	if (rtw_ht_operation_update(padapter) > 0)
	{
		update_beacon(padapter, _HT_CAPABILITY_IE_, NULL, _FALSE);
		update_beacon(padapter, _HT_ADD_INFO_IE_, NULL, _TRUE);
	}	
	
#endif /* CONFIG_80211N_HT */

}

void ap_free_sta(_adapter *padapter, struct sta_info *psta)
{
	_irqL irqL;	
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	if(!psta)
		return;


	if (psta->nonerp_set) {
		psta->nonerp_set = 0;		
		pmlmepriv->num_sta_non_erp--;
		if (pmlmepriv->num_sta_non_erp == 0)
			update_beacon(padapter, _ERPINFO_IE_, NULL, _TRUE);
	}

	if (psta->no_short_slot_time_set) {
		psta->no_short_slot_time_set = 0;
		pmlmepriv->num_sta_no_short_slot_time--;
		if (pmlmeext->cur_wireless_mode > WIRELESS_11B
		    && pmlmepriv->num_sta_no_short_slot_time == 0)
			update_beacon(padapter, 0xFF, NULL, _TRUE);
	}

	if (psta->no_short_preamble_set) {
		psta->no_short_preamble_set = 0;
		pmlmepriv->num_sta_no_short_preamble--;
		if (pmlmeext->cur_wireless_mode > WIRELESS_11B
		    && pmlmepriv->num_sta_no_short_preamble == 0)
			update_beacon(padapter, 0xFF, NULL, _TRUE);
	}
	
#ifdef CONFIG_80211N_HT

	if (psta->no_ht_gf_set) {
		psta->no_ht_gf_set = 0;
		pmlmepriv->num_sta_ht_no_gf--;
	}

	if (psta->no_ht_set) {
		psta->no_ht_set = 0;
		pmlmepriv->num_sta_no_ht--;
	}

	if (psta->ht_20mhz_set) {
		psta->ht_20mhz_set = 0;
		pmlmepriv->num_sta_ht_20mhz--;
	}

	if (rtw_ht_operation_update(padapter) > 0)
	{
		update_beacon(padapter, _HT_CAPABILITY_IE_, NULL, _FALSE);
		update_beacon(padapter, _HT_ADD_INFO_IE_, NULL, _TRUE);
	}
	
#endif /* CONFIG_80211N_HT */
		
	
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

int rtw_sta_flush(_adapter *padapter)
{
	_irqL irqL;
	_list	*phead, *plist;
	int ret=0;	
	struct sta_info *psta = NULL;	
	struct sta_priv *pstapriv = &padapter->stapriv;
	u8 bc_addr[ETH_ALEN] = {0xff,0xff,0xff,0xff,0xff,0xff};

	DBG_871X("%s\n", __FUNCTION__);

	phead = &pstapriv->asoc_list;
	plist = get_next(phead);
	
	//free sta asoc_queue
	while ((end_of_queue_search(phead, plist)) == _FALSE)	
	{		
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		
		plist = get_next(plist);

		list_delete(&psta->asoc_list);		

		ap_free_sta(padapter, psta);
		
	}


	issue_deauth(padapter, bc_addr, WLAN_REASON_DEAUTH_LEAVING);

	return ret;

}

void sta_info_update(_adapter *padapter, struct sta_info *psta)
{	
	int flags = psta->flags;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	
				
	//update wmm cap.
	if(WLAN_STA_WME&flags)
		psta->qos_option = 1;
	else
		psta->qos_option = 0;

	if(pmlmepriv->qospriv.qos_option == 0)	
		psta->qos_option = 0;

		
#ifdef CONFIG_80211N_HT		
	//update 802.11n ht cap.
	if(WLAN_STA_HT&flags)
	{
		psta->htpriv.ht_option = 1;
		psta->qos_option = 1;	
	}
	else		
	{
		psta->htpriv.ht_option = 0;
	}
		
	if(pmlmepriv->htpriv.ht_option == 0)	
		psta->htpriv.ht_option = 0;
#endif		


	update_sta_info_apmode(padapter, psta);
		

}

void start_ap_mode(_adapter *padapter)
{
	int i;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;
	
	
	init_mlme_ap_info(padapter);

	pmlmepriv->num_sta_non_erp = 0;

	pmlmepriv->num_sta_no_short_slot_time = 0;

	pmlmepriv->num_sta_no_short_preamble = 0;

	pmlmepriv->num_sta_ht_no_gf = 0;

	pmlmepriv->num_sta_no_ht = 0;
	
	pmlmepriv->num_sta_ht_20mhz = 0;

	pmlmepriv->olbc = _FALSE;

	pmlmepriv->olbc_ht = _FALSE;
	
#ifdef CONFIG_80211N_HT
	pmlmepriv->ht_op_mode = 0;
#endif

	for(i=0; i<NUM_STA; i++)
		pstapriv->sta_aid[i] = NULL;

	pmlmepriv->wps_beacon_ie = NULL;
	pmlmepriv->wps_probe_resp_ie = NULL;

}

void stop_ap_mode(_adapter *padapter)
{
	_irqL irqL;
	_list	*phead, *plist;
	struct sta_info *psta=NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	phead = &pstapriv->asoc_list;
	plist = get_next(phead);
	
	rtw_sta_flush(padapter);

#if 0	
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

	//free_assoc_sta_resources	
	free_all_stainfo(padapter);
	
	psta = get_bcmc_stainfo(padapter);
	_enter_critical_bh(&(pstapriv->sta_hash_lock), &irqL);		
	free_stainfo(padapter, psta);
	_exit_critical_bh(&(pstapriv->sta_hash_lock), &irqL);
	
	init_bcmc_stainfo(padapter);	


	if(pmlmepriv->wps_beacon_ie)
	{
		_mfree(pmlmepriv->wps_beacon_ie, 0);
		pmlmepriv->wps_beacon_ie = NULL;
	}	


	if(pmlmepriv->wps_probe_resp_ie)
	{
		_mfree(pmlmepriv->wps_probe_resp_ie, 0);
		pmlmepriv->wps_probe_resp_ie = NULL;
	}	

}


#endif

#endif

