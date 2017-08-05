#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "event_groups.h"

#include "board.h"
#include "audio_play.h"
#include "fsl_dac.h"
#include "fsl_pit.h"
#include "fsl_dmamux.h"
#include "fsl_edma.h"
#include "spiflash_config.h"

#include "wav_record.h"

#define AUDIO_SIZE   	1024//6144//6144//12288//24576// one channel audio , Buffers = 1024 16-bit words
#define SAMPLERATE 		8000L // 8kHz sampling rate
#define SOURCE_ADDRESS 	&Audio_Source_Blk_A[0]
#define SOURCE_CNT 		AUDIO_SIZE
#define TARGET_FREQUENCY_HZ SAMPLERATE

uint16_t Audio_Source_Blk_A[AUDIO_SIZE];
uint16_t Audio_Source_Blk_B[AUDIO_SIZE];

QueueHandle_t audio_msg_queue = NULL;
EventGroupHandle_t AudioEventGroup = NULL;

#define B_EVENT_AUDIO 	(1 << 3)

//signed short *p =(signed short *)wav_record_file;

void Init_Audio(void)
{
    SPKR_GPIO_INIT();		//PTA5
    AUDIO_MSG_INIT();		//PTA2

    dac_config_t dacConfigStruct;
    DAC_GetDefaultConfig(&dacConfigStruct);
    dacConfigStruct.referenceVoltageSource = kDAC_ReferenceVoltageSourceVref2;
    dacConfigStruct.enableLowPowerMode = false;
    DAC_Init(DAC0, &dacConfigStruct);
    dac_buffer_config_t dacBufferConfigStruct;
    DAC_GetDefaultBufferConfig(&dacBufferConfigStruct);
    dacBufferConfigStruct.triggerMode = kDAC_BufferTriggerBySoftwareMode;
    DAC_SetBufferConfig(DAC0, &dacBufferConfigStruct);
    DAC_Enable(DAC0, true); //add

    // Use DMAMUX0 internal channel number 0 to connect PIT event to DMA channel 0 request
    DMAMUX_Init(DMAMUX0);
    //DMAMUX_Type *pDMAMUX0 = DMAMUX0; // expose for debugger, since "Peripherals" viewer doesn't work in Kinetis Studio
    DMAMUX_DisableChannel(DMAMUX0, 0/* DMAMUX channel number */); // Disable channel prior configuring it
    // DMAMUX source is unused when using periodic trigger; seems setting source unnecessary?
    // But, default source 0 is "Disable" - set to "always on" source...
    DMAMUX_SetSource          (DMAMUX0, 0/* DMAMUX channel number */, kDmaRequestMux0AlwaysOn63/*(uint8_t)kDmaRequestMux0AlwaysOn63 48 PDB*/);
    DMAMUX_EnablePeriodTrigger(DMAMUX0, 0/* DMAMUX channel number */);
    DMAMUX_EnableChannel      (DMAMUX0, 0/* DMAMUX channel number */);

    // Set up DMA channel 0 to read from data buffer and write to DAC
    //DMA_Type* pDMA0 = DMA0; // expose for debugger, since "Peripherals" viewer doesn't work in Kinetis Studio
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
    //DMA0->TCD[0].SLAST = -SOURCE_CNT*2; // decrement SADDR back to source start address after completing a major loop
    DMA0->TCD[0].SLAST = 0; // decrement SADDR back to source start address after completing a major loop
    DMA0->TCD[0].DLAST_SGA = 0; // destination address (DAC) is not adjusted after completing a major loop
    DMA0->TCD[0].BITER_ELINKNO = SOURCE_CNT; // transfers per major loop
    DMA0->TCD[0].CITER_ELINKNO = SOURCE_CNT;
    //add by ywt
    DMA0->TCD[0].CSR = DMA_CSR_INTMAJOR_MASK;
    //DMA0->SERQ = DMA_SERQ_SERQ(0/*DMA channel#*/); // last, enable hardware requests for channel 0 (enable DMA channel 0)
    DMA0->ERQ = DMA_ERQ_ERQ0_MASK;

    NVIC_SetPriority(DMA0_IRQn, 6);
    NVIC_EnableIRQ(DMA0_IRQn);

    uint8_t dmaErr = DMA0->ES;
    assert(dmaErr==0); // no errors reported by DMA module
    uint8_t dmaErq = DMA0->ERQ;
    assert((dmaErq&1)==1); // DMA channel 0 is enabled

}

void Init_Pit(void)
{
    // Set up PIT timer to generate trigger at interval that yields desired frequency for given number of samples

	 pit_config_t pit_setup;
	 PIT_GetDefaultConfig(&pit_setup);
	 pit_setup.enableRunInDebug = 1;
	 PIT_Init(PIT,&pit_setup);

    // WARNING: Contrary to bogus Freescale documentation, PIT channel 0 (not channel 1)
    // is hardwired to DMA channel 0 trigger in DMAMUX
    PIT_SetTimerPeriod(PIT, PIT_CHANNEL, (CLOCK_GetFreq(kCLOCK_BusClk) / (TARGET_FREQUENCY_HZ)) );//
    PIT_StartTimer(PIT, PIT_CHANNEL); // start the timer...
}

