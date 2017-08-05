#include "message.h"
#include "uart_ppp.h"
#include "version.h"
#include "uplink_protocol.h"
#include "hb_protocol.h"
#include "hbb_config.h"

#ifdef BRIDGE
#else
hbb_config hbb_info_config = {
		HBB_DEFCONFIG
};
#endif

__attribute__((aligned(2048)))
ppp_gprs_apn_pap_info_conf ppp_gprs_apn_pap_info_config = {
		PPP_GPRS_APN_DEFCONFIG
};
