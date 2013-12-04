#if defined(UPDATED_BY_SPIRENT_ABACUS)

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "RvSipMsg.h"
#include "RvSipHeader.h"
#include "RvSipDateHeader.h"
#include "_SipMsg.h"
#include "_SipMsgMgr.h"
#include "_SipOtherHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipAddress.h"
#include "RvSipPartyHeader.h"
#include "RvSipCSeqHeader.h"
#include "RvSipAllowHeader.h"
#include "RvSipContactHeader.h"
#include "RvSipViaHeader.h"
#include "RvSipOtherHeader.h"
#include "RvSipAuthenticationHeader.h"
#include "RvSipAuthorizationHeader.h"
#include "RvSipExpiresHeader.h"
#include "RvSipReferToHeader.h"
#include "RvSipReferredByHeader.h"
#include "RvSipBody.h"
#include "RvSipContentTypeHeader.h"
#include "ParserProcess.h"
#include "RvSipRAckHeader.h"
#include "RvSipRSeqHeader.h"
#include "RvSipContentDispositionHeader.h"
#include "RvSipRetryAfterHeader.h"
#include "RvSipReplacesHeader.h"
#include "RvSipEventHeader.h"
#include "RvSipAllowEventsHeader.h"
#include "RvSipSubscriptionStateHeader.h"
#include "RvSipSessionExpiresHeader.h"
#include "RvSipMinSEHeader.h"
#include "_SipReplacesHeader.h"
#include "_SipTransport.h"
#include "RvSipSPIRENTEncoder.h"

#include "All/sip_params.h"

RVAPI RV_Status sortHeaderList(IN MsgMessage* pMsg);


#define MaxHeadersPerMsg 100
#define HIERARHY_DEPTH (3)
#define DELIM "; \n\r\t"    // delimiters of parameter value
void SPIRENT_RvSIP_HeaderListGetCacheInit ( void );
void* SPIRENT_RvSIP_GetHeaderFromList ( IN   RvSipMsgHandle   hMsg, int param );
size_t SPIRENT_RvSIP_GetParameterValueFromString ( MsgMessage * msg,
                                    const char * string, const char * delimiters,
                                    const char * parmName, char * parmValue, size_t parmBufferSize );

RV_INT32 SPIRENT_RvSIP_CalculateContentLength ( IN MsgMessage* msg, IN HRPOOL hPool, HPAGE*  hTempPage );
RV_INT32 SPIRENT_RvSIP_EncodeBody ( IN MsgMessage* msg, HPAGE* hTempPage, RV_INT32 tempLength, IN HRPOOL hPool, IN HPAGE hPage, OUT RV_UINT32* length );

unsigned char param_hierarhy_[256][HIERARHY_DEPTH+1] = {
{ SIPCFG_REQ_URI                    , SIPCFG_REQ_URI, 0, 0 },
{ SIPCFG_REQ_URI_USER               , SIPCFG_REQ_URI, SIPCFG_REQ_URI_USER, 0 },
{ SIPCFG_REQ_URI_HOST               , SIPCFG_REQ_URI, SIPCFG_REQ_URI_HOST, 0 },
{ SIPCFG_REQ_URI_TRANSPORT          , SIPCFG_REQ_URI, SIPCFG_REQ_URI_TRANSPORT, 0 },
{ SIPCFG_FROM                       , SIPCFG_FROM, 0, 0 },
{ SIPCFG_FROM_URI                   , SIPCFG_FROM, SIPCFG_FROM_URI, 0 },
{ SIPCFG_FROM_URI_USER              , SIPCFG_FROM, SIPCFG_FROM_URI, SIPCFG_FROM_URI_USER },
{ SIPCFG_FROM_URI_HOST              , SIPCFG_FROM, SIPCFG_FROM_URI, SIPCFG_FROM_URI_HOST },
{ SIPCFG_FROM_URI_TRANSPORT         , SIPCFG_FROM, SIPCFG_FROM_URI, SIPCFG_FROM_URI_TRANSPORT },
{ SIPCFG_FROM_TAG                   , SIPCFG_FROM, SIPCFG_FROM_TAG, 0 },
{ SIPCFG_TO                         , SIPCFG_TO, 0, 0 },
{ SIPCFG_TO_URI                     , SIPCFG_TO, SIPCFG_TO_URI, 0 },
{ SIPCFG_TO_URI_USER                , SIPCFG_TO, SIPCFG_TO_URI, SIPCFG_TO_URI_USER },
{ SIPCFG_TO_URI_HOST                , SIPCFG_TO, SIPCFG_TO_URI, SIPCFG_TO_URI_HOST },
{ SIPCFG_TO_URI_TRANSPORT           , SIPCFG_TO, SIPCFG_TO_URI, SIPCFG_TO_URI_TRANSPORT },
{ SIPCFG_TO_TAG                     , SIPCFG_TO, SIPCFG_TO_TAG, 0 },
{ SIPCFG_CALLID                     , SIPCFG_CALLID, 0, 0 },
{ SIPCFG_CSEQ                       , SIPCFG_CSEQ, 0, 0 },
{ SIPCFG_CSEQ_STEP                  , SIPCFG_CSEQ, SIPCFG_CSEQ_STEP, 0 },
{ SIPCFG_CSEQ_METHOD                , SIPCFG_CSEQ, SIPCFG_CSEQ_METHOD, 0 },
{ SIPCFG_VIA1                       , SIPCFG_VIA1, 0, 0 },
{ SIPCFG_VIA1_TRANSPORT             , SIPCFG_VIA1, SIPCFG_VIA1_TRANSPORT, 0 },
{ SIPCFG_VIA1_IP                    , SIPCFG_VIA1, SIPCFG_VIA1_IP, 0 },
{ SIPCFG_VIA1_PORT                  , SIPCFG_VIA1, SIPCFG_VIA1_PORT, 0 },
{ SIPCFG_VIA1_BRANCH                , SIPCFG_VIA1, SIPCFG_VIA1_BRANCH, 0 },
{ SIPCFG_CONTACT1                   , SIPCFG_CONTACT1, 0, 0 },
{ SIPCFG_CONTACT1_URI               , SIPCFG_CONTACT1, SIPCFG_CONTACT1_URI, 0 },
{ SIPCFG_CONTACT1_URI_USER          , SIPCFG_CONTACT1, SIPCFG_CONTACT1_URI, SIPCFG_CONTACT1_URI_USER },
{ SIPCFG_CONTACT1_URI_HOST          , SIPCFG_CONTACT1, SIPCFG_CONTACT1_URI, SIPCFG_CONTACT1_URI_HOST },
{ SIPCFG_CONTACT1_URI_TRANSPORT     , SIPCFG_CONTACT1, SIPCFG_CONTACT1_URI, SIPCFG_CONTACT1_URI_TRANSPORT },
{ SIPCFG_CONTENTLENGTH              , SIPCFG_CONTENTLENGTH, 0, 0 },
{ SIPCFG_SDPBODY                    , SIPCFG_SDPBODY, 0, 0 },
{ SIPCFG_AUTHORIZATION1             , SIPCFG_AUTHORIZATION1, 0, 0 },
{ SIPCFG_AUTHORIZATION1_SCHEME      , SIPCFG_AUTHORIZATION1, SIPCFG_AUTHORIZATION1_SCHEME, 0 },
{ SIPCFG_AUTHORIZATION1_USERNAME    , SIPCFG_AUTHORIZATION1, SIPCFG_AUTHORIZATION1_USERNAME, 0 },
{ SIPCFG_AUTHORIZATION1_REALM       , SIPCFG_AUTHORIZATION1, SIPCFG_AUTHORIZATION1_REALM, 0 },
{ SIPCFG_AUTHORIZATION1_NONCE       , SIPCFG_AUTHORIZATION1, SIPCFG_AUTHORIZATION1_NONCE, 0 },
{ SIPCFG_AUTHORIZATION1_URI         , SIPCFG_AUTHORIZATION1, SIPCFG_AUTHORIZATION1_URI, 0 },
{ SIPCFG_AUTHORIZATION1_RESPONSE    , SIPCFG_AUTHORIZATION1, SIPCFG_AUTHORIZATION1_RESPONSE, 0 },
{ SIPCFG_AUTHORIZATION1_ALGORITHM   , SIPCFG_AUTHORIZATION1, SIPCFG_AUTHORIZATION1_ALGORITHM, 0 },
{ SIPCFG_AUTHORIZATION1_CNONCE      , SIPCFG_AUTHORIZATION1, SIPCFG_AUTHORIZATION1_CNONCE, 0 },
{ SIPCFG_AUTHORIZATION1_OPAQUE      , SIPCFG_AUTHORIZATION1, SIPCFG_AUTHORIZATION1_OPAQUE, 0 },
{ SIPCFG_AUTHORIZATION1_QOP         , SIPCFG_AUTHORIZATION1, SIPCFG_AUTHORIZATION1_QOP, 0 },
{ SIPCFG_AUTHORIZATION1_NC          , SIPCFG_AUTHORIZATION1, SIPCFG_AUTHORIZATION1_NC, 0 },
{ SIPCFG_AUTHORIZATION1_AUTHPARAM   , SIPCFG_AUTHORIZATION1, SIPCFG_AUTHORIZATION1_AUTHPARAM, 0 },
{ SIPCFG_RES_CODE                   , SIPCFG_RES_CODE, 0, 0 },
{ SIPCFG_RES_TEXT                   , SIPCFG_RES_TEXT, 0, 0 },
{ SIPCFG_VIA2                       , SIPCFG_VIA2, 0, 0 },
{ SIPCFG_VIA2_TRANSPORT             , SIPCFG_VIA2, SIPCFG_VIA2_TRANSPORT, 0 },
{ SIPCFG_VIA2_IP                    , SIPCFG_VIA2, SIPCFG_VIA2_IP, 0 },
{ SIPCFG_VIA2_PORT                  , SIPCFG_VIA2, SIPCFG_VIA2_PORT, 0 },
{ SIPCFG_VIA2_BRANCH                , SIPCFG_VIA2, SIPCFG_VIA2_BRANCH, 0 },
{ SIPCFG_CONTENTTYPE                , SIPCFG_CONTENTTYPE, 0, 0 },
{ SIPCFG_CONTENTTYPE_MEDIATYPE      , SIPCFG_CONTENTTYPE, SIPCFG_CONTENTTYPE_MEDIATYPE, 0 },
{ SIPCFG_CONTENTTYPE_MEDIASUBTYPE   , SIPCFG_CONTENTTYPE, SIPCFG_CONTENTTYPE_MEDIASUBTYPE, 0 },
{ SIPCFG_CONTENTTYPE_BOUNDARY       , SIPCFG_CONTENTTYPE, SIPCFG_CONTENTTYPE_BOUNDARY, 0 },
{ SIPCFG_CONTENTTYPE_VERSION        , SIPCFG_CONTENTTYPE, SIPCFG_CONTENTTYPE_VERSION, 0 },
{ SIPCFG_CONTENTTYPE_BASE           , SIPCFG_CONTENTTYPE, SIPCFG_CONTENTTYPE_BASE, 0 },
{ SIPCFG_RSEQ                       , SIPCFG_RSEQ, 0, 0 },
{ SIPCFG_RACK                       , SIPCFG_RACK, 0, 0 },             
{ SIPCFG_RACK_SEQ                   , SIPCFG_RACK, SIPCFG_RACK_SEQ, 0 },
{ SIPCFG_RACK_CSEQ                  , SIPCFG_RACK, SIPCFG_RACK_CSEQ, 0 },
{ SIPCFG_RACK_CSEQ_METHOD           , SIPCFG_RACK, SIPCFG_RACK_CSEQ_METHOD, 0 },
{ SIPCFG_VIA3                       , SIPCFG_VIA3, 0, 0 },
{ SIPCFG_VIA3_TRANSPORT             , SIPCFG_VIA3, SIPCFG_VIA3_TRANSPORT, 0 },
{ SIPCFG_VIA3_IP                    , SIPCFG_VIA3, SIPCFG_VIA3_IP, 0 },
{ SIPCFG_VIA3_PORT                  , SIPCFG_VIA3, SIPCFG_VIA3_PORT, 0 },
{ SIPCFG_VIA3_BRANCH                , SIPCFG_VIA3, SIPCFG_VIA3_BRANCH, 0 },
{ SIPCFG_ROUTE1                     , SIPCFG_ROUTE1,  0, 0 },
{ SIPCFG_ROUTE2                     , SIPCFG_ROUTE2,  0, 0 },
{ SIPCFG_ROUTE3                     , SIPCFG_ROUTE3,  0, 0 },
{ SIPCFG_ROUTE4                     , SIPCFG_ROUTE4,  0, 0 },
{ SIPCFG_ROUTE5                     , SIPCFG_ROUTE5,  0, 0 },
{ SIPCFG_ROUTE6                     , SIPCFG_ROUTE6,  0, 0 },
{ SIPCFG_ROUTE7                     , SIPCFG_ROUTE7,  0, 0 },
{ SIPCFG_ROUTE8                     , SIPCFG_ROUTE8,  0, 0 },
{ SIPCFG_EVENT                      , SIPCFG_EVENT,   0, 0 },
{ SIPCFG_EXPIRES                    , SIPCFG_EXPIRES, 0, 0 },
{ SIPCFG_SUBSCRIPTIONSTATE          , SIPCFG_SUBSCRIPTIONSTATE, 0, 0 },
{ SIPCFG_PROXYREQUIRE               , SIPCFG_PROXYREQUIRE, 0, 0 },
{ SIPCFG_CONTACT1_EXPIRES           , SIPCFG_CONTACT1, SIPCFG_CONTACT1_EXPIRES, 0 },
{ SIPCFG_CONTACT1_ISFOCUS           , SIPCFG_CONTACT1, SIPCFG_CONTACT1_ISFOCUS, 0 },
{ SIPCFG_AUTHORIZATION1_STALE       , SIPCFG_AUTHORIZATION1, SIPCFG_AUTHORIZATION1_STALE, 0 },
{ SIPCFG_SUBSCRIPTION_EXPIRES       , SIPCFG_SUBSCRIPTIONSTATE, SIPCFG_SUBSCRIPTION_EXPIRES, 0 },
{ SIPCFG_REFER_TO                   , SIPCFG_REFER_TO, 0, 0 },
{ SIPCFG_REFERRED_BY                , SIPCFG_REFERRED_BY, 0, 0 },
{ SIPCFG_REPLACES_CALLID            , SIPCFG_REPLACES_CALLID, 0, 0 },
{ SIPCFG_REPLACES_FROM_TAG          , SIPCFG_REPLACES_FROM_TAG, 0, 0 },
{ SIPCFG_REPLACES_TO_TAG            , SIPCFG_REPLACES_TO_TAG, 0, 0 },
{ SIPCFG_ALLOW                      , SIPCFG_ALLOW, 0, 0 },
{ SIPCFG_SUPPORTED_GRUU             , SIPCFG_SUPPORTED_GRUU, 0, 0 },
{ SIPCFG_SUPPORTED_JOIN             , SIPCFG_SUPPORTED_JOIN, 0, 0 },
{ SIPCFG_SUPPORTED_REPLACES         , SIPCFG_SUPPORTED_REPLACES, 0, 0 },
{ SIPCFG_SUPPORTED_100REL           , SIPCFG_SUPPORTED_100REL, 0, 0 },
{ SIPCFG_JOIN_CALLID                , SIPCFG_JOIN_CALLID, 0, 0 },
{ SIPCFG_JOIN_FROM_TAG              , SIPCFG_JOIN_FROM_TAG, 0, 0 },
{ SIPCFG_JOIN_TO_TAG                , SIPCFG_JOIN_TO_TAG, 0, 0 },
{ SIPCFG_ACCEPT1                    , SIPCFG_ACCEPT1, 0, 0 },
{ SIPCFG_ACCEPT2                    , SIPCFG_ACCEPT2, 0, 0 },
{ SIPCFG_ACCEPT3                    , SIPCFG_ACCEPT3, 0, 0 },
{ SIPCFG_ACCEPT_MEDIATYPE1          , SIPCFG_ACCEPT1, SIPCFG_ACCEPT_MEDIATYPE1, 0 },
{ SIPCFG_ACCEPT_MEDIATYPE2          , SIPCFG_ACCEPT2, SIPCFG_ACCEPT_MEDIATYPE2, 0 },
{ SIPCFG_ACCEPT_MEDIATYPE3          , SIPCFG_ACCEPT3, SIPCFG_ACCEPT_MEDIATYPE3, 0 },
{ SIPCFG_ACCEPT_MEDIASUBTYPE1       , SIPCFG_ACCEPT1, SIPCFG_ACCEPT_MEDIASUBTYPE1, 0 },
{ SIPCFG_ACCEPT_MEDIASUBTYPE2       , SIPCFG_ACCEPT2, SIPCFG_ACCEPT_MEDIASUBTYPE2, 0 },
{ SIPCFG_ACCEPT_MEDIASUBTYPE3       , SIPCFG_ACCEPT3, SIPCFG_ACCEPT_MEDIASUBTYPE3, 0 },
{ SIPCFG_ACCEPT_ENCODING1           , SIPCFG_ACCEPT_ENCODING1, 0, 0 },
{ SIPCFG_ACCEPT_ENCODING2           , SIPCFG_ACCEPT_ENCODING2, 0, 0 },
{ SIPCFG_ACCEPT_ENCODING3           , SIPCFG_ACCEPT_ENCODING3, 0, 0 },
{ SIPCFG_ACCEPT_LANGUAGE1           , SIPCFG_ACCEPT_LANGUAGE1, 0, 0 },
{ SIPCFG_ACCEPT_LANGUAGE2           , SIPCFG_ACCEPT_LANGUAGE2, 0, 0 },
{ SIPCFG_ACCEPT_LANGUAGE3           , SIPCFG_ACCEPT_LANGUAGE3, 0, 0 },
{ SIPCFG_PB                         , SIPCFG_PB, 0, 0 },
{ SIPCFG_ST                         , SIPCFG_ST, 0, 0 },
{ SIPCFG_NO_PARAM                   , 0, 0, 0 }
};

