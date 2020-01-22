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
#define _RTW_PWRCTRL_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <osdep_intf.h>

#ifdef CONFIG_SDIO_HCI
#include <sdio_ops.h>
#endif

#ifdef CONFIG_IPS
void ips_enter(_adapter * padapter)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	_enter_pwrlock(&pwrpriv->lock);
	pwrpriv->ips_enter_cnts++;	
	printk("==>ips_enter cnts:%d\n",pwrpriv->ips_enter_cnts);
	
	pwrpriv->bips_processing = _TRUE;	

	if(rf_off == pwrpriv->change_rfpwrstate )
	{	
		printk("==>power_saving_ctrl_wk_hdl change rf to OFF...LED(0x%08x).... \n\n",read32(padapter,0x4c));
		#ifdef CONFIG_IPS_LEVEL_2		
		pwrpriv->bkeepfwalive = _TRUE;
		#endif
		
		rtw_ips_pwr_down(padapter);
		pwrpriv->current_rfpwrstate = rf_off;
	}	
	pwrpriv->bips_processing = _FALSE;	
	_exit_pwrlock(&pwrpriv->lock);
	
}

int ips_leave(_adapter * padapter)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct security_priv* psecuritypriv=&(padapter->securitypriv);
	struct mlme_priv		*pmlmepriv = &(padapter->mlmepriv);
	int result = _SUCCESS;
	sint keyid;
	_enter_pwrlock(&pwrpriv->lock);
	if((pwrpriv->current_rfpwrstate == rf_off) &&(!pwrpriv->bips_processing))
	{
		pwrpriv->change_rfpwrstate = rf_on;
		pwrpriv->ips_leave_cnts++;
		printk("==>ips_leave cnts:%d\n",pwrpriv->ips_leave_cnts);
		#ifdef CONFIG_IPS_LEVEL_2		
		pwrpriv->bkeepfwalive = _TRUE;
		#endif
		result = rtw_ips_pwr_up(padapter);		
		pwrpriv->bips_processing = _TRUE;
		pwrpriv->current_rfpwrstate = rf_on;

		if((_WEP40_ == psecuritypriv->dot11PrivacyAlgrthm) ||(_WEP104_ == psecuritypriv->dot11PrivacyAlgrthm))
		{
			printk("==>%s,channel(%d),processing(%x)\n",__FUNCTION__,padapter->mlmeextpriv.cur_channel,pwrpriv->bips_processing);
			set_channel_bwmode(padapter, padapter->mlmeextpriv.cur_channel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, HT_CHANNEL_WIDTH_20);			
			for(keyid=0;keyid<4;keyid++){
				if(pmlmepriv->key_mask & BIT(keyid)){
					result=set_key(padapter,psecuritypriv, keyid);	
				}
			}
		}
		
		printk("==> ips_leave.....LED(0x%08x)...\n",read32(padapter,0x4c));
		pwrpriv->bips_processing = _FALSE;
		#ifdef CONFIG_IPS_LEVEL_2		
		pwrpriv->bkeepfwalive = _FALSE;
		#endif

	}
	_exit_pwrlock(&pwrpriv->lock);
	return result;
}


#endif

#ifdef CONFIG_AUTOSUSPEND
extern void autosuspend_enter(_adapter* padapter);	
extern int autoresume_enter(_adapter* padapter);
#endif
#ifdef SUPPORT_HW_RFOFF_DETECTED
int rtw_hw_suspend(_adapter *padapter );
int rtw_hw_resume(_adapter *padapter);

#endif

#ifdef PLATFORM_LINUX
void rtw_ps_processor(_adapter*padapter)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	int res;
	rt_rf_power_state rfpwrstate;
	
