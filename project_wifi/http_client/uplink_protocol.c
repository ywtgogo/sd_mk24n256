#define PR_MODULE "http\1"

#include "uplink_protocol.h"
#include "stdbool.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "semphr.h"
#include "message.h"
#include "event_groups.h"

#include "ip_addr.h"
#include "ip.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "SPI_A7139.h"
#include "uplink_protocol.h"
#include "spiflash_config.h"
#include "fota.h"
#include "hb_protocol.h"
#include "homebox_bronze_info.h"
#include "aes256.h"
#include "sha1.h"
#include "json_string.h"
#include "rtc_fun.h"
#include "version.h"
#include "modem_socket.h"

#define RTC_TIME_SYNC_PERIOD	(1000*60*60*24*3)  //The unit is ms. the period is 3 day.


char unencrypted[1000];
char encrypt[1336];
unsigned char guid[20];

char sig_key[33];
char img_blk_url[128];
char img_down_url[128];

char json_buf[JSON_BUF_SIZE];
TimerHandle_t time_sync_timer_handle = NULL;
HOMEBOX_VERSION homebox_ver;

extern void time_sync_timer_callback(TimerHandle_t xTimer);
bool uplink_protocol_init()
{
	UplinkProtoTracePrint("uplink protocol init.........\r\n");
	if(get_version_string(hbb_info.pversion, homebox_ver.prot_ver, sizeof(homebox_ver.prot_ver)) == false)
		return false;
	UplinkProtoTracePrint("protocol version: %s \r\n",homebox_ver.prot_ver);
	if(get_version_string(ver_info.swver, homebox_ver.fw_ver, sizeof(homebox_ver.fw_ver)) == false)
		return false;

	UplinkProtoTracePrint("firmware version: %s \r\n", homebox_ver.fw_ver);
	if(get_version_string(ver_info.hwver, homebox_ver.hard_ver, sizeof(homebox_ver.fw_ver)) == false)
		return false;
	UplinkProtoTracePrint("hardware version: %s \r\n", homebox_ver.hard_ver);
	time_sync_timer_handle = xTimerCreate("time_sync_timer",     /* Text name. */
								   (RTC_TIME_SYNC_PERIOD), 			/* Timer period. */
								    pdTRUE,                   /* Enable auto reload. */
									0,                         /* ID is not used. */
									time_sync_timer_callback);    /* The callback function. */
	xTimerStart(time_sync_timer_handle, 0);
	return true;
}

void time_sync_timer_callback(TimerHandle_t xTimer)
{

	UPLINK_Q_MSG uplink_get_timestamp;
	UplinkProtoTracePrint("Get the time stamp........\r\n");
	uplink_get_timestamp.cmd = UPLINK_GET_SUB_DEVICES;
	uplink_get_timestamp.sub_dev_req.pos = 1;
	xQueueSend(uplink_q_handle, &uplink_get_timestamp, 0);
}

bool get_version_string(uint32_t version, char *str, int len)
{
	uint8_t temp[4];
	*(uint32 *)temp = 0xffffffff;
	if(len < 16){
		UplinkProtoTracePrint("Version buf is too small!\r\n");
		return false;
	}
	temp[0] &= version;
	temp[1] &= version>>8;
	temp[2] &= version>>16;
	temp[3] &= version>>24;
	sprintf(str, "%d.%d.%d.%d", temp[3],temp[2],temp[1],temp[0]);

	return true;
}

unsigned char *get_guid(unsigned char *input, unsigned int ilen, unsigned char output[33])
{
	memset(output, 0, sizeof(output));
	sha1_csum(input, ilen, guid);
	for (int i=0;i<16;i++)
		sprintf(output + 2*i, "%02X", guid[i]);
	return output;
}


#ifndef ENCROPT
char *json_send_event_format =
		"[{\"headers\":{\"pv\":\"%s\",\"enc\":\"%d\",\"sig\":\"%d\",\"sts\":\"%u%03u\",\"ev\":\"%s\"},"
		"\"body\":\"{\"msgid\":\"%s\",\"t\":\"%d\",\"sid\":\"%08X\",\"did\":\"%08X\","
		"\"dt\":\"%08X\",\"src\":\"%s\",\"p\":\"%d\",\"nt\":\"%s\",\"data\":\"%X\","
		"\"ts\":\"%u%03u\",\"m\":\"%s\",\"fwv\":\"%s\",\"hwv\":\"%s\",\"man\":\"%s\","
		"\"os\":\"%s\",\"ext\":{}}\"}]";
#else
char *json_send_event_format_h =
		"[{\"headers\":{\"pv\":\"%s\",\"enc\":\"%d\",\"sig\":\"%d\",\"sts\":\"%u%03u\",\"ev\":\"%s\"},\"body\":\"%s\"}]";
char *json_send_event_format_b =
		"{\"msgid\":\"%s\",\"t\":\"%d\",\"sid\":\"%08X%08X%08X\",\"did\":\"%08X%08X%08X\","
		"\"dt\":\"%08X\",\"src\":\"%s\",\"p\":\"%d\",\"nt\":\"%s\",\"data\":\"%s\","
		"\"ts\":\"%u%03u\",\"m\":\"%s\",\"fwv\":\"%s\",\"hwv\":\"%s\",\"man\":\"%s\","
		"\"os\":\"%s\",\"ext\":{}}";
