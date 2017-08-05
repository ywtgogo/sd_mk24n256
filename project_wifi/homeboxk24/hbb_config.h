#ifndef _HBB_CONFIG_H_
#define _HBB_CONFIG_H_

#define DEVICE_PLATFORM_TEST 1
#define DEVICE_PLATFORM_CLOUD 2


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

#if (DEVICE_PLATFORM == DEVICE_PLATFORM_CLOUD)
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
			.alerturl = "https://device.sandlacus.com/alert",	\
			.eventurl = "https://device.sandlacus.com/event",	\
			.controlurl = "https://device.sandlacus.com/api/dm/v1/device/list", \
			.fotaurl = "https://device.sandlacus.com/fota",\
			.mqtt_address = "device.sandlacus.com",				\
			.mqtt_port = 18830,									\
			.freq = 0xdd00,

#elif (DEVICE_PLATFORM == DEVICE_PLATFORM_TEST)

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
			.mqtt_port = 15590,									\
			.freq = 0xdd00,

#else
	#error please config your DEVICE_PLATFORM
#endif

#endif

#define PPP_GPRS_APN_DEFCONFIG 									\
		.gprs_apn = "\"sl3gnet\"\r",								\
		.ppp_pap_user = "sl_admin",								\
		.ppp_pap_passwd = "sl_passwd",							\
		.sos_call = "112\r",									\
		.incoming_call_answer_timeout = 27,	                    \
		.sms_max_number_me = 50,								\
		.sms_curr_number_me = 30,

#endif

#ifndef WIFI_MODULE
#define NET_TYPE 		"2G"
#else
#define NET_TYPE		"WIFI"
#endif

#define MAX_SENSOR_COUNT 32
#define MAX_BRIDGE_COUNT 1