#ifdef SUPPORT_HW_RFOFF_DETECTED
	if(pwrpriv->bips_processing == _TRUE)	return;		
	
	//printk("==> fw report state(0x%x)\n",rtw_read8(padapter,0x1ca));	
	if(padapter->pwrctrlpriv.bHWPwrPindetect) 
	{
	#ifdef CONFIG_AUTOSUSPEND
		if(padapter->registrypriv.usbss_enable)
		{
			if(padapter->net_closed == _TRUE)	return;
			
			if(pwrpriv->current_rfpwrstate == rf_on)
			{
				rfpwrstate = RfOnOffDetect(padapter);
				printk("@@@@- #1  %s==> rfstate:%s \n",__FUNCTION__,(rfpwrstate==rf_on)?"rf_on":"rf_off");
				if(rfpwrstate!= pwrpriv->current_rfpwrstate)
				{
					if(rfpwrstate == rf_off)
					{
						pwrpriv->change_rfpwrstate = rf_off;
						
						pwrpriv->bkeepfwalive = _TRUE;	
						pwrpriv->brfoffbyhw = _TRUE;						
						
						autosuspend_enter(padapter);							
					}
				}
			}			
		}
		else
	#endif
		{
			rfpwrstate = RfOnOffDetect(padapter);
			printk("@@@@- #2  %s==> rfstate:%s \n",__FUNCTION__,(rfpwrstate==rf_on)?"rf_on":"rf_off");

			if(rfpwrstate!= pwrpriv->current_rfpwrstate)
			{
				if(rfpwrstate == rf_off)
				{	
					pwrpriv->change_rfpwrstate = rf_off;														
					pwrpriv->brfoffbyhw = _TRUE;
					padapter->bCardDisableWOHSM = _TRUE;
					rtw_hw_suspend(padapter );	
				}
				else
				{
					pwrpriv->change_rfpwrstate = rf_on;
					rtw_hw_resume(padapter );			
				}
				printk("current_rfpwrstate(%s)\n",(pwrpriv->current_rfpwrstate == rf_off)?"rf_off":"rf_on");
			}
		}
		pwrpriv->pwr_state_check_cnts ++;	
	}
	
#endif
	if( pwrpriv->power_mgnt == PS_MODE_ACTIVE )	return;
				
	if(padapter->net_closed == _TRUE)	return;

	if((pwrpriv->current_rfpwrstate == rf_on) && ((pwrpriv->pwr_state_check_cnts%4)==0))
	{
		if (	(check_fwstate(pmlmepriv, _FW_LINKED|_FW_UNDER_SURVEY|_FW_UNDER_LINKING) == _TRUE) ||
			(check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE) ||
			(check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == _TRUE) ||
			(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE) ||
			(padapter->net_closed == _TRUE)|| (padapter->bup == _FALSE)	
		)
		{
			return;
		}
		printk("==>%s .fw_state(%x)\n",__FUNCTION__,padapter->mlmepriv.fw_state);
		pwrpriv->change_rfpwrstate = rf_off;

#ifdef CONFIG_AUTOSUSPEND
		if(padapter->registrypriv.usbss_enable)
		{		
			if(padapter->pwrctrlpriv.bHWPwrPindetect) 
				pwrpriv->bkeepfwalive = _TRUE;				
			
			padapter->bCardDisableWOHSM = _TRUE;
			autosuspend_enter(padapter);
		}		
		else if(padapter->pwrctrlpriv.bHWPwrPindetect)
		{
		}
		else
#endif	
		{
#ifdef CONFIG_IPS	
			ips_enter(padapter);			
#endif
		}
	}

	
}

void pwr_state_check_handler(void *FunctionContext)
{
	_adapter *padapter = (_adapter *)FunctionContext;
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct pwrctrl_priv *pwrctrlpriv = &padapter->pwrctrlpriv;

#ifdef SUPPORT_HW_RFOFF_DETECTED
	//printk("%s...bHWPwrPindetect(%d)\n",__FUNCTION__,padapter->pwrctrlpriv.bHWPwrPindetect);
	if(padapter->pwrctrlpriv.bHWPwrPindetect)
	{
		rtw_ps_cmd(padapter);		
		_set_timer(&padapter->pwrctrlpriv.pwr_state_check_timer, padapter->pwrctrlpriv.pwr_state_check_inverval);
	}	
	else	
#endif
	{
		if(padapter->net_closed == _TRUE)		return;
		//printk("%s\n",__FUNCTION__);
		if (	(check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE) ||
			(check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == _TRUE) ||
			(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE) ||	
			(check_fwstate(pmlmepriv, _FW_LINKED|_FW_UNDER_SURVEY|_FW_UNDER_LINKING) == _TRUE)  ||
			(padapter->net_closed == _TRUE)|| (padapter->bup == _FALSE)		
			)
		{	
			//other pwr ctrl....	
			_set_timer(&padapter->pwrctrlpriv.pwr_state_check_timer, padapter->pwrctrlpriv.pwr_state_check_inverval);
		}
		else
		{	
			if((pwrpriv->current_rfpwrstate == rf_on) &&(_FALSE == pwrpriv->bips_processing))
			{	
				pwrpriv->change_rfpwrstate = rf_off;
				pwrctrlpriv->pwr_state_check_cnts = 0;
				printk("==>pwr_state_check_handler .fw_state(%x)\n",padapter->mlmepriv.fw_state);				
				rtw_ps_cmd(padapter);				
			}

		}
	}
	


}
#endif


