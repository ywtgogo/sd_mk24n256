/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"

#include "fsl_uart_freertos.h"
#include "fsl_uart.h"
#include "fsl_adc16.h"
#include "fsl_pflash_freertos.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aes256.h"
#include "sha1.h"
#include "spiflash_config.h"
#include "fota.h"
#include "rtc_fun.h"
#include "message.h"
#include "AT_execute.h"

#define DEMO_ADC16_BASE ADC0
#define DEMO_ADC16_CHANNEL_GROUP    0U
#define DEMO_ADC16_USER_CHANNEL     8U

RF_MSG HwQueryMsg;

void StrToHex(char *src, char *dst);

void HWTEST(char *buf, int len)
{
	if (0 == strcmp(buf, "NOR")) {
		spi_flash_test();
	}else if(0 == strcmp(buf, "AUDIO")){
		start_play_plan(1);
		PRINTF("OK\r\n");
	}else if(0 == strcmp(buf, "RTC")){
		PRINTF("%d\r\n", GET_RTC_SECONDS);
	}else if(0 == strcmp(buf, "RED")){
		PPP_RED_LED_INIT();
		PRINTF("OK\r\n");
	}else if(0 == strcmp(buf, "GREEN")){
		PPP_GREEN_LED_INIT();
		PRINTF("OK\r\n");
	}else if(0 == strcmp(buf, "WIFI")){
		//PRINTF("AUDIO");
	}else if(0 == strcmp(buf, "868TX")){
		SPI_A7139_TX_TEST();
		//PRINTF("AUDIO");
	}

}

void HWQUERY(char *buf, int len)
{
	if (0 == strcmp(buf, "CHARGING")) {
//		if (1==GPIO_ReadPinInput(BAT_nCHG_GPIO, BAT_nCHG_GPIO_PIN)){
//			PRINTF("OK,yes\r\n");
//		}else{
//			PRINTF("OK,no\r\n");
//		};
		PRINTF("OK,%s\r\n",GPIO_ReadPinInput(BAT_nCHG_GPIO, BAT_nCHG_GPIO_PIN)? "yes":"no");
	}else if(0 == strcmp(buf, "BAT")){
		check_battery();
	}else if(0 == strcmp(buf, "868STATUS")){
		printf("%s,%08x,%d\r\n", HwQueryMsg.sn_mitt? "OK":"ERR", HwQueryMsg.sn_mitt, HwQueryMsg.rssi);
		//PRINTF("%d\r\n", HwQueryMsg.rssi);
	}else if(0 == strcmp(buf, "WIFI")){

	}
}

int SETPRODUCEINFO(unsigned char *buf)
{
	int i;
	unsigned char hexbuf[12];

	for (i=0; i<8; i++)
	{
		hexbuf[i] = ((*buf++)-0x30)<<4;
		hexbuf[i] = hexbuf[i] | ((*buf++)-0x30);
	}

	StrToHex(buf, &hexbuf[i]);

	taskENTER_CRITICAL();
	FLASH_RTOS_Program(&pflash_handle, 0x410, (uint32_t *)hexbuf, 12);
	taskEXIT_CRITICAL();
	return 1;
}