#endif
void get_event_request_json(UPLINK_Q_MSG *q_ptr, unsigned char *aeskey, unsigned char *aesivv, char *aesvs)
{
	EVENT_REQ_STRUCT event;
	uint64_t timestamp;
	AES_STRUCT aes;

	aes.key = aeskey;
	aes.ivv = aesivv;
	aes.version = aesvs;
	memset(json_buf,0,sizeof(json_buf));
	/*get constant information*/
	event.header.enc = NOT_ENC;
	event.header.sig = NOT_SIG;
	get_current_systime(&timestamp);
	event.header.sts_s = (uint32_t)(timestamp / 1000);// get current timestamp
	event.header.sts_ms = (uint32_t)(timestamp % 1000);// get current timestamp
	event.body.slid = &q_ptr->event.slid;
	event.body.t = q_ptr->event.status;
	event.body.dt = q_ptr->event.device_type;
	event.body.p = q_ptr->event.p;
	event.body.data = q_ptr->event.data;
	//event.body.hwv = q_ptr->event.hwv;
	event.body.ts_s = (uint32_t)(q_ptr->event.ts / 1000);
	event.body.ts_ms = (uint32_t)(q_ptr->event.ts % 1000);

	if(get_version_string(q_ptr->event.fwv, event.body.fwv, sizeof(event.body.fwv)) == false)
		event.body.fwv[0] = '\0';
	if(get_version_string(q_ptr->event.hwv, event.body.hwv, sizeof(event.body.hwv)) == false)
		event.body.hwv[0] = '\0';
	modem_ipaddr_ntoa_r(&netif_default.ip_addr, event.body.src, sizeof(event.body.src));

	get_guid((unsigned char *)q_ptr,sizeof(UPLINK_Q_MSG), (unsigned char *)event.body.msgid);
#ifndef ENCROPT
	sprintf(json_buf, json_send_event_format,homebox_ver.prot_ver, event.header.enc, event.header.sig, event.header.sts_s,
			event.header.sts_ms, aes.version, event.body.msgid,event.body.t, event.body.sid,hbb_info.id,event.body.dt,
			event.body.src, event.body.p,hbb_info.nt, event.body.data, event.body.ts_s, event.body.ts_ms, event.body.m,
			homebox_ver.fw_ver, homebox_ver.hard_ver, hbb_info.man, hbb_info.os);

#else
	event.header.enc = AES256;
	memset(encrypt,0,sizeof(encrypt));
	memset(unencrypted,0,sizeof(unencrypted));
	sprintf(unencrypted,json_send_event_format_b, event.body.msgid, event.body.t, event.body.slid->idh, event.body.slid->idm,
			event.body.slid->id, hbb_info.config.longidh, hbb_info.config.longidm, hbb_info.config.id, event.body.dt,
			event.body.src, event.body.p,hbb_info.nt, event.body.data, event.body.ts_s, event.body.ts_ms, "18516524401",
			event.body.fwv, event.body.hwv, hbb_info.man, hbb_info.os);

	UplinkProtoTracePrint("The message will be send:\r\n%s %d\r\n",unencrypted, strlen(unencrypted));
	//TESTLOG("did = %X,sid = %X,ts = %u,sts = %u,t = %d\r\n",hbb_info.config.id,event.body.sid,event.body.ts_s,event.header.sts_s,event.body.t);
	AES_base64_encrypt(aes.key, aes.ivv, (unsigned char *)unencrypted, (unsigned char *)encrypt, strlen(unencrypted));

	sprintf(json_buf, json_send_event_format_h,homebox_ver.prot_ver, event.header.enc, event.header.sig,
			event.header.sts_s, event.header.sts_ms, aes.version, encrypt);
#endif
}

#ifndef ENCROPT
char *json_check_sensor_fota_format =
		"{\"headers\":{\"pv\":\"%s\",\"enc\":\"%d\",\"sig\":\"%d\",\"ev\":\"%s\"},"
		"\"body\":{\"msgid\":\"%s\",\"did\":\"%08X\",\"hwv\":\"%s\",\"fwv\":\"%s\",\"bid\":\"%u\"}}";
#else

char *json_check_sensor_fota_format =
		"{\"headers\":{\"pv\":\"%s\",\"enc\":\"%d\",\"sig\":\"%d\",\"ev\":\"%s\"},\"body\":\"%s\"}";
char *json_check_sensor_fota_format_body =
		"{\"msgid\":\"%s\",\"did\":\"%08X%08X%08X\",\"hwv\":\"%s\",\"fwv\":\"%s\",\"bid\":\"%u\"}";
#endif

