#define PR_MODULE "http\1"

#include "http_client.h"
#include "uart_ppp.h"
#include "rtc_fun.h"
#include "log_task.h"

char send_rev_buf[SEND_BUF_SIZE];

static int8_t http_connect_analyse(int8_t connect_code);

typedef struct
{
	char month_math[4];
	uint8_t month_num;
}MONTH;

MONTH month[12] = {
		{"Jan",1},
		{"Feb",2},
		{"Mar",3},
		{"Apr",4},
		{"May",5},
		{"Jun",6},
		{"Jul",7},
		{"Aug",8},
		{"Sep",9},
		{"Oct",10},
		{"Nov",11},
		{"Dec",12},
};

char *post_req_format = "%s %s HTTP/1.1\r\n"
		        "Host: %s\r\n"
			    "Connection: keep-alive\r\n"
			    "Content-length: %d\r\n"
			    "Content-Type: application/x-www-form-urlencoded\r\n"
			    "Accept: */*\r\n"
    	        "Accept-Encoding: gzip, deflate\r\n"
				"Accept-Language: zh-CN,zh;q=0.8\r\n\r\n";
			//	"%s";

int8_t dns_parse_3times(const char *name, ip_addr_t *addr)
{
	int res,loop_num = 3;
	for(int i=1;i<=loop_num;i++)
	{
		res = netconn_gethostbyname(name,addr);
		if(res == ERR_OK)
			return res;
		else if ((res != ERR_OK) && (i >= loop_num)){
			ERR("DNS erro %d! \r\n",res);
			return res;
		}
		else
			ERR("DNS erro %d! retry.....\r\n",res);
	}
	return res;
}


int8_t month_string_to_num(char *str)
{
	for(int i=0;i<12;i++)
	{
		if(strncasecmp(str,month[i].month_math,3) == 0){
			return month[i].month_num;
		}
	}
	WARN("can't match Date....\r\n");
	return false;
}

bool http_check_date_format(const HTTP_DATE *datetime)
{
    /* Table of days in a month for a non leap year. First entry in the table is not used,
     * valid months start from 1
     */
    uint8_t daysPerMonth[] = {0U, 31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};

    /* Check year, month, hour, minute, seconds */
    if ((datetime->year < 1970U) || (datetime->year > 2099U) || (datetime->month > 12U) ||
        (datetime->month < 1U) || (datetime->hour >= 24U) || (datetime->minute >= 60U) || (datetime->second >= 60U))
    {
        /* If not correct then error*/
        return false;
    }

    /* Adjust the days in February for a leap year */
    if (!(datetime->year & 3U))
    {
        daysPerMonth[2] = 29U;
    }

    /* Check the validity of the day */
    if (datetime->day > daysPerMonth[datetime->month])
    {
        return false;
    }

    return true;
}

uint32_t http_date_to_seconds(const HTTP_DATE *datetime)
{
    /* Number of days from begin of the non Leap-year*/
    uint16_t monthDays[] = {0U, 0U, 31U, 59U, 90U, 120U, 151U, 181U, 212U, 243U, 273U, 304U, 334U};
    uint32_t seconds;

    /* Compute number of days from 1970 till given year*/
    seconds = (datetime->year - 1970U) * 365U;
    /* Add leap year days */
    seconds += ((datetime->year / 4) - (1970U / 4));
    /* Add number of days till given month*/
    seconds += monthDays[datetime->month];
    /* Add days in given month. We subtract the current day as it is
     * represented in the hours, minutes and seconds field*/
    seconds += (datetime->day - 1);
    /* For leap year if month less than or equal to Febraury, decrement day counter*/
    if ((!(datetime->year & 3U)) && (datetime->month <= 2U))
    {
        seconds--;
    }

    seconds = (seconds * 86400U) + (datetime->hour * 3600U) +
              (datetime->minute * 60U) + datetime->second;

    return seconds;
}

//e.g.Date: Thu, 27 Oct 2016 06:47:31 GMT
bool http_match_date_second(const char * date_str, uint32_t *second)
{
	char month_str[4];
	int day;
	HTTP_DATE date_struct;
	sscanf(date_str,"Date: %*3s, %d %3s %d %d:%d:%d GMT", &day, month_str,
			&date_struct.year, &date_struct.hour, &date_struct.minute, &date_struct.second);
	date_struct.month = month_string_to_num(month_str);
	date_struct.day = (uint8_t)day;
	if(http_check_date_format(&date_struct) == false){
		PRINTF("get date error..\r\n");
		return false;
	}
	*second = http_date_to_seconds(&date_struct);
	return true;
}

