/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
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

#include "fsl_debug_console.h"
#include "board.h"
#include "fsl_dac.h"

#include "fsl_common.h"
#include "clock_config.h"
#include "pin_mux.h"
#include "fsl_lptmr.h"
#include "fsl_dmamux.h"
#include "fsl_edma.h"

#include "wav_record.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_DAC_BASE DAC0

#define DEMO_DMA_BASEADDR DMA0
#define DEMO_DMA_IRQ_ID DMA0_IRQn
#define DEMO_DMA_CHANNEL 0U
#define DEMO_DMA_DAC_SOURCE kDmaRequestMux0DAC0

#define DEMO_DMAMUX_BASEADDR DMAMUX0

#define DEMO_LPTMR_BASE LPTMR0


#define DEMO_LPTMR_COMPARE_VALUE 100U /* Low Power Timer interrupt time in miliseconds */
#define DEMO_DAC_SAMPLE_COUNT 16U   /* The ADC16 sample count */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
edma_handle_t g_EDMA_Handle;                                     /* Edma handler */
edma_transfer_config_t g_transferConfig;                         /* Edma transfer config */
static uint32_t g_dacSampleDataArray[DEMO_DAC_SAMPLE_COUNT]; /* ADC value array */

uint16_t Audio_Source_Blk_A[1024];
uint16_t Audio_Source_Blk_B[1024];

/*******************************************************************************
 * Code
 ******************************************************************************/
static void Edma_Callback(edma_handle_t *handle, void *userData, bool transferDone, uint32_t tcds)
{
//    /* Stop trigger */
//    LPTMR_StopTimer(DEMO_LPTMR_BASE);
//    /* Clear Edma interrupt flag */
//    EDMA_ClearChannelStatusFlags(DEMO_DMA_BASEADDR, DEMO_DMA_CHANNEL, kEDMA_InterruptFlag);
//    /* Setup transfer */
//    EDMA_PrepareTransfer(&g_transferConfig, (void *)ADC16_RESULT_REG_ADDR, sizeof(uint32_t),
//                         (void *)g_dacSampleDataArray, sizeof(uint32_t), sizeof(uint32_t),
//                         sizeof(g_dacSampleDataArray), kEDMA_PeripheralToMemory);
//    EDMA_SetTransferConfig(DEMO_DMA_BASEADDR, DEMO_DMA_CHANNEL, &g_transferConfig, NULL);
//    /* Enable transfer */
//    EDMA_StartTransfer(&g_EDMA_Handle);
}

/* Enable the trigger source of LPTimer */
static void LPTMR_Configuration(void)
{
    lptmr_config_t lptmrUserConfig;

    LPTMR_GetDefaultConfig(&lptmrUserConfig);
    /* Init LPTimer driver */
    LPTMR_Init(DEMO_LPTMR_BASE, &lptmrUserConfig);

    /* Set the LPTimer period */
    LPTMR_SetTimerPeriod(DEMO_LPTMR_BASE, DEMO_LPTMR_COMPARE_VALUE);
}