void Init_DAC_Data() {

	memset(Audio_Source_Blk_A, 0x0, AUDIO_SIZE);
	memset(Audio_Source_Blk_B, 0x0, AUDIO_SIZE);
}

bool Playing_Buff_A = true;
void audio_feed_task(void * pvParameters)
{
	uint32_t audio_addr_start = AUDIO_DATA_ADDR_START;
	uint32_t audio_addr_end   = 0x780000;
	uint32_t audio_read_num	  = 0;
	uint8_t  Audio_Source_Temp[AUDIO_SIZE*2];
	char 	 audio_cmd;
	signed short *p =(signed short *)Audio_Source_Temp;

	audio_msg_queue = xQueueCreate(3, 1);

	AudioEventGroup = xEventGroupCreate();
	if (AudioEventGroup == NULL)
	{
		printf("AudioEventGroup ERR\r\n");
	}
	xEventGroupSetBits(AudioEventGroup, B_EVENT_AUDIO);


	vTaskDelay(1000);
	Init_Audio();
	Init_DAC_Data();

	//Init_Pit();
	while(1)
	{
		xQueueReceive(audio_msg_queue, &audio_cmd, portMAX_DELAY);
		audio_read_num	= 0;
		AUDIO_MSG_DAC();
		Init_Pit();

		//signed short *p =(signed short *)wav_record_file;
		/* read data from nor */
		while(1)
		{
			/* event wait */
			xEventGroupWaitBits(AudioEventGroup,    	/* The event group handle. */
								B_EVENT_AUDIO,			/* The bit pattern the event group is waiting for. */
								pdTRUE,         		/* BIT will be cleared automatically. */
								pdTRUE,         		/* Don't wait for both bits, either bit unblock task. */
								portMAX_DELAY); 		/* Block indefinitely to wait for the condition to be met. */

			//printf("%d ", audio_read_num);
			if (audio_read_num >= 30) {
				PIT_StopTimer(PIT, PIT_CHANNEL);
				break;
			}
#if 1
			norflash_read_data_ll (&flash_master_rtos_handle, audio_addr_start+AUDIO_SIZE*2*audio_read_num, AUDIO_SIZE*2, Audio_Source_Temp);

	//		for (uint32_t j=0; j<AUDIO_SIZE*2; j++) {
	//			printf("0x%02x ", Audio_Source_Temp[j] );
	//		}
			if (Playing_Buff_A == false){
				for (uint32_t i=0; i<AUDIO_SIZE; i++) {
					Audio_Source_Blk_A[i] = ((p[i])+32767)>>4;
				}
			}

			if (Playing_Buff_A == true){
				for (uint32_t i=0; i<AUDIO_SIZE; i++) {
					Audio_Source_Blk_B[i] = ((p[i])+32767)>>4;
				}
			}
#endif
#if 0
			norflash_read_data_ll (&flash_master_rtos_handle, audio_addr_start+AUDIO_SIZE*audio_read_num, AUDIO_SIZE, Audio_Source_Temp);
			if (Playing_Buff_A == false){
				for (uint32_t i=0; i<AUDIO_SIZE; i++) {
					Audio_Source_Blk_A[i] = (Audio_Source_Temp[i]<<3);
	//	        	Audio_Source_Blk_A[i] = ((p[i+AUDIO_SIZE*audio_read_num])+32767)>>5;
	//	        	Audio_Source_Blk_A[i] = (wav_record_file[i+AUDIO_SIZE*audio_read_num]<<3);
				}
			}

			if (Playing_Buff_A == true){
				for (uint32_t i=0; i<AUDIO_SIZE; i++) {
					Audio_Source_Blk_B[i] = (Audio_Source_Temp[i]<<3);
	//	        	Audio_Source_Blk_B[i] = ((p[i+AUDIO_SIZE*audio_read_num])+32767)>>5;
	//	        	Audio_Source_Blk_B[i] = (wav_record_file[i+AUDIO_SIZE*audio_read_num]<<3);
				}
			}
#endif
			audio_read_num++;
		}
	}
	vTaskSuspend(NULL);
}

extern uint32_t sos_incoming_call_status;
void start_play_plan(AUDIO_Index index)
{
#ifdef WIFI_MODULE
	return;
#endif
	if (sos_incoming_call_status)
	{
		return;
	};
	index = AUDIO_POST_ERROR;
	xQueueSend(audio_msg_queue, &index, 0);
}

void DMA0_DriverIRQHandler(void)
{
#ifdef AUDIO_PLAY

	BaseType_t xHigherPriorityTaskWoken, xResult;
	xHigherPriorityTaskWoken = pdFALSE;
	xResult = xEventGroupSetBitsFromISR(AudioEventGroup, B_EVENT_AUDIO, &xHigherPriorityTaskWoken );
	if( xResult != pdFAIL )
	{
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}

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
