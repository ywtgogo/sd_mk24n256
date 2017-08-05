#include "json.h"
#include "hb_protocol.h"

//char temp_buff[1000];
char aes_buff[5000];
//static int parse_dl(const char *start_ptr, const char *end_ptr, JSON_RECV_DEV_LIST *js);
static int get_dev_list_body_deserialize(char *data_ptr, JSON_RECV_DEV_LIST *js);
static int get_dev_list_head_deserialize(char *data_ptr, JSON_RECV_DEV_LIST *js);
static int parse_dtl(const char *start_ptr, const char *end_ptr, GET_DATA *gd);

char *protocol_head_format =
		"{\"headers\":{\"pv\":\"%s\",\"enc\":\"%d\",\"sig\":\"%d\",\"func\":\"%u\",\"sts\":\"%u%03u\",\"ev\":\"%s\"},\"body\":\"%s\"}";
char *get_dev_list_body_format =
		"{\"did\":\"%X\"}";
char *send_dev_list_result_body_format =
		"{\"msgid\":\"%s\",\"rc\":\"%u\"}";
char *send_event_body_format =
		"{\"msgid\":\"%s\",\"t\":\"%d\",\"sid\":\"%08X%08X%08X\",\"did\":\"%08X%08X%08X\",\"dt\":\"%08X\",\"p\":\"%d\",\"data\":\"%s\",\"ts\":\"%u%03u\","
		"\"fwv\":\"%s\"}";
char *uploading_body_format =
		"{\"msgid\":\"%s\",\"log\":\"%s\",\"bthid\":\"%d\",\"bthct\":\"%d\"}";
char *send_data_body_format =
		"{\"msgid\":\"%s\",\"qt\":\"%d\",\"rc\":\"%d\",\"items\":[%s]}";

void json_serialize_send_data(JSON_SEND_DATA *js, char *jsonBuff, int buffLen)
{
	memset(jsonBuff, 0, buffLen);
	memset(aes_buff, 0, sizeof(aes_buff));
	sprintf(jsonBuff, send_data_body_format, js->send_data_body.msgid, js->send_data_body.qt, js->send_data_body.rc, js->send_data_body.items);
	JsDbPrint("body:%s\r\n",jsonBuff);
	if(js->phead.enc == AES256)
			AES_base64_encrypt(js->phead.aes->key, js->phead.aes->ivv, (unsigned char *)jsonBuff, (unsigned char *)aes_buff, strlen(jsonBuff));
	else
		memcpy(aes_buff, jsonBuff, strlen(jsonBuff)+1);//include '\0'
	memset(jsonBuff, 0, buffLen);
	sprintf(jsonBuff, protocol_head_format, js->phead.pv, js->phead.enc, js->phead.sig, js->phead.func, js->phead.sts_s,
				js->phead.sts_ms, js->phead.aes->version, aes_buff);
	JsDbPrint("send data jsonbuff:\r\n%s\r\n",jsonBuff);
}

void json_serialize_uploading_log(JSON_UPLOADING_LOG *js, char *jsonBuff, int buffLen)
{
	memset(jsonBuff, 0, buffLen);
	//memset(temp_buff, 0, sizeof(temp_buff));
	memset(aes_buff, 0, sizeof(aes_buff));
	sprintf(jsonBuff, uploading_body_format, js->uploading_log_body.msgid, js->uploading_log_body.log_content, js->uploading_log_body.btchct,
			js->uploading_log_body.btchid);
	JsDbPrint("body:%s\r\n",jsonBuff);
	if(js->phead.enc == AES256)
			AES_base64_encrypt(js->phead.aes->key, js->phead.aes->ivv, (unsigned char *)jsonBuff, (unsigned char *)aes_buff, strlen(jsonBuff));
	else
		memcpy(aes_buff, jsonBuff, strlen(jsonBuff)+1);//include '\0'
	memset(jsonBuff, 0, buffLen);
	sprintf(jsonBuff, protocol_head_format, js->phead.pv, js->phead.enc, js->phead.sig, js->phead.func, js->phead.sts_s,
				js->phead.sts_ms, js->phead.aes->version, aes_buff);
	JsDbPrint("uploading log jsonbuff:\r\n%s\r\n",jsonBuff);
}