void get_fw_update_request_json(UPLINK_Q_MSG *q_ptr, unsigned char *aeskey, unsigned char *aesivv, char *aesvs, LONG_ID96 *did)
{
	FW_UPDATE_REQ_STRUCT fw_req;
	AES_STRUCT aes;

	aes.key = aeskey;
	aes.ivv = aesivv;
	aes.version = aesvs;
	memset(json_buf,0,sizeof(json_buf));
	fw_req.header.enc = NOT_ENC;
	fw_req.header.sig = NOT_SIG;
	//if(q_ptr->fw_req.flag == EMERGENCY)
	fw_req.body.bid = q_ptr->fw_req.bid;
	//else fw_req.body.bid = 0;

	get_guid((unsigned char *)q_ptr,sizeof(UPLINK_Q_MSG), (unsigned char *)fw_req.body.msgid);

#ifndef ENCROPT
	sprintf(json_buf, json_check_sensor_fota_format, homebox_ver.prot_ver, fw_req.header.enc, fw_req.header.sig, hbb_info.key_version,
			fw_req.body.msgid,hbb_info.id, homebox_ver.hard_ver, homebox_ver.fw_ver, fw_req.body.bid);
#else
	fw_req.header.enc = AES256;
	memset(encrypt,0,sizeof(encrypt));
	memset(unencrypted,0,sizeof(unencrypted));
	sprintf(unencrypted, json_check_sensor_fota_format_body, fw_req.body.msgid, did->idh, did->idm, did->id, homebox_ver.hard_ver,
			homebox_ver.fw_ver, fw_req.body.bid);
	UplinkProtoTracePrint("The message will be send:\r\n%s\r\n",unencrypted);
	AES_base64_encrypt(aes.key, aes.ivv, (unsigned char *)unencrypted, (unsigned char *)encrypt, strlen(unencrypted));
#if 0
	UplinkProtoDbPrint("encrypt:%s\r\n",encrypt);
#endif
	sprintf(json_buf, json_check_sensor_fota_format, homebox_ver.prot_ver, fw_req.header.enc, fw_req.header.sig, aes.version,encrypt);
#endif
}

#ifndef ENCROPT
char *json_image_request_format =
		"{\"headers\":{\"pv\":\"%s\",\"enc\":\"%d\",\"sig\":\"%d\",\"ev\":\"%s\"},"
		"\"body\":{\"msgid\":\"%s\",\"did\":\"%08X\",\"rid\":\"%d\",\"sign\":\"%s\",\"url\":\"%s/fota_node_%u\"}}";
#else
char *json_image_request_format =
		"{\"headers\":{\"pv\":\"%s\",\"enc\":\"%d\",\"sig\":\"%d\",\"ev\":\"%s\"},\"body\":\"%s\"}";
char *json_image_request_format_body =
		"{\"msgid\":\"%s\",\"did\":\"%08X%08X%08X\",\"rid\":\"%u\",\"sign\":\"%s\",\"url\":\"%s/fota_node_%u\"}";
#endif

void get_sensor_image_request_json(UPLINK_Q_MSG *q_ptr, unsigned char *aeskey, unsigned char *aesivv, char *aesvs)
{
	FW_IMAGE_REQ_STRUCT fw_down;
	char in_sig_sring[100];
	unsigned char sha1[20];
	AES_STRUCT aes;

	aes.key = aeskey;
	aes.ivv = aesivv;
	aes.version = aesvs;
	memset(json_buf,0,sizeof(json_buf));
	fw_down.header.enc = NOT_ENC;
	fw_down.header.sig = SHA1;//SHA1;NOT_SIG
	get_guid((unsigned char *)q_ptr,sizeof(UPLINK_Q_MSG), (unsigned char *)fw_down.body.msgid);

	fw_down.body.rid = q_ptr->fw_down.rid;
	fw_down.body.url = q_ptr->fw_down.url;
	fw_down.body.img_blk_num = q_ptr->fw_down.img_blk_num;
	if(fw_down.header.sig == SHA1)
	{
		sprintf(in_sig_sring,"did=%08X%08X%08X&rid=%lu&key=%s", hbb_info.config.longidh, hbb_info.config.longidm,
				hbb_info.config.id, fw_down.body.rid, q_ptr->fw_down.key);
		sha1_csum((unsigned char *)in_sig_sring,strlen(in_sig_sring),sha1);
		hex_to_str((unsigned char *)fw_down.body.sign,sha1,sizeof(sha1));
	}else{
			fw_down.body.sign[0] = '0';
			fw_down.body.sign[1] = '\0';
	}

#ifndef ENCROPT
	sprintf(json_buf, json_image_request_format, homebox_ver.prot_ver, fw_down.header.enc, fw_down.header.sig, hbb_info.key_version,
			fw_down.body.msgid,hbb_info.id, fw_down.body.rid, fw_down.body.sign, fw_down.body.url ,fw_down.body.img_blk_num);
#else
	fw_down.header.enc = AES256;
	memset(encrypt,0,sizeof(encrypt));
	memset(unencrypted,0,sizeof(unencrypted));
	sprintf(unencrypted, json_image_request_format_body, fw_down.body.msgid, hbb_info.config.longidh, hbb_info.config.longidm,
			hbb_info.config.id, fw_down.body.rid, fw_down.body.sign, fw_down.body.url, fw_down.body.img_blk_num);
	UplinkProtoTracePrint("The message will be send:\r\n%s\r\n",unencrypted);
	AES_base64_encrypt(aes.key, aes.ivv, (unsigned char *)unencrypted, (unsigned char *)encrypt, strlen(unencrypted));
#if 0
	UplinkProtoDbPrint("encrypt:%s\r\n",encrypt);
#endif
	memset(json_buf,0,sizeof(json_buf));
	sprintf(json_buf, json_image_request_format, homebox_ver.prot_ver, fw_down.header.enc, fw_down.header.sig, aes.version, encrypt);
#endif
}
#ifndef ENCROPT
char *json_resp_update_result_format =
		"{\"headers\":{\"pv\":\"%s\",\"enc\":\"%d\",\"sig\":\"%d\",\"ev\":\"%s\"},"
		"\"body\":{\"msgid\":\"%s\",\"did\":\"%08X\",\"rid\":\"%d\",\"rc\":\"%d\"}}";
