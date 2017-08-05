#define PR_MODULE "http\1"

#include "lwip/opt.h"

#if LWIP_NETCONN
#include <stdio.h>
#include <string.h>
#include "lwip/netif.h"
#include "lwip/sys.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/tcpip.h"

#ifndef CPU_MK24FN256VDC12
#include "netif/etharp.h"
#include "ethernetif.h"
#endif
#include "board.h"
#include "rtc_fun.h"
#include "aes256.h"
#include "sha1.h"
#include "version.h"
#include "hb_protocol.h"
#include "log_task.h"

#ifndef HTTPD_DEBUG
#define HTTPD_DEBUG LWIP_DBG_ON
#endif
#ifndef HTTPD_STACKSIZE
#define HTTPD_STACKSIZE 3000
#endif
#ifndef HTTPD_PRIORITY
#define HTTPD_PRIORITY 3
#endif

#include "fsl_device_registers.h"
#include "pin_mux.h"
#include "clock_config.h"

#include "http_client.h"

#include "queue.h"
#include "uplink_protocol.h"
#include "homebox_bronze_info.h"
#include "json_string.h"
#include "spiflash_config.h"
#include "fota.h"
#include "uart_ppp.h"
#include "log_task.h"
#include "mqtt_task.h"
#ifdef WIFI_MODULE
#include "wifi_module.h"
#endif

#define TESTLOG		printf

/*******************************************************************************
 * parameter verify
 ******************************************************************************/

#define HEART_BEAT_MIN 			(30) //the unit is minute
#define HEART_BEAT_MAX 			(90)	//the unit is minute
#define HEART_BEAT_STEP 	     1		//the unit is minute
#define FOTA_INTERVAL_MIN 			(8) //the unit is hour
#define FOTA_INTERVAL_MAX 			(72)	//the unit is hour
#define FOTA_INTERVAL_STEP 			1		//the unit is hour
#define IS_PARA_VALID(max,min,step,value)	(((value > max)||(value < min)||(value%step != 0))?0:1)


/*******************************************************************************
 * Variables
 ******************************************************************************/
#ifdef MQTT
extern int my_netif_status;
#endif
struct netif fsl_netif0;
int cache_status = 1;
int network_state = 0;

int8_t uplink_send_event_handle(UPLINK_Q_MSG *q_ptr);
int8_t uplink_check_sensor_fota_handle(UPLINK_Q_MSG *q_ptr);
int8_t uplink_fw_download_handle(UPLINK_Q_MSG *q_ptr);
int8_t uplink_resp_fw_update_result_handle(UPLINK_Q_MSG *q_ptr);
int8_t uplink_get_sub_devices_handle(UPLINK_Q_MSG *q_ptr);

void http_client_netconn_thread(void *arg);

int8_t http_analysis_result(int16_t status_code);
int httpclient_init(void);
int8_t http_send_then_recv_cache(HTTP_CLIENT_POST_STRUCT *post, UPLINK_Q_MSG *save_q);
int8_t http_send_then_recv_nocache(HTTP_CLIENT_POST_STRUCT *post);
static int do_paremeter_update(FW_UPDATE_RESP_STRUCT *update, HTTP_CLIENT_POST_STRUCT *http);
static int update_url_verify(int url_type, char *url, int urllen, AES_STRUCT * aes);
int update_url(char *new_url, int len, int url_type);


/*!
 * @brief The main function containing client thread.
 */