static void EDMA_Configuration(void)
{
//    edma_config_t userConfig;
//
//    /* Configure DMAMUX */
//    DMAMUX_Init(DEMO_DMAMUX_BASEADDR);
//    DMAMUX_SetSource(DEMO_DMAMUX_BASEADDR, DEMO_DMA_CHANNEL, DEMO_DMA_DAC_SOURCE); /* Map ADC source to channel 0 */
//    DMAMUX_EnableChannel(DEMO_DMAMUX_BASEADDR, DEMO_DMA_CHANNEL);
//
//    EDMA_GetDefaultConfig(&userConfig);
//    EDMA_Init(DEMO_DMA_BASEADDR, &userConfig);
//    EDMA_CreateHandle(&g_EDMA_Handle, DEMO_DMA_BASEADDR, DEMO_DMA_CHANNEL);
//    EDMA_SetCallback(&g_EDMA_Handle, Edma_Callback, NULL);
//    EDMA_PrepareTransfer(&g_transferConfig, (void *)ADC16_RESULT_REG_ADDR, sizeof(uint32_t),
//                         (void *)g_dacSampleDataArray, sizeof(uint32_t), sizeof(uint32_t),
//                         sizeof(g_dacSampleDataArray), kEDMA_PeripheralToMemory);
//    EDMA_SubmitTransfer(&g_EDMA_Handle, &g_transferConfig);
//    /* Enable interrupt when transfer is done. */
//    EDMA_EnableChannelInterrupts(DEMO_DMA_BASEADDR, DEMO_DMA_CHANNEL, kEDMA_MajorInterruptEnable);
//#if defined(FSL_FEATURE_EDMA_ASYNCHRO_REQUEST_CHANNEL_COUNT) && FSL_FEATURE_EDMA_ASYNCHRO_REQUEST_CHANNEL_COUNT
//    /* Enable async DMA request. */
//    EDMA_EnableAsyncRequest(DEMO_DMA_BASEADDR, DEMO_DMA_CHANNEL, true);
//#endif /* FSL_FEATURE_EDMA_ASYNCHRO_REQUEST_CHANNEL_COUNT */
//    /* Enable transfer. */
//    EDMA_StartTransfer(&g_EDMA_Handle);
//    /* Enable IRQ. */
//    NVIC_EnableIRQ(DEMO_DMA_IRQ_ID);
}



// K64F DAC sine wave generator
// (c) Nadler & Associates - DRNadler 12-Oct-2016 - DRN@nadler.com

#include <math.h>

#include "fsl_dac.h"
#include "fsl_pit.h"
#include "fsl_dmamux.h"
#include "fsl_edma.h"

volatile bool irq_flag = true;

void PIT0_IRQHandler(void)
{
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
}

void DAC0_IRQHandler(void)
{
    uint32_t flags = DAC_GetBufferStatusFlags(DEMO_DAC_BASE);

//#if defined(FSL_FEATURE_DAC_HAS_WATERMARK_SELECTION) && FSL_FEATURE_DAC_HAS_WATERMARK_SELECTION
//    if (kDAC_BufferWatermarkFlag == (kDAC_BufferWatermarkFlag & flags))
//    {
//        g_DacBufferWatermarkInterruptFlag = true;
//    }
//#endif /* FSL_FEATURE_DAC_HAS_WATERMARK_SELECTION */
//    if (kDAC_BufferReadPointerTopPositionFlag == (kDAC_BufferReadPointerTopPositionFlag & flags))
//    {
//        g_DacBufferReadPointerTopPositionInterruptFlag = true;
//    }
//    if (kDAC_BufferReadPointerBottomPositionFlag == (kDAC_BufferReadPointerBottomPositionFlag & flags))
//    {
//        g_DacBufferReadPointerBottomPositionInterruptFlag = true;
//    }
    /* Clear flags. */
    DAC_ClearBufferStatusFlags(DEMO_DAC_BASE, flags);
}

void Set_Dma_Mux( void )
{

  SIM->SCGC6 |= SIM_SCGC6_DMAMUX_MASK ;    
  DMAMUX->CHCFG[0] = 0;                       // disable the channel to configure it 
  
  DMAMUX->CHCFG[0] = DMAMUX_CHCFG_SOURCE(35) ;      // FTM2_CH1  trigger - CH1 is used only to generate the DMA request
   
  DMAMUX->CHCFG[0] |= DMAMUX_CHCFG_ENBL_MASK ;      // enable the mux
    
}

void Init_FTM2(void)
{   
  SIM->SCGC3 |= SIM_SCGC3_FTM2_MASK;        // enable clock to FTM2 
  //SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;       // enable clock to PORTB   
  
  FTM2->CONTROLS[1].CnSC |= FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK | FTM_CnSC_CHIE_MASK | FTM_CnSC_DMA_MASK ;  // PWM, true high pulses - generates DMA req    
  FTM2->MOD = (uint32_t)(CLOCK_GetFreq(kCLOCK_BusClk) /(1)); // 50MHz / 16000 = 3125. 
  FTM2->CONTROLS[0].CnV = 1100;                  // any initial random value  
  FTM2->CONTROLS[1].CnV = 50;    // this value does NOT changhe -it is used to generate audio reload DMA request  
  // this starts the timer
  FTM2->SC |= FTM_SC_CLKS(0x01) | FTM_SC_PS(0x00) ;       // Bus Clock, divide-by-1       
}

