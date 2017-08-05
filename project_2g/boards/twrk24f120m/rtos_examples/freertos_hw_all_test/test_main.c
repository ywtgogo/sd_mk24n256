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
#include "fsl_shell.h"

//#include "test_common.h"
#include "pin_mux.h"
#include "clock_config.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uart_rtos_handle_t silver_handle;
struct _uart_handle silver_t_handle;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Application entry point.
 */
extern void shell_task(void *pvParameters);
extern void plc_get_msg_task(void *pvParameters);
extern void plc_put_msg_task(void *pvParameters);
extern void DrvPlcFsm(void *pvParameters);
int main(void)
{
    
    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    
    K24_POWER_LOCK_INIT();
    SPKR_GPIO_INIT();
    SPKR_ON();
    /* Init led */
    Led_Init();
    //ms_delay(10000);
    BQ25050_MaxOutputSet(11);
//    ms_delay(10000);
//    BQ25050_MaxOutputSet(1);
//    
//    //while(1);

    adc16_batery_init();
    
    NVIC_SetPriority( KM_UART_RX_TX_IRQn, 6);
    NVIC_SetPriority(PLC_UART_RX_TX_IRQn, 7);
//    NVIC_SetPriority(SILVER_UART_RX_TX_IRQn, 6);
    NVIC_SetPriority(CONSOLE_UART_RX_TX_IRQn, 8);

//    xTaskCreate(plc_put_msg_task, "Plc_put_msg_task", configMINIMAL_STACK_SIZE * 4, NULL, configMAX_PRIORITIES-4, NULL);
    xTaskCreate(plc_get_msg_task, "Plc_get_msg_task", configMINIMAL_STACK_SIZE * 4, NULL, configMAX_PRIORITIES-2, NULL);
    xTaskCreate(DrvPlcFsm, "Drv_Plc_Fsm_task", configMINIMAL_STACK_SIZE * 3, NULL, configMAX_PRIORITIES-3, NULL);       //状态机优先级需要高于TX
//    xTaskCreate(silver_uart_task, "Silver_uart_task_PRIORITY", configMINIMAL_STACK_SIZE    , NULL,  silver_uart_task_PRIORITY, NULL);
    
    xTaskCreate(shell_task,  "Shell_task",   configMINIMAL_STACK_SIZE * 30, NULL,  configMAX_PRIORITIES-6, NULL);
    
    vTaskStartScheduler();
    for (;;)
        ;
    
}