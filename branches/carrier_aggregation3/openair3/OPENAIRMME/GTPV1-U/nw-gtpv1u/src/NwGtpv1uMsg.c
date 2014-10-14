/*----------------------------------------------------------------------------*
 *                                                                            *
 *                             n w - g t p v 2 u                              * 
 *    G P R S   T u n n e l i n g    P r o t o c o l   v 2 u    S t a c k     *
 *                                                                            *
 *                                                                            *
 * Copyright (c) 2010-2011 Amit Chawre                                        *
 * All rights reserved.                                                       *
 *                                                                            *
 * Redistribution and use in source and binary forms, with or without         *
 * modification, are permitted provided that the following conditions         *
 * are met:                                                                   *
 *                                                                            *
 * 1. Redistributions of source code must retain the above copyright          *
 *    notice, this list of conditions and the following disclaimer.           *
 * 2. Redistributions in binary form must reproduce the above copyright       *
 *    notice, this list of conditions and the following disclaimer in the     *
 *    documentation and/or other materials provided with the distribution.    *
 * 3. The name of the author may not be used to endorse or promote products   *
 *    derived from this software without specific prior written permission.   *
 *                                                                            *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR       *
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES  *
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.    *
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,           *
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT   *
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  *
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY      *
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT        *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF   *
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.          *
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "NwTypes.h"
#include "NwLog.h"
#include "NwUtils.h"
#include "NwGtpv1uLog.h"
#include "NwGtpv1u.h"
#include "NwGtpv1uPrivate.h"
#include "NwGtpv1uMsg.h"

#define NW_GTPV1U_EPC_SPECIFIC_HEADER_SIZE                             (12)   /**< Size of GTPv1u EPC specific header */

