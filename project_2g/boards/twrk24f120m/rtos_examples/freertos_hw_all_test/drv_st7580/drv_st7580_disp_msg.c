/******************** (C) COPYRIGHT 2009 STMicroelectronics ********************/
/**
 * @file    : drv_st7580_disp_msg.c
 * @brief   : ST7580 driver.
 * @author  : Alex Li (alex.li@st.com)
 * @version : V1.0
 * @date    : 02/21/2010
*
**********************************************************************************
*              (C) COPYRIGHT 2009 STMicroelectronics                        <br>
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS  
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT, 
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE 
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING?
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
**********************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#ifdef WIN32
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "st_hal_api.h"
#include "drv_st7580.h"

/* Private define ------------------------------------------------------------*/

//! Error codes used by all modules
//! General success
#define ERR_SUCCESS		0x00
//! Wrong command code 
#define ERR_WCMD		0x01
//! Wrong number of parameters
#define ERR_WPL         0x02    
//! Wrong parameter value
#define ERR_WPV         0x03
//! Sub-System current state not compatible with requested operation (not configured, busy, etc..)
#define ERR_BUSY			0x04
//! Sub-System is not physically present
#define ERR_NOT_PRESENT		0x06
//! Sub-System is disabled by a software option
#define ERR_DISABLED		0x07
//! Sub-System abort operation due to internal timeout
#define ERR_TIMEOUT			0x08
//! Sub-System find out a data corruption
#define ERR_DATA_CORRUPTION	0x09
//! General error
#define ERR_ERROR		0xFF

//! Module specific error prefix
//! Prefix for FLASH specific errors
#define ERR_HI_PREFIX	0x00
//! Prefix for FLASH specific errors
#define ERR_FLASH_PREFIX	0x20
//! Prefix for DFU specific errors
#define ERR_DFU_PREFIX	0x40
//! Prefix for MAC specific errors
#define ERR_MAC_PREFIX	0x60
//! Prefix for PHY specific errors
#define ERR_PHY_PREFIX	0x80
//! Prefix for SS specific errors
#define ERR_SS_PREFIX	0xa0


