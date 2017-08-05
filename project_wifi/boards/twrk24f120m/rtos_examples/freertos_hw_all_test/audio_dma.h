/*****************************************************************************
 * (c) Copyright 2010, Freescale Semiconductor Inc.
 * ALL RIGHTS RESERVED.
 ***************************************************************************//*!
 *
 * @file      audio_dma.h
 *
 * @author    N/A
 *
 * @version   1.0.1.0
 *
 * @date      Mar-22-2012
 *
 * @brief     header file for audio_dma.c, buffer size definitions
 *
 ******************************************************************************/

#ifndef _AUDIO_DMA_H
#define _AUDIO_DMA_H

#define AUDIO_SIZE   512  // one channel audio , Buffers = 1024 16-bit words
#define SAMPLERATE 16000L // 8kHz sampling rate

#define USE_PWM 1
#define USE_I2S 2
#define USE_DAC 3

#define AUDIO_DMA_OUTPUT USE_DAC

// flags
extern volatile int Playing_Buff_A;

// audio buffers with samples
extern int16_t Audio_Source_Blk_A[AUDIO_SIZE], Audio_Source_Blk_B[AUDIO_SIZE];

/***************************************************************************
* API
****************************************************************************/
void Init_Audio(void);
void Start_Audio(void);
void Stop_Audio(void);

void Init_FTM2(void);      
void Set_Audio_Playback_Dma(void);  
 
void Set_Dma_Mux(void);
void Init_Gpio(void);

void Set_VREF(void);
void Init_DAC(void);

void dma_ch0_isr(void* foo);


#endif //_AUDIO_DMA_H