#ifdef CONFIG_LPS
void set_rpwm(_adapter * padapter, u8 val8)
{
	u8	rpwm;
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;

_func_enter_;

	if(pwrpriv->rpwm == val8){
		RT_TRACE(_module_rtl871x_pwrctrl_c_,_drv_err_,("Already set rpwm [%d] ! \n", val8));
		return;
	}

	if ((padapter->bDriverStopped == _TRUE)||(padapter->bSurpriseRemoved== _TRUE)){
		RT_TRACE(_module_rtl871x_xmit_c_,_drv_err_,("set_rpwm=> bDriverStopped or bSurpriseRemoved \n"));
		return;
	}
	rpwm = val8 |pwrpriv->tog;

	pwrpriv->rpwm = val8;
	
	RT_TRACE(_module_rtl871x_pwrctrl_c_,_drv_err_,("set_rpwm: value = %x\n", rpwm));

	padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_SET_RPWM, (u8 *)(&rpwm));
	
	pwrpriv->tog += 0x80;

_func_exit_;
}

u8 PS_RDY_CHECK(_adapter * padapter)
{
	u32 curr_time, delta_time;
	struct pwrctrl_priv	*pwrpriv = &padapter->pwrctrlpriv;
	struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);

	curr_time = get_current_time();	

	delta_time = curr_time -pwrpriv->DelayLPSLastTimeStamp;

	if(delta_time < LPS_DELAY_TIME)
	{		
		return _FALSE;
	}

	if (	(check_fwstate(pmlmepriv, _FW_LINKED) == _FALSE) ||
		(check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE) ||
		(check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == _TRUE) ||
		(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE) )
		return _FALSE;

	if( (padapter->securitypriv.dot11AuthAlgrthm == dot11AuthAlgrthm_8021X) && (padapter->securitypriv.binstallGrpkey == _FALSE) )
	{
		DBG_8192C("Group handshake still in progress !!!\n");
		return _FALSE;
	}

	return _TRUE;
}

void set_ps_mode(_adapter * padapter, uint ps_mode, uint smart_ps)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;

_func_enter_;

	RT_TRACE(_module_rtl871x_pwrctrl_c_,_drv_err_,("========= Power Mode is :%d, Smart_PS = %d\n", ps_mode,smart_ps));
	//printk("========= Power Mode is :%d, Smart_PS = %d\n", ps_mode,smart_ps);

	if(ps_mode > PM_Card_Disable) {
		RT_TRACE(_module_rtl871x_pwrctrl_c_,_drv_err_,("ps_mode:%d error\n", ps_mode));
		return;
	}

	if(pwrpriv->pwr_mode == ps_mode){
		return;
	}

	pwrpriv->pwr_mode = ps_mode;

	if(pwrpriv->pwr_mode == PS_MODE_ACTIVE)
	{
		DBG_8192C("set_ps_mode(): Busy Traffic , Leave 802.11 power save..\n");
		set_rpwm(padapter, PS_STATE_S4);
		padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_H2C_FW_PWRMODE, (u8 *)(&ps_mode));
		pwrpriv->bFwCurrentInPSMode = _FALSE;
	}
	else
	{
		if(PS_RDY_CHECK(padapter))
		{
			DBG_8192C("set_ps_mode(): Enter 802.11 power save mode...\n");
			pwrpriv->bFwCurrentInPSMode = _TRUE;
			padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_H2C_FW_PWRMODE, (u8 *)(&ps_mode));
			set_rpwm(padapter, PS_STATE_S2);
		}
		else
		{
			pwrpriv->pwr_mode = PS_MODE_ACTIVE;
		}
	}