void json_serialize_send_event(JSON_SEND_EVENT *js, char *jsonBuff, int buffLen)
{
	memset(jsonBuff, 0, buffLen);
	//memset(temp_buff, 0, sizeof(temp_buff));
	memset(aes_buff, 0, sizeof(aes_buff));

	sprintf(jsonBuff, send_event_body_format, js->send_event_body.msgid, js->send_event_body.t, js->send_event_body.slid->idh,
			js->send_event_body.slid->idm, js->send_event_body.slid->id, hbb_info.config.longidh, hbb_info.config.longidm,
			hbb_info.config.id, js->send_event_body.dt,js->send_event_body.p, js->send_event_body.data, js->send_event_body.ts_s,
			js->send_event_body.ts_ms, js->send_event_body.fwv);
	JsDbPrint("body:%s\r\n",jsonBuff);
	if(js->phead.enc == AES256)
		AES_base64_encrypt(js->phead.aes->key, js->phead.aes->ivv, (unsigned char *)jsonBuff, (unsigned char *)aes_buff, strlen(jsonBuff));
	else
		memcpy(aes_buff, jsonBuff, strlen(jsonBuff)+1);//include '\0'
	memset(jsonBuff, 0, buffLen);
	sprintf(jsonBuff, protocol_head_format, js->phead.pv, js->phead.enc, js->phead.sig, js->phead.func, js->send_event_body.ts_s,
			js->send_event_body.ts_ms, js->phead.aes->version, aes_buff);
	JsDbPrint("send event jsonbuff:\r\n%s\r\n",jsonBuff);
}

void json_serialize_get_dev_list(JSON_GET_DEV_LIST *js, char *jsonBuff, int buffLen)
{
	memset(jsonBuff, 0, buffLen);
	//memset(temp_buff, 0, sizeof(temp_buff));
	memset(aes_buff, 0, sizeof(aes_buff));

	sprintf(jsonBuff, get_dev_list_body_format, js->get_dev_list_body.did);
	JsDbPrint("body:%s\r\n",jsonBuff);
	if(js->phead.enc == AES256)
		AES_base64_encrypt(hbb_info.config.aeskey, hbb_info.config.aesivv, (unsigned char *)jsonBuff, (unsigned char *)aes_buff, strlen(jsonBuff));
	else
		memcpy(aes_buff, jsonBuff, strlen(jsonBuff)+1);//include '\0'
	memset(jsonBuff, 0, buffLen);
	sprintf(jsonBuff, protocol_head_format, js->phead.pv, js->phead.enc, js->phead.sig, js->phead.func, js->phead.sts_s,
			js->phead.sts_ms, js->phead.aes->version, aes_buff);
	JsDbPrint("get_dev_list jsonbuff:\r\n%s\r\n",jsonBuff);
}

void json_serialize_dev_list_result(JSON_SEND_DEV_LIST_RESULT *js, char *jsonBuff, int buffLen)
{
	memset(jsonBuff, 0, buffLen);
	//memset(temp_buff, 0, sizeof(temp_buff));
	memset(aes_buff, 0, sizeof(aes_buff));

	sprintf(jsonBuff, send_dev_list_result_body_format, js->send_dev_list_result_body.msgid, js->send_dev_list_result_body.rc);
	JsDbPrint("body:%s\r\n",jsonBuff);
	if(js->phead.enc == AES256)
		AES_base64_encrypt(hbb_info.config.aeskey, hbb_info.config.aesivv, (unsigned char *)jsonBuff, (unsigned char *)aes_buff, strlen(jsonBuff));
	else
		memcpy(aes_buff, jsonBuff, strlen(jsonBuff)+1);//include '\0'
	memset(jsonBuff, 0, buffLen);
	sprintf(jsonBuff, protocol_head_format, js->phead.pv, js->phead.enc, js->phead.sig, js->phead.func, js->phead.sts_s,
			js->phead.sts_ms, js->phead.aes->version, aes_buff);//func is undefine
	JsDbPrint("dev_list_result jsonbuff:\r\n%s\r\n",jsonBuff);
}

/*Deserializes the header of control
 * @param data_ptr the point of json buff
 * @param js the point of value structure
 *@return 0 if parse succeed,otherwise -1
 *
 *
 */