#else
char *json_resp_update_result_format =
		"{\"headers\":{\"pv\":\"%s\",\"enc\":\"%d\",\"sig\":\"%d\",\"ev\":\"%s\"},\"body\":\"%s\"}";
char *json_resp_update_result_format_body =
		"{\"msgid\":\"%s\",\"did\":\"%08X%08X%08X\",\"rid\":\"%d\",\"rc\":\"%d\"}";
char *json_resp_update_result_format_body_fl =
		"{\"msgid\":\"%s\",\"did\":\"%08X%08X%08X\",\"rid\":\"%d\",\"rc\":\"%d\",\"fl\":\"%s\"}";
#endif
static bool get_fail_list(LONG_ID96 fl[FAIL_LIST_MAX_NUM], int num, char *out_str)
{
	if(num > FAIL_LIST_MAX_NUM){
		UplinkProtoDbPrint("fail sensor num is more than FAIL_LIST_MAX_NUM!\r\n");
		return false;
	}
	if(num <= 0)
		return false;
	for(int i=0;i<num-1;i++)
		sprintf(out_str+i*26,"%08X%08X%08X##",fl[i].idh, fl[i].idm, fl[i].id);

	sprintf(out_str+(num-1)*26,"%08X%08X%08X",fl[num-1].idh, fl[num-1].idm, fl[num-1].id);
	UplinkProtoTracePrint("fl: %s\r\n",out_str);
	return true;
}
void get_resp_update_result_json(UPLINK_Q_MSG *q_ptr, unsigned char *aeskey, unsigned char *aesivv, char *aesvs)
{
	FW_UPDATE_RESULT_REQ_STRUCT fw_result;
	AES_STRUCT aes;

	aes.key = aeskey;
	aes.ivv = aesivv;
	aes.version = aesvs;
	memset(json_buf,0,sizeof(json_buf));
	fw_result.header.enc = NOT_ENC;
	fw_result.header.sig = NOT_SIG;
	get_guid((unsigned char *)q_ptr, sizeof(UPLINK_Q_MSG), (unsigned char *)fw_result.body.msgid);
	//strcpy(fw_result.body.msgid,"12333423232223223232322334232222");
	fw_result.body.rid = q_ptr->fw_upd_result.rid;
	fw_result.body.rc = q_ptr->fw_upd_result.rc;
#ifndef ENCROPT
	sprintf(json_buf, json_resp_update_result_format, homebox_ver.prot_ver, fw_result.header.enc, fw_result.header.sig,
			hbb_info.key_version, fw_result.body.msgid,hbb_info.id, fw_result.body.rid, fw_result.body.rc);
#else
	fw_result.header.enc = AES256;
	memset(encrypt,0,sizeof(encrypt));
	memset(unencrypted,0,sizeof(unencrypted));
	if(fw_result.body.rc == PART_OF_DEV_FAIL){
		char flstr[FAIL_LIST_MAX_NUM*30];
		memset(flstr,0,sizeof(flstr));
		if(get_fail_list(q_ptr->fw_upd_result.fl, q_ptr->fw_upd_result.flnum, flstr) == true)
			sprintf(unencrypted, json_resp_update_result_format_body_fl, fw_result.body.msgid, hbb_info.config.longidh,
					hbb_info.config.longidm, hbb_info.config.id, fw_result.body.rid, fw_result.body.rc, flstr);
		else
			sprintf(unencrypted, json_resp_update_result_format_body_fl, fw_result.body.msgid, hbb_info.config.longidh,
					hbb_info.config.longidm, hbb_info.config.id, fw_result.body.rid, fw_result.body.rc, "0");
	}
	else{
		sprintf(unencrypted, json_resp_update_result_format_body, fw_result.body.msgid, hbb_info.config.longidh,
				hbb_info.config.longidm, hbb_info.config.id, fw_result.body.rid, fw_result.body.rc);
	}
	//else PART_OF_DEV_FAIL
	UplinkProtoTracePrint("The message will be send:\r\n%s\r\n",unencrypted);
	AES_base64_encrypt(aes.key, aes.ivv, (unsigned char *)unencrypted, (unsigned char *)encrypt, strlen(unencrypted));
#if 0
	UplinkProtoDbPrint("encrypt:%s\r\n",encrypt);
#endif
	sprintf(json_buf, json_resp_update_result_format, homebox_ver.prot_ver, fw_result.header.enc, fw_result.header.sig,
			aes.version, encrypt);
#endif
}