_func_exit_;
}


//
//	Description:
//		Enter the leisure power save mode.
//
void LPS_Enter(PADAPTER padapter)
{
	struct pwrctrl_priv	*pwrpriv = &padapter->pwrctrlpriv;
	struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);

_func_enter_;

	//printk("LeisurePSEnter()...\n");
	
	if (	(check_fwstate(pmlmepriv, _FW_LINKED) == _FALSE) ||
		(check_fwstate(pmlmepriv, _FW_UNDER_SURVEY) == _TRUE) ||
		(check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE) ||
		(check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == _TRUE) ||
		(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE) )
		return;

	if (pwrpriv->bLeisurePs)
	{
		// Idle for a while if we connect to AP a while ago.
		if(pwrpriv->LpsIdleCount >= 2) //  4 Sec 
		{
			if(pwrpriv->pwr_mode == PS_MODE_ACTIVE)
			{
				set_ps_mode(padapter, pwrpriv->power_mgnt, 1);
			}	
		}
		else
			pwrpriv->LpsIdleCount++;
	}

_func_exit_;
}


//
//	Description:
//		Leave the leisure power save mode.
//
void LPS_Leave(PADAPTER padapter)
{
	struct pwrctrl_priv	*pwrpriv = &padapter->pwrctrlpriv;
	struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);

_func_enter_;

	//printk("LeisurePSLeave()...\n");

	if (pwrpriv->bLeisurePs)
	{	
		if(pwrpriv->pwr_mode != PS_MODE_ACTIVE)
		{
			set_ps_mode(padapter, PS_MODE_ACTIVE, 0);
		}
	}

_func_exit_;
}

#endif

//
// Description: Leave all power save mode: LPS, FwLPS, IPS if needed.
// Move code to function by tynli. 2010.03.26. 
//
void LeaveAllPowerSaveMode(IN PADAPTER Adapter)
{
	struct mlme_priv	*pmlmepriv = &(Adapter->mlmepriv);
	u32 LPSLeaveTimeOut = 10000;
	//u32 IPSLeaveTimeOut = 10000;

_func_enter_;

	printk("%s.....\n",__FUNCTION__);
	if (check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE)
	{ //connect
#ifdef CONFIG_LPS
		printk("==> leave LPS.......\n");
		LPS_Leave(Adapter);

		if (Adapter->pwrctrlpriv.bLeisurePs)
		{
			BOOLEAN bAwake = _TRUE;
			Adapter->HalFunc.GetHwRegHandler(Adapter, HW_VAR_FWLPS_RF_ON, (u8 *)(&bAwake));
			while(!bAwake)
			{
				usleep_os(100);
				LPSLeaveTimeOut--;
				if(LPSLeaveTimeOut <= 0)
				{
					printk("Wait for FW LPS leave too long!!! LPSLeaveTimeOut  = %d\n", LPSLeaveTimeOut );
					break;
				}
				Adapter->HalFunc.GetHwRegHandler(Adapter, HW_VAR_FWLPS_RF_ON, (u8 *)(&bAwake));
			}
		}
#endif
	}
	else
	{
		if(Adapter->pwrctrlpriv.current_rfpwrstate== rf_off)
		{
			printk("==> leave IPS.......\n");
			#ifdef CONFIG_AUTOSUSPEND
			if(Adapter->registrypriv.usbss_enable)
			{
				#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
				usb_disable_autosuspend(Adapter->dvobjpriv.pusbdev);
				#elif (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,22) && LINUX_VERSION_CODE<=KERNEL_VERSION(2,6,34))
				Adapter->dvobjpriv.pusbdev->autosuspend_disabled = Adapter->bDisableAutosuspend;//autosuspend disabled by the user
				#endif
			}
			else
			#endif
			{
				#ifdef CONFIG_IPS
				if(_FALSE == ips_leave(Adapter))
				{
					printk("======> ips_leave fail.............\n");			
				}
				#endif
			}				
		}	
	}

_func_exit_;
}

#ifdef CONFIG_PWRCTRL