int8_t HTTPClient_post( HTTP_CLIENT_POST_STRUCT * http_struct) //Blocking
{
  int8_t res;
  http_struct->method = HTTP_POST;
  http_struct->recv_buf_ptr = NULL;
  http_struct->timeout = 40000;
  http_struct->result_code = 0XFFFFFFFF;
  for(int i=0;i<3;i++)
  {

	res = HTTPClient_connect(http_struct);
	if(res == HTTP_CONN_OK)
		return HTTP_SUCC;
	if(res == HTTP_CL && http_struct->purpose == SEND_EVENT)
		return HTTP_SUCC;

	if(http_connect_analyse(res) != HTTP_CONN_RETRY)
		 return HTTP_FAIL;//Add url error and roll back
	PRINTF("try again...\r\n");
  }

  return HTTP_FAIL;
}
static int8_t http_connect_analyse(int8_t connect_code)
{
	switch(connect_code)
	{
	case HTTP_CL:
		PRINTF("Can't match content len.\r\n");
		return HTTP_CONN_RETRY;
	case HTTP_PARSE:
		PR_ERROR("http url parse err.\r\n", HTTP_PARSE);
		return HTTP_CONN_URL;
	case HTTP_DNS:
		PRINTF("dns resolve err.\r\n");
		return HTTP_CONN_RETRY;
	case HTTP_NEW:
		PRINTF("netconn_new err.\r\n");
		return HTTP_CONN_RETRY;
	case HTTP_RC:
		PRINTF("can't match http response code.\r\n");
		return HTTP_CONN_RETRY;
		break;
	case HTTP_CONTENT:
		PRINTF("can't find http content.\r\n");
		return HTTP_CONN_RETRY;
	case HTTP_IF:
		PRINTF("Low-level netif error.\r\n");
		return HTTP_CONN_RETRY;
	case HTTP_ATG:
		PRINTF("Illegal argument. \r\n");
		return HTTP_CONN_RETRY;
	case HTTP_CONN:
		PR_ERROR("Not connected.\r\n", HTTP_CONN);
		return HTTP_CONN_RETRY;
	case HTTP_CLSD:
		PR_ERROR("Connection closed.\r\n", HTTP_CLSD);
		return HTTP_CONN_RETRY;
	case HTTP_RST:
		PR_ERROR("Connection reset.\r\n", HTTP_RST);
		return HTTP_CONN_RETRY;
	case HTTP_ABRT:
		PR_ERROR("Connection aborted.\r\n", HTTP_ABRT);
		return HTTP_CONN_RETRY;
	case HTTP_ISCONN:
		PRINTF("Already connected.\r\n");
		return HTTP_CONN_RETRY;
	case HTTP_USE:
		PRINTF("Address in use.\r\n");
		return HTTP_CONN_RETRY;
	case HTTP_WOULDBLOCK:
		PRINTF("Operation would block.\r\n");
		return HTTP_CONN_RETRY;
	case HTTP_VAL:
		PRINTF("Illegal value.\r\n");
		return HTTP_CONN_RETRY;
	case HTTP_INPROGRESS:
		PRINTF("Operation in progress.\r\n");
		return HTTP_CONN_RETRY;
	case HTTP_RTE:
		PRINTF("Routing problem.\r\n");
		return HTTP_CONN_RETURN_FAIL;
	case HTTP_TIMEOUT:
		PR_ERROR("Timeout.\r\n", HTTP_TIMEOUT);
		return HTTP_CONN_RETRY;
	case HTTP_BUF:
		PRINTF("Buffer error.\r\n");
		return HTTP_CONN_RETRY;
	case HTTP_MEM:
		PRINTF("Out of memory error.\r\n");
		NVIC_SystemReset();
		break;
	case HTTP_LEN:
		PRINTF("The http content length is more then recieve buf size.\r\n");
		return HTTP_CONN_RETRY;
	case HTTP_LEN_UNEQUAL:
		PRINTF("The actual receiving length is unequal to content length.\r\n");
		return HTTP_CONN_RETRY;
#ifdef CPU_MK24FN256VDC12
	case HTTP_PPP:
		PRINTF("PPP link error...\r\n");
		return HTTP_CONN_RETRY;
#endif
	default:
		PRINTF("http return undefine  http rc: %d\r\n",connect_code);
		return HTTP_CONN_RETURN_FAIL;
	}
	return 0;
}