void RunDACsine() {
    uint32_t mask;
     // Construct sine wave:
     // - one complete cycle of 12-bit DAC values in RAM
     // - arbitrary (tunable) number of samples
     #define SINEWAVE_SAMPLE_CNT 128
    static uint16_t sineData[SINEWAVE_SAMPLE_CNT];
    for(int i=0; i<SINEWAVE_SAMPLE_CNT; i++) {
         double rads = ((2.0 * 3.14/*M_PI*/) * i)/SINEWAVE_SAMPLE_CNT;
         // Convert to signal with midpoint at 0x7FF and range 0 to 0xFFE
         // This will never output 0xFFF (using the full range would require an odd number of possible values).
         // Implement rounding and prevent overflows (which shouldn't ever happen)
         double vd = sin(rads) * 0x07FF;
         int vi;
         if(vd > 0) {
              vd += .5;
              vi = (int16_t)vd;
              if(vi > 0x07FF) vi = 0x7FF;
         } else if(vd<0) {
              vd -= .5;
              vi = (int16_t)vd;
              if(vi < -0x07FF) vi = -0x07FF;
         } else vi = 0;
         sineData[i] = (uint16_t)(vi + 0x07FF);
    }
     // Here's what we're trying to send to DAC:
    #if 0
        static const uint16_t testDACdata[16] = {0x0ee0, 0x110, 0x0ee0, 0x110, 0x0ee0, 0x110, 0x0ee0, 0x110, 0x0ee0, 0x990, 0xAA0, 0xBB0, 0xCC0, 0xDD0, 0xEE0, 0xFF0, };
        #define SOURCE_ADDRESS &testDACdata[0]
        #define SOURCE_CNT 16
        #define TARGET_FREQUENCY_HZ 1
    #endif
    #if 0
        #define SOURCE_ADDRESS &sineData[0]
        #define SOURCE_CNT SINEWAVE_SAMPLE_CNT
        #define TARGET_FREQUENCY_HZ 1000
    #endif
    #if 1
   
//        signed short *p =(signed short *)wav_record_file;
//        for (uint32_t i=0; i<8191/*(sizeof(wav_record_file)/2)*/; i++)
//        {
//            Audio_Source_Blk_A[i] = ((p[i])+32767)>>6;
//        }
//        for (uint32_t i=8191; i<(8191*2)/*(sizeof(wav_record_file)/2)*/; i++)
//        {
//            Audio_Source_Blk_B[i-8191] = ((p[i])+32767)>>6;
//        }
    
        #define SOURCE_ADDRESS &Audio_Source_Blk_A[0]
        #define SOURCE_CNT 1024
        #define TARGET_FREQUENCY_HZ 8000
    #endif
    #if 0
   
        for (uint32_t i=0; i<32767/*(sizeof(wav_record_file)/2)*/; i++)
        {
            Audio_Source_Blk_A[i] = (wav_record_file[i]<<3);
        }
        for (uint32_t i=32767; i<(32767*2)/*(sizeof(wav_record_file)/2)*/; i++)
        {
            Audio_Source_Blk_B[i-32767] = (wav_record_file[i]<<3);
        }
    
        #define SOURCE_ADDRESS &Audio_Source_Blk_A[0]
        #define SOURCE_CNT 32767
        #define TARGET_FREQUENCY_HZ 8000
    #endif        
        

    dac_config_t dacConfigStruct;
    DAC_GetDefaultConfig(&dacConfigStruct);
    dacConfigStruct.referenceVoltageSource = kDAC_ReferenceVoltageSourceVref2;
    dacConfigStruct.enableLowPowerMode = false;
    DAC_Init(DAC0, &dacConfigStruct);
    dac_buffer_config_t dacBufferConfigStruct;
    DAC_GetDefaultBufferConfig(&dacBufferConfigStruct);
    dacBufferConfigStruct.triggerMode = kDAC_BufferTriggerBySoftwareMode;
    DAC_SetBufferConfig(DAC0, &dacBufferConfigStruct);
    DAC_Enable(DEMO_DAC_BASE, true); //add 
//    DAC_SetBufferValue(DAC0, 0U, 0x7ff); // Succeeds: quick test to output 3.3v/2
//    /* Enable interrupts. */
//    mask = 0U;
//#if defined(FSL_FEATURE_DAC_HAS_WATERMARK_SELECTION) && FSL_FEATURE_DAC_HAS_WATERMARK_SELECTION
//    mask |= kDAC_BufferWatermarkInterruptEnable;
//#endif /* FSL_FEATURE_DAC_HAS_WATERMARK_SELECTION */
//    mask |= kDAC_BufferReadPointerTopInterruptEnable | kDAC_BufferReadPointerBottomInterruptEnable;
//    DAC_EnableBuffer(DEMO_DAC_BASE, true);
//    DAC_EnableBufferInterrupts(DEMO_DAC_BASE, mask);

    
    // Use DMAMUX0 internal channel number 0 to connect PIT event to DMA channel 0 request
    DMAMUX_Init(DMAMUX0);
    DMAMUX_Type *pDMAMUX0 = DMAMUX0; // expose for debugger, since "Peripherals" viewer doesn't work in Kinetis Studio
    DMAMUX_DisableChannel(DMAMUX0, 0/* DMAMUX channel number */); // Disable channel prior configuring it
    // DMAMUX source is unused when using periodic trigger; seems setting source unnecessary?
    // But, default source 0 is "Disable" - set to "always on" source...
    DMAMUX_SetSource          (DMAMUX0, 0/* DMAMUX channel number */, kDmaRequestMux0AlwaysOn63/*(uint8_t)kDmaRequestMux0AlwaysOn63 48 PDB*/);
    DMAMUX_EnablePeriodTrigger(DMAMUX0, 0/* DMAMUX channel number */);
    DMAMUX_EnableChannel      (DMAMUX0, 0/* DMAMUX channel number */);

    // Set up DMA channel 0 to read from data buffer and write to DAC
    DMA_Type* pDMA0 = DMA0; // expose for debugger, since "Peripherals" viewer doesn't work in Kinetis Studio
    edma_config_t edmaConfig;
    EDMA_GetDefaultConfig(&edmaConfig);
    EDMA_Init(DMA0,&edmaConfig);

    // Crappy FSL driver functions don't set SLAST etc; set up TCD directly, aaarrrggggg....
    DMA0->CR = 0; // default mode of operation for DMA (no minor loop mapping, etc)
    DMA0->TCD[0].SADDR = (uint32_t)(SOURCE_ADDRESS); // source address
    DMA0->TCD[0].DADDR = (uint32_t)(&DAC0->DAT);  // destination address
    // Source data and destination data transfer size
    DMA0->TCD[0].ATTR = DMA_ATTR_SSIZE(kEDMA_TransferSize2Bytes) | DMA_ATTR_DSIZE(kEDMA_TransferSize2Bytes)|DMA_ATTR_SMOD(0)|DMA_ATTR_DMOD(0);
    assert(DMA0->TCD[0].ATTR==0x0101);
    DMA0->TCD[0].SOFF = 2; // increment source address by 2 after each 2-byte transfer
    DMA0->TCD[0].DOFF = 0; // destination address is fixed DAC; do not increment
    DMA0->TCD[0].NBYTES_MLNO = 2; // 2 bytes to DAC per transfer
    DMA0->TCD[0].SLAST = -SOURCE_CNT*2; // decrement SADDR back to source start address after completing a major loop
    //DMA0->TCD[0].SLAST = 0; // decrement SADDR back to source start address after completing a major loop
    DMA0->TCD[0].DLAST_SGA = 0; // destination address (DAC) is not adjusted after completing a major loop
    DMA0->TCD[0].BITER_ELINKNO = SOURCE_CNT; // transfers per major loop
    DMA0->TCD[0].CITER_ELINKNO = SOURCE_CNT;
    //add by ywt 
    DMA0->TCD[0].CSR = DMA_CSR_INTMAJOR_MASK;    
    //DMA0->SERQ = DMA_SERQ_SERQ(0/*DMA channel#*/); // last, enable hardware requests for channel 0 (enable DMA channel 0)
    DMA0->ERQ = DMA_ERQ_ERQ0_MASK;
    
    NVIC_EnableIRQ(DMA0_IRQn);

    uint8_t dmaErr = DMA0->ES;
    assert(dmaErr==0); // no errors reported by DMA module
    uint8_t dmaErq = DMA0->ERQ;
    assert((dmaErq&1)==1); // DMA channel 0 is enabled

    // Set up PIT timer to generate trigger at interval that yields desired frequency for given number of samples
    {
         pit_config_t pit_setup;
         PIT_GetDefaultConfig(&pit_setup);
         pit_setup.enableRunInDebug = 1;
         PIT_Init(PIT,&pit_setup);
    }
    // WARNING: Contrary to bogus Freescale documentation, PIT channel 0 (not channel 1)
    // is hardwired to DMA channel 0 trigger in DMAMUX
     #define PIT_CHANNEL kPIT_Chnl_0
    PIT_SetTimerPeriod(PIT, PIT_CHANNEL, (CLOCK_GetFreq(kCLOCK_BusClk) / (TARGET_FREQUENCY_HZ)) );//
//    PIT_EnableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);    
//    EnableIRQ(PIT0_IRQn);
//    SPKR_GPIO_INIT();
    PIT_StartTimer(PIT, PIT_CHANNEL); // start the timer...

//    Init_FTM2();    // timer trigger / pwm starts running - audio playback DMA
}


