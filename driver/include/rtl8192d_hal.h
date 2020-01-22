#ifndef __RTL8192D_HAL_H__
#define __RTL8192D_HAL_H__

#include "rtl8192d_spec.h"
#include "Hal8192DPhyReg.h"
#include "Hal8192DPhyCfg.h"
#include "rtl8192d_rf.h"
#include "rtl8192d_dm.h"
#include "rtl8192d_recv.h"
#include "rtl8192d_xmit.h"
#include "rtl8192d_cmd.h"

#if (DEV_BUS_TYPE == DEV_BUS_PCI_INTERFACE)

	#define RTL819X_DEFAULT_RF_TYPE			RF_2T2R
	//#define RTL819X_DEFAULT_RF_TYPE			RF_1T2R
	#define RTL819X_TOTAL_RF_PATH				2

	//2TODO:  The following need to check!!
	#define	RTL8192C_FW_IMG					"rtl8192CE\\rtl8192cfw.bin"
	#define	RTL8188C_FW_IMG					"rtl8192CE\\rtl8192cfw.bin"

	#define RTL8188C_PHY_REG					"rtl8192CE\\PHY_REG_1T.txt"
	#define RTL8188C_PHY_RADIO_A				"rtl8192CE\\radio_a_1T.txt"
	#define RTL8188C_PHY_RADIO_B				"rtl8192CE\\radio_b_1T.txt"
	#define RTL8188C_AGC_TAB					"rtl8192CE\\AGC_TAB_1T.txt"
	#define RTL8188C_PHY_MACREG				"rtl8192CE\\MACREG_1T.txt"

	#define RTL8192C_PHY_REG					"rtl8192CE\\PHY_REG_2T.txt"
	#define RTL8192C_PHY_RADIO_A				"rtl8192CE\\radio_a_2T.txt"
	#define RTL8192C_PHY_RADIO_B				"rtl8192CE\\radio_b_2T.txt"
	#define RTL8192C_AGC_TAB					"rtl8192CE\\AGC_TAB_2T.txt"
	#define RTL8192C_PHY_MACREG				"rtl8192CE\\MACREG_2T.txt"

	#define RTL819X_PHY_MACPHY_REG			"rtl8192CE\\MACPHY_reg.txt"
	#define RTL819X_PHY_MACPHY_REG_PG		"rtl8192CE\\MACPHY_reg_PG.txt"
	#define RTL819X_PHY_MACREG				"rtl8192CE\\MAC_REG.txt"
	#define RTL819X_PHY_REG					"rtl8192CE\\PHY_REG.txt"
	#define RTL819X_PHY_REG_1T2R				"rtl8192CE\\PHY_REG_1T2R.txt"
	#define RTL819X_PHY_REG_to1T1R				"rtl8192CE\\phy_to1T1R_a.txt"
	#define RTL819X_PHY_REG_to1T2R				"rtl8192CE\\phy_to1T2R.txt"
	#define RTL819X_PHY_REG_to2T2R				"rtl8192CE\\phy_to2T2R.txt"
	#define RTL819X_PHY_REG_PG					"rtl8192CE\\PHY_REG_PG.txt"
	#define RTL819X_AGC_TAB					"rtl8192CE\\AGC_TAB.txt"
	#define RTL819X_PHY_RADIO_A				"rtl8192CE\\radio_a.txt"
	#define RTL819X_PHY_RADIO_A_1T			"rtl8192CE\\radio_a_1t.txt"
	#define RTL819X_PHY_RADIO_A_2T			"rtl8192CE\\radio_a_2t.txt"
	#define RTL819X_PHY_RADIO_B				"rtl8192CE\\radio_b.txt"
	#define RTL819X_PHY_RADIO_B_GM			"rtl8192CE\\radio_b_gm.txt"
	#define RTL819X_PHY_RADIO_C				"rtl8192CE\\radio_c.txt"
	#define RTL819X_PHY_RADIO_D				"rtl8192CE\\radio_d.txt"
	#define RTL819X_EEPROM_MAP				"rtl8192CE\\8192ce.map"
	#define RTL819X_EFUSE_MAP					"rtl8192CE\\8192ce.map"

	#define RTL8192D_TEST_FW_IMG_FILE			"rtl8192DE\\rtl8192dfw_test.bin"
	#define RTL8192D_TEST_PHY_REG_PG_FILE	"rtl8192DE\\PHY_REG_PG_test.txt"

	#define RTL8192D_TEST_PHY_REG_FILE		"rtl8192DE\\PHY_REG_test.txt"	
	#define RTL8192D_TEST_PHY_RADIO_A_FILE	"rtl8192DE\\radio_a_test.txt"
	#define RTL8192D_TEST_PHY_RADIO_B_FILE	"rtl8192DE\\radio_b_test.txt"	
	#define RTL8192D_TEST_AGC_TAB_2G			"rtl8192DE\\AGC_TAB_2G_test.txt"
	#define RTL8192D_TEST_AGC_TAB_5G			"rtl8192DE\\AGC_TAB_5G_test.txt"
	#define RTL8192D_TEST_MAC_REG_FILE		"rtl8192DE\\MAC_REG_test.txt"

	// The file name "_2T" is for 92CE, "_1T"  is for 88CE. Modified by tynli. 2009.11.24.
	#define Rtl819XFwImageArray					Rtl8192DEFwImgArray
	#define Rtl819XMAC_Array					Rtl8192DEMAC_2TArray
	#define Rtl819XAGCTAB_Array					Rtl8192DEAGCTAB_Array
	#define Rtl819XAGCTAB_5GArray				Rtl8192DEAGCTAB_5GArray
	#define Rtl819XAGCTAB_2GArray				Rtl8192DEAGCTAB_2GArray
	#define Rtl819XPHY_REG_2TArray				Rtl8192DEPHY_REG_2TArray			
	#define Rtl819XPHY_REG_1TArray				Rtl8192DEPHY_REG_1TArray
	#define Rtl819XRadioA_2TArray				Rtl8192DERadioA_2TArray
	#define Rtl819XRadioA_1TArray				Rtl8192DERadioA_1TArray
	#define Rtl819XRadioB_2TArray				Rtl8192DERadioB_2TArray
	#define Rtl819XRadioB_1TArray				Rtl8192DERadioB_1TArray
	#define Rtl819XPHY_REG_Array_PG 			Rtl8192DEPHY_REG_Array_PG
	#define Rtl819XPHY_REG_Array_MP 			Rtl8192DEPHY_REG_Array_MP
	#define Rtl819XAGCTAB_2TArray				Rtl8192DEAGCTAB_2TArray
	#define Rtl819XAGCTAB_1TArray				Rtl8192DEAGCTAB_1TArray

