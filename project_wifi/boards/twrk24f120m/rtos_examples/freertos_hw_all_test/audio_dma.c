/*****************************************************************************
 * (c) Copyright 2010, Freescale Semiconductor Inc.
 * ALL RIGHTS RESERVED.
 ***************************************************************************//*!
 *
 * @file      audio_dma.c
 *
 * @author    b01800
 *
 * @version   1.0.2.0
 *
 * @date      Mar-23-2012
 *
 * @brief     
 ******************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include <event.h>
#include "audio_dma.h"
#include "AudioCard.h"

// 16 bit audio samples - not volatile because only read by interrupts/hw
int16_t Audio_Source_Blk_A[AUDIO_SIZE], Audio_Source_Blk_B[AUDIO_SIZE];

// to be used during mute 
int16_t Audio_Silence[AUDIO_SIZE];                                        
    
volatile int Playing_Buff_A; 

/***************************************************************************//*!
* @function ClearAudioBuffers
*
* @brief    Clear Audio buffers used by DMA channel for transfering to output	
****************************************************************************/
void ClearAudioBuffers(void)
{
  int i;
  int16_t *p1,*p2,*p3;

  p1 = Audio_Source_Blk_A;
  p2 = Audio_Source_Blk_B;
  p3 = Audio_Silence;

  for (i=0 ; i < AUDIO_SIZE; i++) 
    {
    *p1++ = 0;   // mute initially
    *p2++ = 0;   // mute initially
    *p3++ = 0;   // mute forever
    }  
 }
/****************************************************************************/

/***************************************************************************//*!
* @function Init_Audio
*
* @brief    Setup Timer, DMA channels and DMA complete IRQ for audio task  	        
****************************************************************************/
void Init_Audio(void)  
{

  ClearAudioBuffers();
 
  Set_Audio_Playback_Dma();
    
  Playing_Buff_A = TRUE;
  
#if (AUDIO_DMA_OUTPUT == USE_PWM)
  
  Init_FTM2(); // pwm starts running - audio playback DMA must be already setup

#elif (AUDIO_DMA_OUTPUT == USE_I2S)
  
  AudioCard_Init();
  
#elif (AUDIO_DMA_OUTPUT == USE_DAC)
  Set_VREF();     // VREF for DAC
  Init_DAC();     // DACs ON
  Init_FTM2();    // timer trigger / pwm starts running - audio playback DMA
#endif
  
}
/****************************************************************************/

/***************************************************************************//*!
* @function Start_Audio
*
* @brief    Clears the buffers and enable DMA Audio channel feed
****************************************************************************/

void Start_Audio( void) 
{
 ClearAudioBuffers();
 DMA_ERQ |= DMA_ERQ_ERQ0_MASK;  
}
/****************************************************************************/

/***************************************************************************//*!
* @function Stop_Audio
*
* @brief    Clears the buffers and disable DMA Audio channel feed 	        
****************************************************************************/
void Stop_Audio( void) 
{
  
  DMA_TCD0_SADDR = (uint32_t) Audio_Silence; // silence feed for DMA0
  ClearAudioBuffers();
  DMA_ERQ &= ~DMA_ERQ_ERQ0_MASK;
}
/****************************************************************************/