int http_client_init(void)
{
    LWIP_PLATFORM_DIAG(("\r\n************************************************"));
    LWIP_PLATFORM_DIAG((" HTTP Client cfg.....\r\n"));
    LWIP_PLATFORM_DIAG(("************************************************"));
#ifdef CPU_MK24FN256VDC12
#ifdef WIFI_MODULE
    while(wifi_check_netconn_status(2000) != RET_OK)
    	PRINTF("wifi is preparing ...\r\n");
	PRINTF("wifi network status is ok ...\r\n");
#else
    while(!ppp_gprs_info.ppp_negotiate_result)
    	vTaskDelay( 2000 / portTICK_RATE_MS );
#endif
#else
    ip_addr_t fsl_netif0_ipaddr, fsl_netif0_netmask, fsl_netif0_gw;
    MPU_Type *base = MPU;

    /* Disable MPU. */
    base->CESR &= ~MPU_CESR_VLD_MASK;

    LWIP_DEBUGF(HTTPD_DEBUG, ("TCP/IP initializing...\r\n"));
    tcpip_init(NULL, NULL);
    LWIP_DEBUGF(HTTPD_DEBUG, ("TCP/IP initialized.\r\n"));
    IP4_ADDR(&fsl_netif0_ipaddr, configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3);
    IP4_ADDR(&fsl_netif0_netmask, configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3);
    IP4_ADDR(&fsl_netif0_gw, configGW_ADDR0, configGW_ADDR1, configGW_ADDR2, configGW_ADDR3);

    netif_add(&fsl_netif0, &fsl_netif0_ipaddr, &fsl_netif0_netmask, &fsl_netif0_gw, NULL, ethernetif_init, tcpip_input);
    netif_set_default(&fsl_netif0);
    LWIP_PLATFORM_DIAG((" IPv4 Address     : %u.%u.%u.%u", ((u8_t *)&fsl_netif0_ipaddr)[0],
                        ((u8_t *)&fsl_netif0_ipaddr)[1], ((u8_t *)&fsl_netif0_ipaddr)[2],
                        ((u8_t *)&fsl_netif0_ipaddr)[3]));
    LWIP_PLATFORM_DIAG((" IPv4 Subnet mask : %u.%u.%u.%u", ((u8_t *)&fsl_netif0_netmask)[0],
                        ((u8_t *)&fsl_netif0_netmask)[1], ((u8_t *)&fsl_netif0_netmask)[2],
                        ((u8_t *)&fsl_netif0_netmask)[3]));
    LWIP_PLATFORM_DIAG((" IPv4 Gateway     : %u.%u.%u.%u", ((u8_t *)&fsl_netif0_gw)[0], ((u8_t *)&fsl_netif0_gw)[1],
                        ((u8_t *)&fsl_netif0_gw)[2], ((u8_t *)&fsl_netif0_gw)[3]));

    LWIP_PLATFORM_DIAG(("************************************************"));
#endif
#ifndef WIFI_MODULE
    netif_set_up(&fsl_netif0);
#endif
    return 0;
}

/*!
 * @brief The function is return net connection state.
 * return 1: net is working, return 0 net is unworkable.
 */
int is_networking(void)
{
	return network_state;
}

void set_net_working(int value)
{
	network_state = value;
}

void http_client_netconn_thread(void *arg)
{
	int8_t res;
	uint8_t sub_dev_first = 1;
	UPLINK_Q_MSG msg_q;
    LWIP_UNUSED_ARG(arg);
    memset(&msg_q, 0, sizeof(msg_q));
    http_client_init();
    if(uplink_protocol_init() == false)
    	 PRINTF("uplink protocol init fail!!!!\r\n");
#ifdef MQTT
    my_netif_status = 1;
#endif
    PRINTF(" into client...\r\n");
    while(1)
    {
    	//xQueueReceive(uplink_q_handle,&msg_q,portMAX_DELAY);
    	if(xQueueReceive(uplink_q_handle,&msg_q, portMAX_DELAY) == pdPASS)
    	{
    		xEventGroupSetBits(hbbeventgroup, HTTP_QUEUE_EVENT);//set bit, there is data need to send.
			switch(msg_q.cmd)
			{
				case UPLINK_SEND_EVENT:
				{
					PRINTF("\r\n\r\n-------send event-----\r\n");
					res = uplink_send_event_handle(&msg_q);
					uint32_t ts_temp;
					ts_temp = msg_q.event.ts/1000;
					if(res == UPLINK_HANDLE_ERR)
						TESTLOG("## non-uploaded ## did = %08X,sid = %08X,type = %d,occur_ts = %lu\r\n",hbb_info.config.id,msg_q.event.slid.id,msg_q.event.status,ts_temp);
					else
					{
						if(msg_q.event.status == INFO)
							hbb_info.hb_tick = xTaskGetTickCount();//update heartbeat tick
						set_net_working(1);
						TESTLOG("## uploaded ## did = %08X,sid = %08X,type = %d,occur_ts = %lu\r\n",hbb_info.config.id,msg_q.event.slid.id,msg_q.event.status,ts_temp);
					}

					break;
				}
				case UPLINK_FW_UPD_REQ:
				{
					PRINTF("\r\n\r\n-------send fota-----\r\n");
					res = uplink_check_sensor_fota_handle(&msg_q);
					if(res == UPLINK_HANDLE_ERR)
						PR_WARNING("fota Request is fail!!\r\n");
					break;
				}
				case UPLINK_DOWNLOAD_IMG:
				{
					PRINTF("\r\n\r\n-------request img-----\r\n");
					PRINTF("rid:%d\r\n",msg_q.fw_down.rid);
					PRINTF("img block NO:%d\r\n",msg_q.fw_down.img_blk_num);
					PRINTF("url:%s\r\n",msg_q.fw_down.url);
					PRINTF("key----:%s\r\n",msg_q.fw_down.key);
					res = uplink_fw_download_handle(&msg_q);
					if(res == UPLINK_HANDLE_ERR)
						PR_WARNING("\r\n fota download image Request is fail!!\r\n");
					break;
				}
				case UPLINK_FW_UPD_RESULT:
				{
					PRINTF("\r\n\r\n-------send result-----\r\n");
					res = uplink_resp_fw_update_result_handle(&msg_q);
					if(res == UPLINK_HANDLE_ERR)
						PR_WARNING("fota update Request is fail!!\r\n");
					break;
				}
				case UPLINK_GET_SUB_DEVICES:
				{
					PRINTF("\r\n\r\n-------get sensor list-----\r\n");
					uint8_t sub_count = 0;
					while(uplink_get_sub_devices_handle(&msg_q) != UPLINK_HANDLE_OK)
					{
						sub_count++;
						if(sub_dev_first == 0)
							break;
						if(sub_count >= 2)
							break;
						PRINTF("\r\n Request is fail!!\r\n");
						PRINTF("try again...\r\n");
						vTaskDelay(3000);
					}
					if(sub_count < 2)
						set_net_working(1);
					sub_dev_first = 0;
					break;
				}
				default:
				{
					PR_WARNING("cmd is undefine :%d\r\n",msg_q.cmd);
					break;
				}
			}
    	}
		xEventGroupClearBits(hbbeventgroup, HTTP_QUEUE_EVENT);
    }
}