#ifndef ENCROPT
char *json_sub_dev_request_format =
		"{\"headers\":{\"pv\":\"%s\",\"enc\":\"%d\",\"sig\":\"%d\",\"ev\":\"%s\"},"
		"\"body\":{\"msgid\":\"%s\",\"did\":\"%08X\",\"dt\":\"%08X\",\"pos\":%d}}";
#else
char *json_sub_dev_request_format =
		"{\"headers\":{\"pv\":\"%s\",\"enc\":\"%d\",\"sig\":\"%d\",\"ev\":\"%s\"},\"body\":\"%s\"}";
char *json_sub_dev_request_format_body =
		"{\"msgid\":\"%s\",\"did\":\"%08X%08X%08X\",\"dt\":\"%08X\",\"pos\":\"%d\"}";
#endif
void get_sub_devices_request_json(UPLINK_Q_MSG *q_ptr, unsigned char *aeskey, unsigned char *aesivv, char *aesvs)
{
	GET_SUB_DEV_REQ_STRUCT dev;
	AES_STRUCT aes;

	aes.key = aeskey;
	aes.ivv = aesivv;
	aes.version = aesvs;
	memset(json_buf,0,sizeof(json_buf));
	dev.header.enc = NOT_ENC;
	dev.header.sig = NOT_SIG;
	get_guid((unsigned char *)q_ptr,sizeof(UPLINK_Q_MSG), (unsigned char *)dev.body.msgid);
	dev.body.pos = q_ptr->sub_dev_req.pos;
#ifndef ENCROPT
	sprintf(json_buf, json_sub_dev_request_format, homebox_ver.prot_ver, dev.header.enc, dev.header.sig, hbb_info.key_version,
			dev.body.msgid, hbb_info.id, hbb_info.dt, dev.body.pos);
#else
	dev.header.enc = AES256;
	memset(encrypt,0,sizeof(encrypt));
	memset(unencrypted,0,sizeof(unencrypted));
	sprintf(unencrypted, json_sub_dev_request_format_body, dev.body.msgid, hbb_info.config.longidh, hbb_info.config.longidm,
			hbb_info.config.id, ver_info.dt, dev.body.pos);
	UplinkProtoTracePrint("The message will be send:\r\n%s\r\n",unencrypted);
	AES_base64_encrypt(aes.key, aes.ivv, (unsigned char *)unencrypted, (unsigned char *)encrypt, strlen(unencrypted));
#if 0
	UplinkProtoDbPrint("encrypt:%s\r\n",encrypt);
#endif
	sprintf(json_buf, json_sub_dev_request_format, homebox_ver.prot_ver, dev.header.enc, dev.header.sig, aes.version,
			encrypt);
#endif
}

