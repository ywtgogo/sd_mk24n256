/*
 * fsl_plash_freertos.h
 *
 *  Created on: 2016Äê6ÔÂ29ÈÕ
 *      Author: weitao.yang
 */

#ifndef _FSL_PFLASH_FREERTOS_H_
#define _FSL_PFLASH_FREERTOS_H_

#include "fsl_flash.h"
#include "semphr.h"

typedef struct _flash_rtos_handle
{
    flash_config_t drv_handle;
    SemaphoreHandle_t mutex;
} flash_rtos_handle_t;

status_t FLASH_RTOS_Load(flash_rtos_handle_t *handle, uint32_t start, uint32_t *dst, uint32_t lengthInBytes);
status_t FLASH_RTOS_Program(flash_rtos_handle_t *handle, uint32_t start, uint32_t *src, uint32_t lengthInBytes);
status_t FLASH_RTOS_Erase_Sector(flash_rtos_handle_t *handle, uint32_t destAdrss, uint32_t sector_count);
status_t FLASH_RTOS_Init(flash_rtos_handle_t *handle);

#endif