int8_t uplink_send_event_handle(UPLINK_Q_MSG *q_ptr)
{
	HTTP_CLIENT_POST_STRUCT http_client_post_struct;
	get_event_request_json(q_ptr, hbb_info.config.aeskey, hbb_info.config.aesivv, hbb_info.config.key_version);

	if ((q_ptr->event.status == ALERT) || (q_ptr->event.status == UNALERT))
		http_client_post_struct.url_ptr = hbb_info.config.alerturl;
	else
		http_client_post_struct.url_ptr = hbb_info.config.eventurl;

	http_client_post_struct.purpose = SEND_EVENT;
	http_client_post_struct.send_buf_ptr = json_buf;
	http_client_post_struct.send_buf_length = strlen(json_buf);
	if(http_send_then_recv_cache(&http_client_post_struct, q_ptr) == UPLINK_HANDLE_ERR)
		return UPLINK_HANDLE_ERR;
	if(cache_status == 1){
		cache_status = 0;
		PRINTF("get log...\r\n");
		defer_log_uplink();
	}
	return UPLINK_HANDLE_OK;
}

int8_t uplink_check_sensor_fota_handle(UPLINK_Q_MSG *q_ptr)
{
	HTTP_CLIENT_POST_STRUCT http_client_post_struct;
	FW_UPDATE_RESP_STRUCT fw_update_resp_struct;
	UPLINK_Q_MSG result;
	uint32_t parse_result;
	LONG_ID96 longdid;
	char url[128];
	longdid.id = hbb_info.config.id;
	longdid.idh = hbb_info.config.longidh;
	longdid.idm = hbb_info.config.longidm;
	get_fw_update_request_json(q_ptr, hbb_info.config.aeskey, hbb_info.config.aesivv, hbb_info.config.key_version, &longdid);//get the info write to the json buff
	strcpy(url,hbb_info.config.fotaurl);
	strcat(url,"/metadata");
	PRINTF("URL: %s\r\n",url);

	http_client_post_struct.url_ptr = url;
	http_client_post_struct.purpose = CHECK_FW_UPDATE;
	http_client_post_struct.send_buf_ptr = json_buf;
	http_client_post_struct.send_buf_length = strlen(json_buf);
	if(http_send_then_recv_nocache(&http_client_post_struct) != UPLINK_HANDLE_OK)
	{
		//send fail to fota
		return UPLINK_HANDLE_ERR;
	}

	if(http_client_post_struct.result_code == NO_DEV_UPD)
		return UPLINK_HANDLE_OK;

	http_client_post_struct.recv_buf_ptr[http_client_post_struct.recv_buf_length] = '\0';
	//PRINTF("recieve json:\r\n%s\r\n",http_client_post_struct.recv_buf_ptr);

	//parse JSON get the download url
	parse_result = check_fota_resp_json_parse(http_client_post_struct.recv_buf_ptr,&fw_update_resp_struct);
	if(parse_result == JSON_PARSE_ERR)
	{
		PRINTF("parse err \r\n");
		//SEND RC
		result.cmd = UPLINK_FW_UPD_RESULT;
		result.fw_upd_result.rid = 0;
		result.fw_upd_result.rc = GET_METADATA_INVAL;
		if(uplink_resp_fw_update_result_handle(&result) == UPLINK_HANDLE_OK)
			return UPLINK_HANDLE_OK;
		return UPLINK_HANDLE_ERR;
	}
	else{
		PRINTF("parse is successful! \r\n");
		if(fw_update_resp_struct.body.t == PARAMETER){

			if(do_paremeter_update(&fw_update_resp_struct, &http_client_post_struct) == 0)
			{
				result.fw_upd_result.rc = REQ_SUCC;
				hbb_info.hb_tick = xTaskGetTickCount();//update heartbeat tick
			}
			else
			{
				result.fw_upd_result.rc = UPDATE_PARA_ERR;
				PR_WARNING("paremeter verify is FAIL!!\r\n");
			}
			if(fw_update_resp_struct.body.ifm == 1)
			{
				UPLINK_Q_MSG uplink_check_fw;
				uplink_check_fw.cmd = UPLINK_FW_UPD_REQ;
				uplink_check_fw.fw_req.bid = 0;
				xQueueSend(uplink_q_handle, &uplink_check_fw, 0);
			}
			PRINTF("send result...\r\n");
			result.fw_upd_result.rid = fw_update_resp_struct.body.rid;
			return uplink_resp_fw_update_result_handle(&result);
		}
	}
	/*******self test********/
#if TEST
	UPLINK_Q_MSG t;
	for(int i=0;i<fw_update_resp_struct.body.fwblks;i++)
	{
		t.fw_down.img_blk_num = i;
		t.fw_down.key = fw_update_resp_struct.body.key;
		t.fw_down.rid = fw_update_resp_struct.body.rid;
		t.fw_down.url = fw_update_resp_struct.body.url;

		uplink_fw_download_handle(&t);
	}
//	UPLINK_Q_MSG t1;
//	t1.fw_upd_result.rid = 1042;
//	t1.fw_upd_result.rc = 446;
//	uplink_resp_fw_update_result_handle(&t1);
#else
	//send msg queue
	FOTA_RQ_MSG msg;
	msg.cmd = FW_UPD_RESP;
	msg.fw_upd_req.dt = fw_update_resp_struct.body.dt;
	msg.fw_upd_req.fwblks = fw_update_resp_struct.body.img.fwblks;
	msg.fw_upd_req.fwpv = fw_update_resp_struct.body.fwpv;
	msg.fw_upd_req.fws = fw_update_resp_struct.body.img.fws;
	msg.fw_upd_req.fwuv = fw_update_resp_struct.body.fwuv;
	msg.fw_upd_req.hdv = fw_update_resp_struct.body.hdv;
	msg.fw_upd_req.ifm = fw_update_resp_struct.body.ifm;
	msg.fw_upd_req.key = fw_update_resp_struct.body.img.key;
	msg.fw_upd_req.rid = fw_update_resp_struct.body.rid;
	msg.fw_upd_req.t = fw_update_resp_struct.body.t;
	msg.fw_upd_req.url = fw_update_resp_struct.body.img.url;

	xQueueSend(fota_rq_handle, &msg, 2000);
#endif
	return UPLINK_HANDLE_OK;
}