int8_t check_fota_resp_json_parse(char *indata, FW_UPDATE_RESP_STRUCT *out_data)
{
	char *data_ptr = indata;
	char *str_start;
	uint32_t version[4];
	str_start = (char *)strstr(data_ptr,POV);
	if(str_start != NULL)
	{
		str_start += strlen(POV);
		sscanf(str_start,"%7s",out_data->header.pv);
		UplinkProtoDbPrint("pv: %s\r\n",out_data->header.pv);
	}else
		return JSON_PARSE_ERR;

	str_start = (char *)strstr(data_ptr,SIG);
	if(str_start != NULL)
	{
		str_start += strlen(SIG);
		sscanf(str_start,"%d",&out_data->header.sig);
		UplinkProtoDbPrint("sig: %d\r\n",out_data->header.sig);
	}else
		return JSON_PARSE_ERR;

	str_start = (char *)strstr(data_ptr,ENC);
	if(str_start != NULL)
	{
		str_start += strlen(ENC);
		sscanf(str_start,"%d",&out_data->header.enc);
		UplinkProtoDbPrint("enc: %d\r\n",out_data->header.enc);
	}else
		return JSON_PARSE_ERR;

	if(out_data->header.enc == AES256)//if data have be encrypt,we should decode
	{
		int body_length;
		str_start = strstr(data_ptr,BODY);
		if(str_start == NULL)
			return JSON_PARSE_ERR;
		str_start += strlen(BODY);
		body_length = strstr(str_start,END) - str_start;

		str_start[body_length] = '\0';
#if 0
		UplinkProtoDbPrint("encrypted:%s\r\n",str_start);
#endif
		int len = AES_base64_decrypt(hbb_info.config.aeskey, hbb_info.config.aesivv, (unsigned char *)str_start, (unsigned char *)unencrypted, strlen(str_start));
		data_ptr = unencrypted;
		data_ptr[len] = '\0';
		UplinkProtoTracePrint("Received message:\r\n%s\r\n",data_ptr);
	}
	str_start = (char *)strstr(data_ptr,RC);
	if(str_start != NULL)
	{
		str_start += strlen(RC);
		sscanf(str_start,"%lu",&out_data->body.rc);
		UplinkProtoDbPrint("rc: %lu\r\n",out_data->body.rc);
	}else
		return JSON_PARSE_ERR;

	str_start = (char *)strstr(data_ptr,MSGID);
	if(str_start != NULL)
	{
		str_start += strlen(MSGID);
#ifdef __GNUC__
		sscanf(str_start,"%[^\"]",out_data->body.msgid);

#else
		char * start_ptr = str_start;
        while(*start_ptr != '\"')
            start_ptr++;
        strncpy(out_data->body.msgid,str_start,start_ptr-str_start);
        out_data->body.msgid[start_ptr-str_start] = '\0';
#endif
		UplinkProtoDbPrint("msgid: %s\r\n",out_data->body.msgid);
	}else
		return JSON_PARSE_ERR;

	str_start = (char *)strstr(data_ptr,RID);
	if(str_start != NULL)
	{
		str_start += strlen(RID);
		sscanf(str_start,"%lu",&out_data->body.rid);
		UplinkProtoDbPrint("rid: %lu\r\n",out_data->body.rid);
	}else
		return JSON_PARSE_ERR;

	str_start = (char *)strstr(data_ptr,P);
	if(str_start != NULL)
	{
		str_start += strlen(P);
		sscanf(str_start,"%lu",&out_data->body.p);
		UplinkProtoDbPrint("p: %lu\r\n",out_data->body.p);
	}else
		return JSON_PARSE_ERR;

	str_start = (char *)strstr(data_ptr,DT);
	if(str_start != NULL)
	{
		str_start += strlen(DT);
		sscanf(str_start,"%x",&out_data->body.dt);
		UplinkProtoDbPrint("dt: %x\r\n",out_data->body.dt);
	}else
		return JSON_PARSE_ERR;

	str_start = (char *)strstr(data_ptr,HDV);
	if(str_start != NULL)
	{
		str_start += strlen(HDV);
		//sscanf(str_start,"%[^\"]",&out_data->body.hdv);

	    sscanf(str_start, "%lu.%lu.%lu.%lu", &version[0], &version[1],&version[2],&version[3]);
	    VER_TO_UINT32(out_data->body.hdv, version[0], version[1], version[2], version[3]);
//	    printf("V0:%lu V1:%lu V2:%lu V3:%lu \r\n",version[0], version[1], version[2], version[3]);
//	    printf("hdv: 0X%08X\r\n",out_data->body.hdv);
	}else
		return JSON_PARSE_ERR;

	str_start = (char *)strstr(data_ptr,FWPV);
	if(str_start != NULL)
	{
		str_start += strlen(FWPV);
	    sscanf(str_start, "%lu.%lu.%lu.%lu", &version[0], &version[1],&version[2],&version[3]);
	    VER_TO_UINT32(out_data->body.fwpv, version[0], version[1], version[2], version[3]);
	}else
		return JSON_PARSE_ERR;

	str_start = (char *)strstr(data_ptr,FWUV);
	if(str_start != NULL)
	{
		str_start += strlen(FWUV);
		//sscanf(str_start,"%[^\"]",out_data->body.fwuv);
	    sscanf(str_start, "%lu.%lu.%lu.%lu", &version[0], &version[1],&version[2],&version[3]);
	    VER_TO_UINT32(out_data->body.fwuv, version[0], version[1], version[2], version[3]);
	}else
		return JSON_PARSE_ERR;

	str_start = (char *)strstr(data_ptr,IFM);
	if(str_start != NULL)
	{
		str_start += strlen(IFM);
		sscanf(str_start,"%lu",&out_data->body.ifm);
	}else
		return JSON_PARSE_ERR;

	str_start = (char *)strstr(data_ptr,T);
	if(str_start != NULL)
	{
		str_start += strlen(T);
		sscanf(str_start,"%lu",&out_data->body.t);
	}else
		return JSON_PARSE_ERR;

	if((out_data->body.t == IMAGE)||(out_data->body.t == MODULE))
	{
		if(out_data->body.t == MODULE)
		{
			str_start = (char *)strstr(data_ptr,MID);
			if(str_start != NULL)
			{
				str_start += strlen(MID);
				sscanf(str_start,"%lu",&out_data->body.img.mid);
				UplinkProtoDbPrint("mid: %lu\r\n",out_data->body.img.mid);
			}else
				return JSON_PARSE_ERR;
		}

		str_start = (char *)strstr(data_ptr,FWBLKS);
		if(str_start != NULL)
		{
			str_start += strlen(FWBLKS);
			sscanf(str_start,"%lu",&out_data->body.img.fwblks);
			UplinkProtoDbPrint("fwblks: %lu\r\n",out_data->body.img.fwblks);
		}else
			return JSON_PARSE_ERR;

		str_start = (char *)strstr(data_ptr,FWS);
		if(str_start != NULL)
		{
			str_start += strlen(FWS);
			sscanf(str_start,"%lu",&out_data->body.img.fws);
			UplinkProtoDbPrint("fws: %lu\r\n",out_data->body.img.fws);
		}else
			return JSON_PARSE_ERR;

		str_start = (char *)strstr(data_ptr,FWURI);
		if(str_start != NULL)
		{
			str_start += strlen(FWURI);
			if(sscanf(str_start,"%[^##]",img_down_url) != 1)
				return JSON_PARSE_ERR;
			UplinkProtoDbPrint("img_down_url: %s\r\n",img_down_url);
			str_start = (char *)strstr(str_start,"##");

			str_start += strlen("##");
			if(sscanf(str_start,"%[^##]",img_blk_url) != 1)
				return JSON_PARSE_ERR;
			out_data->body.img.url = img_blk_url;
			UplinkProtoDbPrint("img_blk_url: %s\r\n",out_data->body.img.url);
			str_start = (char *)strstr(str_start,"##");

			str_start += strlen("##");
			if(sscanf(str_start,"%32s",sig_key) != 1)
				return JSON_PARSE_ERR;
			out_data->body.img.key = sig_key;
			UplinkProtoDbPrint("key: %s\r\n",out_data->body.img.key);
		}else
			return JSON_PARSE_ERR;
	}
	else if(out_data->body.t == PARAMETER)
	{
		str_start = (char *)strstr(data_ptr,PUI);
		if(str_start != NULL)
		{
			char *pui_end_ptr;
			str_start += strlen(PUI);
			pui_end_ptr = strchr(str_start,'}');
			return get_pui(str_start, pui_end_ptr, out_data);

		}else
			return JSON_PARSE_ERR;
	}
	else
		return JSON_PARSE_ERR;

	return JSON_PARSE_OK;
}

