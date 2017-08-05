/****************************************************************************
*main.c
****************************************************************************/
#ifndef PPP_IF
#define PPP_IF
#define PR_MODULE "ppp\1"


#include "fsl_port.h"
#include "pppif.h"
#include "ppp/ppp.h"
#include "lwip/init.h"
#include "board.h"
#include "uart_ppp.h"
#include "log_task.h"

const CHAR *at_cmd_at = "AT\r";
const CHAR *at_cmd_ata = "ATA\r";
const CHAR *at_cmd_at_c = "AT&C?\r";
const CHAR *at_cmd_gtset_gprs = "AT+GTSET=\"GPRSFIRST\",0\r";
const CHAR *at_cmd_gtset_call = "AT+GTSET=\"CALLBREAK\",0\r";
const CHAR *at_cmd_cgatt = "AT+CGATT=1\r";
const CHAR *at_cmd_ccid= "AT+CCID?\r";
const CHAR *at_cmd_cgreg = "AT+CGREG?\r";
const CHAR *at_cmd_cgatt_down = "AT+CGATT=0\r";
const CHAR *at_cmd_creg = "AT+CREG?\r";
const CHAR *at_cmd_csq = "AT+CSQ\r";
const CHAR *at_cmd_plus = "+++";
const CHAR *at_cmd_ato = "ATO\r";
const CHAR *at_cmd_sipconfig = "AT+SIPCONFIG?\r";
const CHAR *at_cmd_atd = "ATD112\r";
const CHAR *at_cmd_ath = "ATH\r";
const CHAR *at_cmd_chup = "AT+CHUP\r";
const CHAR *at_cmd_remomft = "AT+REMOMFT=6\r";  
const CHAR *at_cmd_dial_gprs = "ATD*99***1#\r";
//const CHAR *at_cmd_cpms = "AT+CPMS=\"ME\",\"ME\",\"ME\"\r";
const CHAR *at_cmd_cpms = "AT+CPMS=\"SM\"\r";
const CHAR *at_cmd_cfun = "AT+CFUN=15\r";
const CHAR *at_cmd_cmgf = "AT+CMGF=1\r";
const CHAR *at_cmd_cmgl = "AT+CMGL=\"ALL\"\r";
const CHAR *at_cmd_csmp = "AT+CSMP=19,143,0,2\r";
const CHAR *at_cmd_mmicg = "AT+MMICG=8\r";
const CHAR *at_cmd_clvl = "AT+CLVL=2\r";
const CHAR *at_cmd_clcc = "AT+CLCC\r";
const CHAR *at_cmd_cmee = "AT+CMEE=2\r";
const CHAR *at_cmd_cnmi = "AT+CNMI=1,1,0,0,0\r";
const CHAR *at_cmd_cmgd_all = "AT+CMGD=1,4\r";
const CHAR *at_cmd_gtset_iprfmt = "AT+GTSET=\"IPRFMT\",5\r";

CHAR at_cmd_cmgd[16] = "AT+CMGD=1\r";
CHAR at_cmd_cmgr[] = "AT+CMGR=X\r";
CHAR at_cmd_sl_sms[16] = "AT+CMGR=";
CHAR at_cmd_cgdcont[32] = "AT+CGDCONT=1,\"IP\",\"3gnet\"\r";
CHAR at_cmd_sos_call[40] = "ATD112\r";
CHAR at_cmd_mipcall[32] = "AT+MIPCALL=1,\"CMNET\"\r";
CHAR at_cmd_mipcall_down[32] = "AT+MIPCALL=0\r";
CHAR at_cmd_mipopen[96] = "AT+MIPOPEN=1,8964,\"officelinux.vicp.net\",15590,0\r";
CHAR at_cmd_mipclose[32] = "AT+MIPCLOSE=1\r";
CHAR at_cmd_mipdns[64] = "AT+MIPDNS=\"officelinux.vicp.net\"\r";
CHAR at_cmd_mipsend[2048] = "AT+MIPSEND=%d,\"88\"\r";
CHAR at_cmd_mippush[32] = "AT+MIPPUSH=1\r";
CHAR at_cmd_mipread[32] = "AT+MIPREAD=1,1024";

const CHAR *at_cmd_at_result = "OK\r\n";
const CHAR *at_cmd_ata_result = "OK\r\n";
const CHAR *at_cmd_cgatt_result = "OK\r\n";
const CHAR *at_cmd_ccid_result = "CCID: ";
const CHAR *at_cmd_cgreg_result = "CGREG: 0,1\r\n";
const CHAR *at_cmd_cgatt_down_result = "CGREG: 0,1\r\n";
const CHAR *at_cmd_creg_result = "CREG: 0,1\r\n";
const CHAR *at_cmd_csq_result = "CSQ: ";
const CHAR *at_cmd_plus_result = "+++";
const CHAR *at_cmd_sipconfig_result = "OK\r\n";
const CHAR *at_cmd_atd_result = "OK\r\n";
const CHAR *at_cmd_ath_result = "OK\r\n";
const CHAR *at_cmd_chup_result = "OK\r\n";
//const CHAR *at_cmd_ato_result = "OK\r\n";
const CHAR *at_cmd_ato_result = "CONNECT\r\n";
const CHAR *at_cmd_remomft_result = "OK\r\n"; 
const CHAR *at_cmd_dial_gprs_result = "CONNECT\r\n";
const CHAR *at_cmd_cpms_result = "CPMS: ";
const CHAR *at_cmd_cmgf_result = "OK\r\n";
const CHAR *at_cmd_cmgl_result = "CMGL: ";
const CHAR *at_cmd_cmgr_result = "CMGR: ";
const CHAR *at_cmd_cmgd_result = "OK\r\n";
const CHAR *at_cmd_cfun_result = "OK\r\n";
const CHAR *at_cmd_mmicg_result = "OK\r\n";
const CHAR *at_cmd_clcc_result = "+CLCC:";
const CHAR *at_cmd_clvl_result = "OK\r\n";
const CHAR *at_check_modem_status_result = "SIM READY";
const CHAR *at_cmd_sos_call_result = "CONN: 1,0";
const CHAR *at_cmd_sos_end_result = "NO CARRIER";
const CHAR *at_check_sl_sms_apn_info_ascii = "73616E646C6163757311534D531161706E3A";
const CHAR *at_check_sl_sms_sos_info_ascii = "73616E646C6163757311534D5311736F733A";
const CHAR *at_check_sl_sms_apn_info = "sandlacus_SMS_apn:";
const CHAR *at_check_sl_sms_sos_info = "sandlacus_SMS_sos:";
const CHAR *at_cmd_cgdcont_result = "OK\r\n";
const CHAR *at_cmd_mipcall_result = "+MIPCALL: ";
const CHAR *at_cmd_mipcall_down_result = "+MIPCALL: 0";
const CHAR *at_cmd_mipopen_result = "+MIPOPEN: ";
const CHAR *at_cmd_mipclose_result = "+MIPCLOSE: ";
CHAR at_cmd_mipdns_result[64] = "+MIPDNS: ";
const CHAR *at_cmd_mipsend_result = "+MIPSEND:";
const CHAR *at_cmd_mipread_result = "+MIPDATA:";
//CHAR at_cmd_mipread_result[24] = "+MIPDATA: 1,1";
CHAR at_cmd_mippush_result[16] = "+MIPPUSH: 1,";
const CHAR *at_cmd_miprtcp_result = "+MIPRTCP: ";

LWIP_LINK_STATE lwip_link_state = PPP_NEGOTIATE;

extern uart_rtos_handle_t ppp_handle;
extern ppp_gprs_info_conf ppp_gprs_info;
extern ppp_gprs_apn_pap_info_conf ppp_gprs_apn_pap_info;
extern u32_t sim_status;
extern char dns_ip_addr[16];
u8_t at_recv;

/*
*return the state of my tcpip
*/
LWIP_LINK_STATE get_lwip_link_state(void)
{
	return lwip_link_state;
}

