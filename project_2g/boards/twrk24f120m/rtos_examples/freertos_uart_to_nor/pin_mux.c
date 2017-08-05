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

#include "fsl_common.h"
#include "fsl_port.h"
#include "pin_mux.h"
#include "fsl_gpio.h"

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_InitPins(void)
{
    CLOCK_EnableClock(kCLOCK_PortA);
    CLOCK_EnableClock(kCLOCK_PortD);
    PORT_SetPinMux(PORTA,  5U, kPORT_MuxAsGpio);
    GPIO_PinInit(GPIOA, 5U, &(gpio_pin_config_t){kGPIO_DigitalOutput, (0)});    
    
    /* Initialize UART5 pins below -> DEBUG*/
    PORT_SetPinMux(PORTD,  8U, kPORT_MuxAlt3);
    PORT_SetPinMux(PORTD,  9U, kPORT_MuxAlt3);
    
// * port_pin_config_t config = {
// *      kPORT_PullUp,
// *      kPORT_FastSlewRate,
// *      kPORT_PassiveFilterDisable,
// *      kPORT_OpenDrainDisable,
// *      kPORT_LowDriveStrength,
// *      kPORT_MuxAsGpio,
// *      kPORT_UnLockRegister,
// * };
    /* spi1 for spi_flash */
    //PORT_SetPinMux(PORTD, 4u, kPORT_MuxAlt7);
    PORT_SetPinConfig(PORTD, 4U, &(port_pin_config_t){
        kPORT_PullUp, 
        kPORT_SlowSlewRate,
        kPORT_PassiveFilterDisable,
        kPORT_OpenDrainDisable,
        kPORT_LowDriveStrength,
        kPORT_MuxAlt7,
        kPORT_UnlockRegister,        
    });
    //PORT_SetPinMux(PORTD, 5u, kPORT_MuxAlt7);
    PORT_SetPinConfig(PORTD, 5U, &(port_pin_config_t){
        kPORT_PullUp, 
        kPORT_SlowSlewRate,
        kPORT_PassiveFilterDisable,
        kPORT_OpenDrainDisable,
        kPORT_LowDriveStrength,
        kPORT_MuxAlt7,
        kPORT_UnlockRegister,        
    });    
    //PORT_SetPinMux(PORTD, 6u, kPORT_MuxAlt7);
    PORT_SetPinConfig(PORTD, 6U, &(port_pin_config_t){
        kPORT_PullUp, 
        kPORT_SlowSlewRate,
        kPORT_PassiveFilterDisable,
        kPORT_OpenDrainDisable,
        kPORT_LowDriveStrength,
        kPORT_MuxAlt7,
        kPORT_UnlockRegister,        
    });        
    //PORT_SetPinMux(PORTD, 7u, kPORT_MuxAlt7);
    PORT_SetPinConfig(PORTD, 7U, &(port_pin_config_t){
        kPORT_PullUp, 
        kPORT_SlowSlewRate,
        kPORT_PassiveFilterDisable,
        kPORT_OpenDrainDisable,
        kPORT_LowDriveStrength,
        kPORT_MuxAlt7,
        kPORT_UnlockRegister,        
    });        
}