int8_t get_pui(const char *start_ptr, const char *end_ptr, FW_UPDATE_RESP_STRUCT *update_resp)
{
	char *ptr;
	update_resp->body.parameter.para_type = 0;
	if(end_ptr <= start_ptr)
	{
		UplinkProtoDbPrint("end_ptr <= start_ptr!!!!\r\n");
		return JSON_PARSE_ERR;
	}

	if((ptr = (char *)strstr(start_ptr,URLFOTA)) != NULL)
	{
		update_resp->body.parameter.para_type = PARA_URL_FOTA;
		ptr +=  strlen(URLFOTA);
	}
	else if((ptr = (char *)strstr(start_ptr,URLEVENT)) != NULL)
	{
		update_resp->body.parameter.para_type = PARA_URL_EVENT;
		ptr +=  strlen(URLEVENT);
	}
	else if((ptr = (char *)strstr(start_ptr,URLALRET)) != NULL)
	{
		update_resp->body.parameter.para_type = PARA_URL_ALERT;
		ptr +=  strlen(URLALRET);
	}
	else if((ptr = (char *)strstr(start_ptr,URLCTRL)) != NULL)
	{
		update_resp->body.parameter.para_type = PARA_URL_CTRL;
		ptr +=  strlen(URLCTRL);
	}

	if(update_resp->body.parameter.para_type != 0)
	{
		if(sscanf(ptr,"%[^\"]",update_resp->body.parameter.url) != 1)
			return JSON_PARSE_ERR;
		return JSON_PARSE_OK;
	}

	if((ptr = (char *)strstr(start_ptr,HBI)) != NULL)
	{
		//uint32_t hb_time; //time unit is s
		ptr +=  strlen(HBI);
		if(sscanf(ptr,"%lu",&update_resp->body.parameter.heartbeat) != 1)
			return JSON_PARSE_ERR;
		update_resp->body.parameter.para_type = PARA_HEART_BEAT;
		return JSON_PARSE_OK;
	}

	if((ptr = (char *)strstr(start_ptr,FI)) != NULL)
	{
		//uint32_t fi_time; //time unit is s
		ptr +=  strlen(FI);
		if(sscanf(ptr,"%lu",&update_resp->body.parameter.fota_interval) != 1)
			return JSON_PARSE_ERR;
		update_resp->body.parameter.para_type = PARA_FOTA_INTERVAL;
		return JSON_PARSE_OK;
	}

	if((ptr = (char *)strstr(start_ptr,SOSPHONE)) != NULL)
	{
		//uint32_t fi_time; //time unit is s
		ptr +=  strlen(SOSPHONE);
		if(sscanf(ptr,"%[^\"]",update_resp->body.parameter.sos) != 1)
			return JSON_PARSE_ERR;
		update_resp->body.parameter.para_type = PARA_SOS_PHONE;
		return JSON_PARSE_OK;
	}

	if((ptr = (char *)strstr(start_ptr,AUTOANSWER)) != NULL)
	{
		//uint32_t fi_time; //time unit is s
		ptr +=  strlen(AUTOANSWER);
		if(sscanf(ptr,"%lu",&update_resp->body.parameter.auto_answer) != 1)
			return JSON_PARSE_ERR;
		update_resp->body.parameter.para_type = PARA_AUTO_ANSWER;
		return JSON_PARSE_OK;
	}

	if((ptr = (char *)strstr(start_ptr,AESKEY)) != NULL)
	{
		update_resp->body.parameter.para_type = PARA_AES_KEY;
		ptr +=  strlen(AESKEY);
		if(sscanf(ptr,"%[^\"]",update_resp->body.parameter.aes.newkey) != 1)//the key shouldn't include '"'
			return JSON_PARSE_ERR;
		if(strlen((char *)update_resp->body.parameter.aes.newkey) != 32)
		{
			UplinkProtoDbPrint("aeskey length error!\r\n");
			return JSON_PARSE_ERR;
		}

		if((ptr = (char *)strstr(start_ptr,AESVCT)) != NULL)
		{
			ptr +=  strlen(AESVCT);
			if(sscanf(ptr,"%[^\"]",update_resp->body.parameter.aes.newVector) != 1)//the key shouldn't include '"'
				return JSON_PARSE_ERR;
			if(strlen((char *)update_resp->body.parameter.aes.newVector) != 16)
			{
				UplinkProtoDbPrint("newVector length error!\r\n");
				return JSON_PARSE_ERR;
			}
		}

		if((ptr = (char *)strstr(start_ptr,ENCVR)) != NULL)
		{
			ptr +=  strlen(ENCVR);
			if(sscanf(ptr,"%[^\"]",update_resp->body.parameter.aes.newEncvr) != 1)//the key shouldn't include '"'
				return JSON_PARSE_ERR;
		}
	}
	else
		return JSON_PARSE_ERR;

	return JSON_PARSE_OK;
}