/*
Caller:ISR handler...

This will be called when CPWM interrupt is up.

using to update cpwn of drv; and drv willl make a decision to up or down pwr level
*/
void cpwm_int_hdl(_adapter *padapter, struct reportpwrstate_parm *preportpwrstate)
{
	struct pwrctrl_priv *pwrpriv = &(padapter->pwrctrlpriv);
	struct cmd_priv	*pcmdpriv = &(padapter->cmdpriv);
	struct xmit_priv	*pxmitpriv = &(padapter->xmitpriv);

_func_enter_;

	if(pwrpriv->cpwm_tog == ((preportpwrstate->state)&0x80)){
		RT_TRACE(_module_rtl871x_pwrctrl_c_,_drv_err_,("cpwm_int_hdl : cpwm_tog = %x this time cpwm=0x%x  toggle bit didn't change !!!\n",pwrpriv->cpwm_tog ,preportpwrstate->state));	
		goto exit;
	}

	_enter_pwrlock(&pwrpriv->lock);

	pwrpriv->cpwm = (preportpwrstate->state)&0xf;

	if(pwrpriv->cpwm >= PS_STATE_S2){
		if(pwrpriv->alives & CMD_ALIVE)
			_up_sema(&(pcmdpriv->cmd_queue_sema));

		if(pwrpriv->alives & XMIT_ALIVE)
			_up_sema(&(pxmitpriv->xmit_sema));
	}
	pwrpriv->cpwm_tog=  (preportpwrstate->state)&0x80;
	_exit_pwrlock(&pwrpriv->lock);
exit:
	RT_TRACE(_module_rtl871x_pwrctrl_c_,_drv_err_,("cpwm_int_hdl : cpwm = %x !!!\n",pwrpriv->cpwm));

_func_exit_;

}


__inline static void	register_task_alive(struct pwrctrl_priv *pwrctrl, uint tag)
{
_func_enter_;
		pwrctrl->alives |= tag;
_func_exit_;
}

__inline static void	unregister_task_alive(struct pwrctrl_priv *pwrctrl, uint tag)
{
_func_enter_;

	if (pwrctrl->alives & tag)
		pwrctrl->alives ^= tag;

_func_exit_;	
}
#endif


void	init_pwrctrl_priv(_adapter *padapter)
{
	struct pwrctrl_priv *pwrctrlpriv = &padapter->pwrctrlpriv;
	struct registry_priv* pregistrypriv = &padapter->registrypriv;
	struct registry_priv  *registry_par = &padapter->registrypriv;

_func_enter_;

	_memset((unsigned char *)pwrctrlpriv, 0, sizeof(struct pwrctrl_priv));

#ifdef PLATFORM_WINDOWS
	pwrctrlpriv->pnp_current_pwr_state=NdisDeviceStateD0;
#endif

	_init_pwrlock(&pwrctrlpriv->lock);
	pwrctrlpriv->current_rfpwrstate = rf_on;
	pwrctrlpriv->ips_enter_cnts=0;
	pwrctrlpriv->ips_leave_cnts=0;

	pwrctrlpriv->pwr_state_check_inverval = 2000;
	pwrctrlpriv->pwr_state_check_cnts = 0;
	pwrctrlpriv->bInternalAutoSuspend = _FALSE;
	pwrctrlpriv->bkeepfwalive = _FALSE;
	
#ifdef CONFIG_AUTOSUSPEND	
#ifdef SUPPORT_HW_RFOFF_DETECTED
	pwrctrlpriv->pwr_state_check_inverval = (pwrctrlpriv->bHWPwrPindetect) ?1000:2000;		
#endif	
#endif
	
	pwrctrlpriv->LpsIdleCount = 0;
	//pwrctrlpriv->FWCtrlPSMode =padapter->registrypriv.power_mgnt;// PS_MODE_MIN;
	pwrctrlpriv->power_mgnt =padapter->registrypriv.power_mgnt;// PS_MODE_MIN;
	pwrctrlpriv->bLeisurePs = (PS_MODE_ACTIVE != pwrctrlpriv->power_mgnt)?_TRUE:_FALSE;

	pwrctrlpriv->bFwCurrentInPSMode = _FALSE;

	pwrctrlpriv->cpwm = PS_STATE_S4;

	pwrctrlpriv->pwr_mode = PS_MODE_ACTIVE;

	
	pwrctrlpriv->smart_ps = 0;

	pwrctrlpriv->tog = 0x80;

#ifdef PLATFORM_LINUX		
	_init_timer(&(pwrctrlpriv->pwr_state_check_timer), padapter->pnetdev, pwr_state_check_handler, (u8 *)padapter);
#endif



_func_exit_;

}


