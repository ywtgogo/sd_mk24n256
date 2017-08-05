#ifndef _HBB_CONFIG_H_
#define _HBB_CONFIG_H_


#ifdef BRIDGE
#define BRIDGE_CONFIG											\
			.id = BRIDGE_ID,									\
			.master = 0,										\
			.key_version = "v1",								\
			.aesivv = "SandlacusData#@1",						\
			.aeskey = "SandlacusData#@1SandlacusData#@1",		\
			.aesivv_old = "SandlacusData#@1",					\
			.aeskey_old = "SandlacusData#@1SandlacusData#@1",
#else
#if 1
#define HBB_DEFCONFIG 											\
			.id = HBB_ID,										\
			.longidh = HBB_LONG_IDH,								\
			.longidm = HBB_LONG_IDM,								\
			.heartbeat_interval = SENSOR_POLLING_INTERVAL,		\
			.fota_check_interval = 86400000,					\
			.key_version = "v1",								\
			.aesivv = "SandlacusData#@1",						\
			.aeskey = "SandlacusData#@1SandlacusData#@1",		\
			.aesivv_old = "SandlacusData#@1",					\
			.aeskey_old = "SandlacusData#@1SandlacusData#@1",	\
			.alerturl = "http://test1.sandlacus.com:15560/alert",	\
			.eventurl = "http://test1.sandlacus.com:15560/event",	\
			.controlurl = "http://test1.sandlacus.com:15560/api/dm/v1/device/list", \
			.fotaurl = "http://test1.sandlacus.com:15560/fota",\
			.mqtt_address = "test1.sandlacus.com",				\
			.mqtt_port = 15590,
#else
#define HBB_DEFCONFIG 											\
			.id = HBB_ID,										\
			.longidh = HBB_LONG_IDH,								\
			.longidm = HBB_LONG_IDM,								\
			.heartbeat_interval = SENSOR_POLLING_INTERVAL,		\
			.fota_check_interval = 86400000,					\
			.key_version = "v1",								\
			.aesivv = "SandlacusData#@1",						\
			.aeskey = "SandlacusData#@1SandlacusData#@1",		\
			.aesivv_old = "SandlacusData#@1",					\
			.aeskey_old = "SandlacusData#@1SandlacusData#@1",	\
			.alerturl = "http://officelinux.vicp.net:29998/",	\
			.eventurl = "http://officelinux.vicp.net:29999/",	\
			.controlurl = "http://officelinux.vicp.net:28081/api/dm/v1/device/list", \
			.fotaurl = "http://officelinux.vicp.net:28080/fota",\
			.mqtt_address = "officelinux.vicp.net",				\
			.mqtt_port = 18830,
#endif
#endif
#define PPP_GPRS_APN_DEFCONFIG 									\
		.gprs_apn = "\"sl3gnet\"\r",								\
		.ppp_pap_user = "sl_admin",								\
		.ppp_pap_passwd = "sl_passwd",							\
		.sos_call = "112\r",									\
		.sms_max_number_me = 200,								\
		.sms_curr_number_me = 184,

#endif

#ifndef WIFI_MODULE
#define NET_TYPE 		"2G"
#else
#define NET_TYPE		"WIFI"
#endif

#define MAX_SENSOR_COUNT 32
#define MAX_BRIDGE_COUNT 1