FOTA_HBB_4KHEAD	Hbb_4kHead;
unsigned char	NorDataDeOut[4096], NorDataReadOut[4096];
int SETNORDATAIMG(unsigned char *buf, int addr, int b64_len)
{
	int			result=1;
	uint8_t		sha1_calc_out[CELL_SHA1_UNIT];
	uint8_t		sha1_temp[CELL_SHA1_UNIT];
	int 		base64_enlen=b64_len, base64_delen;
	int			sha1_offset;
#if 0
	for (int i=0; i<6; i++)
	{
		PRINTF("%x ", buf[i]);
	}

	base64_enlen = base64_encode(buf, 4096, NorDataDeOut);
	for (int i=0; i<base64_enlen; i++)
	{
		PRINTF("%x ", NorDataDeOut[i]);
	}
#endif
	base64_delen = base64_decode(buf, base64_enlen, NorDataDeOut);
//	for (int i=0; i<base64_delen; i++)
//	{
//		PRINTF("%x ", NorDataDeOut[i]);
//	}
	if (addr%0x1000!=0)
	{
		PRINTF("ERRIMGADDR\r\n");
		return -1;
	}

	if (addr==FOTA_HBB_IMG0_ADDR_START){
		memcpy(sha1_temp, NorDataDeOut, CELL_SHA1_UNIT);
		sha1_csum(&NorDataDeOut[32], FOTA_IMG_CELL_DOWN_LEN - 32U, sha1_calc_out);
		if (0 != memcmp(sha1_calc_out, sha1_temp ,(size_t)CELL_SHA1_UNIT)) {
			PRINTF("sha1 check error , addr=%d \r\n", addr);
			return -1;
		}
		else {
			memcpy(&Hbb_4kHead, NorDataDeOut, 4096);
			//PRINTF("sha1 check pass , addr=%d \r\n", addr);
		}
	}else{
		sha1_offset = CELL_SHA1_UNIT*(((addr-FOTA_HBB_IMG0_ADDR_START)/4096)-1);
		memcpy(sha1_temp, Hbb_4kHead.block_sha1+sha1_offset, CELL_SHA1_UNIT);
		sha1_csum(NorDataDeOut, FOTA_IMG_CELL_DOWN_LEN, sha1_calc_out );
		if (0 != memcmp(sha1_calc_out, sha1_temp ,(size_t)CELL_SHA1_UNIT)){
			PRINTF("sha1 check error, addr=%d \r\n", addr);
			return -1;
		}else {
			//PRINTF("sha1 check pass , addr=%d \r\n", addr);
		}
	}

	norflash_sector_erase_ll(&flash_master_rtos_handle, addr);
	norflash_write_data_ll(&flash_master_rtos_handle, addr, sizeof(NorDataDeOut), NorDataDeOut);
	norflash_read_data_ll(&flash_master_rtos_handle, addr, sizeof(NorDataReadOut), NorDataReadOut);
	if (0 == strncmp(NorDataReadOut, NorDataDeOut, 4096)){
		//PRINTF("OK NOR DATA\r\n");
		result = 1;
	}else {
		PRINTF("ERRIMGRW\r\n");
		result = -1;
	}

	norflash_sector_erase_ll(&flash_master_rtos_handle, addr+FOTA_HBB_IMGN_OFFSET);
	norflash_write_data_ll(&flash_master_rtos_handle, addr+FOTA_HBB_IMGN_OFFSET, sizeof(NorDataDeOut), NorDataDeOut);
	norflash_read_data_ll(&flash_master_rtos_handle, addr+FOTA_HBB_IMGN_OFFSET, sizeof(NorDataReadOut), NorDataReadOut);
	if (0 == strncmp(NorDataReadOut, NorDataDeOut, 4096)){
		//PRINTF("OK NOR DATA\r\n");
		result = 1;
	}else {
		PRINTF("ERRIMGRW\r\n");
		result = -1;
	}
	return result;
	//send result to at run
}

