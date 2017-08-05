#include "FreeRTOS.h"
#include "task.h"

#include "test_ping.h"
#include "drv_st7580.h"
#include "plc_codec_ping.h"
#include "fsl_debug_console.h"
//#include "host_uart.h"
//#include "test_led_control.h"
#include "string.h"

typedef struct {    
    TU8         nMyDILen;
    TU8         szMyDI[MTR_ADDR_MAXLEN];
    
    TU8         nRxBufBusy;
    TPlcFrmHdr   tPlcFrmHdr;  
    PLC_Config_t tPlcCfg;
    
    TestPing_SN_TypeDef *pSnPara;
    TU8  * pTxBuf; 
    
    Timer_t     tTmTest;
       
    TCommCntr   tCntr;
}TPrimeIF_Test;

static TPrimeIF_Test    g_tTest;

#ifdef WIN32
extern TestPing_SN_TypeDef g_tSnPara;
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////      
static void APP_TxEnd(TU8 nResult)
{
    if (nResult == ERR_SUCCESS) 
    {
        g_tTest.tCntr.nTxSuccCntr++;
        //TEST_AMR_PlcTx_LedOn();
    }
    else
        g_tTest.tCntr.nTxFailCntr++;
}
static void APP_SN_FSM(void)
{     
    TU16  i, j, i0, j0;   
    TU8   szAppend[8];
    
    if(g_tTest.nRxBufBusy == 1) //received frame not processed
    {
        if((!memcmp(g_tTest.szMyDI, g_tTest.tPlcFrmHdr.szDI, 6)))    
        {
            g_tTest.tCntr.nRxSuccCntr++;
            if(!TIMER_isStarted(&g_tTest.tTmTest))
                vTaskDelay(36000000);   //10Сʱ
//                TIMER_Start(&g_tTest.tTmTest);
            /*
            if (TEST_AMR_IsHostTrans())
            {
                HOST_SendData(HOST_CMD_DATA, g_tTest.tPlcFrmHdr.nDILen, g_tTest.tPlcFrmHdr.szDI, g_tTest.tPlcFrmHdr.szData, g_tTest.tPlcFrmHdr.nDataL);
            } */
//            if(TIMER_Elapsed(&g_tTest.tTmTest)) 
//            {               
//               return;
//            }           
            
            if(g_tTest.tPlcFrmHdr.nDataL > 0)
            {
                g_tTest.tPlcFrmHdr.nDataL--;
                switch (g_tTest.tPlcFrmHdr.szData[0])
                {
                case 0x00: //amr: except "68 DI 68", "RxMod | RxChl | PGA | SNR | MaxTemp | version(8 bytes) | 16" in g_tTest.tPlcFrmHdr.szData                  
                    if(g_tTest.tPlcFrmHdr.nDataL != 0) return;
                    memcpy(szAppend, "\x54\x6F\x6F\x6C\x53\x6E\x30\x00", 8); //ToolSn6  
                    g_tTest.tPlcFrmHdr.nDataL = 8; 
                    break;
                case 0x11: //change Tx gain:0~31
                    if(g_tTest.tPlcFrmHdr.nDataL != 1) return;
                    if(g_tTest.tPlcFrmHdr.szData[1] > 31)
                        g_tTest.tPlcFrmHdr.szData[1] = 31;
                                        
                    szAppend[0] = 0x11; //cmd
                    szAppend[1] = g_tTest.tPlcCfg.TX_Settings.TransmitterGain; //old gain
                    
                    g_tTest.tPlcCfg.TX_Settings.TransmitterGain = g_tTest.tPlcFrmHdr.szData[1];
                    if(ST7580_ChkConfigure(&g_tTest.tPlcCfg) != ERR_SUCCESS)
                    {
                        g_tTest.tPlcCfg.TX_Settings.TransmitterGain = szAppend[1];
                        return;     
                    }
                    szAppend[2] =  g_tTest.tPlcCfg.TX_Settings.TransmitterGain; //new gain
                    g_tTest.tPlcFrmHdr.nDataL = 3; 
                    break;
                case 0x12: //change Tx channel:0-lowChannel, 1-highChannel
                    if(g_tTest.tPlcFrmHdr.nDataL != 1) return;
                    if(g_tTest.tPlcFrmHdr.szData[1] > 1)
                        g_tTest.tPlcFrmHdr.szData[1] = 1;

                    szAppend[0] = 0x12; //cmd
                    szAppend[1] = g_tTest.tPlcCfg.TX_Settings.ChannelSelector; //old TxChl                    
                    
                    g_tTest.tPlcCfg.TX_Settings.ChannelSelector = g_tTest.tPlcFrmHdr.szData[1];
                    if(ST7580_ChkConfigure(&g_tTest.tPlcCfg) != ERR_SUCCESS)
                    {
                        g_tTest.tPlcCfg.TX_Settings.ChannelSelector = szAppend[1];
                        return;  
                    }
                    szAppend[2] =  g_tTest.tPlcCfg.TX_Settings.ChannelSelector; //new TxChl
                    g_tTest.tPlcFrmHdr.nDataL = 3;                                                                             
                    break;
                case 0x13: //change carrier frequencies: highFreq/32, lowFreq/32
                    if(g_tTest.tPlcFrmHdr.nDataL != 4) return;
                    i = ((TU16)(g_tTest.tPlcFrmHdr.szData[2])<<8) + g_tTest.tPlcFrmHdr.szData[1];
                    j = ((TU16)(g_tTest.tPlcFrmHdr.szData[4])<<8) + g_tTest.tPlcFrmHdr.szData[3];
                    
                    szAppend[0] = 0x13; //cmd
                    i0 = g_tTest.tPlcCfg.fHigh;//old high freq
                    j0 = g_tTest.tPlcCfg.fLow;//old low freq
                    szAppend[1] = (i0<<2)/125; //old high freq, unit:kHz
                    szAppend[2] = (j0<<2)/125; //old low freq, unit:kHz
                                        
                    if(i > j)
                    {
                        g_tTest.tPlcCfg.fHigh = i;
                        g_tTest.tPlcCfg.fLow = j;
                    }
                    else
                    {
                        g_tTest.tPlcCfg.fHigh = j;
                        g_tTest.tPlcCfg.fLow = i;
                    }                    
                    if(ST7580_ChkConfigure(&g_tTest.tPlcCfg) != ERR_SUCCESS)
                    {
                        g_tTest.tPlcCfg.fHigh = i0;
                        g_tTest.tPlcCfg.fLow = j0;
                        return;   
                    }
                    szAppend[3] = (g_tTest.tPlcCfg.fHigh<<2)/125; //new high freq, unit:kHz
                    szAppend[4] = (g_tTest.tPlcCfg.fLow<<2)/125; //new low freq, unit:kHz                    
                    g_tTest.tPlcFrmHdr.nDataL = 5;
                    break;
                case 0x14: //change Tx mode:0-bpsk, 1-qpsk, 2-8psk, 3-fsk, 4-bpsk_cod, 5-qpsk_cod, 7-pna
                    if(g_tTest.tPlcFrmHdr.nDataL != 1) return;
                    if(g_tTest.tPlcFrmHdr.szData[1] > 7)
                        g_tTest.tPlcFrmHdr.szData[1] = 0;

                    szAppend[0] = 0x14; //cmd
                    szAppend[1] = g_tTest.tPlcCfg.TX_Settings.Modulation; //old TxMod                    
                    
                    g_tTest.tPlcCfg.TX_Settings.Modulation = g_tTest.tPlcFrmHdr.szData[1];
                    if(ST7580_ChkConfigure(&g_tTest.tPlcCfg) != ERR_SUCCESS)
                    {
                        g_tTest.tPlcCfg.TX_Settings.Modulation = szAppend[1];
                        return;  
                    }
                    szAppend[2] =  g_tTest.tPlcCfg.TX_Settings.Modulation; //new TxMod                                        
                    g_tTest.tPlcFrmHdr.nDataL = 3;                  
                    break;
                case 0x15: //change Rx method (single or dual):0-highChannel, 1-dualChannel
                    if(g_tTest.tPlcFrmHdr.nDataL != 1) return;
                    if(g_tTest.tPlcFrmHdr.szData[1] > 1)
                        g_tTest.tPlcFrmHdr.szData[1] = 1;

                    szAppend[0] = 0x15; //cmd
                    szAppend[1] = g_tTest.tPlcCfg.RX_Settings.as_fields.RxChannelSelector; //old RxChl                    
                    
                    g_tTest.tPlcCfg.RX_Settings.as_fields.RxChannelSelector = g_tTest.tPlcFrmHdr.szData[1];
                    if(ST7580_ChkConfigure(&g_tTest.tPlcCfg) != ERR_SUCCESS)
                    {
                        g_tTest.tPlcCfg.RX_Settings.as_fields.RxChannelSelector = szAppend[1];
                        return;  
                    }
                    szAppend[2] =  g_tTest.tPlcCfg.RX_Settings.as_fields.RxChannelSelector;//new RxChl
                    g_tTest.tPlcFrmHdr.nDataL = 3;   
                    break;
                case 0x16: //sniffer on/off: 0-off, 1-on
                    if(g_tTest.tPlcFrmHdr.nDataL != 1) return;
                    if(g_tTest.tPlcFrmHdr.szData[1] > 1)
                        g_tTest.tPlcFrmHdr.szData[1] = 1;

                    szAppend[0] = 0x16; //cmd
                    szAppend[1] = g_tTest.tPlcCfg.RX_Settings.as_fields.SnifferMode; //old RxSniffer                    
                    
                    g_tTest.tPlcCfg.RX_Settings.as_fields.SnifferMode = g_tTest.tPlcFrmHdr.szData[1];
                    if(ST7580_ChkConfigure(&g_tTest.tPlcCfg) != ERR_SUCCESS)
                    {
                        g_tTest.tPlcCfg.RX_Settings.as_fields.SnifferMode = szAppend[1];
                        return;  
                    }
                    szAppend[2] =  g_tTest.tPlcCfg.RX_Settings.as_fields.SnifferMode;//new RxSniffer
                    g_tTest.tPlcFrmHdr.nDataL = 3;                 
                    break;
                case 0x17: //preamble bytes: 0-16bits, 1-24bits, 2-32bits, 3-40bits
                    if(g_tTest.tPlcFrmHdr.nDataL != 1) return;
                    if(g_tTest.tPlcFrmHdr.szData[1] > 3)
                        g_tTest.tPlcFrmHdr.szData[1] = 1;

                    szAppend[0] = 0x17; //cmd
                    szAppend[1] = g_tTest.tPlcCfg.RX_Settings.as_fields.PSKModulationPreambleLength; //old RxPreLength                    
                    
                    g_tTest.tPlcCfg.RX_Settings.as_fields.PSKModulationPreambleLength = g_tTest.tPlcFrmHdr.szData[1];
                    if(ST7580_ChkConfigure(&g_tTest.tPlcCfg) != ERR_SUCCESS)
                    {
                        g_tTest.tPlcCfg.RX_Settings.as_fields.PSKModulationPreambleLength = szAppend[1];
                        return;  
                    }
                    szAppend[2] =  g_tTest.tPlcCfg.RX_Settings.as_fields.PSKModulationPreambleLength;//new RxPreLength
                    g_tTest.tPlcFrmHdr.nDataL = 3;   
                    break;
                case 0x18: //Rx mode of both channels: RxHighMod(1-psk, 0-fsk), RxLowMod(1-psk, 0-fsk)
                    if(g_tTest.tPlcFrmHdr.nDataL != 2) return;
                    if(g_tTest.tPlcFrmHdr.szData[1] > 1)
                        g_tTest.tPlcFrmHdr.szData[1] = 1;
                    if(g_tTest.tPlcFrmHdr.szData[2] > 1)
                        g_tTest.tPlcFrmHdr.szData[2] = 1;                    
                    
                    szAppend[0] = 0x18; //cmd
                    szAppend[1] = g_tTest.tPlcCfg.RX_Settings.as_fields.RxHighChannelModulation; //old highChl RxMod                    
                    szAppend[2] = g_tTest.tPlcCfg.RX_Settings.as_fields.RxLowChannelModulation; //old lowChl RxMod
                    
                    g_tTest.tPlcCfg.RX_Settings.as_fields.RxHighChannelModulation = g_tTest.tPlcFrmHdr.szData[1];
                    g_tTest.tPlcCfg.RX_Settings.as_fields.RxLowChannelModulation = g_tTest.tPlcFrmHdr.szData[2];
                    if(ST7580_ChkConfigure(&g_tTest.tPlcCfg) != ERR_SUCCESS)
                    {
                        g_tTest.tPlcCfg.RX_Settings.as_fields.RxHighChannelModulation = szAppend[1];
                        g_tTest.tPlcCfg.RX_Settings.as_fields.RxLowChannelModulation = szAppend[2];
                        return;  
                    }
                    szAppend[3] =  g_tTest.tPlcCfg.RX_Settings.as_fields.RxHighChannelModulation;//new highChl RxMod
                    szAppend[4] =  g_tTest.tPlcCfg.RX_Settings.as_fields.RxLowChannelModulation;//new lowChl RxMod
                    g_tTest.tPlcFrmHdr.nDataL = 5;                                                            
                    break;
                default:
                    return;
                    break;
                }  
                g_tTest.tPlcFrmHdr.szData[0] = (PLC_Modulation_t)(ST7580_GetInformations()->RX_Information.Info.as_fields.Modulation);  
                g_tTest.tPlcFrmHdr.szData[1] = (PLC_Channel_t)(ST7580_GetInformations()->RX_Information.Info.as_fields.Channel);     
                g_tTest.tPlcFrmHdr.szData[2] = (ST7580_GetInformations()->RX_Information.Info.as_fields.PGA);
                g_tTest.tPlcFrmHdr.szData[3] = (ST7580_GetInformations()->RX_Information.SNR);
                g_tTest.tPlcFrmHdr.szData[4] = (ST7580_GetInformations()->TX_Information.MaxTemp);                
                memset(g_tTest.tPlcFrmHdr.szData+5, 0x00, 8);
                memcpy(g_tTest.tPlcFrmHdr.szData+5, szAppend, g_tTest.tPlcFrmHdr.nDataL);
                g_tTest.tPlcFrmHdr.nDataL = 14;
                g_tTest.tPlcFrmHdr.szData[g_tTest.tPlcFrmHdr.nDataL-1] = 0x16;                                                                               
            }
                                                                  
            g_tTest.pTxBuf = PING_GetTxBuffer(); 
            if (g_tTest.pTxBuf == NULL)  //the previous one is WAIT_TX, the current one isreceived. Occurs when MaxRandom is too big and App Tx interval is too small
            {              
              g_tTest.tCntr.nRxSuccCntr--;
              return; 
            }
            i = PLC_FrmEnc(g_tTest.pTxBuf, &g_tTest.tPlcFrmHdr); 
            if(i == 0) //encode error, release the tx buffer
            {               
                g_tTest.tCntr.nRxSuccCntr--;
                return; 
            }
            if(Drv_DataRequest(NULL, g_tTest.pTxBuf, i, APP_TxEnd) == ERR_SUCCESS)            
            {
               // LOG("Sn response OK\n");  
                g_tTest.tCntr.nTxCntr++;
            }
            else
            {
               // LOG("Sn response BUSY\n"); 
            }                                                
        }
        g_tTest.nRxBufBusy = 0;   
        

    }else{
        uint8_t temp_array[] = {0x01, 0x23};
        
        if(Drv_DataRequest(NULL, temp_array, sizeof(temp_array), APP_TxEnd) == ERR_SUCCESS)            
        {
           // LOG("Sn response OK\n");  
            g_tTest.tCntr.nTxCntr++;
        }
        vTaskDelay(100);
    }
}
    uint8_t status_plc_tx;