#elif (DEV_BUS_TYPE == DEV_BUS_USB_INTERFACE)

	#include "Hal8192DUHWImg.h"
	#include "Hal8192DUTestHWImg.h"

	//2TODO: We should define 8192S firmware related macro settings here!!
	#define RTL819X_DEFAULT_RF_TYPE		RF_1T2R
	#define RTL819X_TOTAL_RF_PATH			2

	//2TODO:  The following need to check!!
	#define	RTL8192C_FW_IMG					"rtl8192CU\\rtl8192cfw.bin"
	#define	RTL8188C_FW_IMG					"rtl8192CU\\rtl8192cfw.bin"
	#define	RTL8192D_FW_IMG					"rtl8192DU\\rtl8192dfw.bin"
	//#define RTL819X_FW_BOOT_IMG   				"rtl8192CU\\boot.img"
	//#define RTL819X_FW_MAIN_IMG				"rtl8192CU\\main.img"
	//#define RTL819X_FW_DATA_IMG				"rtl8192CU\\data.img"

	#define RTL8188C_PHY_REG					"rtl8192CU\\PHY_REG_1T.txt"	
	#define RTL8188C_PHY_RADIO_A				"rtl8192CU\\radio_a_1T.txt"
	#define RTL8188C_PHY_RADIO_B				"rtl8192CU\\radio_b_1T.txt"	
	#define RTL8188C_AGC_TAB					"rtl8192CU\\AGC_TAB_1T.txt"
	#define RTL8188C_PHY_MACREG				"rtl8192CU\\MAC_REG.txt"
	
	#define RTL8192C_PHY_REG					"rtl8192CU\\PHY_REG.txt"	
	#define RTL8192C_PHY_RADIO_A				"rtl8192CU\\radio_a.txt"
	#define RTL8192C_PHY_RADIO_B				"rtl8192CU\\radio_b.txt"	
	#define RTL8192C_AGC_TAB					"rtl8192CU\\AGC_TAB.txt"
	#define RTL8192C_PHY_MACREG				"rtl8192CU\\MAC_REG.txt"

	//For 92DU
	#define RTL8192D_PHY_REG					"rtl8192DU\\PHY_REG.txt"
	#define RTL8192D_PHY_REG_PG				"rtl8192DU\\PHY_REG_PG.txt"
	#define RTL8192D_PHY_REG_MP				"rtl8192DU\\PHY_REG_MP.txt"			
	
	#define RTL8192D_AGC_TAB					"rtl8192DU\\AGC_TAB.txt"
	#define RTL8192D_AGC_TAB_2G				"rtl8192DU\\AGC_TAB_2G.txt"
	#define RTL8192D_AGC_TAB_5G				"rtl8192DU\\AGC_TAB_5G.txt"
	#define RTL8192D_PHY_RADIO_A				"rtl8192DU\\radio_a.txt"
	#define RTL8192D_PHY_RADIO_B				"rtl8192DU\\radio_b.txt"
	#define RTL8192D_PHY_RADIO_A_intPA		"rtl8192DU\\radio_a_intPA.txt"
	#define RTL8192D_PHY_RADIO_B_intPA		"rtl8192DU\\radio_b_intPA.txt"
	#define RTL8192D_PHY_MACREG				"rtl8192DU\\MAC_REG.txt"
		
	#define RTL819X_PHY_MACPHY_REG			"rtl8192CU\\MACPHY_reg.txt"
	#define RTL819X_PHY_MACPHY_REG_PG		"rtl8192CU\\MACPHY_reg_PG.txt"
	#define RTL819X_PHY_MACREG				"rtl8192CU\\MAC_REG.txt"
	#define RTL819X_PHY_REG					"rtl8192CU\\PHY_REG.txt"
	#define RTL819X_PHY_REG_1T2R				"rtl8192CU\\PHY_REG_1T2R.txt"
	#define RTL819X_PHY_REG_to1T1R				"rtl8192CU\\phy_to1T1R_a.txt"
	#define RTL819X_PHY_REG_to1T2R				"rtl8192CU\\phy_to1T2R.txt"
	#define RTL819X_PHY_REG_to2T2R				"rtl8192CU\\phy_to2T2R.txt"
	#define RTL819X_PHY_REG_PG					"rtl8192CU\\PHY_REG_PG.txt"
	#define RTL819X_PHY_REG_MP 				"rtl8192CU\\PHY_REG_MP.txt" 		

	#define RTL819X_AGC_TAB					"rtl8192CU\\AGC_TAB.txt"
	#define RTL819X_PHY_RADIO_A				"rtl8192CU\\radio_a.txt"
	#define RTL819X_PHY_RADIO_B				"rtl8192CU\\radio_b.txt"			
	#define RTL819X_PHY_RADIO_B_GM			"rtl8192CU\\radio_b_gm.txt"	
	#define RTL819X_PHY_RADIO_C				"rtl8192CU\\radio_c.txt"
	#define RTL819X_PHY_RADIO_D				"rtl8192CU\\radio_d.txt"
	#define RTL819X_EEPROM_MAP				"rtl8192CU\\8192cu.map"
	#define RTL819X_EFUSE_MAP					"rtl8192CU\\8192cu.map"
	#define RTL819X_PHY_RADIO_A_1T			"rtl8192CU\\radio_a_1t.txt"
	#define RTL819X_PHY_RADIO_A_2T			"rtl8192CU\\radio_a_2t.txt"

	#define RTL8192D_TEST_FW_IMG_FILE			"rtl8192DU\\rtl8192dfw_test.bin"
	#define RTL8192D_TEST_PHY_REG_PG_FILE	"rtl8192DU\\PHY_REG_PG_test.txt"

	#define RTL8192D_TEST_PHY_REG_FILE		"rtl8192DU\\PHY_REG_test.txt"	
	#define RTL8192D_TEST_PHY_RADIO_A_FILE	"rtl8192DU\\radio_a_test.txt"
	#define RTL8192D_TEST_PHY_RADIO_B_FILE	"rtl8192DU\\radio_b_test.txt"	
	#define RTL8192D_TEST_AGC_TAB_2G			"rtl8192DU\\AGC_TAB_2G_test.txt"
	#define RTL8192D_TEST_AGC_TAB_5G			"rtl8192DU\\AGC_TAB_5G_test.txt"
	#define RTL8192D_TEST_MAC_REG_FILE		"rtl8192DU\\MAC_REG_test.txt"

	// The file name "_2T" is for 92CU, "_1T"  is for 88CU. Modified by tynli. 2009.11.24.
	#define Rtl819XFwImageArray					Rtl8192DUFwImgArray
	#define Rtl819XMAC_Array					Rtl8192DUMAC_2TArray
	#define Rtl819XAGCTAB_Array					Rtl8192DUAGCTAB_Array			
	#define Rtl819XAGCTAB_5GArray				Rtl8192DUAGCTAB_5GArray
	#define Rtl819XAGCTAB_2GArray				Rtl8192DUAGCTAB_2GArray
	#define Rtl819XPHY_REG_2TArray				Rtl8192DUPHY_REG_2TArray			
	#define Rtl819XPHY_REG_1TArray				Rtl8192DUPHY_REG_1TArray
	#define Rtl819XRadioA_2TArray				Rtl8192DURadioA_2TArray
	#define Rtl819XRadioA_1TArray				Rtl8192DURadioA_1TArray
	#define Rtl819XRadioA_2T_intPAArray 			Rtl8192DURadioA_2T_intPAArray
	#define Rtl819XRadioB_2TArray				Rtl8192DURadioB_2TArray
	#define Rtl819XRadioB_1TArray				Rtl8192DURadioB_1TArray
	#define Rtl819XRadioB_2T_intPAArray 			Rtl8192DURadioB_2T_intPAArray
	#define Rtl819XPHY_REG_Array_PG 			Rtl8192DUPHY_REG_Array_PG
	#define Rtl819XPHY_REG_Array_MP 			Rtl8192DUPHY_REG_Array_MP

	#define Rtl819XAGCTAB_2TArray				Rtl8192DUAGCTAB_2TArray
	#define Rtl819XAGCTAB_1TArray				Rtl8192DUAGCTAB_1TArray

