#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "adc.h"
#include "utilities/vector_table_info.h"

#include "fsl_debug_console.h"
#include "fsl_pmc.h"

//! @brief Returns the user application address and stack pointer.
//!
//! For flash-resident and rom-resident target, gets the user application address
//! and stack pointer from the APP_VECTOR_TABLE.
//! Ram-resident version does not support jumping to application address.
static void get_user_application_entry(uint32_t *appEntry, uint32_t *appStack)
{
    assert(appEntry);
    assert(appStack);

    *appEntry = APP_VECTOR_TABLE[kInitialPC];
    *appStack = APP_VECTOR_TABLE[kInitialSP];
}

static void jump_to_application(uint32_t applicationAddress, uint32_t stackPointer)
{
//    shutdown_cleanup(kShutdownType_Shutdown);

    // Create the function call to the user application.
    // Static variables are needed since changed the stack pointer out from under the compiler
    // we need to ensure the values we are using are not stored on the previous stack
    static uint32_t s_stackPointer = 0;
    s_stackPointer = stackPointer;
    static void (*farewellBootloader)(void) = 0;
    farewellBootloader = (void (*)(void))applicationAddress;

    // Set the VTOR to the application vector table address.
    SCB->VTOR = (uint32_t)APP_VECTOR_TABLE;

    // Set stack pointers to the application stack pointer.
    __set_MSP(s_stackPointer);
    __set_PSP(s_stackPointer);

    // Jump to the application.
    farewellBootloader();
    // Dummy fcuntion call, should never go to this fcuntion call
//    shutdown_cleanup(kShutdownType_Shutdown);
}

void check_battery()
{
    adc16_config_t adc16ConfigStruct;
    adc16_channel_config_t adc16ChannelConfigStruct;

	PRINTF("\r\nSTEP 1 : battery adc check\r\n");
	pmc_bandgap_buffer_config_t pmc_bandgap_buffer_config;
	pmc_bandgap_buffer_config.enable = true;
	PMC_ConfigureBandgapBuffer(PMC,&pmc_bandgap_buffer_config);


	EnableIRQ(DEMO_ADC16_IRQn);
	ADC16_GetDefaultConfig(&adc16ConfigStruct);
	ADC16_Init(DEMO_ADC16_BASE, &adc16ConfigStruct);
	ADC16_EnableHardwareTrigger(DEMO_ADC16_BASE, false);
#if defined(FSL_FEATURE_ADC16_HAS_CALIBRATION) && FSL_FEATURE_ADC16_HAS_CALIBRATION
    if (kStatus_Success == ADC16_DoAutoCalibration(DEMO_ADC16_BASE))
    {
        PRINTF("ADC16_DoAutoCalibration() Done.\r\n");
    }
    else
    {
        PRINTF("ADC16_DoAutoCalibration() Failed.\r\n");
    }
#endif /* FSL_FEATURE_ADC16_HAS_CALIBRATION */

	adc16ChannelConfigStruct.channelNumber = ADC_PDD_SINGLE_ENDED_BANDGAP;
	adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = true;

#if defined(FSL_FEATURE_ADC16_HAS_DIFF_MODE) && FSL_FEATURE_ADC16_HAS_DIFF_MODE
    adc16ChannelConfigStruct.enableDifferentialConversion = false;
#endif /* FSL_FEATURE_ADC16_HAS_DIFF_MODE */


    g_Adc16ConversionDoneFlag = false;

    ADC16_SetChannelConfig(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP, &adc16ChannelConfigStruct);
    while (!g_Adc16ConversionDoneFlag);
    PRINTF("ADC Value: %d\r\n", 4096*1000/g_Adc16ConversionValue);

    uint32_t charging;
    uint32_t charger;
    charger = !GPIO_ReadPinInput(GPIOB, 20);
    PRINTF("%s charger\r\n", charger?"Yes":"No");
    charging = !GPIO_ReadPinInput(GPIOB, 6);
    PRINTF("%s Charging\r\n", charging?"Yes":"No");

}

void record_reset_reason()
{

}

bool check_image_has_update()
{
	return 1;
}
bool check_image_right()
{
	return 1;
}
void main()
{
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    PRINTF("This is bootloader\r\n");

    check_battery();

    record_reset_reason();

    if(check_image_has_update())
    {
    	check_spi_flash_image();
    	//write image into flash,check flash image's sha1
    	//clear update sign
    	//record result into boot cache
    }

    if(check_image_right())
    {
    	//check flash image's sha1
    }



	uint32_t applicationAddress, stackPointer;
	get_user_application_entry(&applicationAddress, &stackPointer);
	jump_to_application(applicationAddress, stackPointer);

	return 0;
}
