/*
 * hostapd / P2P integration
 * Copyright (c) 2009-2010, Atheros Communications
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "utils/includes.h"

#include "utils/common.h"
#include "common/ieee802_11_defs.h"
#include "p2p/p2p.h"
#include "hostapd.h"
#include "ap_config.h"
#include "ap_drv_ops.h"
#include "sta_info.h"
#include "p2p_hostapd.h"


#ifdef CONFIG_P2P

int hostapd_p2p_get_mib_sta(struct hostapd_data *hapd, struct sta_info *sta,
			    char *buf, size_t buflen)
{
	if (sta->p2p_ie == NULL)
		return 0;

	return p2p_ie_text(sta->p2p_ie, buf, buf + buflen);
}


int hostapd_p2p_set_noa(struct hostapd_data *hapd, u8 count, int start,
			int duration)
{
	wpa_printf(MSG_DEBUG, "P2P: Set NoA parameters: count=%u start=%d "
		   "duration=%d", count, start, duration);

	if (count == 0) {
		hapd->noa_enabled = 0;
		hapd->noa_start = 0;
		hapd->noa_duration = 0;
	}

	if (count != 255) {
		wpa_printf(MSG_DEBUG, "P2P: Non-periodic NoA - set "
			   "NoA parameters");
		return hostapd_driver_set_noa(hapd, count, start, duration);
	}

	hapd->noa_enabled = 1;
	hapd->noa_start = start;
	hapd->noa_duration = duration;

	if (hapd->num_sta_no_p2p == 0) {
		wpa_printf(MSG_DEBUG, "P2P: No legacy STAs connected - update "
			   "periodic NoA parameters");
		return hostapd_driver_set_noa(hapd, count, start, duration);
	}

	wpa_printf(MSG_DEBUG, "P2P: Legacy STA(s) connected - do not enable "
		   "periodic NoA");

	return 0;
}


void hostapd_p2p_non_p2p_sta_connected(struct hostapd_data *hapd)
{
	wpa_printf(MSG_DEBUG, "P2P: First non-P2P device connected");

	if (hapd->noa_enabled) {
		wpa_printf(MSG_DEBUG, "P2P: Disable periodic NoA");
		hostapd_driver_set_noa(hapd, 0, 0, 0);
	}
}


void hostapd_p2p_non_p2p_sta_disconnected(struct hostapd_data *hapd)
{
	wpa_printf(MSG_DEBUG, "P2P: Last non-P2P device disconnected");

	if (hapd->noa_enabled) {
		wpa_printf(MSG_DEBUG, "P2P: Enable periodic NoA");
		hostapd_driver_set_noa(hapd, 255, hapd->noa_start,
				       hapd->noa_duration);
	}
}

#endif /* CONFIG_P2P */


#ifdef CONFIG_P2P_MANAGER
u8 * hostapd_eid_p2p_manage(struct hostapd_data *hapd, u8 *eid)
{
	u8 bitmap;
	u16 device_info_bitmap = 0;
	u16 sess_mng_ctl_port = 554; // Default port
	u16 wfd_dev_max_thruput = 8; // 8Mbps max throughput

	*eid++ = WLAN_EID_VENDOR_SPECIFIC;
	*eid++ = 4 + 3 + 1;
	WPA_PUT_BE24(eid, OUI_WFA);
	eid += 3;
	*eid++ = P2P_OUI_TYPE;

	*eid++ = P2P_ATTR_MANAGEABILITY;
	WPA_PUT_LE16(eid, 1);
	eid += 2;
	bitmap = P2P_MAN_DEVICE_MANAGEMENT;
	if (hapd->conf->p2p & P2P_ALLOW_CROSS_CONNECTION)
		bitmap |= P2P_MAN_CROSS_CONNECTION_PERMITTED;
	bitmap |= P2P_MAN_COEXISTENCE_OPTIONAL;
	*eid++ = bitmap;

	*eid++ = WLAN_EID_VENDOR_SPECIFIC;
	*eid++ = 13; // length
	WPA_PUT_BE24(eid, OUI_WFA);
	eid += 3;
	*eid++ = 0xA; // WFD OUI TYPE

	*eid++ = 0;  // Device Info subelement
	WPA_PUT_BE16(eid, 6); //length
	eid += 2;

	device_info_bitmap |= 1 << 4; //available for WFD session

	WPA_PUT_BE16(eid, device_info_bitmap);
	eid += 2;
	WPA_PUT_BE16(eid, sess_mng_ctl_port);
	eid += 2;
	WPA_PUT_BE16(eid, wfd_dev_max_thruput);
	eid += 2;

	return eid;
}
#endif /* CONFIG_P2P_MANAGER */