static int get_dev_list_head_deserialize(char *data_ptr, JSON_RECV_DEV_LIST *js)
{
	char *start_ptr;
	start_ptr = (char *)strstr(data_ptr,POV);
	if(start_ptr != NULL)
	{
		start_ptr += strlen(POV);
		sscanf(start_ptr,"%7s",js->phead.pv);
		JsDbPrint("pv: %s\r\n",js->phead.pv);
	}else
		return -1;

	start_ptr = (char *)strstr(data_ptr,SIG);
	if(start_ptr != NULL)
	{
		start_ptr += strlen(SIG);
		sscanf(start_ptr,"%d",&js->phead.sig);
		JsDbPrint("sig: %d\r\n",js->phead.sig);
	}else
		return -1;

	start_ptr = (char *)strstr(data_ptr,ENC);
	if(start_ptr != NULL)
	{
		start_ptr += strlen(ENC);
		sscanf(start_ptr,"%d", &js->phead.enc);
		JsDbPrint("enc: %d\r\n", js->phead.enc);
	}else
		return -1;
	start_ptr = (char *)strstr(data_ptr,FUNC);
	if(start_ptr != NULL)
	{
		start_ptr += strlen(FUNC);
		sscanf(start_ptr,"%d", &js->phead.func);
		JsDbPrint("func: %d\r\n", js->phead.func);
	}else
		return -1;
	start_ptr = (char *)strstr(data_ptr,STS);
	if(start_ptr != NULL)
	{
		start_ptr += strlen(STS);
		sscanf(start_ptr,"%10lu",&js->phead.sts_s);
		JsDbPrint("sts: %lu\r\n", js->phead.sts_s);
	}else
		return -1;
	return 0;
}
/*Deserializes the header of control
 * @param data_ptr the point of json buff
 * @param js the point of value structure
 * @return 0 if parse succeed,otherwise -1
 *
 */
static int get_dev_list_body_deserialize(char *data_ptr, JSON_RECV_DEV_LIST *js)
{
	char *start_ptr;
	start_ptr = (char *)strstr(data_ptr,MSGID);
	if(js->phead.func != FUN_EMERGENCY_FOTA)
	{
		if(start_ptr != NULL)
		{
			start_ptr += strlen(MSGID);
		#ifdef __GNUC__
			if(sscanf(start_ptr,"%[^\"]",js->recv_dev_list_body.msgid) != 1)
				return -1;
			if(strlen(js->recv_dev_list_body.msgid) > 32)
			{
				JsDbPrint("msgid err in json parse!\r\n");
				return -1;
			}
		#else
			char * end_ptr = start_ptr;
			while(*end_ptr != '\"')
				end_ptr++;
			strncpy(out_data->body.msgid,str_start,end_ptr-start_ptr);
			out_data->body.msgid[end_ptr-start_ptr] = '\0';
		#endif
			JsDbPrint("msgid: %s\r\n",js->recv_dev_list_body.msgid);
		}else
			return -1;
	}
	switch(js->phead.func)
	{
		case FUN_DEV_LIST://update dev list
		{
			start_ptr = (char *)strstr(data_ptr,DL);
			if(start_ptr != NULL)
			{
				char *sid_end_ptr;
				start_ptr += strlen(DL);
				sid_end_ptr = strchr(start_ptr,'\"');
				if(parse_sids(start_ptr, sid_end_ptr, &js->recv_dev_list_body.udl.dl) != JSON_PARSE_OK)
					return -1;
			}else
				return -1;
			break;
		}
		case FUN_GET_LOG:
		{
			uint32_t sec = 0;
			start_ptr = (char *)strstr(data_ptr,ST);
			if(start_ptr != NULL)
			{
				start_ptr += strlen(ST);
				if(sscanf(start_ptr,"%10lu",&sec) != 1)
					return -1;
				js->recv_dev_list_body.ghl.st = sec;
				js->recv_dev_list_body.ghl.st *= 1000;
				JsDbPrint("st: %lu S\r\n", sec);
			}else
				return -1;

			start_ptr = (char *)strstr(data_ptr,ET);
			if(start_ptr != NULL)
			{
				start_ptr += strlen(ET);
				if(sscanf(start_ptr,"%10lu",&sec) != 1)
					return -1;
				js->recv_dev_list_body.ghl.et = sec;
				js->recv_dev_list_body.ghl.et *= 1000;
				JsDbPrint("et: %lu S\r\n", sec);
			}else
				return -1;
			break;
		}
		case FUN_GET_DATA:
		{
			start_ptr = (char *)strstr(data_ptr,QDT);
			if(start_ptr != NULL)
			{
				start_ptr += strlen(QDT);
				if(sscanf(start_ptr,"%d",&js->recv_dev_list_body.gd.qdt) != 1)
					return -1;
				JsDbPrint("qdt: %d \r\n", js->recv_dev_list_body.gd.qdt);
			}else
				return -1;

			start_ptr = (char *)strstr(data_ptr,QT);
			if(start_ptr != NULL)
			{
				start_ptr += strlen(QT);
				if(sscanf(start_ptr,"%d",&js->recv_dev_list_body.gd.qt) != 1)
					return -1;
				JsDbPrint("dt: %d \r\n", js->recv_dev_list_body.gd.qt);
			}else
				return -1;

			start_ptr = (char *)strstr(data_ptr,DTL);
			if(start_ptr != NULL)
			{
				start_ptr += strlen(DTL);
				if(parse_dtl(start_ptr, strstr(start_ptr,"\","), &js->recv_dev_list_body.gd) != 1)
					return -1;
			}else
				return -1;
			break;
		}
		case FUN_EMERGENCY_FOTA:
		{
			break;//Body is null
		}
		case FUN_TIME_STAMP://Update time
		{
			start_ptr = (char *)strstr(data_ptr,TS);
			if(start_ptr != NULL)
			{
				start_ptr += strlen(TS);
				sscanf(start_ptr,"%10lu",&js->recv_dev_list_body.ut.ts);
				JsDbPrint("ts: %lu\r\n",js->recv_dev_list_body.ut.ts);
			}else
				return -1;
			break;
		}
		default:
			JsTracePrint("func is undefine\r\n");
	}
	return 0;
}