void send_download_fail_msg(int fail_code, uint32_t timeout_ms)
{
	FOTA_RQ_MSG msg;
	msg.cmd = FW_IMG_DOWNLOAD_FAIL;
	msg.fw_fail_info.fail_code = fail_code;
	xQueueSend(fota_rq_handle, &msg, timeout_ms);
}

int8_t uplink_fw_download_handle(UPLINK_Q_MSG *q_ptr)
{
	HTTP_CLIENT_POST_STRUCT http_client_post_struct;
	get_sensor_image_request_json(q_ptr, hbb_info.config.aeskey, hbb_info.config.aesivv, hbb_info.config.key_version);

	http_client_post_struct.url_ptr = img_down_url;
	http_client_post_struct.purpose = DOWNLOAD_IMAGE_BLKS;
	http_client_post_struct.send_buf_ptr = json_buf;
	http_client_post_struct.send_buf_length = strlen(json_buf);

	if(http_send_then_recv_nocache(&http_client_post_struct) != UPLINK_HANDLE_OK){
		//send img download error to fota
		PRINTF("IMG block download fail!!!\r\n");
		send_download_fail_msg(443, 2000);
		return UPLINK_HANDLE_ERR;
	}
	PRINTF("Receive img size is %d byte!\r\n", http_client_post_struct.recv_buf_length);
	if(http_client_post_struct.recv_buf_length == FOTA_IMG_CELL_DOWN_LEN)
	{
		FOTA_RQ_MSG msg;
		memset(fota_data_read, 0, FOTA_IMG_CELL_DOWN_LEN);
		msg.cmd = q_ptr->fw_down.back_cmd;
		msg.fw_blk_down.img_blk_len = http_client_post_struct.recv_buf_length;//sizeof(remote_sha1);
		memcpy(fota_data_read, http_client_post_struct.recv_buf_ptr, FOTA_IMG_CELL_DOWN_LEN);
#if !TEST
		xQueueSend(fota_rq_handle, &msg, 2000);
#endif
	}
	else{
		//send img size error to fota
		PRINTF("image size is error!!\r\n");//return result 443
		send_download_fail_msg(443, 2000);
		return UPLINK_HANDLE_ERR;
	}
	return UPLINK_HANDLE_OK;
}