#endif

#define PageNum_128(_Len)		(u32)(((_Len)>>7) + ((_Len)&0x7F ? 1:0))


//
// Check if FW header exists. We do not consider the lower 4 bits in this case. 
// By tynli. 2009.12.04.
//
#define IS_FW_HEADER_EXIST(_pFwHdr)	((_pFwHdr->Signature&0xFFF0) == 0x92C0 ||\
									(_pFwHdr->Signature&0xFFF0) == 0x88C0 ||\
									(_pFwHdr->Signature&0xFFFF) == 0x92D0 ||\
									(_pFwHdr->Signature&0xFFFF) == 0x92D1 ||\
									(_pFwHdr->Signature&0xFFFF) == 0x92D2 ||\
									(_pFwHdr->Signature&0xFFFF) == 0x92D3 )

#define FW_8192D_SIZE				0x8000
#define FW_8192C_SIZE				0x4000
#define FW_8192C_START_ADDRESS	0x1000
#define FW_8192C_END_ADDRESS		0x3FFF

#define MAX_PAGE_SIZE				4096	// @ page : 4k bytes

typedef enum _FIRMWARE_SOURCE{
	FW_SOURCE_IMG_FILE = 0,
	FW_SOURCE_HEADER_FILE = 1,		//from header file
}FIRMWARE_SOURCE, *PFIRMWARE_SOURCE;