void plc_send_poling_task(void *pvParameters)
{
    uint8_t temp_array[] = {0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23,0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, };

    vTaskSuspend(NULL);
    
    while (1)
    {
        status_plc_tx = Drv_DataRequest(NULL, temp_array, sizeof(temp_array), APP_TxEnd);
        if(status_plc_tx == ERR_SUCCESS)            
        {
           // LOG("Sn response OK\n");  
            g_tTest.tCntr.nTxCntr++;
        }
        else{
            PRINTF("Sn NO response\n");  
  
        }//vTaskDelay(100);
    }
}

void TEST_PING_FrmRcvd(TU8 * pBuf, TU8 nLen)
{      
     //TEST_AMR_PlcRx_LedOn();
     g_tTest.tCntr.nRxCntr++;
     // Check if previous frame handled
    if (g_tTest.nRxBufBusy == 1)
    {
        //LOG("APP_FrmRcvd: pRxBuf!=NULL\n"); //lost in app layer because slower process in app layer                         
        PING_ReleaseRxBuffer();
        g_tTest.tCntr.nRxLostCntr++;
        return;
    }
    
    // Check illegal frame
    if (!PLC_FrmDec(pBuf, nLen, &g_tTest.tPlcFrmHdr))
    {
        g_tTest.tCntr.nRxErrCntr++;
        //LOG("APP_FrmRcvd: Illegal frm rcvd, len=%d\n", nLen);                    
    }
    else    
        g_tTest.nRxBufBusy = 1;  //decode success, to be processed   
    
    printf("\r\nUSER_DATA:");
    for(uint32_t i=0; i<nLen; i++)
    {
        printf("%02x", pBuf[i]);
    }
   
    PING_ReleaseRxBuffer();  
}