unsigned char param_hierarhy [256][HIERARHY_DEPTH];

void SPIRENT_SIPConfigurable_Init ( void )
{
   int i = 0;
   memset ( param_hierarhy, 0, sizeof(param_hierarhy) );
   while ( param_hierarhy_[i][0] )
   {
      param_hierarhy[param_hierarhy_[i][0]][0] = param_hierarhy_[i][1];
      param_hierarhy[param_hierarhy_[i][0]][1] = param_hierarhy_[i][2];
      param_hierarhy[param_hierarhy_[i][0]][2] = param_hierarhy_[i][3];
      i++;
   }
}

      
RVAPI RV_Status SPIRENT_CheckParam (IN  RvSipMsgHandle hSipMsg, unsigned char param, 
                                    ptl_cfg_elem* str_cfg, SPIRENT_Encoder_Cfg_Type *Enc_cfg )
{
   int res = 0;
   int cur_depth;
   MsgMessage*   msg = (MsgMessage*)hSipMsg;

   for ( cur_depth = 0; cur_depth < HIERARHY_DEPTH; cur_depth++ )
   {
      if ( param_hierarhy[param][cur_depth] )
      {
          switch (param_hierarhy[param][cur_depth] )
          {
          case SIPCFG_RES_CODE: // done intentionally
          case SIPCFG_RES_TEXT:
              {
                  res = RVSIP_MSG_RESPONSE != RvSipMsgGetMsgType(hSipMsg) ;
                  if (res) break;
              }
              break;
          case SIPCFG_REQ_URI:
              {
                  res = RVSIP_MSG_REQUEST != RvSipMsgGetMsgType(hSipMsg) ;
                  if (res) break;
                  res = ((MsgAddress*)(msg->startLine.requestLine.hRequestUri))->eAddrType != RVSIP_ADDRTYPE_URL
                     && ((MsgAddress*)(msg->startLine.requestLine.hRequestUri))->eAddrType != RVSIP_ADDRTYPE_TEL;
                  if (res) break;
              }
              break;
          case SIPCFG_REQ_URI_USER:
              {
                  if(RvSipAddrGetAddrType(((MsgPartyHeader*)msg->hToHeader)->hAddrSpec) == RVSIP_ADDRTYPE_URL) 
                  {
                     MsgAddress* adr = (MsgAddress*)(msg->startLine.requestLine.hRequestUri);
                     res = ((MsgAddrUrl*)(adr->uAddrBody.hUrl))->strUser <= UNDEFINED;
                  }else if(RvSipAddrGetAddrType(((MsgPartyHeader*)msg->hToHeader)->hAddrSpec) == RVSIP_ADDRTYPE_TEL)
                  {
                     MsgAddress* adr = (MsgAddress*)(msg->startLine.requestLine.hRequestUri);
                     res = adr->uAddrBody.pTelUri->strPhoneNumber <= UNDEFINED;
                  }
              }
              break;
          case SIPCFG_REQ_URI_HOST:
              {
                  if(RvSipAddrGetAddrType(((MsgPartyHeader*)msg->hToHeader)->hAddrSpec) == RVSIP_ADDRTYPE_URL)
                  {
                     MsgAddress* adr = (MsgAddress*)(msg->startLine.requestLine.hRequestUri);
                     res = ((MsgAddrUrl*)(adr->uAddrBody.hUrl))->strHost <= UNDEFINED;
                     if ( res )
                     {
                        /* this is not optional */
                        RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                                 "SPIRENT_CheckParam - Failed. SIPCFG_REQ_URI_HOST"));
                        return RV_InvalidParameter;
                     }
                  }                 
              }
              break;
          case SIPCFG_REQ_URI_TRANSPORT:
              {
                  MsgAddress* adr = (MsgAddress*)(msg->startLine.requestLine.hRequestUri);
                  res = ((MsgAddrUrl*)(adr->uAddrBody.hUrl))->eTransport == RVSIP_TRANSPORT_UNDEFINED;
              }
              break;
          case SIPCFG_FROM:
              {
                  res = msg->hFromHeader == NULL;
              }
              break;
          case SIPCFG_FROM_URI:
              {
                  MsgPartyHeader* pHeader = (MsgPartyHeader*)(msg->hFromHeader);
                  res = pHeader->hAddrSpec == NULL;
                  if ( res )
                  {
                      RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                                 "SPIRENT_CheckParam - Failed. SIPCFG_FROM_URI"));

                      return RV_InvalidParameter;
                  }
              }
              break;
          case SIPCFG_FROM_URI_USER:
              {
                  MsgAddress* adr = (MsgAddress*)(((MsgPartyHeader*)msg->hFromHeader)->hAddrSpec);
                  res = ((MsgAddrUrl*)(adr->uAddrBody.hUrl))->strUser <= UNDEFINED;
              }
              break;
          case SIPCFG_FROM_URI_HOST:
              {
                 if(RvSipAddrGetAddrType(((MsgPartyHeader*)msg->hToHeader)->hAddrSpec) == RVSIP_ADDRTYPE_URL)
                 {
                    MsgAddress* adr = (MsgAddress*)(((MsgPartyHeader*)msg->hFromHeader)->hAddrSpec);
                    res = ((MsgAddrUrl*)(adr->uAddrBody.hUrl))->strHost <= UNDEFINED;
                    if ( res )
                    {
                       /* this is not optional */
                       RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                                "SPIRENT_CheckParam - Failed. SIPCFG_REQ_URI_HOST"));
                       return RV_InvalidParameter;
                    }
                 }
              }
              break;
          case SIPCFG_FROM_URI_TRANSPORT:
              {
                  MsgAddress* adr = (MsgAddress*)(((MsgPartyHeader*)msg->hFromHeader)->hAddrSpec);
                  res = ((MsgAddrUrl*)(adr->uAddrBody.hUrl))->eTransport == RVSIP_TRANSPORT_UNDEFINED;
              }
              break;
          case SIPCFG_FROM_TAG:
              {
                  res = ((MsgPartyHeader*)msg->hFromHeader)->strTag <= UNDEFINED;
              }
              break;
          case SIPCFG_TO:
              {
                  res = msg->hToHeader == NULL;
              }
              break;
          case SIPCFG_TO_URI:
              {
                  MsgPartyHeader* pHeader = (MsgPartyHeader*)(msg->hToHeader);
                  res = pHeader->hAddrSpec == NULL;
                  if ( res )
                  {
                      RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                                "SPIRENT_CheckParam - Failed. SIPCFG_TO_URI"));

                      return RV_InvalidParameter;
                  }
              }
              break;
          case SIPCFG_TO_URI_USER:
              {
                  MsgAddress* adr = (MsgAddress*)(((MsgPartyHeader*)msg->hToHeader)->hAddrSpec);
                  res = ((MsgAddrUrl*)(adr->uAddrBody.hUrl))->strUser <= UNDEFINED;
              }
              break;
          case SIPCFG_TO_URI_HOST:
              {
                  if(RvSipAddrGetAddrType(((MsgPartyHeader*)msg->hToHeader)->hAddrSpec) == RVSIP_ADDRTYPE_URL)
                  {
                     MsgAddress* adr = (MsgAddress*)(((MsgPartyHeader*)msg->hToHeader)->hAddrSpec);
                     res = ((MsgAddrUrl*)(adr->uAddrBody.hUrl))->strHost <= UNDEFINED;
                     if ( res )
                     {
                        /* this is not optional */
                        RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                                 "SPIRENT_CheckParam - Failed. SIPCFG_REQ_URI_HOST"));
                        return RV_InvalidParameter;
                     }
                  }
              }
              break;
          case SIPCFG_TO_URI_TRANSPORT:
              {
                  MsgAddress* adr = (MsgAddress*)(((MsgPartyHeader*)msg->hToHeader)->hAddrSpec);
                  res = ((MsgAddrUrl*)(adr->uAddrBody.hUrl))->eTransport == RVSIP_TRANSPORT_UNDEFINED;
              }
              break;
          case SIPCFG_TO_TAG:
              {
                  res = ((MsgPartyHeader*)msg->hToHeader)->strTag <= UNDEFINED;
              }
              break;
          case SIPCFG_CALLID:
              {
                  res = (msg->hCallIdHeader == NULL);
                  if ( res ) break;
                  res = ((MsgOtherHeader*)msg->hCallIdHeader)->strHeaderVal <= UNDEFINED;
              }
              break;
          case SIPCFG_CSEQ:
              {
                  res = msg->hCSeqHeader == NULL;
              }
              break;
          case SIPCFG_CSEQ_STEP:
              {
                  res = 0;
              }
              break;
          case SIPCFG_CSEQ_METHOD:
              {
                  res = 0;
              }
              break;
          case SIPCFG_VIA1:
          case SIPCFG_VIA2:
          case SIPCFG_VIA3:
              {
                  res = (NULL == SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] ) );
              }
              break;
          case SIPCFG_VIA1_TRANSPORT:
          case SIPCFG_VIA2_TRANSPORT:
          case SIPCFG_VIA3_TRANSPORT:
              {
                  MsgViaHeader*   pHeader = (MsgViaHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res = (pHeader->eTransport == RVSIP_TRANSPORT_UNDEFINED);
                  if ( res )
                  {
                      RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                                "SPIRENT_CheckParam - Failed. eTransport is UNDEFINED. not an optional param!!!" ));

                      return RV_InvalidParameter;
                  }
                  
              }
              break;
          case SIPCFG_VIA1_IP:
          case SIPCFG_VIA2_IP:
          case SIPCFG_VIA3_IP:
              {
                  MsgViaHeader*   pHeader = (MsgViaHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res =  (pHeader->strHost == UNDEFINED);
                  if ( res )
                  {
                      RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                                "RvSipViaHeaderEncode - Failed. strHost is NULL. not an optional param!!! hHeader %x", pHeader ));

                      return RV_InvalidParameter;
                  }
              }
              break;
          case SIPCFG_VIA1_PORT:
          case SIPCFG_VIA2_PORT:
          case SIPCFG_VIA3_PORT:
              {
                  MsgViaHeader*   pHeader = (MsgViaHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res =  (pHeader->portNum == UNDEFINED);
              }
              break;
          case SIPCFG_VIA1_BRANCH:
          case SIPCFG_VIA2_BRANCH:
          case SIPCFG_VIA3_BRANCH:
              {
                  MsgViaHeader*   pHeader = (MsgViaHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res = (pHeader->strBranchParam  == UNDEFINED);
              }
              break;
          case SIPCFG_CONTACT1:
              {
                  res = (NULL == SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] ) );
              }
              break;
          case SIPCFG_CONTACT1_URI:
              {
                  MsgContactHeader* pHeader = (MsgContactHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res = pHeader->hAddrSpec == NULL;
                  if ( res )
                  {
                      RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                                "SPIRENT_CheckParam - Failed. SIPCFG_CONTACT1_URI"));

                      return RV_InvalidParameter;
                  }
              }
              break;
          case SIPCFG_CONTACT1_URI_USER:
              {
                  MsgContactHeader* pHeader = (MsgContactHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  MsgAddress* adr = (MsgAddress*)(pHeader->hAddrSpec);
                  res = ((MsgAddrUrl*)(adr->uAddrBody.hUrl))->strUser <= UNDEFINED;
              }
              break;
          case SIPCFG_CONTACT1_URI_HOST:
              {
                  MsgContactHeader* pHeader = (MsgContactHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  MsgAddress* adr = (MsgAddress*)(pHeader->hAddrSpec);
                  res = ((MsgAddrUrl*)(adr->uAddrBody.hUrl))->strHost <= UNDEFINED;
                  if ( res )
                  {
                      /* this is not optional */
                      RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                                "SPIRENT_CheckParam - Failed. SIPCFG_CONTACT1_URI_HOST"));

                      return RV_InvalidParameter;
                  }
              }
              break;
          case SIPCFG_CONTACT1_URI_TRANSPORT:
              {
                  MsgContactHeader* pHeader = (MsgContactHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  MsgAddress* adr = (MsgAddress*)(pHeader->hAddrSpec);
                  res = ((MsgAddrUrl*)(adr->uAddrBody.hUrl))->eTransport == RVSIP_TRANSPORT_UNDEFINED;
              }
              break;
          case SIPCFG_CONTACT1_EXPIRES:
              {
                  MsgContactHeader* pHeader = (MsgContactHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res = ( NULL == pHeader->hExpiresParam );
              }
              break;
          case SIPCFG_CONTACT1_ISFOCUS:
              {
                  MsgContactHeader* pHeader = (MsgContactHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res = (pHeader->bIsIsFocus ? 0 : 1);
              }
              break;
          case SIPCFG_AUTHORIZATION1:
              {
                  res = (NULL == SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] ) );
              }
              break;
          case SIPCFG_AUTHORIZATION1_SCHEME:
              {
                  res = 0; // allways there even if empty
              }
              break;
          case SIPCFG_AUTHORIZATION1_USERNAME:
              {
                  res = 0; // allways there even if empty
              }
              break;
          case SIPCFG_AUTHORIZATION1_REALM:
              {
                  res = 0; // allways there even if empty
              }
              break;
          case SIPCFG_AUTHORIZATION1_NONCE:
              {
                  res = 0; // allways there even if empty
              }
              break;
          case SIPCFG_AUTHORIZATION1_URI:
              {
                  res = 0; // allways there even if empty
              }
              break;
          case SIPCFG_AUTHORIZATION1_RESPONSE:
              {
                  res = 0;
              }
              break;
          case SIPCFG_AUTHORIZATION1_ALGORITHM:
              {
                  MsgAuthorizationHeader*  pHeader = (MsgAuthorizationHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );;
                  res = (pHeader->eAlgorithm == RVSIP_AUTH_ALGORITHM_UNDEFINED);
              }
              break;
          case SIPCFG_AUTHORIZATION1_CNONCE:
              {
                  MsgAuthorizationHeader*  pHeader = (MsgAuthorizationHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );;
                  res = (pHeader->strCnonce <= UNDEFINED);
              }
              break;
          case SIPCFG_AUTHORIZATION1_OPAQUE:
              {
                  MsgAuthorizationHeader*  pHeader = (MsgAuthorizationHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );;
                  res = (pHeader->strOpaque <= UNDEFINED);
              }
              break;
          case SIPCFG_AUTHORIZATION1_QOP:
              {
                  MsgAuthorizationHeader*  pHeader = (MsgAuthorizationHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );;
                  res = !(pHeader->eAuthQop != RVSIP_AUTH_QOP_OTHER && pHeader->eAuthQop != RVSIP_AUTH_QOP_UNDEFINED);
              }
              break;
          case SIPCFG_AUTHORIZATION1_NC:
              {
                  MsgAuthorizationHeader*  pHeader = (MsgAuthorizationHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );;
                  res = (pHeader->nonceCount == UNDEFINED);
              }
              break;
          case SIPCFG_AUTHORIZATION1_AUTHPARAM:
              {
                  MsgAuthorizationHeader*  pHeader = (MsgAuthorizationHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );;
                  res = (pHeader->strAuthParam <= UNDEFINED);
              }
              break;
          case SIPCFG_CONTENTTYPE:
              {
                  res = ((NULL == msg->pBodyObject) || (NULL == msg->pBodyObject->pContentType));
              }
              break;
          case SIPCFG_CONTENTTYPE_MEDIATYPE:
              {
                  res = 0; // allways there
              }
              break;
          case SIPCFG_CONTENTTYPE_MEDIASUBTYPE:
              {
                  res = 0; // allways there
              }
              break;
          case SIPCFG_CONTENTTYPE_BOUNDARY:
              {
                  MsgContentTypeHeader* pHeader = (MsgContentTypeHeader*)((RvSipContentTypeHeaderHandle)msg->pBodyObject->pContentType);
                  res = (pHeader->strBoundary <= UNDEFINED);
              }
              break;
          case SIPCFG_CONTENTTYPE_VERSION:
              {
                  MsgContentTypeHeader* pHeader = (MsgContentTypeHeader*)((RvSipContentTypeHeaderHandle)msg->pBodyObject->pContentType);
                  res = (pHeader->strVersion <= UNDEFINED);
              }
              break;
          case SIPCFG_CONTENTTYPE_BASE:
              {
                  MsgContentTypeHeader* pHeader = (MsgContentTypeHeader*)((RvSipContentTypeHeaderHandle)msg->pBodyObject->pContentType);
                  res = (pHeader->strBase <= UNDEFINED);
              }
              break;
          case SIPCFG_RSEQ:
              {
                  MsgRSeqHeader* pHeader = SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res = (NULL == pHeader);
                  if ( !res  )
                     res = ( (int)pHeader->respNum.val <= UNDEFINED );
              }
              break;
          case SIPCFG_RACK:
              {
                  res = (NULL == SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] ) );
              }
              break;
          case SIPCFG_RACK_SEQ:
              {
                  res = 0; // allways there
              }
              break;
          case SIPCFG_RACK_CSEQ:
              {
                  res = 0; // allways there
              }
              break;
          case SIPCFG_RACK_CSEQ_METHOD:
              {
                  res = 0; // allways there
              }
              break;
          case SIPCFG_CONTENTLENGTH:
              {
                  res = 0; // allways there
              }
              break;
          case SIPCFG_SDPBODY:
              {
                  res = 0; // allways there even if empty
              }
              break;
          case SIPCFG_ROUTE1:
          case SIPCFG_ROUTE2:
          case SIPCFG_ROUTE3:
      	  case SIPCFG_ROUTE4:
          case SIPCFG_ROUTE5:
      	  case SIPCFG_ROUTE6:
      	  case SIPCFG_ROUTE7:
      	  case SIPCFG_ROUTE8:
              {
                  MsgRouteHopHeader* pHeader = (MsgRouteHopHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res = (NULL == pHeader);
                  if ( !res  )
                     res = ( pHeader->hAddrSpec == NULL );
              }
              break;
          case SIPCFG_EVENT:
              {
                  MsgEventHeader* pHeader = (MsgEventHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res = (NULL == pHeader);
                  if ( !res  )
                     res = ( pHeader->strEventPackage <= UNDEFINED );
              }
              break;
          case SIPCFG_EXPIRES:
              {
                  MsgExpiresHeader* pHeader = (MsgExpiresHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res = (NULL == pHeader);
                  if ( !res  )
                     res = ( pHeader->pInterval == NULL );
              }
              break;
          case SIPCFG_SUBSCRIPTIONSTATE:
              {
                  MsgSubscriptionStateHeader* pHeader = (MsgSubscriptionStateHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res = (NULL == pHeader);
                  if ( !res  )
                     res = ( pHeader->eSubstate < 0 );
              }
              break;
          case SIPCFG_SUBSCRIPTION_EXPIRES:
              {
                  MsgSubscriptionStateHeader* pHeader = (MsgSubscriptionStateHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res = ( pHeader->expiresVal <= UNDEFINED );
              }
              break;
          case SIPCFG_PROXYREQUIRE:
              {
                  RvSipOtherHeaderHandle pHeader = (RvSipOtherHeaderHandle)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res = (NULL == pHeader);
                  if ( res ) break;
                  res = ((MsgOtherHeader*)pHeader)->strHeaderVal <= UNDEFINED;
              }
              break;
          case SIPCFG_REFER_TO:
              {
                  MsgReferToHeader* pHeader = (MsgReferToHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res = (NULL == pHeader);
                  if (!res)
                      res = ( NULL == pHeader->hAddrSpec );
              }
              break;
          case SIPCFG_REFERRED_BY:
              {
                  MsgReferredByHeader* pHeader = (MsgReferredByHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res = (NULL == pHeader);
                  if (!res)
                      res = ( NULL == pHeader->hReferrerAddrSpec );
              }
              break;
          case SIPCFG_REPLACES_CALLID:
              {
                  MsgReplacesHeader* pHeader = (MsgReplacesHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res = ( NULL == pHeader );
                  if (res)
                  {
                      // if Replaces header is not found, try it inside Refer-To header
                      MsgReferToHeader* pReferToHeader = (MsgReferToHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[SIPCFG_REFER_TO][cur_depth] );
                      res = (NULL == pReferToHeader);
                      if (!res)
                      {
                          pHeader = (MsgReplacesHeader*)pReferToHeader->hReplacesHeader;
                          res = ( NULL == pHeader );
                      }
                  }
                  if (!res)
                  {
                      if ( pHeader->strCallID <= UNDEFINED )
                      {
                          RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                                    "SPIRENT_CheckParam - Failed. SIPCFG_REPLACES_CALLID"));

                          return RV_InvalidParameter;
                      }
                  }
              }
              break;
          case SIPCFG_REPLACES_FROM_TAG:
              {
                  MsgReplacesHeader* pHeader = (MsgReplacesHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res = ( NULL == pHeader );
                  if (res)
                  {
                      // if Replaces header is not found, try it inside Refer-To header
                      MsgReferToHeader* pReferToHeader = (MsgReferToHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[SIPCFG_REFER_TO][cur_depth] );
                      res = (NULL == pReferToHeader);
                      if (!res)
                      {
                          pHeader = (MsgReplacesHeader*)pReferToHeader->hReplacesHeader;
                          res = ( NULL == pHeader );
                      }
                  }
                  if (!res)
                  {
                      if ( pHeader->strFromTag <= UNDEFINED )
                      {
                          RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                                    "SPIRENT_CheckParam - Failed. SIPCFG_REPLACES_FROM_TAG"));

                          return RV_InvalidParameter;
                      }
                  }
              }
              break;
          case SIPCFG_REPLACES_TO_TAG:
              {
                  MsgReplacesHeader* pHeader = (MsgReplacesHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res = ( NULL == pHeader );
                  if (res)
                  {
                      // if Replaces header is not found, try it inside Refer-To header
                      MsgReferToHeader* pReferToHeader = (MsgReferToHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[SIPCFG_REFER_TO][cur_depth] );
                      res = (NULL == pReferToHeader);
                      if (!res)
                      {
                          pHeader = (MsgReplacesHeader*)pReferToHeader->hReplacesHeader;
                          res = ( NULL == pHeader );
                      }
                  }
                  if (!res)
                  {
                      if ( pHeader->strToTag <= UNDEFINED )
                      {
                          RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                                    "SPIRENT_CheckParam - Failed. SIPCFG_REPLACES_TO_TAG"));

                          return RV_InvalidParameter;
                      }
                  }
              }
              break;
          case SIPCFG_ALLOW:
          case SIPCFG_SUPPORTED_GRUU:
          case SIPCFG_SUPPORTED_JOIN:
          case SIPCFG_SUPPORTED_REPLACES:
          case SIPCFG_SUPPORTED_100REL:
              {
                  res = ( NULL == SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] ) );
              }
              break;
          case SIPCFG_JOIN_CALLID:
              {
                  RvSipOtherHeaderHandle hHeader = (RvSipOtherHeaderHandle)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res = (NULL == hHeader);
                  if (!res)
                  {
                      uint actualLen = 0;
                      char buf[1000];
                      res = ( RvSipOtherHeaderGetValue ( hHeader, buf, sizeof(buf), &actualLen ) < 0 );
                      if ( actualLen <= 1 ) res = 1;
                      if (!res)
                      {
                          char parmBuf[257];
                          if (0 == SPIRENT_RvSIP_GetParameterValueFromString ( msg, buf, DELIM, NULL,
                                                                               parmBuf, sizeof(parmBuf) ) )
                              res = 1; // empty Call-ID
                      }
                      if ( res )
                      {
                          RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                                    "SPIRENT_CheckParam - Failed. SIPCFG_JOIN_CALLID"));

                          return RV_InvalidParameter;
                      }
                  }
              }
              break;
          case SIPCFG_JOIN_FROM_TAG:
              {
                  RvSipOtherHeaderHandle hHeader = (RvSipOtherHeaderHandle)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res = (NULL == hHeader);
                  if (!res)
                  {
                      uint actualLen = 0;
                      char buf[1000];
                      res = ( RvSipOtherHeaderGetValue ( hHeader, buf, sizeof(buf), &actualLen ) < 0 );
                      if ( actualLen <= 1 ) res = 1;
                      if (!res)
                      {
                          char parmBuf[257];
                          if (0 == SPIRENT_RvSIP_GetParameterValueFromString ( msg, buf, DELIM, "from-tag",
                                                                               parmBuf, sizeof(parmBuf) ) )
                              res = 1; // no from-tag
                      }
                      if ( res )
                      {
                          RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                                    "SPIRENT_CheckParam - Failed. SIPCFG_JOIN_FROM_TAG"));

                          return RV_InvalidParameter;
                      }
                  }
              }
              break;
          case SIPCFG_JOIN_TO_TAG:
              {
                  RvSipOtherHeaderHandle hHeader = (RvSipOtherHeaderHandle)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res = (NULL == hHeader);
                  if (!res)
                  {
                      uint actualLen = 0;
                      char buf[1000];
                      res = ( RvSipOtherHeaderGetValue ( hHeader, buf, sizeof(buf), &actualLen ) < 0 );
                      if ( actualLen <= 1 ) res = 1;
                      if (!res)
                      {
                          char parmBuf[257];
                          if (0 == SPIRENT_RvSIP_GetParameterValueFromString ( msg, buf, DELIM, "to-tag",
                                                                               parmBuf, sizeof(parmBuf) ) )
                              res = 1; // no to-tag
                      }
                      if ( res )
                      {
                          RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                                    "SPIRENT_CheckParam - Failed. SIPCFG_JOIN_TO_TAG"));

                          return RV_InvalidParameter;
                      }
                  }
              }
              break;
          case SIPCFG_ACCEPT1:
          case SIPCFG_ACCEPT2:
          case SIPCFG_ACCEPT3:
              {
                  res = ( NULL == SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] ) );
              }
              break;
          case SIPCFG_ACCEPT_MEDIATYPE1:
          case SIPCFG_ACCEPT_MEDIATYPE2:
          case SIPCFG_ACCEPT_MEDIATYPE3:
              {
                  RvSipOtherHeaderHandle hHeader = (RvSipOtherHeaderHandle)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res = (NULL == hHeader);
                  if (!res)
                  {
                      uint actualLen = 0;
                      char buf[1000];
                      res = ( RvSipOtherHeaderGetValue ( hHeader, buf, sizeof(buf), &actualLen ) < 0 );
                      if ( actualLen <= 1 ) res = 1;
                      if (!res)
                      {
                          char * pSlash = strchr (buf, '/');
                          if (!pSlash || pSlash == buf)
                              res = 1;  // no MediaType
                      }
                      if ( res )
                      {
                          RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                                    "SPIRENT_CheckParam - Failed. SIPCFG_ACCEPT_MEDIATYPE"));

                          return RV_InvalidParameter;
                      }
                  }
              }
              break;
          case SIPCFG_ACCEPT_MEDIASUBTYPE1:
          case SIPCFG_ACCEPT_MEDIASUBTYPE2:
          case SIPCFG_ACCEPT_MEDIASUBTYPE3:
              {
                  RvSipOtherHeaderHandle hHeader = (RvSipOtherHeaderHandle)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] );
                  res = (NULL == hHeader);
                  if (!res)
                  {
                      uint actualLen = 0;
                      char buf[1000];
                      res = ( RvSipOtherHeaderGetValue ( hHeader, buf, sizeof(buf), &actualLen ) < 0 );
                      if ( actualLen <= 1 ) res = 1;
                      if (!res)
                      {
                          char * pSlash = strchr (buf, '/');
                          if (!pSlash || *(pSlash+1) == '\0')
                              res = 1;  // no MediaSubtype
                          else if (strchr (pSlash+1, '/'))
                              res = 1;  // second slash
                      }
                      if ( res )
                      {
                          RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                                    "SPIRENT_CheckParam - Failed. SIPCFG_ACCEPT_MEDIASUBTYPE"));

                          return RV_InvalidParameter;
                      }
                  }
              }
              break;
          case SIPCFG_ACCEPT_ENCODING1:
          case SIPCFG_ACCEPT_ENCODING2:
          case SIPCFG_ACCEPT_ENCODING3:
          case SIPCFG_ACCEPT_LANGUAGE1:
          case SIPCFG_ACCEPT_LANGUAGE2:
          case SIPCFG_ACCEPT_LANGUAGE3:
              {
                  res = ( NULL == SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, param_hierarhy[param][cur_depth] ) );
              }
              break;
          case    SIPCFG_PB:
          case    SIPCFG_ST:
              {
                  if ( Enc_cfg->check_fptr )
                  {
                        res = Enc_cfg->check_fptr ( param_hierarhy[param][cur_depth], (char*)str_cfg->param_name, Enc_cfg->app_param );
                  }
                  else
                        res = 1;
              }
              break;
         } 
      } 
      else
      {
         // check completed
         break;
      }
      if ( res ) break;
   }
   return res;
}