int SETNORDATAAUDIO(unsigned char *buf, int addr, int b64_len)	//(base64+sha1)->sha1+base64->sha1cmp->debase64
{
	uint8_t		sha1_calc_out[CELL_SHA1_UNIT];
	uint8_t		sha1_temp[CELL_SHA1_UNIT*2+1];
	uint8_t		sha1_temp_read[CELL_SHA1_UNIT*2];
	uint32_t    base64_enlen=b64_len-(CELL_SHA1_UNIT*2), base64_delen;

	memcpy(sha1_temp_read, buf+base64_enlen, CELL_SHA1_UNIT*2);
	sha1_csum(buf, base64_enlen, sha1_calc_out );

	hex_to_str(sha1_temp, sha1_calc_out, CELL_SHA1_UNIT);
	if (0 != memcmp(sha1_temp, sha1_temp_read, CELL_SHA1_UNIT*2))
	{
		PRINTF("sha1 check error, addr=%d \r\n", addr);
		return -1;
	}

	base64_delen = base64_decode(buf, base64_enlen, NorDataDeOut);

	if (addr%0x1000!=0)
	{
		PRINTF("ERRAUDIOADDR\r\n");
		return -1;
	}

	norflash_sector_erase_ll(&flash_master_rtos_handle, addr);
	norflash_write_data_ll(&flash_master_rtos_handle, addr, sizeof(NorDataDeOut), NorDataDeOut);
	norflash_read_data_ll(&flash_master_rtos_handle, addr, sizeof(NorDataReadOut), NorDataReadOut);
	if (0 == strncmp(NorDataReadOut, NorDataDeOut, 4096)){
		//PRINTF("OK NOR DATA\r\n");
		return 1;
	}else {
		PRINTF("ERRAUDIORW\r\n");
		return -1;
	}
	return 1;
}

//int SETNORDATAAUDIO(unsigned char *buf, int addr, int b64_len)
//{
//	uint8_t		sha1_calc_out[CELL_SHA1_UNIT];
//	uint8_t		sha1_temp[CELL_SHA1_UNIT];
//	uint32_t    base64_enlen=b64_len, base64_delen;
//
//	base64_delen = base64_decode(buf, base64_enlen, NorDataDeOut);
//
//	if (addr%0x1000!=0)
//	{
//		PRINTF("ERR NOR ADDR\r\n");
//		return;
//	}
//
//	memcpy(sha1_temp, buf+base64_enlen, CELL_SHA1_UNIT);
//	sha1_csum(NorDataDeOut, FOTA_IMG_CELL_DOWN_LEN, sha1_calc_out );
//	if (0 != memcmp(sha1_calc_out, sha1_temp ,(size_t)CELL_SHA1_UNIT)){
//		PRINTF("sha1 check error, addr=%d \r\n", addr);
//	}else {
//		//PRINTF("sha1 check pass , addr=%d \r\n", addr);
//	}
//
//	norflash_sector_erase_ll(&flash_master_rtos_handle, addr);
//	norflash_write_data_ll(&flash_master_rtos_handle, addr, sizeof(NorDataDeOut), NorDataDeOut);
//	norflash_read_data_ll(&flash_master_rtos_handle, addr, sizeof(NorDataReadOut), NorDataReadOut);
//	if (0 == strncmp(NorDataReadOut, NorDataDeOut, 4096)){
//		PRINTF("OK NOR DATA\r\n");
//		return 1;
//	}else {
//		PRINTF("ERR NOR DATA\r\n");
//		return -1;
//	}
//}


