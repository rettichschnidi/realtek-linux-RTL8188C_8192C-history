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
/*
 * Automatically generated C config: don't edit
 */
#define AUTOCONF_INCLUDED
#define RTL871X_MODULE_NAME "92CU"
#define DRV_NAME "rtl8192cu"

//#define CONFIG_DEBUG_RTL871X 1

#define CONFIG_USB_HCI	1
#undef  CONFIG_SDIO_HCI
#undef CONFIG_PCI_HCI

#undef CONFIG_RTL8711
#undef  CONFIG_RTL8712
#define	CONFIG_RTL8192C 1
//#define	CONFIG_RTL8192D 1


//#define CONFIG_LITTLE_ENDIAN 1 //move to Makefile depends on platforms
//#undef CONFIG_BIG_ENDIAN

#undef PLATFORM_WINDOWS
#undef PLATFORM_OS_XP 
#undef PLATFORM_OS_CE


#define PLATFORM_LINUX 1

//#define CONFIG_PWRCTRL	1
//#define CONFIG_H2CLBK 1

//#undef CONFIG_EMBEDDED_FWIMG
#define CONFIG_EMBEDDED_FWIMG 1

#define CONFIG_R871X_TEST 1

#define CONFIG_80211N_HT 1

#define CONFIG_RECV_REORDERING_CTRL 1

//#define CONFIG_RTL8712_TCP_CSUM_OFFLOAD_RX 1

//#define CONFIG_DRVEXT_MODULE 1

#ifndef CONFIG_MP_INCLUDED
#define CONFIG_IPS	1
#ifdef CONFIG_IPS
//#define CONFIG_IPS_LEVEL_2	
#endif
#define CONFIG_LPS	1
#define CONFIG_BT_COEXIST  	1
#define CONFIG_ANTENNA_DIVERSITY	1
#define SUPPORT_HW_RFOFF_DETECTED	1
#else
#define MP_IWPRIV_SUPPORT 1
#endif

#ifdef PLATFORM_LINUX
//	#define CONFIG_PROC_DEBUG 1
	
	#define CONFIG_AP_MODE 1
	#define CONFIG_NATIVEAP_MLME 1

	//	Added by Albert 20110314
	//	The P2P code won't be included into the driver if we change the following value from 1 to 0
	#define P2P_INCLUDED	1

	#ifdef CONFIG_AP_MODE
		#ifndef CONFIG_NATIVEAP_MLME
			#define CONFIG_HOSTAPD_MLME 1
		#endif			
		//#define CONFIG_FIND_BEST_CHANNEL
	#endif
	
#endif

#ifdef CONFIG_RTL8192C

	#define DBG 0

	#define CONFIG_DEBUG_RTL8192C		1

	#define DEV_BUS_PCI_INTERFACE				1
	#define DEV_BUS_USB_INTERFACE				2	

	#define RTL8192C_RX_PACKET_NO_INCLUDE_CRC	1

	#define SUPPORTED_BLOCK_IO
	
	#ifdef CONFIG_USB_HCI

		#define DEV_BUS_TYPE	DEV_BUS_USB_INTERFACE

		#ifdef CONFIG_MINIMAL_MEMORY_USAGE
			#define USB_TX_AGGREGATION	0
			#define USB_RX_AGGREGATION	0
		#else
			#define USB_TX_AGGREGATION	1
			#define USB_RX_AGGREGATION	1
		#endif
		
	        #ifdef CONFIG_WISTRON_PLATFORM	
			#define SILENT_RESET_FOR_SPECIFIC_PLATFOM	1				
		#endif

		#define RTL8192CU_FW_DOWNLOAD_ENABLE	1

		#define CONFIG_ONLY_ONE_OUT_EP_TO_LOW	0
	
		#define CONFIG_OUT_EP_WIFI_MODE	0

		//#define CONFIG_USB_INTERRUPT_IN_PIPE 1

		#define ENABLE_USB_DROP_INCORRECT_OUT	0

		#define RTL8192CU_ASIC_VERIFICATION	0	// For ASIC verification.

		#define RTL8192CU_ADHOC_WORKAROUND_SETTING 1

		#ifdef PLATFORM_LINUX
			#define CONFIG_SKB_COPY 		1//for amsdu
			#define CONFIG_PREALLOC_RECV_SKB	1
			#define CONFIG_REDUCE_USB_TX_INT	1
			#define CONFIG_EASY_REPLACEMENT	1
			#ifdef CONFIG_WISTRON_PLATFORM
			#define DYNAMIC_ALLOCIATE_VENDOR_CMD	0
			#else
			#define DYNAMIC_ALLOCIATE_VENDOR_CMD	1
			#endif

			//#define CONFIG_USE_USB_BUFFER_ALLOC 1
			
		#endif

		#ifdef CONFIG_R871X_TEST

		#endif

	#endif

	#ifdef CONFIG_PCI_HCI

		#define DEV_BUS_TYPE	DEV_BUS_PCI_INTERFACE
		
	#endif

	
	#define DISABLE_BB_RF	0	

	#define RTL8191C_FPGA_NETWORKTYPE_ADHOC 0

	//#define FW_PROCESS_VENDOR_CMD 1

	#ifdef CONFIG_MP_INCLUDED
		#define MP_DRIVER 1
		#undef USB_TX_AGGREGATION
		#undef USB_RX_AGGREGATION
	#else
		#define MP_DRIVER 0
	#endif

	
	#define MEM_ALLOC_REFINE // general, now applied on 8192c only
	#define INDICATE_SCAN_COMPLETE_EVENT// general
	#define USB_INTERFERENCE_ISSUE // this should be checked in all usb interface
	#define WPA_SET_ENCRYPTION_REFINE // general
	#define STADEL_EVENT_REMOVE_SCANNED_ENTRY // general
	#define USE_ATOMIC_EVENT_SEQ // general
	#define HANDLE_JOINBSS_ON_ASSOC_RSP // general

#endif

//#define DBG_TX
//#define DBG_XMIT_BUF

