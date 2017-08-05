/* plc_codec.c */
#include "plc_codec_ping.h"
#include "string.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

TU16 PLC_FrmEnc(TU8 * pFrm, TPlcFrmHdr * pHdr)//from pHdr to pFrm
{
    TU8  *  pCur = pFrm;   
    
    *pCur++ = 0x68;   
    memcpy(pCur, pHdr->szDI, 6);
    pCur += 6;
    *pCur++ = 0x68;        
    
    if(pHdr->nDataL > APP_FRM_MAXLEN - (TU8)(pCur - pFrm)) return 0;
    if(pHdr->nDataL > 0)
    {
        memcpy(pCur, pHdr->szData, pHdr->nDataL);
        pCur += pHdr->nDataL;
    }

    return (TU8)(pCur - pFrm);
}

TBool PLC_FrmDec(TU8 * pFrm, TU16 nLen, TPlcFrmHdr * pHdr)//from pFrm and nLen to pHdr
{
    TU8  *  pCur = pFrm;
    TU8  uChar;

    if (pFrm == NULL || nLen < APP_FRM_BAIC_LEN) return TFalse;
    
    uChar = *pCur++;
    if(uChar != 0x68) return TFalse;      
       
    memcpy(pHdr->szDI, pCur, 6);
    pCur += 6;
    
    uChar = *pCur++;
    if(uChar != 0x68) return TFalse;         
    
    pHdr->nDataL = nLen - (TU8)(pCur - pFrm);        
    if(pHdr->nDataL == 0) return TFalse;
    memcpy(pHdr->szData, pCur, pHdr->nDataL);
        
    return TTrue;
}