void check_battery()
{
	uint32_t 				g_Adc16ConversionValue;
	adc16_config_t 			adc16ConfigStruct;
    adc16_channel_config_t 	adc16ChannelConfigStruct;

    ADC16_GetDefaultConfig(&adc16ConfigStruct);
    ADC16_Init(DEMO_ADC16_BASE, &adc16ConfigStruct);
    ADC16_EnableHardwareTrigger(DEMO_ADC16_BASE, false); /* Make sure the software trigger is used. */
#if defined(FSL_FEATURE_ADC16_HAS_CALIBRATION) && FSL_FEATURE_ADC16_HAS_CALIBRATION
    if (kStatus_Success == ADC16_DoAutoCalibration(DEMO_ADC16_BASE))
    {

    }
    else
    {
        PRINTF("ADC16_DoAutoCalibration() Failed.\r\n");
    }
#endif /* FSL_FEATURE_ADC16_HAS_CALIBRATION */
    adc16ChannelConfigStruct.channelNumber = DEMO_ADC16_USER_CHANNEL;
    adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = false;
#if defined(FSL_FEATURE_ADC16_HAS_DIFF_MODE) && FSL_FEATURE_ADC16_HAS_DIFF_MODE
    adc16ChannelConfigStruct.enableDifferentialConversion = false;
#endif /* FSL_FEATURE_ADC16_HAS_DIFF_MODE */


	ADC16_SetChannelConfig(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP, &adc16ChannelConfigStruct);
	while (0U == (kADC16_ChannelConversionDoneFlag & ADC16_GetChannelStatusFlags(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP)))
	{
	}
	g_Adc16ConversionValue = ADC16_GetChannelConversionValue(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP);
	if (2000 > 3300*g_Adc16ConversionValue/4095*1470/1000) PRINTF("ERR %d\r\n", ERR_BAT);
	//PRINTF("ADC Value: %d  BAT Voltage: %d\r\n", g_Adc16ConversionValue, 3300*g_Adc16ConversionValue/4095*1470/1000);
	//no_charging = GPIO_ReadPinInput(BAT_nCHG_GPIO, BAT_nCHG_GPIO_PIN);
	//PRINTF("%s Charger\r\n", no_charging?"Has no ":"HAS");

}

int km_uart_test(void)
{
	uart_rtos_handle_t km_handle;
	struct _uart_handle km_t_handle;
    int error;
    size_t n;
    uint8_t background_buffer[64];
    uint8_t recv_buffer[1];

    struct rtos_uart_config uart_config = {
        .baudrate = 115200,
        .parity = kUART_ParityDisabled,
        .stopbits = kUART_OneStopBit,
        .buffer = background_buffer,
        .buffer_size = sizeof(background_buffer),
    };

    uart_config.srcclk = CLOCK_GetFreq(MODEM_UART_CLKSRC);
    uart_config.base = MODEM_UART;
    NVIC_SetPriority(CONSOLE_UART_RX_TX_IRQn, 8);

    if (0 > UART_RTOS_Init(&km_handle, &km_t_handle, &uart_config))
    {
        PRINTF("Error during UART initialization.\r\n");
        vTaskSuspend(NULL);
    }

    /* Send some data */
    if (0 > UART_RTOS_Send(&km_handle, (uint8_t *)"AT\r\n", strlen("AT\r\n")))
    {
        PRINTF("Error during UART send.\r\n");
        vTaskSuspend(NULL);
    }

//    KM_POWER_INIT();
//    KM_POWER_ON();//拉低开GSM
//	vTaskDelay(6000);
//	KM_POWER_OFF();

    while(1)
    {
        error = UART_RTOS_Receive(&km_handle, recv_buffer, sizeof(recv_buffer), &n);
        if (n > 0)
        {
            PRINTF("%c",recv_buffer[0]);
            if (0 == strncmp("K", (char *)recv_buffer, 1))
            {
                PRINTF("\r\n");
                break;
            }
        }
    }
    UART_RTOS_Deinit(&km_handle);
}


void StrToHex(char *src, char *dst)    //字符串转换为HEX码
{
	int i,j=0;

	for(i=0;i<strlen(src);i+=2)//strlen(src)/2
	{
		if (src[i]>='0'&&src[i]<='9'){
			dst[j] = (src[i]-'0')<<4;
		}else if (src[i]>='A'&&src[i]<='Z'){
			dst[j] = (src[i]-'A'+10)<<4;
		}else if (src[i]>='a'&&src[i]<='z'){
			dst[j] = (src[i]-'a'+10)<<4;
		}
		if (src[i+1]>='0'&&src[i+1]<='9'){
			dst[j] = dst[j]|(src[i+1]-'0');
		}else if (src[i+1]>='A'&&src[i+1]<='Z'){
			dst[j] = dst[j]|(src[i+1]-'A'+10);
		}else if (src[i+1]>='a'&&src[i+1]<='z'){
			dst[j] = dst[j]|(src[i+1]-'a'+10);
		}
		j++;
	}
}