typedef struct _RT_FIRMWARE{
	FIRMWARE_SOURCE	eFWSource;
	u8			szFwBuffer[FW_8192D_SIZE];
	u32			ulFwLength;
}RT_FIRMWARE, *PRT_FIRMWARE, RT_FIRMWARE_92D, *PRT_FIRMWARE_92D;

//
// This structure must be cared byte-ordering
//
// Added by tynli. 2009.12.04.
typedef struct _RT_8192D_FIRMWARE_HDR {//8-byte alinment required

	//--- LONG WORD 0 ----
	u16		Signature;	// 92C0: test chip; 92C, 88C0: test chip; 88C1: MP A-cut; 92C1: MP A-cut
	u8		Category;	// AP/NIC and USB/PCI
	u8		Function;	// Reserved for different FW function indcation, for further use when driver needs to download different FW in different conditions
	u16		Version;		// FW Version
	u8		Subversion;	// FW Subversion, default 0x00
	u8		Rsvd1;


	//--- LONG WORD 1 ----
	u8		Month;	// Release time Month field
	u8		Date;	// Release time Date field
	u8		Hour;	// Release time Hour field
	u8		Minute;	// Release time Minute field
	u16		RamCodeSize;	// The size of RAM code
	u16		Rsvd2;

	//--- LONG WORD 2 ----
	u32		SvnIdx;	// The SVN entry index
	u32		Rsvd3;

	//--- LONG WORD 3 ----
	u32		Rsvd4;
	u32		Rsvd5;

}RT_8192D_FIRMWARE_HDR, *PRT_8192D_FIRMWARE_HDR;

#define DRIVER_EARLY_INT_TIME		0x05
#define BCN_DMA_ATIME_INT_TIME		0x02

typedef	enum _BT_CoType{
	BT_2Wire			= 0,		
	BT_ISSC_3Wire	= 1,
	BT_Accel			= 2,
	BT_CSR			= 3,
	BT_CSR_ENHAN	= 4,
	BT_RTL8756		= 5,
} BT_CoType, *PBT_CoType;

typedef	enum _BT_CurState{
	BT_OFF		= 0,	
	BT_ON		= 1,
} BT_CurState, *PBT_CurState;

typedef	enum _BT_ServiceType{
	BT_SCO			= 0,	
	BT_A2DP			= 1,
	BT_HID			= 2,
	BT_HID_Idle		= 3,
	BT_Scan			= 4,
	BT_Idle			= 5,
	BT_OtherAction	= 6,
	BT_Busy			= 7,
	BT_OtherBusy		= 8,
} BT_ServiceType, *PBT_ServiceType;

typedef	enum _BT_RadioShared{
	BT_Radio_Shared 	= 0,	
	BT_Radio_Individual	= 1,
} BT_RadioShared, *PBT_RadioShared;

typedef struct _BT_COEXIST_STR{
	u8					BluetoothCoexist;
	u8					BT_Ant_Num;
	u8					BT_CoexistType;
	u8					BT_State;
	u8					BT_CUR_State;		//0:on, 1:off
	u8					BT_Ant_isolation;	//0:good, 1:bad
	u8					BT_PapeCtrl;		//0:SW, 1:SW/HW dynamic
	u8					BT_Service;			
	u8					BT_RadioSharedType;
	u8					Ratio_Tx;
	u8					Ratio_PRI;
}BT_COEXIST_STR, *PBT_COEXIST_STR;