int8_t uplink_resp_fw_update_result_handle(UPLINK_Q_MSG *q_ptr)
{
	HTTP_CLIENT_POST_STRUCT http_client_post_struct;
	char url[128];
	//get the info write to the json buff

	get_resp_update_result_json(q_ptr,  hbb_info.config.aeskey, hbb_info.config.aesivv, hbb_info.config.key_version);
	strcpy(url,hbb_info.config.fotaurl);
	strcat(url,"/update-result");
	PRINTF("URL: %s\r\n",url);

	http_client_post_struct.url_ptr = url;
	http_client_post_struct.purpose = RESP_UPDATE_RESULT;
	http_client_post_struct.send_buf_ptr = json_buf;
	http_client_post_struct.send_buf_length = strlen(json_buf);
	if(http_send_then_recv_cache(&http_client_post_struct, q_ptr) != UPLINK_HANDLE_OK)
		return UPLINK_HANDLE_ERR;

	return UPLINK_HANDLE_OK;
}

int8_t uplink_get_sub_devices_handle(UPLINK_Q_MSG *q_ptr)
{
	HTTP_CLIENT_POST_STRUCT http_client_post_struct;
	GET_SUB_DEV_RESP_STRUCT get_sub_dev_resp_struct;
	int8_t res;
	//get the info write to the json buff
	get_sub_devices_request_json(q_ptr, hbb_info.config.aeskey, hbb_info.config.aesivv, hbb_info.config.key_version);

	http_client_post_struct.url_ptr = hbb_info.config.controlurl;
	http_client_post_struct.purpose = GET_SUB_DEV;
	http_client_post_struct.send_buf_ptr = json_buf;
	http_client_post_struct.send_buf_length = strlen(json_buf);
	if(http_send_then_recv_nocache(&http_client_post_struct) != UPLINK_HANDLE_OK)
	{
		if(REQ_DEV_NOT_FOUND == http_client_post_struct.result_code)
		{
			xEventGroupSetBits(hbbeventgroup, HBB_GETLIST_EVENT);
			return UPLINK_HANDLE_OK;
		}

		return UPLINK_HANDLE_ERR;
	}

	http_client_post_struct.recv_buf_ptr[http_client_post_struct.recv_buf_length] = '\0';
	//PRINTF("recieve json:\r\n%s\r\n",http_client_post_struct.recv_buf_ptr);

	res = get_sub_devices_resp_json_parse(http_client_post_struct.recv_buf_ptr,&get_sub_dev_resp_struct);
	if(res == JSON_PARSE_ERR)
	{
		//SEND RC
		PRINTF("parse err \r\n");
		return UPLINK_HANDLE_ERR;
	}
	PRINTF("parse is successful! \r\n");
	set_rtc_date(get_sub_dev_resp_struct.body.cts);
	uninstall_sensors(&get_sub_dev_resp_struct.body.sids);
	for(int i=0;i<get_sub_dev_resp_struct.body.sids.num;i++)
		install_sensor(&get_sub_dev_resp_struct.body.sids.idn[i].lid, get_sub_dev_resp_struct.body.sids.idn[i].dt);

	xEventGroupSetBits(hbbeventgroup, HBB_GETLIST_EVENT);
	return UPLINK_HANDLE_OK;
}