void	free_pwrctrl_priv(_adapter *adapter)
{
	struct pwrctrl_priv *pwrctrlpriv = &adapter->pwrctrlpriv;

_func_enter_;

	_memset((unsigned char *)pwrctrlpriv, 0, sizeof(struct pwrctrl_priv));

	_free_pwrlock(&pwrctrlpriv->lock);

_func_exit_;
}


/*
Caller: xmit_thread

Check if the fw_pwrstate is okay for xmit.
If not (cpwm is less than P1 state), then the sub-routine
will raise the cpwm to be greater than or equal to P1. 

Calling Context: Passive

Return Value:

_SUCCESS: xmit_thread can write fifo/txcmd afterwards.
_FAIL: xmit_thread can not do anything.
*/
sint register_tx_alive(_adapter *padapter)
{
	uint res = _SUCCESS;
	
#ifdef CONFIG_PWRCTRL

	struct pwrctrl_priv *pwrctrl = &padapter->pwrctrlpriv;

_func_enter_;

	_enter_pwrlock(&pwrctrl->lock);

	register_task_alive(pwrctrl, XMIT_ALIVE);
	RT_TRACE(_module_rtl871x_pwrctrl_c_,_drv_err_,("register_tx_alive: cpwm:%d alives:%x\n", pwrctrl->cpwm, pwrctrl->alives));

	if(pwrctrl->cpwm < PS_STATE_S2){
		set_rpwm(padapter, PS_STATE_S3);
		res = _FAIL;
	}

	_exit_pwrlock(&pwrctrl->lock);
	
_func_exit_;

#endif	/* CONFIG_PWRCTRL */

	return res;	

}

/*
Caller: cmd_thread

Check if the fw_pwrstate is okay for issuing cmd.
If not (cpwm should be is less than P2 state), then the sub-routine
will raise the cpwm to be greater than or equal to P2. 

Calling Context: Passive

Return Value:

_SUCCESS: cmd_thread can issue cmds to firmware afterwards.
_FAIL: cmd_thread can not do anything.
*/
sint register_cmd_alive(_adapter *padapter)
{
	uint res = _SUCCESS;
	
#ifdef CONFIG_PWRCTRL

	struct pwrctrl_priv *pwrctrl = &padapter->pwrctrlpriv;

_func_enter_;

	_enter_pwrlock(&pwrctrl->lock);

	register_task_alive(pwrctrl, CMD_ALIVE);
	RT_TRACE(_module_rtl871x_pwrctrl_c_,_drv_err_,("register_cmd_alive: cpwm:%d alives:%x\n", pwrctrl->cpwm, pwrctrl->alives));

	if(pwrctrl->cpwm < PS_STATE_S2){
		set_rpwm(padapter, PS_STATE_S3);
		res = _FAIL;
	}

	_exit_pwrlock(&pwrctrl->lock);
_func_exit_;
#endif

	return res;
}


/*
Caller: rx_isr

Calling Context: Dispatch/ISR

Return Value:

*/
sint register_rx_alive(_adapter *padapter)
{

#ifdef CONFIG_PWRCTRL

	struct pwrctrl_priv *pwrctrl = &padapter->pwrctrlpriv;

_func_enter_;

	_enter_pwrlock(&pwrctrl->lock);

	register_task_alive(pwrctrl, RECV_ALIVE);

	RT_TRACE(_module_rtl871x_pwrctrl_c_,_drv_err_,("register_rx_alive: cpwm:%d alives:%x\n", pwrctrl->cpwm, pwrctrl->alives));

	_exit_pwrlock(&pwrctrl->lock);

_func_exit_;
	
#endif /*CONFIG_PWRCTRL*/

	return _SUCCESS;
}


