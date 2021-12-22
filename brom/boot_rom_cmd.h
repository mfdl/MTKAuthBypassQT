#ifndef __BOOT_ROM_COMMAND__
#define __BOOT_ROM_COMMAND__

#include <sys/types.h>

#define BROM_CMD_I2C_INIT           0xB0
#define BROM_CMD_I2C_DEINIT         0xB1
#define BROM_CMD_I2C_WRITE8         0xB2
#define BROM_CMD_I2C_READ8          0xB3
#define BROM_CMD_I2C_SET_SPEED      0xB4

#define BROM_CMD_I2C_INIT_EX        0xB6
#define BROM_CMD_I2C_DEINIT_EX      0xB7
#define BROM_CMD_I2C_WRITE8_EX      0xB8
#define BROM_CMD_I2C_READ8_EX       0xB9
#define BROM_CMD_I2C_SET_SPEED_EX   0xBA

#define BROM_CMD_PWR_INIT           0xC4
#define BROM_CMD_PWR_DEINIT         0xC5
#define BROM_CMD_PWR_READ16         0xC6
#define BROM_CMD_PWR_WRITE16        0xC7

#define BROM_CMD_READ16             0xD0
#define BROM_CMD_READ32             0xD1
#define BROM_CMD_WRITE16            0xD2
#define BROM_CMD_WRITE16_NO_ECHO    0xD3
#define BROM_CMD_WRITE32            0xD4
#define BROM_CMD_JUMP_DA            0xD5		//				-> Jump to DA manually
#define BROM_CMD_JUMP_BL            0xD6		//				-> Jump to BL of eMMC/NFB/NOR automatically
#define BROM_CMD_SEND_DA            0xD7
#define BROM_CMD_GET_TARGET_CONFIG  0xD8
#define BROM_CMD_ENV_PREPARE        0xD9		//	EPP

//==============================================================================
// BOOTROM security relevant command	(From MT6276 @ 2009H2)
//==============================================================================
#define BROM_SCMD_SEND_CERT         0xE0
#define BROM_SCMD_GET_ME_ID         0xE1
#define BROM_SCMD_SEND_AUTH         0xE2
#define BROM_SCMD_SLA               0xE3

#define SEC_CFG_SBC_EN              0x00000001
#define SEC_CFG_SLA_EN              0x00000002
#define SEC_CFG_DAA_EN              0x00000004


#define BROM_CMD_GET_HW_SW_VER              0xFC // MT6575 for HW SW VERSION CHECK
#define BROM_CMD_GET_HW_CODE                0xFD

#define BROM_SCMD_GET_BROM_VER				0xFF

// MT6228 BootRom response from BROM_CMD_OLD_JUMP_NFB
#define BROM_NFB_DETECTION_MW01				0x4D573031	//MW01: Boot-loader successfully loaded!
#define BROM_NFB_DETECTION_ME01				0x4D453031	//ME01: Illegal checksum of Boot-loader!
#define BROM_NFB_DETECTION_ME02				0x4D453032	//ME02: Boot-loader header ID not found from neither replication!
#define BROM_NFB_DETECTION_ME03				0x4D453033	//ME03: Good block is not found!

//==============================================================================
// BOOTLOADER Command
//==============================================================================
#define BLDR_CMD_SEND_PARTITION_DATA    0x70
#define BLDR_CMD_JUMP_TO_PARTITION      0x71

#define BLDR_CMD_JUMP_MAUI					0xB7		//				-> Jump to MAUI automatically by BLDR
#define BLDR_CMD_READY						0xB8
#define	BLDR_CMD_GET_MAUI_FW_VER			0xBF
#define BLDR_CMD_GET_BLDR_VER				0xFE

// Boot-loader resposne from BLDR_CMD_READY (0xB8)
#define BLDR_STATUS_READY					0x00		// secure RO is found and ready to serve
#define BLDR_STATUS_SECURE_RO_NOT_FOUND		0x01		// secure RO is not found: first download? => dead end...
#define BLDR_STATUS_SUSBDL_NOT_SUPPORTED	0x02		// BL didn't enable Secure USB DL

//==============================================================================


typedef enum
{
   // OK
   B_OK = 0
   // Error
   ,E_ERROR = (__int32)0x1000
} GFH_STATUS;



#endif