int8_t http_send_then_recv_cache(HTTP_CLIENT_POST_STRUCT *post, UPLINK_Q_MSG *save_q)
{
	int8_t http_res, analy_res;

	http_res = HTTPClient_post(post);
	 //it's allow when http can't match content length in event handle.
	if(http_res == HTTP_SUCC){
		analy_res = http_analysis_result(post->result_code);
		if(analy_res == CLIENT_OK)
			return UPLINK_HANDLE_OK;
		else
			return UPLINK_HANDLE_ERR;
	}

	if((post->purpose == SEND_EVENT) && (save_q->event.status == INFO)){//heartbeat msg no cache
		return UPLINK_HANDLE_ERR;
	}
	else if((post->purpose == SEND_EVENT) && ((save_q->event.status == ALERT)||(save_q->event.status == UNALERT)))
	{
		if(post->result_code == BAD_REQ)
			return UPLINK_HANDLE_ERR;
		PRINTF("save alert msg...\r\n");
		save_defer_log(save_q);
		cache_status = 1;
	}
	else if(post->purpose == RESP_UPDATE_RESULT)
	{
		if(post->result_code == DEV_NOT_FOUND)
			return UPLINK_HANDLE_ERR;
		PRINTF("save fota result msg...\r\n");
		save_defer_log(save_q);
		cache_status = 1;
	}

	return UPLINK_HANDLE_ERR;
}

int8_t http_send_then_recv_nocache(HTTP_CLIENT_POST_STRUCT *post)
{
	int8_t http_res, analy_res;

	http_res = HTTPClient_post(post);
	if(http_res == HTTP_SUCC){
		analy_res = http_analysis_result(post->result_code);
		if(analy_res == CLIENT_OK)
			return UPLINK_HANDLE_OK;
	}
	return UPLINK_HANDLE_ERR;
}

int8_t http_analysis_result(int16_t status_code)
{
	/*Print the http result log*/

	/*Print http status code*/
	switch(status_code)
	{
	case REQ_SUCC://1
		PRINTF("Request is successful!\r\n");
		return CLIENT_OK;
	case BAD_REQ://1
		PRINTF(".Bad request from client!\r\n");
		return CLIENT_ERR;
	case ERR_404://
		PR_ERROR("error 404 Not found !\r\n", ERR_404);
		return SERVER_ERR;
	case ERR_403://
		PR_ERROR("error 403 refuse denied!\r\n", ERR_403);
		return SERVER_ERR;
	case FOTA_REQ_DECRYP_FAIL:
		PRINTF("Fota client request decryption fail!\r\n");
		return CLIENT_ERR;
	case FOTA_REQ_DATA_INVAL:
		PR_ERROR("The fota client request data invalid.\r\n", FOTA_REQ_DATA_INVAL);
		return SERVER_ERR;
	case NO_DEV_UPD:
		PRINTF("No device need to upgrade in the homebox!\r\n");
		PRINTF("Request is successful!\r\n");
		return CLIENT_OK;
	case PARAM_NULL_OR_INVAL:
		PRINTF("Some parameter of the device to upgrade is null or invalid!\r\n");
		return CLIENT_ERR;
	case DEV_NOT_FOUND:
		PR_ERROR("The upgraded device can not found in database!\r\n", DEV_NOT_FOUND);
		return SERVER_ERR;
	case SIGNATURE_NOT_GET:
		PR_ERROR("Signature key can't get from database!\r\n", SIGNATURE_NOT_GET);
		return SERVER_ERR;
	case SIGNATURE_NOT_MATCH:
		PR_ERROR("The signature key doesn't match!\r\n", SIGNATURE_NOT_MATCH);
		return SERVER_ERR;
	case IMG_NOT_EXIST:
		PR_ERROR("The img need to download does not exist!\r\n", IMG_NOT_EXIST);
		return SERVER_ERR;
	case SERVER_UNEXPECT_EER:
		PR_ERROR("The service occurred unexpected error!\r\n", SERVER_UNEXPECT_EER);
		return SERVER_ERR;
	case SERVER_UNSUP_ENCODING:
		PR_ERROR("The service occurred unexpected unsupported encoding exception!\r\n", SERVER_UNSUP_ENCODING);
		return SERVER_ERR;
	case SERVER_IOEXCEPTION:
		PR_ERROR("The service occurred IOException!\r\n", SERVER_IOEXCEPTION);
		return SERVER_ERR;
	case REQ_DEV_NOT_FOUND:
		PR_ERROR("The requested device couldn't be found!\r\n", REQ_DEV_NOT_FOUND);
		return SERVER_ERR;
	case REQ_DATA_INVAL:
		PR_ERROR("The request data invalid.!\r\n", REQ_DATA_INVAL);
		return SERVER_ERR;
	case SERVER_OC_UNEXP_ERR:
		PR_ERROR("The service occurred unexpected error!\r\n", SERVER_OC_UNEXP_ERR);
		return SERVER_ERR;
	case NO_DEV_FOUND:
		PR_ERROR("No device be found by the DID.!\r\n", NO_DEV_FOUND);
		return SERVER_ERR;
	case SERVER_OC_UNSUP_ERR:
		PR_ERROR("The service occurred unexpected unsupported encoding exception.\r\n", SERVER_OC_UNSUP_ERR);
		return SERVER_ERR;
	case SERVER_OC_IOE:
		PR_ERROR("The service occurred IOException.\r\n", SERVER_OC_IOE);
		return SERVER_ERR;
	case SIG_KEY_NOT_MATCH:
		PR_ERROR("The signature key doesn't match.!\r\n", SIG_KEY_NOT_MATCH);
		return SERVER_ERR;
	default:
		PRINTF("http status code undefine  status code:%d\r\n",status_code);
		break;
	}
	return CLIENT_ERR;
}


