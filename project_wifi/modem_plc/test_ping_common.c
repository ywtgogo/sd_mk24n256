#include "test_ping.h"
#include "string.h"

#define PLC_MAX_FRM_LEN     (255)

typedef struct {
    TU8     bIsRxBufBusy:1;   
    TU8     nPad:7;       

    TU8     tRxBuf[PLC_MAX_FRM_LEN];
    TU8     tTxBuf[PLC_MAX_FRM_LEN-1];            
} TPingCommon;

TPingCommon g_tPingCmTest = {0};

TVoid ST7580_DataIndication(TU8 * pBuf, TU8 nLen, TU16 nDelayInMs)//nDelayInMs is the delay between real Rx over time and time to call ST7580_PLCDataIndication
{   
    // notify up layer, actual data len is nLen(<= 251) for ST7580    
    TEST_PING_FrmRcvd(pBuf, nLen);     
}

TU8 *ST7580_BufferRequest(TU8 length)
{    
    if (g_tPingCmTest.bIsRxBufBusy || (TU16)length > PLC_MAX_FRM_LEN) 
    {
        return NULL; //ST7580 driver needs 255 bytes buffer for the maximum frame (251 bytes app data)
    }
    g_tPingCmTest.bIsRxBufBusy = 1;
    
    return &(g_tPingCmTest.tRxBuf[0]);
}

TU8 * PING_GetTxBuffer(void)
{
    return &(g_tPingCmTest.tTxBuf[0]);
}

void  PING_ReleaseRxBuffer(void)
{
    g_tPingCmTest.bIsRxBufBusy = 0;
}

void PING_GetDefaultPlcCfg(PLC_Config_t * ptCfg)
{
    memset(ptCfg, 0, sizeof(PLC_Config_t));
    
    ptCfg->fHigh = DFT_FREQ_H;
    ptCfg->fLow = DFT_FREQ_L;
    
    ptCfg->RX_Settings.as_fields.SnifferMode = TTrue;
    ptCfg->RX_Settings.as_fields.PSKModulationPreambleLength = BIT_32;  
    ptCfg->RX_Settings.as_fields.RxLowChannelModulation = ANY_PSK_MODULATION;
    ptCfg->RX_Settings.as_fields.RxHighChannelModulation = ANY_PSK_MODULATION;
    ptCfg->RX_Settings.as_fields.RxChannelSelector = DUAL_CHANNEL;

    ptCfg->TX_Settings.Modulation = MODE_8PSK;//MODE_BPSK;
    ptCfg->TX_Settings.TransmitterGain = 21;            //max:21,faeset:10
    ptCfg->TX_Settings.ChannelSelector = HIGH_CHANNEL;
    ptCfg->TX_Settings.ZeroCrossingSynchronous = TFalse;
    ptCfg->TX_Settings.TransmitDelay = 0;
    
    ptCfg->FSK_Settings.as_fields.BaudRate = 1;   //3-9600,1-2400
    ptCfg->FSK_Settings.as_fields.Deviation = 1;
    ptCfg->FSK_Settings.as_fields.PreambleLength = 2;
    ptCfg->FSK_Settings.as_fields.HeaderLength = 1;
    
    ptCfg->FskUW = 0xA859;
}
