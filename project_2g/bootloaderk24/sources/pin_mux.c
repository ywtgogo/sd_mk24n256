/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "fsl_port.h"
#include "pin_mux.h"
#include "fsl_common.h"
#include "board.h"
/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_InitPins(void)
{

    CLOCK_EnableClock(kCLOCK_PortA);
    CLOCK_EnableClock(kCLOCK_PortB);
    CLOCK_EnableClock(kCLOCK_PortD);
    CLOCK_EnableClock(kCLOCK_PortE);

#if (BOARD_NAME == HW_V01)
    PORT_SetPinMux(PORTA, 14U, kPORT_MuxAlt3);
    PORT_SetPinMux(PORTA, 15U, kPORT_MuxAlt3);
#else
    PORT_SetPinMux(PORTD,  8U, kPORT_MuxAlt3);
    PORT_SetPinMux(PORTD,  9U, kPORT_MuxAlt3);
#endif

    PORT_SetPinMux(PORTD, 15U, kPORT_MuxAsGpio);		//New board PTE4
	PORT_SetPinMux(PORTE,  4U, kPORT_MuxAsGpio);		//New board PTE4

    port_pin_config_t config_poweroff = {0};
    config_poweroff.pullSelect = kPORT_PullDisable;
    config_poweroff.mux = kPORT_MuxAsGpio;
    PORT_SetPinConfig(PORTE, 6U, &config_poweroff);     //8051检测到关机按键后通知K24
    /* Configure pin for ADC example. */

    PORT_SetPinMux(PORTB, 0U, kPORT_PinDisabledOrAnalog);
    PORT_SetPinMux(PORTB, 6U, kPORT_MuxAsGpio);
    PORT_SetPinMux(PORTB, 20U, kPORT_MuxAsGpio);
#if (BOARD_NAME == HW_V01)
    /* Led pin mux Configuration */
    PORT_SetPinMux(PORTD, 14U, kPORT_MuxAsGpio);
    PORT_SetPinMux(PORTD, 15U, kPORT_MuxAsGpio);
#endif
    /* SPI1 */
    PORT_SetPinMux(PORTD, 4U, kPORT_MuxAlt7);
    PORT_SetPinMux(PORTD, 5U, kPORT_MuxAlt7);
    PORT_SetPinMux(PORTD, 6U, kPORT_MuxAlt7);
    PORT_SetPinMux(PORTD, 7U, kPORT_MuxAlt7);


}