static void FormatCommand (char *str, unsigned char ucCommandCode)
{
    switch (ucCommandCode)
    {                       
        case PHY_DataRequest_CODE:
            sprintf(str,"%sPHY_DataRequest", str);
            break;
        case PHY_DataConfirm_CODE:
            sprintf(str,"%sPHY_DataConfirm", str);
            break;
        case PHY_DataIndication_CODE:
            sprintf(str,"%sPHY_DataIndication", str);
            break;
        case DL_DataRequest_CODE:
            sprintf(str,"%sDL_DataRequest", str);
            break;
        case DL_DataConfirm_CODE:
            sprintf(str,"%sDL_DataConfirm", str);
            break;
        case DL_DataIndication_CODE:
            sprintf(str,"%sDL_DataIndication", str);
            break;
        case SS_DataRequest_CODE:
            sprintf(str,"%sSS_DataRequest", str);
            break;
        case SS_DataConfirm_CODE:
            sprintf(str,"%sSS_DataConfirm", str);
            break;
        case SS_DataIndication_CODE:
            sprintf(str,"%sSS_DataIndication", str);
            break;
			
        case DL_SnifferIndication_CODE:
            sprintf(str,"%sDL_SnifferIndication", str);
            break;
        case SS_SnifferIndication_CODE:
            sprintf(str,"%sSS_SnifferIndication", str);
            break;
       case BIO_ResetRequest_CODE:
            sprintf(str,"%sBIO_ResetIndication", str);
            break;
       case BIO_ResetIndication_CODE:
            sprintf(str,"%sBIO_ResetIndication", str);
            break;
        case PingRequest_CODE:
            sprintf(str,"%sPING_Request", str);
            break;
        case PingConfirm_CODE:
            sprintf(str,"%sPING_Confirm", str);
            break;
        case MIB_WriteRequest_CODE:
            sprintf(str,"%sMIB_WriteRequest", str);
            break;
        case MIB_WriteConfirm_CODE:
            sprintf(str,"%sMIB_WriteConfirm", str);
            break;			
        case MIB_ReadRequest_CODE:
            sprintf(str,"%sMIB_ReadRequest", str);
            break;
        case MIB_ReadConfirm_CODE:
            sprintf(str,"%sMIB_ReadConfirm", str);
            break;
        case MIB_EraseRequest_CODE:
            sprintf(str,"%sMIB_EraseRequest", str);
            break;
        case MIB_EraseConfirm_CODE:
            sprintf(str,"%sMIB_EraseConfirm", str);
            break;
        case BIO_ResetError_CODE:
            sprintf(str,"%sBIO_ResetError", str);
            break;
        case MIB_WriteError_CODE:
            sprintf(str,"%sMIB_WriteError", str);
            break;
        case MIB_ReadError_CODE:
            sprintf(str,"%sMIB_ReadError", str);
            break;
        case MIB_EraseError_CODE:
            sprintf(str,"%sMIB_EraseError", str);
            break;
        case PHY_DataError_CODE:
            sprintf(str,"%sPHY_DataError", str);
            break;
        case DL_DataError_CODE:
            sprintf(str,"%sDL_DataError", str);
            break;
        case SS_DataError_CODE:
            sprintf(str,"%sSS_DataError", str);
            break;
        case CMD_SyntaxError_CODE:
            sprintf(str,"%sCMD_SyntaxError", str);
            break;	
        default:
            sprintf(str,"%sUndefined command code", str);
            break;
    }
}
#if 0
static void ERR_ErrorIndication (TU8 nCmd, unsigned char buffer[], unsigned char length)
{
    char rawStr[256];
	TU8  i;
    
	switch(nCmd)
	{
        case BIO_ResetError_CODE:
            sprintf(rawStr,"BIO_ResetError on");
            break;
        case MIB_WriteError_CODE:
            sprintf(rawStr,"MIB_WriteError on");
            break;
        case MIB_ReadError_CODE:
            sprintf(rawStr,"MIB_ReadError on");
            break;
        case MIB_EraseError_CODE:
            sprintf(rawStr,"MIB_EraseError on");
            break;
        case PHY_DataError_CODE:
            sprintf(rawStr,"PHY_DataError on");
            break;
        case DL_DataError_CODE:
            sprintf(rawStr,"DL_DataError on");
            break;
        case SS_DataError_CODE:
            sprintf(rawStr,"SS_DataError on");
            break;
        case CMD_SyntaxError_CODE:
            sprintf(rawStr,"CMD_SyntaxError on");
            break;
		default:
            sprintf(rawStr,"Other_Error on");
            break;
	}       
    
    switch(buffer[0])
    {
    case ErrorCause_WPL:
        sprintf(rawStr, "%s Wrong Parameter Length", rawStr);
        break;
    case ErrorCause_WPV:
        sprintf(rawStr, "%s Wrong Parameter Value", rawStr);
        break;
    case ErrorCause_BUSY:
        sprintf(rawStr, "%s Busy", rawStr);
        break;
    case ErrorCause_HEAT:
        sprintf(rawStr, "%s Thermal error", rawStr);
        break;
    case ErrorCause_GEN:
        sprintf(rawStr, "%s General error", rawStr);
        break;            
    default:
		if(nCmd == CMD_SyntaxError_CODE)
		{
            sprintf(rawStr, "%s len=%d: ", rawStr, length);
            for(i=0; i<length; i++)
                sprintf(rawStr, "%s0x%02x,", rawStr, buffer[i]);
		}
        else
		    sprintf(rawStr, "%s Undefined system code [0x%02X] ", rawStr, buffer[0]);
        break;
    }        
    
    LOG(rawStr);
}
#endif

TVoid Disp_PlcMsg(TU8 nCmd, TU8 *pData, TU8 nDataLen)
{
	TU8  i;
	/*
    if (nCmd == ERR_ErrorIndication_CODE)
    {
        ERR_ErrorIndication(nCmd, pData, nDataLen);
    }
    else */
    {
        TU8 szBuf[256];
        
        szBuf[0] = 0;
        FormatCommand((char *)szBuf, nCmd);		        
        LOG("MsgRcvd: [%s]cmd=0x%02X, len=%d, data=", szBuf, nCmd, nDataLen);
		memset(szBuf, 0x00, sizeof(szBuf));		
		if(nCmd == PHY_DataIndication_CODE || nCmd == DL_DataIndication_CODE || nCmd == SS_DataIndication_CODE)
		{
            LOG("\n");
		}
		            
            for(i=0; i<nDataLen; i++)
			{
                sprintf(szBuf, "%s%02x,", szBuf, pData[i]);
				if((i+1)%81 == 0)
				{
                    LOG("%s\n", szBuf); 
                    memset(szBuf, 0x00, sizeof(szBuf));
				}
			}
		/*
		if(nCmd == PHY_DataIndication_CODE || nCmd == DL_DataIndication_CODE || nCmd == SS_DataIndication_CODE)
		{
            for(i=0; i<4; i++)
                sprintf(szBuf, "%s%02x,", szBuf, pData[i]);
		}
		else
		{            
            for(i=0; i<nDataLen; i++)
			{
                sprintf(szBuf, "%s%02x,", szBuf, pData[i]);
				if((i+1)%81 == 0)
				{
                    LOG("%s\n", szBuf); 
                    memset(szBuf, 0x00, sizeof(szBuf));
				}
			}
		}*/
		LOG("%s\n\n", szBuf); 
    }
}
#endif