int parse_sids(const char *start_ptr, const char *end_ptr, DEVICE_LIST *sdl)
{

	int sidnum = 0;
	char tempstr[9];
	uint32_t *lid_ptr;
	if(end_ptr == start_ptr)//sid context is null
	{
		sdl->num = sidnum;
		return JSON_PARSE_OK;
	}
	if(end_ptr - start_ptr <= 6)//if less than 1 sidn
		return JSON_PARSE_ERR;
	for(int i=0;i<(MAX_SENSOR_COUNT+1);i++)
	{
		lid_ptr = &sdl->idn[i].lid.idh;
		for(int j=0;j<=2;j++)
		{
			strncpy(tempstr,start_ptr,8);
			tempstr[8] = '\0';
			if(sscanf(tempstr, "%x",&lid_ptr[j]) != 1)
				return JSON_PARSE_ERR;
			start_ptr += 8;
		}
		start_ptr += 1;
		strncpy(tempstr,start_ptr,8);
		tempstr[8] = '\0';
		if(sscanf(tempstr, "%x",&lid_ptr[3]) != 1)
			return JSON_PARSE_ERR;
		sidnum++;
		start_ptr = strchr(start_ptr,',');
		if((start_ptr == NULL) || (start_ptr >= end_ptr))
			break;
		start_ptr +=1;
	}

	sdl->num = sidnum;
	return JSON_PARSE_OK;
}

int8_t get_sub_devices_resp_json_parse(char *indata, GET_SUB_DEV_RESP_STRUCT *out_data)
{
	char *data_ptr = indata;
	char *str_start;
	//u16_t version[4];
	str_start = (char *)strstr(data_ptr,POV);
	if(str_start != NULL)
	{
		str_start += strlen(POV);
		sscanf(str_start,"%7s",out_data->header.pv);
		UplinkProtoDbPrint("pv: %s\r\n",out_data->header.pv);
	}else
		return JSON_PARSE_ERR;

	str_start = (char *)strstr(data_ptr,SIG);
	if(str_start != NULL)
	{
		str_start += strlen(SIG);
		sscanf(str_start,"%d",&out_data->header.sig);
		UplinkProtoDbPrint("sig: %d\r\n",out_data->header.sig);
	}else
		return JSON_PARSE_ERR;

	str_start = (char *)strstr(data_ptr,ENC);
	if(str_start != NULL)
	{
		str_start += strlen(ENC);
		sscanf(str_start,"%d",&out_data->header.enc);
		UplinkProtoDbPrint("enc: %d\r\n",out_data->header.enc);
	}else
		return JSON_PARSE_ERR;

	if(out_data->header.enc == AES256)//if data have be encrypt,we should decode
	{
		int body_length;
		str_start = strstr(data_ptr,BODY);
		if(str_start == NULL)
			return JSON_PARSE_ERR;
		str_start += strlen(BODY);
		body_length = strstr(str_start,END) - str_start;
		str_start[body_length] = '\0';
#if 0
		UplinkProtoDbPrint("encrypted:%s\r\n",str_start);
#endif
		int len = AES_base64_decrypt(hbb_info.config.aeskey, hbb_info.config.aesivv, (unsigned char *)str_start, (unsigned char *)unencrypted, strlen(str_start));
		data_ptr = unencrypted;
		data_ptr[len] = '\0';
		UplinkProtoTracePrint("Received message:\r\n%s\r\n",data_ptr);
	}

	str_start = (char *)strstr(data_ptr,CTS);
	if(str_start != NULL)
	{
		str_start += strlen(CTS);
		sscanf(str_start,"%10lu",&out_data->body.cts);
		UplinkProtoDbPrint("cts: %lu\r\n",out_data->body.cts);
	}else
		return JSON_PARSE_ERR;

	str_start = (char *)strstr(data_ptr,RC);
	if(str_start != NULL)
	{
		str_start += strlen(RC);
		sscanf(str_start,"%lu",&out_data->body.rc);
		UplinkProtoDbPrint("rc: %lu\r\n",out_data->body.rc);
	}else
		return JSON_PARSE_ERR;
	str_start = (char *)strstr(data_ptr,SIDS);
	if(str_start != NULL)
	{
		char *sid_end_ptr;
		str_start += strlen(SIDS);
		sid_end_ptr = strchr(str_start,'\"');
		if(parse_sids(str_start, sid_end_ptr, &out_data->body.sids) != JSON_PARSE_OK)
			return JSON_PARSE_ERR;
	}

	return JSON_PARSE_OK;
}
