#include "rtc_fun.h"
#include "FreeRTOS.h"
#include "task.h"
#include "rtc_fun.h"
#include "plc_if.h"
#include "uart_plc.h"

void get_current_systime(uint64_t *outts)
{
	static uint64_t lastst = 0;
	static uint32_t lasttick = 0;
	*outts = GET_RTC_SECONDS;
	if(*outts <= (lastst/1000)){//do this if arrive more than once between one second
		*outts = lastst + (xTaskGetTickCount() - lasttick)%1000;
		lasttick = xTaskGetTickCount();
		lastst = *outts;
		return;
	}
	(*outts) *= 1000;
	lasttick = xTaskGetTickCount();
	*outts += lasttick%1000;//get remainder of tick
	lastst = *outts;

}

void set_rtc_date(uint32_t seconds)
{
    RTC_StopTimer(RTC);

    RTC->TSR = seconds;
	RTC_StartTimer(RTC);

#ifdef PLC
#ifndef BRIDGE
	sync_bridge_rtc_time(bridge_list[0].id,++plc_local_seq);
#endif
#endif
}

void rtc_init(void)
{
	//rtc_datetime_t date;
	rtc_config_t rtcConfig;
	RTC_GetDefaultConfig(&rtcConfig);
	RTC_Init(RTC, &rtcConfig);
	//RTC_SetCapLoad(kOSC_Cap2P | kOSC_Cap8P);
	/* Select RTC clock source */
	set_rtc_clock_source();

    RTC_StartTimer(RTC);
}
