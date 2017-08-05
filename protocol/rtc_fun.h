#ifndef _RTC_FUN_H
#define _RTC_FUN_H
#include "fsl_rtc.h"
#include "fsl_debug_console.h"
#define GET_RTC_SECONDS  RTC->TSR

static inline void RTC_SetCapLoad(uint8_t capLoad)
{
    uint8_t reg = RTC->CR;
    reg &= ~(OSC_CR_SC2P_MASK | OSC_CR_SC4P_MASK | OSC_CR_SC8P_MASK | OSC_CR_SC16P_MASK);
    reg |= capLoad;
    RTC->CR = reg;
}

static inline void set_rtc_clock_source(void)
{
    /* Enable the RTC 32KHz oscillator */
    RTC->CR |= RTC_CR_OSCE_MASK;
}

void get_current_systime(uint64_t *outts);
void rtc_init(void);
void set_rtc_date(uint32_t seconds);


#endif