/******do_paremeter_update(FW_UPDATE_RESP_STRUCT *update)
 *@param update the point of parameter
 *
 *@return 0 if verify succeed,otherwise -1
 */

static int do_paremeter_update(FW_UPDATE_RESP_STRUCT *update, HTTP_CLIENT_POST_STRUCT *http)
{
	/*PARA_URL_FOTA = 1,
	 PARA_URL_EVENT,
	 PARA_URL_ALERT,
	 PARA_URL_CTRL,
	 PARA_AES_KEY,
	 PARA_HEART_BEAT,
	 PARA_FOTA_INTERVAL,*/
	AES_STRUCT aes;
	aes.key = hbb_info.config.aeskey;
	aes.ivv = hbb_info.config.aesivv;
	aes.version = hbb_info.config.key_version;
	if((update->body.parameter.para_type == PARA_URL_EVENT)||(update->body.parameter.para_type == PARA_URL_ALERT)||
	  (update->body.parameter.para_type == PARA_URL_CTRL)||(update->body.parameter.para_type == PARA_URL_FOTA))
	{
		PRINTF("Do fota url verify...\r\n");
		if(update->body.parameter.para_type == PARA_URL_FOTA)
		{
			char url[128];
			strcpy(url,update->body.parameter.url);
			strcat(url,"/metadata");
			if(update_url_verify(update->body.parameter.para_type, url, strlen(url), &aes) == 0)
				return update_url(update->body.parameter.url, strlen(update->body.parameter.url), update->body.parameter.para_type);
		}
		else
		{
			if(update_url_verify(update->body.parameter.para_type, update->body.parameter.url,
				strlen(update->body.parameter.url), &aes) == 0)
				return update_url(update->body.parameter.url, strlen(update->body.parameter.url), update->body.parameter.para_type);
		}
	}
	else if(update->body.parameter.para_type == PARA_AES_KEY)
	{
		PRINTF("Do aes key verify...\n");
		aes.key = update->body.parameter.aes.newkey;
		aes.ivv = update->body.parameter.aes.newVector;
		aes.version = update->body.parameter.aes.newEncvr;
		if(update_url_verify(PARA_URL_EVENT, hbb_info.config.eventurl,
			strlen(hbb_info.config.eventurl), &aes) != 0)
			return -1;
		PRINTF("event done...\n");
		if(update_url_verify(PARA_URL_CTRL, hbb_info.config.controlurl,
			strlen(hbb_info.config.controlurl), &aes) != 0)
			return -1;
		PRINTF("control done...\n");
		if(update_url_verify(PARA_URL_FOTA, http->url_ptr, strlen(http->url_ptr), &aes) != 0)
			return -1;
		PRINTF("fota done...\r\n");
		return update_hbb_aeskey(aes.ivv, aes.key, aes.version);
	}
	else if(update->body.parameter.para_type == PARA_HEART_BEAT)
	{

		PRINTF("Do heart beat verify...\r\n");
		if(IS_PARA_VALID(HEART_BEAT_MAX,HEART_BEAT_MIN,HEART_BEAT_STEP,update->body.parameter.heartbeat))
			return update_hb_heartbeat(update->body.parameter.heartbeat * 60 * 1000);//minute conversion to ms
		else
			return -1;
	}
	else if(update->body.parameter.para_type == PARA_FOTA_INTERVAL)
	{

		PRINTF("Do fota interval verify...\r\n");
		if(IS_PARA_VALID(FOTA_INTERVAL_MAX,FOTA_INTERVAL_MIN,FOTA_INTERVAL_STEP,update->body.parameter.fota_interval))
		{
			update_fota_timing(update->body.parameter.fota_interval * 3600 * 1000);//hour conversion ms
			return 0;
		}else
			return -1;

	}
	else if(update->body.parameter.para_type == PARA_SOS_PHONE)
	{
		PRINTF("Update sos phone:%s\r\n", update->body.parameter.sos);
#ifdef CPU_MK24FN256VDC12
		update_sos_call_number(update->body.parameter.sos);
#endif
		return 0;
	}
	else if(update->body.parameter.para_type == PARA_AUTO_ANSWER)
	{
		PRINTF("Update auto answer:%d\r\n", update->body.parameter.auto_answer);
#ifdef CPU_MK24FN256VDC12
		update_incoming_call_auto_answer_timeout(update->body.parameter.auto_answer);//the unit is second
#endif
		return 0;
	}
	return -1;
}