//Added for 92D IQK setting.
typedef struct _IQK_MATRIX_REGS_SETTING{
	BOOLEAN 	bIQKDone;
#if 1
	int		Value[1][IQK_Matrix_REG_NUM];
#else	
	u32		Mark[IQK_Matrix_REG_NUM];
	u32		Value[IQK_Matrix_REG_NUM];
#endif	
}IQK_MATRIX_REGS_SETTING,*PIQK_MATRIX_REGS_SETTING;

#ifdef USB_RX_AGGREGATION_92C

typedef enum _USB_RX_AGG_MODE{
	USB_RX_AGG_DISABLE,
	USB_RX_AGG_DMA,
	USB_RX_AGG_USB,
	USB_RX_AGG_DMA_USB
}USB_RX_AGG_MODE;

#define MAX_RX_DMA_BUFFER_SIZE	10240		// 10K for 8192C RX DMA buffer

#endif


#define TX_SELE_HQ			BIT(0)		// High Queue
#define TX_SELE_LQ			BIT(1)		// Low Queue
#define TX_SELE_NQ			BIT(2)		// Normal Queue


// Note: We will divide number of page equally for each queue other than public queue!

#define TX_TOTAL_PAGE_NUMBER		0xF8
#define TX_PAGE_BOUNDARY			(TX_TOTAL_PAGE_NUMBER + 1)

// For Normal Chip Setting
// (HPQ + LPQ + NPQ + PUBQ) shall be TX_TOTAL_PAGE_NUMBER
#define NORMAL_PAGE_NUM_PUBQ		0x56


// For Test Chip Setting
// (HPQ + LPQ + PUBQ) shall be TX_TOTAL_PAGE_NUMBER
#define TEST_PAGE_NUM_PUBQ			0x89
#define TX_TOTAL_PAGE_NUMBER_92D_DUAL_MAC		0x7A
#define NORMAL_PAGE_NUM_PUBQ_92D_DUAL_MAC			0x5A
#define NORMAL_PAGE_NUM_HPQ_92D_DUAL_MAC			0x10
#define NORMAL_PAGE_NUM_LPQ_92D_DUAL_MAC			0x10
#define NORMAL_PAGE_NUM_NORMALQ_92D_DUAL_MAC		0

#define TX_PAGE_BOUNDARY_DUAL_MAC			(TX_TOTAL_PAGE_NUMBER_92D_DUAL_MAC + 1)

// For Test Chip Setting
#define WMM_TEST_TX_TOTAL_PAGE_NUMBER	0xF5
#define WMM_TEST_TX_PAGE_BOUNDARY	(WMM_TEST_TX_TOTAL_PAGE_NUMBER + 1) //F6

#define WMM_TEST_PAGE_NUM_PUBQ		0xA3
#define WMM_TEST_PAGE_NUM_HPQ		0x29
#define WMM_TEST_PAGE_NUM_LPQ		0x29


//Note: For Normal Chip Setting ,modify later
#define WMM_NORMAL_TX_TOTAL_PAGE_NUMBER	0xF5
#define WMM_NORMAL_TX_PAGE_BOUNDARY	(WMM_TEST_TX_TOTAL_PAGE_NUMBER + 1) //F6

#define WMM_NORMAL_PAGE_NUM_PUBQ		0xB0
#define WMM_NORMAL_PAGE_NUM_HPQ		0x29
#define WMM_NORMAL_PAGE_NUM_LPQ			0x1C
#define WMM_NORMAL_PAGE_NUM_NPQ		0x1C

#define WMM_NORMAL_PAGE_NUM_PUBQ_92D		0X65//0x82
#define WMM_NORMAL_PAGE_NUM_HPQ_92D		0X30//0x29
#define WMM_NORMAL_PAGE_NUM_LPQ_92D		0X30
#define WMM_NORMAL_PAGE_NUM_NPQ_92D		0X30

//-------------------------------------------------------------------------
//	Chip specific
//-------------------------------------------------------------------------
#define CHIP_92C  					BIT(0)
#define CHIP_92C_1T2R			BIT(1)
#define CHIP_8723					BIT(2) // RTL8723 With BT feature
#define CHIP_8723_DRV_REV		BIT(3) // RTL8723 Driver Revised
#define NORMAL_CHIP  				BIT(4)
#define CHIP_VENDOR_UMC			BIT(5)
#define CHIP_VENDOR_UMC_B_CUT	BIT(6) // Chip version for ECO

//for 92D
#define CHIP_92D					BIT(8)
#define CHIP_92D_SINGLEPHY             BIT(9)
#define CHIP_92D_C_CUT			BIT(10)
#define CHIP_92D_D_CUT			BIT(11)