static TBool TEST_PING_SN_Init(unsigned char *pDI, unsigned short nDILen, TestPing_SN_TypeDef *pSnPara)
{      
    if ((pDI == NULL) || (nDILen == 0) || (nDILen > MTR_ADDR_MAXLEN)) return TFalse;
    
    memset((TU8 *)&g_tTest, 0, sizeof(g_tTest));        
  
    g_tTest.pSnPara = pSnPara;

    if(g_tTest.pSnPara != NULL)
    {       
        g_tTest.nMyDILen = g_tTest.pSnPara->nDILen;
         memcpy(g_tTest.szMyDI, g_tTest.pSnPara->szDI, g_tTest.nMyDILen);
         //TIMER_SetDelay_ms(&g_tTest.tTmTest, (TU32)(g_tTest.pSnPara->nTestTime)*1000);
    }
    else
    {
        g_tTest.nMyDILen = (TU8)nDILen;
        memcpy(g_tTest.szMyDI, pDI, g_tTest.nMyDILen);       
        //TIMER_SetDelay_ms(&g_tTest.tTmTest, 36000000L); //ten hours
    }
           
    g_tTest.nRxBufBusy = 0;
    //TEST_AMR_COMMON_Init();
    //TEST_AMR_LED_Init();
      
    if(DrvPlcInit() != ERR_SUCCESS) return TFalse;
    PING_GetDefaultPlcCfg(&g_tTest.tPlcCfg);
    if (Drv_Configure(&g_tTest.tPlcCfg) != ERR_SUCCESS) return TFalse;                                     
    return TTrue;         
}

