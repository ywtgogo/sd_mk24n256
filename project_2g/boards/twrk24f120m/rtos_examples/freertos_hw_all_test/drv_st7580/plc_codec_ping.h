/* plc_codec.h */

#ifndef _PLC_FRAME_CODEC_H_
#define _PLC_FRAME_CODEC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "test_ping.h"

/** 
  Format: //the maximum length is 251 bytes
  { 68 | DI[6] | 68 | Dat }  //low byte first
*/
    
#define APP_FRM_BAIC_LEN  (8)
  
typedef struct {       
  
    TU8 szDI[6];           
    
    TU8 nDataL;
    TU8 szData[APP_FRM_MAXLEN];
} TPlcFrmHdr;

TU16  PLC_FrmEnc(TU8 * pFrm, TPlcFrmHdr * pHdr);
TBool PLC_FrmDec(TU8 * pFrm, TU16 nLen, TPlcFrmHdr * pHdr);

#ifdef __cplusplus
}
#endif


#endif /* _PLC_FRAME_CODEC_H_ */