#ifndef WIFI_MODULE
int8_t HTTPClient_connect( HTTP_CLIENT_POST_STRUCT * http_struct)
{

	struct netconn *conn;
	char scheme[8];
	uint16_t port;
	char host[64];
    char path[64];
    /*parser url*/
    //First we need to parse the url (http[s]://host[:port][/[path]]) -- HTTPS not supported (yet?)
    int8_t res = HTTPClient_parseURL(http_struct->url_ptr, scheme, sizeof(scheme), host, sizeof(host),
    						       &port, path, sizeof(path));
    if(res != HTTP_CONN_OK)
    {
      ERR("url error : %s \r\n", http_struct->url_ptr);
      return res;
    }

    if(port == 0)
    {
      port = 80;
    }

    DBG("Scheme: %s\r\n", scheme);
    DBG("Host: %s\r\n", host);
    DBG("Port: %d\r\n", port);
    DBG("Path: %s\r\n", path);
#ifdef CPU_MK24FN256VDC12
    res = ppp_check_status();
    if(res != ERR_OK)
    	return HTTP_PPP;
#endif
    /***get the remote ip according host from DNS server******/
    ip_addr_t addr;
    res = dns_parse_3times(host,&addr);
    if(res != ERR_OK)
    	return res;
    if(addr.addr == 0)
    {
    	WARN("ip is error\r\n");
    	return ERR_VAL;
    }

    DBG("dns is success!\r\n");
    DBG("IP: %X\r\n",addr.addr);

    /*get the meth*/
    char* meth = (http_struct->method==HTTP_GET)?"GET":(http_struct->method==HTTP_POST)?"POST":
    			 (http_struct->method==HTTP_PUT)?"PUT":(http_struct->method==HTTP_DELETE)?"DELETE":"";

    memset(send_rev_buf,0,sizeof(send_rev_buf));

    /*write data to send_rev_buf*/
    sprintf(send_rev_buf, post_req_format, meth, path, host, http_struct->send_buf_length);
    uint16_t http_send_totle_lentgh = strlen(send_rev_buf);
    memcpy(strstr(send_rev_buf,"\r\n\r\n")+4, http_struct->send_buf_ptr, http_struct->send_buf_length);//+4 is point to request text
    http_send_totle_lentgh += http_struct->send_buf_length;
#if 0
    DBG("send:\r\n%s",send_rev_buf);
#endif
    /* Create a new TCP connection handle */
    conn = netconn_new(NETCONN_TCP);
    if(conn == NULL)
    {
    	ERR("netconn new return null point!\r\n");
        return HTTP_NEW;
    }

   /*connection*/
    res = netconn_connect(conn,&addr,port);
    if(res != ERR_OK)
    {
    	ERR("connect erro %d! \r\n",res);
    	//if(res != ERR_ABRT)
    	netconn_close(conn);
        netconn_delete(conn);
       	return res;
    }

    /*send data*/
    DBG("send %d byte\r\n",http_send_totle_lentgh);
    conn->send_timeout = http_struct->timeout;
    res = netconn_write(conn, send_rev_buf, http_send_totle_lentgh, NETCONN_NOCOPY);

    if(res != ERR_OK)
    {
    	ERR("send erro %d! \r\n",res);
        netconn_close(conn);
        netconn_delete(conn);
        return res;
    }

    DBG("send success!\r\n");

    struct netbuf *inbuf;
    char *buf;
    uint16_t buflen;
	uint32_t all_content_len = 0, already_rev_len;
	char *current_rev_ptr, *content_star_ptr, *key_ptr;
	//uint32_t http_second;

	conn->recv_timeout = http_struct->timeout;//set the recieve timeout time
	res = netconn_recv(conn, &inbuf);
	if(res != ERR_OK)
	{
		ERR("receiving error %d! \r\n",res);
		ERR_HANDLE();
		return res;
	}
	netbuf_data(inbuf, (void **)&buf, &buflen);
	buf[buflen] = '\0';//add the ending of string
#if 0
	DBG("receive len = %d:\r\n%s\r\n",buflen,buf);
#endif

	//key_ptr = strstr(buf,"Date: ");
	//if(http_match_date_second(key_ptr,&http_second) == true)
	//	set_rtc_date(http_second);
	//else
	//	WARN("can't match http date.....\r\n");


#ifdef __GNUC__
	if( sscanf(buf, "HTTP/%*d.%*d %lu", &(http_struct->result_code)) != 1 ) //Kludge for newlib nano
#else
	if( sscanf(buf, /*"HTTP/%*d.%*d %d"*/"HTTP/1.1 %d", &(http_struct->result_code)) != 1 ) //Kludge for newlib nano
#endif
    {
	    //Cannot match string, error
		ERR("can't match ResponseCode!\r\n");
		ERR_HANDLE();
		return HTTP_RC;
    }
	DBG("http result code: %d\r\n",http_struct->result_code);
	key_ptr = strstr(buf,"Content-Length:");
	if(sscanf(key_ptr, "Content-Length: %lu", &all_content_len) !=1)
	{
		WARN("warning: can't match content-length!\r\n");
		ERR_HANDLE();
		return HTTP_CL;
	}

	if(all_content_len > SEND_BUF_SIZE)
	{
		WARN("The http content length is more then %d byte!\r\n",SEND_BUF_SIZE);
		ERR_HANDLE();
		return HTTP_LEN;
	}
	content_star_ptr = strstr(buf, "\r\n\r\n");
	if(content_star_ptr == NULL)
	{
		ERR("can't find content !\r\n");
		ERR_HANDLE();
		return HTTP_CONTENT;
	}

	content_star_ptr += 4;
	already_rev_len = buflen - (content_star_ptr - buf);
	memset(send_rev_buf,0,sizeof(send_rev_buf));
	current_rev_ptr = send_rev_buf;
	memcpy(current_rev_ptr,content_star_ptr,already_rev_len);
	current_rev_ptr += already_rev_len;
	netbuf_delete(inbuf);

	//DBG("already_rev_len: %d\r\n all_content_len: %d\r\n",already_rev_len,all_content_len);
	for(int i=0;already_rev_len<all_content_len;i++)
	{
		res = netconn_recv(conn, &inbuf);
		if(res != ERR_OK)
		{
			ERR("receiving error %d! \r\n",res);
			ERR_HANDLE();
			return res;
		}
		 //read data from receive buff
		netbuf_data(inbuf, (void **)&buf, &buflen);
		memcpy(current_rev_ptr,buf,buflen);

		buf[buflen] = '\0';//add the ending of string

		current_rev_ptr += buflen;
		already_rev_len += buflen;
		//DBG("i:%d\r\nbuflen:%d\r\n already_rev_len:%d\r\n",i,buflen,already_rev_len);
		netbuf_delete(inbuf);
	}

	if(all_content_len == already_rev_len)
	{
		http_struct->recv_buf_ptr = send_rev_buf;
		http_struct->recv_buf_length = already_rev_len;
	}else{
		ERR("receive error content length :%d!= %d\r\n",all_content_len, already_rev_len);
		http_struct->recv_buf_ptr = send_rev_buf;
		//http_struct->recv_buf_length = content_len;
		http_struct->recv_buf_length = already_rev_len;
		netconn_close(conn);
		netconn_delete(conn);
		return HTTP_LEN_UNEQUAL;
	}

	netconn_close(conn);
	netconn_delete(conn);
	return HTTP_CONN_OK;

}
#else
int8_t HTTPClient_connect( HTTP_CLIENT_POST_STRUCT * http_struct)
{

	struct netconn *conn;
	char scheme[8];
	uint16_t port;
	char host[64];
    char path[64];
    char wifi[80];
    /*parser url*/
    //First we need to parse the url (http[s]://host[:port][/[path]]) -- HTTPS not supported (yet?)
    int8_t res = HTTPClient_parseURL(http_struct->url_ptr, scheme, sizeof(scheme), host, sizeof(host),
    						       &port, path, sizeof(path));
    if(res != HTTP_CONN_OK)
    {
      ERR("url error : %s \r\n", http_struct->url_ptr);
      return res;
    }

    if(port == 0)
    {
      port = 80;
    }

    DBG("Scheme: %s\r\n", scheme);
    DBG("Host: %s\r\n", host);
    DBG("Port: %d\r\n", port);
    DBG("Path: %s\r\n", path);

    /***get the remote ip according host from DNS server******/

    /*get the meth*/
    char* meth = (http_struct->method==HTTP_GET)?"GET":(http_struct->method==HTTP_POST)?"POST":
    			 (http_struct->method==HTTP_PUT)?"PUT":(http_struct->method==HTTP_DELETE)?"DELETE":"";

    memset(send_rev_buf,0,sizeof(send_rev_buf));

    /*write data to send_rev_buf*/
    sprintf(send_rev_buf, post_req_format, meth, path, host, http_struct->send_buf_length);
    uint16_t http_send_totle_lentgh = strlen(send_rev_buf);
    memcpy(strstr(send_rev_buf,"\r\n\r\n")+4, http_struct->send_buf_ptr, http_struct->send_buf_length);//+4 is point to request text
    http_send_totle_lentgh += http_struct->send_buf_length;
#if 0
    DBG("send:\r\n%s",send_rev_buf);
#endif
    /* Create a new TCP connection handle */
    sprintf(wifi,"http://%s:%d/", host, port);


    if(wifi_connect_to_server(wifi, http_struct->timeout) == RET_FAIL)
    {
    	DBG("wifi connect error!\r\n");

    	return ERR_ABRT;
    }

    DBG("wifi connect success!\r\n");

    /*send data*/
    DBG("send %d byte\r\n",http_send_totle_lentgh);
    if(wifi_netconn_write(send_rev_buf, http_send_totle_lentgh, UART_SEND_DATA, http_struct->timeout) == RET_FAIL)
    {
    	ERR("wifi send timeout...\r\n");
    	wifi_netconn_close(http_struct->timeout);
    	return ERR_TIMEOUT;
    }

    DBG("send success!\r\n");


   // struct netbuf *inbuf;
    char *buf;
    uint16_t buflen;
	uint32_t all_content_len = 0, already_rev_len;
	char *current_rev_ptr, *content_star_ptr, *key_ptr;
	//uint32_t http_second;

	if(wifi_netconn_recv(&buf, &buflen, http_struct->timeout) == RET_FAIL)
	{
		ERR("receiving timeout... \r\n");
		wifi_netconn_close(http_struct->timeout);
		return ERR_TIMEOUT;
	}
	//netbuf_data(inbuf, (void **)&buf, &buflen);
	buf[buflen] = '\0';//add the ending of string
#if 1
	//DBG("receive len = %d:\r\n%s\r\n",buflen,buf);
	DBG("http receive len = %d------------------------\r\n\r\n",buflen);
#endif


	//key_ptr = strstr(buf,"Date: ");
	//if(http_match_date_second(key_ptr,&http_second) == true)
	//	set_rtc_date(http_second);
	//else
	//	WARN("can't match http date.....\r\n");


#ifdef __GNUC__
	if( sscanf(buf, "HTTP/%*d.%*d %lu", &(http_struct->result_code)) != 1 ) //Kludge for newlib nano
#else
	if( sscanf(buf, /*"HTTP/%*d.%*d %d"*/"HTTP/1.1 %d", &(http_struct->result_code)) != 1 ) //Kludge for newlib nano
#endif
    {
	    //Cannot match string, error
		ERR("can't match ResponseCode!\r\n");
		wifi_netconn_close(http_struct->timeout);
		return HTTP_RC;
    }

	key_ptr = strstr(buf,"Content-Length:");
	if(sscanf(key_ptr, "Content-Length: %lu", &all_content_len) !=1)
	{
		WARN("warning: can't match content-length!\r\n");
		wifi_netconn_close(http_struct->timeout);
		return HTTP_CL;
	}

	if(all_content_len > SEND_BUF_SIZE)
	{
		WARN("The http content length is more then %d byte!\r\n",SEND_BUF_SIZE);
		wifi_netconn_close(http_struct->timeout);
		return HTTP_LEN;
	}
	content_star_ptr = strstr(buf, "\r\n\r\n");
	if(content_star_ptr == NULL)
	{
		ERR("can't find content !\r\n");
		wifi_netconn_close(http_struct->timeout);
		return HTTP_CONTENT;
	}

	content_star_ptr += 4;
	already_rev_len = buflen - (content_star_ptr - buf);
	memset(send_rev_buf,0,sizeof(send_rev_buf));
	current_rev_ptr = send_rev_buf;
	memcpy(current_rev_ptr,content_star_ptr,already_rev_len);
	current_rev_ptr += already_rev_len;

	//DBG("already_rev_len: %d\r\n all_content_len: %d\r\n",already_rev_len,all_content_len);
	for(int i=0;already_rev_len<all_content_len;i++)
	{
		if(wifi_netconn_recv(&buf, &buflen, http_struct->timeout) == RET_FAIL)
		{
			ERR("receiving timeout... \r\n");
			wifi_netconn_close(http_struct->timeout);
			return ERR_TIMEOUT;
		}
		DBG("http receive len = %d------------------------\r\n\r\n",buflen);
		 //read data from receive buff
		memcpy(current_rev_ptr,buf,buflen);

		buf[buflen] = '\0';//add the ending of string

		current_rev_ptr += buflen;
		already_rev_len += buflen;
		//DBG("i:%d\r\nbuflen:%d\r\n already_rev_len:%d\r\n",i,buflen,already_rev_len);
	}

	if(all_content_len == already_rev_len)
	{
		http_struct->recv_buf_ptr = send_rev_buf;
		http_struct->recv_buf_length = already_rev_len;
	}else{
		ERR("receive error content length :%d!= %d\r\n",all_content_len, already_rev_len);
		http_struct->recv_buf_ptr = send_rev_buf;
		//http_struct->recv_buf_length = content_len;
		http_struct->recv_buf_length = already_rev_len;
		wifi_netconn_close(http_struct->timeout);
		return HTTP_LEN_UNEQUAL;
	}
	wifi_netconn_close(http_struct->timeout);
	return HTTP_CONN_OK;

}
#endif