static int update_url_verify(int url_type, char *url, int urllen, AES_STRUCT * aes)
{
	UPLINK_Q_MSG verify;
	HTTP_CLIENT_POST_STRUCT http_client_post_struct;
	memset(&verify, 0, sizeof(verify));
	switch(url_type)
	{
		case PARA_URL_EVENT:
		{
			verify.event.slid.id = hbb_info.config.id;
			verify.event.slid.idh = hbb_info.config.longidh;
			verify.event.slid.idm = hbb_info.config.longidm;
			verify.event.device_type = ver_info.dt;
			verify.event.status = INFO;
			verify.event.p = 1;
			verify.event.fwv = ver_info.swver;
			verify.event.hwv = ver_info.hwver;
			verify.event.data[0] = '0';
			verify.event.data[1] = '\0';
			get_current_systime(&verify.event.ts);
			get_event_request_json(&verify, aes->key, aes->ivv, aes->version);
			http_client_post_struct.purpose = SEND_EVENT;
			break;
		}
		case PARA_URL_ALERT:
		{
			verify.event.slid.id = hbb_info.config.id;
			verify.event.slid.idh = hbb_info.config.longidh;
			verify.event.slid.idm = hbb_info.config.longidm;
			verify.event.device_type = ver_info.dt;
			verify.event.status = INFO;
			verify.event.p = 1;
			verify.event.fwv = ver_info.swver;
			verify.event.hwv = ver_info.hwver;
			verify.event.data[0] = '0';
			verify.event.data[1] = '\0';
			get_current_systime(&verify.event.ts);
			get_event_request_json(&verify, aes->key, aes->ivv, aes->version);
			http_client_post_struct.purpose = SEND_EVENT;
			break;
		}
		case PARA_URL_CTRL:
		{
			verify.sub_dev_req.pos = 1;
			get_sub_devices_request_json(&verify, aes->key, aes->ivv, aes->version);
			http_client_post_struct.purpose = GET_SUB_DEV;
			break;
		}
		case PARA_URL_FOTA:
		{
			LONG_ID96 longdid;
			longdid.id = hbb_info.config.id;
			longdid.idh = hbb_info.config.longidh;
			longdid.idm = hbb_info.config.longidm;
			verify.fw_req.bid = 0;
			get_fw_update_request_json(&verify, aes->key, aes->ivv, aes->version, &longdid);//0x10000000 is special for verify fota url.
			http_client_post_struct.purpose = CHECK_FW_UPDATE;
			break;
		}
		default:
			return -1;
	}

	http_client_post_struct.url_ptr = url;
	http_client_post_struct.send_buf_ptr = json_buf;
	http_client_post_struct.send_buf_length = strlen(json_buf);
	if(HTTPClient_post(&http_client_post_struct) != HTTP_SUCC)
	{
		PR_WARNING("verify is error, post is failed!!!\r\n");
		return -1;
	}
	if((http_client_post_struct.result_code != REQ_SUCC) && (http_client_post_struct.result_code != NO_DEV_UPD))
	{
		PR_ERROR("verify is error, http rc = %lu \r\n!!!\r\n",http_client_post_struct.result_code);
		return -1;
	}
	return 0;
}

int update_url(char *new_url, int len, int url_type)
{
	switch(url_type)
	{
	case PARA_URL_FOTA:
		update_hbb_fotaurl(new_url,len);
		break;
	case PARA_URL_EVENT:
		update_hbb_eventurl(new_url,len);
		break;
	case PARA_URL_ALERT:
		update_hbb_alerturl(new_url,len);
		break;
	case PARA_URL_CTRL:
		update_hbb_controlturl(new_url,len);
		break;
	default:
		return -1;
	}
	return 0;
}

#endif /* LWIP_NETCONN */