#define IS_NORMAL_CHIP(version)  	(((version) & NORMAL_CHIP) ? _TRUE : _FALSE) 
#define IS_92C_SERIAL(version)   		(((version) & CHIP_92C) ? _TRUE : _FALSE)
#define IS_8723_SERIES(version)   	(((version) & CHIP_8723) ? _TRUE : _FALSE)
#define IS_92C_1T2R(version)			(((version) & CHIP_92C) && ((version) & CHIP_92C_1T2R))
#define IS_VENDOR_UMC(version)		(((version) & CHIP_VENDOR_UMC) ? _TRUE : _FALSE)
#define IS_VENDOR_UMC_A_CUT(version)	(((version) & CHIP_VENDOR_UMC) ? (((version) & (BIT6|BIT7)) ? _FALSE : _TRUE) : _FALSE)
#define IS_VENDOR_8723_A_CUT(version)	(((version) & CHIP_VENDOR_UMC) ? (((version) & (BIT6)) ? _FALSE : _TRUE) : _FALSE)

#define IS_92D_SINGLEPHY(version)     ((version & CHIP_92D_SINGLEPHY) ? _TRUE : _FALSE)
#define IS_92D_C_CUT(version)     ((version & CHIP_92D_C_CUT) ? _TRUE : _FALSE)
#define IS_92D_D_CUT(version)     ((version & CHIP_92D_D_CUT) ? _TRUE : _FALSE)

// 20100707 Joseph: Add vendor information into chip version definition.
// 20100902 Roger: Add UMC B-Cut and RTL8723 chip info definition.
/*
|    BIT 7   |     BIT6     |                BIT 5                | BIT 4              |       BIT 3     |  BIT 2   |  BIT 1   |   BIT 0    |
+--------+---------+---------------------- +------------ +----------- +------ +-----------------+
|Reserved | UMC BCut |Manufacturer(TSMC/UMC)  | TEST/NORMAL | 8723 Version | 8723?   | 1T2R?  | 88C/92C |
*/
/*
92D chip ver:
BIT8: IS 92D
BIT9: single phy
BIT10: C-cut
BIT11: D-cut

*/
typedef enum _VERSION_8192D{
	VERSION_TEST_CHIP_88C = 0x00,
	VERSION_TEST_CHIP_92C = 0x01,
	VERSION_NORMAL_TSMC_CHIP_88C = 0x10,
	VERSION_NORMAL_TSMC_CHIP_92C = 0x11,
	VERSION_NORMAL_TSMC_CHIP_92C_1T2R = 0x13,
	VERSION_NORMAL_UMC_CHIP_88C_A_CUT = 0x30,
	VERSION_NORMAL_UMC_CHIP_92C_A_CUT = 0x31,
	VERSION_NORMAL_UMC_CHIP_92C_1T2R_A_CUT = 0x33,
	VERSION_NORMA_UMC_CHIP_8723_1T1R_A_CUT = 0x34,
	VERSION_NORMA_UMC_CHIP_8723_1T1R_B_CUT = 0x3c,
	VERSION_NORMAL_UMC_CHIP_88C_B_CUT = 0x70,
	VERSION_NORMAL_UMC_CHIP_92C_B_CUT = 0x71,
	VERSION_NORMAL_UMC_CHIP_92C_1T2R_B_CUT = 0x73,
	VERSION_TEST_CHIP_92D_SINGLEPHY= 0x300,
	VERSION_TEST_CHIP_92D_DUALPHY = 0x100,
	VERSION_NORMAL_CHIP_92D_SINGLEPHY= 0x310,
	VERSION_NORMAL_CHIP_92D_DUALPHY = 0x110,
	VERSION_NORMAL_CHIP_92D_C_CUT_SINGLEPHY = 0x710,
	VERSION_NORMAL_CHIP_92D_C_CUT_DUALPHY = 0x510,
	VERSION_NORMAL_CHIP_92D_D_CUT_SINGLEPHY = 0xB10,
	VERSION_NORMAL_CHIP_92D_D_CUT_DUALPHY = 0x910,
}VERSION_8192D,*PVERSION_8192D;


//-------------------------------------------------------------------------
//	Channel Plan
//-------------------------------------------------------------------------
enum ChannelPlan{
	CHPL_FCC	= 0,
	CHPL_IC		= 1,
	CHPL_ETSI	= 2,
	CHPL_SPAIN	= 3,
	CHPL_FRANCE	= 4,
	CHPL_MKK	= 5,
	CHPL_MKK1	= 6,
	CHPL_ISRAEL	= 7,
	CHPL_TELEC	= 8,
	CHPL_GLOBAL	= 9,
	CHPL_WORLD	= 10,
};

typedef struct _TxPowerInfo{
	u8 CCKIndex[RF90_PATH_MAX][CHANNEL_GROUP_MAX];
	u8 HT40_1SIndex[RF90_PATH_MAX][CHANNEL_GROUP_MAX];
	u8 HT40_2SIndexDiff[RF90_PATH_MAX][CHANNEL_GROUP_MAX];
	u8 HT20IndexDiff[RF90_PATH_MAX][CHANNEL_GROUP_MAX];
	u8 OFDMIndexDiff[RF90_PATH_MAX][CHANNEL_GROUP_MAX];
	u8 HT40MaxOffset[RF90_PATH_MAX][CHANNEL_GROUP_MAX];
	u8 HT20MaxOffset[RF90_PATH_MAX][CHANNEL_GROUP_MAX];
	u8 TSSI_A[3];
	u8 TSSI_B[3];
}TxPowerInfo, *PTxPowerInfo;

