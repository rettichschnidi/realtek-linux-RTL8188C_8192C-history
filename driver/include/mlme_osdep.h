
#ifndef	__MLME_OSDEP_H_
#define __MLME_OSDEP_H_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_MPIXEL)
extern int time_after(u32 now, u32 old);
#endif

extern void init_mlme_timer(_adapter *padapter);
extern void os_indicate_disconnect( _adapter *adapter );
extern void os_indicate_connect( _adapter *adapter );
extern void report_sec_ie(_adapter *adapter,u8 authmode,u8 *sec_ie);

#ifdef CONFIG_AP_MODE
void indicate_sta_assoc_event(_adapter *padapter, struct sta_info *psta);
void indicate_sta_disassoc_event(_adapter *padapter, struct sta_info *psta);
#endif

#endif	//_MLME_OSDEP_H_