#ifdef __cplusplus
extern "C" {
#endif

static NwGtpv1uMsgT* gpGtpv1uMsgPool = NULL;

NwGtpv1uRcT
nwGtpv1uMsgNew( NW_IN NwGtpv1uStackHandleT hGtpuStackHandle,
                NW_IN NwU8T     seqNumFlag,
                NW_IN NwU8T     npduNumFlag,
                NW_IN NwU8T     extHdrFlag,
                NW_IN NwU8T     msgType,
                NW_IN NwU8T     teid,
                NW_IN NwU16T    seqNum,
                NW_IN NwU8T     npduNum,
                NW_IN NwU8T     nextExtHeader,
                NW_OUT NwGtpv1uMsgHandleT *phMsg)
{
  NwGtpv1uStackT* pStack = (NwGtpv1uStackT*) hGtpuStackHandle;
  NwGtpv1uMsgT *pMsg;

  if(gpGtpv1uMsgPool)
  {
    pMsg = gpGtpv1uMsgPool;
    gpGtpv1uMsgPool = gpGtpv1uMsgPool->next;
  }
  else
  {
    NW_GTPV1U_MALLOC(pStack, sizeof(NwGtpv1uMsgT), pMsg, NwGtpv1uMsgT*);
  }


  if(pMsg)
  {
    pMsg->version       = NW_GTPU_VERSION;
    pMsg->protocolType  = NW_GTP_PROTOCOL_TYPE_GTP;
    pMsg->seqNumFlag    = seqNumFlag;
    pMsg->npduNumFlag   = npduNumFlag;
    pMsg->extHdrFlag    = extHdrFlag;
    pMsg->msgType       = msgType;

    if(seqNumFlag)
      pMsg->seqNum        = seqNum;
    if(npduNumFlag)
      pMsg->npduNum       = npduNum;
    if(extHdrFlag)
      pMsg->nextExtHdrType = nextExtHeader;

    pMsg->msgLen        = ((pMsg->seqNumFlag || pMsg->npduNumFlag || pMsg->extHdrFlag) ? 
        NW_GTPV1U_EPC_SPECIFIC_HEADER_SIZE : (NW_GTPV1U_EPC_SPECIFIC_HEADER_SIZE - 4));

    *phMsg = (NwGtpv1uMsgHandleT) pMsg;
    return NW_GTPV1U_OK;
  }

  return NW_GTPV1U_FAILURE;
}

NwGtpv1uRcT
nwGtpv1uGpduMsgNew( NW_IN NwGtpv1uStackHandleT hGtpuStackHandle,
                NW_IN NwU32T    teid,
                NW_IN NwU8T     seqNumFlag,
                NW_IN NwU16T    seqNum,
                NW_IN NwU8T*    tpdu,
                NW_IN NwU16T    tpduLength,
                NW_OUT NwGtpv1uMsgHandleT *phMsg)
{
  NwGtpv1uStackT* pStack = (NwGtpv1uStackT*) hGtpuStackHandle;
  NwGtpv1uMsgT *pMsg;

  if(gpGtpv1uMsgPool)
  {
    pMsg = gpGtpv1uMsgPool;
    gpGtpv1uMsgPool = gpGtpv1uMsgPool->next;
  }
  else
  {
    NW_GTPV1U_MALLOC(pStack, sizeof(NwGtpv1uMsgT), pMsg, NwGtpv1uMsgT*);
  }

  if(pMsg)
  {
    pMsg->version       = NW_GTPU_VERSION;
    pMsg->protocolType  = NW_GTP_PROTOCOL_TYPE_GTP;
    pMsg->extHdrFlag    = NW_FALSE;
    pMsg->seqNumFlag    = (seqNumFlag? NW_TRUE : NW_FALSE);
    pMsg->npduNumFlag   = NW_FALSE;
    pMsg->msgType       = NW_GTP_GPDU;
    pMsg->teid          = teid;
    pMsg->seqNum        = seqNum;
    pMsg->npduNum       = 0x00;
    pMsg->nextExtHdrType= 0x00;
    pMsg->msgLen        = ((pMsg->seqNumFlag || pMsg->npduNumFlag || pMsg->extHdrFlag ) ? 
                          NW_GTPV1U_EPC_SPECIFIC_HEADER_SIZE : (NW_GTPV1U_EPC_SPECIFIC_HEADER_SIZE - 4));

    memcpy(pMsg->msgBuf + pMsg->msgLen, tpdu, tpduLength);
    pMsg->msgLen        += tpduLength;

    *phMsg = (NwGtpv1uMsgHandleT) pMsg;
    return NW_GTPV1U_OK;
  }

  return NW_GTPV1U_FAILURE;
}

NwGtpv1uRcT
nwGtpv1uMsgFromMsgNew( NW_IN NwGtpv1uStackHandleT hGtpuStackHandle,
                       NW_IN NwGtpv1uMsgHandleT hMsg,
                       NW_OUT NwGtpv1uMsgHandleT *phMsg)
{
  NwGtpv1uStackT* pStack = (NwGtpv1uStackT*) hGtpuStackHandle;
  NwGtpv1uMsgT *pMsg;

  if(gpGtpv1uMsgPool)
  {
    pMsg = gpGtpv1uMsgPool;
    gpGtpv1uMsgPool = gpGtpv1uMsgPool->next;
  }
  else
  {
    NW_GTPV1U_MALLOC(pStack, sizeof(NwGtpv1uMsgT), pMsg, NwGtpv1uMsgT*);
  }


  if(pMsg)
  {
    memcpy(pMsg, (NwGtpv1uMsgT*)hMsg, sizeof(NwGtpv1uMsgT));
    *phMsg = (NwGtpv1uMsgHandleT) pMsg;
    return NW_GTPV1U_OK;
  }
  return NW_GTPV1U_FAILURE;
}

NwGtpv1uRcT
nwGtpv1uMsgFromBufferNew( NW_IN NwGtpv1uStackHandleT hGtpuStackHandle,
                         NW_IN NwU8T* pBuf,
                         NW_IN NwU32T bufLen,
                         NW_OUT NwGtpv1uMsgHandleT *phMsg)
{
  NwGtpv1uStackT* pStack = (NwGtpv1uStackT*) hGtpuStackHandle;
  NwGtpv1uMsgT *pMsg;

  if(gpGtpv1uMsgPool)
  {
    pMsg = gpGtpv1uMsgPool;
    gpGtpv1uMsgPool = gpGtpv1uMsgPool->next;
  }
  else
  {
    NW_GTPV1U_MALLOC(pStack, sizeof(NwGtpv1uMsgT), pMsg, NwGtpv1uMsgT*);
  }


  if(pMsg)
  {
    memcpy(pMsg->msgBuf, pBuf, bufLen);
    pMsg->msgLen = bufLen;

    pMsg->version       = ((*pBuf) & 0xE0) >> 5;
    pMsg->protocolType  = ((*pBuf) & 0x10) >> 4;
    pMsg->extHdrFlag    = ((*pBuf) & 0x04) >> 2;
    pMsg->seqNumFlag    = ((*pBuf) & 0x02) >> 1;
    pMsg->npduNumFlag   = ((*pBuf) & 0x01);
    pBuf++;

    pMsg->msgType       = *(pBuf);
    pBuf++;

    pBuf += 2;

    pMsg->teid          = ntohl(*((NwU32T*)pBuf));
    pBuf += 4;

    if(pMsg->extHdrFlag || pMsg->seqNumFlag || pMsg->npduNumFlag)
    {
      pMsg->seqNum              = ntohs(*(((NwU16T*)pBuf)));
      pBuf += 2;
      pMsg->npduNum             = *(pBuf++);
      pMsg->nextExtHdrType      = *(pBuf++);
    }
    *phMsg = (NwGtpv1uMsgHandleT) pMsg;
    return NW_GTPV1U_OK;
  }
  return NW_GTPV1U_FAILURE;
}

NwGtpv1uRcT
nwGtpv1uMsgDelete( NW_IN NwGtpv1uStackHandleT hGtpuStackHandle,
                   NW_IN NwGtpv1uMsgHandleT hMsg)
{
  ((NwGtpv1uMsgT*)hMsg)->next = gpGtpv1uMsgPool;
  gpGtpv1uMsgPool = (NwGtpv1uMsgT*) hMsg;
  return NW_GTPV1U_OK;
}

 /**
  * Set TEID for gtpv1u message.
  *
  * @param[in] hMsg : Message handle.
  * @param[in] teid: TEID value.
  */

NwGtpv1uRcT
nwGtpv1uMsgSetTeid(NW_IN NwGtpv1uMsgHandleT hMsg, NwU32T teid)
{
  NwGtpv1uMsgT *thiz = (NwGtpv1uMsgT*) hMsg;
  thiz->teid = teid; 
  return NW_GTPV1U_OK;
}

 /**
  * Set sequence for gtpv1u message.
  *
  * @param[in] hMsg : Message handle.
  * @param[in] seqNum: Flag boolean value.
  */

NwGtpv1uRcT
nwGtpv1uMsgSetSeqNumber(NW_IN NwGtpv1uMsgHandleT hMsg, NwU32T seqNum)
{
  NwGtpv1uMsgT *thiz = (NwGtpv1uMsgT*) hMsg;
  thiz->seqNum = seqNum; 
  return NW_GTPV1U_OK;
}

 /**
  * Get TEID present for gtpv1u message.
  *
  * @param[in] hMsg : Message handle.
  */

NwU32T
nwGtpv1uMsgGetTeid(NW_IN NwGtpv1uMsgHandleT hMsg)
{
  NwGtpv1uMsgT *thiz = (NwGtpv1uMsgT*) hMsg;
  return (thiz->teid); 
}


 /**
  * Get sequence number for gtpv1u message.
  *
  * @param[in] hMsg : Message handle.
  */

NwU32T
nwGtpv1uMsgGetSeqNumber(NW_IN NwGtpv1uMsgHandleT hMsg)
{
  NwGtpv1uMsgT *thiz = (NwGtpv1uMsgT*) hMsg;
  return (thiz->seqNum);
}

 /**
  * Get msg type for gtpv1u message.
  *
  * @param[in] hMsg : Message handle.
  */

NwU32T
nwGtpv1uMsgGetMsgType(NW_IN NwGtpv1uMsgHandleT hMsg)
{
  NwGtpv1uMsgT *thiz = (NwGtpv1uMsgT*) hMsg;
  return (thiz->msgType);
}

 /**
  * Get tpdu for gtpv1u message.
  *
  * @param[in] hMsg : Message handle.
  */

NwGtpv1uRcT
nwGtpv1uMsgGetTpdu(NW_IN NwGtpv1uMsgHandleT hMsg, NwU8T* pTpduBuf, NwU32T* pTpduLength)
{
  NwGtpv1uMsgT *thiz = (NwGtpv1uMsgT*) hMsg;
  NwU8T headerLength = ((thiz->seqNumFlag || thiz->extHdrFlag || thiz->npduNumFlag) ? 12 : 8);

  *pTpduLength = thiz->msgLen - headerLength;
  memcpy(pTpduBuf, thiz->msgBuf + headerLength, *pTpduLength);
  return NW_GTPV1U_OK;
}

NwU8T*
nwGtpv1uMsgGetTpduHandle(NW_IN NwGtpv1uMsgHandleT hMsg)
{
  NwGtpv1uMsgT *thiz = (NwGtpv1uMsgT*) hMsg;
  return (thiz->msgBuf + ((thiz->seqNumFlag || thiz->extHdrFlag || thiz->npduNumFlag) ? 12 : 8));
}

NwU32T
nwGtpv1uMsgGetTpduLength(NW_IN NwGtpv1uMsgHandleT hMsg)
{
  NwGtpv1uMsgT *thiz = (NwGtpv1uMsgT*) hMsg;
  return (thiz->msgLen - ((thiz->seqNumFlag || thiz->extHdrFlag || thiz->npduNumFlag) ? 12 : 8));
}

NwGtpv1uRcT
nwGtpv1uMsgAddIeTV1(NW_IN NwGtpv1uMsgHandleT hMsg, 
              NW_IN NwU8T       type,
              NW_IN NwU8T       value)
{
  NwGtpv1uMsgT *pMsg = (NwGtpv1uMsgT*) hMsg;
  NwGtpv1uIeTv1T *pIe;

  pIe = (NwGtpv1uIeTv1T*) (pMsg->msgBuf + pMsg->msgLen);

  pIe->t        = type;
  pIe->v        = value;

  pMsg->msgLen += sizeof(NwGtpv1uIeTv1T);

  return NW_GTPV1U_OK;
}

NwGtpv1uRcT
nwGtpv1uMsgAddIeTV2(NW_IN NwGtpv1uMsgHandleT hMsg, 
              NW_IN NwU8T       type,
              NW_IN NwU16T      length,
              NW_IN NwU16T      value)
{
  NwGtpv1uMsgT *pMsg = (NwGtpv1uMsgT*) hMsg;
  NwGtpv1uIeTv2T *pIe;

  pIe = (NwGtpv1uIeTv2T*) (pMsg->msgBuf + pMsg->msgLen);

  pIe->t        = type;
  pIe->v        = htons(value);

  pMsg->msgLen += sizeof(NwGtpv1uIeTv2T);

  return NW_GTPV1U_OK;
}

NwGtpv1uRcT
nwGtpv1uMsgAddIeTV4(NW_IN NwGtpv1uMsgHandleT hMsg, 
              NW_IN NwU8T       type,
              NW_IN NwU16T      length,
              NW_IN NwU32T      value)
{
  NwGtpv1uMsgT *pMsg = (NwGtpv1uMsgT*) hMsg;
  NwGtpv1uIeTv4T *pIe;

  pIe = (NwGtpv1uIeTv4T*) (pMsg->msgBuf + pMsg->msgLen);

  pIe->t        = type;
  pIe->v        = htonl(value);

  pMsg->msgLen += sizeof(NwGtpv1uIeTv4T);

  return NW_GTPV1U_OK;
}

NwGtpv1uRcT
nwGtpv1uMsgAddIe(NW_IN NwGtpv1uMsgHandleT hMsg, 
              NW_IN NwU8T       type,
              NW_IN NwU16T      length,
              NW_IN NwU8T*      pVal)
{
  NwGtpv1uMsgT *pMsg = (NwGtpv1uMsgT*) hMsg;
  NwGtpv1uIeTlvT *pIe;

  pIe = (NwGtpv1uIeTlvT*) (pMsg->msgBuf + pMsg->msgLen);

  pIe->t        = type;
  pIe->l        = htons(length);

  memcpy(pIe + 4, pVal, length);
  pMsg->msgLen += (4 + length);

  return NW_GTPV1U_OK;
}

NwGtpv1uRcT
nwGtpv1uMsgHexDump(NwGtpv1uMsgHandleT hMsg, FILE* fp)
{

  NwGtpv1uMsgT* pMsg = (NwGtpv1uMsgT*) hMsg;
  NwU8T* data = pMsg->msgBuf;
  NwU32T size = pMsg->msgLen;

  unsigned char *p = (unsigned char*)data;
  unsigned char c;
  int n;
  char bytestr[4] = {0};
  char addrstr[10] = {0};
  char hexstr[ 16*3 + 5] = {0};
  char charstr[16*1 + 5] = {0};
  fprintf((FILE*)fp, "\n");
  for(n=1;n<=size;n++) {
    if (n%16 == 1) {
      /* store address for this line */
      snprintf(addrstr, sizeof(addrstr), "%.4x",
          ((unsigned int)p-(unsigned int)data) );
    }

    c = *p;
    if (isalnum(c) == 0) {
      c = '.';
    }

    /* store hex str (for left side) */
    snprintf(bytestr, sizeof(bytestr), "%02X ", *p);
    strncat(hexstr, bytestr, sizeof(hexstr)-strlen(hexstr)-1);

    /* store char str (for right side) */
    snprintf(bytestr, sizeof(bytestr), "%c", c);
    strncat(charstr, bytestr, sizeof(charstr)-strlen(charstr)-1);
    if(n%16 == 0) {
      /* line completed */
      fprintf((FILE*)fp, "[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
      hexstr[0] = 0;
      charstr[0] = 0;
    } else if(n%8 == 0) {
      /* half line: add whitespaces */
      strncat(hexstr, "  ", sizeof(hexstr)-strlen(hexstr)-1);
      strncat(charstr, " ", sizeof(charstr)-strlen(charstr)-1);
    }
    p++; /* next byte */
  }

  if (strlen(hexstr) > 0) {
    /* print rest of buffer if not empty */
    fprintf((FILE*)fp, "[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);

  }
  fprintf((FILE*)fp, "\n");

  return NW_GTPV1U_OK;
}

#ifdef __cplusplus
}
#endif



/*--------------------------------------------------------------------------*
 *                          E N D   O F   F I L E                           * 
 *--------------------------------------------------------------------------*/