/*
*hao.yunran
*2016.10.10
*set the state of my tcpip
*/
void set_lwip_link_state(LWIP_LINK_STATE state)
{
    lwip_link_state = state;
}

/*
*hao.yunran
*2016.10.10
*set the state of my tcpip
*/
at_ret wait_at_cmd_return(uart_rtos_handle_t *handle, const CHAR *str, u32_t cmd_result_len, u32_t msecond, u32_t at_type, u32_t buf_len, u32_t *return_value, CHAR *buf, u32_t *ret_len)
{
	u32_t time = xTaskGetTickCount();//get_sys_tick();
	u32_t m=0;
	u32_t cmd_result_len_tmp = cmd_result_len;
	u32_t rec_flag=0;
	u32_t len_read=0;
	u32_t len_count[5]={0,0,10,100,1000};
	u32_t i=0;
	u32_t j=0;
	CHAR buf_tmp[10];
	const CHAR *tmp_str = str;
	at_ret ret = AT_RET_FAIL;
	
	switch(at_type)
	{
		case AT_CMD_AT:
		case AT_CMD_INIT_GPRS:
		case AT_CMD_DIAL_GPRS:
		case AT_CMD_ATD:
			while((xTaskGetTickCount() - time) < msecond)
			{
				//if(GET_AT(handle, &ch, &j) == kStatus_Success)
				if(GET_AT(handle, &at_recv, &j))
				{		
					/*waiting for the string that we expected*/
					if(*tmp_str  == at_recv)
					{
						tmp_str++;		  
						if(*tmp_str  == '\0')
						{
							ret = AT_RET_OK;
							break;
						}
					}		 
					else
					{	  
						tmp_str = str;
					}	
				}
				else
				{
					vTaskDelay(20);
				}
			}	
			break;

		case AT_CMD_CCID:
		case AT_CMD_CGREG:
		case AT_CMD_CREG:
		case AT_CMD_CHECK_MODEM_STAT:
			while((xTaskGetTickCount() - time) < msecond)
			{
				//if(GET_AT(handle, &ch, &j) == kStatus_Success)
				if(GET_AT(handle, &at_recv, &j))
				{		
					/*waiting for the string that we expected*/
					if(*tmp_str  == at_recv)
					{
						tmp_str++;	
						m++;
						cmd_result_len_tmp--;
						if(cmd_result_len_tmp  == 0x0)
						{		
							ret =  AT_RET_OK;
							break;
						}
					}		 
					else
					{	  
						tmp_str = str;
						cmd_result_len_tmp=cmd_result_len;
						m=0;
					}	
				}	 
				else
				{
					vTaskDelay(20);
				}
			}			
			break;
		case AT_CMD_CPMS:
		case AT_CMD_CMGF:
		case AT_CMD_CMGL:
		case AT_CMD_CMGR: 
		case AT_CMD_APN:
		case AT_CMD_SOS:  
		case AT_CMD_CSQ:
			while((xTaskGetTickCount() - time) < msecond)
			{
				//if(GET_AT(handle, &ch, &j) == kStatus_Success)
				if(GET_AT(handle, &at_recv, &j))
				{		
					/*waiting for the string that we expected*/
					if(*tmp_str  == at_recv)
					{
						tmp_str++;	
						m++;
						cmd_result_len_tmp--;
						if(cmd_result_len_tmp  == 0x0)
						{	
							if(AT_CMD_CSQ == at_type)
							{
								GET_AT(handle, &at_recv, &j);
								*return_value = (at_recv&0x0F);
								GET_AT(handle, &at_recv, &j);
								if(at_recv != 0x2C)
								{
									*return_value = (*return_value)*10+(at_recv&0x0F);
								}
								
							}
							else if(AT_CMD_CPMS == at_type)
							{
								GET_AT(handle, &at_recv, &j);
								return_value[0] = (at_recv&0x0F);
								GET_AT(handle, &at_recv, &j);
								if(at_recv != ',')
								{
									return_value[0] = return_value[0]*10+(at_recv&0x0F);
									GET_AT(handle, &at_recv, &j);
									if(at_recv != ',')
									{
										return_value[0] = return_value[0]*10+(at_recv&0x0F);
										GET_AT(handle, &at_recv, &j);
										if(at_recv == ',')
										{
											GET_AT(handle, &at_recv, &j);
											return_value[1] = (at_recv&0x0F);
											GET_AT(handle, &at_recv, &j);
											return_value[1] = return_value[1]*10+(at_recv&0x0F);
											GET_AT(handle, &at_recv, &j);
											if(at_recv != ',')
											{
												return_value[1] = return_value[1]*10+(at_recv&0x0F);
												//ret =  AT_RET_OK;
												//break;
											}
										}
									}
									else
									{
										GET_AT(handle, &at_recv, &j);
										return_value[1] = (at_recv&0x0F);
										GET_AT(handle, &at_recv, &j);
										return_value[1] = return_value[1]*10+(at_recv&0x0F);
										GET_AT(handle, &at_recv, &j);
										if(at_recv != ',')
										{
											return_value[1] = return_value[1]*10+(at_recv&0x0F);
											//ret =  AT_RET_OK;
											//break;
										}
									}
								}
								else
								{
									GET_AT(handle, &at_recv, &j);
									return_value[1] = (at_recv&0x0F);
									GET_AT(handle, &at_recv, &j);
									return_value[1] = return_value[1]*10+(at_recv&0x0F);
									GET_AT(handle, &at_recv, &j);
									if(at_recv != ',')
									{
										return_value[1] = return_value[1]*10+(at_recv&0x0F);
										//ret =  AT_RET_OK;
										//break;
									}
								}
							}
							else if(AT_CMD_CMGL == at_type)
							{

							}
							else if(AT_CMD_CMGR == at_type)
							{
								for(i=0; i<64; i++)
								{
									GET_AT(handle, &at_recv, &j);
									if(at_recv < 0x3A)
									{
										return_value[i] = at_recv&0x0F;
									}
									else if(at_recv > 0x40)
									{
										return_value[i] = (at_recv&0x0F)+9;
									}
									//PRINTF("AT_CMD_CMGR.ch.%x\n",return_value[i]);
								}
								ret =  AT_RET_OK;
								break;
							}
							else if(AT_CMD_APN == at_type)
							{	
								//PRINTF("AT_CMD_CMGR.ch.00\n");
								for(i=0; i<48; i++)
								{
									if(GET_AT(handle, &at_recv, &j))
									{
										return_value[i] = at_recv;
										
									}
									else
									{
										vTaskDelay(40);
									}
								}
								ret =  AT_RET_OK;
								break;
							}
							else if(AT_CMD_SOS == at_type)
							{
								for(i=0; i<24; i++)
								{
									if(GET_AT(handle, &at_recv, &j))
									{
										return_value[i] = at_recv;
									}
									else
									{
										vTaskDelay(40);
									}
								}
								ret =  AT_RET_OK;
								break;
							}
							ret =  AT_RET_OK;
							break;
						}
					}		 
					else
					{	  
						tmp_str = str;
						cmd_result_len_tmp=cmd_result_len;
						m=0;
					}	
				}	 
				else
				{
					vTaskDelay(30);
				}
			}	
			break;
		case AT_CMD_MIPCALL:
			while((xTaskGetTickCount() - time) < msecond)
			{
				//if(GET_AT(handle, &ch, &j) == kStatus_Success)
				if(GET_AT(handle, &at_recv, &j))
				{		
					/*waiting for the string that we expected*/
					if(*tmp_str  == at_recv)
					{
						tmp_str++;	
						m++;
						cmd_result_len_tmp--;						
						if(cmd_result_len_tmp  == 0x0)
						{								
							ret = AT_RET_OK;
							break;
						}
					}
					else
					{	  
						tmp_str = str;
						cmd_result_len_tmp=cmd_result_len;
						m=0;
					}	
				}
				else
				{
					vTaskDelay(30);
				}
			}	
			break;
		case AT_CMD_MIPDNS:
			//PRINTF("......................22......%d...\r\n",cmd_result_len_tmp);
			while((xTaskGetTickCount() - time) < msecond)
			{
			
				//if(GET_AT(handle, &ch, &j) == kStatus_Success)
				if(GET_AT(handle, &at_recv, &j))
				{	
					//PRINTF("......................0......%c...\r\n",at_recv);
					/*waiting for the string that we expected*/
					if(*tmp_str  == at_recv)
					{
						tmp_str++;	
						m++;
						cmd_result_len_tmp--;
						
						//PRINTF(".................1.............%c...\r\n",at_recv);
						if(cmd_result_len_tmp  == 0x0)
						{	
						
							for(i=0; i<16; i++)
							{
								if(GET_AT(handle, &at_recv, &j))
								{
								
								//PRINTF(".............................%c..\r\n",at_recv);
									return_value[i] = at_recv;
									//PRINTF("......%c.........................\r\n",return_value[i]);
								}
								else
								{
									vTaskDelay(20);
								}
							}
							ret =  AT_RET_OK;
							break;
						}
					}
					else
					{	  
						tmp_str = str;
						cmd_result_len_tmp=cmd_result_len;
						m=0;
					}	
				}
				else
				{
					vTaskDelay(30);
				}
			}
			
			break;
		case AT_CMD_MIPSEND:
			while((xTaskGetTickCount() - time) < msecond)
			{
				//if(GET_AT(handle, &ch, &j) == kStatus_Success)
				if(GET_AT(handle, &at_recv, &j))
				{		
					/*waiting for the string that we expected*/
					if(*tmp_str  == at_recv)
					{
						tmp_str++;	
						m++;
						cmd_result_len_tmp--;
						if(cmd_result_len_tmp  == 0x0)
						{	
							ret =  AT_RET_OK;
							break;
						}
					}
					else
					{	  
						tmp_str = str;
						cmd_result_len_tmp=cmd_result_len;
						m=0;
					}	
				}
				else
				{
					vTaskDelay(20);
				}
			}	
			break;
		case AT_CMD_MIPPUSH:
			while((xTaskGetTickCount() - time) < msecond)
			{
				//if(GET_AT(handle, &ch, &j) == kStatus_Success)
				if(GET_AT(handle, &at_recv, &j))
				{		
					/*waiting for the string that we expected*/
					if(*tmp_str  == at_recv)
					{
						tmp_str++;	
						m++;
						cmd_result_len_tmp--;
						if(cmd_result_len_tmp  == 0x0)
						{	
							ret =  AT_RET_OK;
							break;
						}
					}
					else
					{	  
						tmp_str = str;
						cmd_result_len_tmp=cmd_result_len;
						m=0;
					}	
				}
				else
				{
					vTaskDelay(30);
				}
			}	
			break;
		case AT_CMD_MIPREAD:
			while((xTaskGetTickCount() - time) < msecond)
			{
				//if(GET_AT(handle, &ch, &j) == kStatus_Success)
				if(GET_AT(handle, &at_recv, &j))
				{		
					/*waiting for the string that we expected*/
					if(*tmp_str  == at_recv)
					{
						tmp_str++;	
						m++;
						cmd_result_len_tmp--;
						if(cmd_result_len_tmp  == 0x0)
						{	
							GET_AT(handle, &at_recv, &j);
							for(i=0; i<7; i++)
							{
								if(GET_AT(handle, &at_recv, &j))
								{			
									buf_tmp[i] = at_recv;
									if(at_recv == ',')
									{
										rec_flag = i;
									}
									if(at_recv == '\r')
									{
										rec_flag = (i-rec_flag)-1;
										switch(rec_flag)
										{
											case 1:
												len_read = (buf_tmp[i-1]-'0')&0xf;
												break;													
											case 2:
												len_read = ((buf_tmp[i-2]-'0')&0xf)*10+((buf_tmp[i-1]-'0')&0xf);
												break;
											case 3:
												len_read = (((buf_tmp[i-3]-'0')&0xf)*10+((buf_tmp[i-2]-'0')&0xf))*10+((buf_tmp[i-1]-'0')&0xf);
												break;
											case 4:
												len_read = ((((buf_tmp[i-4]-'0')&0xf)*10+((buf_tmp[i-3]-'0')&0xf))+((buf_tmp[i-2]-'0')&0xf))*10+((buf_tmp[i-1]-'0')&0xf);
												break;
											default:
												PRINTF("..find...len read error............................\r\n");
												break;
										}
										GET_AT(handle, &at_recv, &j);
										break;
									}
								}
								else
								{
									vTaskDelay(20);
								}
							}
							PRINTF("..find...len_read..%d..........................\r\n",len_read);
							if(len_read > 0)
							{
								i=0;
								for(i=0; i<len_read; i++)
								{
									GET_AT(handle, &at_recv, &j);
									buf[i] = at_recv;
									//PRINTF("..%d..at_recv..%c..\r\n",i,buf[i]);
								}
								*ret_len = len_read;
								ret =  AT_RET_OK;
							}
							else
							{
								ret =  AT_RET_FAIL;
							}
							break;
						}
					}
					else
					{	  
						tmp_str = str;
						cmd_result_len_tmp=cmd_result_len;
						m=0;
					}	
				}
				else
				{
					vTaskDelay(30);
				}
			}	
			break;
			
		default:

			break;
	}
	return ret;
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_at(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
    u32_t i;
    u32_t ret = AT_RET_OK;

    for(i = 0; i < repeat; i++)
    {
        SEND_AT(handle, (uint8_t *)at_cmd_at, strlen(at_cmd_at));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_at_result, strlen(at_cmd_at_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret;	
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_ata(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_ata, strlen(at_cmd_ata));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_ata_result, strlen(at_cmd_ata_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_atd(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_atd, strlen(at_cmd_atd));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_atd_result, strlen(at_cmd_atd_result), timeout, AT_CMD_ATD, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_ath(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_ath, strlen(at_cmd_ath));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_ath_result, strlen(at_cmd_ath_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_at_c(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_at_c, strlen(at_cmd_at_c));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_at_result, strlen(at_cmd_at_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_ato(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_ato, strlen(at_cmd_ato));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_ato_result, strlen(at_cmd_ato_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_ccid(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_ccid, strlen(at_cmd_ccid));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_ccid_result, strlen(at_cmd_ccid_result), timeout, AT_CMD_CCID, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_cfun(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_cfun, strlen(at_cmd_cfun));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_at_result, strlen(at_cmd_at_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_cgatt(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_cgatt, strlen(at_cmd_cgatt));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_cgatt_result, strlen(at_cmd_cgatt_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_cgdcont(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_cgdcont, strlen(at_cmd_cgdcont));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_cgdcont_result, strlen(at_cmd_cgdcont_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_cgreg(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_cgreg, strlen(at_cmd_cgreg));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_cgreg_result, strlen(at_cmd_cgreg_result), timeout, AT_CMD_CGREG, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_cgatt_down(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_cgatt_down, strlen(at_cmd_cgatt_down));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_at_result, strlen(at_cmd_at_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_chup(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_chup, strlen(at_cmd_chup));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_chup_result, strlen(at_cmd_chup_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_clcc(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_clcc, strlen(at_cmd_clcc));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_clcc_result, strlen(at_cmd_clcc_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	if(ret != AT_RET_OK)
	{
		ret = AT_RET_CALL_DOWN;

	}	
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_clvl(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_clvl, strlen(at_cmd_clvl));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_clvl_result, strlen(at_cmd_clvl_result), timeout, AT_CMD_CLCC, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_cmgf(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_cmgf, strlen(at_cmd_cmgf));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_cmgf_result, strlen(at_cmd_cmgf_result), timeout, AT_CMD_CMGF, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_cmgl(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_cmgl, strlen(at_cmd_cmgl));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_cmgl_result, strlen(at_cmd_cmgl_result), timeout, AT_CMD_CMGL, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_cmgr(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_cmgr, strlen(at_cmd_cmgr));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_cmgr_result, strlen(at_cmd_cmgr_result), timeout, AT_CMD_CMGR, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_cmgd(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_cmgd, strlen(at_cmd_cmgd));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_cmgd_result, strlen(at_cmd_cmgd_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_cmgd_all(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_cmgd_all, strlen(at_cmd_cmgd_all));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_cmgd_result, strlen(at_cmd_cmgd_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_cmee(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_cmee, strlen(at_cmd_cmee));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_at_result, strlen(at_cmd_at_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_cnmi(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_cnmi, strlen(at_cmd_cnmi));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_at_result, strlen(at_cmd_at_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_cpms(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
    u32_t i;
    u32_t ret = AT_RET_OK;
	u32_t sms_numb[2];
    for(i = 0; i < repeat; i++)
    {
        SEND_AT(handle, (uint8_t *)at_cmd_cpms, strlen(at_cmd_cpms));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_cpms_result, strlen(at_cmd_cpms_result), timeout, AT_CMD_CPMS, AT_BUF_LEN_MIN, sms_numb, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				ppp_gprs_info.sms_current_number_me = sms_numb[0];
				ppp_gprs_info.sms_max_number_me = sms_numb[1];
				
				PRINTF(".sms_curr_numb:%d.\n",ppp_gprs_info.sms_current_number_me);
				PRINTF(".sms_max_numb:%d.\n",ppp_gprs_info.sms_max_number_me);
				return AT_RET_OK;
			}	
		}
	}
	return ret;	
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_cpms_incomingcall(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
    u32_t i;
    u32_t ret = AT_RET_FAIL;
	u32_t sms_numb[2];
    for(i = 0; i < repeat; i++)
    {
        SEND_AT(handle, (uint8_t *)at_cmd_cpms, strlen(at_cmd_cpms));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_cpms_result, strlen(at_cmd_cpms_result), timeout, AT_CMD_CPMS, AT_BUF_LEN_MIN, sms_numb, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				if(sms_numb[0] == ppp_gprs_info.sms_current_number_me)
				{
					return AT_RET_CALL_ING;	
				}
				PRINTF("..sms.curr.%d..\n",sms_numb[0]);
				PRINTF("..sms.max.%d..\n",sms_numb[1]);
				ppp_gprs_info.sms_current_number_me = sms_numb[0];
				ppp_gprs_info.sms_max_number_me = sms_numb[1];
				
				return AT_RET_OK;
			}
		}
	}
	return ret;	
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_creg(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_creg, strlen(at_cmd_creg));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_creg_result, strlen(at_cmd_creg_result), timeout, AT_CMD_CREG, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_csq(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;
	u32_t csq_value;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_csq, strlen(at_cmd_csq));		
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_csq_result, strlen(at_cmd_csq_result), timeout, AT_CMD_CSQ, AT_BUF_LEN_MIN, &csq_value, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				ppp_gprs_info.signal_quility = csq_value;
				PRINTF("..csq_value:%d..\n",csq_value);
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_gtset_gprs(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_gtset_gprs, strlen(at_cmd_gtset_gprs));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_at_result, strlen(at_cmd_at_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_gtset_call(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_gtset_call, strlen(at_cmd_gtset_call));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_at_result, strlen(at_cmd_at_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_mmicg(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_mmicg, strlen(at_cmd_mmicg));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_mmicg_result, strlen(at_cmd_mmicg_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_plus(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_plus, strlen(at_cmd_plus));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_plus_result, strlen(at_cmd_plus_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_sl_sms_apn(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
    u32_t i=0;
	u32_t j;
	u8_t ppp_para_len=1;
	u8_t apn_flag=0;
	u8_t gprs_user_flag=0;
	u8_t gprs_pwd_flag=0;;
    u32_t ret = AT_RET_FAIL;
	u32_t sl_apn_info[48];
	CHAR sl_apn_info_char[48];
    for(i = 0; i < repeat; i++)
    {
        SEND_AT(handle, (uint8_t *)at_cmd_sl_sms, strlen(at_cmd_sl_sms));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_check_sl_sms_apn_info, strlen(at_check_sl_sms_apn_info), timeout, AT_CMD_APN, AT_BUF_LEN_MIN, sl_apn_info, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				memset(&ppp_gprs_info.gprs_apn, 0, sizeof(ppp_gprs_info.gprs_apn));
				memset(&ppp_gprs_info.ppp_pap_user, 0, sizeof(ppp_gprs_info.ppp_pap_user));
				memset(&ppp_gprs_info.ppp_pap_passwd, 0, sizeof(ppp_gprs_info.ppp_pap_passwd));
						
				for(j=0; j<48;j++)
				{
					sl_apn_info_char[j] = (CHAR)sl_apn_info[j];
					if(apn_flag == 0)
					{
						if(sl_apn_info_char[j] == ':')
						{
							apn_flag = j;						
							ppp_gprs_info.gprs_apn[0]='\"';
							ppp_gprs_info.gprs_apn[ppp_para_len]='\"';
							ppp_gprs_info.gprs_apn[ppp_para_len+1]='\r';
							ppp_para_len=0;
							continue;
						}
						else
						{
							ppp_gprs_info.gprs_apn[ppp_para_len] = sl_apn_info_char[j];
							ppp_para_len++;
						}
					}
					if((gprs_user_flag == 0)&&(apn_flag != 0))
					{
						if(sl_apn_info_char[j] == ':')
						{
							gprs_user_flag = j;
							ppp_para_len=0;
							continue;
						}
						else
						{
							ppp_gprs_info.ppp_pap_user[ppp_para_len] = sl_apn_info_char[j];
							ppp_para_len++;
						}
					}	
					if((gprs_pwd_flag == 0)&&(gprs_user_flag != 0)&&(apn_flag != 0))
					{
						if(sl_apn_info_char[j] == ':')
						{
							gprs_pwd_flag = j;
							ppp_para_len=0;
							continue;
						}
						else
						{
							ppp_gprs_info.ppp_pap_passwd[ppp_para_len] = sl_apn_info_char[j];
							ppp_para_len++;
						}
					}
				}			
				PRINTF(".sl_apn_info_char:%s.\r\n",sl_apn_info_char);
				PRINTF(".gprs_apn:%s.\r\n",ppp_gprs_info.gprs_apn);
				PRINTF(".pap_user:%s.\r\n",ppp_gprs_info.ppp_pap_user);
				PRINTF(".pap_passwd:%s.\r\n",ppp_gprs_info.ppp_pap_passwd);
				return AT_RET_OK;
			}
			else
			{
				ret = AT_RET_FAIL;
			}
		}	
	}
	return ret;	
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_sl_sms_sos(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
    u32_t i=0;
	u32_t j;
	u8_t ppp_para_len=0;
	u8_t apn_flag=0;
	//u8_t gprs_user_flag=0;
	//u8_t gprs_pwd_flag=0;;
    u32_t ret = AT_RET_OK;
	u32_t sl_sos_info[24];
	CHAR sl_sos_info_char[24];
    for(i = 0; i < repeat; i++)
    {
        SEND_AT(handle, (uint8_t *)at_cmd_sl_sms, strlen(at_cmd_sl_sms));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_check_sl_sms_sos_info, strlen(at_check_sl_sms_sos_info), timeout, AT_CMD_SOS, AT_BUF_LEN_MIN, sl_sos_info, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				memset(&ppp_gprs_info.sos_call, 0, sizeof(ppp_gprs_info.sos_call));
				for(j=0; j<24;j++)
				{
					sl_sos_info_char[j] = (CHAR)sl_sos_info[j];
					if(apn_flag == 0)
					{
						if(sl_sos_info_char[j] == ':')
						{
							apn_flag = j;						
							ppp_gprs_info.sos_call[ppp_para_len]='\r';
							ppp_para_len=0;
							break;
						}
						else
						{
							ppp_gprs_info.sos_call[ppp_para_len] = sl_sos_info_char[j];
							ppp_para_len++;
						}
					}

				}	
				
				PRINTF(".sl_sos_info_char:%s.\r\n",sl_sos_info_char);
				if(apn_flag == 0)
				{
					return AT_RET_FAIL;	
				}
				PRINTF(".sos_call_numb:%s.\r\n",ppp_gprs_info.sos_call);
				return AT_RET_OK;
			}	
			else
			{
				ret = AT_RET_FAIL;
			}
		}

	}
	return ret;	
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_sos(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;
	
	PRINTF("..sos..%s\r\n",at_cmd_sos_call);
	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_sos_call, strlen(at_cmd_sos_call));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_at_result, strlen(at_cmd_at_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_gtset_iprfmt(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_OK;
	
	PRINTF("..gtset..%s\r\n",at_cmd_gtset_iprfmt);
	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_gtset_iprfmt, strlen(at_cmd_gtset_iprfmt));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_at_result, strlen(at_cmd_at_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_mipcall(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_FAIL;
	
	PRINTF("..mipcall..%s\r\n",at_cmd_mipcall);
	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_mipcall, strlen(at_cmd_mipcall));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_mipcall_result, strlen(at_cmd_mipcall_result), timeout, AT_CMD_MIPCALL, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				
				PRINTF("..mipcall00..\r\n");
				return AT_RET_OK;
			}	
		}
	}
	PRINTF("..mipcall01..\r\n");
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_mipcall_down(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_FAIL;
	
	//PRINTF("..mipcall_down..%s\r\n",at_cmd_mipcall_down);
	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_mipcall_down, strlen(at_cmd_mipcall_down));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_mipcall_down_result, strlen(at_cmd_mipcall_down_result), timeout, AT_CMD_MIPCALL, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_mipopen(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_FAIL;
	
	//PRINTF("..mipopen..%s\r\n",at_cmd_mipopen);
	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_mipopen, strlen(at_cmd_mipopen));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_mipopen_result, strlen(at_cmd_mipopen_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_mipclose(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_FAIL;
	
	//PRINTF("..mipclose..%s\r\n",at_cmd_mipclose);
	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_mipclose, strlen(at_cmd_mipclose));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_mipclose_result, strlen(at_cmd_mipclose_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_mipdns(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t j;
	u32_t ret = AT_RET_FAIL;
	u32_t dns_value[16];

	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_mipdns, strlen(at_cmd_mipdns));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_mipdns_result, strlen(at_cmd_mipdns_result), timeout, AT_CMD_MIPDNS, AT_BUF_LEN_MIN, &dns_value, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				memset(&dns_ip_addr, 0, sizeof(dns_ip_addr));
				for(j=0;j<16;j++)
				{
					dns_ip_addr[j] = (u8_t)dns_value[j];
				}				
				//strncpy(dns_ip_addr, dns_value, sizeof(dns_value));	
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_mipsend(uart_rtos_handle_t *handle, u32_t repeat, u32_t len, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_FAIL;
	
	PRINTF("..mipsend..%s\r\n",at_cmd_mipsend);
	for(i = 0; i < repeat; i++)
	{
		SEND_AT(handle, (uint8_t *)at_cmd_mipsend, len);
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_mipsend_result, strlen(at_cmd_mipsend_result), timeout, AT_CMD_MIPSEND, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}				
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_mipread(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return, u32_t buf_len, CHAR *buf, u32_t *len_ret)
{
	u32_t i,j;
	u32_t ret = AT_RET_FAIL;
	
	for(i = 0; i < repeat; i++)
	{
	
		//PRINTF("..mipread..%s\r\n",at_cmd_mipread);
		SEND_AT(handle, (uint8_t *)at_cmd_mipread, strlen(at_cmd_mipread));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_mipread_result, strlen(at_cmd_mipread_result), timeout, AT_CMD_MIPREAD, buf_len, NULL, buf, len_ret);
			if(ret == AT_RET_OK)
			{
				//PRINTF("..mipread.buf..=%c=..\r\n",*buf);
				//PRINTF("..mipread.buf..=0x%x=..\r\n",*buf);
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}

/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_mippush(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_FAIL;
	u32_t push_value=0;
	
	for(i = 0; i < repeat; i++)
	{
		PRINTF("..mippush..%s\r\n",at_cmd_mippush);
		PRINTF("..mippush_result..%s\r\n",at_cmd_mippush_result);
		SEND_AT(handle, (uint8_t *)at_cmd_mippush, strlen(at_cmd_mippush));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_mippush_result, strlen(at_cmd_mippush_result), timeout, AT_CMD_MIPPUSH, AT_BUF_LEN_MIN, &push_value, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				//PRINTF("..mippush ret =%d..\r\n",push_value);
				//PRINTF("..mippush ret =%c..\r\n",push_value);
				if(push_value == 0)
				{
					PRINTF("..mippush..ok..\r\n");
					return AT_RET_OK;
				}
				else if(push_value == 1)
				{
					PRINTF("..mippush not ok..\r\n");
					return AT_RET_MIPPUSH_ERR;
				}
				else
				{
					PRINTF("..mippush miss..\r\n");
					return AT_RET_FAIL;
				}
			}	
		}
	}
	return ret; 
}


/*
*hao.yunran
*2016.10.10
* send AT to GPRS Modem, test if Modem is ready
*/
at_ret modem_at_cmd_miprtcp(uart_rtos_handle_t *handle, u32_t repeat, u32_t timeout, u32_t wait_return)
{
	u32_t i;
	u32_t ret = AT_RET_FAIL;
	
	for(i = 0; i < repeat; i++)
	{
		//SEND_AT(handle, (uint8_t *)at_cmd_mipdns, strlen(at_cmd_mipdns));
		if(WAIT_RETURN == wait_return)
		{
			ret = wait_at_cmd_return(handle, at_cmd_miprtcp_result, strlen(at_cmd_miprtcp_result), timeout, AT_CMD_AT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
			if(ret == AT_RET_OK)
			{
				return AT_RET_OK;
			}	
		}
	}
	return ret; 
}


/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
at_ret modem_sms_delete(u16_t sms_numb)
{
    u32_t ret = AT_RET_OK;

	memset(&at_cmd_cmgd, 0, sizeof(at_cmd_cmgd));				
	sprintf(at_cmd_cmgd,"AT+CMGD=%d\r", sms_numb); 			
	PRINTF("..delet..%s\r\n",at_cmd_cmgd);
    ret = modem_at_cmd_cmgd(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, WAIT_RETURN);
    if(ret != AT_RET_OK)
    {
		PRINTF("hh ret:...%d\n",ret);
    }	
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
at_ret modem_sms_read_apn_pap(u16_t sms_numb)
{
    u32_t ret = AT_RET_OK;

	memset(&at_cmd_sl_sms, 0, sizeof(at_cmd_sl_sms));				
	sprintf(at_cmd_sl_sms,"AT+CMGR=%d\r", sms_numb); 			
	
	PRINTF(".sms_cmd.%s\r\n",at_cmd_sl_sms);
	
    ret = modem_at_cmd_sl_sms_apn(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MIN, WAIT_RETURN);
    if(ret != AT_RET_OK)
    {
		PRINTF("..get_apn_fail..\r\n");
    }	
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
at_ret modem_sms_read_sos(u16_t sms_numb)
{
    u32_t ret = AT_RET_OK;

	memset(&at_cmd_sl_sms, 0, sizeof(at_cmd_sl_sms));				
	sprintf(at_cmd_sl_sms,"AT+CMGR=%d\r", sms_numb); 			

	
	PRINTF(".sms_cmd.%s\n",at_cmd_sl_sms);
    ret = modem_at_cmd_sl_sms_sos(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MIN, WAIT_RETURN);
    if(ret != AT_RET_OK)
    {
		PRINTF("..get_sos_fail..\n");
    }	
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
at_ret modem_get_gprs_apn()
{
    u32_t ret = AT_RET_OK;
	u16_t sms_numb = 0;

	sms_numb = ppp_gprs_info.sms_current_number_me;
	ret = modem_at_cmd_cmgf(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MIN, WAIT_RETURN);
	if(ret != AT_RET_OK)
    {

    }
	vTaskDelay(60);
	//get gprs APN and ppp pap user&passwd	
	ret = modem_sms_read_apn_pap(sms_numb);
	if(ret == AT_RET_OK)
	{
		return ret;
	}	
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
at_ret modem_get_sos_call_numb()
{
    u32_t ret = AT_RET_OK;
	//u16_t sms_numb_read = 0;
	u16_t sms_numb = 0;
	#if 0
	ret = modem_at_cmd_csmp(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, WAIT_RETURN);
	if(ret != AT_RET_OK)
    {

    }
	#endif
	sms_numb = ppp_gprs_info.sms_current_number_me;
	vTaskDelay(60);
	ret = modem_at_cmd_cmgf(&ppp_handle, AT_REPEAT_MIN, AT_TIMEOUT_MIN, WAIT_RETURN);
	if(ret != AT_RET_OK)
    {

    }
	vTaskDelay(60);
	//get gprs APN and ppp pap user&passwd	
	ret = modem_sms_read_sos(sms_numb);
	if(ret == AT_RET_OK)
	{
		return ret;
	}	
	return ret;
}

/*
*hao.yunran
*2016.10.10
*set the state of my tcpip
*/
at_ret modem_wait_sos_call_end_clcc(uart_rtos_handle_t *handle)
{
	u32_t i;
    u32_t ret = AT_RET_CALL_DOWN_TIMER_OUT;

    for(i = 0; i < MAX_CALL_IN_TIMES; i++)
    {
		if(SOS_CALL_HAND_UP == ppp_gprs_info.irq_for_sos_call)
		{
			return AT_RET_SOS_CALL_DOWN;
		}
		vTaskDelay(400);
		ret = modem_at_cmd_clcc(&ppp_handle, AT_REPEAT_NOR, AT_CLCC_TIMEOUT_NOR, WAIT_RETURN);
		if(ret == AT_RET_CALL_DOWN)
		{
			return ret;
		}		
    	
	}
	return AT_RET_CALL_DOWN_TIMER_OUT;
}

/*
*hao.yunran
*2016.10.10
*set the state of my tcpip
*/
at_ret modem_wait_incoming_call_end_clcc(uart_rtos_handle_t *handle)
{
	u32_t i;
    u32_t ret = AT_RET_CALL_DOWN_TIMER_OUT;

    for(i = 0; i < MAX_CALL_IN_TIMES; i++)
    {
		if(SOS_CALL_IRQ == ppp_gprs_info.irq_for_sos_call)
		{
			return AT_RET_SOS_CALL_UP;
		}
		else if(SOS_CALL_HAND_UP == ppp_gprs_info.irq_for_sos_call)
		{
			return AT_RET_SOS_CALL_DOWN;
		}
		vTaskDelay(400);
		ret = modem_at_cmd_clcc(&ppp_handle, AT_REPEAT_NOR, AT_CLCC_TIMEOUT_NOR, WAIT_RETURN);
		if(ret == AT_RET_CALL_DOWN)
		{
			return ret;
		}		
    	
	}
	return AT_RET_CALL_DOWN_TIMER_OUT;
}

/*
*hao.yunran
*2016.10.10
*set the state of my tcpip
*/
at_ret modem_wait_incoming_call_answer_clcc(uart_rtos_handle_t *handle)
{
    u32_t ret = AT_RET_OK;
	u32_t time = xTaskGetTickCount();//get_sys_tick();
	while((xTaskGetTickCount() - time) < (ppp_gprs_apn_pap_info.incoming_call_answer_timeout*1000))
	{
		if(SOS_CALL_IRQ == ppp_gprs_info.irq_for_sos_call)
		{
			return AT_RET_SOS_CALL_UP;
		}
		vTaskDelay(400);
		ret = modem_at_cmd_clcc(&ppp_handle, AT_REPEAT_NOR, AT_CLCC_TIMEOUT_MIN, WAIT_RETURN);
		if(ret == AT_RET_CALL_DOWN)
		{
			return AT_RET_CALL_DOWN;
		}		
	}

	return AT_RET_CALL_DOWN_TIMER_OUT;
}

/*
*hao.yunran
*2016.10.10
*set the state of my tcpip
*/
at_ret modem_check_modem_status(uart_rtos_handle_t *handle)
{
    u32_t ret = AT_RET_OK;
	ret = wait_at_cmd_return(handle, at_check_modem_status_result, strlen(at_check_modem_status_result), AT_CHECK_MODEM_TIMEOUT, AT_CMD_CHECK_MODEM_STAT, AT_BUF_LEN_MIN, NULL, NULL, NULL);
	
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
at_ret modem_dialog_gprs(uart_rtos_handle_t *handle)
{
    u32_t i;
    u32_t ret = AT_RET_OK;

    for(i = 0; i < MAX_GPRS_DIAL_TIMES; i++)
    {
        SEND_AT(handle, (uint8_t *)at_cmd_dial_gprs, strlen(at_cmd_dial_gprs));
		ret = wait_at_cmd_return(handle, at_cmd_dial_gprs_result, strlen(at_cmd_dial_gprs_result), GPRS_DIAL_TIMEOUT, AT_CMD_DIAL_GPRS, AT_BUF_LEN_MIN, NULL, NULL, NULL);
	    if(ret == AT_RET_OK)
	    {
	    	return AT_RET_OK;
	    }	 	
	    //send_at_cmd_at(handle);
    }
   return AT_RET_FAIL;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gprs_modem_init()
{
    u32_t ret = AT_RET_OK;
#ifdef CPU_MK24FN256VDC12

	PRINTF("\r\n^_^seq002.modem init\r\n");
	PR_INFO("..modem init..\r\n");
	ppp_gprs_info.ppp_modem_init_flag = 1;
	ppp_gprs_info.modem_need_at_mode = 1;
	ppp_gprs_info.irq_for_sms_or_incoming_call = NO_SMS_OR_INCOMING_CALL;
	ppp_gprs_info.ppp_gprs_current_seq = SEQ_002;

	vTaskDelay(2000);
    modem_at_cmd_plus(&ppp_handle, AT_REPEAT_MIN, AT_TIMEOUT_MAX, NOT_WAIT);
	vTaskDelay(1200);
    ret = modem_at_cmd_at(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, WAIT_RETURN);
	if(ret != AT_RET_OK)
    {
		#if 0
		MODEM_POWER_ON();//拉低开GSM
		vTaskDelay(3200);
		MODEM_POWER_OFF();
		#else
		
		PRINTF("..MODEM_EMERG_RST_ON..\r\n");
		MODEM_EMERG_RST_ON();//拉低开GSM
		vTaskDelay(1200);
		MODEM_EMERG_RST_OFF();
		vTaskDelay(10000);
		
		MODEM_POWER_ON();//拉低开GSM
		vTaskDelay(1200);
		MODEM_POWER_OFF();
		#endif
	}
	else
	{
		modem_at_cmd_cfun(&ppp_handle, AT_REPEAT_MIN, AT_TIMEOUT_MAX, NOT_WAIT);
		vTaskDelay(500);
		PRINTF("..modem_at_cmd_cfun..\r\n");
	}
	//vTaskDelay(3000);
	ret = modem_check_modem_status(&ppp_handle);
	if(ret != AT_RET_OK)
	{
		return ret;
	}
	GPIO_ClearPinsInterruptFlags(MODEM_RING_SMS_GPIO, 1U << MODEM_RING_SMS_GPIO_PIN);
	EnableIRQ(MODEM_RING_SMS_IRQ);		
	GPIO_ClearPinsInterruptFlags(MODEM_SOS_GPIO, 1U << MODEM_SOS_GPIO_PIN);
	EnableIRQ(MODEM_SOS_IRQ);
    
	memset(&ppp_gprs_apn_pap_info, 0, sizeof(ppp_gprs_apn_pap_info));
	get_gprs_info();
	PRINTF(".apn_flash:%c%c%c%c.\r\n",ppp_gprs_apn_pap_info.gprs_apn[0],ppp_gprs_apn_pap_info.gprs_apn[1],ppp_gprs_apn_pap_info.gprs_apn[2],ppp_gprs_apn_pap_info.gprs_apn[3]);

	if((ppp_gprs_apn_pap_info.gprs_apn[0]=='"')&&(ppp_gprs_apn_pap_info.gprs_apn[1]=='s')&&(ppp_gprs_apn_pap_info.gprs_apn[2]=='l'))
	{
		PRINTF("\n .read_apn_from_flash.\n");
		memset(&ppp_gprs_info.gprs_apn, 0, sizeof(ppp_gprs_info.gprs_apn));
		memset(&ppp_gprs_info.ppp_pap_user, 0, sizeof(ppp_gprs_info.ppp_pap_user));
		memset(&ppp_gprs_info.ppp_pap_passwd, 0, sizeof(ppp_gprs_info.ppp_pap_passwd));
		memset(&ppp_gprs_info.sos_call, 0, sizeof(ppp_gprs_info.sos_call));
		
		strncpy(ppp_gprs_info.gprs_apn, ppp_gprs_apn_pap_info.gprs_apn, strlen(ppp_gprs_apn_pap_info.gprs_apn));
		strncpy(ppp_gprs_info.ppp_pap_user, ppp_gprs_apn_pap_info.ppp_pap_user, strlen(ppp_gprs_apn_pap_info.ppp_pap_user));
		strncpy(ppp_gprs_info.ppp_pap_passwd, ppp_gprs_apn_pap_info.ppp_pap_passwd, strlen(ppp_gprs_apn_pap_info.ppp_pap_passwd));				
		strncpy(ppp_gprs_info.sos_call, ppp_gprs_apn_pap_info.sos_call, strlen(ppp_gprs_apn_pap_info.sos_call));				
		ppp_gprs_info.incoming_call_answer_timeout = ppp_gprs_apn_pap_info.incoming_call_answer_timeout;
		ppp_gprs_info.sms_current_number_me = ppp_gprs_apn_pap_info.sms_curr_number_me;
		ppp_gprs_info.sms_max_number_me = ppp_gprs_apn_pap_info.sms_max_number_me;
	}
	else
	{
		PRINTF("\n .get_ppp_gprs_info.\n");
		memset(&ppp_gprs_apn_pap_info, 0, sizeof(ppp_gprs_apn_pap_info));
		strncpy(ppp_gprs_apn_pap_info.gprs_apn, ppp_gprs_info.gprs_apn, strlen(ppp_gprs_info.gprs_apn));
		strncpy(ppp_gprs_apn_pap_info.ppp_pap_user, ppp_gprs_info.ppp_pap_user, strlen(ppp_gprs_info.ppp_pap_user));
		strncpy(ppp_gprs_apn_pap_info.ppp_pap_passwd, ppp_gprs_info.ppp_pap_passwd, strlen(ppp_gprs_info.ppp_pap_passwd));				
		strncpy(ppp_gprs_apn_pap_info.sos_call, ppp_gprs_info.sos_call, strlen(ppp_gprs_info.sos_call));
		ppp_gprs_apn_pap_info.incoming_call_answer_timeout = ppp_gprs_info.incoming_call_answer_timeout;
		ppp_gprs_apn_pap_info.sms_curr_number_me = ppp_gprs_info.sms_current_number_me;
		ppp_gprs_apn_pap_info.sms_max_number_me = ppp_gprs_info.sms_max_number_me;
		sync_gprs_info();		
	}
	PRINTF(".apn:%s.user:%s.pwd:%s.sos:%s.auto_answer:%d\r\n",ppp_gprs_info.gprs_apn, ppp_gprs_info.ppp_pap_user, ppp_gprs_info.ppp_pap_passwd, ppp_gprs_info.sos_call, ppp_gprs_info.incoming_call_answer_timeout);
	PRINTF(".curr_sms:%d.max_sms:%d.\r\n",ppp_gprs_info.sms_current_number_me,ppp_gprs_info.sms_max_number_me);

	memset(&at_cmd_sos_call, 0, sizeof(at_cmd_sos_call));
	sprintf(at_cmd_sos_call,"ATD%s\r", ppp_gprs_info.sos_call);
	PRINTF(".sos_cmd:%s.\r\n",at_cmd_sos_call);

    ret = modem_at_cmd_at(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, WAIT_RETURN);
    if(ret != AT_RET_OK)
    {
    	PRINTF("seq002..modem init Fail..\r\n");
		return AT_RET_FAIL;
    }	
	vTaskDelay(60);
	ret = modem_at_cmd_at_c(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MIN, NOT_WAIT);
    if(ret != AT_RET_OK)
    {

    }
	vTaskDelay(60);
	ret = modem_at_cmd_gtset_gprs(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MIN, NOT_WAIT);
    if(ret != AT_RET_OK)
    {

    }
	vTaskDelay(60);
	ret = modem_at_cmd_gtset_call(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MIN, NOT_WAIT);
    if(ret != AT_RET_OK)
    {

    }
	vTaskDelay(60);
	ret = modem_at_cmd_mmicg(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MAX, NOT_WAIT);
	if(ret != AT_RET_OK)
	{

	}
	vTaskDelay(60);
	ret = modem_at_cmd_clvl(&ppp_handle, AT_REPEAT_NOR, AT_TIMEOUT_MAX, NOT_WAIT);
	if(ret != AT_RET_OK)
	{

	}
	vTaskDelay(60);
	ret = modem_at_cmd_cmee(&ppp_handle, AT_REPEAT_MIN, AT_TIMEOUT_MAX, NOT_WAIT);
	if(ret != AT_RET_OK)
	{

	}
	vTaskDelay(60);
	ret = modem_at_cmd_gtset_iprfmt(&ppp_handle, AT_REPEAT_MIN, AT_TIMEOUT_MAX, NOT_WAIT);
	if(ret != AT_RET_OK)
	{

	}
	//ppp_gprs_info.ppp_modem_init_flag = 0;
	ppp_gprs_info.modem_need_at_mode = 0;
#endif
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gprs_set_mode_to_at()
{
    u32_t ret = AT_RET_OK;
	
	ppp_gprs_info.modem_need_at_mode = 1;
	vTaskDelay(2000);
    ret = modem_at_cmd_plus(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, WAIT_RETURN);
    if(ret != AT_RET_OK)
    {

    }	
	vTaskDelay(2000);
    //modem_at_cmd_atd(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, WAIT_RETURN);
	ppp_gprs_info.modem_need_at_mode = 0;
	
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gprs_check_sim()
{
    u32_t ret = AT_RET_OK;
	
	PRINTF("\r\n^_^seq003.check sim\r\n");
	ppp_gprs_info.ppp_gprs_current_seq = SEQ_003;
    ret = modem_at_cmd_ccid(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, WAIT_RETURN);
	vTaskDelay(60);
    if(ret != AT_RET_OK)
    {
		sim_status = 0;
    }
	else
	{
		sim_status = 1;
	}

	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gprs_get_gprs_sign()
{
    u32_t ret = AT_RET_OK;
	
	PRINTF("\r\n^_^seq005.sign\r\n");
	ppp_gprs_info.ppp_gprs_current_seq = SEQ_005;
    ret = modem_at_cmd_csq(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, WAIT_RETURN);
    if(ret != AT_RET_OK)
    {

    }
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gprs_get_gprs_status()
{
    u32_t ret = AT_RET_OK;
	
	PRINTF("\r\n^_^seq006.gprs stat\r\n");
	ppp_gprs_info.ppp_gprs_current_seq = SEQ_006;
    ret = modem_at_cmd_cgreg(&ppp_handle, AT_REPEAT_MAX, AT_CGREG_TIMEOUT_MAX, WAIT_RETURN);
    if(ret != AT_RET_OK)
    {

    }	
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gprs_set_sms_storage()
{
    u32_t ret = AT_RET_OK;
	u32_t sms_numb_me_tmp = 0;
	u32_t sms_numb = 0;

	PRINTF("\r\n^_^seq004.sms\r\n");
	ppp_gprs_info.ppp_gprs_current_seq = SEQ_004;
	//get sms curr number from flash
	sms_numb_me_tmp = ppp_gprs_info.sms_current_number_me;
	//get modem' sms number and save to ppp_gprs_info
    ret = modem_at_cmd_cpms(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, WAIT_RETURN);
    if(ret != AT_RET_OK)
    {

    }	

	if(ppp_gprs_info.sms_max_number_me != MAX_SMS_NUMB)
	{
		ppp_gprs_info.sms_max_number_me = MAX_SMS_NUMB;
	}
	if(ppp_gprs_info.sms_current_number_me > MAX_SMS_NUMB)
	{
		return AT_RET_FAIL;
	}
	// sms number of flash is smaller than km' sms number
	if(sms_numb_me_tmp < ppp_gprs_info.sms_current_number_me)
	{		
		ppp_gprs_apn_pap_info.sms_curr_number_me = ppp_gprs_info.sms_current_number_me;
		ppp_gprs_apn_pap_info.sms_max_number_me = ppp_gprs_info.sms_max_number_me;	
		vTaskDelay(60);
		ret = modem_at_cmd_cmgf(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, WAIT_RETURN);
		if(ret != AT_RET_OK)
	    {

	    }
		sms_numb = ppp_gprs_info.sms_current_number_me;
		for(; sms_numb > sms_numb_me_tmp; sms_numb--)
		{
			vTaskDelay(60);
			PRINTF("seq004.sms_numb.%d\n",sms_numb);
			ret = modem_sms_read_apn_pap(sms_numb);
			if(ret == AT_RET_OK)
			{	
				memset(&ppp_gprs_apn_pap_info.gprs_apn, 0, sizeof(ppp_gprs_apn_pap_info.gprs_apn));
				memset(&ppp_gprs_apn_pap_info.ppp_pap_user, 0, sizeof(ppp_gprs_apn_pap_info.ppp_pap_user));
				memset(&ppp_gprs_apn_pap_info.ppp_pap_passwd, 0, sizeof(ppp_gprs_apn_pap_info.ppp_pap_passwd));
				strncpy(ppp_gprs_apn_pap_info.gprs_apn, ppp_gprs_info.gprs_apn, strlen(ppp_gprs_info.gprs_apn));
				strncpy(ppp_gprs_apn_pap_info.ppp_pap_user, ppp_gprs_info.ppp_pap_user, strlen(ppp_gprs_info.ppp_pap_user));
				strncpy(ppp_gprs_apn_pap_info.ppp_pap_passwd, ppp_gprs_info.ppp_pap_passwd, strlen(ppp_gprs_info.ppp_pap_passwd));				
				sync_gprs_info();
				break;
			}
		}
		sms_numb = ppp_gprs_info.sms_current_number_me;
		for(; sms_numb > sms_numb_me_tmp; sms_numb--)
		{
			vTaskDelay(60);
			PRINTF("seq004.sms_numb.%d\n",sms_numb);
			ret = modem_sms_read_sos(sms_numb);
			if(ret == AT_RET_OK)
			{	
				memset(&ppp_gprs_apn_pap_info.sos_call, 0, sizeof(ppp_gprs_apn_pap_info.sos_call));			
				strncpy(ppp_gprs_apn_pap_info.sos_call, ppp_gprs_info.sos_call, strlen(ppp_gprs_info.sos_call));				
				//sos_len = strlen(ppp_gprs_info.sos_call);
				memset(&at_cmd_sos_call, 0, sizeof(at_cmd_sos_call));				
				sprintf(at_cmd_sos_call,"ATD%s\r", ppp_gprs_info.sos_call);				
				PRINTF("seq004.sos_call.%s\n",at_cmd_sos_call);
				sync_gprs_info();
				break;
			}
		}		
		ret = AT_RET_OK;
	}
	//if km is power off, and then someone send sms to it, when km power on, the number maybe more than the sms number margin
	if((ppp_gprs_info.sms_current_number_me+MAX_MARGIN_SMS_NUMB_ME) >= ppp_gprs_info.sms_max_number_me)
	{
		sms_numb = ppp_gprs_info.sms_current_number_me;
		sms_numb_me_tmp = ppp_gprs_info.sms_max_number_me;	
		#if 0
		for(; sms_numb > 0; sms_numb--)
		{
			modem_sms_delete(sms_numb);			
			vTaskDelay(60);
		}
		#endif
		vTaskDelay(60);
		ret = modem_at_cmd_cmgd_all(&ppp_handle, AT_REPEAT_MIN, AT_DELET_SMS_TIMEOUT_MAX, WAIT_RETURN);
	    if(ret != AT_RET_OK)
	    {
			PRINTF(".delete sms all fail.\r\n");
	    }
		vTaskDelay(60);
		modem_at_cmd_cnmi(&ppp_handle, AT_REPEAT_MIN, AT_TIMEOUT_MAX, NOT_WAIT);
		vTaskDelay(60);
		// get the new number of sms and store into flash
		ret = modem_at_cmd_cpms(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, WAIT_RETURN);
	    if(ret != AT_RET_OK)
	    {

	    }	
		ppp_gprs_apn_pap_info.sms_curr_number_me = 1;
		ppp_gprs_apn_pap_info.sms_max_number_me = ppp_gprs_info.sms_max_number_me;		
		sync_gprs_info();
		ret = AT_RET_OK;
	}
	else
	{
		ppp_gprs_apn_pap_info.sms_curr_number_me = ppp_gprs_info.sms_current_number_me;
		ppp_gprs_apn_pap_info.sms_max_number_me = ppp_gprs_info.sms_max_number_me;		
		sync_gprs_info();
	}
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gprs_call_gprs()
{
    u32_t ret = AT_RET_OK;
	
	PRINTF("\r\n^_^seq007.gprs_call_gprs\r\n");
	ppp_gprs_info.ppp_gprs_current_seq = SEQ_007;
    ret = modem_at_cmd_mipcall(&ppp_handle, AT_REPEAT_MIN, AT_GPRS_CALLTIMEOUT_MAX, WAIT_RETURN);
    if(ret != AT_RET_OK)
    {
		PRINTF("..call gprs fail..\r\n");
    }
	if(ret == AT_RET_OK)
	{
		ppp_gprs_info.ppp_negotiate_result = PPP_NEGOTIATE_OK;
	}
	return ret;
}

/*
*hao.yunran
*2016.10.10
*dial GPRS, if successful, we start LCP Config
*/
u32_t gprs_set_gprs_down()
{
    u32_t ret = AT_RET_OK;
	
    ret = modem_at_cmd_cgatt_down(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, NOT_WAIT);
    if(ret != AT_RET_OK)
    {

    }	
	return ret;
}

/*
*hao.yunran
*2017.2.15
*dial GPRS, if successful, we start LCP Config
*/
u32_t gprs_get_gprs_sign_test()
{
    u32_t ret = AT_RET_OK;
	
    ret = modem_at_cmd_csq(&ppp_handle, AT_REPEAT_MAX, AT_TIMEOUT_MAX, WAIT_RETURN);
    if(ret != AT_RET_OK)
    {

    }	
	return ret;
}




#endif