/*
Caller: evt_isr or evt_thread

Calling Context: Dispatch/ISR or Passive

Return Value:
*/
sint register_evt_alive(_adapter *padapter)
{

#ifdef CONFIG_PWRCTRL

	struct pwrctrl_priv *pwrctrl = &padapter->pwrctrlpriv;

_func_enter_;

	_enter_pwrlock(&pwrctrl->lock);

	register_task_alive(pwrctrl, EVT_ALIVE);

	RT_TRACE(_module_rtl871x_pwrctrl_c_,_drv_info_,("register_evt_alive: cpwm:%d alives:%x\n", pwrctrl->cpwm, pwrctrl->alives));

	_exit_pwrlock(&pwrctrl->lock);

_func_exit_;

#endif /*CONFIG_PWRCTRL*/

	return _SUCCESS;
}


/*
Caller: ISR

If ISR's txdone,
No more pkts for TX,
Then driver shall call this fun. to power down firmware again.
*/

void unregister_tx_alive(_adapter *padapter)
{
#ifdef CONFIG_PWRCTRL

	struct pwrctrl_priv *pwrctrl = &padapter->pwrctrlpriv;

_func_enter_;

	_enter_pwrlock(&pwrctrl->lock);

	unregister_task_alive(pwrctrl, XMIT_ALIVE);

	if((pwrctrl->cpwm > PS_STATE_S2) && (pwrctrl->pwr_mode > PS_MODE_ACTIVE)){
		if(pwrctrl->alives == 0){
			set_rpwm(padapter, PS_STATE_S0);
		}
	}

	RT_TRACE(_module_rtl871x_pwrctrl_c_,_drv_err_,("unregister_tx_alive: cpwm:%d alives:%x\n", pwrctrl->cpwm, pwrctrl->alives));
	
	_exit_pwrlock(&pwrctrl->lock);

_func_exit_;

#endif /*CONFIG_PWRCTRL*/
}

/*
Caller: ISR

If ISR's txdone,
No more pkts for TX,
Then driver shall call this fun. to power down firmware again.
*/

void unregister_cmd_alive(_adapter *padapter)
{
#ifdef CONFIG_PWRCTRL

	struct pwrctrl_priv *pwrctrl = &padapter->pwrctrlpriv;

_func_enter_;

	_enter_pwrlock(&pwrctrl->lock);

	unregister_task_alive(pwrctrl, CMD_ALIVE);

	if((pwrctrl->cpwm > PS_STATE_S2) && (pwrctrl->pwr_mode > PS_MODE_ACTIVE)){
		if((pwrctrl->alives == 0)&&(check_fwstate(&padapter->mlmepriv, _FW_UNDER_LINKING)!=_TRUE)){
			set_rpwm(padapter, PS_STATE_S0);
		}
	}

	RT_TRACE(_module_rtl871x_pwrctrl_c_,_drv_err_,("unregister_cmd_alive: cpwm:%d alives:%x\n", pwrctrl->cpwm, pwrctrl->alives));

	_exit_pwrlock(&pwrctrl->lock);

_func_exit_;

#endif /*CONFIG_PWRCTRL*/
}


/*

Caller: ISR

*/
void unregister_rx_alive(_adapter *padapter)
{
#ifdef CONFIG_PWRCTRL

	struct pwrctrl_priv *pwrctrl = &padapter->pwrctrlpriv;

_func_enter_;

	_enter_pwrlock(&pwrctrl->lock);

	unregister_task_alive(pwrctrl, RECV_ALIVE);

	RT_TRACE(_module_rtl871x_pwrctrl_c_,_drv_err_,("unregister_rx_alive: cpwm:%d alives:%x\n", pwrctrl->cpwm, pwrctrl->alives));
	
	_exit_pwrlock(&pwrctrl->lock);

_func_exit_;

#endif
}


void unregister_evt_alive(_adapter *padapter)
{
#ifdef CONFIG_PWRCTRL

	struct pwrctrl_priv *pwrctrl = &padapter->pwrctrlpriv;

_func_enter_;

	_enter_pwrlock(&pwrctrl->lock);

	unregister_task_alive(pwrctrl, EVT_ALIVE);

	RT_TRACE(_module_rtl871x_pwrctrl_c_,_drv_err_,("unregister_evt_alive: cpwm:%d alives:%x\n", pwrctrl->cpwm, pwrctrl->alives));
	
	_exit_pwrlock(&pwrctrl->lock);

_func_exit_;

#endif /*CONFIG_PWRCTRL*/
}