/* Function parse_dtl
 * The DTL format is "[010010000166666600A0A0A0,010010000166666600A0A0A1]"
 * @param start_ptr is point to '['
 * @param end_ptr is point to ']'
 * @return 0 if parse succeed,otherwise return -1
 */
static int parse_dtl(const char *start_ptr, const char *end_ptr, GET_DATA *gd)
{
	unsigned int dtnum = 0;
	char tempstr[9];
	uint32_t *lid_ptr;
	if(end_ptr == start_ptr)//sid context is null
	{
		gd->num = dtnum;
		return 1;
	}
	if(end_ptr - start_ptr < 24)//if less than 1 sidn
		return -1;
	//start_ptr += 1;//point to first longid
	for(int i=0;i<MAX_SENSOR_COUNT;i++)
	{
		lid_ptr = &gd->dtl[i].idh;
		for(int j=0;j<=2;j++)
		{
			strncpy(tempstr,start_ptr,8);
			tempstr[8] = '\0';
			if(sscanf(tempstr, "%x",&lid_ptr[j]) != 1)
				return -1;
			start_ptr += 8;
		}

		dtnum++;
		start_ptr = strchr(start_ptr,',');
		if((start_ptr == NULL) || (start_ptr >= end_ptr))
			break;
		start_ptr +=1;
	}

	gd->num = dtnum;
	return 1;
}

int json_deserialize_msg_recv(char *jsonBuff, int buffLen, JSON_RECV_DEV_LIST *js)
{
	char *data_ptr = jsonBuff;
	char *start_ptr;
	data_ptr[buffLen] = '\0';
	if(get_dev_list_head_deserialize(data_ptr, js) != 0)
		return JSON_PARSE_ERR;
	if(js->phead.enc == AES256)
	{
		int body_length;
		start_ptr = strstr(data_ptr,BODY);
		if(start_ptr == NULL)
			return JSON_PARSE_ERR;
		start_ptr += strlen(BODY);
		if((body_length = strstr(start_ptr,END) - start_ptr) <= 0)
			return JSON_PARSE_ERR;
		start_ptr[body_length] = '\0';
#if 0
		JsDbPrint("encrypted:%s\r\n",start_ptr);
#endif
		int len = AES_base64_decrypt(hbb_info.config.aeskey, hbb_info.config.aesivv, (unsigned char *)start_ptr, (unsigned char *)aes_buff, strlen(start_ptr));
		data_ptr = aes_buff;
		data_ptr[len] = '\0';
		JsDbPrint("aes:%s\r\n",data_ptr);
	}
	if(get_dev_list_body_deserialize(data_ptr, js) != 0)
		return JSON_PARSE_ERR;
	return JSON_PARSE_OK;
}