/*!
 * @brief Main function
 */
bool Playing_Buff_A = true;
uint32_t dacValue=0;
int main(void)
{
    uint32_t i=0, j;
    dac_config_t dacConfigStruct;
    uint32_t audio_read_num	  = 0;
    //uint32_t dacValue=0;

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    SPKR_GPIO_INIT();
    
    AUDIO_MSG_INIT();
    AUDIO_MSG_DAC();
    
    PRINTF("\r\nDAC basic Example.\r\n");
    
    /* K64 example */
    RunDACsine();
    signed short *p =(signed short *)wav_record_file;
    while(1)
    {
        if (irq_flag){
            irq_flag = false;
        }else{
            continue;
        }
        

        if (Playing_Buff_A == false){
            for (uint32_t i=0; i<1024/*(sizeof(wav_record_file)/2)*/; i++)
            {
                //Audio_Source_Blk_A[i] = ((p[i+1024*audio_read_num])+32767)>>8;
                Audio_Source_Blk_A[i] = (wav_record_file[i+1024*audio_read_num]<<3);
            }
        }
        if (Playing_Buff_A == true){
            for (uint32_t i=0; i<1024/*(sizeof(wav_record_file)/2)*/; i++)
            {
                //Audio_Source_Blk_B[i] = ((p[i+1024*audio_read_num])+32767)>>8;
                Audio_Source_Blk_B[i] = (wav_record_file[i+1024*audio_read_num]<<3);
            }
        }
        
        audio_read_num++;
    }
    
    
    
    /* Configure the DAC. */
    /*
     * dacConfigStruct.referenceVoltageSource = kDAC_ReferenceVoltageSourceVref2;
     * dacConfigStruct.enableLowPowerMode = false;
     */
    DAC_GetDefaultConfig(&dacConfigStruct);
    DAC_Init(DEMO_DAC_BASE, &dacConfigStruct);
    DAC_Enable(DEMO_DAC_BASE, true);
    /* Configure the DAC buffer. add by ywt */
//    DAC_GetDefaultBufferConfig(&dacBufferConfigStruct);
//    dacBufferConfigStruct.triggerMode = kDAC_BufferTriggerByHardwareMode;
//    dacBufferConfigStruct.
//    DAC_SetBufferConfig(DEMO_DAC_BASE, &dacBufferConfigStruct);
//    /* Initialize EDMA */
//    EDMA_Configuration();
//    /* Initialize the HW trigger source */
//    LPTMR_Configuration();
    
    DAC_SetBufferReadPointer(DEMO_DAC_BASE, 0U); /* Make sure the read pointer to the start. */
                                                     /*
                                                     * The buffer is not enabled, so the read pointer can not move automatically. However, the buffer's read pointer
                                                     * and itemss can be written manually by user.
                                                     */
    
#if 1
    while (1)
    {
        //PRINTF("\r\nPlease input a value (0 - 4095) to output with DAC: ");
        //SCANF("%d", &dacValue);
        //dacValue = wav_record_file[i++];
        
        //PRINTF("\r\nInput value is %d\r\n", dacValue);
        i++;

        //dacValue = ((p[i])+32767)>>4;
        dacValue = (wav_record_file[i]<<4);
        
        if (dacValue > 0xFFFU)
        {
            PRINTF("Your value is output of range.\r\n");
            continue;
        }
        DAC_SetBufferValue(DEMO_DAC_BASE, 0U, dacValue);
                
        if (i>sizeof(wav_record_file)) break;
        
        {
            j = 2500;
            while(j--);
        }
    }
#endif
    
    
#if 0
    signed short *p =(signed short *)wav_record_file;
    while (1)
    {
        //PRINTF("\r\nPlease input a value (0 - 4095) to output with DAC: ");
        //SCANF("%d", &dacValue);
        //dacValue = wav_record_file[i++];
        
        //PRINTF("\r\nInput value is %d\r\n", dacValue);
        i++;

        dacValue = ((p[i])+32767)>>4;
        
        if (dacValue > 0xFFFU)
        {
            PRINTF("Your value is output of range.\r\n");
            continue;
        }
        DAC_SetBufferValue(DEMO_DAC_BASE, 0U, dacValue);
                
        if (i>sizeof(wav_record_file)/2) break;

     //   if (i%2 == 0)
        {
            j = 1500;
            while(j--);
        }

        //PRINTF("DAC out: %d\r\n", dacValue);
        /*
        * The value in the first item would be converted. User can measure the output voltage from DAC_OUTx pin.
        */
    }
#endif
}