/***************************************************************************
 * SPIRENT_RvSipMsgEncode
 * ------------------------------------------------------------------------
 * General: Encodes a message object to a textual SIP message according to SPIRENT 
 *          configuration. 
 *          The textual SIP message is placed on a page taken from a given 
 *          memory pool. In order to copy the message from the page to a 
 *          consecutive buffer, use RPOOL_CopyToExternal().
 *          The application must free the allocated page, using RPOOL_FreePage(). 
 *          The allocated page must be freed only if this function 
 *          returns RV_Success.
 * Return Value: RV_Success, RV_Failure, RV_InvalidParameter, RV_OutOfResources.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hSipMsg - Handle to the message to encode.
 *          hPool   - Handle to a pool.
 * Output: 	hPage   - The memory page allocated to hold the encoded message.
 *          length  - The length of the encoded message.
 ***************************************************************************/


RVAPI RV_Status RVCALLCONV SPIRENT_RvSipMsgEncode(IN  RvSipMsgHandle hSipMsg,
                                          IN  HRPOOL         hPool,
                                          OUT HPAGE*         hPage,
                                          OUT RV_UINT32*     length)
{
   RV_Status     stat;
   MsgMessage*   msg = NULL;
   RV_CHAR       tempString[16];
   int           Check_status;
   int           Current_Param;
   SPIRENT_Encoder_Cfg_Type Enc_cfg;
   int           string_num = 0;
   int         ContentLength_value;
   HPAGE       Body_hPage = NULL;
   ptl_cfg_elem  *str_cfg = NULL;
   RvBool  isMessageBodyRequired = RV_FALSE;
   RV_BOOL      pre_exist = RV_TRUE;

   /* This set of parameters is used to add, automatically,
   the "boundary=..." clause for multipart MIME messages. */
   int last_param   = 0;
   int last_mediatype = 0;
   int last_mediasubtype = 0;

    if(hPage == NULL || length == NULL)
		return RV_BadPointer;

    RvSipMsgGetExternalEncoder ( hSipMsg, &Enc_cfg );
//    sip_cfg_buf = Enc_cfg.cfg;
      if ( !Enc_cfg.cfg ) 
        return RV_BadPointer;

	 *hPage = NULL_PAGE;
	 *length = 0;

    if((hSipMsg == NULL) ||(hPool == NULL))
        return RV_InvalidHandle;

    stat = RPOOL_GetPage(hPool, 0, hPage);
    
    msg = (MsgMessage*)hSipMsg;

    if(stat != RV_Success)
    {
        RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                   "SPIRENT_RvSipMsgEncode - Failed to encode message. RPOOL_GetPage failed, hPool is %x, hSipMsg is %x",
                   hPool, hSipMsg));

        return stat;
    }
    else
    {
        RvLogDebug(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                "SPIRENT_RvSipMsgEncode - Got new page %d on pool %x. hSipMsg is %x",
                *hPage, hPool, hSipMsg));
    }

    *length = 0;

    stat = sortHeaderList(msg);
    if(stat != RV_Success)
    {
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                "SPIRENT_RvSipMsgEncode - hMsg %x. sortHeaderList failed",
                hSipMsg));

        return stat;
    }

    // Check to see whether users want to include a sip-message-body:
    string_num = 0;
    while(1)
    {
       str_cfg = &(((ptl_cfg_elem*)Enc_cfg.cfg)[string_num]);
       if ( str_cfg->param == SIPCFG_END_OF_LIST ) break;
       if ( str_cfg->param == SIPCFG_SDPBODY)
       {
          isMessageBodyRequired = RV_TRUE;
          break;
       }
       string_num++;
    }

    if(isMessageBodyRequired == RV_TRUE)
    {
       // users want to include a message-body to this message.
       // let's calcuate the content-length based on the provided message-body.
       ContentLength_value = SPIRENT_RvSIP_CalculateContentLength ( msg, hPool, &Body_hPage );
       if ( ContentLength_value < 0 )
       {
           RPOOL_FreePage(hPool, *hPage);
           RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                   "SPIRENT_RvSipMsgEncode - hMsg %x. ContentLength calculation failed",
                   hSipMsg));

           return ContentLength_value;
       }

       if (NULL != msg->pBodyObject)
       {
           stat = BodyUpdateContentType((RvSipBodyHandle)msg->pBodyObject);
           if (RV_Success != stat)
           {
               RPOOL_FreePage(hPool, *hPage);
               RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                         "SPIRENT_RvSipMsgEncode - msg %x. BodyUpdateContentType failed. ",
                         msg));

               return stat;
           }
       }
    }
    else
    {
       // users don't want to include a message-body. --> Set the content-length header field to zero.
       ContentLength_value = 0;
    }

    SPIRENT_RvSIP_HeaderListGetCacheInit();
    string_num = 0;
    while (1)
    {
       RV_INT32         offset;
       RvStatus         stat=RV_OK;
       RPOOL_ITEM_HANDLE encoded;
       int              isExternalParam = 0;
       unsigned char*   text_str;
       unsigned char*   name_str;

       str_cfg = &(((ptl_cfg_elem*)Enc_cfg.cfg)[string_num]);
       string_num++;

       if ( str_cfg->param == SIPCFG_END_OF_LIST ) break;

       if ( str_cfg->param > SIPCFG_PARAM_EXIST ) 
       {
           Check_status = SPIRENT_CheckParam ( hSipMsg, str_cfg->param - SIPCFG_PARAM_EXIST, str_cfg, &Enc_cfg );          
           Current_Param = SIPCFG_NO_PARAM;
       }
       else
       {
           Check_status = SPIRENT_CheckParam ( hSipMsg, str_cfg->param, str_cfg, &Enc_cfg );          
           Current_Param = str_cfg->param;
       }

       if ( Check_status > 0 )
       {
          // parameter is not present and optional
          /* SPC: To check whether there is any symbol left in the missed
             encoding line.
             pre_exist is used to check whether the previous encoding line is
             succeed to be added to the message. */
          if(str_cfg->len && pre_exist)
          {
             int i;
             int rnlength = 0;
             char *str = NULL;

             for(i = str_cfg->len - 1; i > 0; i--)
                if(str_cfg->str[i - 1] == '\r' && str_cfg->str[i] == '\n')
                   rnlength = i - 1;

             if(rnlength > 0)
             {
                   stat = RPOOL_AppendFromExternal(hPool,
                         *hPage,
                         (void*)str_cfg->str,
                         rnlength,
                         RV_FALSE,
                         &offset,
                         &encoded);

                   if(stat != RV_Success)
                      RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                               "MsgUtilsEncodeString - Failed, AppendFromExternal failed. hPool %x, hPage %d, RV_Status: %d",
                               hPool, hPage, stat));
             }
          }

          pre_exist = RV_FALSE;

          continue;
       }
       else if ( Check_status < 0 )
       {
          // parameter is not present and required
          pre_exist = RV_FALSE;

          return Check_status;
       }

       pre_exist = RV_TRUE;

       // Encode string first
       if ( str_cfg->len )
       {
          *length += str_cfg->len;
          
          stat = RPOOL_AppendFromExternal(hPool,
             *hPage,
             (void*)str_cfg->str,
             str_cfg->len,
             RV_FALSE,
             &offset,
             &encoded);
          
          if(stat != RV_Success)
          {
              RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                "MsgUtilsEncodeString - Failed, AppendFromExternal failed. hPool %x, hPage %d, RV_Status: %d",
                hPool, hPage, stat));

          }
       }
       // Now we should encode parameter

       switch ( Current_Param )
       {
       case SIPCFG_RES_CODE:
           {
               RV_CHAR     code[5];
               MsgUtils_itoa(msg->startLine.statusLine.statusCode, code);
               stat = MsgUtilsEncodeExternalString(msg->pMsgMgr, hPool, *hPage, code, length);
           }
           break;
       case SIPCFG_RES_TEXT:
           {
               if(msg->startLine.statusLine.strReasonePhrase > UNDEFINED)
                   stat = MsgUtilsEncodeString(msg->pMsgMgr, hPool, *hPage, msg->hPool, msg->hPage, msg->startLine.statusLine.strReasonePhrase, length);
           }
           break;
       case SIPCFG_REQ_URI:
           {
               stat = AddrEncode ( msg->startLine.requestLine.hRequestUri, hPool, *hPage, RV_FALSE, RV_FALSE, length );
           }
           break;
       case SIPCFG_REQ_URI_USER:
           {
               if(RvSipAddrGetAddrType(((MsgPartyHeader*)msg->hToHeader)->hAddrSpec) == RVSIP_ADDRTYPE_URL)
               {
                  MsgAddress* pAddr = (MsgAddress*)(msg->startLine.requestLine.hRequestUri);
                  MsgAddrUrl* pUrl  = (MsgAddrUrl*)(pAddr->uAddrBody.hUrl);
                  stat = MsgUtilsEncodeString( msg->pMsgMgr, hPool, *hPage, pAddr->hPool, pAddr->hPage, pUrl->strUser, length);
               }else if(RvSipAddrGetAddrType(((MsgPartyHeader*)msg->hToHeader)->hAddrSpec) == RVSIP_ADDRTYPE_TEL)
               {
                  MsgAddress* pAddr = (MsgAddress*)(msg->startLine.requestLine.hRequestUri);
                  stat = MsgUtilsEncodeString( msg->pMsgMgr, hPool, *hPage, pAddr->hPool, pAddr->hPage, pAddr->uAddrBody.pTelUri->strPhoneNumber, length);
               }
           }
           break;
       case SIPCFG_REQ_URI_HOST:
           {
               if(RvSipAddrGetAddrType(((MsgPartyHeader*)msg->hToHeader)->hAddrSpec) == RVSIP_ADDRTYPE_URL)
               {
                  MsgAddress* pAddr = (MsgAddress*)(msg->startLine.requestLine.hRequestUri);
                  MsgAddrUrl* pUrl  = (MsgAddrUrl*)(pAddr->uAddrBody.hUrl);
                  stat = MsgUtilsEncodeString( msg->pMsgMgr, hPool, *hPage, pAddr->hPool, pAddr->hPage, pUrl->strHost, length);
               }
           }
           break;
       case SIPCFG_REQ_URI_TRANSPORT:
           {
               MsgAddress* pAddr = (MsgAddress*)(msg->startLine.requestLine.hRequestUri);
               MsgAddrUrl* pUrl  = (MsgAddrUrl*)(pAddr->uAddrBody.hUrl);
               stat = MsgUtilsEncodeTransportType( msg->pMsgMgr, hPool, *hPage, pUrl->eTransport, pAddr->hPool, pAddr->hPage, pUrl->strTransport, length);
           }
           break;
       case SIPCFG_FROM_URI:
           {
               MsgPartyHeader* pHeader = (MsgPartyHeader*) msg->hFromHeader;
               stat = AddrEncode(pHeader->hAddrSpec, hPool, *hPage, RV_FALSE, RV_FALSE, length);
           }
           break;
       case SIPCFG_FROM_URI_USER:
           {
               MsgAddress* pAddr = (MsgAddress*)(((MsgPartyHeader*)msg->hFromHeader)->hAddrSpec);
               MsgAddrUrl* pUrl  = (MsgAddrUrl*)(pAddr->uAddrBody.hUrl);
               stat = MsgUtilsEncodeString( msg->pMsgMgr, hPool, *hPage, pAddr->hPool, pAddr->hPage, pUrl->strUser, length );
           }
           break;
       case SIPCFG_FROM_URI_HOST:
           {
              if(RvSipAddrGetAddrType(((MsgPartyHeader*)msg->hToHeader)->hAddrSpec) == RVSIP_ADDRTYPE_URL)
              {
                 MsgAddress* pAddr = (MsgAddress*)(((MsgPartyHeader*)msg->hFromHeader)->hAddrSpec);
                 MsgAddrUrl* pUrl  = (MsgAddrUrl*)(pAddr->uAddrBody.hUrl);
                 stat = MsgUtilsEncodeString( msg->pMsgMgr, hPool, *hPage, pAddr->hPool, pAddr->hPage, pUrl->strHost, length);
              }
           }
           break;
       case SIPCFG_FROM_URI_TRANSPORT:
           {
               MsgAddress* pAddr = (MsgAddress*)(((MsgPartyHeader*)msg->hFromHeader)->hAddrSpec);
               MsgAddrUrl* pUrl  = (MsgAddrUrl*)(pAddr->uAddrBody.hUrl);
               stat = MsgUtilsEncodeTransportType( msg->pMsgMgr, hPool, *hPage, pUrl->eTransport, pAddr->hPool, pAddr->hPage, pUrl->strTransport, length);
           }
           break;
       case SIPCFG_FROM_TAG:
           {
               MsgPartyHeader* pHeader = (MsgPartyHeader*)msg->hFromHeader;
               stat = MsgUtilsEncodeString( msg->pMsgMgr, hPool, *hPage, pHeader->hPool, pHeader->hPage, pHeader->strTag, length );
           }
           break;
       case SIPCFG_TO_URI:
           {
               MsgPartyHeader* pHeader = (MsgPartyHeader*) msg->hToHeader;
               stat = AddrEncode(pHeader->hAddrSpec, hPool, *hPage, RV_FALSE, RV_FALSE, length);
           }
           break;
       case SIPCFG_TO_URI_USER:
           {
               MsgAddress* pAddr = (MsgAddress*)(((MsgPartyHeader*)msg->hToHeader)->hAddrSpec);
               MsgAddrUrl* pUrl  = (MsgAddrUrl*)(pAddr->uAddrBody.hUrl);
               stat = MsgUtilsEncodeString( msg->pMsgMgr, hPool, *hPage, pAddr->hPool, pAddr->hPage, pUrl->strUser, length );
           }
           break;
       case SIPCFG_TO_URI_HOST:
           {
               if(RvSipAddrGetAddrType(((MsgPartyHeader*)msg->hToHeader)->hAddrSpec) == RVSIP_ADDRTYPE_URL)
               {
                  MsgAddress* pAddr = (MsgAddress*)(((MsgPartyHeader*)msg->hToHeader)->hAddrSpec);
                  MsgAddrUrl* pUrl  = (MsgAddrUrl*)(pAddr->uAddrBody.hUrl);
                  stat = MsgUtilsEncodeString( msg->pMsgMgr, hPool, *hPage, pAddr->hPool, pAddr->hPage, pUrl->strHost, length);
               }
           }
           break;
       case SIPCFG_TO_URI_TRANSPORT:
           {
               MsgAddress* pAddr = (MsgAddress*)(((MsgPartyHeader*)msg->hToHeader)->hAddrSpec);
               MsgAddrUrl* pUrl  = (MsgAddrUrl*)(pAddr->uAddrBody.hUrl);
               stat = MsgUtilsEncodeTransportType( msg->pMsgMgr, hPool, *hPage, pUrl->eTransport, pAddr->hPool, pAddr->hPage, pUrl->strTransport, length);
           }
           break;
       case SIPCFG_TO_TAG:
           {
               MsgPartyHeader* pHeader = (MsgPartyHeader*)msg->hToHeader;
               stat = MsgUtilsEncodeString( msg->pMsgMgr, hPool, *hPage, pHeader->hPool, pHeader->hPage, pHeader->strTag, length );
           }
           break;
       case SIPCFG_NO_PARAM:
           stat = RV_Success;
           // nothing to add
           break;
       case SIPCFG_CALLID:
           {
               MsgOtherHeader* pHeader;
               pHeader = (MsgOtherHeader*)msg->hCallIdHeader;
               stat = MsgUtilsEncodeString(pHeader->pMsgMgr, hPool, *hPage, pHeader->hPool, pHeader->hPage, pHeader->strHeaderVal, length );
           }
           break;
       case SIPCFG_CSEQ_STEP:
           {
               RV_CHAR         strHelper[16];
               MsgCSeqHeader* pHeader = (MsgCSeqHeader*)msg->hCSeqHeader;
               
               MsgUtils_itoa(pHeader->step.val, strHelper);
               stat = MsgUtilsEncodeExternalString ( pHeader->pMsgMgr, hPool, *hPage, strHelper, length );
           }
           break;
       case SIPCFG_CSEQ_METHOD:
           {
               MsgCSeqHeader* pHeader = (MsgCSeqHeader*)msg->hCSeqHeader;
               stat = MsgUtilsEncodeMethodType ( pHeader->pMsgMgr, hPool, *hPage, pHeader->eMethod, pHeader->hPool, pHeader->hPage, pHeader->strOtherMethod, length );
           }
           break;
       case SIPCFG_VIA1_TRANSPORT:
       case SIPCFG_VIA2_TRANSPORT:
       case SIPCFG_VIA3_TRANSPORT:
           {
               MsgViaHeader* pHeader = (MsgViaHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               stat = MsgUtilsEncodeTransportType ( pHeader->pMsgMgr, hPool, *hPage, pHeader->eTransport, pHeader->hPool, pHeader->hPage, pHeader->strTransport, length );
           }
           break;
       case SIPCFG_VIA1_IP:
       case SIPCFG_VIA2_IP:
       case SIPCFG_VIA3_IP:
           {
               MsgViaHeader*   pHeader = (MsgViaHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               stat = MsgUtilsEncodeString ( pHeader->pMsgMgr, hPool, *hPage, pHeader->hPool, pHeader->hPage, pHeader->strHost, length );
           }
           break;
       case SIPCFG_VIA1_PORT:
       case SIPCFG_VIA2_PORT:
       case SIPCFG_VIA3_PORT:
           {
               RV_CHAR         strHelper[16];
               MsgViaHeader*   pHeader = (MsgViaHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               MsgUtils_itoa(pHeader->portNum, strHelper);
               stat = MsgUtilsEncodeExternalString ( pHeader->pMsgMgr, hPool, *hPage, strHelper, length );
           }
           break;
       case SIPCFG_VIA1_BRANCH:
       case SIPCFG_VIA2_BRANCH:
       case SIPCFG_VIA3_BRANCH:
           {
               MsgViaHeader*   pHeader = (MsgViaHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               stat = MsgUtilsEncodeString ( pHeader->pMsgMgr, hPool, *hPage, pHeader->hPool, pHeader->hPage, pHeader->strBranchParam, length );
           }
           break;
       case SIPCFG_CONTACT1_URI:
           {
               MsgContactHeader* pHeader = (MsgContactHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               stat = AddrEncode(pHeader->hAddrSpec, hPool, *hPage, RV_FALSE, RV_FALSE, length);
           }
           break;
       case SIPCFG_CONTACT1_URI_USER:
           {
               MsgContactHeader* pHeader = (MsgContactHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               MsgAddress* pAddr = (MsgAddress*)(pHeader->hAddrSpec);
               MsgAddrUrl* pUrl  = (MsgAddrUrl*)(pAddr->uAddrBody.hUrl);
               stat = MsgUtilsEncodeString( msg->pMsgMgr, hPool, *hPage, pAddr->hPool, pAddr->hPage, pUrl->strUser, length );
           }
           break;
       case SIPCFG_CONTACT1_URI_HOST:
           {
               MsgContactHeader* pHeader = (MsgContactHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               MsgAddress* pAddr = (MsgAddress*)(pHeader->hAddrSpec);
               MsgAddrUrl* pUrl  = (MsgAddrUrl*)(pAddr->uAddrBody.hUrl);
               stat = MsgUtilsEncodeString( msg->pMsgMgr, hPool, *hPage, pAddr->hPool, pAddr->hPage, pUrl->strHost, length);
           }
           break;
       case SIPCFG_CONTACT1_URI_TRANSPORT:
           {
               MsgContactHeader* pHeader = (MsgContactHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               MsgAddress* pAddr = (MsgAddress*)(pHeader->hAddrSpec);
               MsgAddrUrl* pUrl  = (MsgAddrUrl*)(pAddr->uAddrBody.hUrl);
               stat = MsgUtilsEncodeTransportType( msg->pMsgMgr, hPool, *hPage, pUrl->eTransport, pAddr->hPool, pAddr->hPage, pUrl->strTransport, length);
           }
           break;
       case SIPCFG_CONTACT1_EXPIRES:
           {
               MsgContactHeader* pHeader = (MsgContactHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               MsgExpiresHeader* pExpHeader = (MsgExpiresHeader*)pHeader->hExpiresParam;
               stat = IntervalEncode(pExpHeader->pInterval, hPool, *hPage, RV_FALSE, RV_FALSE, length);
               /*
               char bufInterval[20];
               MsgUtils_itoa(pExpHeader->pInterval->deltaSeconds , bufInterval);
               stat = MsgUtilsEncodeExternalString( pHeader->pMsgMgr, hPool, *hPage, bufInterval, length );
               */
           }
           break;
       case SIPCFG_CONTACT1_ISFOCUS:
           stat = RV_Success;
           // nothing to add
           break;
       case SIPCFG_AUTHORIZATION1_SCHEME:          
           {
               MsgAuthorizationHeader*  pHeader = (MsgAuthorizationHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               stat = MsgUtilsEncodeAuthScheme(pHeader->pMsgMgr, hPool, *hPage, pHeader->eAuthScheme, pHeader->hPool, pHeader->hPage, pHeader->strAuthScheme, length);
           }
           break;
       case SIPCFG_AUTHORIZATION1_USERNAME:
           {
               MsgAuthorizationHeader*  pHeader = (MsgAuthorizationHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               if(pHeader->strUserName > UNDEFINED) 
                   stat = MsgUtilsEncodeString(pHeader->pMsgMgr, hPool, *hPage, pHeader->hPool, pHeader->hPage, pHeader->strUserName, length);
               else 
                   stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, *hPage, "\"\"", length);
           }
           break;
       case SIPCFG_AUTHORIZATION1_REALM:
           {
               MsgAuthorizationHeader*  pHeader = (MsgAuthorizationHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               if(pHeader->strRealm > UNDEFINED)
                   stat = MsgUtilsEncodeString(pHeader->pMsgMgr, hPool, *hPage, pHeader->hPool, pHeader->hPage, pHeader->strRealm, length);
               else
                   stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, *hPage, "\"\"", length);
           }
           break;
       case SIPCFG_AUTHORIZATION1_NONCE:
           {
               MsgAuthorizationHeader*  pHeader = (MsgAuthorizationHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               if(pHeader->strNonce > UNDEFINED)
                   stat = MsgUtilsEncodeString(pHeader->pMsgMgr, hPool, *hPage, pHeader->hPool, pHeader->hPage, pHeader->strNonce, length);
               else
                   stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, *hPage, "\"\"", length);
           }
           break;
       case SIPCFG_AUTHORIZATION1_URI:
           {
               MsgAuthorizationHeader*  pHeader = (MsgAuthorizationHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               if(pHeader->hDigestUri != NULL)
                   stat = AddrEncode(pHeader->hDigestUri, hPool, *hPage, RV_FALSE, RV_FALSE, length);
               // else leave it empty " is not part of uri
           }
           break;
       case SIPCFG_AUTHORIZATION1_RESPONSE:
           {
               MsgAuthorizationHeader*  pHeader = (MsgAuthorizationHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               if(pHeader->strResponse != UNDEFINED)
                   stat = MsgUtilsEncodeString(pHeader->pMsgMgr, hPool, *hPage, pHeader->hPool, pHeader->hPage, pHeader->strResponse, length);
               else
                   stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, *hPage, "\"\"", length);
           }
           break;
       case SIPCFG_AUTHORIZATION1_ALGORITHM:
           {
               MsgAuthorizationHeader*  pHeader = (MsgAuthorizationHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               stat = MsgUtilsEncodeAuthAlgorithm(pHeader->pMsgMgr, hPool, *hPage, pHeader->eAlgorithm, pHeader->hPool, pHeader->hPage, pHeader->strAlgorithm, length);
           }
           break;
       case SIPCFG_AUTHORIZATION1_CNONCE:
           {
               MsgAuthorizationHeader*  pHeader = (MsgAuthorizationHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               stat = MsgUtilsEncodeString(pHeader->pMsgMgr, hPool, *hPage, pHeader->hPool, pHeader->hPage, pHeader->strCnonce, length);
           }
           break;
       case SIPCFG_AUTHORIZATION1_OPAQUE:
           {
               MsgAuthorizationHeader*  pHeader = (MsgAuthorizationHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               stat = MsgUtilsEncodeString(pHeader->pMsgMgr, hPool, *hPage, pHeader->hPool, pHeader->hPage, pHeader->strOpaque, length);
           }
           break;
       case SIPCFG_AUTHORIZATION1_QOP:
           {
               MsgAuthorizationHeader*  pHeader = (MsgAuthorizationHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               stat = MsgUtilsEncodeQopOptions(pHeader->pMsgMgr, hPool, *hPage, pHeader->eAuthQop, length);
           }
           break;
       case SIPCFG_AUTHORIZATION1_NC:
           {
               RV_CHAR tempNc[9];
               MsgAuthorizationHeader*  pHeader = (MsgAuthorizationHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               sprintf(tempNc, "%08x", pHeader->nonceCount);
               stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, *hPage, tempNc, length);
           }
           break;
       case SIPCFG_AUTHORIZATION1_AUTHPARAM:
           {
               MsgAuthorizationHeader*  pHeader = (MsgAuthorizationHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               stat = MsgUtilsEncodeString(pHeader->pMsgMgr, hPool, *hPage, pHeader->hPool, pHeader->hPage, pHeader->strAuthParam, length);
           }
           break;
       case SIPCFG_CONTENTTYPE_MEDIATYPE:
           {
               MsgContentTypeHeader* pHeader = (MsgContentTypeHeader*)((RvSipContentTypeHeaderHandle)msg->pBodyObject->pContentType);
               stat = MsgUtilsEncodeMediaType(pHeader->pMsgMgr, hPool, *hPage, pHeader->eMediaType, pHeader->hPool, pHeader->hPage,
                                              pHeader->strMediaType, length);

               last_mediatype = (int)(pHeader->eMediaType);
           }
           break;
       case SIPCFG_CONTENTTYPE_MEDIASUBTYPE:
           {
               MsgContentTypeHeader* pHeader = (MsgContentTypeHeader*)((RvSipContentTypeHeaderHandle)msg->pBodyObject->pContentType);

               stat = MsgUtilsEncodeMediaSubType(pHeader->pMsgMgr, hPool, *hPage, pHeader->eMediaSubType, pHeader->hPool, pHeader->hPage, 
                                                 pHeader->strMediaSubType, length);

               last_mediasubtype = (int)pHeader->eMediaSubType;
           }
           break;
       case SIPCFG_CONTENTTYPE_BOUNDARY:
           {
               MsgContentTypeHeader* pHeader = (MsgContentTypeHeader*)((RvSipContentTypeHeaderHandle)msg->pBodyObject->pContentType);
               stat = MsgUtilsEncodeString(pHeader->pMsgMgr, hPool, *hPage, pHeader->hPool, pHeader->hPage, pHeader->strBoundary, length);
           }
           break;
       case SIPCFG_CONTENTTYPE_VERSION:
           {
               MsgContentTypeHeader* pHeader = (MsgContentTypeHeader*)((RvSipContentTypeHeaderHandle)msg->pBodyObject->pContentType);
               stat = MsgUtilsEncodeString(pHeader->pMsgMgr, hPool, *hPage, pHeader->hPool, pHeader->hPage, pHeader->strVersion, length);
           }
           break;
       case SIPCFG_CONTENTTYPE_BASE:
           {
               MsgContentTypeHeader* pHeader = (MsgContentTypeHeader*)((RvSipContentTypeHeaderHandle)msg->pBodyObject->pContentType);
               stat = MsgUtilsEncodeString(pHeader->pMsgMgr, hPool, *hPage, pHeader->hPool, pHeader->hPage, pHeader->strBase, length);
           }
           break;
       case SIPCFG_RSEQ:
           {
                RV_CHAR           strHelper[16];
                MsgRSeqHeader* pHeader = (MsgRSeqHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
                MsgUtils_itoa(pHeader->respNum.val, strHelper);
                stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, *hPage, strHelper, length);
           }
           break;
       case SIPCFG_RACK_SEQ:
           {
               RV_CHAR           strHelper[16];
               MsgRAckHeader*  pHeader = (MsgRAckHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               MsgUtils_itoa(pHeader->rseq.val, strHelper);
               stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, *hPage, strHelper, length);
           }
           break;
       case SIPCFG_RACK_CSEQ:
           {
               RV_CHAR           strHelper[16];
               MsgRAckHeader*  pHeader_rack = (MsgRAckHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               MsgCSeqHeader*  pHeader = (MsgCSeqHeader*)pHeader_rack->hCSeq;
               MsgUtils_itoa(pHeader->step.val, strHelper);
               stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, *hPage, strHelper, length);
           }
           break;
       case SIPCFG_RACK_CSEQ_METHOD:
           {
               MsgRAckHeader*  pHeader_rack = (MsgRAckHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               MsgCSeqHeader*  pHeader = (MsgCSeqHeader*)pHeader_rack->hCSeq;
               stat = MsgUtilsEncodeMethodType(pHeader->pMsgMgr, hPool, *hPage, pHeader->eMethod, pHeader->hPool, pHeader->hPage, 
                                               pHeader->strOtherMethod, length);
           }
           break;
       case SIPCFG_CONTENTLENGTH:
           {
               MsgUtils_itoa( ContentLength_value, tempString);
               stat = MsgUtilsEncodeExternalString( msg->pMsgMgr, hPool, *hPage, tempString, length);
           }
           break;

       case SIPCFG_ROUTE1:
       case SIPCFG_ROUTE2:
       case SIPCFG_ROUTE3:
       case SIPCFG_ROUTE4:
       case SIPCFG_ROUTE5:
       case SIPCFG_ROUTE6:
       case SIPCFG_ROUTE7:
       case SIPCFG_ROUTE8:
           {
               MsgRouteHopHeader* pHeader = (MsgRouteHopHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               stat = AddrEncode(pHeader->hAddrSpec, hPool, *hPage, RV_FALSE, RV_FALSE, length);
           }
           break;
       case SIPCFG_EVENT:
           {
               MsgEventHeader* pHeader = (MsgEventHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               stat = MsgUtilsEncodeString(pHeader->pMsgMgr, hPool, *hPage,pHeader->hPool,
                                    pHeader->hPage, pHeader->strEventPackage, length);
           }
           break;
       case SIPCFG_EXPIRES:
           {
               MsgExpiresHeader* pHeader = (MsgExpiresHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               stat = IntervalEncode(pHeader->pInterval, hPool, *hPage, RV_FALSE, RV_FALSE, length);
           }
           break;
       case SIPCFG_SUBSCRIPTIONSTATE:
           {
               MsgSubscriptionStateHeader* pHeader = (MsgSubscriptionStateHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               stat = MsgUtilsEncodeSubstateVal(pHeader->pMsgMgr, hPool, *hPage, pHeader->eSubstate, 
                                     pHeader->hPool, pHeader->hPage, pHeader->strSubstate, length);
           }
           break;
       case SIPCFG_SUBSCRIPTION_EXPIRES:
           {
               MsgSubscriptionStateHeader* pHeader = (MsgSubscriptionStateHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               char bufInterval[20];
               MsgUtils_itoa(pHeader->expiresVal, bufInterval);
               stat = MsgUtilsEncodeExternalString( pHeader->pMsgMgr, hPool, *hPage, bufInterval, length );
           }
           break;
       case SIPCFG_PROXYREQUIRE:
           {
               MsgOtherHeader* pHeader = (MsgOtherHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               stat = MsgUtilsEncodeString(pHeader->pMsgMgr, hPool, *hPage, pHeader->hPool, 
                                     pHeader->hPage, pHeader->strHeaderVal, length );
           }
           break;
       case SIPCFG_REFER_TO:
           {
               MsgReferToHeader* pHeader = (MsgReferToHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               stat = AddrEncode(pHeader->hAddrSpec, hPool, *hPage, RV_FALSE, RV_FALSE, length);
           }
           break;
       case SIPCFG_REFERRED_BY:
           {
               MsgReferredByHeader* pHeader = (MsgReferredByHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               stat = AddrEncode(pHeader->hReferrerAddrSpec, hPool, *hPage, RV_FALSE, RV_FALSE, length);
           }
           break;
       case SIPCFG_REPLACES_CALLID:
           {
               MsgReplacesHeader* pHeader = (MsgReplacesHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               if ( NULL == pHeader )
               {
                   // if Replaces header is not found, try it inside Refer-To header
                   MsgReferToHeader* pReferToHeader = (MsgReferToHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, SIPCFG_REFER_TO );
                   if (pReferToHeader)
                   {
                       pHeader = (MsgReplacesHeader*)pReferToHeader->hReplacesHeader;
                   }
               }
               if (pHeader)
               {
                   stat = MsgUtilsEncodeString( pHeader->pMsgMgr, hPool, *hPage, pHeader->hPool,
                                                pHeader->hPage, pHeader->strCallID, length );
               }
               else stat = RV_InvalidParameter;
           }
           break;
       case SIPCFG_REPLACES_FROM_TAG:
           {
               MsgReplacesHeader* pHeader = (MsgReplacesHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               if ( NULL == pHeader )
               {
                   // if Replaces header is not found, try it inside Refer-To header
                   MsgReferToHeader* pReferToHeader = (MsgReferToHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, SIPCFG_REFER_TO );
                   if (pReferToHeader)
                   {
                       pHeader = (MsgReplacesHeader*)pReferToHeader->hReplacesHeader;
                   }
               }
               if (pHeader)
               {
                   stat = MsgUtilsEncodeString( pHeader->pMsgMgr, hPool, *hPage, pHeader->hPool,
                                                pHeader->hPage, pHeader->strFromTag, length );
               }
               else stat = RV_InvalidParameter;
           }
           break;
       case SIPCFG_REPLACES_TO_TAG:
           {
               MsgReplacesHeader* pHeader = (MsgReplacesHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               if ( NULL == pHeader )
               {
                   // if Replaces header is not found, try it inside Refer-To header
                   MsgReferToHeader* pReferToHeader = (MsgReferToHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, SIPCFG_REFER_TO );
                   if (pReferToHeader)
                   {
                       pHeader = (MsgReplacesHeader*)pReferToHeader->hReplacesHeader;
                   }
               }
               if (pHeader)
               {
                   stat = MsgUtilsEncodeString( pHeader->pMsgMgr, hPool, *hPage, pHeader->hPool,
                                                pHeader->hPage, pHeader->strToTag, length );
               }
               else stat = RV_InvalidParameter;
           }
           break;
       case SIPCFG_ALLOW:
       case SIPCFG_SUPPORTED_GRUU:
       case SIPCFG_SUPPORTED_JOIN:
       case SIPCFG_SUPPORTED_REPLACES:
       case SIPCFG_SUPPORTED_100REL:
           stat = RV_Success;
           // nothing to add
           break;
       case SIPCFG_JOIN_CALLID:
           {
               MsgOtherHeader* pHeader = (MsgOtherHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               uint actualLen = 0;
               char buf[1000];
               char parmBuf[257];
               RvSipOtherHeaderGetValue ( (RvSipOtherHeaderHandle)pHeader, buf, sizeof(buf), &actualLen );
               SPIRENT_RvSIP_GetParameterValueFromString ( msg, buf, DELIM, NULL, parmBuf, sizeof(parmBuf) );
               stat = MsgUtilsEncodeExternalString( pHeader->pMsgMgr, hPool, *hPage, parmBuf, length );
           }
           break;
       case SIPCFG_JOIN_FROM_TAG:
           {
               MsgOtherHeader* pHeader = (MsgOtherHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               uint actualLen = 0;
               char buf[1000];
               char parmBuf[257];
               RvSipOtherHeaderGetValue ( (RvSipOtherHeaderHandle)pHeader, buf, sizeof(buf), &actualLen );
               SPIRENT_RvSIP_GetParameterValueFromString ( msg, buf, DELIM, "from-tag", parmBuf, sizeof(parmBuf) );
               stat = MsgUtilsEncodeExternalString( pHeader->pMsgMgr, hPool, *hPage, parmBuf, length );
           }
           break;
       case SIPCFG_JOIN_TO_TAG:
           {
               MsgOtherHeader* pHeader = (MsgOtherHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               uint actualLen = 0;
               char buf[1000];
               char parmBuf[257];
               RvSipOtherHeaderGetValue ( (RvSipOtherHeaderHandle)pHeader, buf, sizeof(buf), &actualLen );
               SPIRENT_RvSIP_GetParameterValueFromString ( msg, buf, DELIM, "to-tag", parmBuf, sizeof(parmBuf) );
               stat = MsgUtilsEncodeExternalString( pHeader->pMsgMgr, hPool, *hPage, parmBuf, length );
           }
           break;
       case SIPCFG_ACCEPT1:
       case SIPCFG_ACCEPT2:
       case SIPCFG_ACCEPT3:
           stat = RV_Success;
           // nothing to add
           break;
       case SIPCFG_ACCEPT_MEDIATYPE1:
       case SIPCFG_ACCEPT_MEDIATYPE2:
       case SIPCFG_ACCEPT_MEDIATYPE3:
           {
               MsgOtherHeader* pHeader = (MsgOtherHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               uint actualLen = 0;
               char buf[1000];
               char * pSlash;
               RvSipOtherHeaderGetValue ( (RvSipOtherHeaderHandle)pHeader, buf, sizeof(buf), &actualLen );
               pSlash = strchr (buf, '/');
               if (pSlash)
                   *pSlash = '\0';  // end of MediaType
               stat = MsgUtilsEncodeExternalString( pHeader->pMsgMgr, hPool, *hPage, buf, length );
           }
           break;
       case SIPCFG_ACCEPT_MEDIASUBTYPE1:
       case SIPCFG_ACCEPT_MEDIASUBTYPE2:
       case SIPCFG_ACCEPT_MEDIASUBTYPE3:
           {
               MsgOtherHeader* pHeader = (MsgOtherHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               uint actualLen = 0;
               char buf[1000];
               char * pSlash;
               RvSipOtherHeaderGetValue ( (RvSipOtherHeaderHandle)pHeader, buf, sizeof(buf), &actualLen );
               pSlash = strchr (buf, '/');  // start of MediaSubtype
               if (!pSlash)
                   pSlash = buf-1;
               stat = MsgUtilsEncodeExternalString( pHeader->pMsgMgr, hPool, *hPage, pSlash+1, length );
           }
           break;
       case SIPCFG_ACCEPT_ENCODING1:
       case SIPCFG_ACCEPT_ENCODING2:
       case SIPCFG_ACCEPT_ENCODING3:
       case SIPCFG_ACCEPT_LANGUAGE1:
       case SIPCFG_ACCEPT_LANGUAGE2:
       case SIPCFG_ACCEPT_LANGUAGE3:
           {
               MsgOtherHeader* pHeader = (MsgOtherHeader*)SPIRENT_RvSIP_GetHeaderFromList ( hSipMsg, Current_Param );
               uint actualLen = 0;
               char buf[1000];
               RvSipOtherHeaderGetValue ( (RvSipOtherHeaderHandle)pHeader, buf, sizeof(buf), &actualLen );
               stat = MsgUtilsEncodeExternalString( pHeader->pMsgMgr, hPool, *hPage, buf, length );
           }
           break;
       case    SIPCFG_PB:
       case    SIPCFG_ST:
           {
               if (Enc_cfg.get_fptr)
               {
                   int len = 0;
                   const void *str_toadd = Enc_cfg.get_fptr ( Current_Param, (char*)str_cfg->param_name, Enc_cfg.app_param, &len );
                   if ( str_toadd && len > 0 )
                   {

                       *length += len;

                       stat = RPOOL_AppendFromExternal(hPool,
                           *hPage,
                           (void*)str_toadd,
                           len,
                           RV_FALSE,
                           &offset,
                           &encoded);
                       
                       if(stat != RV_Success)
                       {
                           RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                               "MsgUtilsEncodeString - Failed, AppendFromExternal failed. hPool %x, hPage %d, RV_Status: %d",
                               hPool, hPage, stat));
                       }
                   }
               }
           }
           break;
      
          
       case SIPCFG_SDPBODY:
           {
               stat = SPIRENT_RvSIP_EncodeBody ( msg, &Body_hPage, ContentLength_value, hPool, *hPage, length );
           }
           break;
       default:
           stat = RV_Failure;
           RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
               "SPIRENT_RvSipMsgEncode: unhandled parameter %d\n",Current_Param));

           break;
       }

       if(stat != RV_Success)
       {
          RPOOL_FreePage(hPool, *hPage);
          if (NULL_PAGE != Body_hPage) RPOOL_FreePage(hPool, Body_hPage);
          RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
             "SPIRENT_RvSipMsgEncode - Failed to encode message. Free page %d on pool %x. hMsg %x param =%d",
             *hPage, hPool, hSipMsg,Current_Param));
          return stat;
       }

       /* This functionality is used to add, automatically,
          the "boundary=..." clause for multipart MIME messages. */
       if(Current_Param == SIPCFG_CONTENTTYPE_MEDIASUBTYPE &&
          last_param == SIPCFG_CONTENTTYPE_MEDIATYPE &&
          last_mediatype == RVSIP_MEDIATYPE_MULTIPART &&
          last_mediasubtype == RVSIP_MEDIASUBTYPE_MIXED) {
          
          static char ub[]="; boundary=unique-boundary-1";
          
          ptl_cfg_elem* str_cfg_next = &(((ptl_cfg_elem*)Enc_cfg.cfg)[string_num]);
          
          if(str_cfg_next->str && (strcasestr((char*)str_cfg_next->str,"boundary="))) {

          } else {
             
             stat = MsgUtilsEncodeExternalString(msg->pMsgMgr, hPool, *hPage, ub, length);
             if(stat != RV_Success)
             {
                RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                   "MsgUtilsEncodeString - Failed, MsgUtilsEncodeExternalString failed. hPool %x, hPage %d, RV_Status: %d",
                   hPool, hPage, stat));
             }
          }
       }

       last_param=(int)Current_Param;
    }
   if (NULL_PAGE != Body_hPage) RPOOL_FreePage(hPool, Body_hPage);
   return stat;
}

#define NOT_FOUND_POINTER ((void*)(-1))

#define RVSIP_HEADERTYPE_LAST RVSIP_HEADERTYPE_CONTENT_TYPE
typedef enum
{
   SpirentSIP_HEADERTYPE_OTHER_PROXYREQUIRE = RVSIP_HEADERTYPE_LAST + 1,
   SpirentSIP_HEADERTYPE_OTHER_SUPPORTED_GRUU,
   SpirentSIP_HEADERTYPE_OTHER_SUPPORTED_JOIN,
   SpirentSIP_HEADERTYPE_OTHER_SUPPORTED_REPLACES,
   SpirentSIP_HEADERTYPE_OTHER_SUPPORTED_100REL,
   SpirentSIP_HEADERTYPE_OTHER_JOIN,
   SpirentSIP_HEADERTYPE_OTHER_ACCEPT,
   SpirentSIP_HEADERTYPE_OTHER_ACCEPT_ENCODING,
   SpirentSIP_HEADERTYPE_OTHER_ACCEPT_LANGUAGE,

   SpirentSIP_HEADERTYPE_TOTAL
} SpirentSipHeaderType;

void* header_list[SpirentSIP_HEADERTYPE_TOTAL][10]; // 10 pointers to vias

void SPIRENT_RvSIP_HeaderListGetCacheInit ( void )
{
   memset (header_list,0,sizeof(header_list));
}

static char* SPIRENT_GetOtherHeaderName( SpirentSipHeaderType type )
{
   // Get SIP header name for OTHER header type.
   switch( type )
   {
   case SpirentSIP_HEADERTYPE_OTHER_PROXYREQUIRE:
      return "Proxy-Require";

   case SpirentSIP_HEADERTYPE_OTHER_SUPPORTED_GRUU:
   case SpirentSIP_HEADERTYPE_OTHER_SUPPORTED_JOIN:
   case SpirentSIP_HEADERTYPE_OTHER_SUPPORTED_REPLACES:
   case SpirentSIP_HEADERTYPE_OTHER_SUPPORTED_100REL:
      return "Supported";

   case SpirentSIP_HEADERTYPE_OTHER_JOIN:
      return "Join";

   case SpirentSIP_HEADERTYPE_OTHER_ACCEPT:
      return "Accept";

   case SpirentSIP_HEADERTYPE_OTHER_ACCEPT_ENCODING:
      return "Accept-Encoding";

   case SpirentSIP_HEADERTYPE_OTHER_ACCEPT_LANGUAGE:
      return "Accept-Language";

   default:
      return NULL;
   }

   return NULL;
}

static int SPIRENT_GetSupportedOptionTag (SpirentSipHeaderType type, char * tag_buffer)
{
   // Get option tag for OTHER Supported header.
   int res = 1;   // option tag found
   switch( type )
   {
   case SpirentSIP_HEADERTYPE_OTHER_SUPPORTED_GRUU:
      strcpy (tag_buffer, "gruu");
      break;
   case SpirentSIP_HEADERTYPE_OTHER_SUPPORTED_JOIN:
      strcpy (tag_buffer, "join");
      break;
   case SpirentSIP_HEADERTYPE_OTHER_SUPPORTED_REPLACES:
      strcpy (tag_buffer, "replaces");
      break;
   case SpirentSIP_HEADERTYPE_OTHER_SUPPORTED_100REL:
      strcpy (tag_buffer, "100rel");
      break;
   default:
      res = 0;
      break;
   }

   return res;
}

static void SPIRENT_CheckSupportedOptionTag (void* returnedHeader, char * tag)
{
   // clears tag (*tag = 0) if tag is found in Supported header value
   RvSipOtherHeaderHandle hHeader = (RvSipOtherHeaderHandle)returnedHeader;
   uint actualLen = 0;
   char buf[1000];
   int  found = 0;
   char * pTag;
   char * pBuf = buf;
   RvSipOtherHeaderGetValue ( hHeader, buf, sizeof(buf), &actualLen );
   do {
      pTag = strcasestr ( pBuf, tag );
      if (pTag)
      {
         if ( pTag == pBuf || strchr (", \t", *(pTag-1)) )
         {
            // got delimiter before tag
            int post_char = *(pTag + strlen (tag));
            if ( post_char == 0 || strchr (", \t", post_char) )
            {
               // got delimiter after tag
               found = 1;
               *tag  = 0;
            }
            else
               pBuf = pTag+1; // look for another location
         }
         else
            pBuf = pTag+1; // look for another location
      }
      else
         break;
   } while (!found);
}

void* SPIRENT_RvSIP_GetHeaderFromList ( IN   RvSipMsgHandle   hMsg, int param )
{

   void*                          returnedHeader;
   MsgMessage*                    msg = (MsgMessage*)hMsg;
   RvSipHeaderListElemHandle      elem;
   RvSipHeaderType                type;

   int current_num = 0;
   int header_type = -1;
   int header_num = -1;
   RV_INT  safeCounter = 0;

   if (msg->headerList == NULL)
   {
     return NULL;
   }

   switch ( param )
   {
   case SIPCFG_VIA1:
   case SIPCFG_VIA1_TRANSPORT:
   case SIPCFG_VIA1_IP:
   case SIPCFG_VIA1_PORT:
   case SIPCFG_VIA1_BRANCH:
      header_type = RVSIP_HEADERTYPE_VIA;
      header_num  = 0;
      break;
   case SIPCFG_VIA2:
   case SIPCFG_VIA2_TRANSPORT:
   case SIPCFG_VIA2_IP:
   case SIPCFG_VIA2_PORT:
   case SIPCFG_VIA2_BRANCH:
      header_type = RVSIP_HEADERTYPE_VIA;
      header_num  = 1;
      break;
   case SIPCFG_VIA3:
   case SIPCFG_VIA3_TRANSPORT:
   case SIPCFG_VIA3_IP:
   case SIPCFG_VIA3_PORT:
   case SIPCFG_VIA3_BRANCH:
      header_type = RVSIP_HEADERTYPE_VIA;
      header_num  = 2;
      break;
   case SIPCFG_CONTACT1:
   case SIPCFG_CONTACT1_URI:
   case SIPCFG_CONTACT1_URI_USER:
   case SIPCFG_CONTACT1_URI_HOST:
   case SIPCFG_CONTACT1_URI_TRANSPORT:
   case SIPCFG_CONTACT1_EXPIRES:
   case SIPCFG_CONTACT1_ISFOCUS:
      header_type = RVSIP_HEADERTYPE_CONTACT;
      header_num  = 0;
      break;
   case SIPCFG_AUTHORIZATION1:
   case SIPCFG_AUTHORIZATION1_SCHEME:
   case SIPCFG_AUTHORIZATION1_USERNAME:
   case SIPCFG_AUTHORIZATION1_REALM:
   case SIPCFG_AUTHORIZATION1_NONCE:
   case SIPCFG_AUTHORIZATION1_URI:
   case SIPCFG_AUTHORIZATION1_RESPONSE:
   case SIPCFG_AUTHORIZATION1_ALGORITHM:
   case SIPCFG_AUTHORIZATION1_CNONCE:
   case SIPCFG_AUTHORIZATION1_OPAQUE:
   case SIPCFG_AUTHORIZATION1_QOP:
   case SIPCFG_AUTHORIZATION1_NC:
   case SIPCFG_AUTHORIZATION1_AUTHPARAM:
   //case SIPCFG_AUTHORIZATION1_STALE:  // is used by Server only in 401/407 response
      header_type = RVSIP_HEADERTYPE_AUTHORIZATION;
      header_num  = 0;
      break;
   case SIPCFG_RACK:
   case SIPCFG_RACK_SEQ:
   case SIPCFG_RACK_CSEQ:
   case SIPCFG_RACK_CSEQ_METHOD:
      header_type = RVSIP_HEADERTYPE_RACK;
      header_num  = 0;
      break;
   case SIPCFG_RSEQ:
      header_type = RVSIP_HEADERTYPE_RSEQ;
      header_num  = 0;
      break;
   case SIPCFG_ROUTE1:
      header_type = RVSIP_HEADERTYPE_ROUTE_HOP;
      header_num  = 0;
      break;
   case SIPCFG_ROUTE2:
      header_type = RVSIP_HEADERTYPE_ROUTE_HOP;
      header_num  = 1;
      break;
   case SIPCFG_ROUTE3:
      header_type = RVSIP_HEADERTYPE_ROUTE_HOP;
      header_num  = 2;
      break;
   case SIPCFG_ROUTE4:
      header_type = RVSIP_HEADERTYPE_ROUTE_HOP;
      header_num  = 3;
      break;      
   case SIPCFG_ROUTE5:
      header_type = RVSIP_HEADERTYPE_ROUTE_HOP;
      header_num  = 4;
      break;
   case SIPCFG_ROUTE6:
      header_type = RVSIP_HEADERTYPE_ROUTE_HOP;
      header_num  = 5;
      break;
   case SIPCFG_ROUTE7:
      header_type = RVSIP_HEADERTYPE_ROUTE_HOP;
      header_num  = 6;
      break;
   case SIPCFG_ROUTE8:
      header_type = RVSIP_HEADERTYPE_ROUTE_HOP;
      header_num  = 7;
      break;      
   case SIPCFG_EVENT:
      header_type = RVSIP_HEADERTYPE_EVENT;
      header_num  = 0;
      break;
   case SIPCFG_EXPIRES:
      header_type = RVSIP_HEADERTYPE_EXPIRES;
      header_num  = 0;
      break;
   case SIPCFG_SUBSCRIPTIONSTATE:
   case SIPCFG_SUBSCRIPTION_EXPIRES:
      header_type = RVSIP_HEADERTYPE_SUBSCRIPTION_STATE;
      header_num  = 0;
      break;
   case SIPCFG_PROXYREQUIRE:
      header_type = SpirentSIP_HEADERTYPE_OTHER_PROXYREQUIRE;
      header_num  = 0;
      break;
   case SIPCFG_REFER_TO:
      header_type = RVSIP_HEADERTYPE_REFER_TO;
      header_num  = 0;
      break;
   case SIPCFG_REFERRED_BY:
      header_type = RVSIP_HEADERTYPE_REFERRED_BY;
      header_num  = 0;
      break;
   case SIPCFG_REPLACES_CALLID:
   case SIPCFG_REPLACES_FROM_TAG:
   case SIPCFG_REPLACES_TO_TAG:
      header_type = RVSIP_HEADERTYPE_REPLACES;
      header_num  = 0;
      break;
   case SIPCFG_ALLOW:
      header_type = RVSIP_HEADERTYPE_ALLOW;
      header_num  = 0;
      break;
   case SIPCFG_SUPPORTED_GRUU:
      header_type = SpirentSIP_HEADERTYPE_OTHER_SUPPORTED_GRUU;
      header_num  = -1;
      break;
   case SIPCFG_SUPPORTED_JOIN:
      header_type = SpirentSIP_HEADERTYPE_OTHER_SUPPORTED_JOIN;
      header_num  = -1;
      break;
   case SIPCFG_SUPPORTED_REPLACES:
      header_type = SpirentSIP_HEADERTYPE_OTHER_SUPPORTED_REPLACES;
      header_num  = -1;
      break;
   case SIPCFG_SUPPORTED_100REL:
      header_type = SpirentSIP_HEADERTYPE_OTHER_SUPPORTED_100REL;
      header_num  = -1;
      break;
   case SIPCFG_JOIN_CALLID:
   case SIPCFG_JOIN_FROM_TAG:
   case SIPCFG_JOIN_TO_TAG:
      // RVSIP_HEADERTYPE_JOIN is not defined in SIP stack
      header_type = SpirentSIP_HEADERTYPE_OTHER_JOIN;
      header_num  = 0;
      break;
   case SIPCFG_ACCEPT1:
   case SIPCFG_ACCEPT_MEDIATYPE1:
   case SIPCFG_ACCEPT_MEDIASUBTYPE1:
      header_type = SpirentSIP_HEADERTYPE_OTHER_ACCEPT;
      header_num  = 0;
      break;
   case SIPCFG_ACCEPT_ENCODING1:
      header_type = SpirentSIP_HEADERTYPE_OTHER_ACCEPT_ENCODING;
      header_num  = 0;
      break;
   case SIPCFG_ACCEPT_LANGUAGE1:
      header_type = SpirentSIP_HEADERTYPE_OTHER_ACCEPT_LANGUAGE;
      header_num  = 0;
      break;
   case SIPCFG_ACCEPT2:
   case SIPCFG_ACCEPT_MEDIATYPE2:
   case SIPCFG_ACCEPT_MEDIASUBTYPE2:
      header_type = SpirentSIP_HEADERTYPE_OTHER_ACCEPT;
      header_num  = 1;
      break;
   case SIPCFG_ACCEPT_ENCODING2:
      header_type = SpirentSIP_HEADERTYPE_OTHER_ACCEPT_ENCODING;
      header_num  = 1;
      break;
   case SIPCFG_ACCEPT_LANGUAGE2:
      header_type = SpirentSIP_HEADERTYPE_OTHER_ACCEPT_LANGUAGE;
      header_num  = 1;
      break;
   case SIPCFG_ACCEPT3:
   case SIPCFG_ACCEPT_MEDIATYPE3:
   case SIPCFG_ACCEPT_MEDIASUBTYPE3:
      header_type = SpirentSIP_HEADERTYPE_OTHER_ACCEPT;
      header_num  = 2;
      break;
   case SIPCFG_ACCEPT_ENCODING3:
      header_type = SpirentSIP_HEADERTYPE_OTHER_ACCEPT_ENCODING;
      header_num  = 2;
      break;
   case SIPCFG_ACCEPT_LANGUAGE3:
      header_type = SpirentSIP_HEADERTYPE_OTHER_ACCEPT_LANGUAGE;
      header_num  = 2;
      break;
   default:
      return NULL;
   }

   if ( header_list[header_type][header_num] == NOT_FOUND_POINTER )
   {
      return NULL;
   } 
   else if ( header_list[header_type][header_num] )
   {
      return header_list[header_type][header_num];
   }

   if( header_type <= RVSIP_HEADERTYPE_LAST )
   {  
      // Get regular (RvSipHeaderType) SIP header handle.

      returnedHeader = RvSipMsgGetHeaderExt ( hMsg, RVSIP_FIRST_HEADER, RVSIP_MSG_HEADERS_OPTION_ALL, &elem, &type);
      while((returnedHeader != NULL) && (safeCounter < MaxHeadersPerMsg))
      {
         ++safeCounter;
         if ( type == header_type )
         {
            if ( header_num == current_num )
            {
               // write it to cache
               header_list[header_type][header_num] = returnedHeader;
               return returnedHeader;
            }
            else
            {
               current_num++;
            }
         }
         returnedHeader = RvSipMsgGetHeaderExt ( hMsg, RVSIP_NEXT_HEADER, RVSIP_MSG_HEADERS_OPTION_ALL, &elem, &type);
      }
      header_list[header_type][header_num] = NOT_FOUND_POINTER;
   }
   else
   {
      // Get OTHER SIP header handle.

      returnedHeader = (void*)RvSipMsgGetHeaderByName(hMsg, SPIRENT_GetOtherHeaderName(header_type), RVSIP_FIRST_HEADER, &elem);
      while((returnedHeader != NULL) && (safeCounter < MaxHeadersPerMsg))
      {
         char tag[20];
         *tag = 0;
         if (SPIRENT_GetSupportedOptionTag(header_type, tag))
         {
            SPIRENT_CheckSupportedOptionTag(returnedHeader,tag);
            // it will reset *tag = 0 if found in returnedHeader
         }
         ++safeCounter;
         if ( *tag == 0 && (header_num == current_num || header_num < 0) )
         {
               // write it to cache
            header_list[header_type][header_num] = returnedHeader;
            return returnedHeader;
         }
         else
         {
            current_num++;
         }
         returnedHeader = (void*)RvSipMsgGetHeaderByName(hMsg, SPIRENT_GetOtherHeaderName(header_type), RVSIP_NEXT_HEADER, &elem);
      }
      header_list[header_type][header_num] = NOT_FOUND_POINTER;
   }

   return NULL;
}

RV_INT32 SPIRENT_RvSIP_CalculateContentLength ( IN MsgMessage* msg, IN HRPOOL hPool, HPAGE*  hTempPage ) 
{ 
  /* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
  RvUint32 tempLength;
#else
  RV_INT32 tempLength;
#endif
  /* SPIRENT_END */

    
    RV_Status     stat;
    if ((NULL != msg->pBodyObject) &&
        (NULL != msg->pBodyObject->hBodyPartsList))
    {
        /* Encode message body on a temporary page to calculate the Content-Length */
        stat = RvSipBodyEncode((RvSipBodyHandle)msg->pBodyObject, hPool,
                                  hTempPage, (RvUint32*)&tempLength);
        if(stat != RV_Success)
        {
            RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                       "MsgEncodeBody - msg %x - RvSipBodyEncode failed.", msg));
            return stat;
        }
        /* The length of the body is the length that is returned by
           RvSipBodyEncode() */
    }
    else if ((NULL != msg->pBodyObject) &&
             (UNDEFINED != msg->pBodyObject->strBody))
    {
        tempLength = msg->pBodyObject->length;
    }
    else
    {
        tempLength = msg->contentLength;
    }
    return tempLength;
}

RV_INT32 SPIRENT_RvSIP_EncodeBody ( IN  MsgMessage*  msg,
                                        HPAGE*      hTempPage,
                                        RV_INT32    tempLength,
                                    IN  HRPOOL      hPool,
   							        IN  HPAGE       hPage,
                                    OUT RV_UINT32*  length)
{    
    RV_Status     stat = RV_Success;
/* Encode message-Body */
    if ((NULL != msg->pBodyObject) && (msg->pBodyObject->strBody != UNDEFINED) && (NULL == msg->pBodyObject->hBodyPartsList))
    {
        RV_INT32 offset;
        stat = RPOOL_Append(hPool, hPage, msg->pBodyObject->length, RV_FALSE, &offset);
        if(stat != RV_Success)
        {
           /*dtprintf("failure at %s:%d\n",__FILE__,__LINE__);*/
           return stat;
        }
        stat = RPOOL_CopyFrom(hPool, hPage, offset, msg->hPool, msg->hPage, msg->pBodyObject->strBody, msg->pBodyObject->length);
        *length += msg->pBodyObject->length;
    }
    else if ((NULL != msg->pBodyObject) && (NULL != msg->pBodyObject->hBodyPartsList))
    {
        RV_INT32 offset;
        stat = RPOOL_Append(hPool, hPage, tempLength, RV_FALSE, &offset);
        if(stat != RV_Success)
        {
           /*dtprintf("failure at %s:%d\n",__FILE__,__LINE__);*/
           return stat;
        }
        stat = RPOOL_CopyFrom(hPool, hPage, offset, hPool, *hTempPage, 0, tempLength);
        if(stat != RV_Success)
        {
           /*dtprintf("failure at %s:%d\n",__FILE__,__LINE__);*/
           return stat;
        }
        *length += tempLength;
    }
    return stat;
}

///////////////////////////////////////////////////////////////////////////////
// SPIRENT_RvSIP_GetParameterValueFromString
//  to be used for decoding value string of OtherHeader
//
// INPUT:   string     - NULL-terminated character string for search of parameters
//          delimiters - set of characters to be skipped before parameter value
//            and to be recognized as an end of parameter value
//          parmName   - parameter-name to be found, if NULL then
//            parameter-value is taken from the beginning of string (header-value)
//          parmBufferSize - size in bytes of parmValue buffer
//
// OUTPUT:  parmValue  - parameter-value as NULL-terminated string
//          returns    - actual length of parmValue string (excluding NULL-byte), error if 0
//
size_t SPIRENT_RvSIP_GetParameterValueFromString ( MsgMessage * msg,
                                    const char * string, const char * delimiters,
                                    const char * parmName, char * parmValue, size_t parmBufferSize )
{
    size_t       length = 0;
    size_t       num_spaces;
    const char * pText = NULL;
    const char * pName = NULL;
    const char * pParm = NULL;

    if ( !string || !delimiters || !parmValue )
    {
        RvLogError(msg->pMsgMgr->pLogSrc, (msg->pMsgMgr->pLogSrc,
                   "SPIRENT_RvSIP_GetParameterValueFromString - Failed: Invalid Parameters"));
        return 0;
    }
    if ( !strchr( delimiters, ';' ) )
    {
        RvLogError(msg->pMsgMgr->pLogSrc, (msg->pMsgMgr->pLogSrc,
                   "SPIRENT_RvSIP_GetParameterValueFromString - Failed: SEMI is Not a Delimiter"));
        return 0;
    }
    /* 1) Find parmName and make sure
          that it is prepended by delimiters only (after ';') */
    if (parmName && parmName[0])
    {
        if ( strpbrk( parmName, delimiters ) )
        {
            RvLogError(msg->pMsgMgr->pLogSrc, (msg->pMsgMgr->pLogSrc,
                       "SPIRENT_RvSIP_GetParameterValueFromString - Failed: Delimiters inside parmName"));
            return 0;
        }
        pName = strcasestr( string, parmName );
        if (pName)
        {
            char save = *pName;
            *(char *)pName = '\0';          // temporary end of string
            pText = strrchr( string, ';' ); // last SEMI before parameter-name
            *(char *)pName = save;          // restore parameter-name
            if (pText)
            {
                num_spaces = strspn( pText, delimiters );
                if ( (pText + num_spaces) != pName )
                {
                    RvLogError(msg->pMsgMgr->pLogSrc, (msg->pMsgMgr->pLogSrc,
                               "SPIRENT_RvSIP_GetParameterValueFromString - Failed: characters between SEMI and parmName"));
                    return 0;
                }
            }
            else
            {
                RvLogError(msg->pMsgMgr->pLogSrc, (msg->pMsgMgr->pLogSrc,
                           "SPIRENT_RvSIP_GetParameterValueFromString - Failed: no SEMI before parmName"));
                return 0;
            }
        }
        else
        {
            RvLogError(msg->pMsgMgr->pLogSrc, (msg->pMsgMgr->pLogSrc,
                       "SPIRENT_RvSIP_GetParameterValueFromString - Failed: parmName Not Found"));
            return 0;
        }

        /* 2a) Find pParm (start of parameter-value) */
        pText = pName + strlen(parmName);
        num_spaces = strspn( pText, delimiters );
        pName = strchr( pText, ';' );   // first SEMI after parmName
        if ( pName && (pText + num_spaces) > pName )
        {
            RvLogError(msg->pMsgMgr->pLogSrc, (msg->pMsgMgr->pLogSrc,
                       "SPIRENT_RvSIP_GetParameterValueFromString - Failed: SEMI between parmName and EQUAL"));
            return 0;
        }
        pText += num_spaces;
        if ( *pText != '=' )
        {
            RvLogError(msg->pMsgMgr->pLogSrc, (msg->pMsgMgr->pLogSrc,
                       "SPIRENT_RvSIP_GetParameterValueFromString - Failed: EQUAL Not Found before parmValue"));
            return 0;
        }
        ++pText;
        num_spaces = strspn( pText, delimiters );
        if ( pName && (pText + num_spaces) > pName )
        {
            RvLogError(msg->pMsgMgr->pLogSrc, (msg->pMsgMgr->pLogSrc,
                       "SPIRENT_RvSIP_GetParameterValueFromString - Failed: SEMI between EQUAL and parmValue"));
            return 0;
        }
        pParm = pText + num_spaces;
    }
    else
    {
        /* 2b) Find pParm (start of header-value) */
        char save;
        num_spaces = strspn( string, delimiters );
        pParm = string + num_spaces;
        save = *pParm;
        *(char *)pParm = '\0';          // temporary end of string
        pText = strrchr( string, ';' ); // last SEMI before header-value
        *(char *)pParm = save;          // restore header-value
        if (pText)
        {
            RvLogError(msg->pMsgMgr->pLogSrc, (msg->pMsgMgr->pLogSrc,
                       "SPIRENT_RvSIP_GetParameterValueFromString - Failed: SEMI before header-value"));
            return 0;
        }
        pName = strchr( pParm, ';' );   // first SEMI after header-value
    }

    /* 3) Find parmValue length */
    pText = strpbrk( pParm, delimiters );
    if (pText)
    {
        length = (size_t)(pText - pParm);
        num_spaces = strspn( pText, delimiters );
        if ( pText[num_spaces] != '\0' && !(pName && (pText + num_spaces) > pName) )
        {
            RvLogError(msg->pMsgMgr->pLogSrc, (msg->pMsgMgr->pLogSrc,
                       "SPIRENT_RvSIP_GetParameterValueFromString - Failed: No SEMI between parmValue and next parameter-header"));
            return 0;
        }
    }
    else
    {
        length = strlen( pParm );
    }

    /* 4) Copy parmValue into the buffer */
    if ( length >= parmBufferSize )
    {
        RvLogError(msg->pMsgMgr->pLogSrc, (msg->pMsgMgr->pLogSrc,
                   "SPIRENT_RvSIP_GetParameterValueFromString - Failed: parmValue Oversizes the Buffer"));
        return 0;
    }
    strncpy( parmValue, pParm, length );
    parmValue[length] = '\0';

    return length;
}

#endif //defined(UPDATED_BY_SPIRENT_ABACUS)