// url e.g :http://www.guet.edu.cn:80/page/
int8_t HTTPClient_parseURL( char* url, char* scheme, uint8_t maxSchemeLen,
									 char* host, uint8_t maxHostLen, uint16_t* port,
									 char* path, uint8_t maxPathLen) //Parse URL
{
	  char* schemePtr = (char*) url;
	  char* hostPtr = (char*) strstr(url, "://");
	  if(hostPtr == NULL)
	  {
	    WARN("Could not find host");
	    return HTTP_PARSE; //URL is invalid
	  }
	  /*get Scheme*/
	  if( maxSchemeLen < hostPtr - schemePtr + 1 ) //including NULL-terminating char
	  {
	    WARN("Scheme buf is too small (%d <= %d)", maxSchemeLen, hostPtr - schemePtr + 1);
	    return HTTP_PARSE;
	  }
	  memcpy(scheme, schemePtr, hostPtr - schemePtr);
	  scheme[hostPtr - schemePtr] = '\0';//add the ending of string
	  if((strcmp(scheme,"http") != 0) && (strcmp(scheme,"https") != 0))
		  return HTTP_PARSE;
	  /*get the host and port*/
	  hostPtr+=3;//point to host
	  uint8_t hostLen = 0;
	  char* portPtr = strchr(hostPtr, ':');
	  if( portPtr != NULL )
	  {
	    hostLen = portPtr - hostPtr;
	    portPtr++;
	    //get the port
	    if( sscanf(portPtr, "%hu", port) != 1)
	    {
	      WARN("Could not find port");
	      return HTTP_PARSE;
	    }
	  }
	  else //port have no set
	  {
	    *port=0;
	  }
	  char* pathPtr = strchr(hostPtr, '/');
	  if( hostLen == 0 )  //if dont have set port
	  {
	    hostLen = pathPtr - hostPtr;
	  }
	  //get the host
	  if( maxHostLen < hostLen + 1 ) //including NULL-terminating char
	  {
	    WARN("Host buf is too small (%d =< %d)", maxHostLen, hostLen + 1);
	    return HTTP_PARSE;
	  }
	  memcpy(host, hostPtr, hostLen);
	  host[hostLen] = '\0';
      /* get the path */
	  size_t pathLen;
	  char* fragmentPtr = strchr(hostPtr, '#');
	  if(fragmentPtr != NULL)
	  {
	    pathLen = fragmentPtr - pathPtr;
	  }
	  else
	  {
	    pathLen = strlen(pathPtr);
	  }
	  if( maxPathLen < pathLen + 1 ) //including NULL-terminating char
	  {
	    WARN("Path str is too small (%d >= %d)", maxPathLen, pathLen + 1);
	    return HTTP_PARSE;
	  }
	  memcpy(path, pathPtr, pathLen);
	  path[pathLen] = '\0';

	  return HTTP_CONN_OK;
}