/***************************************************************************//*!
* @function Set_Audio_Playback_Dma
*
* @brief    SetUp Audio DMA Playback, enable DMA major loop complete IRQ
*
* @note   		        
****************************************************************************/
void Set_Audio_Playback_Dma(void)  
{
// Install DMA0 Major loop complete IRQ  
  _int_install_isr(INT_DMA0, dma_ch0_isr, NULL);
  _cortex_int_init(INT_DMA0, 3, TRUE);
  
  
  // setup the DMA channels ( CH0 = playback and CH1 = buffer transfer )
  DMA_CR = 0
  | DMA_CR_EDBG_MASK // Stall DMA transfers when debugger is halted (avoid noise)
  ;       // no minor loop mapping
 
  DMA_DCHPRI15 = 0;   // just not to have same priorities
  DMA_DCHPRI0 = 15;  // cannot be pre-empeted, can pre-empt, highest priority
  
  // fill the TCD area 
  DMA_TCD0_SADDR          = (uint32_t) Audio_Source_Blk_A ; // alternated with Audio_Source_Blk_B
  DMA_TCD0_SOFF           = 2;                              // 2 byte offset 
  DMA_TCD0_ATTR           = DMA_ATTR_SMOD(0) | DMA_ATTR_SSIZE(1) | DMA_ATTR_DMOD(0) | DMA_ATTR_DSIZE(1);   // no circular addressing S&D, 16 bit S&D 
  DMA_TCD0_NBYTES_MLNO    = 2;                              // one  16bit sample every minor loop 
  DMA_TCD0_SLAST          = 0;//-(AUDIO_SIZE*2);         // source address will always be newly written before each new start  DMA_TCD0_DADDR  
#if (AUDIO_DMA_OUTPUT == USE_PWM)
  DMA_TCD0_DADDR          = (uint32_t) &FTM2_C0V;    // the FTM Channel 0 duty value  
#elif (AUDIO_DMA_OUTPUT == USE_I2S)
  DMA_TCD0_DADDR          = (uint32_t) &I2S0_TX0;    // the FTM Channel 0 duty value
#elif (AUDIO_DMA_OUTPUT == USE_DAC)
  DMA_TCD0_DADDR          = (uint32_t) &DAC0_DAT0L;    // the FTM Channel 0 duty value 
  DMA_TCD0_DOFF           = 0;
  DMA_TCD0_CITER_ELINKNO  = AUDIO_SIZE;              // total samples ( 128 )
  DMA_TCD0_DLASTSGA       = 0;                       // no final last adjustment ( does not move )
  DMA_TCD0_CSR            = DMA_CSR_INTMAJOR_MASK ;  // interrupt when done  
  DMA_TCD0_BITER_ELINKNO  = AUDIO_SIZE;              // no chan links, total samples ( 128 )
#endif //DMA_OUTPUT
    
  // configure DMA_MUX to trigger DMA channel 0  with FTM2 CH1 
  Set_Dma_Mux();
  
  // now enable chan0 for HW triggers
  DMA_ERQ = DMA_ERQ_ERQ0_MASK ; 
  
}
/****************************************************************************/

/***************************************************************************//*!
* @function Set_Dma_Mux
*
* @brief    PWM0 always on periodical mode channel consumes the audio data  
*           sends to the otput device (PWM, DAC, Audio Codec...)
*   
* @note      		        
****************************************************************************/
void Set_Dma_Mux( void )
{
  
  
  SIM_SCGC6 |= SIM_SCGC6_DMAMUX_MASK ;    
  DMAMUX_CHCFG0 = 0;                       // disable the channel to configure it 
  
#if (AUDIO_DMA_OUTPUT == USE_PWM)
  
  DMAMUX_CHCFG0 = DMAMUX_CHCFG_SOURCE(35) ;      // FTM2_CH1  trigger - CH1 is used only to generate the DMA request
  
#elif (AUDIO_DMA_OUTPUT == USE_I2S)
  
  DMAMUX_CHCFG0 = DMAMUX_CHCFG_SOURCE(15) ;      //I2S0 Transmit
  
#elif (AUDIO_DMA_OUTPUT == USE_DAC)
  
  DMAMUX_CHCFG0 = DMAMUX_CHCFG_SOURCE(35) ;      // FTM2_CH1  trigger - CH1 is used only to generate the DMA request
  
#endif //DMA_OUTPUT
  
  DMAMUX_CHCFG0 |= DMAMUX_CHCFG_ENBL_MASK ;      // enable the mux
    
}  
/****************************************************************************/  
    
/***************************************************************************//*!
* @function  Init_FTM2 
*
* @brief     This function configures the flextimer to generate run a pwm
*            for the audio system We will aim for 8000Hz
* @note      		        
****************************************************************************/

