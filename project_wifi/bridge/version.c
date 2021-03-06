#include "version.h"
#include "message.h"

#ifndef __SVN_BRANCH
#define __SVN_BRANCH "release"
#endif

#ifndef __SVN_REV
#define __SVN_REV "0"
#endif

__attribute__ ((section (".ver_info")))
const version_info ver_info = {
		.dt = DEVICE_TYPE(BRIDGE_GROUP, BRIDGE_, 1),
		.hwver = 0x00050000,
		.hw_info = { "OCTO_PLC_BRIDGE" },
		.swver = FW_VERSION,
		.sw_info = { __SVN_BRANCH " @ " __SVN_REV ", " __DATE__ ", " __TIME__ },
};