void DMA0_DriverIRQHandler(void)
{
#ifdef AUDIO_PLAY

//	BaseType_t xHigherPriorityTaskWoken, xResult;
//	xHigherPriorityTaskWoken = pdFALSE;
//	xResult = xEventGroupSetBitsFromISR(AudioEventGroup, B_EVENT_AUDIO, &xHigherPriorityTaskWoken );
//	if( xResult != pdFAIL )
//	{
//		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
//	}
	irq_flag = true;

    DMA0->CINT = DMA_CINT_CINT(0);
    //NVIC_DisableIRQ(DMA0_IRQn);
    //DMA0->SERQ = DMA_SERQ_SERQ(0/*DMA channel#*/); // last, enable hardware requests for channel 0 (enable DMA channel 0)
    DMA0->ERQ &= ~DMA_ERQ_ERQ0_MASK;

    if (Playing_Buff_A) {                        // finished playing buffer A
        Playing_Buff_A = false;
        DMA0->TCD[0].SADDR = (uint32_t) Audio_Source_Blk_B ;
        /* event B */
    }
    else{
        Playing_Buff_A = true;
        DMA0->TCD[0].SADDR = (uint32_t) Audio_Source_Blk_A ;
        /* event A */
    }

    DMA0->ERQ = DMA_ERQ_ERQ0_MASK;
#else
    EDMA_HandleIRQ(s_EDMAHandle[0]);
#endif
}