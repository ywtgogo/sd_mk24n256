#include "version.h"
#include "message.h"

#include "board.h"

#ifndef __SVN_BRANCH
#define __SVN_BRANCH "release"
#endif

#ifndef __SVN_REV
#define __SVN_REV "0"
#endif

#if (BOARD_NAME == HW_V02)
	#define HW_VER "02"
#elif (BOARD_NAME == HW_V03)
	#define HW_VER "03"
#else
	#error please config your BOARD_NAME
#endif

	
__attribute__ ((section (".ver_info")))

#ifdef WIFI_MODULE
const version_info ver_info = {
		.dt = DEVICE_TYPE(HOMEBOX_GROUP, HOMEBOX, 0x102),
		.hwver = 0x01010000,
		.hw_info = { "S-Box-WiFi " HW_VER},
		.swver = FW_VERSION,
		.sw_info = { __SVN_BRANCH " @ " __SVN_REV ", " __DATE__ ", " __TIME__ },
};
#else
const version_info ver_info = {
		.dt = DEVICE_TYPE(HOMEBOX_GROUP, HOMEBOX, 0x101),
		.hwver = 0x01010000,
		.hw_info = { "S-Box-2G " HW_VER},
		.swver = FW_VERSION,
		.sw_info = { __SVN_BRANCH " @ " __SVN_REV ", " __DATE__ ", " __TIME__ },
};
#endif