#define EFUSE_REAL_CONTENT_LEN	1024
#define EFUSE_MAP_LEN				256
#define EFUSE_MAX_SECTION			32
#define EFUSE_MAX_SECTION_BASE	16
// <Roger_Notes> To prevent out of boundary programming case, leave 1byte and program full section
// 9bytes + 1byt + 5bytes and pre 1byte.
// For worst case:
// | 2byte|----8bytes----|1byte|--7bytes--| //92D
#define EFUSE_OOB_PROTECT_BYTES 	18 // PG data exclude header, dummy 7 bytes frome CP test and reserved 1byte.

#ifdef CONFIG_USB_HCI

//should be renamed and moved to another file
typedef	enum _INTERFACE_SELECT_8192DUSB{
	INTF_SEL0_USB 			= 0,		// USB
	INTF_SEL1_MINICARD	= 1,		// Minicard
	INTF_SEL2_EKB_PRO		= 2,		// Eee keyboard proprietary
	INTF_SEL3_PRO			= 3,		// Customized proprietary
} INTERFACE_SELECT_8192DUSB, *PINTERFACE_SELECT_8192DUSB;

typedef INTERFACE_SELECT_8192DUSB INTERFACE_SELECT_USB;

struct hal_data_8192du
{
	VERSION_8192D	VersionID;

	// add for 92D Phy mode/mac/Band mode 
	MACPHY_MODE_8192D	MacPhyMode92D;
	BAND_TYPE	CurrentBandType92D;	//0:2.4G, 1:5G
	BAND_TYPE	BandSet92D;
	BOOLEAN		bIsVS;

	BOOLEAN		bSupportRemoteWakeUp;

	u16	CustomerID;

	u16	FirmwareVersion;
	u16	FirmwareVersionRev;
	u16	FirmwareSubVersion;

	//current WIFI_PHY values
	u32	ReceiveConfig;
	WIRELESS_MODE	CurrentWirelessMode;
	HT_CHANNEL_WIDTH	CurrentChannelBW;
	u8	CurrentChannel;
	u8	nCur40MhzPrimeSC;// Control channel sub-carrier
	u16	BasicRateSet;

	INTERFACE_SELECT_8192DUSB	InterfaceSel;

	//rf_ctrl
	u8	rf_chip;
	u8	rf_type;
	u8	NumTotalRFPath;

	//
	// EEPROM setting.
	//
	u8	EEPROMVersion;
	u16	EEPROMVID;
	u16	EEPROMPID;
	u16	EEPROMSVID;
	u16	EEPROMSDID;
	u8	EEPROMCustomerID;
	u8	EEPROMSubCustomerID;	
	u8	EEPROMRegulatory;

	u8	EEPROMThermalMeter;

	u8	EEPROMC9;
	u8	EEPROMCC;

	u8	TxPwrLevelCck[RF90_PATH_MAX][CHANNEL_MAX_NUMBER_2G];
	u8	TxPwrLevelHT40_1S[RF90_PATH_MAX][CHANNEL_MAX_NUMBER];	// For HT 40MHZ pwr
	u8	TxPwrLevelHT40_2S[RF90_PATH_MAX][CHANNEL_MAX_NUMBER];	// For HT 40MHZ pwr	
	u8	TxPwrHt20Diff[RF90_PATH_MAX][CHANNEL_MAX_NUMBER];// HT 20<->40 Pwr diff
	u8	TxPwrLegacyHtDiff[RF90_PATH_MAX][CHANNEL_MAX_NUMBER];// For HT<->legacy pwr diff
	// For power group
	u8	PwrGroupHT20[RF90_PATH_MAX][CHANNEL_MAX_NUMBER];
	u8	PwrGroupHT40[RF90_PATH_MAX][CHANNEL_MAX_NUMBER];

	u8	LegacyHTTxPowerDiff;// Legacy to HT rate power diff

	u8	CrystalCap;	// CrystalCap.

#ifdef CONFIG_BT_COEXIST
	struct btcoexist_priv	bt_coexist;
#endif

	// Read/write are allow for following hardware information variables
	u8	framesync;
	u32	framesyncC34;
	u8	framesyncMonitor;
	u8	DefaultInitialGain[4];
	u8	pwrGroupCnt;
	u32	MCSTxPowerLevelOriginalOffset[MAX_PG_GROUP][16];
	u32	CCKTxPowerLevelOriginalOffset;

	u32	AntennaTxPath;					// Antenna path Tx
	u32	AntennaRxPath;					// Antenna path Rx
	u8	BluetoothCoexist;
	u8	ExternalPA;
	u8	InternalPA5G[2];	//pathA / pathB