void TEST_PING_SN(TestPing_SN_TypeDef *pSnPara)
{
    TU8  szTemp[MTR_ADDR_MAXLEN];
    
    memcpy(szTemp, MTR_ADDR_SN1, MTR_ADDR_LEN);            
    if (!TEST_PING_SN_Init(szTemp, MTR_ADDR_LEN, pSnPara)) return ;
    
//    while (1)
//    {
//        DrvPlcFsm();
//
//        APP_SN_FSM();
//        vTaskDelay(4000);
//    }
}


void TEST_PING_MN(TestPing_SN_TypeDef *pSnPara)
{
    TU8         szTemp[MTR_ADDR_MAXLEN];
    
    memcpy(szTemp, MTR_ADDR_SN1, MTR_ADDR_LEN);            
    if (!TEST_PING_SN_Init(szTemp, MTR_ADDR_LEN, pSnPara)) return ;
    
//    while (1)
//    {
//        //DrvPlcFsm(); 
//        
//        APP_SN_FSM();
//    }
}



void TEST_PING_FetchAppCntr(TCommCntr *pAppCntr)
{
    (*pAppCntr) = g_tTest.tCntr;  
}
void TEST_PING_ClrAppCntr(void)
{
    memset((TU8 *)&(g_tTest.tCntr), 0, sizeof(TCommCntr)); 
}