void Init_FTM2(void)      
{   
  SIM_SCGC3 |= SIM_SCGC3_FTM2_MASK;       // enable clock to FTM2 
  SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;      // enable clock to PORTB
  
#if (AUDIO_DMA_OUTPUT == USE_PWM)
  PORTB_PCR18  = PORT_PCR_MUX(3);         // FTM2_CH1 output PTB18   TWR-Elevator - B66 
  //PORTB_PCR17  = PORT_PCR_MUX(3);         // FTM2_CH1 output PTB17   TWR-Elevator - B67 
  FTM2_C0SC |= FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK ;  // PWM, true high pulses 
#endif
  
  FTM2_C1SC |= FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK | FTM_CnSC_CHIE_MASK | FTM_CnSC_DMA_MASK ;  // PWM, true high pulses - generates DMA req    
  FTM2_MOD = (uint_32)((uint_32)BSP_BUS_CLOCK /(SAMPLERATE)); // 50MHz / 16000 = 3125. 
  FTM2_C0V = 1100;                  // any initial random value  
  FTM2_C1V = 50;    // this value does NOT changhe -it is used to generate audio reload DMA request  
  // this starts the timer
  FTM2_SC |= FTM_SC_CLKS(0x01) | FTM_SC_PS(0x00) ;       // Bus Clock, divide-by-1       
}
/****************************************************************************/


/***************************************************************************//*!
* @function  Set_VREF 
*
* @brief     Sets the VREF module, light regulation without VREF_OUT signal
*        
* @note      need to have also Vref enabled	        
****************************************************************************/
void Set_VREF(void) 
{
  
SIM_SCGC4 |= SIM_SCGC4_VREF_MASK ;     // enable VRERF clock   

// VREF_TRM = 32;    // mid trim value  - trim register removed  

VREF_SC = VREF_SC_VREFEN_MASK | VREF_SC_REGEN_MASK ;              // enable the BANDGAP, and regulator, but remain in bandgap-only mode
while ((VREF_SC & VREF_SC_VREFST_MASK) != VREF_SC_VREFST_MASK );  // wait for bandgap to be ready
VREF_SC |= VREF_SC_MODE_LV(1);                                    // now switch to lowpower buffered mode 
                                                                  
}


/***************************************************************************//*!
* @function  Init_DAC 
*
* @brief     This function configures the DAC0, use VREF as ref
*        
* @note      need to have also Vref enabled	        
****************************************************************************/
void Init_DAC(void) 
{
 
SIM_SCGC2 |= SIM_SCGC2_DAC0_MASK | SIM_SCGC2_DAC1_MASK ;     // enable DAC clocks   

// under reset conditions the DACs will use Vref as reference , high power mode, no FIFO (work in normal mode)
// to be tested : low power mode ( lower slew rate so lower high frequency content on output ) 

DAC0_C0 |=  DAC_C0_DACEN_MASK;
DAC0_C0 |=  DAC_C0_DACRFS_MASK;

//DAC1_C0 |=  DAC_C0_DACEN_MASK ;           // DACs now enabled,

}

/***************************************************************************//*!
* @function dma_ch0_isr
*
* @brief    Swap the playback buffers and signal the app one of them is free 
*
* @param    none
*
* @return   none	
*
* @note   		        
****************************************************************************/
// MQX Event
extern pointer  event_dma_rdy;
void dma_ch0_isr(pointer foo)
{  
  DMA_CINT = DMA_CINT_CINT(0);                 // use the Clear Intr. Request register 
 
  if (Playing_Buff_A) {                        // finished playing buffer A
    Playing_Buff_A = FALSE;
    DMA_TCD0_SADDR          = (uint32_t) Audio_Source_Blk_B ;
    }
  else{
    Playing_Buff_A = TRUE;
    DMA_TCD0_SADDR          = (uint32_t) Audio_Source_Blk_A ; 
    } 
   
  // DMA finished playback an ready for a new buffer
  _event_set(event_dma_rdy,0x01);

}
/****************************************************************************/