	//u32	LedControlNum;
	//u32	LedControlMode;
	//u32	TxPowerTrackControl;
	u8	b1x1RecvCombine;	// for 1T1R receive combining

	u8	bCurrentTurboEDCA;
	u32	AcParam_BE; //Original parameter for BE, use for EDCA turbo.

	//vivi, for tx power tracking, 20080407
	//u16	TSSI_13dBm;
	//u32	Pwr_Track;
	// The current Tx Power Level
	u8	CurrentCckTxPwrIdx;
	u8	CurrentOfdm24GTxPwrIdx;

	BB_REGISTER_DEFINITION_T	PHYRegDef[4];	//Radio A/B/C/D

	BOOLEAN		bRFPathRxEnable[4];	// We support 4 RF path now.

	u32	RfRegChnlVal[2];

	u8	bCckHighPower;

	BOOLEAN		bPhyValueInitReady;

	// 20100428 Guangan: for 92D MAC1 init radio A
	BOOLEAN		bDuringMac1InitRadioA;
	BOOLEAN		bDuringMac0InitRadioB;

	BOOLEAN		bTXPowerDataReadFromEEPORM;

	BOOLEAN		bInSetPower;
	BOOLEAN		bCardDisableHWSM;

	//RDG enable
	BOOLEAN		bRDGEnable;

	BOOLEAN		bLoadIMRandIQKSettingFor2G;// True if IMR or IQK  have done  for 2.4G in scan progress
	BOOLEAN		bNeedIQK;

	BOOLEAN		bLCKInProgress;

	BOOLEAN		bEarlyModeEanble;

#if 1
	IQK_MATRIX_REGS_SETTING IQKMatrixRegSetting[IQK_Matrix_Settings_NUM];
#else
	//regc80��regc94��regc4c��regc88��regc9c��regc14��regca0��regc1c��regc78
	u4Byte				IQKMatrixReg[IQK_Matrix_REG_NUM];
	IQK_MATRIX_REGS_SETTING			   IQKMatrixRegSetting[IQK_Matrix_Settings_NUM];	// 1->2G,24->5G 20M channel,21->5G 40M channel.													
#endif

	//for host message to fw
	u8	LastHMEBoxNum;

	u8	fw_ractrl;
	u8	RegTxPause;
	// Beacon function related global variable.
	u32	RegBcnCtrlVal;
	u8	RegFwHwTxQCtrl;
	u8	RegReg542;

	struct dm_priv	dmpriv;

	u8	FwRsvdPageStartOffset; //2010.06.23. Added by tynli. Reserve page start offset except beacon in TxQ.

	// For 92C USB endpoint setting
	//

	u32	UsbBulkOutSize;

	int	RtBulkOutPipe[3];
	int	RtBulkInPipe;
	int	RtIntInPipe;

	// Add for dual MAC  0--Mac0 1--Mac1
	u32	interfaceIndex;

	u8	OutEpQueueSel;
	u8	OutEpNumber;

	u8	Queue2EPNum[8];//for out endpoint number mapping

#ifdef USB_TX_AGGREGATION_92C
	u8	UsbTxAggMode;
	u8	UsbTxAggDescNum;
#endif
#ifdef USB_RX_AGGREGATION_92C
	u16	HwRxPageSize;				// Hardware setting
	u32	MaxUsbRxAggBlock;

	USB_RX_AGG_MODE	UsbRxAggMode;
	u8	UsbRxAggBlockCount;			// USB Block count. Block size is 512-byte in hight speed and 64-byte in full speed
	u8	UsbRxAggBlockTimeout;
	u8	UsbRxAggPageCount;			// 8192C DMA page count
	u8	UsbRxAggPageTimeout;
#endif

	u16	EfuseUsedBytes;
	u8	RTSInitRate;	 // 2010.11.24.by tynli.
};

typedef struct hal_data_8192du HAL_DATA_TYPE, *PHAL_DATA_TYPE;
#endif

#define GET_HAL_DATA(__pAdapter)	((HAL_DATA_TYPE *)((__pAdapter)->HalData))
#define GET_RF_TYPE(priv)	(GET_HAL_DATA(priv)->rf_type)

int FirmwareDownload92D(IN PADAPTER Adapter);
void rtl8192d_ReadChipVersion(IN PADAPTER Adapter);
u8 GetEEPROMSize8192D(PADAPTER Adapter);
void rtl8192d_HalSetBrateCfg(PADAPTER Adapter, u8 *mBratesOS, u16 *pBrateCfg);
BOOLEAN PHY_CheckPowerOffFor8192D(PADAPTER Adapter);
VOID PHY_SetPowerOnFor8192D(PADAPTER Adapter);
void PHY_ConfigMacPhyMode92D(PADAPTER Adapter);
void rtl8192d_free_hal_data(_adapter * padapter);
void rtl8192d_set_hal_ops(struct hal_ops *pHalFunc);

#endif

