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

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "fsl_flash.h"

#include "pin_mux.h"
#include "clock_config.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Task priorities. */
#define flashrw_task_PRIORITY (configMAX_PRIORITIES - 1)


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

typedef struct _flash_rtos_handle
{
    flash_config_t drv_handle;
    SemaphoreHandle_t mutex;
} flash_rtos_handle_t;


void error_trap(void)
{
    PRINTF("\r\n\r\n\r\n\t---- HALTED DUE TO FLASH ERROR! ----\r\n");
}

status_t FLASH_RTOS_Init(flash_rtos_handle_t *handle)
{
    status_t result;    /* Return code from each flash driver function */
    flash_security_state_t securityStatus = kFLASH_SecurityStateNotSecure; /* Return protection status */

    uint32_t pflashBlockBase = 0;
    uint32_t pflashTotalSize = 0;
    uint32_t pflashSectorSize = 0;
    
    if (handle == NULL)
    {
        return kStatus_InvalidArgument;
    }
    memset(&handle->drv_handle, 0, sizeof(flash_config_t));
    
    handle->mutex = xSemaphoreCreateMutex();
    if (handle->mutex == NULL)
    {
        return kStatus_Fail;
    }
    
    /* Setup flash driver structure for device and initialize variables. */
    result = FLASH_Init(&handle->drv_handle);
    if (kStatus_FLASH_Success != result)
    {
        error_trap();
    }
    
    
    /* Get flash properties*/
    FLASH_GetProperty(&handle->drv_handle, kFLASH_PropertyPflashBlockBaseAddr, &pflashBlockBase);
    FLASH_GetProperty(&handle->drv_handle, kFLASH_PropertyPflashTotalSize, &pflashTotalSize);
    FLASH_GetProperty(&handle->drv_handle, kFLASH_PropertyPflashSectorSize, &pflashSectorSize);
    
//    /* Print flash information - PFlash. */
//    PRINTF("\r\n Flash Information: ");
//    PRINTF("\r\n pflashBlockBase:\t%d , Hex: (0x%x)", pflashBlockBase, pflashBlockBase);
//    PRINTF("\r\n Total Program Flash Size:\t%d KB, Hex: (0x%x)", (pflashTotalSize / 1024), pflashTotalSize);
//    PRINTF("\r\n Program Flash Sector Size:\t%d KB, Hex: (0x%x) ", (pflashSectorSize / 1024), pflashSectorSize);

    /* Check security status. */
    result = FLASH_GetSecurityState(&handle->drv_handle, &securityStatus);
    if (kStatus_FLASH_Success != result)
    {
        error_trap();
    }

    /* Print security status. */
    switch (securityStatus)
    {
        case kFLASH_SecurityStateNotSecure:
            //PRINTF("\r\n Flash is UNSECURE!");
            break;
        case kFLASH_SecurityStateBackdoorEnabled:
            PRINTF("\r\n Flash is SECURE, BACKDOOR is ENABLED!");
            break;
        case kFLASH_SecurityStateBackdoorDisabled:
            PRINTF("\r\n Flash is SECURE, BACKDOOR is DISABLED!");
            break;
        default:
            break;
    }
    PRINTF("\r\n");

    return kStatus_Success;
}

status_t FLASH_RTOS_Erase_Sector(flash_rtos_handle_t *handle, uint32_t destAdrss, uint32_t sector_count)
{
    status_t result;
    if (destAdrss%4096 != 0)
    {
        PRINTF("\r\n\r\n\r\n\t---- Erase ERROR! Sector Addr Error!----\r\n");
        return -1;
    }
    
    if (xSemaphoreTake(handle->mutex, portMAX_DELAY) != pdTRUE)
    {
        PRINTF("kStatus_FLASH_Busy \r\n");
    }    
    for (uint32_t i=0; i<sector_count; i++)
    {
        result = FLASH_Erase(&handle->drv_handle, destAdrss+(i*4096), 4096, kFLASH_ApiEraseKey);
        if (kStatus_FLASH_Success != result)
        {
            error_trap();
        }
        
        /* Verify sector if it's been erased. */
        result = FLASH_VerifyErase(&handle->drv_handle, destAdrss+(i*4096), 4096, kFLASH_MarginValueUser);//destAdrss*(i+1)
        if (kStatus_FLASH_Success != result)
        {
            error_trap();
        }       
    }
    /* Unlock resource mutex */
    xSemaphoreGive(handle->mutex);
    
    return result;
}



status_t FLASH_RTOS_Program(flash_rtos_handle_t *handle, uint32_t start, uint32_t *src, uint32_t lengthInBytes)
{
    
    status_t result;    /* Return code from each flash driver function */
    uint32_t failAddr, failDat;
    
    if (xSemaphoreTake(handle->mutex, portMAX_DELAY) != pdTRUE)
    {
        PRINTF("kStatus_FLASH_Busy \r\n");
    }
    
    result = FLASH_Program(&handle->drv_handle, start, src, lengthInBytes);
    if (kStatus_FLASH_Success != result)
    {
        error_trap();
    }
    /* Program Check user margin levels */
    result = FLASH_VerifyProgram(&handle->drv_handle, start, lengthInBytes, src, kFLASH_MarginValueUser, &failAddr,
                                 &failDat);
    if (kStatus_FLASH_Success != result)
    {
        error_trap();
    }

    /* Unlock resource mutex */
    xSemaphoreGive(handle->mutex);
    
    return result;
}

status_t FLASH_RTOS_Load(flash_rtos_handle_t *handle, uint32_t start, uint32_t *dst, uint32_t lengthInBytes)
{
    status_t result;    /* Return code from each flash driver function */    
    if (xSemaphoreTake(handle->mutex, portMAX_DELAY) != pdTRUE)
    {
        PRINTF("kStatus_FLASH_Busy \r\n");
    }

    result = FLASH_Read(&handle->drv_handle, start, (uint8_t *)dst, lengthInBytes);
    if (kStatus_FLASH_Success != result)
    {
        error_trap();
    }
    
    /* Unlock resource mutex */
    xSemaphoreGive(handle->mutex);

    return result;
}

/*!
 * @brief Task responsible for printing of "Hello world." message.
 */
void flashrw_task(void *pvParameters)
{
    flash_rtos_handle_t fd;
    uint8_t buf[] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t'};
    FLASH_RTOS_Init(&fd);
    
    taskENTER_CRITICAL();
    FLASH_RTOS_Erase_Sector(&fd, 0x3f000, 1);
    
    //FLASH_RTOS_Program(&fd, 0xff000, (uint32_t*)buf, 0x10);
    for (uint32_t i=0; i<60; i++)
    {
        FLASH_RTOS_Program(&fd, 0x3f000+(0x20*i), (uint32_t*)buf, 0x10);
    }
    memset(buf, 0, 20);
    FLASH_RTOS_Load(&fd, 0x3f000, (uint32_t*)buf, 0x10);
    
    taskEXIT_CRITICAL();

    for (;;)
    {
        PRINTF("Hello world.\r\n");
        vTaskSuspend(NULL);
    }
}

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Application entry point.
 */
int pflash_main(void)
{
    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    if (xTaskCreate(flashrw_task, "FlashRW_task", configMINIMAL_STACK_SIZE, NULL, flashrw_task_PRIORITY, NULL) != pdPASS)
    {
        PRINTF("Failed to create flashrw task");
    }
    vTaskStartScheduler();
    for (;;)
        ;
}
