#if defined(RV_SIP_AUTH_ON) && defined(RV_SIP_PRIMITIVES) && !defined(RV_SIP_IMS_DCS_HEADER_SUPPORT) && !defined(RV_SIP_IMS_HEADER_SUPPORT) && !defined(RV_SIP_EXTENDED_HEADER_SUPPORT) && !defined(RV_SIP_JSR32_SUPPORT) && !defined(RV_SIP_LIGHT) && defined(RV_SIP_TEL_URI_SUPPORT) && !defined(RV_SIP_SUBS_ON)
/*
 AnaGram Parsing Engine
 Copyright (c) 1993-1999, Parsifal Software.
 All Rights Reserved.
 Serial number 2P20671
 Registered to:
   Meir Fuchs
   RADVision Ltd.
*/
#ifndef PARSERENGINESIPPRIMITIVES_H_1248033508
#include ".\ParserEngineSipPrimitives.h"
#endif
#ifndef PARSERENGINESIPPRIMITIVES_H_1248033508
#error Mismatched header file
#endif
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define RULE_CONTEXT (&((PCB).cs[(PCB).ssx]))
#define ERROR_CONTEXT ((PCB).cs[(PCB).error_frame_ssx])
#define CONTEXT ((PCB).cs[(PCB).ssx])
#ifndef PCB_TYPE
#define PCB_TYPE SipParser_pcb_type
#endif
#define PCB (*pcb_pointer)
#define PCB_DECL PCB_TYPE *pcb_pointer
#define PCB_POINTER pcb_pointer
#ifndef CONVERT_CASE
static const char agCaseTable[31] = {
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,    0,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
};
static int agConvertCase(int c) {
  if (c >= 'a' && c <= 'z') return c ^= 0x20;
  if (c >= 0xe0 && c < 0xff) c ^= agCaseTable[c-0xe0];
  return c;
}
#define CONVERT_CASE(c) agConvertCase(c)
#endif
#ifndef TAB_SPACING
#define TAB_SPACING 8
#endif
#define ag_rp_1(PCB_POINTER) (SET_HEADER_VALUE())
#define ag_rp_2(PCB_POINTER) (SIP_BEGIN_TOKEN())
#define ag_rp_3(PCB_POINTER) (SIP_BEGIN_TOKEN())
static void ag_rp_4(PCB_DECL) {
/* Line 318, RV SIP Stack */
        SIP_BEGIN_TOKEN();
        REMOVE_COMMA_BETWEEN_HEADERS();
    
}
#define ag_rp_5(PCB_POINTER) (SIP_BEGIN_TOKEN())
#define ag_rp_6(PCB_POINTER) (SIP_BEGIN_TOKEN())
#define ag_rp_7(PCB_POINTER) (SIP_BEGIN_TOKEN())
#define ag_rp_8(PCB_POINTER) (SIP_BEGIN_TOKEN())
#define ag_rp_9(PCB_POINTER) (SIP_BEGIN_TOKEN())
#define ag_rp_10(PCB_POINTER) (SIP_BEGIN_TOKEN())
#define ag_rp_11(PCB_POINTER) (SIP_BEGIN_TOKEN())
static void ag_rp_12(PCB_DECL) {
/* Line 364, RV SIP Stack */
                PCB.isWithinAngleBrackets = RV_TRUE;
                SIP_BEGIN_TOKEN();
            
}
static void ag_rp_13(PCB_DECL) {
/* Line 371, RV SIP Stack */
                PCB.isWithinAngleBrackets = RV_FALSE;
            
}
#define ag_rp_14(PCB_POINTER) (SIP_BEGIN_TOKEN())
#define ag_rp_15(PCB_POINTER) (SIP_BEGIN_TOKEN())
#define ag_rp_16(PCB_POINTER) (CUR_STRING())
#define ag_rp_17(PCB_POINTER) (CUR_STRING())
#define ag_rp_18(PCB_POINTER) (CUR_STRING())
#define ag_rp_19(PCB_POINTER) (CUR_STRING())
#define ag_rp_20(PCB_POINTER) (CUR_STRING())
#define ag_rp_21(PCB_POINTER) (CUR_STRING())
#define ag_rp_22(PCB_POINTER) (CUR_METHOD(PARSER_METHOD_TYPE_INVITE))
#define ag_rp_23(PCB_POINTER) (CUR_METHOD(PARSER_METHOD_TYPE_ACK))
#define ag_rp_24(PCB_POINTER) (CUR_METHOD(PARSER_METHOD_TYPE_BYE))
#define ag_rp_25(PCB_POINTER) (CUR_METHOD(PARSER_METHOD_TYPE_CANCEL))
#define ag_rp_26(PCB_POINTER) (CUR_METHOD(PARSER_METHOD_TYPE_REGISTER))
static ParserMethod ag_rp_27(PCB_DECL, ParserBasicToken t) {
/* Line 515, RV SIP Stack */
 ParserMethod method;
                            method.type = PARSER_METHOD_TYPE_OTHER;
                            method.other = t;
                            RV_UNUSED_ARG(pcb_pointer);
                            return method;
                          
}
static void ag_rp_28(PCB_DECL, ParserCSeq cseq) {
/* Line 531, RV SIP Stack */
   
       RvStatus status;
       status = ParserInitCSeq(PCB.pParserMgr, PCB.pointer,&cseq,PCB.eHeaderType,PCB.pSipObject);
       if(RV_ERROR_ILLEGAL_SYNTAX == status)
       {
           PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
           SYNTAX_ERROR;
       }
       else if (RV_OK != status)
       {
           
           PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
           PCB.eStat = status;
       }
    
}
static ParserCSeq ag_rp_29(PCB_DECL, ParserSequenceNumber sequenceNumber, ParserMethod method) {
/* Line 552, RV SIP Stack */
        ParserCSeq cseq;
        cseq.method = method;
		cseq.sequenceNumber = sequenceNumber;
        RV_UNUSED_ARG(pcb_pointer);
        return(cseq);
	 
}
#define ag_rp_30(PCB_POINTER) (CUR_STRING())
static void ag_rp_31(PCB_DECL, ParserContentLength contentLength) {
/* Line 574, RV SIP Stack */
          RvStatus status;
          contentLength.isCompact = PCB.isCompactForm;
          status = ParserInitContentLength(PCB.pParserMgr,PCB.pointer,&contentLength,PCB.pSipObject);
          if(RV_ERROR_ILLEGAL_SYNTAX == status)
          {
              PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
              SYNTAX_ERROR;
          }
          else if (RV_OK != status)
          {
            
            PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
            PCB.eStat = status;
          }
     
}
static ParserContentLength ag_rp_32(PCB_DECL) {
/* Line 597, RV SIP Stack */
        ParserContentLength contentLength;
        contentLength.contentLenVal = CUR_STRING();
        contentLength.isCompact = RV_FALSE;
        return contentLength;
    
}
static void ag_rp_33(PCB_DECL, ParserCallId callId) {
/* Line 613, RV SIP Stack */
  
          RvStatus status;
          callId.isCompact = PCB.isCompactForm;
          status = ParserInitCallId(PCB.pParserMgr,PCB.pointer,&callId,PCB.pSipObject);
          if(RV_ERROR_ILLEGAL_SYNTAX == status)
          {
              PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
              SYNTAX_ERROR;
          }
          else if(RV_OK != status)
          {
            
            PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
            PCB.eStat = status;
          }
    
}
static ParserCallId ag_rp_34(PCB_DECL, ParserBasicToken token) {
/* Line 635, RV SIP Stack */
         ParserCallId callId;
         callId.callIdVal = token;
         callId.isCompact = PCB.isCompactForm;
         return callId;
    
}
static ParserCallId ag_rp_35(PCB_DECL, ParserBasicToken token, ParserBasicToken token2) {
/* Line 642, RV SIP Stack */
        ParserCallId callId;
        token.len = (token2.len +token.len + 1);
        callId.callIdVal = token;
        callId.isCompact = PCB.isCompactForm;
        return callId;
    
}
static void ag_rp_36(PCB_DECL) {
/* Line 660, RV SIP Stack */
        RvStatus status;
        PCB.contentType.isCompactForm = PCB.isCompactForm;
        status = ParserInitContentType(PCB.pParserMgr,PCB.pointer,&(PCB.contentType),PCB.pSipObject);
        if(RV_ERROR_ILLEGAL_SYNTAX == status)
        {
             PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
             SYNTAX_ERROR;
        }
        else if (RV_OK != status) 
        {
            
            PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
            PCB.eStat = status;
        }
        ParserCleanExtParams (PCB.pExtParams);
		ParserCleanExtParams (PCB.pUrlExtParams);
     
}
static void ag_rp_37(PCB_DECL, ParserMediaType t, ParserMediaSubType st) {
/* Line 689, RV SIP Stack */
 
        PCB.contentType.mediaType    = t;
        PCB.contentType.mediaSubType = st;
        PCB.contentType.genericParamList = NULL;
        
        if (0 != ((ParserExtensionString *)PCB.pExtParams)->size)
        {
             PCB.contentType.genericParamList=PCB.pExtParams;
        }
    
}
static void ag_rp_38(PCB_DECL) {
/* Line 702, RV SIP Stack */
 
               RV_UNUSED_ARG(pcb_pointer);
            
}
static void ag_rp_39(PCB_DECL) {
/* Line 766, RV SIP Stack */
        
        
        RV_UNUSED_ARG(pcb_pointer);
    
}
#define ag_rp_40(PCB_POINTER, t) (t)
#define ag_rp_41(PCB_POINTER, t) (t)
static void ag_rp_42(PCB_DECL, ParserBasicToken token1, ParserGenericParamDataVal token2) {
/* Line 931, RV SIP Stack */
      if (0!=((ParserExtensionString *)PCB.pExtParams)->size)
      {
          
          APPEND_DATA(";", 1,PCB.pExtParams);
          APPEND_DATA(token1.buf, token1.len,PCB.pExtParams);
          APPEND_DATA("=" , 1,PCB.pExtParams);
          APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);            
      }
      else
      {      
          
          APPEND_DATA(token1.buf, token1.len,PCB.pExtParams);
          APPEND_DATA("=" , 1,PCB.pExtParams);
          APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);            
      }
   
}
#define ag_rp_43(PCB_POINTER, token) (token)
#define ag_rp_44(PCB_POINTER, qs) (qs)
static void ag_rp_45(PCB_DECL, ParserBasicToken token) {
/* Line 1026, RV SIP Stack */
      if (0!=((ParserExtensionString *)PCB.pExtParams)->size)
      {
          
          APPEND_DATA(";", 1,PCB.pExtParams);
          APPEND_DATA(token.buf,token.len, PCB.pExtParams);
      }
      else
      {      
          
          APPEND_DATA(token.buf,token.len,PCB.pExtParams);
      }
    
}
static void ag_rp_46(PCB_DECL, ParserBasicToken token1, ParserGenericParamDataVal token2) {
/* Line 1040, RV SIP Stack */
        if (0!=((ParserExtensionString *)PCB.pExtParams)->size)
        {
          
          APPEND_DATA(";", 1,PCB.pExtParams);
          APPEND_DATA(token1.buf, token1.len,PCB.pExtParams);
          APPEND_DATA("=" , 1,PCB.pExtParams);
          APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);            
        }
        else
        {      
          
          APPEND_DATA(token1.buf, token1.len,PCB.pExtParams);
          APPEND_DATA("=" , 1,PCB.pExtParams);
          APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);            
        }
    
}
#define ag_rp_47(PCB_POINTER, token) (token)
#define ag_rp_48(PCB_POINTER, qs) (qs)
static void ag_rp_49(PCB_DECL, ParserSipVersion ver, ParserStatusCode status, ParserReasonPhrase reason) {
/* Line 1074, RV SIP Stack */
        RvStatus        stat;
        ParserStatusLine statusLine;
        statusLine.sipVersion = ver;
        statusLine.statusCode = status;
        statusLine.reasonPhrase = reason;
        stat = ParserInitStatusLine(PCB.pParserMgr,&PCB, PCB.pointer, &statusLine, PCB.pSipObject);
        if (RV_ERROR_ILLEGAL_SYNTAX == stat)
        {
            PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
            SYNTAX_ERROR;
        }
        else if (RV_OK != stat)
        {
            
            PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
            PCB.eStat = stat;
        }
    
}
static void ag_rp_50(PCB_DECL, ParserSipVersion ver, ParserStatusCode status) {
/* Line 1094, RV SIP Stack */
        RvStatus        stat;
        ParserStatusLine statusLine;
        statusLine.sipVersion = ver;
        statusLine.statusCode = status;       
        statusLine.reasonPhrase.buf=NULL;
        statusLine.reasonPhrase.len=0;
        stat = ParserInitStatusLine(PCB.pParserMgr,&PCB, PCB.pointer, &statusLine, PCB.pSipObject);
        if(RV_ERROR_ILLEGAL_SYNTAX == stat)
        {
            PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
            SYNTAX_ERROR;
        }
        else if (RV_OK != stat)
        {
            
            PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
            PCB.eStat = stat;
        }
    
}
#define ag_rp_51(PCB_POINTER) (CUR_STRING())
#define ag_rp_52(PCB_POINTER) (CUR_STRING())
static ParserStatusCode ag_rp_53(PCB_DECL, int D100, int D10, int D1) {
/* Line 1132, RV SIP Stack */
        RvUint16 code;
        RV_UNUSED_ARG(pcb_pointer);
        code =  (RvUint16)((D100 - '0')*100 + (D10 - '0')*10 + (D1 - '0'));
        return code;
    
}
static void ag_rp_54(PCB_DECL) {
/* Line 1196, RV SIP Stack */
 
       RvStatus status;
       PCB.singleVia.isCompact    = PCB.isCompactForm;
       status = ParserInitVia(PCB.pParserMgr, &PCB, PCB.pointer, &(PCB.singleVia),PCB.eHeaderType, PCB.pSipObject);
       if(RV_ERROR_ILLEGAL_SYNTAX == status)
       {
           PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
           SYNTAX_ERROR;
       }
       else if (RV_OK != status && RV_ERROR_NOTSUPPORTED != status)
       {
            
            PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
            PCB.eStat = status;
       }
        
       ParserCleanExtParams (PCB.pExtParams);
    
}
static void ag_rp_55(PCB_DECL) {
/* Line 1216, RV SIP Stack */
 
       RvStatus status;
       PCB.singleVia.isCompact    = PCB.isCompactForm;
       status = ParserInitVia(PCB.pParserMgr,&PCB, PCB.pointer,&(PCB.singleVia),PCB.eHeaderType, PCB.pSipObject);
       if(RV_ERROR_ILLEGAL_SYNTAX == status)
       {
           PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
           SYNTAX_ERROR;
       }
       else if (RV_OK != status && RV_ERROR_NOTSUPPORTED != status)
       {
            
            PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
            PCB.eStat = status;
       }
        
       ParserCleanExtParams (PCB.pExtParams);
    
}
static void ag_rp_56(PCB_DECL) {
/* Line 1237, RV SIP Stack */
 
        PCB.singleVia.sentProtocol = PCB.sentProtocol;
        PCB.singleVia.sentBy       = PCB.viaSentBy;
         
        if (((ParserExtensionString *)PCB.pExtParams)->size !=0)
        {
            if (PCB.singleVia.params.isOtherParams==RV_FALSE)
            {
                PCB.singleVia.params.isOtherParams = RV_TRUE;
                PCB.singleVia.params.otherParam = PCB.pExtParams;
            }
        }                
    
}
static void ag_rp_57(PCB_DECL) {
/* Line 1254, RV SIP Stack */
 
        PCB.singleVia.params.hidden     = RV_FALSE;
        PCB.singleVia.params.alias     = RV_FALSE;
        PCB.singleVia.params.isTtl      = RV_FALSE;
        PCB.singleVia.params.isMaddr    = RV_FALSE;
	    PCB.singleVia.params.isReceived = RV_FALSE;
        PCB.singleVia.params.isBranch   = RV_FALSE;
        PCB.singleVia.params.isOtherParams=RV_FALSE;  
        PCB.singleVia.params.isRport	= RV_FALSE;
        PCB.singleVia.params.isComp     = RV_FALSE;
	PCB.singleVia.params.isSigCompId= RV_FALSE;
    
}
static void ag_rp_58(PCB_DECL, ParserViaParam p) {
/* Line 1269, RV SIP Stack */
        switch( p.paramType )
        {
            case PARSER_PARAM_TYPE_HIDDEN :
                PCB.singleVia.params.hidden = RV_TRUE; 
                break;
            case PARSER_PARAM_TYPE_ALIAS:
                PCB.singleVia.params.alias = RV_TRUE; 
                break;
            case PARSER_PARAM_TYPE_TTL :
                PCB.singleVia.params.isTtl = RV_TRUE;
                PCB.singleVia.params.ttl   = p.param.ttl;
                break;
            
            case PARSER_PARAM_TYPE_MADDR :
                PCB.singleVia.params.isMaddr = RV_TRUE;
                PCB.singleVia.params.maddr   = p.param.maddr;
                break;
            
            case PARSER_PARAM_TYPE_RECEIVED :
                PCB.singleVia.params.isReceived = RV_TRUE;
                PCB.singleVia.params.received   = p.param.received;
                break;
            
            case PARSER_PARAM_TYPE_BRANCH :
                PCB.singleVia.params.isBranch = RV_TRUE;
                PCB.singleVia.params.branch   = p.param.branch;
                break;
            case PARSER_PARAM_TYPE_RPORT:
                PCB.singleVia.params.isRport = RV_TRUE;
                PCB.singleVia.params.rPort   = p.param.rport;
                break;
                 
            case PARSER_PARAM_TYPE_COMP :
                PCB.singleVia.params.isComp  = RV_TRUE;
                PCB.singleVia.params.comp    = p.param.comp;
                break;   
	case PARSER_PARAM_TYPE_SIGCOMPID:
		PCB.singleVia.params.isSigCompId = RV_TRUE;
		PCB.singleVia.params.sigCompId   = p.param.sigCompId;
		break;
            default:
                if (p.isOther == RV_TRUE)
                {
                     PCB.singleVia.params.isOtherParams = RV_TRUE;
                     PCB.singleVia.params.otherParam   = p.otherParam;
                }                   
                break;
        }       
    
}
static void ag_rp_59(PCB_DECL, ParserProtocolName pn, ParserProtocolVersion pv, ParserTransport t) {
/* Line 1327, RV SIP Stack */
 
        PCB.sentProtocol.protocolName    = pn;
        PCB.sentProtocol.protocolVersion = pv;
        PCB.sentProtocol.transport       = t;
    
}
#define ag_rp_60(PCB_POINTER, t) (t)
#define ag_rp_61(PCB_POINTER) (CUR_STRING())
#define ag_rp_62(PCB_POINTER, t) (t)
#define ag_rp_63(PCB_POINTER, t) (t)
static ParserTransport ag_rp_64(PCB_DECL) {
/* Line 1354, RV SIP Stack */
        ParserTransport t;
        t.transport    = RVSIP_TRANSPORT_UDP;
        t.strTransport = CUR_STRING();
        return t;
    
}
static ParserTransport ag_rp_65(PCB_DECL) {
/* Line 1361, RV SIP Stack */
        ParserTransport t;
        t.transport    = RVSIP_TRANSPORT_TCP;
        t.strTransport = CUR_STRING();
        return t;
    
}
static ParserTransport ag_rp_66(PCB_DECL) {
/* Line 1368, RV SIP Stack */
        ParserTransport t;
        t.transport    = RVSIP_TRANSPORT_SCTP;
        t.strTransport = CUR_STRING();
        return t;
    
}
static ParserTransport ag_rp_67(PCB_DECL) {
/* Line 1375, RV SIP Stack */
        ParserTransport t;
        t.transport    = RVSIP_TRANSPORT_TLS;
        t.strTransport = CUR_STRING();
        return t;
    
}
static ParserTransport ag_rp_68(PCB_DECL, ParserBasicToken token) {
/* Line 1382, RV SIP Stack */
        ParserTransport t;
        t.transport    = RVSIP_TRANSPORT_OTHER;
        t.strTransport = token;
        RV_UNUSED_ARG(pcb_pointer);
        return t;
    
}
#define ag_rp_69(PCB_POINTER) (CUR_STRING())
static void ag_rp_70(PCB_DECL, ParserBasicToken host) {
/* Line 1401, RV SIP Stack */
 
        
        PCB.viaSentBy.isPort   = RV_FALSE;
        PCB.viaSentBy.hostName = host;
     
}
static void ag_rp_71(PCB_DECL, ParserBasicToken host, ParserPort port) {
/* Line 1407, RV SIP Stack */
 
        
        PCB.viaSentBy.isPort   = RV_TRUE;
        PCB.viaSentBy.hostName = host;
        PCB.viaSentBy.port     = port;
      
}
static ParserViaParam ag_rp_72(PCB_DECL) {
/* Line 1421, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_HIDDEN;                   
          viaParam.param.hidden = RV_TRUE;
	      viaParam.isOther = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_73(PCB_DECL) {
/* Line 1430, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_ALIAS;                   
          viaParam.param.alias = RV_TRUE;
	      viaParam.isOther = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_74(PCB_DECL, ParserTTLParam t) {
/* Line 1439, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_TTL;                   
          viaParam.param.ttl = t;
	      viaParam.isOther = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_75(PCB_DECL, ParserMaddrParam ma) {
/* Line 1448, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_MADDR;                   
          viaParam.param.maddr = ma;
	      viaParam.isOther = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_76(PCB_DECL, ParserReceived r) {
/* Line 1457, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_RECEIVED;                   
          viaParam.param.received = r;
	      viaParam.isOther = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_77(PCB_DECL, ParserBranch b) {
/* Line 1466, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_BRANCH;                   
          viaParam.param.branch = b;
	      viaParam.isOther = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_78(PCB_DECL, ParserPort rport) {
/* Line 1476, RV SIP Stack */
          ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_RPORT;                   
          viaParam.param.rport = rport;
	      viaParam.isOther = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_79(PCB_DECL, ParserCompParam comp) {
/* Line 1485, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType  = PARSER_PARAM_TYPE_COMP;                   
          viaParam.param.comp = comp;
	      viaParam.isOther    = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_80(PCB_DECL, ParserSigCompIdParam sigcompid) {
/* Line 1495, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType  = PARSER_PARAM_TYPE_SIGCOMPID;                   
          viaParam.param.sigCompId = sigcompid;
	      viaParam.isOther    = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_81(PCB_DECL) {
/* Line 1505, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_OTHER; 
	      viaParam.isOther = RV_TRUE;
          viaParam.otherParam = PCB.pExtParams;
          return viaParam;    
        
}
static void ag_rp_82(PCB_DECL, ParserBasicToken token) {
/* Line 1522, RV SIP Stack */
        if (((ParserExtensionString *)PCB.pExtParams)->size !=0)
        {
            
            APPEND_DATA(";", 1, PCB.pExtParams);
            APPEND_DATA(token.buf, token.len,PCB.pExtParams);
        }
        else
        {      
            
            APPEND_DATA(token.buf, token.len,PCB.pExtParams);
        }
    
}
static void ag_rp_83(PCB_DECL, ParserBasicToken token, ParserBasicToken token2) {
/* Line 1537, RV SIP Stack */
        if (0!=((ParserExtensionString *)PCB.pExtParams)->size)
        {
            
            APPEND_DATA(";", 1,PCB.pExtParams);
            APPEND_DATA(token.buf, token.len,PCB.pExtParams);
            APPEND_DATA("=", 1,PCB.pExtParams);
            APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);
        }
        else
        {      
            
            APPEND_DATA(token.buf, token.len,PCB.pExtParams);
            APPEND_DATA("=", 1,PCB.pExtParams);
            APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);
        }
    
}
static void ag_rp_84(PCB_DECL, ParserBasicToken token, ParserQuotedString token2) {
/* Line 1556, RV SIP Stack */
        if (((ParserExtensionString *)PCB.pExtParams)->size !=0)
        {
            
            APPEND_DATA(";", 1,PCB.pExtParams);
            APPEND_DATA(token.buf, token.len,PCB.pExtParams);
            APPEND_DATA("=", 1,PCB.pExtParams);
            APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);
        }
        else
        {      
            
            APPEND_DATA(token.buf, token.len,PCB.pExtParams);
            APPEND_DATA("=", 1,PCB.pExtParams);
            APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);
        }
    
}
static void ag_rp_85(PCB_DECL, ParserBasicToken token, ParserBasicToken token2) {
/* Line 1579, RV SIP Stack */
        if (((ParserExtensionString *)PCB.pExtParams)->size !=0)
        {
            
            APPEND_DATA(";", 1,PCB.pExtParams);
            APPEND_DATA(token.buf, token.len,PCB.pExtParams);
            APPEND_DATA("=", 1,PCB.pExtParams);
            APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);
        }
        else
        {      
            
            APPEND_DATA(token.buf, token.len,PCB.pExtParams);
            APPEND_DATA("=", 1,PCB.pExtParams);
            APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);
        }
    
}
#define ag_rp_86(PCB_POINTER, t) (t)
#define ag_rp_87(PCB_POINTER, ttl) (ttl)
#define ag_rp_88(PCB_POINTER) (CUR_STRING())
#define ag_rp_89(PCB_POINTER) (CUR_STRING())
#define ag_rp_90(PCB_POINTER) (CUR_STRING())
#define ag_rp_91(PCB_POINTER, rport) (rport)
static ParserPort ag_rp_92(PCB_DECL) {
/* Line 1634, RV SIP Stack */
        ParserPort port;
        port.len = 0;
		port.buf = NULL;
        RV_UNUSED_ARG(pcb_pointer);
        return port;
    
}
#define ag_rp_93(PCB_POINTER, port) (port)
#define ag_rp_94(PCB_POINTER, ma) (ma)
#define ag_rp_95(PCB_POINTER, host) (host)
#define ag_rp_96(PCB_POINTER, host) (host)
#define ag_rp_97(PCB_POINTER, t) (t)
#define ag_rp_98(PCB_POINTER, comp) (comp)
static ParserCompParam ag_rp_99(PCB_DECL) {
/* Line 1671, RV SIP Stack */
        ParserCompParam comp;
        comp.compType     = RVSIP_COMP_SIGCOMP;
        comp.strCompParam = CUR_STRING();
        return comp;
    
}
static ParserCompParam ag_rp_100(PCB_DECL, ParserBasicToken token) {
/* Line 1678, RV SIP Stack */
        ParserCompParam comp;
        comp.compType     = RVSIP_COMP_OTHER;
        comp.strCompParam = CUR_STRING();
        RV_UNUSED_ARG(token.buf);
        return comp;
    
}
#define ag_rp_101(PCB_POINTER, sigCompId) (sigCompId)
#define ag_rp_102(PCB_POINTER) (CUR_STRING())
static void ag_rp_103(PCB_DECL, ParserAbsoluteUri uri) {
/* Line 1927, RV SIP Stack */
       RvStatus status;
       status = ParserInitAbsUri(PCB.pParserMgr, &PCB, &uri, PCB.eHeaderType,PCB.pSipObject);
       if(RV_ERROR_ILLEGAL_SYNTAX == status)
       {
           PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
           SYNTAX_ERROR;
       }
       else if (status != RV_OK)
       {
            
            PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
            PCB.eStat = status;
       }
    
}
static ParserAbsoluteUri ag_rp_104(PCB_DECL, ParserBasicToken scheme, ParserBasicToken id) {
/* Line 1947, RV SIP Stack */
        ParserAbsoluteUri uri;
        uri.scheme = scheme;
        uri.idefntifier = id;
        RV_UNUSED_ARG(pcb_pointer);
        return uri;
    
}
#define ag_rp_105(PCB_POINTER) (CUR_STRING())
#define ag_rp_106(PCB_POINTER, displayName) (displayName)
#define ag_rp_107(PCB_POINTER) (CUR_STRING())
#define ag_rp_108(PCB_POINTER) (CUR_STRING())
static void ag_rp_109(PCB_DECL) {
/* Line 1988, RV SIP Stack */
        RvStatus status;
        status = ParserInitTelUri(PCB.pParserMgr, &PCB, &(PCB.telUri), PCB.eHeaderType,PCB.pSipObject);
        if(RV_ERROR_ILLEGAL_SYNTAX == status)
        {
            PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
            SYNTAX_ERROR;
        }
        else if (RV_OK != status)
        {
            
            PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
            PCB.eStat = status;
        }
    
}
static void ag_rp_110(PCB_DECL) {
/* Line 2042, RV SIP Stack */
		SIP_BEGIN_TOKEN();
	
}
static void ag_rp_111(PCB_DECL) {
/* Line 2047, RV SIP Stack */
  
         
         if (0 != ((ParserExtensionString *)PCB.pUrlExtParams)->size)
         {
              PCB.telUri.strOtherParams.Params         = PCB.pUrlExtParams;
              PCB.telUri.strOtherParams.bIsSpecified   = RV_TRUE;
         }          
  	
}
static void ag_rp_112(PCB_DECL, ParserBasicToken num) {
/* Line 2059, RV SIP Stack */
		PCB.telUri.strPhoneNumber.bIsSpecified = RV_TRUE;
		PCB.telUri.strPhoneNumber.strToken     = num;
		PCB.telUri.bIsGlobalPhoneNumber        = RV_TRUE;
	
}
static void ag_rp_113(PCB_DECL, ParserBasicToken num) {
/* Line 2065, RV SIP Stack */
		PCB.telUri.strPhoneNumber.bIsSpecified = RV_TRUE;
		PCB.telUri.strPhoneNumber.strToken     = num;
		PCB.telUri.bIsGlobalPhoneNumber        = RV_FALSE;
	
}
static ParserBasicToken ag_rp_114(PCB_DECL) {
/* Line 2075, RV SIP Stack */
		ParserBasicToken val;
		
		val.buf = SIP_TOKEN_START+1;
		val.len = SIP_TOKEN_LENGTH-1;  
		return val;
	
}
#define ag_rp_115(PCB_POINTER) (CUR_STRING())
static void ag_rp_116(PCB_DECL) {
/* Line 2104, RV SIP Stack */
 
		PCB.telUri.strExtension.bIsSpecified   = RV_FALSE;
		PCB.telUri.strPostDial.bIsSpecified    = RV_FALSE;
		PCB.telUri.strIsdnSubAddr.bIsSpecified = RV_FALSE;
		PCB.telUri.strContext.bIsSpecified     = RV_FALSE;
		PCB.telUri.strOtherParams.bIsSpecified = RV_FALSE;
		PCB.telUri.eEnumdiType                 = ParserEnumdiParamUndefined;
    
}
static void ag_rp_117(PCB_DECL) {
/* Line 2126, RV SIP Stack */
 
        PCB.telUri.strExtension.bIsSpecified   = RV_FALSE;
		PCB.telUri.strPostDial.bIsSpecified    = RV_FALSE;
		PCB.telUri.strIsdnSubAddr.bIsSpecified = RV_FALSE;
		PCB.telUri.strContext.bIsSpecified     = RV_FALSE;
		PCB.telUri.strOtherParams.bIsSpecified = RV_FALSE;
		PCB.telUri.eEnumdiType                 = ParserEnumdiParamUndefined;
         								
        switch(PCB.telUriParameter.paramType)
        {
            case PARSER_TEL_URI_EXTENSION_PARAM: 
	            PCB.telUri.strExtension.bIsSpecified        = RV_TRUE;
	            PCB.telUri.strExtension.strToken            = PCB.telUriParameter.param.strExtension; 
	            break;
            case PARSER_TEL_URI_POST_DIAL_PARAM: 
	            PCB.telUri.strPostDial.bIsSpecified         = RV_TRUE;
	            PCB.telUri.strPostDial.strToken             = PCB.telUriParameter.param.strPostDial; 
	            break;
            case PARSER_TEL_URI_ISDN_SUB_ADDR_PARAM: 
	            PCB.telUri.strIsdnSubAddr.bIsSpecified      = RV_TRUE;
	            PCB.telUri.strIsdnSubAddr.strToken          = PCB.telUriParameter.param.strIsdnSubAddr; 
	            break;
            case PARSER_TEL_URI_CONTEXT_PARAM: 
	            PCB.telUri.strContext.bIsSpecified          = RV_TRUE;
	            PCB.telUri.strContext.strToken              = PCB.telUriParameter.param.strContext; 
	            break;
			case PARSER_TEL_URI_ENUMDI_PARAM: 
				PCB.telUri.eEnumdiType                      = PCB.telUriParameter.param.eEnumdiType;
                break;
            case PARSER_TEL_URI_OTHER_PARAM:    
	            break;
             default:
                 PCB.exit_flag = AG_SEMANTIC_ERROR_CODE;
                 PCB.eStat = RV_ERROR_ILLEGAL_SYNTAX;            
        } 	 														
    
}
static void ag_rp_118(PCB_DECL) {
/* Line 2196, RV SIP Stack */
 
         
        switch(PCB.telUriParameter.paramType)
        {
			case PARSER_TEL_URI_EXTENSION_PARAM: 
	            PCB.telUri.strExtension.bIsSpecified        = RV_TRUE;
	            PCB.telUri.strExtension.strToken            = PCB.telUriParameter.param.strExtension; 
	            break;
            case PARSER_TEL_URI_POST_DIAL_PARAM: 
	            PCB.telUri.strPostDial.bIsSpecified         = RV_TRUE;
	            PCB.telUri.strPostDial.strToken             = PCB.telUriParameter.param.strPostDial; 
	            break;
            case PARSER_TEL_URI_ISDN_SUB_ADDR_PARAM: 
	            PCB.telUri.strIsdnSubAddr.bIsSpecified      = RV_TRUE;
	            PCB.telUri.strIsdnSubAddr.strToken          = PCB.telUriParameter.param.strIsdnSubAddr; 
	            break;
            case PARSER_TEL_URI_CONTEXT_PARAM: 
	            PCB.telUri.strContext.bIsSpecified          = RV_TRUE;
	            PCB.telUri.strContext.strToken              = PCB.telUriParameter.param.strContext; 
	            break;
            case PARSER_TEL_URI_ENUMDI_PARAM: 
				PCB.telUri.eEnumdiType                      = PCB.telUriParameter.param.eEnumdiType;
                break;
            case PARSER_TEL_URI_OTHER_PARAM:    
                PCB.telUri.strOtherParams.bIsSpecified      = RV_TRUE;
		        break; 
            default:
                PCB.exit_flag = AG_SEMANTIC_ERROR_CODE;
                PCB.eStat = RV_ERROR_ILLEGAL_SYNTAX;                  
        }	 
    
}
static void ag_rp_119(PCB_DECL, ParserBasicToken ext) {
/* Line 2256, RV SIP Stack */
 
         PCB.telUriParameter.paramType = PARSER_TEL_URI_EXTENSION_PARAM;
         PCB.telUriParameter.param.strExtension = ext;
    
}
static void ag_rp_120(PCB_DECL, ParserBasicToken pstd) {
/* Line 2261, RV SIP Stack */
 
		PCB.telUriParameter.paramType = PARSER_TEL_URI_POST_DIAL_PARAM;
		PCB.telUriParameter.param.strPostDial = pstd;
	
}
static void ag_rp_121(PCB_DECL, ParserBasicToken isub) {
/* Line 2266, RV SIP Stack */
 
		PCB.telUriParameter.paramType = PARSER_TEL_URI_ISDN_SUB_ADDR_PARAM;
		PCB.telUriParameter.param.strIsdnSubAddr = isub;
	
}
static void ag_rp_122(PCB_DECL, ParserBasicToken cntx) {
/* Line 2271, RV SIP Stack */
 
		PCB.telUriParameter.paramType = PARSER_TEL_URI_CONTEXT_PARAM;
		PCB.telUriParameter.param.strContext = cntx;
	
}
static void ag_rp_123(PCB_DECL, ParserEnumdiParamType enumdi) {
/* Line 2276, RV SIP Stack */
 
		PCB.telUriParameter.paramType = PARSER_TEL_URI_ENUMDI_PARAM;
		PCB.telUriParameter.param.eEnumdiType   = enumdi;
	
}
static void ag_rp_124(PCB_DECL) {
/* Line 2282, RV SIP Stack */
	     PCB.telUriParameter.paramType = PARSER_TEL_URI_OTHER_PARAM;
    
}
#define ag_rp_125(PCB_POINTER, ext) (ext)
#define ag_rp_126(PCB_POINTER) (CUR_STRING())
#define ag_rp_127(PCB_POINTER, postd) (postd)
#define ag_rp_128(PCB_POINTER) (CUR_STRING())
#define ag_rp_129(PCB_POINTER, isub) (isub)
#define ag_rp_130(PCB_POINTER) (CUR_STRING())
#define ag_rp_131(PCB_POINTER, cntx) (cntx)
#define ag_rp_132(PCB_POINTER) (CUR_STRING())
static void ag_rp_133(PCB_DECL) {
/* Line 2392, RV SIP Stack */
		if (PCB.isTopLabel == RV_FALSE)
		{
			PCB.exit_flag = AG_SEMANTIC_ERROR_CODE;
			PCB.eStat = RV_ERROR_ILLEGAL_SYNTAX;
		}
	
}
static void ag_rp_134(PCB_DECL) {
/* Line 2402, RV SIP Stack */
		
		if (LAST_CHAR_IS_HYPHEN())
		{
			PCB.exit_flag = AG_SEMANTIC_ERROR_CODE;
			PCB.eStat = RV_ERROR_ILLEGAL_SYNTAX;
		}
		PCB.isTopLabel = RV_TRUE;
	
}
static void ag_rp_135(PCB_DECL) {
/* Line 2412, RV SIP Stack */
		
		if (LAST_CHAR_IS_HYPHEN())
		{
			PCB.exit_flag = AG_SEMANTIC_ERROR_CODE;
			PCB.eStat = RV_ERROR_ILLEGAL_SYNTAX;
		}
		PCB.isTopLabel = RV_FALSE;
	
}
#define ag_rp_136(PCB_POINTER) (ParserEnumdiParamEmpty)
static void ag_rp_137(PCB_DECL) {
/* Line 2567, RV SIP Stack */
        RvStatus status;
        status = ParserInitUrl(PCB.pParserMgr,&PCB,&(PCB.sipUrl), PCB.eHeaderType,PCB.pSipObject);
        if(RV_ERROR_ILLEGAL_SYNTAX == status)
        {
            PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
            SYNTAX_ERROR;
        }
        else if (RV_OK != status)
        {
            
            PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
            PCB.eStat = status;
        }
    
}
#define ag_rp_138(PCB_POINTER, uri) (INIT_PRES_IM_STRUCT(uri))
#define ag_rp_139(PCB_POINTER, uri) (INIT_PRES_IM_STRUCT(uri))
static void ag_rp_140(PCB_DECL) {
/* Line 2614, RV SIP Stack */
        PCB.sipUrl.bIsSecure = RV_FALSE;
        PCB.sipUrl.bOldAddrSpec = RV_FALSE;
        SIP_BEGIN_TOKEN();
        DEFINE_URL_PREFIX();
    
}
static void ag_rp_141(PCB_DECL) {
/* Line 2621, RV SIP Stack */
        PCB.sipUrl.bIsSecure = RV_TRUE;
        PCB.sipUrl.bOldAddrSpec = RV_FALSE;
        SIP_BEGIN_TOKEN();
        DEFINE_URL_PREFIX();
    
}
#define ag_rp_142(PCB_POINTER, uri) (uri)
static ParserAbsoluteUri ag_rp_143(PCB_DECL) {
/* Line 2635, RV SIP Stack */
        ParserAbsoluteUri uri;
        
        uri.scheme.len = 0;
		uri.scheme.buf = NULL;
        uri.idefntifier.len = 0;
		uri.idefntifier.buf = NULL;
        RV_UNUSED_ARG(pcb_pointer)
        return uri;
    
}
#define ag_rp_144(PCB_POINTER) (SET_PARSER_PCB_BACK_TO_BUFF(RV_SIPPARSER_PREFIX_RV_ABS))
static void ag_rp_145(PCB_DECL) {
/* Line 2651, RV SIP Stack */
        SIP_BEGIN_TOKEN();
        DEFINE_PRES_IM_PREFIX(RV_FALSE);
    
}
#define ag_rp_146(PCB_POINTER, uri) (uri)
static ParserAbsoluteUri ag_rp_147(PCB_DECL) {
/* Line 2661, RV SIP Stack */
        ParserAbsoluteUri uri;
        
        uri.scheme.len = 0;
        uri.scheme.buf = NULL;
        uri.idefntifier.len = 0;
		uri.idefntifier.buf = NULL;
        RV_UNUSED_ARG(pcb_pointer)
        return uri;
    
}
static void ag_rp_148(PCB_DECL) {
/* Line 2674, RV SIP Stack */
        SIP_BEGIN_TOKEN();
        DEFINE_PRES_IM_PREFIX(RV_TRUE);
    
}
static void ag_rp_149(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2682, RV SIP Stack */
  
           
         PCB.sipUrl.optionalHeaders = hs;
         
         if (0 != ((ParserExtensionString *)PCB.pUrlExtParams)->size)
         {
              PCB.sipUrl.urlParameters.genericParamList= PCB.pUrlExtParams;
              PCB.sipUrl.urlParameters.isOtherParams   = RV_TRUE;
         }          
  	
}
static void ag_rp_150(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2693, RV SIP Stack */
  
         
       PCB.sipUrl.optionalHeaders = hs;
       
       if (0 != ((ParserExtensionString *)PCB.pUrlExtParams)->size)
       {
            PCB.sipUrl.urlParameters.genericParamList=PCB.pUrlExtParams;
            PCB.sipUrl.urlParameters.isOtherParams   = RV_TRUE;
       }          
	
}
static void ag_rp_151(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2726, RV SIP Stack */
 
        RESET_URI_PARAMS();
        PCB.sipUrl.optionalHeaders.isSpecified = RV_FALSE;
        PCB.sipUrl.optionalHeaders = hs;
        PCB.exUri.uriType = PARSER_ADDR_PARAM_TYPE_SIP_URL;
        PCB.exUri.ExUriInfo.SipUrl = PCB.sipUrl;
      
}
static void ag_rp_152(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2735, RV SIP Stack */
 
        RESET_URI_PARAMS();
        PCB.sipUrl.optionalHeaders.isSpecified = RV_FALSE;
        PCB.sipUrl.optionalHeaders = hs;
        PCB.exUri.uriType = PARSER_ADDR_PARAM_TYPE_SIP_URL;
        PCB.exUri.ExUriInfo.SipUrl = PCB.sipUrl;
      
}
static void ag_rp_153(PCB_DECL) {
/* Line 2748, RV SIP Stack */
		PCB.telUri.strExtension.bIsSpecified   = RV_FALSE;
		PCB.telUri.strPostDial.bIsSpecified    = RV_FALSE;
		PCB.telUri.strIsdnSubAddr.bIsSpecified = RV_FALSE;
		PCB.telUri.strContext.bIsSpecified     = RV_FALSE;
		PCB.telUri.strOtherParams.bIsSpecified = RV_FALSE;
		
		PCB.exUri.uriType = PARSER_ADDR_PARAM_TYPE_TEL_URI;
        PCB.exUri.ExUriInfo.telUri = PCB.telUri;
	
}
static void ag_rp_154(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2775, RV SIP Stack */
 
        RESET_URI_PARAMS();
        PCB.sipUrl.optionalHeaders.isSpecified = RV_FALSE;
        PCB.sipUrl.optionalHeaders = hs;
        PCB.exUri.uriType = PARSER_ADDR_PARAM_TYPE_PRES_URI;
        PCB.exUri.ExUriInfo.SipUrl = PCB.sipUrl;
      
}
static void ag_rp_155(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2784, RV SIP Stack */
 
        RESET_URI_PARAMS();
        PCB.sipUrl.optionalHeaders.isSpecified = RV_FALSE;
        PCB.sipUrl.optionalHeaders = hs;
        
        PCB.exUri.uriType = PARSER_ADDR_PARAM_TYPE_PRES_URI;
        PCB.exUri.ExUriInfo.SipUrl = PCB.sipUrl;
      
}
static void ag_rp_156(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2796, RV SIP Stack */
 
        RESET_URI_PARAMS();
        PCB.sipUrl.optionalHeaders.isSpecified = RV_FALSE;
        PCB.sipUrl.optionalHeaders = hs;
        PCB.exUri.uriType = PARSER_ADDR_PARAM_TYPE_IM_URI;
        PCB.exUri.ExUriInfo.SipUrl = PCB.sipUrl;
      
}
static void ag_rp_157(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2805, RV SIP Stack */
 
        RESET_URI_PARAMS();
        PCB.sipUrl.optionalHeaders.isSpecified = RV_FALSE;
        PCB.sipUrl.optionalHeaders = hs;
        
        PCB.exUri.uriType = PARSER_ADDR_PARAM_TYPE_IM_URI;
        PCB.exUri.ExUriInfo.SipUrl = PCB.sipUrl;
      
}
static void ag_rp_158(PCB_DECL, ParserDisplayName scheme, ParserBasicToken id) {
/* Line 2819, RV SIP Stack */
          PCB.exUri.ExUriInfo.absUri.scheme = scheme;
          PCB.exUri.ExUriInfo.absUri.idefntifier = id;
          PCB.exUri.uriType = PARSER_ADDR_PARAM_TYPE_ABS_URI;
      
}
static void ag_rp_159(PCB_DECL) {
/* Line 2829, RV SIP Stack */
        PCB.sipUrl.bIsUserInfo = RV_TRUE;
    
}
static void ag_rp_160(PCB_DECL) {
/* Line 2834, RV SIP Stack */
        PCB.sipUrl.bIsUserInfo = RV_FALSE;
    
}
static void ag_rp_161(PCB_DECL) {
/* Line 2841, RV SIP Stack */
        SET_PARSER_PCB_BACK_TO_BUFF(RV_SIPPARSER_PREFIX_USERINFO);
    
}
static void ag_rp_162(PCB_DECL, ParserBasicToken user) {
/* Line 2847, RV SIP Stack */
        PCB.sipUrl.urlUserInfo.userName   = user;
        PCB.sipUrl.urlUserInfo.isPassword = RV_FALSE;
    
}
static void ag_rp_163(PCB_DECL, ParserBasicToken user, ParserBasicToken pass) {
/* Line 2852, RV SIP Stack */
        PCB.sipUrl.urlUserInfo.userName   = user;
        PCB.sipUrl.urlUserInfo.isPassword = RV_TRUE;
        PCB.sipUrl.urlUserInfo.password   = pass;
     
}
#define ag_rp_164(PCB_POINTER, t) (t)
#define ag_rp_165(PCB_POINTER) (CUR_STRING())
#define ag_rp_166(PCB_POINTER) (CUR_STRING())
static void ag_rp_167(PCB_DECL) {
/* Line 2885, RV SIP Stack */
        PCB.sipUrl.bIsUserInfo = RV_FALSE;
    
}
static void ag_rp_168(PCB_DECL) {
/* Line 2891, RV SIP Stack */
        SET_PARSER_PCB_BACK_TO_BUFF(RV_SIPPARSER_PREFIX_HOST);
    
}
static void ag_rp_169(PCB_DECL, ParserBasicToken host) {
/* Line 2899, RV SIP Stack */
        PCB.sipUrl.urlHost.hostName = host;
        PCB.sipUrl.urlHost.isPort   = RV_FALSE;
    
}
static void ag_rp_170(PCB_DECL, ParserBasicToken host, ParserPort port) {
/* Line 2905, RV SIP Stack */
        PCB.sipUrl.urlHost.hostName = host;
        PCB.sipUrl.urlHost.isPort   = RV_TRUE;
        PCB.sipUrl.urlHost.port     = port;
    
}
#define ag_rp_171(PCB_POINTER, t) (t)
#define ag_rp_172(PCB_POINTER, ip6ref) (ip6ref)
#define ag_rp_173(PCB_POINTER) (CUR_STRING())
#define ag_rp_174(PCB_POINTER) (CUR_STRING())
#define ag_rp_175(PCB_POINTER) (CUR_STRING())
#define ag_rp_176(PCB_POINTER) (RESET_URI_PARAMS())
static void ag_rp_177(PCB_DECL) {
/* Line 2959, RV SIP Stack */
	
       PCB.sipUrl.urlParameters.isValid = RV_TRUE;
    
}
static void ag_rp_178(PCB_DECL) {
/* Line 2972, RV SIP Stack */
 
        RESET_URI_PARAMS();  
         								
        switch( PCB.urlParameter.paramType )
        {
            case PARSER_TRANSPORT_PARAM: 
	            PCB.sipUrl.urlParameters.isTransport        = RV_TRUE;
	            PCB.sipUrl.urlParameters.transport          = PCB.urlParameter.param.transport; 
	            break;
            case PARSER_USER_PARAM: 
	            PCB.sipUrl.urlParameters.isUserParam        = RV_TRUE;
	            PCB.sipUrl.urlParameters.userParam          = PCB.urlParameter.param.userParam; 
	            break;		
            case PARSER_TTL_PARAM: 
	            PCB.sipUrl.urlParameters.isTtlParam         = RV_TRUE;
	            PCB.sipUrl.urlParameters.ttlParam           = PCB.urlParameter.param.ttlParam; 
	            break;
            case PARSER_MADDR_PARAM: 
	            PCB.sipUrl.urlParameters.isMaddrParam       = RV_TRUE;
	            PCB.sipUrl.urlParameters.maddrParam         = PCB.urlParameter.param.maddrParam; 
	            break;
            case PARSER_METHOD_PARAM: 
	            PCB.sipUrl.urlParameters.isMethodParam       = RV_TRUE;
	            PCB.sipUrl.urlParameters.methodParam         = PCB.urlParameter.param.methodParam; 
	            break;
            case PARSER_LR_PARAM: 
	            PCB.sipUrl.urlParameters.lrParamType         = PCB.urlParameter.param.lrParamType;
	            break;
            case PARSER_COMP_PARAM:
                PCB.sipUrl.urlParameters.isCompParam         = RV_TRUE;
                PCB.sipUrl.urlParameters.compParam           = PCB.urlParameter.param.compParam;
                break;
            case PARSER_SIGCOMPID_PARAM:
                PCB.sipUrl.urlParameters.isSigCompIdParam    = RV_TRUE;
                PCB.sipUrl.urlParameters.sigCompIdParam      = PCB.urlParameter.param.sigCompIdParam;
                break;
			case PARSER_TOKENIZED_BY_PARAM: 
		        PCB.sipUrl.urlParameters.isTokenizedByParam	= RV_TRUE;
		        PCB.sipUrl.urlParameters.tokenizedByParam	= PCB.urlParameter.param.tokenizedByParam; 
		        break;
			case PARSER_ORIG_PARAM: 
		        PCB.sipUrl.urlParameters.bOrigParam			= PCB.urlParameter.param.bOrigParam;
		        break;
            case PARSER_OTHER_PARAM:    
	            break;
             default:
                 PCB.exit_flag = AG_SEMANTIC_ERROR_CODE;
                 PCB.eStat = RV_ERROR_ILLEGAL_SYNTAX;            
        } 	 														
    
}
static void ag_rp_179(PCB_DECL) {
/* Line 3033, RV SIP Stack */
 
         
        switch( PCB.urlParameter.paramType )
        {
	        case PARSER_TRANSPORT_PARAM: 
		        PCB.sipUrl.urlParameters.isTransport        = RV_TRUE;
		        PCB.sipUrl.urlParameters.transport          = PCB.urlParameter.param.transport; 
		        break;
	        case PARSER_USER_PARAM: 
		        PCB.sipUrl.urlParameters.isUserParam        = RV_TRUE;
		        PCB.sipUrl.urlParameters.userParam          = PCB.urlParameter.param.userParam; 
		        break;	
	        case PARSER_TTL_PARAM: 
		        PCB.sipUrl.urlParameters.isTtlParam         = RV_TRUE;
		        PCB.sipUrl.urlParameters.ttlParam           = PCB.urlParameter.param.ttlParam; 
		        break;
	        case PARSER_MADDR_PARAM: 
		        PCB.sipUrl.urlParameters.isMaddrParam       = RV_TRUE;
		        PCB.sipUrl.urlParameters.maddrParam         = PCB.urlParameter.param.maddrParam; 
		        break;  
            case PARSER_METHOD_PARAM: 
		        PCB.sipUrl.urlParameters.isMethodParam		= RV_TRUE;
		        PCB.sipUrl.urlParameters.methodParam		= PCB.urlParameter.param.methodParam; 
		        break; 
            case PARSER_LR_PARAM: 
		        PCB.sipUrl.urlParameters.lrParamType		= PCB.urlParameter.param.lrParamType;
		        break; 
            case PARSER_COMP_PARAM:
                PCB.sipUrl.urlParameters.isCompParam		= RV_TRUE;
                PCB.sipUrl.urlParameters.compParam			= PCB.urlParameter.param.compParam;
                break;
            case PARSER_SIGCOMPID_PARAM:
                PCB.sipUrl.urlParameters.isSigCompIdParam   = RV_TRUE;
                PCB.sipUrl.urlParameters.sigCompIdParam     = PCB.urlParameter.param.sigCompIdParam;
                break;
			case PARSER_TOKENIZED_BY_PARAM: 
		        PCB.sipUrl.urlParameters.isTokenizedByParam	= RV_TRUE;
		        PCB.sipUrl.urlParameters.tokenizedByParam	= PCB.urlParameter.param.tokenizedByParam; 
		        break;
			case PARSER_ORIG_PARAM: 
		        PCB.sipUrl.urlParameters.bOrigParam			= PCB.urlParameter.param.bOrigParam;
		        break;
            case PARSER_OTHER_PARAM:    
		        PCB.sipUrl.urlParameters.isOtherParams		= RV_TRUE;
		        break; 
            default:
                PCB.exit_flag = AG_SEMANTIC_ERROR_CODE;
                PCB.eStat = RV_ERROR_ILLEGAL_SYNTAX;                  
        }	 
    
}
static void ag_rp_180(PCB_DECL, ParserTransport t) {
/* Line 3097, RV SIP Stack */
 
         PCB.urlParameter.paramType = PARSER_TRANSPORT_PARAM;
         PCB.urlParameter.param.transport = t;
    
}
static void ag_rp_181(PCB_DECL, ParserUserParam u) {
/* Line 3102, RV SIP Stack */
 
	     PCB.urlParameter.paramType = PARSER_USER_PARAM;
		 PCB.urlParameter.param.userParam = u;
	
}
static void ag_rp_182(PCB_DECL, ParserTtlParam t) {
/* Line 3107, RV SIP Stack */
 
	     PCB.urlParameter.paramType = PARSER_TTL_PARAM;
		 PCB.urlParameter.param.ttlParam = t;
	
}
static void ag_rp_183(PCB_DECL, ParserMaddrParam ma) {
/* Line 3112, RV SIP Stack */
 
	     PCB.urlParameter.paramType = PARSER_MADDR_PARAM;
		 PCB.urlParameter.param.maddrParam = ma;
	
}
static void ag_rp_184(PCB_DECL, ParserMethod met) {
/* Line 3118, RV SIP Stack */
	     PCB.urlParameter.paramType = PARSER_METHOD_PARAM;
		 PCB.urlParameter.param.methodParam =met;
    
}
static void ag_rp_185(PCB_DECL, ParserLrParamType lrParam) {
/* Line 3123, RV SIP Stack */
	     PCB.urlParameter.paramType = PARSER_LR_PARAM;
		 PCB.urlParameter.param.lrParamType   = lrParam;
    
}
static void ag_rp_186(PCB_DECL, ParserCompParam comp) {
/* Line 3128, RV SIP Stack */
        PCB.urlParameter.paramType       = PARSER_COMP_PARAM;
        PCB.urlParameter.param.compParam = comp;
    
}
static void ag_rp_187(PCB_DECL, ParserSigCompIdParam sigCompId) {
/* Line 3133, RV SIP Stack */
		PCB.urlParameter.paramType = PARSER_SIGCOMPID_PARAM;
		PCB.urlParameter.param.sigCompIdParam = sigCompId;
	
}
static void ag_rp_188(PCB_DECL, ParserTokenizedByParam tokenizedBy) {
/* Line 3138, RV SIP Stack */
        PCB.urlParameter.paramType       = PARSER_TOKENIZED_BY_PARAM;
        PCB.urlParameter.param.tokenizedByParam = tokenizedBy;
    
}
static void ag_rp_189(PCB_DECL, RvBool orig) {
/* Line 3143, RV SIP Stack */
        PCB.urlParameter.paramType        = PARSER_ORIG_PARAM;
        PCB.urlParameter.param.bOrigParam = orig;
    
}
static void ag_rp_190(PCB_DECL) {
/* Line 3165, RV SIP Stack */
	     PCB.urlParameter.paramType = PARSER_OTHER_PARAM;
    
}
#define ag_rp_191(PCB_POINTER, t) (t)
static ParserUserParam ag_rp_192(PCB_DECL, RvSipUserParam pt) {
/* Line 3180, RV SIP Stack */
 
        ParserUserParam userParam;
	    userParam.paramType = pt;
		userParam.otherUser.buf = NULL;
		userParam.otherUser.len = 0;
        RV_UNUSED_ARG(pcb_pointer);
	    return userParam;
    
}
static ParserUserParam ag_rp_193(PCB_DECL, ParserOtherUser ou) {
/* Line 3189, RV SIP Stack */
 
        ParserUserParam userParam;
        userParam.paramType = RVSIP_USERPARAM_OTHER;
	    userParam.otherUser = ou;
        RV_UNUSED_ARG(pcb_pointer);
	    return userParam;
    
}
#define ag_rp_194(PCB_POINTER) (RVSIP_USERPARAM_PHONE)
#define ag_rp_195(PCB_POINTER) (RVSIP_USERPARAM_IP)
#define ag_rp_196(PCB_POINTER, t) (t)
#define ag_rp_197(PCB_POINTER, ttl) (ttl)
#define ag_rp_198(PCB_POINTER, host) (host)
#define ag_rp_199(PCB_POINTER, met) (met)
#define ag_rp_200(PCB_POINTER) (ParserLrParam1)
#define ag_rp_201(PCB_POINTER) (ParserLrParamTrue)
#define ag_rp_202(PCB_POINTER) (ParserLrParamOn)
#define ag_rp_203(PCB_POINTER) (ParserLrParamEmpty)
#define ag_rp_204(PCB_POINTER, comp) (comp)
#define ag_rp_205(PCB_POINTER, sigCompId) (sigCompId)
#define ag_rp_206(PCB_POINTER) (CUR_STRING())
#define ag_rp_207(PCB_POINTER, hostname) (hostname)
#define ag_rp_208(PCB_POINTER) (RV_TRUE)
static void ag_rp_209(PCB_DECL, ParserBasicToken token) {
/* Line 3325, RV SIP Stack */
          if (0!=((ParserExtensionString *)PCB.pUrlExtParams)->size)
          {
              
              APPEND_DATA(";", 1,PCB.pUrlExtParams);
              APPEND_DATA(token.buf,token.len, PCB.pUrlExtParams);
          }
          else
          {      
              
              APPEND_DATA(token.buf,token.len,PCB.pUrlExtParams);
          }
    
}
static void ag_rp_210(PCB_DECL, ParserBasicToken token1, ParserBasicToken token2) {
/* Line 3339, RV SIP Stack */
          if (0!=((ParserExtensionString *)PCB.pUrlExtParams)->size)
          {
              
              APPEND_DATA(";", 1,PCB.pUrlExtParams);
              APPEND_DATA(token1.buf, token1.len,PCB.pUrlExtParams);
              APPEND_DATA("=" , 1,PCB.pUrlExtParams);
              APPEND_DATA(token2.buf, token2.len,PCB.pUrlExtParams);            
          }
          else
          {      
              
              APPEND_DATA(token1.buf, token1.len,PCB.pUrlExtParams);
              APPEND_DATA("=" , 1,PCB.pUrlExtParams);
              APPEND_DATA(token2.buf, token2.len,PCB.pUrlExtParams);            
          }
    
}
#define ag_rp_211(PCB_POINTER) (CUR_STRING())
#define ag_rp_212(PCB_POINTER, token) (token)
static ParserOptionalHeaders ag_rp_213(PCB_DECL) {
/* Line 3382, RV SIP Stack */
 
        ParserOptionalHeaders optionalHeaders;
        optionalHeaders.isSpecified = RV_FALSE;
		optionalHeaders.body.buf = NULL;
		optionalHeaders.body.len = 0;
        RV_UNUSED_ARG(pcb_pointer);
        return optionalHeaders;
    
}
static ParserOptionalHeaders ag_rp_214(PCB_DECL, ParserBasicToken headersList) {
/* Line 3392, RV SIP Stack */
 
        ParserOptionalHeaders optionalHeaders;
        optionalHeaders.isSpecified = RV_TRUE;
        optionalHeaders.body=headersList;
        RV_UNUSED_ARG(pcb_pointer);
        return optionalHeaders;
    
}
#define ag_rp_215(PCB_POINTER) (CUR_STRING())
#define ag_rp_216(PCB_POINTER) (CUR_STRING())
#define ag_rp_217(PCB_POINTER) (CUR_STRING())
#define ag_rp_218(PCB_POINTER) (CUR_STRING())
#define ag_rp_219(PCB_POINTER) (CUR_STRING())
static void ag_rp_220(PCB_DECL) {
/* Line 3428, RV SIP Stack */
        RvStatus status;
        status = ParserInitRequsetLine(PCB.pParserMgr,&PCB,PCB.pointer,&(PCB.requestLine), PCB.pSipObject);
        if(RV_ERROR_ILLEGAL_SYNTAX == status)
        {
            PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
            SYNTAX_ERROR;
        }
        else if (RV_OK != status)
        {
            
            PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
            PCB.eStat = status;
        }
    
}
static void ag_rp_221(PCB_DECL, ParserMethod met, ParserSipVersion ver) {
/* Line 3447, RV SIP Stack */
         PCB.requestLine.method = met;
         PCB.requestLine.sipVer = ver;
		 PCB.requestLine.exUri = PCB.exUri;         
    
}
#define ag_rp_222(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("m"))
#define ag_rp_223(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("i"))
#define ag_rp_224(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("c"))
#define ag_rp_225(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("v"))
#define ag_rp_226(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("t"))
#define ag_rp_227(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("f"))
#define ag_rp_228(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("r"))
#define ag_rp_229(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("b"))
#define ag_rp_230(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("k"))
#define ag_rp_231(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("x"))
#define ag_rp_232(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("o"))
#define ag_rp_233(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("u"))
#define ag_rp_234(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("l"))
#define ag_rp_235(PCB_POINTER, met) (met)
#define ag_rp_236(PCB_POINTER) (SET_URI_TYPE(PARSER_ADDR_PARAM_TYPE_SIP_URL, PCB.exUri.ExUriInfo.absUri))
#define ag_rp_237(PCB_POINTER, uri) (SET_URI_TYPE(PARSER_ADDR_PARAM_TYPE_PRES_URI, uri))
#define ag_rp_238(PCB_POINTER, uri) (SET_URI_TYPE(PARSER_ADDR_PARAM_TYPE_IM_URI, uri))
#define ag_rp_239(PCB_POINTER) (SET_URI_TYPE(PARSER_ADDR_PARAM_TYPE_TEL_URI, PCB.exUri.ExUriInfo.absUri))
#define ag_rp_240(PCB_POINTER, uri) (SET_URI_TYPE(PARSER_ADDR_PARAM_TYPE_ABS_URI, uri))
static void ag_rp_241(PCB_DECL) {
/* Line 3505, RV SIP Stack */
 
         RvStatus status;
         status = ParserInitParty(PCB.pParserMgr,PCB.pointer,&(PCB.party), PCB.eHeaderType, PCB.eWhichHeader, PCB.pSipObject);
         if(RV_ERROR_ILLEGAL_SYNTAX == status)
         {
             PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
             SYNTAX_ERROR;
         }
         else if (RV_OK != status)
         {
             
             PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
             PCB.eStat = status;
         }        
	 
}
static void ag_rp_242(PCB_DECL) {
/* Line 3524, RV SIP Stack */
 
	  PCB.party.nameAddr = PCB.nameAddr;
	  PCB.party.partyParams.isTag = RV_FALSE;
	  PCB.party.partyParams.isAddrParams = RV_FALSE;
      PCB.party.isCompact = PCB.isCompactForm;
	
}
static void ag_rp_243(PCB_DECL, ParserPartyParams partyParams) {
/* Line 3533, RV SIP Stack */
 
	  PCB.party.nameAddr = PCB.nameAddr;
	  PCB.party.partyParams = partyParams; 
      PCB.party.isCompact = PCB.isCompactForm;
	
}
static void ag_rp_244(PCB_DECL) {
/* Line 3542, RV SIP Stack */
 
	  PCB.party.nameAddr.isDisplayName = RV_FALSE;
      PCB.party.partyParams.isTag = RV_FALSE;
	  PCB.party.partyParams.isAddrParams = RV_FALSE;
	  PCB.party.nameAddr.exUri = PCB.exUri;
      PCB.party.isCompact = PCB.isCompactForm;
	
}
static void ag_rp_245(PCB_DECL, ParserPartyParams partyParams) {
/* Line 3552, RV SIP Stack */
 
        PCB.party.nameAddr.isDisplayName = RV_FALSE;
        PCB.party.partyParams = partyParams;
		PCB.party.nameAddr.exUri = PCB.exUri;       
        PCB.party.isCompact = PCB.isCompactForm;
      
	
}
static void ag_rp_246(PCB_DECL, ParserDisplayName name) {
/* Line 3564, RV SIP Stack */
 
		PCB.nameAddr.exUri = PCB.exUri;    
        PCB.nameAddr.isDisplayName = RV_TRUE;  
	    PCB.nameAddr.name = name;
	
}
static void ag_rp_247(PCB_DECL) {
/* Line 3571, RV SIP Stack */
 
		PCB.nameAddr.exUri = PCB.exUri;
        PCB.nameAddr.isDisplayName = RV_FALSE;  
    
}
#define ag_rp_248(PCB_POINTER, qs) (qs)
#define ag_rp_249(PCB_POINTER, mtot) (mtot)
#define ag_rp_250(PCB_POINTER) (CUR_STRING())
#define ag_rp_251(PCB_POINTER) (CUR_STRING())
#define ag_rp_252(PCB_POINTER, t) (t)
#define ag_rp_253(PCB_POINTER, ppd) (ppd)
static ParserPartyParams ag_rp_254(PCB_DECL, ParserPartyParams ppl, ParserPartyParams ppd) {
/* Line 3602, RV SIP Stack */
        if (RV_TRUE == ppd.isTag)
        {
            ppl.isTag = RV_TRUE;
            ppl.tag = ppd.tag;
        }
        if (RV_TRUE == ppd.isAddrParams) 
        {
            ppl.isAddrParams = RV_TRUE;
            ppl.addrParams = ppd.addrParams;
        }
        RV_UNUSED_ARG(pcb_pointer);
        return ppl;
    
}
static ParserPartyParams ag_rp_255(PCB_DECL, ParserTagParam tag) {
/* Line 3621, RV SIP Stack */
        ParserPartyParams partyParams;
        partyParams.isTag = RV_TRUE;
        partyParams.tag = tag;
        partyParams.isAddrParams = RV_FALSE;
        partyParams.addrParams = NULL;
        RV_UNUSED_ARG(pcb_pointer);
        return partyParams; 
    
}
static ParserPartyParams ag_rp_256(PCB_DECL) {
/* Line 3631, RV SIP Stack */
        ParserPartyParams partyParams;
        partyParams.isAddrParams = RV_TRUE;
        partyParams.isTag = RV_FALSE;
        partyParams.addrParams = PCB.pExtParams;
		partyParams.tag.buf = NULL;
		partyParams.tag.len = 0;
        return partyParams; 
    
}
static void ag_rp_257(PCB_DECL) {
/* Line 3657, RV SIP Stack */
         RvStatus status;
         ParserContactHeaderValues contactHeaderValues;
         contactHeaderValues.isStar = RV_TRUE; 
         contactHeaderValues.isCompact = PCB.isCompactForm;
         status = ParserInitContactHeader(PCB.pParserMgr, &PCB, PCB.pointer,
                                          &contactHeaderValues,
                                          PCB.eHeaderType,
                                          PCB.pSipObject);
         if(RV_ERROR_ILLEGAL_SYNTAX == status)
         {
             PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
             SYNTAX_ERROR;
         }
         else if (RV_OK != status)
         {
             
             PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
             PCB.eStat = status;
         }
    
}
static void ag_rp_258(PCB_DECL) {
/* Line 3690, RV SIP Stack */
         RvStatus status;
         ParserContactHeaderValues  contactHeaderValues;
         contactHeaderValues.isStar        = RV_FALSE;
         contactHeaderValues.header        = PCB.contact;
         contactHeaderValues.isCompact     = PCB.isCompactForm;
         status = ParserInitContactHeader(PCB.pParserMgr,&PCB,PCB.pointer,
                                          &contactHeaderValues,
                                          PCB.eHeaderType,
                                          PCB.pSipObject);
         if(RV_ERROR_ILLEGAL_SYNTAX == status)
         {
             PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
             SYNTAX_ERROR;
         }
         else if (RV_OK != status)
         {
             
             PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
             PCB.eStat = status;
         }
         
    
}
static void ag_rp_259(PCB_DECL) {
/* Line 3715, RV SIP Stack */
         RvStatus                 status;
         ParserContactHeaderValues contactHeaderValues;
         contactHeaderValues.isStar = RV_FALSE;
         contactHeaderValues.header = PCB.contact;
         contactHeaderValues.isCompact = PCB.isCompactForm;
         status = ParserInitContactHeader(PCB.pParserMgr,&PCB, PCB.pointer,
                                          &contactHeaderValues,
                                          PCB.eHeaderType,
                                          PCB.pSipObject);
         if(RV_ERROR_ILLEGAL_SYNTAX == status)
         {
             PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
             SYNTAX_ERROR;
         }
         else if (RV_OK != status)
         {
             
             PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
             PCB.eStat = status;
         }
    
}
static void ag_rp_260(PCB_DECL) {
/* Line 3742, RV SIP Stack */
 
	  PCB.contact.nameAddr = PCB.nameAddr;
      PCB.contact.isParams = RV_FALSE;
	
}
static void ag_rp_261(PCB_DECL) {
/* Line 3748, RV SIP Stack */
 
	  PCB.contact.nameAddr = PCB.nameAddr;
      PCB.contact.isParams = RV_TRUE;
      
	
}
static void ag_rp_262(PCB_DECL) {
/* Line 3758, RV SIP Stack */
 
	  PCB.contact.nameAddr.isDisplayName = RV_FALSE;  
      PCB.contact.isParams = RV_FALSE;
	  PCB.contact.nameAddr.exUri = PCB.exUri;      
	
}
static void ag_rp_263(PCB_DECL) {
/* Line 3765, RV SIP Stack */
 
      PCB.contact.nameAddr.isDisplayName = RV_FALSE;
	  PCB.contact.isParams = RV_TRUE;      
      PCB.contact.nameAddr.exUri = PCB.exUri;
	
}
static void ag_rp_264(PCB_DECL, ParserQVal q) {
/* Line 3785, RV SIP Stack */
 
        PCB.contact.params.isQ = RV_TRUE;
        PCB.contact.params.q = q;
    
}
static void ag_rp_265(PCB_DECL, ParserContactActionType action) {
/* Line 3791, RV SIP Stack */
 
        PCB.contact.params.isAction = RV_TRUE;
        PCB.contact.params.action = action;
    
}
static void ag_rp_266(PCB_DECL) {
/* Line 3797, RV SIP Stack */
 
        PCB.contact.params.isExpires = RV_TRUE;
        PCB.contact.params.expires = PCB.expires;
    
}
static void ag_rp_267(PCB_DECL) {
/* Line 3971, RV SIP Stack */
  
        PCB.contact.params.isExtention = RV_TRUE;
		PCB.contact.params.exten = PCB.pExtParams;
    
}
#define ag_rp_268(PCB_POINTER, qVal) (qVal)
#define ag_rp_269(PCB_POINTER) (CUR_STRING())
#define ag_rp_270(PCB_POINTER) (CUR_STRING())
#define ag_rp_271(PCB_POINTER, actionVal) (actionVal)
#define ag_rp_272(PCB_POINTER) (PARSER_CONTACT_ACTION_PROXY)
#define ag_rp_273(PCB_POINTER) (PARSER_CONTACT_ACTION_REDIRECT)
static void ag_rp_274(PCB_DECL) {
/* Line 4144, RV SIP Stack */
       RvStatus status;
       status = ParserInitExpiresHeader(PCB.pParserMgr,PCB.pointer,&(PCB.expires),PCB.eHeaderType,PCB.pSipObject);
       if(RV_ERROR_ILLEGAL_SYNTAX == status)
       {
           PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
           SYNTAX_ERROR;
       }
       else if (RV_OK != status)
       {
           
           PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
           PCB.eStat = status;
       }
    
}
static void ag_rp_275(PCB_DECL, ParserDeltaSeconds delta) {
/* Line 4165, RV SIP Stack */
  
		PCB.expires.eFormat = RVSIP_EXPIRES_FORMAT_DELTA_SECONDS;
		PCB.expires.deltaSeconds = delta;
    
}
static void ag_rp_276(PCB_DECL, ParserSipDate date) {
/* Line 4171, RV SIP Stack */
   
	    PCB.expires.eFormat = RVSIP_EXPIRES_FORMAT_DATE;
	    PCB.expires.sipDate = date;
	
}
static void ag_rp_277(PCB_DECL, ParserDeltaSeconds delta) {
/* Line 4180, RV SIP Stack */
   
		PCB.expires.eFormat = RVSIP_EXPIRES_FORMAT_DELTA_SECONDS;
		PCB.expires.deltaSeconds = delta;
    
}
static void ag_rp_278(PCB_DECL, ParserSipDate date) {
/* Line 4186, RV SIP Stack */
   
	    PCB.expires.eFormat = RVSIP_EXPIRES_FORMAT_DATE;
	    PCB.expires.sipDate = date;
	
}
#define ag_rp_279(PCB_POINTER) (CUR_STRING())
static void ag_rp_280(PCB_DECL, ParserSipDate date) {
/* Line 4211, RV SIP Stack */
   
          RvStatus status;
          status = ParserInitDateHeader(PCB.pParserMgr,PCB.pointer,&date,PCB.eHeaderType,PCB.pSipObject);
          if(RV_ERROR_ILLEGAL_SYNTAX == status)
          {
              PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
              SYNTAX_ERROR;
          }
          else if (RV_OK != status)
          {
                
              PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
              PCB.eStat = status;
          }
    
}
static ParserSipDate ag_rp_281(PCB_DECL, ParserDDMMYY date, ParserTime t) {
/* Line 4233, RV SIP Stack */
  
        ParserSipDate sipDate;
        sipDate.eWeekDay = RVSIP_WEEKDAY_UNDEFINED;
		sipDate.ddmmyy   = date;
        sipDate.time     = t;
        RV_UNUSED_ARG(pcb_pointer);
        return sipDate;
    
}
#define ag_rp_282(PCB_POINTER, date) (date)
static ParserSipDate ag_rp_283(PCB_DECL, RvSipDateWeekDay day, ParserDDMMYY date, ParserTime t) {
/* Line 4248, RV SIP Stack */
   
        ParserSipDate sipDate;
        sipDate.eWeekDay = RVSIP_WEEKDAY_UNDEFINED;
	    sipDate.eWeekDay = day; 
		sipDate.ddmmyy   = date;
        sipDate.time     = t;
        RV_UNUSED_ARG(pcb_pointer);
        return sipDate;
    
}
#define ag_rp_284(PCB_POINTER) (RVSIP_WEEKDAY_SUN)
#define ag_rp_285(PCB_POINTER) (RVSIP_WEEKDAY_MON)
#define ag_rp_286(PCB_POINTER) (RVSIP_WEEKDAY_TUE)
#define ag_rp_287(PCB_POINTER) (RVSIP_WEEKDAY_WED)
#define ag_rp_288(PCB_POINTER) (RVSIP_WEEKDAY_THU)
#define ag_rp_289(PCB_POINTER) (RVSIP_WEEKDAY_FRI)
#define ag_rp_290(PCB_POINTER) (RVSIP_WEEKDAY_SAT)
static ParserDDMMYY ag_rp_291(PCB_DECL, ParserInt32 dd, RvSipDateMonth mm, ParserInt32 yy) {
/* Line 4273, RV SIP Stack */
        ParserDDMMYY ddmmyy;
        ddmmyy.day = dd;
        ddmmyy.eMonth = mm;
        ddmmyy.year = yy;
        RV_UNUSED_ARG(pcb_pointer);
        return ddmmyy;
    
}
static ParserInt32 ag_rp_292(PCB_DECL, int d) {
/* Line 4285, RV SIP Stack */
    	RV_UNUSED_ARG(pcb_pointer);
    	return d - '0';	
    
}
static ParserInt32 ag_rp_293(PCB_DECL, int d1, int d2) {
/* Line 4290, RV SIP Stack */
		RV_UNUSED_ARG(pcb_pointer);
	    return 10 * (d1-'0') + (d2-'0');
    
}
#define ag_rp_294(PCB_POINTER) (RVSIP_MONTH_JAN)
#define ag_rp_295(PCB_POINTER) (RVSIP_MONTH_FEB)
#define ag_rp_296(PCB_POINTER) (RVSIP_MONTH_MAR)
#define ag_rp_297(PCB_POINTER) (RVSIP_MONTH_APR)
#define ag_rp_298(PCB_POINTER) (RVSIP_MONTH_MAY)
#define ag_rp_299(PCB_POINTER) (RVSIP_MONTH_JUN)
#define ag_rp_300(PCB_POINTER) (RVSIP_MONTH_JUL)
#define ag_rp_301(PCB_POINTER) (RVSIP_MONTH_AUG)
#define ag_rp_302(PCB_POINTER) (RVSIP_MONTH_SEP)
#define ag_rp_303(PCB_POINTER) (RVSIP_MONTH_OCT)
#define ag_rp_304(PCB_POINTER) (RVSIP_MONTH_NOV)
#define ag_rp_305(PCB_POINTER) (RVSIP_MONTH_DEC)
#define ag_rp_306(PCB_POINTER, d1, d2) (2000 + 10 * (d1-'0') + (d2-'0'))
#define ag_rp_307(PCB_POINTER, d1, d2, d3) (2000 + 100 * (d1-'0') + 10 * (d2-'0') + (d3-'0'))
#define ag_rp_308(PCB_POINTER, d1, d2, d3, d4) (1000 * (d1-'0') + 100*(d2-'0') + 10*(d3-'0') + (d4-'0'))
static ParserTime ag_rp_309(PCB_DECL, ParserInt32 hour, ParserInt32 min, ParserInt32 second, ParserTimeZone zone) {
/* Line 4320, RV SIP Stack */
        ParserTime t;
        t.hour   = hour;
        t.minute = min;
        t.second = second;
        t.zone   = zone;
        RV_UNUSED_ARG(pcb_pointer);
        return t;        
    
}
#define ag_rp_310(PCB_POINTER, d1, d2) (10*(d1-'0') + (d2-'0'))
#define ag_rp_311(PCB_POINTER, d1, d2) (10*(d1-'0') + (d2-'0'))
#define ag_rp_312(PCB_POINTER, d1, d2) (10*(d1-'0') + (d2-'0'))
#define ag_rp_313(PCB_POINTER) (SIPPARSER_TIME_ZONE_GMT)
#define ag_rp_314(PCB_POINTER) (SIPPARSER_TIME_ZONE_UTC)
#define ag_rp_315(PCB_POINTER) (SIPPARSER_TIME_ZONE_GMST)
static void ag_rp_316(PCB_DECL, ParserMethod method) {
/* Line 4367, RV SIP Stack */
        RvStatus         status;
        ParserOtherHeader otherHeader;
        otherHeader.name = method.other;
        otherHeader.value.buf = NULL;
        otherHeader.value.len = 0;
        ParserMethodInOtherHeader(&method, &otherHeader);
        status = ParserInitOtherHeader(PCB.pParserMgr,&PCB,PCB.pointer,
                                       &otherHeader,
                                       PCB.eHeaderType,
                                       PCB.pSipObject);
        if(RV_ERROR_ILLEGAL_SYNTAX == status)
        {
             PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
             SYNTAX_ERROR;
        }
        else if (RV_OK != status)
        {
             
             PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
             PCB.eStat = status;
        }
        
    
}
static void ag_rp_317(PCB_DECL, ParserMethod method, ParserBasicToken fv) {
/* Line 4392, RV SIP Stack */
        RvStatus         status;
        ParserOtherHeader otherHeader;
        otherHeader.name = method.other;
        otherHeader.value = fv;
        ParserMethodInOtherHeader(&method, &otherHeader);
        status = ParserInitOtherHeader(PCB.pParserMgr,&PCB,PCB.pointer,
                                       &otherHeader,
                                       PCB.eHeaderType,
                                       PCB.pSipObject);
        if(RV_ERROR_ILLEGAL_SYNTAX == status)
        {
            PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
            SYNTAX_ERROR;
        }
        else if (RV_OK != status)
        {
            
            PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
            PCB.eStat = status;
        }
    
}
static void ag_rp_318(PCB_DECL) {
/* Line 4422, RV SIP Stack */
          RvStatus status; 
          status = ParserInitAuthentication(PCB.pParserMgr,&PCB,PCB.pointer,&(PCB.authentication),PCB.eHeaderType,PCB.eWhichHeader,PCB.pSipObject);
          if(RV_ERROR_ILLEGAL_SYNTAX == status)
          {
              PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
              SYNTAX_ERROR;
          }
          else if (RV_OK != status)
          {
               
               PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
               PCB.eStat = status;
          }
    
}
static void ag_rp_319(PCB_DECL) {
/* Line 4445, RV SIP Stack */
    
        PCB.authentication.eAuthScheme = RVSIP_AUTH_SCHEME_DIGEST;
        PCB.authentication.isAuthParamList = RV_FALSE;
    
}
static void ag_rp_320(PCB_DECL, ParserBasicToken authScheme) {
/* Line 4451, RV SIP Stack */
        
        PCB.authentication.eAuthScheme = RVSIP_AUTH_SCHEME_OTHER;
        PCB.authentication.authScheme  = authScheme;
        PCB.authentication.isRealm =     RV_FALSE;
        PCB.authentication.isDomain =    RV_FALSE;
        PCB.authentication.isNonce =     RV_FALSE;
        PCB.authentication.isOpaque =    RV_FALSE;
        PCB.authentication.isAuthParam = RV_FALSE;
        PCB.authentication.eStale =      RVSIP_AUTH_STALE_UNDEFINED;
        PCB.authentication.algorithm.eAlgorithm = RVSIP_AUTH_ALGORITHM_UNDEFINED;
		PCB.authentication.algorithm.isAKAVersion = RV_FALSE;
		PCB.authentication.qopValue.eAuthQop    = RVSIP_AUTH_QOP_UNDEFINED;
        PCB.authentication.qopValue.isOther = RV_FALSE;
        PCB.authentication.isAuthParamList = RV_TRUE;
		PCB.authentication.isIntegrityKey =	RV_FALSE;
		PCB.authentication.isCipherKey =	RV_FALSE;
        PCB.authentication.authParamList = PCB.pExtParams;
    
}
static void ag_rp_321(PCB_DECL) {
/* Line 4478, RV SIP Stack */
        PCB.authentication.eAuthScheme = RVSIP_AUTH_SCHEME_UNDEFINED;
        PCB.authentication.isRealm =     RV_FALSE;
        PCB.authentication.isDomain =    RV_FALSE;
        PCB.authentication.isNonce =     RV_FALSE;
        PCB.authentication.isOpaque =    RV_FALSE;
        PCB.authentication.isAuthParam = RV_FALSE;
        PCB.authentication.eStale =      RVSIP_AUTH_STALE_UNDEFINED;
        PCB.authentication.algorithm.eAlgorithm = RVSIP_AUTH_ALGORITHM_UNDEFINED;
		PCB.authentication.algorithm.isAKAVersion = RV_FALSE;
        PCB.authentication.qopValue.eAuthQop    = RVSIP_AUTH_QOP_UNDEFINED;
        PCB.authentication.qopValue.isOther = RV_FALSE;
		PCB.authentication.isIntegrityKey =	RV_FALSE;
		PCB.authentication.isCipherKey =	RV_FALSE;
        switch (PCB.digestChallenge.eType)
        {
            case PARSER_DIGEST_TYPE_REALM:
            {
                PCB.authentication.isRealm = RV_TRUE;
                PCB.authentication.realm = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_DOMAIN:
            {
                PCB.authentication.isDomain = RV_TRUE;
                PCB.authentication.domain = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_NONCE:
            {
                PCB.authentication.isNonce = RV_TRUE;
                PCB.authentication.nonce = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_OPAQUE:
            {
                PCB.authentication.isOpaque = RV_TRUE;
                PCB.authentication.opaque = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_STALE:
            {
                PCB.authentication.eStale = PCB.digestChallenge.eStale;
                break;
            }
            case PARSER_DIGEST_TYPE_ALGORITHM:
            {
                PCB.authentication.algorithm = PCB.digestChallenge.algorithm;
                break;
            }
			case PARSER_DIGEST_TYPE_INTEGRITY_KEY:
            {
                PCB.authentication.integrityKey = PCB.digestChallenge.challenge;
                break;
            }
			case PARSER_DIGEST_TYPE_CIPHER_KEY:
            {
                PCB.authentication.cipherKey = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_OPQ_OPTIONS:
            {
                PCB.authentication.qopValue = PCB.digestChallenge.qopVal;
                break;
            }
            case PARSER_DIGEST_TYPE_AUTH_PARAM:
            {
                PCB.authentication.isAuthParam = RV_TRUE;
                PCB.authentication.authParam = PCB.digestChallenge.challenge;
                break;
            }
            default:
                    PCB.exit_flag = AG_SEMANTIC_ERROR_CODE;
                    PCB.eStat = RV_ERROR_ILLEGAL_SYNTAX;
        }
    
}
static void ag_rp_322(PCB_DECL) {
/* Line 4556, RV SIP Stack */
        if (PCB.isNewChallenge == RV_FALSE)
        {
            switch (PCB.digestChallenge.eType)
            {
                case PARSER_DIGEST_TYPE_REALM:
                {
                    PCB.authentication.isRealm = RV_TRUE;
                    PCB.authentication.realm = PCB.digestChallenge.challenge;
                    break;
                }
                case PARSER_DIGEST_TYPE_DOMAIN:
                {
                    PCB.authentication.isDomain = RV_TRUE;
                    PCB.authentication.domain = PCB.digestChallenge.challenge;
                    break;
                }
                case PARSER_DIGEST_TYPE_NONCE:
                {
                    PCB.authentication.isNonce = RV_TRUE;
                    PCB.authentication.nonce = PCB.digestChallenge.challenge;
                    break;
                }
                case PARSER_DIGEST_TYPE_OPAQUE:
                {
                    PCB.authentication.isOpaque = RV_TRUE;
                    PCB.authentication.opaque = PCB.digestChallenge.challenge;
                    break;
                }
                case PARSER_DIGEST_TYPE_STALE:
                {
                    PCB.authentication.eStale = PCB.digestChallenge.eStale;
                    break;
                }
                case PARSER_DIGEST_TYPE_ALGORITHM:
                {
                    PCB.authentication.algorithm = PCB.digestChallenge.algorithm;
                    break;
                }
				case PARSER_DIGEST_TYPE_INTEGRITY_KEY:
				{
					PCB.authentication.isIntegrityKey = RV_TRUE;
					PCB.authentication.integrityKey = PCB.digestChallenge.challenge;
					break;
				}
				case PARSER_DIGEST_TYPE_CIPHER_KEY:
				{
					PCB.authentication.isCipherKey = RV_TRUE;
					PCB.authentication.cipherKey = PCB.digestChallenge.challenge;
					break;
				}
                case PARSER_DIGEST_TYPE_OPQ_OPTIONS:
                {
                    PCB.authentication.qopValue = PCB.digestChallenge.qopVal;
                    break;
                }
                case PARSER_DIGEST_TYPE_AUTH_PARAM:
                {
                    PCB.authentication.isAuthParam = RV_TRUE;
                    PCB.authentication.authParam = PCB.digestChallenge.challenge;
                    break;
                }
                default:
                        PCB.exit_flag = AG_SEMANTIC_ERROR_CODE;
                            
                        PCB.eStat = RV_ERROR_ILLEGAL_SYNTAX;
            }
        }
    
}
#define ag_rp_323(PCB_POINTER, realm) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_REALM, realm))
#define ag_rp_324(PCB_POINTER, domain) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_DOMAIN, domain))
#define ag_rp_325(PCB_POINTER, nonce) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_NONCE, nonce))
#define ag_rp_326(PCB_POINTER, opaque) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_OPAQUE, opaque))
#define ag_rp_327(PCB_POINTER, ik) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_INTEGRITY_KEY, ik))
#define ag_rp_328(PCB_POINTER, ck) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_CIPHER_KEY, ck))
#define ag_rp_329(PCB_POINTER, authParam) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_AUTH_PARAM, authParam))
static void ag_rp_330(PCB_DECL, RvSipAuthStale stale) {
/* Line 4637, RV SIP Stack */
        PCB.digestChallenge.eType = PARSER_DIGEST_TYPE_STALE;
        PCB.digestChallenge.eStale = stale;
    
}
static void ag_rp_331(PCB_DECL, ParserAlgorithm algorithm) {
/* Line 4642, RV SIP Stack */
        PCB.digestChallenge.eType = PARSER_DIGEST_TYPE_ALGORITHM;
        
        if(PCB.digestChallenge.algorithm.eAlgorithm == RVSIP_AUTH_ALGORITHM_UNDEFINED)
        {
            PCB.digestChallenge.algorithm = algorithm;
        }
    
}
static void ag_rp_332(PCB_DECL, ParserQopValue qopOptions) {
/* Line 4651, RV SIP Stack */
        PCB.digestChallenge.eType = PARSER_DIGEST_TYPE_OPQ_OPTIONS;
        PCB.digestChallenge.qopVal = qopOptions;
    
}
static void ag_rp_333(PCB_DECL) {
/* Line 4657, RV SIP Stack */
        PCB.digestChallenge.eType = PARSER_DIGEST_TYPE_NEW_CHALLENGE;
        PCB.digestChallenge.challenge.buf = "NewChallenge";
        PCB.isNewChallenge = RV_TRUE;
    
}
#define ag_rp_334(PCB_POINTER, t) (t)
#define ag_rp_335(PCB_POINTER, t) (t)
#define ag_rp_336(PCB_POINTER) (CUR_LINE())
#define ag_rp_337(PCB_POINTER) (CUR_LINE())
static void ag_rp_338(PCB_DECL, ParserBasicToken token) {
/* Line 4693, RV SIP Stack */
        RV_UNUSED_ARG(pcb_pointer);
        RV_UNUSED_ARG(token.buf);
    
}
#define ag_rp_339(PCB_POINTER) (CUR_STRING())
#define ag_rp_340(PCB_POINTER) (SIP_BEGIN_LINE())
#define ag_rp_341(PCB_POINTER) (SIP_BEGIN_LINE())
#define ag_rp_342(PCB_POINTER, t) (t)
#define ag_rp_343(PCB_POINTER, t) (t)
#define ag_rp_344(PCB_POINTER, t) (t)
#define ag_rp_345(PCB_POINTER, t) (t)
#define ag_rp_346(PCB_POINTER) (RVSIP_AUTH_STALE_TRUE)
#define ag_rp_347(PCB_POINTER) (RVSIP_AUTH_STALE_FALSE)
#define ag_rp_348(PCB_POINTER, algo) (algo)
static void ag_rp_349(PCB_DECL) {
/* Line 4761, RV SIP Stack */
		DEFINE_AKAV_PREFIX();
		SIP_BEGIN_TOKEN();			
	
}
static void ag_rp_350(PCB_DECL) {
/* Line 4766, RV SIP Stack */
		DEFINE_AKAV_PREFIX();
		SIP_BEGIN_TOKEN();			
	
}
static void ag_rp_351(PCB_DECL) {
/* Line 4771, RV SIP Stack */
		DEFINE_AKAV_PREFIX();
		SIP_BEGIN_TOKEN();			
	
}
static void ag_rp_352(PCB_DECL) {
/* Line 4776, RV SIP Stack */
		DEFINE_AKAV_PREFIX();
		SIP_BEGIN_TOKEN();			
	
}
static ParserAlgorithm ag_rp_353(PCB_DECL, ParserBasicToken version, ParserAlgorithm algo) {
/* Line 4784, RV SIP Stack */
		algo.isAKAVersion = RV_TRUE;
		algo.nAKAVersion  = version;
		RV_UNUSED_ARG(pcb_pointer);
		return algo;
	
}
static ParserAlgorithm ag_rp_354(PCB_DECL, ParserAlgorithm algo) {
/* Line 4791, RV SIP Stack */
		algo.isAKAVersion = RV_FALSE;
		RV_UNUSED_ARG(pcb_pointer);
		return algo;
	
}
#define ag_rp_355(PCB_POINTER) (SIP_BEGIN_TOKEN())
static ParserAlgorithm ag_rp_356(PCB_DECL) {
/* Line 4803, RV SIP Stack */
        ParserAlgorithm algo;
        algo.eAlgorithm = RVSIP_AUTH_ALGORITHM_MD5;
		algo.algorithm.buf = NULL;
		algo.algorithm.len = 0;
		algo.nAKAVersion.buf = NULL;
		algo.nAKAVersion.len = 0;
		algo.isAKAVersion = RV_FALSE;
        RV_UNUSED_ARG(pcb_pointer);
        return algo;
    
}
static ParserAlgorithm ag_rp_357(PCB_DECL, ParserBasicToken t) {
/* Line 4815, RV SIP Stack */
        ParserAlgorithm algo;
        algo.eAlgorithm = RVSIP_AUTH_ALGORITHM_OTHER;
        algo.algorithm = t;
		algo.nAKAVersion.buf = NULL;
		algo.nAKAVersion.len = 0;
		algo.isAKAVersion = RV_FALSE;
        RV_UNUSED_ARG(pcb_pointer);
        return algo;
    
}
#define ag_rp_358(PCB_POINTER) (SET_PARSER_PCB_BACK_TO_BUFF(RV_SIPPARSER_PREFIX_AKAV))
#define ag_rp_359(PCB_POINTER) (SET_PARSER_PCB_BACK_TO_BUFF(RV_SIPPARSER_PREFIX_ALGORITHM))
#define ag_rp_360(PCB_POINTER) (CUR_STRING())
#define ag_rp_361(PCB_POINTER, qvList) (qvList)
static ParserQopValue ag_rp_362(PCB_DECL, ParserAuthQopOptions val) {
/* Line 4855, RV SIP Stack */
        ParserQopValue qop;
        qop.eAuthQop = RVSIP_AUTH_QOP_OTHER;
        qop.isOther  = RV_FALSE;
		qop.qop = NULL;
		
        if (RV_TRUE == val.isAuth)
        {
            qop.eAuthQop = RVSIP_AUTH_QOP_AUTH_ONLY;    
        }
        else if (RV_TRUE == val.isAuthInt)
        {
            qop.eAuthQop = RVSIP_AUTH_QOP_AUTHINT_ONLY;
        }
        else if (RV_TRUE == val.isOther)
        {
            if (0!=((ParserExtensionString *)PCB.pExtParams)->size)
            {
                APPEND_DATA(",", 1,PCB.pExtParams);
                
                APPEND_DATA(val.strOther.buf, val.strOther.len,PCB.pExtParams);
                qop.isOther = RV_TRUE;
                qop.qop =  PCB.pExtParams;
            }
            else
            {
                
                APPEND_DATA(val.strOther.buf, val.strOther.len,PCB.pExtParams);
                qop.isOther = RV_TRUE;
                qop.qop = PCB.pExtParams;
            }        
        }      
        return qop;        
    
}
static ParserQopValue ag_rp_363(PCB_DECL, ParserQopValue qop, ParserAuthQopOptions val) {
/* Line 4891, RV SIP Stack */
        if (RV_TRUE == val.isAuth && 
            RVSIP_AUTH_QOP_AUTHINT_ONLY == qop.eAuthQop)
        {
            qop.eAuthQop = RVSIP_AUTH_QOP_AUTH_AND_AUTHINT;    
        }
        else if (RV_TRUE == val.isAuth)
        {
            qop.eAuthQop = RVSIP_AUTH_QOP_AUTH_ONLY;    
        }
        else if (RV_TRUE == val.isAuthInt &&
                 RVSIP_AUTH_QOP_AUTH_ONLY == qop.eAuthQop)
        {
            qop.eAuthQop = RVSIP_AUTH_QOP_AUTH_AND_AUTHINT;
        }
        else if (RV_TRUE == val.isAuthInt)
        {
            qop.eAuthQop = RVSIP_AUTH_QOP_AUTHINT_ONLY;
        }
        else if (RV_TRUE == val.isOther)
        {
            if (0!=((ParserExtensionString *)PCB.pExtParams)->size)
            {
                APPEND_DATA(",", 1,PCB.pExtParams);
                
                APPEND_DATA(val.strOther.buf, val.strOther.len,PCB.pExtParams);
                qop.isOther = RV_TRUE;
                qop.qop =  PCB.pExtParams;
            }
            else
            {
                
                APPEND_DATA(val.strOther.buf, val.strOther.len,PCB.pExtParams);
                qop.isOther = RV_TRUE;
                qop.qop = PCB.pExtParams;
            }        
        }
        return qop;
    
}
static ParserAuthQopOptions ag_rp_364(PCB_DECL) {
/* Line 4936, RV SIP Stack */
        ParserAuthQopOptions qopOption;
        qopOption.isAuthInt = RV_TRUE;
        qopOption.isAuth    = RV_FALSE; 
        qopOption.isOther   = RV_FALSE;
		qopOption.strOther.buf = NULL;
		qopOption.strOther.len = 0;
        RV_UNUSED_ARG(pcb_pointer);
        return qopOption;
    
}
static ParserAuthQopOptions ag_rp_365(PCB_DECL) {
/* Line 4947, RV SIP Stack */
        ParserAuthQopOptions qopOption;
        qopOption.isAuth = RV_TRUE;
        qopOption.isAuthInt = RV_FALSE; 
        qopOption.isOther   = RV_FALSE;
		qopOption.strOther.buf = NULL;
		qopOption.strOther.len = 0;
        RV_UNUSED_ARG(pcb_pointer);
        return qopOption;
    
}
static ParserAuthQopOptions ag_rp_366(PCB_DECL, ParserBasicToken t) {
/* Line 4959, RV SIP Stack */
        ParserAuthQopOptions qopOption;
        qopOption.isOther = RV_TRUE;
        qopOption.isAuthInt = RV_FALSE;
        qopOption.isAuth    = RV_FALSE; 
        qopOption.strOther = t;
        RV_UNUSED_ARG(pcb_pointer);
        return qopOption;
    
}
#define ag_rp_367(PCB_POINTER, ik) (ik)
#define ag_rp_368(PCB_POINTER, ck) (ck)
#define ag_rp_369(PCB_POINTER) (CUR_STRING())
#define ag_rp_370(PCB_POINTER) (CUR_STRING())
#define ag_rp_371(PCB_POINTER) (CUR_STRING())
#define ag_rp_372(PCB_POINTER) (CUR_STRING())
#define ag_rp_373(PCB_POINTER) (CUR_STRING())
static void ag_rp_374(PCB_DECL, ParserBasicToken token1, ParserGenericParamDataVal token2) {
/* Line 5001, RV SIP Stack */
      if (0!=((ParserExtensionString *)PCB.pExtParams)->size)
      {
          
          APPEND_DATA(",", 1,PCB.pExtParams);
          APPEND_DATA(token1.buf, token1.len,PCB.pExtParams);
          APPEND_DATA("=" , 1,PCB.pExtParams);
          APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);            
      }
      else
      {      
          
          APPEND_DATA(token1.buf, token1.len,PCB.pExtParams);
          APPEND_DATA("=" , 1,PCB.pExtParams);
          APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);            
      }
   
}
static void ag_rp_375(PCB_DECL) {
/* Line 5028, RV SIP Stack */
   
          RvStatus status; 
          status = ParserInitAuthorization(PCB.pParserMgr,&PCB,PCB.pointer,&(PCB.authorization),PCB.eHeaderType,PCB.eWhichHeader,PCB.pSipObject);
          if(RV_ERROR_ILLEGAL_SYNTAX == status)
          {
              PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
              SYNTAX_ERROR;
          }
          else if (RV_OK != status)
          {
               
               PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
               PCB.eStat = status;
          }
    
}
static void ag_rp_376(PCB_DECL) {
/* Line 5053, RV SIP Stack */
        
        PCB.authorization.eAuthScheme = RVSIP_AUTH_SCHEME_DIGEST;  
        PCB.authorization.isAuthParamList = RV_FALSE;
    
}
static void ag_rp_377(PCB_DECL, ParserBasicToken authScheme) {
/* Line 5059, RV SIP Stack */
        
        PCB.authorization.eAuthScheme			= RVSIP_AUTH_SCHEME_OTHER;
        PCB.authorization.authScheme			= authScheme; 
        PCB.authorization.isUsername			= RV_FALSE;
        PCB.authorization.isRealm				= RV_FALSE;
        PCB.authorization.isNonce				= RV_FALSE;
        PCB.authorization.isResponse			= RV_FALSE;
        PCB.authorization.algorithm.eAlgorithm	= RVSIP_AUTH_ALGORITHM_UNDEFINED;   
        PCB.authorization.isCnonce				= RV_FALSE;
        PCB.authorization.isOpaque				= RV_FALSE;
        PCB.authorization.isNonceCount			= RV_FALSE;
        PCB.authorization.eAuthQop				= RVSIP_AUTH_QOP_UNDEFINED;
        PCB.authorization.isAuthParam			= RV_FALSE;
		PCB.authorization.isAuts				= RV_FALSE;
		PCB.authorization.algorithm.isAKAVersion= RV_FALSE;
		PCB.authorization.eProtected			= RVSIP_AUTH_INTEGRITY_PROTECTED_UNDEFINED;
        PCB.authorization.isAuthParamList = RV_TRUE;
        PCB.authorization.authParamList = PCB.pExtParams;
    
}
static void ag_rp_378(PCB_DECL) {
/* Line 5089, RV SIP Stack */
      
        PCB.authorization.isUsername			= RV_FALSE;
        PCB.authorization.isRealm				= RV_FALSE;
        PCB.authorization.isNonce				= RV_FALSE;
        PCB.authorization.isResponse			= RV_FALSE;
        PCB.authorization.algorithm.eAlgorithm	= RVSIP_AUTH_ALGORITHM_UNDEFINED;   
        PCB.authorization.isCnonce				= RV_FALSE;
        PCB.authorization.isOpaque				= RV_FALSE;
        PCB.authorization.isNonceCount			= RV_FALSE;
        PCB.authorization.eAuthQop				= RVSIP_AUTH_QOP_UNDEFINED;
        PCB.authorization.isAuthParam			= RV_FALSE;
		PCB.authorization.isAuts				= RV_FALSE;
		PCB.authorization.algorithm.isAKAVersion= RV_FALSE;
		PCB.authorization.eProtected			= RVSIP_AUTH_INTEGRITY_PROTECTED_UNDEFINED;
        switch (PCB.digestChallenge.eType)
        {
             case PARSER_DIGEST_TYPE_USERNAME:
            {
                PCB.authorization.isUsername = RV_TRUE;
                PCB.authorization.userName = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_REALM:
            {
                PCB.authorization.isRealm = RV_TRUE;
                PCB.authorization.realm = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_NONCE:
            {
                PCB.authorization.isNonce = RV_TRUE;
                PCB.authorization.nonce = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_DIGEST_URI:
            {
                PCB.authorization.isUri = RV_TRUE;
                PCB.authorization.hDigestUri = PCB.digestChallenge.uri;
                break;
            }
            case PARSER_DIGEST_TYPE_DIGEST_RESPONSE:
            {
                PCB.authorization.isResponse = RV_TRUE;
                PCB.authorization.response = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_ALGORITHM:
            {
                PCB.authorization.algorithm = PCB.digestChallenge.algorithm;
                break;
            }
            case PARSER_DIGEST_TYPE_CNONCE:
            {
                PCB.authorization.isCnonce = RV_TRUE;
                PCB.authorization.cnonce = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_OPAQUE:
            {
                PCB.authorization.isOpaque = RV_TRUE;
                PCB.authorization.opaque = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_NONCE_COUNT:
            {
                PCB.authorization.isNonceCount = RV_TRUE;
                PCB.authorization.nonceCount = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_MESSAGE_QOP:
            {
                PCB.authorization.eAuthQop = PCB.digestChallenge.qopVal.eAuthQop;
                break;
            }
            case PARSER_DIGEST_TYPE_AUTH_PARAM:
            {
                PCB.authorization.isAuthParam = RV_TRUE;
                PCB.authorization.authParam = PCB.digestChallenge.challenge;
                break;
            }
			case PARSER_DIGEST_TYPE_AUTS:
            {
                PCB.authorization.isAuts = RV_TRUE;
                PCB.authorization.auts = PCB.digestChallenge.challenge;
                break;
            }
			case PARSER_DIGEST_TYPE_INTEGRITY_PROTECTED:
            {
                PCB.authorization.eProtected = PCB.digestChallenge.eProtected;
                break;
            }
            default:
				PCB.exit_flag = AG_SEMANTIC_ERROR_CODE;
				PCB.eStat = RV_ERROR_ILLEGAL_SYNTAX;
        }
    
}
static void ag_rp_379(PCB_DECL) {
/* Line 5188, RV SIP Stack */
        switch (PCB.digestChallenge.eType)
        {
             case PARSER_DIGEST_TYPE_USERNAME:
            {
                PCB.authorization.isUsername = RV_TRUE;
                PCB.authorization.userName = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_REALM:
            {
                PCB.authorization.isRealm = RV_TRUE;
                PCB.authorization.realm = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_NONCE:
            {
                PCB.authorization.isNonce = RV_TRUE;
                PCB.authorization.nonce = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_DIGEST_URI:
            {
                PCB.authorization.isUri = RV_TRUE;
                PCB.authorization.hDigestUri = PCB.digestChallenge.uri;
                break;
            }
            case PARSER_DIGEST_TYPE_DIGEST_RESPONSE:
            {
                PCB.authorization.isResponse = RV_TRUE;
                PCB.authorization.response = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_ALGORITHM:
            {
                PCB.authorization.algorithm = PCB.digestChallenge.algorithm;
                break;
            }
            case PARSER_DIGEST_TYPE_CNONCE:
            {
                PCB.authorization.isCnonce = RV_TRUE;
                PCB.authorization.cnonce = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_OPAQUE:
            {
                PCB.authorization.isOpaque = RV_TRUE;
                PCB.authorization.opaque = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_NONCE_COUNT:
            {
                PCB.authorization.isNonceCount = RV_TRUE;
                PCB.authorization.nonceCount = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_MESSAGE_QOP:
            {
                PCB.authorization.eAuthQop = PCB.digestChallenge.qopVal.eAuthQop;
                break;
            }
            case PARSER_DIGEST_TYPE_AUTH_PARAM:
            {
                PCB.authorization.isAuthParam = RV_TRUE;
                PCB.authorization.authParam = PCB.digestChallenge.challenge;
                break;
            }
			case PARSER_DIGEST_TYPE_AUTS:
            {
                PCB.authorization.isAuts = RV_TRUE;
                PCB.authorization.auts = PCB.digestChallenge.challenge;
                break;
            }
			case PARSER_DIGEST_TYPE_INTEGRITY_PROTECTED:
            {
                PCB.authorization.eProtected = PCB.digestChallenge.eProtected;
                break;
            }
            default:
                    PCB.exit_flag = AG_SEMANTIC_ERROR_CODE;
                    PCB.eStat = RV_ERROR_ILLEGAL_SYNTAX;
        }
    
}
#define ag_rp_380(PCB_POINTER, useranme) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_USERNAME, useranme))
#define ag_rp_381(PCB_POINTER, realm) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_REALM, realm))
#define ag_rp_382(PCB_POINTER, nonce) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_NONCE, nonce))
#define ag_rp_383(PCB_POINTER, response) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_DIGEST_RESPONSE, response))
#define ag_rp_384(PCB_POINTER, cnonce) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_CNONCE, cnonce))
#define ag_rp_385(PCB_POINTER, opaque) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_OPAQUE, opaque))
#define ag_rp_386(PCB_POINTER, nonceCount) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_NONCE_COUNT, nonceCount))
#define ag_rp_387(PCB_POINTER, authParam) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_AUTH_PARAM, authParam))
#define ag_rp_388(PCB_POINTER, auts) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_AUTS, auts))
static void ag_rp_389(PCB_DECL, RvSipAuthIntegrityProtected protected) {
/* Line 5283, RV SIP Stack */
        PCB.digestChallenge.eType = PARSER_DIGEST_TYPE_INTEGRITY_PROTECTED;
        PCB.digestChallenge.eProtected = protected;
    
}
static void ag_rp_390(PCB_DECL, RvSipAuthQopOption messageQop) {
/* Line 5288, RV SIP Stack */
        PCB.digestChallenge.eType = PARSER_DIGEST_TYPE_MESSAGE_QOP;
        PCB.digestChallenge.qopVal.eAuthQop = messageQop;
    
}
static void ag_rp_391(PCB_DECL) {
/* Line 5293, RV SIP Stack */
        PCB.digestChallenge.eType = PARSER_DIGEST_TYPE_DIGEST_URI;
        PCB.digestChallenge.uri = PCB.exUri;
    
}
static void ag_rp_392(PCB_DECL, ParserAlgorithm algorithm) {
/* Line 5298, RV SIP Stack */
        PCB.digestChallenge.eType = PARSER_DIGEST_TYPE_ALGORITHM;
        
        if(PCB.digestChallenge.algorithm.eAlgorithm == RVSIP_AUTH_ALGORITHM_UNDEFINED)
        {
            PCB.digestChallenge.algorithm = algorithm;
        }
    
}
#define ag_rp_393(PCB_POINTER, username) (username)
#define ag_rp_394(PCB_POINTER, t) (t)
static void ag_rp_395(PCB_DECL) {
/* Line 5330, RV SIP Stack */
PCB.authorization.uriEnd = (RvChar *)PCB.pointer;
}
static void ag_rp_396(PCB_DECL) {
/* Line 5336, RV SIP Stack */
 
                SIP_BEGIN_TOKEN();
                PCB.authorization.uriBegin = (RvChar *)PCB.pointer;
              
}
static RvSipAuthQopOption ag_rp_397(PCB_DECL, ParserAuthQopOptions qopVal) {
/* Line 5347, RV SIP Stack */
        RV_UNUSED_ARG(pcb_pointer);
        if (RV_TRUE == qopVal.isAuth)
        {
            return RVSIP_AUTH_QOP_AUTH_ONLY;
        }
        else if (RV_TRUE == qopVal.isAuthInt)
        {
            return RVSIP_AUTH_QOP_AUTHINT_ONLY;
        }
        else if (RV_TRUE == qopVal.isOther)
        {
            return RVSIP_AUTH_QOP_OTHER;
        }
        else 
        {
             return RVSIP_AUTH_QOP_UNDEFINED;
        }
    
}
#define ag_rp_398(PCB_POINTER, cnonceVal) (cnonceVal)
#define ag_rp_399(PCB_POINTER, nonceVal) (nonceVal)
#define ag_rp_400(PCB_POINTER, ncVal) (ncVal)
#define ag_rp_401(PCB_POINTER) (CUR_STRING())
#define ag_rp_402(PCB_POINTER) (CUR_STRING())
#define ag_rp_403(PCB_POINTER) (CUR_STRING())
#define ag_rp_404(PCB_POINTER) (CUR_STRING())
#define ag_rp_405(PCB_POINTER) (CUR_STRING())
#define ag_rp_406(PCB_POINTER) (CUR_STRING())
#define ag_rp_407(PCB_POINTER) (CUR_STRING())
#define ag_rp_408(PCB_POINTER) (CUR_STRING())
#define ag_rp_409(PCB_POINTER, requestDigest) (requestDigest)
#define ag_rp_410(PCB_POINTER) (CUR_STRING())
#define ag_rp_411(PCB_POINTER) (CUR_STRING())
#define ag_rp_412(PCB_POINTER, autsValue) (autsValue)
#define ag_rp_413(PCB_POINTER) (CUR_STRING())
#define ag_rp_414(PCB_POINTER) (RVSIP_AUTH_INTEGRITY_PROTECTED_NO)
#define ag_rp_415(PCB_POINTER) (RVSIP_AUTH_INTEGRITY_PROTECTED_YES)
static void ag_rp_416(PCB_DECL) {
/* Line 5466, RV SIP Stack */
         RvStatus status;
         status = ParserInitRoute(PCB.pParserMgr,&PCB,PCB.pointer,&(PCB.route), PCB.eHeaderType, PCB.pSipObject);
         if(RV_ERROR_ILLEGAL_SYNTAX == status)
         {
             PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
             SYNTAX_ERROR;
         }
         else if (RV_OK != status)
         {
             
             PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
             PCB.eStat = status;
         }
    
}
static void ag_rp_417(PCB_DECL) {
/* Line 5482, RV SIP Stack */
 
        RvStatus status;
        status = ParserInitRoute(PCB.pParserMgr,&PCB,PCB.pointer,&(PCB.route), PCB.eHeaderType, PCB.pSipObject);
        if(RV_ERROR_ILLEGAL_SYNTAX == status)
        {
             PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
             SYNTAX_ERROR;
        }
        else if (RV_OK != status)
        {
             
             PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
             PCB.eStat = status;
        }
    
}
static void ag_rp_418(PCB_DECL) {
/* Line 5502, RV SIP Stack */
        PCB.route.nameAddr = PCB.nameAddr;
        PCB.route.isParams = RV_FALSE;
        PCB.route.eRouteType = PCB.eWhichHeader;
    
}
static void ag_rp_419(PCB_DECL) {
/* Line 5507, RV SIP Stack */
 
        PCB.route.nameAddr = PCB.nameAddr;
        PCB.route.isParams = RV_TRUE;
        PCB.route.eRouteType = PCB.eWhichHeader;
          
         if (((ParserExtensionString *)PCB.pExtParams)->size !=0)
         {
             PCB.route.routeParams = PCB.pExtParams;
         }                
     
}
static void ag_rp_420(PCB_DECL) {
/* Line 6728, RV SIP Stack */
   
		RvStatus status; 
		status = ParserInitAuthenticationInfo(PCB.pParserMgr,PCB.pointer,&(PCB.auth_info),PCB.eHeaderType,PCB.pSipObject);
		if(RV_ERROR_ILLEGAL_SYNTAX == status)
        {
            PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
            SYNTAX_ERROR;
        }
        else if (RV_OK != status)
		{
			
			PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
			PCB.eStat = status;
		}
		
    
}
static void ag_rp_421(PCB_DECL, ParserBasicToken nc) {
/* Line 6752, RV SIP Stack */
		PCB.auth_info.isNonceCount  = RV_TRUE;
		PCB.auth_info.strNonceCount = nc;
	
}
static void ag_rp_422(PCB_DECL, ParserBasicToken nv) {
/* Line 6757, RV SIP Stack */
		PCB.auth_info.isNextNonce   = RV_TRUE;
		PCB.auth_info.strNextNonce  = nv;
	
}
static void ag_rp_423(PCB_DECL, ParserBasicToken cn) {
/* Line 6762, RV SIP Stack */
		PCB.auth_info.isCNonce      = RV_TRUE;
		PCB.auth_info.strCNonce     = cn;
	
}
static void ag_rp_424(PCB_DECL, ParserBasicToken rd) {
/* Line 6767, RV SIP Stack */
		PCB.auth_info.isRspAuth     = RV_TRUE;
		PCB.auth_info.strRspAuth    = rd;
	
}
static void ag_rp_425(PCB_DECL) {
/* Line 6772, RV SIP Stack */
		PCB.auth_info.isMsgQop      = RV_TRUE;
	
}
static void ag_rp_426(PCB_DECL, ParserAuthQopOptions qopVal) {
/* Line 6780, RV SIP Stack */
		if (RV_TRUE == qopVal.isAuth)
        {
            PCB.auth_info.eMsgQop = RVSIP_AUTH_QOP_AUTH_ONLY;
        }
        else if (RV_TRUE == qopVal.isAuthInt)
        {
            PCB.auth_info.eMsgQop = RVSIP_AUTH_QOP_AUTHINT_ONLY;
        }
        else if (RV_TRUE == qopVal.isOther)
        {
            PCB.auth_info.eMsgQop = RVSIP_AUTH_QOP_OTHER;
        }
        else 
        {
	    PCB.auth_info.eMsgQop = RVSIP_AUTH_QOP_UNDEFINED;
        }
		
		PCB.auth_info.strMsgQop = qopVal.strOther;
	
}
#define ag_rp_427(PCB_POINTER, nv) (nv)
#define ag_rp_428(PCB_POINTER, val) (val)
#define ag_rp_429(PCB_POINTER) (CUR_STRING())
#define READ_COUNTS 
#define WRITE_COUNTS 
#undef V
#define V(i,t) (*(t *) (&(PCB).vs[(PCB).ssx + i]))
#undef VS
#define VS(i) (PCB).vs[(PCB).ssx + i]
#ifndef GET_CONTEXT
#define GET_CONTEXT CONTEXT = (PCB).input_context
#endif
typedef enum {
  ag_action_1,
  ag_action_2,
  ag_action_3,
  ag_action_4,
  ag_action_5,
  ag_action_6,
  ag_action_7,
  ag_action_8,
  ag_action_9,
  ag_action_10,
  ag_action_11,
  ag_action_12
} ag_parser_action;
#ifndef NULL_VALUE_INITIALIZER
#define NULL_VALUE_INITIALIZER = { 0 }
#endif
static SipParser_vs_type const ag_null_value NULL_VALUE_INITIALIZER;
static const unsigned short ag_rpx[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  1,  2,  3,  4,  0,  0,  5,  6,  7,  8,  9, 10, 11,
   12, 13, 14, 15,  0,  0, 16,  0,  0, 17,  0,  0, 18,  0,  0,  0,  0,  0,
   19,  0,  0,  0,  0,  0, 20,  0,  0,  0,  0, 21, 22, 23, 24, 25, 26, 27,
   28, 29,  0,  0, 30, 31, 32, 33, 34, 35, 36, 37, 38,  0, 39,  0, 40, 41,
   42, 43, 44,  0,  0, 45, 46, 47, 48,  0,  0, 49, 50, 51,  0,  0,  0,  0,
    0,  0, 52, 53,  0, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66,
   67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81,  0, 82, 83,
   84, 85,  0,  0, 86, 87, 88, 89, 90, 91,  0, 92, 93, 94, 95, 96, 97, 98,
   99,100,101,102,  0,103,104,  0,  0,105,106,  0,  0,107,  0,  0,108,109,
    0,110,111,112,113,114,  0,  0,  0,  0,  0,  0,  0,  0,  0,115,  0,  0,
    0,  0,  0,  0,  0,  0,  0,116,  0,117,118,119,120,121,122,123,124,125,
    0,  0,126,127,  0,  0,128,  0,  0,  0,129,  0,  0,130,  0,  0,  0,  0,
  131,132,  0,  0,  0,  0,  0,  0,  0,133,  0,  0,  0,  0,  0,134,135,  0,
    0,  0,136,137,138,139,  0,140,141,142,143,144,145,146,147,148,149,150,
    0,  0,  0,  0,  0,151,152,153,154,155,  0,156,157,  0,158,159,160,161,
  162,163,164,  0,  0,165,166,  0,  0,  0,  0,  0,  0,167,168,169,170,171,
  172,173,  0,  0,174,175,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,176,177,178,179,180,181,182,183,184,
  185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,
    0,  0,  0,  0,203,204,205,  0,  0,  0,  0,  0,206,207,208,209,210,211,
  212,213,214,215,216,217,  0,  0,218,  0,  0,  0,  0,  0,219,220,221,222,
  223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,
  241,242,243,244,245,246,247,248,249,  0,  0,  0,  0,  0,250,251,252,253,
  254,255,256,  0,257,  0,  0,258,259,260,261,262,263,  0,  0,264,265,266,
  267,268,  0,  0,269,270,  0,  0,  0,  0,271,272,273,274,  0,275,276,277,
  278,279,280,281,282,283,284,285,286,287,288,289,290,291,292,293,294,295,
  296,297,298,299,300,301,302,303,304,305,306,307,308,309,310,311,312,313,
  314,315,316,317,318,  0,319,320,321,322,323,324,325,326,327,328,329,330,
  331,332,333,334,335,  0,  0,336,337,  0,  0,338,  0,  0,339,340,341,  0,
    0,  0,342,343,344,345,346,347,  0,  0,348,349,350,351,352,353,354,355,
  356,357,358,359,360,361,362,363,364,365,366,367,368,  0,  0,369,370,371,
  372,373,  0,  0,  0,  0,374,375,  0,376,377,378,379,380,381,382,383,384,
  385,386,387,388,389,390,391,392,393,394,  0,395,396,  0,  0,397,398,399,
  400,401,402,403,404,405,406,407,408,409,410,411,  0,  0,  0,412,  0,  0,
    0,  0,413,  0,  0,414,415,  0,416,417,418,419,420,  0,  0,421,422,423,
  424,425,426,427,428,  0,  0,  0,  0,  0,429
};
static const unsigned char ag_key_itt[] = {
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0
};
static const unsigned short ag_key_pt[] = {
  0,  2,  0,  6,  0,  8,  0, 10,  0, 12,  0, 14,  0, 16,  0, 18,
  0, 20,  0, 22,  0, 24,  0, 26,  0, 30,  0, 93,  0, 94,  0, 95,
  0, 96,  0, 97,  0,134,  0,136,  0,137,  0,138,  0,139,  0,154,
  0,155,  0,157,  0,161,  0,163,  0,164,  0,166,  0,167,  0,169,
  0,170,  0,218,  0,221,  0,227,  0,235,  0,251,  0,318,  0,319,
  0,322,  0,323,  0,324,  0,325,  0,326,  0,327,  0,328,  0,335,
  0,336,  0,352,  0,353,  0,354,  0,355,  0,356,  0,357,  0,358,
  0,359,  0,360,  0,361,  0,362,  0,363,  0,364,  0,373,  0,378,
  0,385,  0,391,  0,393,  0,394,  0,396,  0,404,  0,405,  0,406,
  0,407,  0,408,  0,409,  0,410,  0,414,  0,415,  0,416,  0,417,
  0,418,  0,419,  0,420,  0,421,  0,422,  0,423,  0,424,  0,425,
  0,430,  0,431,  0,432,  0,435,  0,450,  0,452,  0,464,  0,466,
  0,468,  0,469,  0,473,  0,482,  0,485,  0,488,  0,489,  0,490,
  0,492,  0,507,  0,509,  0,513,  0,515,  0,518,  0,523,  0,531,
  0,532,  0,533,  0,541,  0,542,0
};
static const unsigned char ag_key_ch[] = {
    0, 49, 50, 67, 73,255, 89,255, 65, 73, 76, 79, 83,255, 77, 78,255, 69,
  255, 82,255, 69, 72,255, 47, 58, 83,255, 80,255, 73,255, 69,255, 82,255,
   73,255, 65, 66, 67, 68, 69, 70, 73, 75, 76, 77, 79, 80, 82, 83, 84, 85,
   86, 88,255, 32,255, 72, 85,255, 72, 82, 85,255, 75, 84,255, 58, 82,255,
   79,255, 71, 73,255, 45,255, 72, 83,255, 71, 84,255, 67, 75, 76, 80, 85,
  255, 82, 89,255, 65, 75, 78, 79,255, 69, 73, 79,255, 80, 84,255, 78, 88,
  255, 65, 69, 82,255, 83, 84,255, 77,255, 84, 86,255, 75, 77, 78, 80, 83,
  255, 76, 78,255, 65, 85,255, 68, 82, 89,255, 65, 69, 79,255, 78, 86,255,
   67, 69, 79,255, 67, 80, 82,255, 45,255, 69,255, 78,255, 79,255, 69, 79,
  255, 72, 79, 82,255, 79,255, 65, 67, 68, 71, 83,255, 69, 80, 83,255, 45,
  255, 80,255, 77,255, 79,255, 67,255, 58, 83,255, 71, 80,255, 65, 67, 69,
   73, 84, 85,255, 65, 85,255, 65, 67, 69, 72, 76, 79, 82, 84, 85,255, 78,
  255, 82,255, 69,255, 68, 82, 83, 84,255, 42, 58, 65, 66, 67, 68, 69, 70,
   71, 72, 73, 74, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 87, 89,255, 58,
   83,255, 80,255, 73,255, 73, 80, 83, 84,255, 67, 69,255, 58, 83,255, 80,
  255, 65, 73, 85,255, 69, 72, 85,255, 42, 67, 68, 70, 73, 77, 78, 80, 81,
   82, 83, 84, 87,255, 67, 69,255, 67, 78, 81, 82,255, 68,255, 83,255, 58,
   83,255, 80,255, 73,255, 42, 73, 80, 83, 84,255, 65, 85,255, 72, 85,255,
   70, 77, 83, 84, 87,255, 78, 88,255, 72, 79,255, 69, 73, 80,255, 58,255,
   65, 69,255, 79, 82, 84,255, 67, 76, 77, 79, 83, 84, 85,255, 83,255, 76,
   85,255, 67, 79,255, 65, 83,255, 69,255, 82, 83,255, 65, 67, 73, 78, 79,
   81, 82, 85,255, 65, 67, 68, 73, 78, 79, 81, 82, 83,255, 65, 69, 81,255,
   80, 85,255, 76, 78,255, 65, 85,255, 82, 89,255, 65,255, 65, 68, 70, 74,
   77, 78, 79, 83,255, 84,255, 65, 66, 67, 73, 82,255, 45,255, 72,255, 84,
  255, 85,255, 65,255, 75, 76,255, 65,255, 32, 49, 79, 84,255, 77,255, 83,
  255, 73, 80,255, 67, 76,255, 83, 84, 85,255, 78, 89,255, 70, 84,255, 69,
   80,255, 65, 66, 67, 72, 77, 82, 83, 84,255, 80, 82,255, 58, 83,255, 80,
  255, 73,255, 83,255, 61,255, 61,255, 83, 84,255, 77,255, 71, 85,255
};
static const unsigned char ag_key_act[] = {
  0,5,5,7,5,4,7,4,7,5,5,5,5,4,3,7,4,3,4,6,4,7,5,4,3,0,3,4,2,4,2,4,3,4,5,
  4,5,4,2,6,6,7,7,5,6,5,5,5,5,2,6,2,6,6,6,5,4,0,4,3,3,4,3,3,3,4,5,7,4,0,
  7,4,2,4,2,7,4,7,4,6,5,4,5,2,4,2,3,2,7,2,4,7,7,4,7,5,7,7,4,7,7,7,4,7,5,
  4,7,2,4,7,7,7,4,7,5,4,2,4,7,7,4,5,3,2,5,7,4,5,5,4,7,2,4,7,5,5,4,2,7,7,
  4,7,5,4,5,7,6,4,7,7,7,4,7,4,6,4,2,4,2,4,3,7,4,2,7,2,4,7,4,7,7,7,7,7,4,
  2,7,7,4,7,4,6,4,2,4,2,4,2,4,0,3,4,2,6,4,7,7,7,2,7,7,4,7,7,4,7,7,3,7,7,
  7,2,7,7,4,7,4,6,4,2,4,7,7,2,7,4,5,3,2,2,2,2,2,2,2,7,2,2,7,2,2,2,2,6,2,
  2,2,2,7,7,4,0,3,4,2,4,2,4,3,3,2,3,4,5,7,4,0,3,4,6,4,7,2,7,4,3,7,7,4,5,
  7,7,7,3,7,2,3,7,7,2,2,7,4,5,7,4,7,2,7,7,4,7,4,7,4,0,3,4,2,4,2,4,5,3,3,
  2,3,4,7,7,4,7,7,4,7,7,2,2,7,4,7,7,4,7,7,4,2,7,2,4,3,4,7,7,4,7,7,7,4,7,
  7,2,7,7,2,7,4,3,4,7,7,4,5,7,4,7,7,4,2,4,7,7,4,2,7,7,2,7,7,2,2,4,7,7,7,
  7,7,7,7,7,7,4,7,7,5,4,7,7,4,5,5,4,7,2,4,5,5,4,2,4,2,7,7,2,2,7,7,7,4,7,
  4,7,7,7,7,7,4,7,4,6,4,2,4,2,4,2,4,3,3,4,2,4,0,5,7,7,4,7,4,7,4,7,7,4,7,
  7,4,7,2,7,4,7,7,4,7,7,4,7,7,4,7,7,7,7,7,2,7,7,4,7,7,4,0,3,4,2,4,2,4,2,
  4,0,4,1,4,7,5,4,2,4,2,7,4
};
static const unsigned short ag_key_parm[] = {
    0, 18, 20, 28, 22,  0, 30,  0, 32, 10, 12,  8,  0,  0,264, 26,  0,262,
    0,  2,  0, 34, 16,  0,119,257,258,  0,  0,  0,  0,  0,190,  0, 24,  0,
   14,  0,  0,112,102,  6,  4,108,100,114,122, 98,118,  0,110,  0,106,120,
  104,116,  0,463,  0,290,278,  0,290,261,278,  0, 28,130,  0,484,196,  0,
    0,  0,  0, 48,  0,202,  0,204,220,  0,166,  0,  0,  0,483,  0,158,  0,
    0, 58, 30,  0, 32,208,214, 60,  0,174,182,186,  0,136, 66,  0, 74,  0,
    0,194,154,148,  0,180,176,  0,  0,  0,222, 26,  0,206,264,  0, 82, 70,
    0,164,162,  0,152,  0,  0, 54,156,160,  0,  0, 84,140,  0,188,172,  0,
  216,228,224,  0,170,190, 96,  0, 72,  0, 80,  0,  0,  0,  0,  0,262,132,
    0,  0, 68,  0,  0,200,  0,184, 56,134, 34,218,  0,  0, 52,230,  0, 64,
    0, 62,  0,  0,  0,  0,  0,  0,  0,257,258,  0,  0, 36,  0,150, 42,168,
    0,192,138,  0, 76, 90,  0,124, 40,190,146, 44, 94,  0, 50,142,  0,210,
    0, 78,  0,  0,  0, 38,212,  0,178,  0,126,300,  0,  0,  0,  0,  0,  0,
    0, 46,  0,  0, 86,  0,  0,  0,  0,128,  0,  0,  0,  0,144,226,  0,257,
  258,  0,  0,  0,  0,  0,264,262,  0,190,  0,216,228,  0,257,258,  0, 36,
    0,150,  0,138,  0,190,146,142,  0,126,214,182,148,264,140,  0,262,200,
  230,  0,  0,144,  0,216,228,  0,214,  0,200,230,  0,182,  0, 36,  0,257,
  258,  0,  0,  0,  0,  0,126,264,262,  0,190,  0,150,138,  0,146,142,  0,
  148,140,  0,  0,144,  0, 74, 66,  0, 72, 68,  0,  0, 70,  0,  0,300,  0,
   54, 84,  0, 94, 76, 50,  0, 60, 86,  0, 96, 64,  0, 78,  0,119,  0,196,
  220,  0,216,188,  0,184,218,  0,  0,  0,212,210,  0,  0,214,222,  0,190,
  200,  0,  0,  0,196,208,186,206,188,190,200,184,192,  0,130,136,128,  0,
  158,166,  0,164,162,  0,152,  0,  0,156,160,  0,  0,  0,  0,174,154,  0,
    0,172,170,168,  0,124,  0, 28, 30, 32, 26, 34,  0,202,  0,204,  0,  0,
    0,  0,  0,  0,  0,483,484,  0,  0,  0,463, 88, 92, 90,  0,198,  0, 62,
    0, 82, 80,  0, 40, 44,  0, 42,  0, 38,  0,224,226,  0,194, 90,  0, 56,
   52,  0, 48, 58, 60, 46, 54,  0, 64, 50,  0,132,134,  0,257,258,  0,  0,
    0,  0,  0,  0,  0,530,  0,529,  0,180,176,  0,  0,  0,  0,178,  0
};
static const unsigned short ag_key_jmp[] = {
    0,  0,  0,  0,  0,  0,  2,  0,  4,  0,  0,  0,  0,  0, 13, 15,  0, 20,
    0, 17,  0, 23,  0,  0, 30,  0, 34,  0, 24,  0, 28,  0, 36,  0,  0,  0,
    0,  0,  1,  6,  8,  9, 11,  0, 14,  0,  0,  0,  0, 19, 21, 30, 32, 34,
   36,  0,  0,  0,  0, 39, 44,  0, 53, 58, 65,  0,  0, 76,  0,  0, 86,  0,
   69,  0, 72, 91,  0, 96,  0, 77,  0,  0,  0, 79,  0, 66, 80, 74, 94, 82,
    0,100,105,  0,107,  0,112,117,  0,120,122,127,  0,137,  0,  0,132,103,
    0,142,146,148,  0,150,  0,  0,113,  0,160,177,  0,  0,158,118,  0,181,
    0,  0,  0,  0,184,127,  0,188,  0,  0,  0,133,191,196,  0,206,  0,  0,
    0,198,141,  0,209,211,216,  0,219,  0,152,  0,154,  0,156,  0,231,234,
    0,158,227,160,  0,237,  0,239,242,248,254,260,  0,169,266,270,  0,283,
    0,179,  0,181,  0,183,  0,185,  0,  0,286,  0,187,189,  0,276,278,281,
  192,288,292,  0,316,323,  0,294,296,298,301,303,305,202,325,327,  0,333,
    0,215,  0,217,  0,329,331,219,337,  0,  0, 74, 85, 91, 94, 99,106,109,
  116,152,121,130,186,137,144,148,163,167,175,195,205,221,339,342,  0,  0,
  353,  0,251,  0,254,  0,345,348,256,355,  0,  0,380,  0,  0,405,  0,266,
    0,403,269,407,  0,409,412,414,  0,  0,359,365,371,374,377,263,388,393,
  396,271,275,416,  0,  0,425,  0,419,293,433,436,  0,443,  0,449,  0,  0,
  460,  0,305,  0,308,  0,  0,452,455,310,462,  0,472,474,  0,476,478,  0,
  466,469,318,321,480,  0,483,488,  0,494,506,  0,330,490,333,  0,510,  0,
  518,522,  0,541,552,560,  0,512,516,342,527,531,345,562,  0,566,  0,573,
  581,  0,  0,609,  0,622,625,  0,365,  0,631,633,  0,359,584,590,362,613,
  619,368,370,  0,640,649,651,657,659,664,670,673,678,  0,683,689,  0,  0,
  696,698,  0,  0,  0,  0,706,399,  0,  0,  0,  0,405,  0,396,700,703,402,
  408,708,711,714,  0,717,  0,720,723,726,732,738,  0,746,  0,427,  0,429,
    0,431,  0,433,  0,750,756,  0,437,  0,  0,  0,760,762,  0,766,  0,769,
    0,776,778,  0,787,789,  0,783,454,791,  0,794,796,  0,799,804,  0,834,
  841,  0,808,813,819,823,829,467,845,855,  0,858,863,  0,  0,871,  0,482,
    0,485,  0,487,  0,  0,  0,491,  0,873,  0,  0,495,  0,498,875,  0
};
static const unsigned short ag_key_index[] = {
   38, 57,  0, 59, 62, 62,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 57, 57,
    0,  0,226,  0,258,258,  0,279,  0,296,301,301,  0,303,  0,  0,312,324,
  324,258,  0,  0,  0,  0,  0,336,  0,  0, 57,340, 57,  0,  0,  0,  0,349,
   57, 57, 57,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,258,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0, 62, 62,  0, 59,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,336,
  336, 57,340,340,  0,340,  0,  0,  0,  0,  0,349,349,  0, 57,  0,  0,357,
    0,  0,  0,  0,  0,  0,  0,  0,  0,373,  0,382,  0,  0,  0,  0,  0,258,
  258,  0,  0,  0,  0,303,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,392,  0,
    0,258,  0,410,  0,419,  0,  0,421, 57, 57,  0,  0,  0,  0,336,340,  0,
  340,  0,  0, 57,  0,  0,  0,  0, 57,  0,  0,  0,  0,  0,349,  0,  0,  0,
    0,  0,  0,  0,435,  0,  0,  0,  0,296,  0,  0,  0,440,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  258,  0,  0,  0,  0,  0,  0,303,  0,  0,  0,  0,  0,  0,  0,  0,392,392,
    0,258,  0,  0,  0,  0,419,419,  0, 57,  0,  0,  0,  0,  0,340,  0,  0,
    0,  0,  0, 57,442,  0,  0,  0,  0,  0, 57,  0,  0,435,  0,  0,  0,  0,
  296,  0,  0,  0,447,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  373,  0,  0,  0,  0,  0,  0,382,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,392,  0,  0,  0,  0,  0,419,  0,  0,  0,  0,  0,  0,  0,
    0,  0,449,421,  0,  0,451,457, 57,435,  0,  0,  0,  0,440,  0,  0,  0,
  435,461,  0,  0,  0,  0,  0,  0,  0,  0,373,  0,464,  0,  0,  0,  0,382,
    0,  0,  0,  0,  0,470,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,449,421,  0,  0,451,457,  0,  0,  0,  0,  0,440,  0,  0,
    0,  0,435,461,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,464,  0,  0,
    0,  0,  0,  0,  0,470,  0,  0,479,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0, 57, 57,  0,  0,  0,  0,447,258,435,  0,  0,  0,  0,  0,
  435,  0,  0,489,  0,457,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,479,  0,  0,  0,  0,  0,  0,  0,  0,  0, 57,  0,  0,  0,493,  0,  0,
    0,  0,  0,489,489,  0,457,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,324,
    0,  0,500,  0,  0,  0,  0,  0, 57,  0,  0,  0,  0,  0,  0,449,  0,  0,
  340,324,  0,  0,  0,435,489,  0,  0,  0,  0,  0,449,  0,340,  0,  0,  0,
    0,435,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0
};
static const unsigned char ag_key_ends[] = {
75,0, 69,0, 78,67,69,76,0, 84,0, 88,0, 58,0, 86,73,84,69,0, 
83,58,0, 71,73,83,84,69,82,0, 50,46,48,0, 58,0, 76,58,0, 
79,83,84,58,0, 83,69,82,73,78,70,79,58,0, 79,83,84,58,0, 
86,45,65,66,83,58,0, 83,69,82,73,78,70,79,58,0, 58,0, 73,79,78,0, 
65,86,69,82,58,0, 73,84,72,77,0, 65,83,0, 82,0, 73,78,84,0, 
65,78,67,72,0, 69,0, 78,67,69,76,0, 79,78,67,69,0, 77,80,0, 
67,0, 71,69,83,84,0, 77,65,73,78,0, 85,77,68,73,0, 73,82,69,83,0, 
76,83,69,0, 66,0, 73,0, 84,0, 73,68,68,69,78,0, 58,0, 
69,71,82,73,84,89,45,80,82,79,84,69,67,84,69,68,0, 73,84,69,0, 
85,66,0, 78,0, 82,0, 68,82,0, 84,72,79,68,0, 78,0, 
88,84,78,79,78,67,69,0, 67,69,0, 84,0, 65,81,85,69,0, 73,71,0, 
67,79,78,84,69,88,84,0, 83,84,68,0, 83,58,0, 88,89,0, 80,0, 
76,77,0, 69,73,86,69,68,0, 73,82,69,67,84,0, 73,83,84,69,82,0, 
80,79,78,83,69,0, 79,82,84,0, 80,65,85,84,72,0, 84,0, 84,80,0, 
80,0, 73,68,0, 58,0, 65,76,69,0, 78,0, 71,0, 80,0, 76,58,0, 
85,0, 83,0, 75,69,78,73,90,69,68,45,66,89,0, 78,83,80,79,82,84,0, 
69,0, 76,0, 69,0, 80,0, 73,0, 65,77,69,0, 67,0, 69,68,0, 
69,83,0, 77,58,0, 82,69,83,58,0, 58,0, 69,76,58,0, 
78,79,78,67,69,0, 73,71,69,83,84,0, 82,73,0, 77,58,0, 79,78,0, 
88,84,78,79,78,67,69,0, 82,69,83,58,0, 79,80,0, 
83,80,65,85,84,72,0, 84,0, 58,0, 78,0, 76,58,0, 85,0, 69,0, 
69,68,0, 78,79,78,67,69,0, 88,84,78,79,78,67,69,0, 79,80,0, 
83,80,65,85,84,72,0, 73,71,69,83,84,0, 73,80,0, 77,58,0, 
82,69,83,58,0, 58,0, 69,76,58,0, 82,73,0, 79,78,0, 84,0, 78,0, 
85,0, 69,0, 69,68,0, 85,77,68,73,0, 84,0, 83,85,66,0, 
79,78,69,45,67,79,78,84,69,88,84,0, 83,84,68,0, 58,0, 79,77,80,0, 
82,0, 68,68,82,0, 84,72,79,68,0, 82,73,71,0, 
73,71,67,79,77,80,45,73,68,0, 75,69,78,73,90,69,68,45,66,89,0, 
65,78,83,80,79,82,84,0, 76,0, 83,69,82,0, 73,80,47,50,46,48,0, 
71,79,82,73,84,72,77,0, 84,83,0, 78,79,78,67,69,0, 
78,84,69,71,82,73,84,89,45,80,82,79,84,69,67,84,69,68,0, 78,67,69,0, 
80,65,81,85,69,0, 79,80,0, 76,77,0, 80,79,78,83,69,0, 73,0, 
69,82,78,65,77,69,0, 76,71,79,82,73,84,72,77,0, 75,0, 
79,77,65,73,78,0, 75,0, 79,78,67,69,0, 80,65,81,85,69,0, 79,80,0, 
69,65,76,77,0, 84,65,76,69,0, 67,84,73,79,78,0, 
88,80,73,82,69,83,0, 82,0, 71,0, 69,67,0, 69,66,0, 78,0, 
79,86,0, 67,84,0, 69,80,0, 65,71,0, 67,75,0, 89,69,0, 
65,78,67,69,76,0, 78,86,73,84,69,0, 69,71,73,83,84,69,82,0, 
73,78,84,0, 65,86,69,82,58,0, 71,79,58,0, 78,0, 82,85,69,0, 
68,53,0, 73,71,67,79,77,80,0, 80,0, 72,79,78,69,0, 67,84,80,0, 
80,0, 83,0, 68,80,0, 79,0, 69,83,0, 65,76,83,69,0, 82,85,69,0, 
76,73,65,83,0, 82,65,78,67,72,0, 79,77,80,0, 73,68,68,69,78,0, 
65,68,68,82,0, 67,69,73,86,69,68,0, 79,82,84,0, 
73,71,67,79,77,80,45,73,68,0, 84,76,0, 82,79,88,89,0, 
69,68,73,82,69,67,84,0, 58,0, 84,0, 84,67,0, 
};
#define AG_TCV(x) ag_tcv[(x)]
static const unsigned short ag_tcv[] = {
    5,562,562,562,562,562,562,562,562,547,  0,562,562,  0,562,562,562,562,
  562,562,562,562,562,562,562,562,562,562,562,562,562,562, 41,548, 63,559,
  560,234,341,548,556,556,549,196, 43, 65,249, 57,387,390,550,550,550,550,
  550,550,550,550, 37, 53, 59, 48, 61, 55, 50,479,551,551,551,552,552,553,
  553,553,553,480,553,553,553,553,226,553,553,553,553,553,481,226,553,553,
  553,294, 87,295,561,554,555,479,551,551,551,552,552,553,553,553,553,480,
  553,553,553,553,226,553,553,553,553,553,481,226,553,553,553,557,561,557,
  548,562,561,561,561,561,561,561,561,561,561,561,561,561,561,561,561,561,
  561,558,558,561,561,561,561,561,558,561,561,561,561,561,561,561,561,561,
  561,561,561,561,561,561,561,561,561,561,561,561,561,561,561,561,561,561,
  561,561,561,561,561,561,561,561,561,561,561,561,561,561,561,561,561,561,
  561,561,561,561,561,561,561,561,561,561,561,561,561,561,561,561,561,561,
  561,561,561,561,561,561,561,561,561,561,561,561,561,561,561,561,561,561,
  561,561,561,561,561,561,561,561,561,561,561,561,561,561,561,561,561,561,
  561,561,561,561
};
#ifndef SYNTAX_ERROR
#define SYNTAX_ERROR fprintf(stderr,"%s, line %d, column %d\n", \
  (PCB).error_message, (PCB).line, (PCB).column)
#endif
#ifndef FIRST_LINE
#define FIRST_LINE 1
#endif
#ifndef FIRST_COLUMN
#define FIRST_COLUMN 1
#endif
#ifndef PARSER_STACK_OVERFLOW
#define PARSER_STACK_OVERFLOW {fprintf(stderr, \
   "\nParser stack overflow, line %d, column %d\n",\
   (PCB).line, (PCB).column);}
#endif
#ifndef REDUCTION_TOKEN_ERROR
#define REDUCTION_TOKEN_ERROR {fprintf(stderr, \
    "\nReduction token error, line %d, column %d\n", \
    (PCB).line, (PCB).column);}
#endif
#ifndef INPUT_CODE
#define INPUT_CODE(T) (T)
#endif
typedef enum
  {ag_accept_key, ag_set_key, ag_jmp_key, ag_end_key, ag_no_match_key,
   ag_cf_accept_key, ag_cf_set_key, ag_cf_end_key} key_words;
static void ag_get_key_word(PCB_DECL, int ag_k) {
  int ag_save = (int) ((PCB).la_ptr - (PCB).pointer);
  const  unsigned char *ag_p;
  int ag_ch;
  while (1) {
    switch (ag_key_act[ag_k]) {
    case ag_cf_end_key: {
      const  unsigned char *sp = ag_key_ends + ag_key_jmp[ag_k];
      do {
        if ((ag_ch = *sp++) == 0) {
          int ag_k1 = ag_key_parm[ag_k];
          int ag_k2 = ag_key_pt[ag_k1];
          if (ag_key_itt[ag_k2 + CONVERT_CASE(*(PCB).la_ptr)]) goto ag_fail;
          (PCB).token_number = (SipParser_token_type) ag_key_pt[ag_k1 + 1];
          return;
        }
      } while (CONVERT_CASE(*(PCB).la_ptr++) == ag_ch);
      goto ag_fail;
    }
    case ag_end_key: {
      const  unsigned char *sp = ag_key_ends + ag_key_jmp[ag_k];
      do {
        if ((ag_ch = *sp++) == 0) {
          (PCB).token_number = (SipParser_token_type) ag_key_parm[ag_k];
          return;
        }
      } while (CONVERT_CASE(*(PCB).la_ptr++) == ag_ch);
    }
    case ag_no_match_key:
ag_fail:
      (PCB).la_ptr = (PCB).pointer + ag_save;
      return;
    case ag_cf_set_key: {
      int ag_k1 = ag_key_parm[ag_k];
      int ag_k2 = ag_key_pt[ag_k1];
      ag_k = ag_key_jmp[ag_k];
      if (ag_key_itt[ag_k2 + CONVERT_CASE(*(PCB).la_ptr)]) break;
      ag_save = (int) ((PCB).la_ptr - (PCB).pointer);
      (PCB).token_number = (SipParser_token_type) ag_key_pt[ag_k1+1];
      break;
    }
    case ag_set_key:
      ag_save = (int) ((PCB).la_ptr - (PCB).pointer);
      (PCB).token_number = (SipParser_token_type) ag_key_parm[ag_k];
    case ag_jmp_key:
      ag_k = ag_key_jmp[ag_k];
      break;
    case ag_accept_key:
      (PCB).token_number = (SipParser_token_type) ag_key_parm[ag_k];
      return;
    case ag_cf_accept_key: {
      int ag_k1 = ag_key_parm[ag_k];
      int ag_k2 = ag_key_pt[ag_k1];
      if (ag_key_itt[ag_k2 + CONVERT_CASE(*(PCB).la_ptr)])
        (PCB).la_ptr = (PCB).pointer + ag_save;
      else (PCB).token_number = (SipParser_token_type) ag_key_pt[ag_k1+1];
      return;
    }
    }
    ag_ch = CONVERT_CASE(*(PCB).la_ptr++);
    ag_p = &ag_key_ch[ag_k];
    while (*ag_p < ag_ch) ag_p++;
    if (*ag_p != ag_ch) {
      (PCB).la_ptr = (PCB).pointer + ag_save;
      return;
    }
    ag_k = (int) (ag_p - ag_key_ch);
  }
}
#ifndef AG_NEWLINE
#define AG_NEWLINE 10
#endif
#ifndef AG_RETURN
#define AG_RETURN 13
#endif
#ifndef AG_FORMFEED
#define AG_FORMFEED 12
#endif
#ifndef AG_TABCHAR
#define AG_TABCHAR 9
#endif
static void ag_track(PCB_DECL) {
  int ag_k = (int) ((PCB).la_ptr - (PCB).pointer);
  while (ag_k--) {
    switch (*(PCB).pointer++) {
    case AG_NEWLINE:
      (PCB).column = 1, (PCB).line++;
    case AG_RETURN:
    case AG_FORMFEED:
      break;
    case AG_TABCHAR:
      (PCB).column += (TAB_SPACING) - ((PCB).column - 1) % (TAB_SPACING);
      break;
    default:
      (PCB).column++;
    }
  }
}
static const unsigned short ag_tstt[] = {
555,554,553,552,551,550,549,548,481,480,479,390,387,364,363,362,361,360,359,
  358,357,356,355,354,353,352,264,262,258,257,249,234,226,196,190,119,97,
  96,95,94,93,65,30,26,24,22,20,18,16,14,12,10,8,6,2,0,1,28,29,32,33,34,
  35,36,66,68,92,115,187,188,252,253,254,255,259,263,350,351,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,65,0,
559,556,552,551,550,549,479,390,387,249,196,65,0,189,191,193,194,195,202,
  203,204,
290,278,0,256,265,268,275,289,
290,278,261,0,256,260,265,268,275,289,
290,278,261,0,256,260,265,268,275,289,
547,41,37,0,38,39,40,46,
41,0,40,116,
5,0,
5,0,
5,0,
5,0,
5,0,
37,0,3,
5,0,
5,0,
37,0,3,
37,0,3,
37,0,3,
37,0,3,
37,0,3,
37,0,3,
37,0,3,
37,0,3,
37,0,3,
37,0,3,
37,0,3,
37,0,3,
559,556,552,551,550,549,479,390,387,249,65,0,204,
559,556,552,551,550,549,547,479,390,387,249,65,63,61,53,43,41,5,0,206,207,
  208,
556,550,390,387,249,65,0,202,
547,63,61,53,41,5,0,52,192,210,
554,553,552,551,550,481,480,479,390,387,294,249,226,65,0,141,153,277,291,
560,559,556,554,553,552,551,550,549,548,481,480,479,390,387,341,249,234,226,
  196,65,57,55,53,50,48,43,0,49,276,279,281,283,
547,463,63,61,55,53,41,5,0,52,266,306,
547,463,63,61,55,53,41,5,0,52,266,306,
553,552,551,550,481,480,479,390,387,249,226,196,65,0,175,176,179,
553,552,551,550,481,480,479,390,387,249,226,196,65,0,175,176,179,
547,41,0,
37,0,51,
553,552,551,550,547,481,480,479,390,387,264,262,258,257,249,226,196,190,65,
  41,37,0,
553,552,551,550,481,480,479,390,387,264,262,258,257,249,226,196,190,65,0,
  173,174,175,176,179,187,188,252,253,254,255,259,263,
550,390,387,41,0,40,117,120,
558,557,556,555,554,553,552,551,550,549,548,547,542,541,515,513,485,481,480,
  479,435,410,409,408,407,406,405,404,390,387,378,295,294,264,262,258,257,
  249,234,226,196,190,134,87,65,63,61,59,57,55,41,37,0,38,39,46,
553,552,551,550,481,480,479,390,387,249,226,196,65,0,31,175,176,179,
542,541,515,513,485,0,27,501,502,536,537,538,539,540,
555,554,553,552,551,550,549,548,481,480,479,435,390,387,249,234,226,196,65,
  0,25,66,68,495,496,
555,554,553,552,551,550,549,548,481,480,479,435,390,387,249,234,226,196,65,
  0,23,66,68,433,434,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,65,63,
  59,0,21,58,81,181,366,368,534,535,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,134,65,
  0,19,66,68,125,126,127,131,
550,390,387,0,17,101,102,
558,557,556,555,554,553,552,551,550,549,548,481,480,479,390,387,295,294,249,
  234,226,196,87,65,63,61,59,57,55,37,0,15,69,71,103,
555,554,553,552,551,550,549,548,481,480,479,390,387,378,264,262,258,257,249,
  234,226,196,190,65,63,59,0,13,58,81,180,181,188,255,259,263,269,270,271,
  272,273,274,366,368,375,376,377,379,
550,410,409,408,407,406,405,404,390,387,0,11,399,400,401,403,411,
550,410,409,408,407,406,405,404,390,387,0,9,101,395,398,399,403,
555,554,553,552,551,550,549,548,481,480,479,390,387,264,262,258,257,249,234,
  226,196,190,65,63,59,0,7,58,81,180,181,188,255,259,263,269,270,271,272,
  273,274,365,366,368,
550,390,387,0,4,98,99,101,
559,556,552,551,550,549,547,479,390,387,249,65,63,61,53,43,41,5,0,206,207,
  208,
559,556,552,551,550,549,479,390,387,249,65,0,206,
556,550,390,387,249,65,0,
556,550,547,390,387,249,65,63,61,53,43,41,5,0,198,199,200,
560,556,554,553,552,551,550,549,548,547,481,480,479,390,387,341,295,294,251,
  249,235,234,227,226,221,218,196,65,57,41,37,0,38,39,46,
53,0,52,
554,553,552,551,550,481,480,479,390,387,249,226,65,0,293,
554,553,552,551,550,481,480,479,390,387,249,226,65,0,293,
552,551,550,479,390,387,300,0,172,298,299,302,
37,0,51,
554,553,552,551,550,481,480,479,390,387,294,249,226,65,0,141,153,277,291,
560,559,556,554,553,552,551,550,549,548,481,480,479,390,387,341,249,234,226,
  196,65,57,55,53,48,43,0,
37,0,51,
50,0,49,
560,556,554,553,552,551,550,549,548,547,481,480,479,390,387,341,336,335,325,
  324,319,318,295,294,249,234,226,196,170,167,163,157,65,57,41,37,0,38,39,
  46,
53,0,52,
547,463,63,61,55,41,5,0,54,267,339,
547,463,63,61,55,41,5,0,54,267,339,
553,552,551,550,481,480,479,390,387,249,226,196,65,0,
37,0,51,
562,561,560,559,558,557,556,555,554,553,552,551,550,549,548,547,481,480,479,
  390,387,341,295,294,249,234,226,196,87,65,63,61,59,57,55,53,50,48,43,41,
  37,5,0,38,39,46,
41,0,40,116,
550,390,387,0,
41,0,40,
5,0,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,43,41,5,0,38,39,46,
5,0,
547,41,0,38,46,
547,41,0,38,46,
5,0,
547,41,0,38,46,
547,41,0,38,46,
5,0,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,65,
  59,41,37,0,369,370,371,
562,561,560,559,558,557,556,555,554,553,552,551,550,549,548,547,481,480,479,
  390,387,341,295,294,249,234,226,196,87,65,63,61,59,57,55,53,50,48,43,41,
  37,5,0,82,83,84,85,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,65,0,
547,59,41,37,0,38,39,46,
553,552,551,550,547,481,480,479,390,387,264,262,258,257,249,226,196,190,65,
  41,0,38,39,46,
59,0,58,
547,53,43,41,5,0,38,39,46,
43,0,44,
5,0,
547,57,41,0,38,39,46,
547,41,0,38,46,
43,0,44,
5,0,
550,390,387,0,
547,41,5,0,38,39,46,
5,0,
558,557,556,555,554,553,552,551,550,549,548,481,480,479,390,387,295,294,249,
  234,226,196,87,65,63,61,59,57,55,37,0,
50,0,49,
547,41,5,0,38,39,46,
5,0,
37,0,51,
290,278,261,0,260,265,268,275,289,
290,278,261,0,260,265,268,275,289,
559,556,552,551,550,549,479,390,387,249,196,65,0,191,193,194,195,202,203,
  204,
290,278,0,265,268,275,289,
59,0,58,
53,0,52,380,
547,53,43,41,5,0,38,39,46,
43,0,44,
5,0,
550,390,387,0,
547,43,41,0,38,39,46,
547,41,0,38,46,
547,41,0,38,46,
547,41,5,0,38,39,46,
5,0,
550,390,387,0,
547,41,5,0,38,39,46,
5,0,
53,0,52,367,
547,53,41,5,0,38,39,46,
5,0,
550,390,387,0,
547,41,0,38,46,
547,41,5,0,38,39,46,
5,0,
556,550,547,390,387,249,65,63,61,53,43,41,5,0,198,199,200,
556,550,390,387,249,65,0,198,
560,556,554,553,552,551,550,549,548,481,480,479,390,387,341,295,294,251,249,
  235,234,227,226,221,218,196,65,57,37,0,211,212,213,214,215,216,217,337,
560,556,554,553,552,551,550,549,548,547,481,480,479,390,387,341,295,294,251,
  249,235,234,227,226,221,218,196,65,57,41,37,0,38,39,46,
554,553,552,551,550,481,480,479,390,387,249,226,65,0,
552,551,550,479,390,387,0,
547,300,295,249,53,43,41,37,5,0,303,304,305,
552,551,550,547,479,390,387,295,249,53,43,41,5,0,299,301,302,
300,0,
249,0,
295,0,
550,390,387,0,101,140,
560,556,554,553,552,551,550,549,548,481,480,479,390,387,341,249,234,226,196,
  65,50,48,43,0,280,284,286,287,288,
554,553,552,551,550,481,480,479,390,387,294,249,226,65,0,141,153,277,291,
560,556,554,553,552,551,550,549,548,481,480,479,390,387,341,336,335,325,324,
  319,318,295,294,249,234,226,196,170,167,163,157,65,57,37,0,217,307,308,
  309,310,311,312,313,314,315,316,317,337,
560,556,554,553,552,551,550,549,548,547,481,480,479,390,387,341,336,335,325,
  324,319,318,295,294,249,234,226,196,170,167,163,157,65,57,41,37,0,38,39,
  46,
560,556,554,553,552,551,550,549,548,481,480,479,390,387,295,294,249,234,226,
  196,65,57,55,37,0,340,342,345,
341,0,
560,559,556,554,553,552,551,550,549,548,547,481,480,479,390,387,341,295,294,
  249,234,226,196,65,57,55,53,50,48,43,41,37,0,38,39,46,
562,561,560,559,558,557,556,555,554,553,552,551,550,549,548,481,480,479,390,
  387,341,295,294,249,234,226,196,87,65,63,61,59,57,55,53,50,48,43,37,0,
  75,
119,41,0,40,115,
550,390,387,0,
561,560,559,558,557,556,555,554,553,552,551,550,549,548,481,480,479,390,387,
  341,295,294,249,234,226,196,87,65,63,61,59,57,55,53,50,48,43,41,37,5,0,
  118,122,123,124,
48,0,47,
48,0,47,
48,0,47,
48,0,47,
48,0,47,
43,0,42,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,65,0,66,
  68,437,494,
555,554,553,552,551,550,549,548,531,523,518,515,513,509,507,485,481,480,479,
  473,466,464,450,390,387,249,234,226,196,65,0,68,439,441,442,445,447,470,
  497,498,499,500,501,502,503,504,505,506,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,65,0,66,
  68,437,494,
555,554,553,552,551,550,549,548,492,490,485,481,480,479,473,468,466,464,452,
  450,390,387,249,234,226,196,65,0,68,436,438,439,440,441,442,443,444,445,
  446,447,448,470,
547,41,0,369,
562,561,560,559,558,557,556,555,554,553,552,551,550,549,548,547,481,480,479,
  390,387,341,295,294,249,234,226,196,87,65,63,61,59,57,55,53,50,48,43,41,
  37,5,0,
562,561,560,559,558,557,556,555,554,553,552,551,550,549,548,547,481,480,479,
  390,387,341,295,294,249,234,226,196,87,65,61,59,57,55,53,50,48,43,41,37,
  5,0,82,83,
63,0,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,65,
  59,41,37,0,369,370,371,
553,552,551,550,481,480,479,390,387,264,262,258,257,249,226,196,190,65,0,
  174,175,176,179,187,188,252,253,254,255,259,263,
553,552,551,550,547,481,480,479,390,387,264,262,258,257,249,226,196,190,65,
  41,0,38,39,46,
53,0,52,112,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,65,
  63,59,41,0,38,39,46,
57,0,56,
554,553,552,551,550,481,480,479,390,387,294,249,226,65,0,128,141,153,291,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,134,
  65,41,0,38,39,46,
558,557,556,555,554,553,552,551,550,549,548,481,480,479,390,387,295,294,249,
  234,226,196,87,65,63,61,59,57,55,37,0,69,71,
560,559,556,554,553,552,551,550,549,548,481,480,479,390,387,341,295,294,249,
  234,226,196,65,57,50,48,37,0,184,186,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,65,63,0,
  81,180,181,274,368,
547,55,53,43,41,5,0,38,39,46,
547,55,53,43,41,5,0,38,39,46,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,65,63,0,
  81,180,181,274,368,
547,55,53,43,41,5,0,38,39,46,
547,55,53,43,41,5,0,38,39,46,
547,55,53,43,41,5,0,38,39,46,
547,55,53,43,41,5,0,38,39,46,
555,554,553,552,551,550,549,548,547,481,480,479,396,391,390,387,385,249,234,
  226,196,65,41,0,38,39,46,
53,0,52,
53,0,52,380,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,264,262,258,257,249,
  234,226,196,190,65,63,59,41,0,38,39,46,
43,0,42,
425,424,423,422,421,420,419,418,417,416,415,414,0,412,
550,390,387,0,402,426,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,373,249,234,226,196,
  65,41,0,38,39,46,
53,0,52,
53,0,52,367,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,97,96,
  95,94,93,65,0,66,68,92,
560,556,554,553,552,551,550,549,548,547,481,480,479,463,390,387,341,295,294,
  249,234,226,196,65,63,61,57,55,53,48,41,37,5,0,332,333,334,
48,0,47,338,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
560,556,554,553,552,551,550,549,548,481,480,479,390,387,341,295,294,251,249,
  235,234,227,226,221,218,196,65,57,37,0,211,212,213,214,215,216,217,337,
552,551,550,479,390,387,0,
552,551,550,479,390,387,0,302,
37,0,303,
552,551,550,547,479,390,387,295,249,53,43,41,5,0,299,301,302,
550,390,387,0,296,297,
550,390,387,0,
560,556,554,553,552,551,550,549,548,481,480,479,390,387,341,249,234,226,196,
  65,48,43,0,286,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
555,554,553,552,551,550,549,548,547,481,480,479,463,390,387,249,234,226,196,
  65,63,61,55,53,48,41,5,0,47,329,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
560,556,554,553,552,551,550,549,548,481,480,479,390,387,341,336,335,325,324,
  319,318,295,294,249,234,226,196,170,167,163,157,65,57,37,0,217,307,308,
  309,310,311,312,313,314,315,316,317,337,
560,556,554,553,552,551,550,549,548,481,480,479,390,387,295,294,249,234,226,
  196,65,57,55,37,0,
48,0,
560,556,554,553,552,551,550,549,548,481,480,479,390,387,295,294,249,234,226,
  196,65,57,55,37,0,340,342,345,
560,559,556,554,553,552,551,550,549,548,481,480,479,390,387,341,295,294,249,
  234,226,196,65,57,55,53,50,48,43,37,0,177,183,
562,561,560,559,558,557,556,555,554,553,552,551,550,549,548,547,481,480,479,
  390,387,341,295,294,249,234,226,196,87,65,63,61,59,57,55,53,50,48,43,41,
  37,5,0,78,79,80,
547,41,5,0,38,39,46,
561,560,559,558,557,556,555,554,553,552,551,550,549,548,481,480,479,390,387,
  341,295,294,249,234,226,196,87,65,63,61,59,57,55,53,50,48,43,41,37,0,
  122,
555,554,553,552,551,550,549,548,547,489,488,481,480,479,390,387,249,234,226,
  196,65,63,41,0,38,39,46,
547,63,41,0,38,39,46,
547,63,41,0,38,39,46,
547,63,41,0,38,39,46,
552,551,550,547,479,390,387,41,0,38,39,46,
547,542,541,515,513,485,41,0,38,39,46,
547,48,41,0,38,39,46,
43,0,42,
547,48,41,0,38,39,46,
484,483,0,471,474,478,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,65,
  48,41,0,46,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,43,41,5,0,38,39,46,
43,0,42,
43,0,42,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,65,
  48,41,0,46,
547,43,41,5,0,38,39,46,
43,0,42,
547,61,41,0,38,39,46,
553,552,551,550,481,480,479,390,387,264,262,258,257,249,226,196,190,65,0,
  174,175,176,179,187,188,252,253,254,255,259,263,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,65,
  41,0,38,39,46,
53,0,52,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,65,63,
  59,0,58,81,181,366,368,535,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,65,
  41,0,38,39,46,
547,53,43,41,37,5,0,38,39,46,
53,43,5,0,129,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,134,65,
  0,66,68,126,127,131,
560,559,556,554,553,552,551,550,549,548,481,480,479,390,387,341,295,294,249,
  234,226,196,65,57,50,48,37,0,
547,53,43,41,5,0,38,39,46,
55,53,43,5,0,54,267,339,
55,53,43,5,0,54,267,339,
55,53,43,5,0,54,267,339,
55,53,43,5,0,54,267,339,
55,53,43,5,0,54,267,339,
55,53,43,5,0,54,267,339,
555,554,553,552,551,550,549,548,481,480,479,396,391,390,387,385,249,234,226,
  196,65,0,66,68,152,381,382,383,384,
555,554,553,552,551,550,549,548,547,481,480,479,396,391,390,387,385,249,234,
  226,196,65,41,0,38,39,46,
53,0,52,
555,554,553,552,551,550,549,548,481,480,479,390,387,264,262,258,257,249,234,
  226,196,190,65,63,59,0,58,81,180,181,188,255,259,263,269,270,271,272,
  273,274,366,368,379,
550,547,390,387,41,0,38,39,46,
547,41,0,38,46,
550,390,387,0,
37,0,51,
555,554,553,552,551,550,549,548,481,480,479,390,387,373,249,234,226,196,65,
  0,66,68,152,372,374,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,373,249,234,226,196,
  65,41,0,38,39,46,
53,0,52,
560,556,554,553,552,551,550,549,548,481,480,479,390,387,341,295,294,249,234,
  226,196,65,57,37,0,332,
560,556,554,553,552,551,550,549,548,547,481,480,479,390,387,341,295,294,249,
  234,226,196,65,57,41,37,0,38,39,46,
48,0,47,
48,0,47,
48,0,47,
48,0,47,
552,551,550,479,390,387,0,
550,390,387,0,
249,0,
48,0,47,
48,0,47,
48,0,47,
555,554,553,552,551,550,549,548,547,481,480,479,463,390,387,249,234,226,196,
  65,63,61,55,53,41,5,0,66,68,330,
328,327,326,0,
48,0,47,
48,0,47,
48,0,47,
48,0,47,
48,0,47,
560,556,554,553,552,551,550,549,548,547,481,480,479,463,390,387,341,295,294,
  249,234,226,196,65,63,61,57,55,53,50,43,41,37,5,0,343,347,348,349,
560,559,556,554,553,552,551,550,549,548,481,480,479,390,387,341,295,294,249,
  234,226,196,65,57,55,53,50,48,43,37,0,
562,561,560,559,558,557,556,555,554,553,552,551,550,549,548,547,481,480,479,
  390,387,341,295,294,249,234,226,196,87,65,63,61,59,57,55,53,50,48,43,41,
  37,0,78,
555,554,553,552,551,550,549,548,489,488,481,480,479,390,387,249,234,226,196,
  65,63,0,62,512,
63,0,543,
63,0,81,465,514,
63,0,81,465,
552,551,550,479,390,387,0,516,517,522,
542,541,515,513,485,0,501,502,536,537,538,539,540,
48,0,47,114,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,65,
  41,0,38,39,46,
48,0,47,
555,554,553,552,551,550,549,548,482,481,480,479,390,387,249,234,226,196,65,
  0,66,68,477,
479,0,475,
547,63,43,41,5,0,472,
48,0,47,
48,0,47,
48,0,47,
48,0,47,
547,48,41,0,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,65,
  63,41,0,369,370,371,
48,0,47,
48,0,47,
48,0,47,
48,0,47,
48,0,47,
555,554,553,552,551,550,549,548,547,531,523,518,515,513,509,507,485,481,480,
  479,473,466,464,450,390,387,249,234,226,196,65,41,0,38,39,46,
48,0,47,
48,0,47,
48,0,47,
48,0,47,
48,0,47,453,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,65,
  48,41,0,68,449,
555,554,553,552,551,550,549,548,547,492,490,485,481,480,479,473,468,466,464,
  452,450,390,387,249,234,226,196,65,41,0,38,39,46,
61,0,60,
547,61,41,0,38,39,46,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,65,0,66,
  68,113,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,65,
  41,0,38,39,46,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,65,0,66,
  68,132,
37,0,51,
53,0,52,
547,53,48,43,41,5,0,38,39,46,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,53,43,41,5,0,38,39,46,
547,53,43,41,5,0,38,39,46,
547,53,43,41,5,0,38,39,46,
555,554,553,552,551,550,549,548,481,480,479,396,391,390,387,385,249,234,226,
  196,65,0,66,68,152,381,382,383,384,
550,390,387,0,401,411,
550,390,387,0,413,
550,390,387,0,427,
547,48,41,0,38,39,46,
547,53,41,5,0,38,39,46,
555,554,553,552,551,550,549,548,481,480,479,390,387,373,249,234,226,196,65,
  0,66,68,152,372,374,
560,556,554,553,552,551,550,549,548,481,480,479,390,387,341,295,294,249,234,
  226,196,65,57,37,0,337,
553,552,551,550,547,481,480,479,390,387,226,196,41,0,38,39,46,
560,556,554,553,552,551,550,549,548,547,481,480,479,390,387,341,249,234,226,
  196,65,57,55,50,48,43,41,37,0,38,39,46,
559,556,551,550,549,547,479,390,387,249,226,65,41,0,38,39,46,
556,550,547,390,387,249,65,41,0,38,39,46,
550,390,387,0,
550,390,387,0,297,
554,553,552,551,550,547,481,480,479,390,387,294,249,226,65,41,0,38,39,46,
560,556,554,553,552,551,550,549,548,547,481,480,479,390,387,341,295,294,249,
  234,226,196,65,57,41,37,0,38,39,46,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,169,
  65,41,0,38,39,46,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,97,
  96,95,94,93,65,41,0,38,39,46,
554,553,552,551,550,547,481,480,479,390,387,294,249,226,65,41,0,38,39,46,
550,547,390,387,41,0,38,39,46,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,323,322,249,234,226,
  196,65,41,0,38,39,46,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,139,
  138,137,136,65,41,0,38,39,46,
560,556,554,553,552,551,550,549,548,481,480,479,390,387,295,294,249,234,226,
  196,65,57,55,50,37,0,347,
555,554,553,552,551,550,549,548,489,488,481,480,479,390,387,249,234,226,196,
  65,0,66,68,487,
552,551,550,479,390,387,63,0,517,522,544,545,546,
552,551,550,479,390,387,0,517,522,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,65,
  63,41,0,38,39,46,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,65,0,66,
  68,494,
547,63,41,0,38,46,62,
480,0,
550,390,387,0,101,476,
547,63,41,0,38,39,46,
555,554,553,552,551,550,549,548,547,489,488,481,480,479,390,387,249,234,226,
  196,65,63,41,0,38,39,46,
547,533,532,41,0,38,39,46,
547,63,41,0,38,39,46,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,65,
  63,41,0,369,370,371,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,65,63,0,
  68,
547,63,41,0,38,39,46,
547,63,41,0,38,39,46,
547,63,41,0,38,39,46,
547,63,41,0,38,39,46,
547,63,41,0,38,39,46,
555,554,553,552,551,550,549,548,531,523,518,515,513,509,507,485,481,480,479,
  473,466,464,450,390,387,249,234,226,196,65,0,68,439,441,442,445,447,470,
  498,499,500,501,502,503,504,505,506,
547,63,41,0,38,39,46,
547,469,327,41,0,38,39,46,
63,0,491,
63,0,491,
547,63,41,0,38,39,46,454,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,65,
  48,41,0,369,370,371,
555,554,553,552,551,550,549,548,492,490,485,481,480,479,473,468,466,464,452,
  450,390,387,249,234,226,196,65,0,68,438,439,440,441,442,443,444,445,446,
  447,448,470,
61,0,60,
547,53,48,43,41,5,0,38,39,46,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,65,0,66,
  68,113,
547,57,41,0,38,39,46,
550,547,390,387,41,0,38,39,46,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,170,
  167,166,164,163,161,157,155,154,65,41,0,38,39,46,
48,0,47,
48,0,47,
48,0,47,
48,0,47,
547,41,0,38,46,
550,390,387,0,
550,390,387,0,
37,0,51,
48,0,47,
553,552,551,550,481,480,479,390,387,226,196,0,195,236,237,238,239,
560,556,554,553,552,551,550,549,548,481,480,479,390,387,341,249,234,226,196,
  65,57,55,50,48,43,37,0,228,229,230,233,
559,556,551,550,549,479,390,387,249,226,65,0,222,223,224,
556,550,390,387,249,65,0,219,220,
249,0,
554,553,552,551,550,481,480,479,390,387,294,249,226,65,0,141,153,291,
560,556,554,553,552,551,550,549,548,481,480,479,390,387,341,295,294,249,234,
  226,196,65,57,37,0,171,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,169,65,
  0,66,68,168,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,97,96,
  95,94,93,65,0,66,68,92,
554,553,552,551,550,481,480,479,390,387,294,249,226,65,0,141,153,291,
550,390,387,0,158,
555,554,553,552,551,550,549,548,481,480,479,390,387,323,322,249,234,226,196,
  65,0,66,68,320,321,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,139,138,
  137,136,65,0,66,68,135,
547,63,43,41,5,0,472,
552,551,550,479,390,387,0,517,522,544,
63,0,
552,551,550,479,390,387,0,517,522,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,65,63,0,
  66,68,81,
63,0,62,
479,0,
550,390,387,0,
65,0,64,
63,0,510,511,
555,554,553,552,551,550,549,548,489,488,481,480,479,390,387,249,234,226,196,
  65,63,0,62,512,
533,532,0,
63,0,524,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,65,63,0,
  68,
562,561,560,559,558,557,556,555,554,553,552,551,550,549,548,547,481,480,479,
  390,387,341,295,294,249,234,226,196,87,65,63,61,59,57,55,53,50,48,43,41,
  37,5,0,82,83,84,85,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,65,0,
63,0,81,467,
63,0,519,
63,0,81,465,
63,0,81,451,
63,0,81,508,
547,43,41,5,0,38,39,46,
63,0,62,
469,327,0,
552,551,550,479,390,387,63,0,89,91,493,
63,0,
48,0,
547,43,41,5,0,38,39,46,
48,0,47,114,
57,0,56,
550,390,387,0,101,140,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,170,167,
  166,164,163,161,157,155,154,65,0,66,68,130,142,143,144,145,146,147,148,
  149,150,151,152,156,159,162,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,294,249,234,226,196,
  65,63,41,0,38,39,46,
550,547,390,387,63,41,0,38,39,46,
547,394,393,41,0,38,39,46,
547,390,387,41,0,38,39,46,
550,390,387,0,402,426,
550,390,387,0,
550,390,387,0,428,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,65,
  41,0,38,39,46,
553,552,551,550,547,481,480,479,390,387,249,226,65,63,61,53,41,5,0,245,246,
  247,248,
553,552,551,550,547,481,480,479,390,387,249,226,65,63,61,53,41,5,0,245,246,
  247,248,
547,249,63,61,53,41,5,0,240,241,242,243,
552,551,550,479,390,387,0,
560,556,554,553,552,551,550,549,548,481,480,479,390,387,341,249,234,226,196,
  65,57,55,50,48,43,37,0,229,233,
559,556,551,550,549,479,390,387,249,226,65,0,223,
556,550,390,387,249,65,0,
550,390,387,0,297,
560,556,554,553,552,551,550,549,548,547,481,480,479,463,390,387,341,295,294,
  249,234,226,196,65,63,61,57,55,53,41,37,5,0,332,333,334,
550,390,387,0,
552,551,550,479,390,387,0,517,522,
547,53,43,41,5,0,38,39,46,
547,53,43,41,5,0,38,39,46,
481,0,
555,554,553,552,551,550,549,548,482,481,480,479,390,387,249,234,226,196,65,
  0,66,68,477,
553,552,551,550,481,480,479,390,387,264,262,258,257,249,226,196,190,65,0,
  173,174,175,176,179,187,188,252,253,254,255,259,263,
555,554,553,552,551,550,549,548,489,488,481,480,479,390,387,249,234,226,196,
  65,0,66,68,487,
553,552,551,550,481,480,479,390,387,226,196,57,0,526,
562,561,560,559,558,557,556,555,554,553,552,551,550,549,548,547,481,480,479,
  390,387,341,295,294,249,234,226,196,87,65,63,61,59,57,55,53,50,48,43,41,
  37,5,0,82,83,84,85,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,65,0,
63,0,
552,551,550,547,479,390,387,63,41,0,38,39,46,517,520,522,
555,554,553,552,551,550,549,548,489,488,481,480,479,390,387,249,234,226,196,
  65,0,66,68,486,487,
552,551,550,479,390,387,0,
63,0,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,295,294,258,257,249,
  234,226,196,65,63,41,37,0,46,455,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,65,
  63,41,0,369,370,371,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,139,
  138,137,136,65,41,0,38,39,46,
547,53,43,41,5,0,38,39,46,
547,53,48,43,41,5,0,38,39,46,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
53,48,43,5,0,47,160,
547,48,41,0,38,39,46,
547,48,41,0,38,39,46,
547,53,43,41,5,0,38,39,46,
547,53,43,41,5,0,38,39,46,
555,554,553,552,551,550,549,548,481,480,479,390,387,294,249,234,226,196,65,
  63,0,66,68,81,153,
550,390,387,63,0,62,101,397,398,
394,393,0,392,
390,387,0,386,
550,390,387,0,
550,390,387,0,
547,41,0,38,46,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,65,0,66,
  68,
553,552,551,550,481,480,479,390,387,226,65,0,245,246,
553,552,551,550,481,480,479,390,387,226,0,239,
249,0,240,241,
552,551,550,479,390,387,0,
550,390,387,0,
552,551,550,479,390,387,0,517,522,
63,0,
547,63,43,41,5,0,472,
553,552,551,550,530,529,481,480,479,390,387,226,196,63,57,0,527,528,
63,0,
63,0,
552,551,550,479,390,387,0,517,522,
63,0,
547,63,43,41,0,38,39,46,
547,41,0,
555,554,553,552,551,550,549,548,481,480,479,390,387,295,294,258,257,249,234,
  226,196,65,63,37,0,252,255,456,457,458,459,461,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,65,63,0,
  68,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,139,138,
  137,136,65,0,66,68,133,135,
48,0,47,
48,0,47,
48,0,47,
48,0,47,
550,547,390,387,41,0,38,39,46,
48,0,47,
48,0,47,
547,53,43,41,5,0,38,39,46,
547,53,43,41,5,0,38,39,46,
547,53,43,41,5,0,38,39,46,
547,410,409,408,407,406,405,404,41,0,38,39,46,
547,249,53,43,41,5,0,389,
547,249,53,43,41,5,0,389,
432,431,430,0,429,
552,551,550,479,390,387,0,517,522,
63,0,
552,551,550,479,390,387,0,517,522,
43,0,42,
555,554,553,552,551,550,549,548,481,480,479,390,387,295,294,249,234,226,196,
  65,37,0,
463,0,462,
547,63,41,0,46,455,
562,561,560,559,558,557,556,555,554,553,552,551,550,549,548,547,481,480,479,
  390,387,341,295,294,249,234,226,196,87,65,63,61,59,57,55,53,50,48,43,41,
  37,5,0,82,83,84,85,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,65,0,
554,553,552,551,550,547,481,480,479,390,387,294,249,226,65,41,0,38,39,46,
550,547,390,387,41,0,38,39,46,
547,63,41,0,38,39,46,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,169,
  65,41,0,38,39,46,
550,390,387,0,101,140,
555,554,553,552,551,550,549,548,547,481,480,479,390,387,249,234,226,196,65,
  41,0,38,39,46,
552,551,550,547,479,390,387,300,41,0,38,39,46,
410,409,408,407,406,405,404,0,399,403,
550,547,390,387,53,43,41,5,0,388,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
555,554,553,552,551,550,549,548,547,489,488,481,480,479,390,387,249,234,226,
  196,65,41,0,38,39,46,
258,257,0,252,255,
63,0,
63,0,
554,553,552,551,550,481,480,479,390,387,294,249,226,65,0,141,153,291,
550,390,387,0,158,
63,0,62,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,169,65,
  0,66,68,168,
555,554,553,552,551,550,549,548,481,480,479,390,387,249,234,226,196,65,0,66,
  68,
552,551,550,479,390,387,300,0,165,172,298,299,302,
547,63,41,0,38,39,46,
550,390,387,0,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
555,554,553,552,551,550,549,548,489,488,481,480,479,390,387,249,234,226,196,
  65,0,66,68,487,
547,53,43,41,5,0,38,39,46,
547,53,43,41,5,0,38,39,46,
560,556,554,553,552,551,550,549,548,481,480,479,390,387,341,295,294,249,234,
  226,196,65,57,37,0,171,
547,53,43,41,5,0,38,39,46,
547,53,43,41,5,0,38,39,46,
547,53,43,41,5,0,38,39,46,
63,0,
550,390,387,0,
552,551,550,479,390,387,0,517,522,
63,0,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
552,551,550,479,390,387,0,517,522,
};
static unsigned const char ag_astt[7818] = {
  1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,2,2,2,
  2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,0,1,1,1,1,1,1,1,2,1,2,1,2,1,2,2,2,1,
  1,1,2,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,4,1,1,1,1,1,1,1,1,1,1,1,1,7,3,
  1,2,2,2,1,2,1,2,2,7,3,1,1,1,1,2,2,2,7,2,1,1,1,1,1,2,2,2,7,2,1,1,1,1,1,1,1,
  8,7,1,1,1,1,2,7,1,1,3,7,3,7,3,7,3,7,3,7,1,7,1,3,7,3,7,1,7,1,1,7,1,1,7,1,1,
  7,1,1,7,1,1,7,1,1,7,1,1,7,1,1,7,1,1,7,1,1,7,1,1,7,1,1,9,1,1,1,1,1,1,1,9,9,
  7,1,1,1,1,1,1,1,5,1,1,1,1,1,5,5,5,5,5,5,7,1,1,3,1,1,1,1,1,1,7,1,4,4,4,2,4,
  4,7,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,2,1,1,7,1,1,1,2,1,4,4,4,4,4,2,4,4,7,1,1,1,4,4,4,4,4,
  2,4,4,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  7,2,1,1,9,9,4,2,7,1,4,4,4,4,5,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5,5,7,1,1,1,1,1,
  1,1,1,1,2,2,2,2,1,1,1,2,1,7,1,1,2,1,1,2,1,2,2,2,1,1,1,1,1,1,2,7,3,1,1,5,5,
  5,5,5,5,5,5,5,5,5,1,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
  5,5,5,5,5,5,5,5,5,5,5,1,5,7,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,2,1,1,1,1,
  1,1,1,7,1,2,2,2,1,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,2,2,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,2,2,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,2,7,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,7,
  1,2,1,1,2,1,1,1,1,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,1,1,1,1,2,1,1,
  2,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,2,1,2,2,2,2,2,2,2,1,1,7,1,2,1,
  1,1,1,1,2,2,2,2,2,2,2,1,1,7,1,1,1,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,
  1,1,1,1,2,1,1,2,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,7,1,1,1,1,1,1,
  1,1,1,1,5,1,1,1,1,1,5,5,5,5,5,5,7,1,1,3,9,9,9,9,9,9,9,9,9,9,9,5,3,9,1,1,1,
  9,9,7,1,1,5,1,1,1,1,5,5,5,5,5,5,7,1,1,3,8,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,1,8,7,1,1,1,2,5,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,4,1,1,1,1,1,1,1,1,7,1,1,1,1,2,4,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,7,1,2,2,2,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  4,2,4,1,2,7,1,8,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,1,8,7,1,1,1,2,4,1,4,4,4,4,2,4,4,7,1,2,1,4,4,4,4,2,4,4,7,1,2,1,9,9,
  9,9,9,9,9,9,9,9,9,9,9,4,2,7,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,8,5,7,1,1,1,2,7,1,1,1,1,1,7,2,4,1,3,7,
  1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,
  1,5,7,1,1,1,3,7,1,1,7,1,1,1,1,7,1,1,3,7,1,1,7,1,1,1,1,7,1,1,3,7,5,5,5,5,5,
  5,5,5,1,5,5,5,5,5,5,5,5,5,5,5,1,5,7,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,8,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,4,1,5,1,5,7,2,2,1,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,1,7,1,1,1,2,7,1,1,8,5,1,5,7,1,1,1,2,5,1,3,7,1,8,1,7,1,1,1,1,1,7,1,1,
  2,5,1,3,7,9,9,9,4,1,1,5,7,2,2,1,3,7,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9,9,9,9,9,9,9,9,9,9,9,4,2,4,1,1,1,5,7,2,2,1,3,7,2,7,1,2,2,2,7,1,1,1,1,1,2,
  2,2,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,2,2,2,2,1,2,1,2,2,7,1,1,1,1,2,4,
  1,2,4,1,1,1,8,5,1,5,7,1,1,1,2,5,1,3,7,2,2,2,4,1,8,1,7,1,1,1,1,1,7,1,1,1,1,
  7,1,1,1,1,5,7,2,2,1,3,7,9,9,9,4,1,1,5,7,2,2,1,3,7,2,4,1,1,1,8,1,5,7,1,1,1,
  3,7,9,9,9,4,1,1,7,1,1,1,1,5,7,2,2,1,3,7,1,1,5,1,1,1,1,5,5,5,5,5,5,7,1,1,3,
  9,9,9,9,9,9,5,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,
  7,2,2,2,2,2,2,2,1,8,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,1,8,7,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,4,1,1,1,1,1,1,5,5,5,5,5,5,5,5,1,5,
  7,1,1,3,1,1,1,5,1,1,1,5,5,5,5,5,5,7,3,3,1,1,5,1,5,2,7,1,1,1,7,1,2,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,5,1,1,7,2,2,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,7,1,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,7,2,2,2,2,2,2,2,2,2,2,2,2,1,8,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,8,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,7,2,1,1,1,4,8,8,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,1,8,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,4,2,2,2,7,3,1,2,2,2,7,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,5,7,2,1,1,2,2,7,1,
  2,7,1,2,7,1,2,7,1,2,7,1,2,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,2,2,2,2,
  2,1,1,1,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,2,2,2,2,2,2,2,2,
  2,2,1,9,9,5,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  3,3,3,3,3,3,3,3,3,3,3,3,7,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9,9,9,9,1,9,9,9,9,9,9,9,9,9,9,9,9,5,3,3,2,7,5,5,5,5,5,5,5,5,1,5,5,5,5,5,5,
  5,5,5,5,5,1,5,7,1,1,2,1,1,1,1,1,1,1,1,1,2,2,2,2,1,1,1,2,1,7,1,2,1,1,2,1,2,
  2,2,1,1,1,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,2,4,1,1,8,8,8,8,
  8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,2,7,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,7,1,1,2,2,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,2,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,7,1,1,2,3,1,1,8,5,5,1,5,7,1,1,1,1,8,5,5,1,5,7,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,7,1,1,2,3,1,1,8,5,5,1,5,7,1,1,1,1,8,5,5,1,5,7,1,1,1,
  1,8,5,5,1,5,7,1,1,1,1,8,5,5,1,5,7,1,1,1,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,
  8,8,8,8,8,1,7,1,1,1,2,4,1,2,4,1,1,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,1,7,1,1,1,2,7,1,2,2,2,2,2,2,2,2,2,2,2,2,7,1,1,1,1,7,2,1,8,8,8,
  8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,2,4,1,2,4,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,2,2,2,2,2,1,7,2,1,2,1,1,1,1,1,1,1,1,1,5,1,1,1,5,1,1,1,1,
  1,1,1,1,1,1,5,5,1,5,5,5,5,1,5,7,1,1,2,2,4,1,2,1,8,1,7,1,1,1,1,8,1,7,1,1,1,
  1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,
  1,1,1,1,1,1,7,2,2,2,2,2,2,2,1,1,1,1,1,1,1,5,1,1,1,1,1,1,7,3,1,5,3,1,1,1,5,
  1,1,1,5,5,5,5,5,5,7,3,3,1,1,1,1,7,3,1,9,9,9,4,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9,9,9,9,9,9,9,9,5,3,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,8,8,8,8,8,8,
  8,8,5,8,8,8,5,8,8,8,8,8,8,8,5,5,5,5,2,5,5,7,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,
  1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,2,2,2,2,2,2,2,2,2,2,2,2,1,9,9,9,9,
  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,4,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,7,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,7,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,5,7,1,1,2,1,1,5,7,2,2,1,9,9,9,9,9,9,9,9,9,9,9,
  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,5,3,5,5,5,5,5,5,5,
  5,1,5,5,5,5,5,5,5,5,5,5,5,5,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,
  7,1,1,1,8,8,8,1,8,8,8,1,7,1,1,1,1,8,8,8,8,8,1,7,1,1,1,1,8,1,7,1,1,1,2,4,1,
  1,8,1,7,1,1,1,2,2,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,
  7,1,1,1,9,9,9,9,9,9,9,9,1,9,9,9,9,9,9,9,9,9,9,1,1,7,1,1,8,1,7,1,1,1,1,8,1,
  7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,5,1,5,7,2,2,1,2,4,1,2,
  4,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,
  9,9,9,9,9,9,9,9,1,9,9,9,9,9,9,9,9,9,9,1,1,7,1,1,5,1,5,7,2,2,1,2,4,1,1,8,1,
  7,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,1,1,1,2,1,7,1,2,1,1,2,1,2,2,2,1,1,1,8,8,
  8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,2,4,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,2,7,1,1,1,1,1,2,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,1,7,1,
  1,1,1,5,5,1,8,5,7,1,1,1,4,4,4,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,7,
  2,1,2,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,4,1,5,5,1,
  5,7,2,2,1,2,4,4,4,7,1,2,1,2,4,4,4,7,1,2,1,2,4,4,4,7,1,2,1,2,4,4,4,7,1,2,1,
  2,4,4,4,7,1,2,1,2,4,4,4,7,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  7,1,1,2,3,1,1,1,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,2,4,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,1,1,1,1,2,1,1,2,7,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,2,8,1,8,8,1,7,1,1,1,1,1,7,1,1,2,2,2,7,2,7,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,7,1,1,2,1,2,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,1,
  7,1,1,1,2,4,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,5,3,8,8,8,8,
  8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,8,7,1,1,1,2,7,1,2,7,1,2,7,1,2,7,
  1,3,3,3,3,3,3,5,1,1,1,5,1,7,2,7,1,2,7,1,2,7,1,1,1,1,1,1,1,1,1,5,1,1,1,5,1,
  1,1,1,1,1,1,5,5,5,5,5,5,7,2,1,2,2,2,2,5,2,7,1,2,7,1,2,7,1,2,7,1,2,7,1,1,1,
  1,1,1,1,1,1,1,5,1,1,1,5,1,1,5,1,1,1,1,1,1,1,5,5,1,1,5,1,5,5,1,5,7,2,1,1,2,
  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,4,9,9,9,9,9,9,
  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,5,3,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,2,7,1,1,1,7,2,1,7,2,2,2,1,7,2,2,1,
  1,1,1,1,1,7,2,1,1,1,1,1,1,1,7,2,2,3,1,2,2,2,2,7,1,2,8,8,8,8,8,8,8,8,1,8,8,
  8,8,8,8,8,8,8,8,1,7,1,1,1,2,7,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,7,2,
  1,2,1,7,1,5,2,5,5,5,7,2,2,7,1,2,7,1,2,7,1,2,7,1,9,1,9,7,8,8,8,8,8,8,8,8,1,
  8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,2,7,1,2,7,1,2,7,1,2,7,1,2,7,1,8,8,8,8,8,8,
  8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,2,7,1,2,7,1,2,
  7,1,2,7,1,2,7,2,1,1,1,1,1,1,1,1,1,9,1,1,1,1,1,1,1,1,1,1,1,9,7,1,2,8,8,8,8,
  8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,2,7,2,1,8,1,7,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,3,8,8,8,8,8,8,8,8,1,8,8,8,8,
  8,8,8,8,8,8,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,2,1,1,2,4,1,2,
  4,1,1,5,8,5,1,5,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,5,5,1,
  5,7,2,2,1,1,5,5,1,5,7,2,2,1,1,5,5,1,5,7,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,7,1,1,2,3,1,1,1,1,1,1,7,1,1,1,1,1,7,2,1,1,1,7,1,1,8,1,7,1,1,
  1,1,5,1,5,7,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,2,1,2,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,2,8,8,8,8,1,8,8,8,8,8,8,8,1,7,
  1,1,1,8,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,8,7,1,1,1,8,8,
  8,8,8,1,8,8,8,8,8,8,1,7,1,1,1,8,8,1,8,8,8,8,1,7,1,1,1,3,3,3,5,1,1,1,7,1,8,
  8,8,8,8,1,8,8,8,8,8,8,8,8,8,1,7,1,1,1,8,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,1,8,7,1,1,1,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,
  8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,8,8,8,8,8,1,8,8,
  8,8,8,8,8,8,8,1,7,1,1,1,8,1,8,8,1,7,1,1,1,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,
  8,8,8,8,8,1,7,1,1,1,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,
  1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,5,3,1,1,1,1,1,1,1,1,2,
  2,1,1,1,1,1,1,1,1,1,1,7,2,1,1,1,1,1,1,1,1,8,7,1,1,1,1,1,1,1,1,1,1,1,4,1,1,
  8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,7,1,1,3,1,2,1,4,1,1,2,1,7,1,1,1,7,1,1,1,8,1,7,1,1,1,5,5,5,5,5,
  5,5,5,1,5,5,5,5,5,5,5,5,5,5,5,5,8,1,7,1,1,1,1,8,8,1,7,1,1,1,1,8,1,7,1,1,1,
  8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,7,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,
  1,8,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  7,1,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,1,8,1,7,1,1,1,1,8,8,1,7,1,1,1,1,7,2,1,7,
  2,1,5,1,7,2,2,1,1,9,9,9,9,9,9,9,9,1,9,9,9,9,9,9,9,9,9,9,8,1,7,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,2,2,2,2,2,2,2,2,2,2,
  1,2,7,2,1,5,8,5,1,5,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,3,1,
  8,1,7,1,1,1,8,1,8,8,1,7,1,1,1,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,1,7,1,1,1,2,4,1,2,7,1,2,7,1,2,7,1,1,1,7,1,1,1,1,1,7,2,2,2,7,2,
  7,1,2,7,1,1,1,1,1,1,1,1,1,1,1,1,7,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,7,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,2,1,1,1,1,1,1,1,1,
  7,2,1,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,7,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,7,2,1,2,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,1,7,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,7,2,2,2,1,1,1,7,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,1,1,1,1,1,7,2,1,2,2,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,1,7,2,1,2,5,2,5,5,5,7,2,9,9,9,9,9,
  9,5,3,3,3,2,7,1,1,1,1,1,1,4,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,
  1,1,2,4,2,1,7,9,9,9,4,2,7,1,2,7,3,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,2,7,1,1,2,2,7,1,7,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,8,1,1,1,1,1,1,1,1,1,1,1,
  7,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,4,1,7,2,2,1,7,2,1,7,2,2,1,7,
  2,2,1,7,2,2,1,5,1,5,7,2,2,1,2,7,1,2,2,7,1,1,1,1,1,1,8,7,1,1,1,1,7,1,7,1,5,
  1,5,7,2,2,1,2,4,1,2,2,7,1,1,1,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,7,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,8,8,8,8,8,8,8,8,1,8,
  8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,8,1,8,8,8,1,7,1,1,1,1,8,8,1,7,1,1,1,1,8,8,
  1,7,1,1,1,1,1,1,7,2,1,1,1,1,4,1,1,1,7,1,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,
  8,8,1,7,1,1,1,1,1,1,1,5,1,1,1,1,1,5,1,1,5,5,5,5,5,7,1,1,1,2,1,1,1,1,5,1,1,
  1,1,1,5,1,1,5,5,5,5,5,7,1,1,1,2,5,1,5,5,5,5,5,7,1,1,1,2,1,1,1,1,1,1,7,9,9,
  9,9,9,9,9,9,9,9,9,9,9,9,9,9,1,9,9,9,9,9,9,9,9,9,4,3,3,9,9,9,9,9,9,9,9,9,9,
  9,4,3,9,9,9,9,9,9,4,1,1,1,7,3,1,1,1,1,1,1,1,1,1,5,1,1,1,5,1,1,1,1,1,1,1,1,
  1,1,5,5,1,5,5,5,1,5,7,1,1,2,1,1,1,4,1,1,1,1,1,1,4,1,1,1,5,5,1,5,7,2,2,1,1,
  5,5,1,5,7,2,2,1,2,7,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,7,2,1,2,1,1,1,1,
  1,1,1,1,1,2,2,2,2,1,1,1,2,1,7,1,1,2,1,1,2,1,2,2,2,1,1,1,1,1,1,1,1,1,1,1,2,
  2,1,1,1,1,1,1,1,1,1,1,7,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,8,1,1,1,1,1,1,1,1,1,1,1,7,1,1,
  1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,4,2,7,1,1,1,1,1,1,1,8,1,7,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,2,2,1,1,1,1,1,1,1,1,1,1,7,2,1,1,2,9,9,9,9,9,9,4,2,7,8,
  8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,8,7,1,1,8,8,8,8,8,8,8,8,1,
  8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,1,7,1,1,1,1,5,5,1,5,7,2,2,1,1,5,5,5,1,5,7,3,3,1,1,8,1,7,1,1,1,1,8,1,7,
  1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,4,2,4,4,7,1,2,1,8,1,7,1,1,1,1,8,1,7,1,1,
  1,1,5,5,1,5,7,3,3,1,1,5,5,1,5,7,3,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,7,1,1,1,1,1,1,1,2,7,1,1,3,2,2,2,7,2,1,1,7,2,2,2,2,4,2,2,2,7,1,1,7,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,2,1,9,9,9,9,9,9,9,9,9,9,9,5,3,3,1,1,
  1,1,1,1,1,1,1,1,7,3,1,5,3,3,3,3,3,3,3,3,7,2,2,2,4,1,1,1,1,1,1,4,1,1,2,7,5,
  2,5,5,5,7,2,9,9,9,9,1,1,9,9,9,9,9,9,9,8,9,7,1,1,2,7,2,7,1,1,1,1,1,1,7,1,1,
  2,7,1,2,8,1,7,1,1,1,9,9,5,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,1,1,1,1,1,2,1,
  7,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,2,2,2,2,1,7,2,1,2,2,2,7,1,2,7,1,2,7,1,2,7,1,8,1,8,8,1,7,
  1,1,1,2,7,1,2,7,1,1,5,5,1,5,7,2,2,1,1,5,5,1,5,7,2,2,1,1,5,5,1,5,7,2,2,1,1,
  8,8,8,8,8,8,8,1,7,1,1,1,5,1,5,5,5,5,7,2,5,1,5,5,5,5,7,2,2,2,2,7,2,1,1,1,1,
  1,1,4,1,1,2,7,1,1,1,1,1,1,7,1,1,2,7,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9,9,9,4,1,5,1,1,8,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,8,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9,9,9,9,5,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,1,7,1,1,1,8,1,8,8,1,7,1,1,1,1,8,1,
  7,1,1,1,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,1,1,1,7,1,2,8,8,
  8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,8,8,8,1,8,8,8,8,1,7,1,1,1,2,2,
  2,2,2,2,2,7,1,1,1,5,1,1,5,5,5,5,7,3,1,1,1,1,1,1,4,1,1,1,1,1,1,1,1,7,1,1,8,
  8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,2,2,7,3,1,2,7,3,7,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,7,1,2,2,1,1,1,7,1,2,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,2,1,7,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,1,
  7,1,2,1,1,1,1,8,1,7,1,1,1,1,1,1,5,2,2,2,2,2,2,4,2,2,1,1,1,1,1,1,7,1,1,1,1,
  1,1,1,1,1,1,2,2,1,1,1,1,1,1,1,1,1,1,7,2,1,2,1,5,5,1,5,7,2,2,1,1,5,5,1,5,7,
  2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,5,5,1,5,7,2,2,
  1,1,5,5,1,5,7,2,2,1,1,5,5,1,5,7,2,2,1,2,7,3,3,3,5,1,1,1,1,1,1,7,1,1,2,7,1,
  1,1,1,1,1,7,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,7,1,1,1,1,
  1,1,1,1,7,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,7,1,1,1,1,1,
  1,1,1,7,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,
  1,1,7,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,
  1,7,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,
  7,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,7,
  1,1,3,3,3,3,3,3,7,3,3
};
static const unsigned short ag_pstt[] = {
1,1,1,1,1,1,1,1,1,1,1,1,1,407,406,405,404,403,402,401,400,399,398,397,396,
  395,267,264,260,259,1,1,1,1,181,103,70,69,68,67,66,1,13,16,17,18,19,20,
  21,22,23,24,25,26,27,0,0,15,14,12,11,10,9,8,71,1,408,7,179,2,255,256,
  257,3,5,4,393,6,
41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,42,
29,28,29,29,29,29,29,29,29,28,30,28,2,180,31,183,184,185,28,195,29,
302,287,3,258,35,34,33,32,
302,287,263,4,266,36,35,34,33,32,
302,287,263,5,262,37,35,34,33,32,
38,40,39,6,39,39,41,38,
24,7,42,42,
20,8,
19,9,
18,10,
17,11,
16,12,
43,13,44,
14,14,
13,15,
43,16,45,
43,17,46,
43,18,47,
43,19,48,
43,20,49,
43,21,50,
43,22,51,
43,23,52,
43,24,53,
43,25,54,
43,26,55,
43,27,56,
57,193,57,57,57,57,57,57,57,193,193,28,57,
58,58,58,58,58,58,199,58,58,58,58,58,199,199,199,199,199,199,29,58,58,201,
59,60,60,60,59,59,30,59,
205,205,205,33,205,205,31,61,182,62,
63,64,64,64,64,64,64,64,64,64,65,63,64,63,32,66,306,301,305,
68,68,68,68,68,68,68,68,68,68,68,68,68,68,68,68,68,68,68,68,68,68,68,68,31,
  68,68,33,67,70,69,290,68,
333,333,333,333,333,33,333,333,34,71,73,72,
333,333,333,333,333,33,333,333,35,71,74,72,
75,75,75,75,75,75,75,75,75,75,75,75,75,36,265,76,75,
75,75,75,75,75,75,75,75,75,75,75,75,75,37,261,76,75,
28,28,29,
32,39,77,
24,24,24,24,27,24,24,24,24,24,24,24,24,24,24,24,24,24,24,27,27,40,
75,75,75,75,75,75,75,75,75,267,264,260,259,75,75,75,181,75,41,78,78,413,76,
  75,412,2,409,410,411,3,5,4,
79,79,79,24,42,100,80,80,
21,21,21,21,21,21,21,21,21,21,21,38,21,21,21,21,21,21,21,21,21,21,21,21,21,
  21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,
  21,38,21,43,23,23,38,
75,75,75,75,75,75,75,75,75,75,75,75,75,44,81,167,76,75,
83,85,86,84,82,45,88,647,645,642,87,646,648,649,
1,1,1,1,1,1,1,1,1,1,1,90,1,1,1,1,1,1,1,46,91,89,1,583,583,
1,1,1,1,1,1,1,1,1,1,1,93,1,1,1,1,1,1,1,47,94,92,1,508,508,
95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,96,36,48,103,99,98,
  100,101,97,102,638,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,120,1,49,107,119,1,106,113,105,104,
108,108,108,50,110,108,109,
111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,
  111,111,111,111,111,111,111,111,111,111,111,51,114,112,111,113,
95,95,95,95,95,95,95,95,95,95,95,95,95,436,267,264,260,259,95,95,95,95,181,
  95,96,36,52,124,99,98,115,120,118,119,117,116,121,121,121,121,121,121,
  122,97,124,436,123,439,
125,480,479,478,477,476,475,474,125,125,53,130,472,129,128,126,127,
131,480,479,478,477,476,475,474,131,131,54,133,131,132,467,468,126,
95,95,95,95,95,95,95,95,95,95,95,95,95,267,264,260,259,95,95,95,95,181,95,
  96,36,55,136,99,98,115,120,118,119,117,116,134,134,134,134,134,134,414,
  135,97,
137,137,137,56,140,139,138,137,
58,58,58,58,58,58,199,58,58,58,58,58,199,199,199,199,199,199,57,58,58,202,
198,198,198,198,198,198,198,198,198,198,198,200,198,
193,141,141,141,193,193,59,
142,142,189,142,142,142,142,189,189,189,189,189,189,60,142,142,191,
143,143,143,143,143,143,143,143,143,38,143,143,143,143,143,143,143,143,143,
  143,143,143,143,143,143,143,143,143,143,38,143,61,143,143,38,
33,206,144,
145,145,145,145,145,145,145,145,145,145,145,145,145,63,145,
145,145,145,145,145,145,145,145,145,145,145,145,145,307,145,
146,146,146,146,146,146,148,65,151,150,149,147,
32,303,152,
63,64,64,64,64,64,64,64,64,64,65,63,64,63,67,66,306,286,305,
292,292,292,292,292,292,292,292,292,292,292,292,292,292,292,292,292,292,292,
  292,292,292,292,292,292,292,293,
32,288,153,
31,70,154,
155,155,155,155,155,155,155,155,155,38,155,155,155,155,155,155,155,155,155,
  155,155,155,155,155,155,155,155,155,155,155,155,155,155,155,38,155,71,
  155,155,38,
33,334,156,
379,379,379,379,34,379,379,73,157,269,158,
379,379,379,379,34,379,379,74,157,268,158,
170,170,170,170,170,170,170,170,170,170,170,170,170,171,
32,76,159,
160,160,160,160,160,160,160,160,160,160,160,160,160,160,160,38,160,160,160,
  160,160,160,160,160,160,160,160,160,160,160,160,160,160,160,160,160,160,
  160,160,38,160,21,77,160,160,38,
24,78,161,161,
162,162,162,79,
24,102,163,
15,81,
38,164,38,82,164,164,38,
38,165,38,83,165,165,38,
38,166,38,84,166,166,38,
38,167,38,85,167,167,38,
38,168,38,86,168,168,38,
38,169,38,643,87,169,169,38,
12,88,
38,38,89,170,38,
38,38,90,171,38,
11,91,
38,38,92,172,38,
38,38,93,173,38,
10,94,
426,426,426,426,426,426,426,426,174,426,426,426,426,426,426,426,426,426,426,
  426,174,426,95,174,174,429,
176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,
  176,176,176,176,176,176,176,176,176,175,176,177,176,176,176,176,176,176,
  176,176,176,176,176,96,176,176,176,177,
178,178,178,178,178,178,178,178,178,178,178,178,178,178,178,178,178,178,422,
38,21,38,21,98,421,421,38,
179,179,179,179,38,179,179,179,179,179,179,179,179,179,179,179,179,179,179,
  38,99,179,179,38,
36,100,180,
38,181,21,38,21,101,181,181,38,
26,637,182,
9,103,
38,183,38,104,183,183,38,
38,38,105,184,38,
26,112,185,
8,107,
75,75,75,78,
38,38,21,109,77,77,38,
7,110,
44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,
  44,44,44,44,44,45,
31,80,186,
38,38,21,113,79,79,38,
6,114,
32,115,187,
302,287,263,116,188,190,189,33,32,
302,287,263,117,191,193,192,33,32,
29,28,29,29,29,29,29,29,29,28,30,28,118,277,183,184,185,28,195,29,
302,287,119,195,194,33,32,
36,172,180,
33,443,196,197,
38,198,21,38,21,122,198,198,38,
26,437,199,
5,124,
483,483,483,482,
38,200,38,126,200,200,38,
38,38,127,201,38,
38,38,128,202,38,
38,38,21,129,470,470,38,
4,130,
75,75,75,469,
38,38,21,132,463,463,38,
3,133,
33,417,203,204,
38,205,38,21,135,205,205,38,
2,136,
75,75,75,76,
38,38,138,206,38,
38,38,21,139,72,72,38,
1,140,
142,142,189,142,142,142,142,189,189,189,189,189,189,141,142,142,194,
188,188,188,188,188,188,190,188,
207,207,207,207,207,207,207,207,207,207,207,207,207,207,207,207,207,254,207,
  209,207,210,207,211,212,207,207,207,207,143,207,209,210,211,212,213,214,
  208,
213,213,213,213,213,213,213,213,213,38,213,213,213,213,213,213,213,213,213,
  213,213,213,213,213,213,213,213,213,213,38,213,144,213,213,38,
309,309,309,309,309,309,309,309,309,309,309,309,309,310,
214,214,214,214,214,214,329,
326,326,326,326,326,326,326,215,326,147,216,216,328,
146,146,146,319,146,146,146,319,319,319,319,319,319,148,322,322,147,
217,318,
218,316,
311,151,
219,219,219,152,219,304,
220,220,220,220,220,220,220,220,220,220,220,220,220,220,220,220,220,220,220,
  220,298,220,220,153,289,294,220,220,294,
63,64,64,64,64,64,64,64,64,64,65,63,64,63,154,66,306,285,305,
207,207,207,207,207,207,207,207,207,207,207,207,207,207,207,374,221,224,225,
  228,229,207,207,207,207,207,207,222,223,226,227,207,207,207,155,347,335,
  337,338,339,340,341,342,343,344,345,346,208,
230,230,230,230,230,230,230,230,230,38,230,230,230,230,230,230,230,230,230,
  230,230,230,230,230,230,230,230,230,230,230,230,230,230,230,38,230,156,
  230,230,38,
231,231,231,231,231,231,231,231,231,231,231,231,231,231,231,231,231,231,231,
  231,231,231,231,231,157,381,232,231,
233,380,
234,234,234,234,234,234,234,234,234,234,38,234,234,234,234,234,234,234,234,
  234,234,234,234,234,234,234,234,234,234,234,38,234,159,234,234,38,
235,235,235,235,235,235,235,235,235,235,235,235,235,235,235,235,235,235,235,
  235,235,235,235,235,235,235,235,235,235,235,235,235,235,235,235,235,235,
  235,235,506,507,
103,24,161,100,236,
111,111,111,162,
237,237,237,237,237,237,237,237,237,237,237,237,237,237,237,237,237,237,237,
  237,237,237,237,237,237,237,237,237,237,237,237,237,237,237,237,237,237,
  237,237,108,163,101,237,237,110,
30,164,238,
30,165,239,
30,166,240,
30,167,241,
30,168,242,
25,169,243,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,170,244,1,245,245,
252,252,252,252,252,252,252,252,250,251,254,86,84,248,257,249,252,252,252,
  246,253,255,256,252,252,252,252,252,252,252,171,252,590,591,594,596,601,
  247,259,258,589,592,593,595,597,598,599,600,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,172,244,1,260,260,
266,266,266,266,266,266,266,266,263,264,261,266,266,266,246,262,253,255,265,
  256,266,266,266,266,266,266,266,173,266,268,267,514,515,516,517,518,519,
  520,521,522,523,247,
425,425,427,425,
62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,
  62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,175,
57,57,57,57,57,57,57,57,57,57,57,57,57,57,57,57,57,57,57,57,57,57,57,57,57,
  57,57,57,175,57,57,57,57,57,57,57,57,57,57,57,57,59,57,57,
60,177,
426,426,426,426,426,426,426,426,174,426,426,426,426,426,426,426,426,426,426,
  426,174,426,178,174,174,428,
75,75,75,75,75,75,75,75,75,267,264,260,259,75,75,75,181,75,179,269,413,76,
  75,412,2,409,410,411,3,5,4,
270,270,270,270,38,270,270,270,270,270,270,270,270,270,270,270,270,270,270,
  38,180,270,270,38,
33,640,271,272,
273,273,273,273,273,273,273,273,38,273,273,273,273,273,273,273,273,273,273,
  273,273,38,182,273,273,38,
35,183,274,
63,64,64,64,64,64,64,64,64,64,65,63,64,63,184,276,275,306,305,
277,277,277,277,277,277,277,277,38,277,277,277,277,277,277,277,277,277,277,
  277,38,185,277,277,38,
111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,
  111,111,111,111,111,111,111,111,111,111,111,186,81,111,
278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,
  278,278,278,278,278,278,278,278,187,279,278,
95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,96,188,98,115,172,283,
  97,
38,280,21,21,38,21,189,280,280,38,
38,281,21,21,38,21,190,281,281,38,
95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,96,191,98,115,172,280,
  97,
38,282,21,21,38,21,192,282,282,38,
38,283,21,21,38,21,193,283,283,38,
38,284,21,21,38,21,194,284,284,38,
38,285,21,21,38,21,195,285,285,38,
286,286,286,286,286,286,286,286,38,286,286,286,286,286,286,286,286,286,286,
  286,286,286,38,196,286,286,38,
33,444,287,
33,441,196,288,
289,289,289,289,289,289,289,289,38,289,289,289,289,289,289,289,289,289,289,
  289,289,289,289,289,289,289,38,199,289,289,38,
25,200,290,
495,494,493,492,491,490,489,488,487,486,485,484,201,291,
292,292,292,202,471,293,
294,294,294,294,294,294,294,294,38,294,294,294,294,294,294,294,294,294,294,
  294,38,203,294,294,38,
33,418,295,
33,415,203,296,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,70,69,68,67,66,1,206,71,1,73,
297,297,297,297,297,297,297,297,297,370,297,297,297,370,297,297,297,297,297,
  297,297,297,297,297,370,370,297,370,370,370,370,297,370,207,297,297,377,
30,375,298,376,
38,299,38,209,299,299,38,
38,300,38,210,300,300,38,
38,301,38,211,301,301,38,
38,302,38,212,302,302,38,
207,207,207,207,207,207,207,207,207,207,207,207,207,207,207,207,207,254,207,
  209,207,210,207,211,212,207,207,207,207,213,208,209,210,211,212,213,214,
  208,
303,303,303,303,303,303,330,
146,146,146,146,146,146,215,323,
215,327,325,
146,146,146,319,146,146,146,319,319,319,319,319,319,217,321,321,147,
304,304,304,218,317,305,
75,75,75,128,
297,297,297,297,297,297,297,297,297,297,297,297,297,297,297,297,297,297,297,
  297,297,297,299,297,
38,306,38,221,306,306,38,
38,307,38,222,307,307,38,
38,308,38,223,308,308,38,
309,309,309,309,309,309,309,309,360,309,309,309,360,309,309,309,309,309,309,
  309,360,360,360,360,30,360,360,224,310,309,
38,311,38,225,311,311,38,
38,312,38,226,312,312,38,
38,313,38,227,313,313,38,
38,314,38,228,314,314,38,
38,315,38,229,315,315,38,
207,207,207,207,207,207,207,207,207,207,207,207,207,207,207,374,221,224,225,
  228,229,207,207,207,207,207,207,222,223,226,227,207,207,207,230,347,336,
  337,338,339,340,341,342,343,344,345,346,208,
385,385,385,385,385,385,385,385,385,385,385,385,385,385,385,385,385,385,385,
  385,385,385,385,385,386,
316,232,
231,231,231,231,231,231,231,231,231,231,231,231,231,231,231,231,231,231,231,
  231,231,231,231,231,233,382,232,231,
317,317,317,317,317,317,317,317,317,317,317,317,317,317,317,317,317,317,317,
  317,317,317,317,317,317,317,317,317,317,317,234,168,317,
318,318,318,318,318,318,318,318,318,318,318,318,318,318,318,318,318,318,318,
  318,318,318,318,318,318,318,318,318,318,318,318,318,318,318,318,318,318,
  318,318,318,318,52,235,318,318,54,
38,38,21,236,394,394,38,
107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,
  107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,
  107,107,109,107,
21,21,21,21,21,21,21,21,38,21,21,21,21,21,21,21,21,21,21,21,21,319,38,238,
  319,319,38,
38,320,38,239,320,320,38,
38,321,38,240,321,321,38,
38,322,38,241,322,322,38,
323,323,323,38,323,323,323,38,242,323,323,38,
38,324,324,324,324,324,38,243,324,324,38,
38,325,38,244,325,325,38,
25,586,326,
38,327,38,246,327,327,38,
561,560,247,330,329,328,
38,331,38,248,331,331,38,
38,332,38,249,332,332,38,
38,333,38,250,333,333,38,
38,334,38,251,334,334,38,
41,41,41,41,41,41,41,41,335,41,41,41,41,41,41,41,41,41,41,336,335,252,335,
38,337,38,253,337,337,38,
38,338,38,254,338,338,38,
38,339,38,255,339,339,38,
38,340,38,256,340,340,38,
38,341,38,257,341,341,38,
38,21,38,21,258,587,587,38,
25,585,342,
25,511,326,
38,343,38,261,343,343,38,
38,344,38,262,344,344,38,
38,345,38,263,345,345,38,
38,346,38,264,346,346,38,
38,347,38,265,347,347,38,
41,41,41,41,41,41,41,41,348,41,41,41,41,41,41,41,41,41,41,336,348,266,348,
38,21,38,21,267,512,512,38,
25,510,349,
38,350,38,269,350,350,38,
75,75,75,75,75,75,75,75,75,267,264,260,259,75,75,75,181,75,270,351,413,76,
  75,412,2,409,410,411,3,5,4,
352,352,352,352,352,352,352,352,38,352,352,352,352,352,352,352,352,352,352,
  38,271,352,352,38,
33,641,353,
95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,95,96,36,273,99,98,100,
  101,97,639,
354,354,354,354,354,354,354,354,38,354,354,354,354,354,354,354,354,354,354,
  38,274,354,354,38,
38,21,21,38,355,21,275,355,355,38,
116,116,116,276,356,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,120,1,277,119,1,114,105,104,
177,177,177,177,177,177,177,177,177,177,177,177,177,177,177,177,177,177,177,
  177,177,177,177,177,177,177,177,178,
38,21,21,38,21,279,284,284,38,
34,379,379,379,280,157,282,158,
34,379,379,379,281,157,281,158,
34,379,379,379,282,157,279,158,
34,379,379,379,283,157,278,158,
34,379,379,379,284,157,276,158,
34,379,379,379,285,157,275,158,
1,1,1,1,1,1,1,1,1,1,1,358,359,1,1,360,1,1,1,1,1,286,357,1,450,445,363,362,
  361,
364,364,364,364,364,364,364,364,38,364,364,364,364,364,364,364,364,364,364,
  364,364,364,38,287,364,364,38,
33,442,287,
95,95,95,95,95,95,95,95,95,95,95,95,95,267,264,260,259,95,95,95,95,181,95,
  96,36,289,99,98,115,120,118,119,117,116,121,121,121,121,121,121,122,97,
  440,
365,38,365,365,38,290,365,365,38,
38,38,291,366,38,
500,500,500,292,
32,293,367,
1,1,1,1,1,1,1,1,1,1,1,1,1,368,1,1,1,1,1,294,357,1,434,369,431,
370,370,370,370,370,370,370,370,38,370,370,370,370,370,370,370,370,370,370,
  370,38,295,370,370,38,
33,416,295,
369,369,369,369,369,369,369,369,369,369,369,369,369,369,369,369,369,369,369,
  369,369,369,369,369,371,369,
371,371,371,371,371,371,371,371,371,38,371,371,371,371,371,371,371,371,371,
  371,371,371,371,371,38,371,298,371,371,38,
30,299,372,
30,300,373,
30,301,374,
30,302,375,
332,332,332,332,332,332,331,
376,376,376,313,
377,305,
30,306,378,
30,307,379,
30,308,380,
1,1,1,1,1,1,1,1,362,1,1,1,362,1,1,1,1,1,1,1,362,362,362,362,362,362,309,364,
  1,364,
359,358,357,361,
30,311,381,
30,312,382,
30,313,383,
30,314,384,
30,315,385,
386,386,386,386,386,386,386,386,386,390,386,386,386,390,386,386,390,386,386,
  386,386,386,386,386,390,390,386,386,390,386,390,390,386,390,316,383,386,
  386,392,
174,174,174,174,174,174,174,174,174,174,174,174,174,174,174,174,174,174,174,
  174,174,174,174,174,174,174,174,174,174,174,175,
51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,
  51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,51,53,51,
387,387,387,387,387,387,387,387,387,387,387,387,387,387,387,387,387,387,387,
  387,38,319,387,387,
388,320,652,
96,321,543,611,610,
96,322,543,651,
389,389,389,389,389,389,323,612,389,389,
83,85,86,84,82,324,647,645,644,87,646,648,649,
30,325,390,582,
391,391,391,391,391,391,391,391,38,391,391,391,391,391,391,391,391,391,391,
  38,326,391,391,38,
30,327,392,
1,1,1,1,1,1,1,1,558,1,1,1,1,1,1,1,1,1,1,328,559,1,556,
393,329,394,
548,550,548,548,548,330,550,
30,331,395,
30,332,396,
30,333,397,
30,334,398,
28,399,28,335,
400,400,400,400,400,400,400,400,174,400,400,400,400,400,400,400,400,400,400,
  400,174,336,174,174,400,
30,337,401,
30,338,402,
30,339,403,
30,340,404,
30,341,405,
406,406,406,406,406,406,406,406,38,406,406,406,406,406,406,406,406,406,406,
  406,406,406,406,406,406,406,406,406,406,406,406,38,342,406,406,38,
30,343,407,
30,344,408,
30,345,409,
30,346,410,
30,347,538,411,
412,412,412,412,412,412,412,412,28,412,412,412,412,412,412,412,412,412,412,
  399,28,348,412,524,
413,413,413,413,413,413,413,413,38,413,413,413,413,413,413,413,413,413,413,
  413,413,413,413,413,413,413,413,413,38,349,413,413,38,
37,350,420,
38,414,38,351,414,414,38,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,352,415,1,93,
416,416,416,416,416,416,416,416,38,416,416,416,416,416,416,416,416,416,416,
  38,353,416,416,38,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,354,121,1,417,
32,129,418,
33,115,419,
38,21,420,21,38,21,357,420,420,38,
38,421,38,358,421,421,38,
38,422,38,359,422,422,38,
38,423,38,360,423,423,38,
38,21,21,38,21,361,449,449,38,
38,21,21,38,21,362,448,448,38,
38,21,21,38,21,363,447,447,38,
1,1,1,1,1,1,1,1,1,1,1,358,359,1,1,360,1,1,1,1,1,364,357,1,450,446,363,362,
  361,
125,125,125,365,424,127,
425,425,425,366,481,
426,426,426,367,427,
38,428,38,368,428,428,38,
38,21,38,21,369,433,433,38,
1,1,1,1,1,1,1,1,1,1,1,1,1,368,1,1,1,1,1,370,357,1,434,369,432,
207,207,207,207,207,207,207,207,207,207,207,207,207,207,207,207,207,207,207,
  207,207,207,207,207,371,378,
429,429,429,429,38,429,429,429,429,429,429,429,38,372,429,429,38,
430,430,430,430,430,430,430,430,430,38,430,430,430,430,430,430,430,430,430,
  430,430,430,430,430,430,430,38,430,373,430,430,38,
431,431,431,431,431,38,431,431,431,431,431,431,38,374,431,431,38,
432,432,38,432,432,432,432,38,375,432,432,38,
315,315,315,314,
304,304,304,377,433,
434,434,434,434,434,38,434,434,434,434,434,434,434,434,434,38,378,434,434,
  38,
435,435,435,435,435,435,435,435,435,38,435,435,435,435,435,435,435,435,435,
  435,435,435,435,435,38,435,379,435,435,38,
436,436,436,436,436,436,436,436,38,436,436,436,436,436,436,436,436,436,436,
  436,38,380,436,436,38,
437,437,437,437,437,437,437,437,38,437,437,437,437,437,437,437,437,437,437,
  437,437,437,437,437,38,381,437,437,38,
438,438,438,438,438,38,438,438,438,438,438,438,438,438,438,38,382,438,438,
  38,
439,38,439,439,38,383,439,439,38,
440,440,440,440,440,440,440,440,38,440,440,440,440,440,440,440,440,440,440,
  440,440,38,384,440,440,38,
441,441,441,441,441,441,441,441,38,441,441,441,441,441,441,441,441,441,441,
  441,441,441,441,38,385,441,441,38,
389,389,389,389,389,389,389,389,389,389,389,389,389,389,389,389,389,389,389,
  389,389,389,389,389,389,391,389,
1,1,1,1,1,1,1,1,567,566,1,1,1,1,1,1,1,1,1,1,387,568,1,442,
443,443,443,443,443,443,444,388,443,443,443,443,444,
445,445,445,445,445,445,613,445,445,
446,446,446,446,446,446,446,446,38,446,446,446,446,446,446,446,446,446,446,
  446,38,390,446,446,38,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,391,244,1,581,
38,38,38,551,447,38,552,
448,393,
449,449,449,394,449,450,
38,451,38,395,451,451,38,
21,21,21,21,21,21,21,21,38,21,21,21,21,21,21,21,21,21,21,21,21,452,38,396,
  452,452,38,
38,453,453,38,397,453,453,38,
38,454,38,398,454,454,38,
455,455,455,455,455,455,455,455,174,455,455,455,455,455,455,455,455,455,455,
  455,174,399,174,174,455,
457,457,457,457,457,457,457,457,457,457,457,457,457,457,457,457,457,457,456,
  400,457,
38,458,38,401,458,458,38,
38,459,38,402,459,459,38,
38,460,38,403,460,460,38,
38,461,38,404,461,461,38,
38,462,38,405,462,462,38,
252,252,252,252,252,252,252,252,250,251,254,86,84,248,257,249,252,252,252,
  246,253,255,256,252,252,252,252,252,252,252,406,252,590,591,594,596,601,
  247,463,589,592,593,595,597,598,599,600,
38,464,38,407,464,464,38,
38,465,465,38,408,465,465,38,
466,409,570,
466,410,569,
38,21,38,411,537,537,38,467,
41,41,41,41,41,41,41,41,174,41,41,41,41,41,41,41,41,41,41,468,174,412,174,
  174,468,
266,266,266,266,266,266,266,266,263,264,261,266,266,266,246,262,253,255,265,
  256,266,266,266,266,266,266,266,413,266,469,514,515,516,517,518,519,520,
  521,522,523,247,
37,414,419,
38,21,470,21,38,21,415,470,470,38,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,416,415,1,94,
38,471,38,417,471,471,38,
472,38,472,472,38,418,472,472,38,
473,473,473,473,473,473,473,473,38,473,473,473,473,473,473,473,473,473,473,
  473,473,473,473,473,473,473,473,473,38,419,473,473,38,
30,142,474,
30,421,475,
30,422,476,
30,423,477,
38,38,424,478,38,
479,479,479,425,
501,501,501,426,
32,427,480,
30,428,481,
483,483,483,482,483,483,483,482,482,483,30,429,235,234,235,235,484,
486,486,486,486,486,486,486,486,486,486,486,486,486,486,486,486,485,486,486,
  486,486,486,486,486,486,486,430,226,486,486,486,
487,487,487,487,487,487,487,487,487,487,487,431,219,487,487,
488,488,488,488,488,488,432,215,488,
489,433,
63,64,64,64,64,64,64,64,64,64,65,63,64,63,434,373,306,305,
490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,
  490,490,490,490,490,435,366,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,162,1,436,163,1,365,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,70,69,68,67,66,1,437,71,1,356,
63,64,64,64,64,64,64,64,64,64,65,63,64,63,438,355,306,305,
491,491,491,439,354,
1,1,1,1,1,1,1,1,1,1,1,1,1,352,351,1,1,1,1,1,440,353,1,349,350,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,126,125,124,123,1,441,127,1,348,
548,650,548,548,548,442,650,
655,655,655,655,655,655,657,655,655,655,
658,444,
492,492,492,492,492,492,614,492,492,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,96,446,494,1,493,
38,553,554,
495,448,
75,75,75,562,
39,450,496,
606,451,604,497,
498,498,498,498,498,498,498,498,498,498,498,498,498,498,498,498,498,498,498,
  498,38,452,498,498,
636,635,453,
499,454,627,
501,501,501,501,501,501,501,501,501,501,501,501,501,501,501,501,501,501,500,
  455,501,
176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,
  176,176,176,176,176,176,176,176,176,175,176,502,176,176,176,176,176,176,
  176,176,176,176,176,456,176,176,176,502,
41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,574,
96,458,545,544,
503,459,621,
96,460,543,542,
96,461,526,525,
96,462,603,602,
38,21,38,21,463,588,588,38,
38,464,504,
547,546,465,
505,505,505,505,505,505,506,466,506,505,506,
507,467,
508,468,
38,21,38,21,469,513,513,38,
30,95,390,96,
35,471,509,
219,219,219,472,219,510,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,514,515,517,518,512,511,513,519,520,1,473,
  357,1,117,131,132,133,134,135,136,137,138,139,140,140,148,516,157,
521,521,521,521,521,521,521,521,38,521,521,521,521,521,521,521,521,521,521,
  521,521,38,474,521,521,38,
522,38,522,522,522,38,475,522,522,38,
38,523,523,38,476,523,523,38,
38,524,524,38,477,524,524,38,
292,292,292,478,473,293,
525,525,525,496,
526,526,526,480,527,
528,528,528,528,528,528,528,528,38,528,528,528,528,528,528,528,528,528,528,
  38,481,528,528,38,
529,529,529,529,247,529,529,529,529,529,247,529,529,247,247,247,247,247,482,
  529,529,529,250,
529,529,529,529,247,529,529,529,529,529,247,529,529,247,247,247,247,247,483,
  529,529,529,249,
241,530,241,241,241,241,241,484,531,531,531,243,
532,532,532,532,532,532,485,
228,228,228,228,228,228,228,228,228,228,228,228,228,228,228,228,485,228,228,
  228,228,228,228,228,228,228,229,228,228,
221,221,221,221,221,221,221,221,221,221,221,222,221,
217,217,217,217,217,217,218,
304,304,304,489,312,
297,297,297,297,297,297,297,297,297,370,297,297,297,370,297,297,297,297,297,
  297,297,297,297,297,370,370,297,370,370,370,297,370,490,297,297,372,
533,533,533,150,
534,534,534,534,534,534,615,534,534,
38,21,21,38,21,493,98,98,38,
38,21,21,38,21,494,97,97,38,
557,495,
1,1,1,1,1,1,1,1,558,1,1,1,1,1,1,1,1,1,1,496,559,1,555,
75,75,75,75,75,75,75,75,75,267,264,260,259,75,75,75,181,75,497,535,535,413,
  76,75,412,2,409,410,411,3,5,4,
1,1,1,1,1,1,1,1,567,566,1,1,1,1,1,1,1,1,1,1,498,568,1,536,
537,537,537,537,537,537,537,537,537,537,537,537,499,537,
176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,
  176,176,176,176,176,176,176,176,176,175,176,538,176,176,176,176,176,176,
  176,176,176,176,176,500,176,176,176,538,
41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,576,
575,502,
540,540,540,38,540,540,540,539,38,503,539,539,38,540,541,540,
1,1,1,1,1,1,1,1,567,566,1,1,1,1,1,1,1,1,1,1,504,568,1,542,564,
64,64,64,64,64,64,65,
573,506,
544,544,544,544,544,544,544,544,543,544,544,544,544,544,544,544,544,544,544,
  544,544,544,544,544,543,544,507,543,544,
545,545,545,545,545,545,545,545,174,545,545,545,545,545,545,545,545,545,545,
  545,174,508,174,174,545,
546,546,546,546,546,546,546,546,38,546,546,546,546,546,546,546,546,546,546,
  546,546,546,546,38,509,546,546,38,
38,21,21,38,21,510,130,130,38,
38,21,21,21,38,21,511,154,154,38,
38,547,38,512,547,547,38,
38,548,38,513,548,548,38,
38,549,38,514,549,549,38,
38,550,38,515,550,550,38,
155,30,155,155,516,551,153,
38,552,38,517,552,552,38,
38,553,38,518,553,553,38,
38,21,21,38,21,519,147,147,38,
38,21,21,38,21,520,146,146,38,
1,1,1,1,1,1,1,1,1,1,1,1,1,65,1,1,1,1,1,96,521,556,1,555,554,
131,131,131,38,522,557,131,464,465,
462,461,523,460,
558,559,524,451,
498,498,498,497,
502,502,502,526,
38,38,527,560,38,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,528,430,1,
246,246,246,246,246,246,246,246,246,246,246,248,246,246,
483,483,483,482,483,483,483,482,482,483,530,251,
530,242,240,240,
233,233,233,233,233,233,532,
152,152,152,151,
561,561,561,561,561,561,616,561,561,
605,535,
548,609,548,548,548,536,609,
629,629,629,629,562,562,629,629,629,629,629,629,629,562,629,537,562,562,
577,538,
623,539,
563,563,563,563,563,563,540,563,563,
622,541,
38,563,564,38,542,564,564,38,
28,28,528,
565,565,565,565,565,565,565,565,565,565,565,565,565,565,565,260,259,565,565,
  565,565,565,529,565,544,566,3,567,566,567,533,565,
569,569,569,569,569,569,569,569,569,569,569,569,569,569,569,569,569,569,568,
  545,569,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,126,125,124,123,1,546,127,1,118,122,
30,547,570,
30,548,571,
30,549,572,
30,550,573,
574,38,574,574,38,551,574,574,38,
30,552,575,
30,553,576,
38,21,21,38,21,554,145,145,38,
38,21,21,38,21,555,144,144,38,
38,21,21,38,21,556,143,143,38,
38,577,577,577,577,577,577,577,38,557,577,577,38,
452,578,452,452,452,452,558,455,
452,578,452,452,452,452,559,454,
505,504,503,560,499,
579,579,579,579,579,579,617,579,579,
632,562,
580,580,580,580,580,580,563,580,580,
25,564,581,
535,535,535,535,535,535,535,535,535,535,535,535,535,535,535,535,535,535,535,
  535,535,536,
582,531,582,
543,583,543,567,543,583,
176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,
  176,176,176,176,176,176,176,176,176,175,176,584,176,176,176,176,176,176,
  176,176,176,176,176,568,176,176,176,584,
41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,578,
585,585,585,585,585,38,585,585,585,585,585,585,585,585,585,38,570,585,585,
  38,
586,38,586,586,38,571,586,586,38,
38,587,38,572,587,587,38,
588,588,588,588,588,588,588,588,38,588,588,588,588,588,588,588,588,588,588,
  588,38,573,588,588,38,
219,219,219,574,219,156,
589,589,589,589,589,589,589,589,38,589,589,589,589,589,589,589,589,589,589,
  38,575,589,589,38,
590,590,590,38,590,590,590,590,38,576,590,590,38,
480,479,478,477,476,475,474,577,591,126,
592,456,592,592,456,456,456,456,578,453,
593,593,593,593,593,593,618,593,593,
594,594,594,594,594,594,580,594,594,
595,595,595,595,595,595,595,595,38,595,595,595,595,595,595,595,595,595,595,
  595,595,38,581,595,595,38,
260,259,582,540,3,
530,583,
579,584,
63,64,64,64,64,64,64,64,64,64,65,63,64,63,585,596,306,305,
491,491,491,586,597,
38,587,598,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,162,1,588,163,1,599,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,589,600,1,
146,146,146,146,146,146,148,590,601,165,150,149,147,
38,602,38,591,602,602,38,
603,603,603,457,
620,620,620,620,620,620,619,620,620,
604,604,604,604,604,604,594,604,604,
1,1,1,1,1,1,1,1,567,566,1,1,1,1,1,1,1,1,1,1,595,568,1,565,
38,21,21,38,21,596,158,158,38,
38,21,21,38,21,597,149,149,38,
490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,490,
  490,490,490,490,490,598,605,
38,21,21,38,21,599,161,161,38,
38,21,21,38,21,600,160,160,38,
38,21,21,38,21,601,159,159,38,
466,602,
459,459,459,458,
606,606,606,606,606,606,604,606,606,
164,605,
607,607,607,607,607,607,606,607,607,
608,608,608,608,608,608,607,608,608,
609,609,609,609,609,609,608,609,609,
610,610,610,610,610,610,609,610,610,
611,611,611,611,611,611,610,611,611,
612,612,612,612,612,612,611,612,612,
613,613,613,613,613,613,612,613,613,
614,614,614,614,614,614,613,614,614,
615,615,615,615,615,615,614,615,615,
616,616,616,616,616,616,615,616,616,
617,617,617,617,617,617,616,617,617,
618,618,618,618,618,618,617,618,618,
619,619,619,619,619,619,618,619,619,
620,620,620,620,620,620,619,620,620,
621,621,621,621,621,621,620,621,621,
622,622,622,622,622,622,621,622,622,
623,623,623,623,623,623,622,623,623,
624,624,624,624,624,624,623,624,624,
625,625,625,625,625,625,624,625,625,
626,626,626,626,626,626,625,626,626,
627,627,627,627,627,627,626,627,627,
628,628,628,628,628,628,627,628,628,
629,629,629,629,629,629,628,629,629,
630,630,630,630,630,630,629,630,630,
631,631,631,631,631,631,630,631,631,
624,624,624,624,624,624,631,624,624,
};
static const unsigned short ag_sbt[] = {
     0,  78,  97, 118, 126, 136, 146, 154, 158, 160, 162, 164, 166, 168,
   171, 173, 175, 178, 181, 184, 187, 190, 193, 196, 199, 202, 205, 208,
   211, 224, 246, 254, 264, 283, 316, 328, 340, 357, 374, 377, 380, 402,
   434, 442, 498, 516, 530, 555, 580, 609, 636, 643, 678, 726, 743, 760,
   804, 812, 834, 847, 854, 871, 906, 909, 924, 939, 951, 954, 973,1000,
  1003,1006,1046,1049,1060,1071,1085,1088,1134,1138,1142,1145,1147,1154,
  1161,1168,1175,1182,1190,1192,1197,1202,1204,1209,1214,1216,1242,1289,
  1308,1316,1340,1343,1352,1355,1357,1364,1369,1372,1374,1378,1385,1387,
  1418,1421,1428,1430,1433,1442,1451,1471,1478,1481,1485,1494,1497,1499,
  1503,1510,1515,1520,1527,1529,1533,1540,1542,1546,1554,1556,1560,1565,
  1572,1574,1591,1599,1637,1672,1686,1693,1706,1723,1725,1727,1729,1735,
  1764,1783,1831,1871,1899,1901,1937,1978,1983,1987,2032,2035,2038,2041,
  2044,2047,2050,2073,2121,2144,2186,2190,2233,2277,2279,2305,2336,2360,
  2364,2390,2393,2412,2437,2470,2500,2525,2535,2545,2570,2580,2590,2600,
  2610,2637,2640,2644,2675,2678,2692,2698,2723,2726,2730,2757,2794,2798,
  2805,2812,2819,2826,2864,2871,2879,2882,2899,2905,2909,2933,2940,2947,
  2954,2984,2991,2998,3005,3012,3019,3067,3092,3094,3122,3155,3201,3208,
  3249,3276,3283,3290,3297,3309,3320,3327,3330,3337,3343,3350,3357,3364,
  3371,3394,3401,3408,3415,3422,3429,3437,3440,3443,3450,3457,3464,3471,
  3478,3501,3509,3512,3519,3550,3574,3577,3604,3628,3638,3643,3668,3696,
  3705,3713,3721,3729,3737,3745,3753,3782,3809,3812,3855,3864,3869,3873,
  3876,3901,3926,3929,3955,3985,3988,3991,3994,3997,4004,4008,4010,4013,
  4016,4019,4049,4053,4056,4059,4062,4065,4068,4107,4138,4181,4205,4208,
  4213,4217,4227,4240,4244,4268,4271,4294,4297,4304,4307,4310,4313,4316,
  4320,4345,4348,4351,4354,4357,4360,4396,4399,4402,4405,4408,4412,4436,
  4469,4472,4479,4501,4525,4547,4550,4553,4563,4570,4577,4584,4593,4602,
  4611,4640,4646,4651,4656,4663,4671,4696,4722,4739,4771,4788,4800,4804,
  4809,4829,4859,4884,4913,4933,4942,4968,4996,5023,5047,5060,5069,5094,
  5116,5123,5125,5131,5138,5165,5173,5180,5205,5226,5233,5240,5247,5254,
  5261,5308,5315,5323,5326,5329,5337,5362,5403,5406,5416,5438,5445,5454,
  5487,5490,5493,5496,5499,5504,5508,5512,5515,5518,5535,5566,5581,5590,
  5592,5610,5636,5659,5686,5704,5709,5734,5760,5767,5777,5779,5788,5811,
  5814,5816,5820,5823,5827,5851,5854,5857,5878,5925,5944,5948,5951,5955,
  5959,5963,5971,5974,5977,5988,5990,5992,6000,6004,6007,6013,6058,6084,
  6094,6102,6110,6116,6120,6125,6149,6172,6195,6207,6214,6243,6256,6263,
  6268,6304,6308,6317,6326,6335,6337,6360,6392,6416,6430,6477,6496,6498,
  6514,6539,6546,6548,6577,6602,6630,6639,6649,6656,6663,6670,6677,6684,
  6691,6698,6707,6716,6741,6750,6754,6758,6762,6766,6771,6792,6806,6818,
  6822,6829,6833,6842,6844,6851,6869,6871,6873,6882,6884,6892,6895,6927,
  6948,6975,6978,6981,6984,6987,6996,6999,7002,7011,7020,7029,7042,7050,
  7058,7063,7072,7074,7083,7086,7108,7111,7117,7164,7183,7203,7212,7219,
  7244,7250,7274,7287,7297,7307,7316,7325,7351,7356,7358,7360,7378,7383,
  7386,7409,7430,7443,7450,7454,7463,7472,7496,7505,7514,7540,7549,7558,
  7567,7569,7573,7582,7584,7593,7602,7611,7620,7629,7638,7647,7656,7665,
  7674,7683,7692,7701,7710,7719,7728,7737,7746,7755,7764,7773,7782,7791,
  7800,7809,7818
};
static const unsigned short ag_sbe[] = {
    55,  96, 109, 120, 129, 139, 149, 155, 159, 161, 163, 165, 167, 169,
   172, 174, 176, 179, 182, 185, 188, 191, 194, 197, 200, 203, 206, 209,
   222, 242, 252, 260, 278, 310, 324, 336, 353, 370, 376, 378, 401, 420,
   438, 494, 511, 521, 549, 574, 600, 628, 639, 673, 704, 736, 753, 785,
   807, 830, 845, 853, 867, 902, 907, 922, 937, 946, 952, 968, 999,1001,
  1004,1042,1047,1056,1067,1084,1086,1130,1135,1141,1143,1146,1150,1157,
  1164,1171,1178,1186,1191,1194,1199,1203,1206,1211,1215,1238,1284,1307,
  1312,1336,1341,1348,1353,1356,1360,1366,1370,1373,1377,1381,1386,1417,
  1419,1424,1429,1431,1436,1445,1463,1473,1479,1482,1490,1495,1498,1502,
  1506,1512,1517,1523,1528,1532,1536,1541,1543,1550,1555,1559,1562,1568,
  1573,1587,1597,1628,1668,1685,1692,1702,1719,1724,1726,1728,1732,1758,
  1778,1817,1867,1895,1900,1933,1976,1980,1986,2027,2033,2036,2039,2042,
  2045,2048,2068,2103,2139,2171,2188,2232,2274,2278,2301,2323,2356,2361,
  2386,2391,2407,2433,2467,2497,2519,2531,2541,2564,2576,2586,2596,2606,
  2633,2638,2641,2671,2676,2690,2695,2719,2724,2727,2753,2790,2795,2801,
  2808,2815,2822,2855,2870,2877,2880,2895,2902,2908,2931,2936,2943,2950,
  2981,2987,2994,3001,3008,3015,3053,3091,3093,3118,3152,3197,3204,3247,
  3272,3279,3286,3293,3305,3316,3323,3328,3333,3339,3346,3353,3360,3367,
  3392,3397,3404,3411,3418,3425,3433,3438,3441,3446,3453,3460,3467,3474,
  3499,3505,3510,3515,3537,3570,3575,3597,3624,3634,3641,3662,3695,3701,
  3709,3717,3725,3733,3741,3749,3774,3805,3810,3837,3860,3866,3872,3874,
  3895,3922,3927,3953,3981,3986,3989,3992,3995,4003,4007,4009,4011,4014,
  4017,4045,4052,4054,4057,4060,4063,4066,4102,4137,4179,4202,4206,4209,
  4214,4223,4232,4241,4264,4269,4290,4295,4302,4305,4308,4311,4314,4319,
  4341,4346,4349,4352,4355,4358,4392,4397,4400,4403,4406,4409,4433,4465,
  4470,4475,4497,4521,4543,4548,4551,4559,4566,4573,4580,4589,4598,4607,
  4632,4643,4649,4654,4659,4667,4690,4720,4735,4767,4784,4796,4803,4807,
  4825,4855,4880,4909,4929,4938,4964,4992,5021,5043,5054,5066,5090,5112,
  5119,5124,5128,5134,5161,5169,5176,5201,5224,5229,5236,5243,5250,5257,
  5291,5311,5319,5324,5327,5332,5358,5389,5404,5412,5434,5441,5450,5483,
  5488,5491,5494,5497,5501,5507,5511,5513,5516,5529,5561,5577,5587,5591,
  5606,5634,5655,5682,5700,5707,5729,5756,5765,5773,5778,5785,5807,5812,
  5815,5819,5821,5824,5848,5853,5855,5876,5920,5943,5945,5949,5952,5956,
  5960,5967,5972,5976,5984,5989,5991,5996,6001,6005,6010,6040,6080,6090,
  6098,6106,6113,6119,6123,6145,6167,6190,6202,6213,6240,6254,6262,6266,
  6300,6307,6314,6322,6331,6336,6356,6378,6412,6428,6472,6495,6497,6507,
  6534,6545,6547,6574,6598,6626,6635,6645,6652,6659,6666,6673,6681,6687,
  6694,6703,6712,6736,6745,6752,6756,6761,6765,6768,6789,6803,6816,6819,
  6828,6832,6839,6843,6849,6866,6870,6872,6879,6883,6888,6894,6919,6946,
  6970,6976,6979,6982,6985,6992,6997,7000,7007,7016,7025,7038,7048,7056,
  7061,7069,7073,7080,7084,7107,7109,7114,7159,7182,7199,7208,7215,7240,
  7247,7270,7283,7294,7305,7313,7322,7347,7353,7357,7359,7374,7381,7384,
  7405,7427,7437,7446,7453,7460,7469,7492,7501,7510,7538,7545,7554,7563,
  7568,7572,7579,7583,7590,7599,7608,7617,7626,7635,7644,7653,7662,7671,
  7680,7689,7698,7707,7716,7725,7734,7743,7752,7761,7770,7779,7788,7797,
  7806,7815,7818
};
static const unsigned char ag_fl[] = {
  1,4,4,4,4,4,4,4,4,4,4,4,4,2,2,4,2,2,2,2,2,0,1,2,1,1,1,1,2,1,1,1,1,1,1,
  1,1,1,1,1,1,2,1,1,2,1,1,2,1,1,1,2,0,1,2,1,1,2,0,1,3,1,2,1,2,1,1,1,1,1,
  1,1,2,3,1,2,1,2,1,2,1,3,1,5,0,1,3,4,1,1,3,4,4,3,4,2,3,4,4,1,2,5,3,1,1,
  1,1,2,0,1,1,3,1,1,4,4,0,4,9,1,1,1,1,1,1,1,1,1,1,2,6,1,1,1,1,1,1,1,1,1,
  1,1,2,6,6,6,2,2,1,6,1,2,3,2,2,0,3,1,6,6,6,6,1,1,7,1,1,1,4,1,2,1,1,1,2,
  1,1,2,1,1,2,1,2,1,1,1,1,1,2,0,1,3,1,2,4,1,1,1,2,0,1,2,3,1,1,0,1,3,4,1,
  1,1,1,1,1,5,1,2,1,5,1,2,1,1,1,1,5,1,2,1,1,1,1,3,5,1,1,1,1,1,2,0,1,2,1,
  1,2,0,1,2,2,2,1,1,1,1,1,1,2,1,1,3,2,1,1,3,2,1,3,3,1,1,1,1,1,4,4,2,4,4,
  3,4,4,3,4,4,3,1,1,3,1,1,2,1,1,1,1,2,0,1,1,2,1,1,3,1,1,1,1,2,2,3,5,1,2,
  3,1,3,1,0,1,3,2,2,1,2,0,1,2,1,2,3,4,0,1,3,4,1,1,1,1,1,1,1,1,1,1,1,5,5,
  5,1,1,1,5,5,5,3,3,3,0,1,0,1,3,5,5,1,1,2,0,1,2,5,1,1,2,2,3,0,1,2,3,3,1,
  2,1,1,1,2,0,1,1,1,6,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,1,2,6,
  5,2,1,1,1,2,0,1,3,2,5,3,4,2,1,1,1,1,1,1,4,2,3,1,2,3,4,2,2,2,1,5,0,2,2,
  2,0,1,2,3,5,1,1,2,5,1,5,1,1,1,2,3,1,7,1,1,1,1,1,1,1,5,1,2,1,1,1,1,1,1,
  1,1,1,1,1,1,2,3,4,7,2,2,2,1,1,1,4,5,1,1,3,3,2,5,1,1,1,1,1,1,1,1,1,1,3,
  5,1,0,1,7,9,1,1,1,1,2,1,1,1,1,3,1,5,1,5,1,5,5,0,1,3,3,4,4,5,5,2,4,1,1,
  1,1,1,7,1,5,1,1,1,4,4,0,1,3,4,6,5,7,5,7,1,4,3,1,1,3,3,2,5,1,1,1,1,1,1,
  1,1,1,1,1,1,1,5,1,5,3,1,0,1,7,5,1,5,1,2,3,4,5,6,7,8,5,3,3,32,1,1,5,1,2,
  0,1,4,1,1,5,5,1,1,4,2,3,1,1,5,1,1,1,1,1,7,5,5,1,1,2,0,1,3
};
static const unsigned short ag_ptt[] = {
    0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1, 39, 39,  3, 40, 42, 44, 46, 46, 38, 47, 49, 51, 52, 54, 56,
   58, 60, 62, 64, 68, 68, 66, 71, 71, 69, 74, 74, 72, 78, 79, 79, 80, 80,
   75, 83, 84, 84, 85, 85, 81, 82, 82, 91, 91, 89, 92, 92, 92, 92, 92, 92,
    4, 98,101,101, 99, 17,102, 15,103,103,104,105,108,108,109,109,106,107,
  110,111,111,112,112,113,113,114,114,116,116, 28, 28,115,117,122,123,123,
  124,124,118,120, 19,125,125,126,129,129,127,131,131,132,133,135,135,135,
  135,135,140,128,128,130,130,130,130,130,130,130,130,130,130,151,152,152,
  152,152,142,143,144,156,158,158,158,148,159,160,160,145,162,146,147,149,
  168,168,150,165,173, 31,175,179,179,176,180,183,183,177,186,186,184, 35,
  187,188,189,191,191,193,198,199,199,200,200,195,202,202,195,194,206,207,
  207,208,208,203,203,204,204,192,192,210,210,211,211,211,211,211,211,212,
  220,220,219,213,224,224,222,223,223,223,214,230,230,228,229,229,229,233,
  215,236,237,237,241,242,242,243,243,238,246,247,247,248,248,239,239,240,
  245,245,216, 34, 32, 33,252,255,255,253,253,260,259,254,254,263,256,256,
  269,269,269,269,269,270,270,271,272,272,272,273,273,273,274,265,265,275,
  276,276,279,283,283,281,280,286,287,287,288,288,284,268,289,277,277,141,
  141,291,293,293,291,153,296,297,297,297,172,172,298,301,301,298,298,303,
  304,304,305,305,299,302,302,302,302,266,266,306,306,307,307,307,307,307,
  307,307,307,307,307,307,308,309,309,320,320,321,310,311,312,313,313,313,
  329,329,330,330,313,314,315,332,333,333,334,334,171,316,317,217,217,337,
  338,267,267,339,339,340,345,345,342,347,348,348,349,349,343, 29,350,351,
  351,351,351,351,351,351,351,351,351,351,351,351,351,174,174,174,174,174,
    7,365,365,365,365,366,366,181,181,369,370,370,371,371,368,368,372,367,
  367,374,374, 13,375,375,376,377,377,379,379,379,379,380,380,381,381,381,
  381,382,389,389,386,386,388,388,388,388,383,392,392,  9,384,397,397,395,
  395,398, 11,400,400,399,403,403,403,403,403,403,403,401,411,411,412,412,
  412,412,412,412,412,412,412,412,412,412,413,413,413,402,426,427,428,429,
  429,429, 36, 36, 23,433,434,434,436,436,438,438,438,438,438,438,438,438,
  438,438,438,439,451,455,455,440,440,456,456,458,461,461,459,454,453,457,
  457,462,441,465,442,467,446,446,472,472,447,470,470,470,470,471,471,475,
  477,477,474,478,476,448,486,486,487,487,487,443,444,493,493,491,445,445,
  445,445,449,449,437,437,494, 25,495,496,496,497,497,498,498,498,498,498,
  498,498,498,498,498,498,498,498,499,508,506,510,511,512,512,505,501,514,
  502,516,516,516,516,516,516,516,516,500,519,519,520,522,517,503,526,526,
  528,528,524,527,527,504,504, 21,534,534,535,535, 27,536,536,537,537,537,
  537,537,540,538,539,544,545,545,546,546,543
};
static void ag_ra(PCB_DECL)
{
  switch(ag_rpx[(PCB).ag_ap]) {
    case 1: ag_rp_1(PCB_POINTER); break;
    case 2: ag_rp_2(PCB_POINTER); break;
    case 3: ag_rp_3(PCB_POINTER); break;
    case 4: ag_rp_4(PCB_POINTER); break;
    case 5: ag_rp_5(PCB_POINTER); break;
    case 6: ag_rp_6(PCB_POINTER); break;
    case 7: ag_rp_7(PCB_POINTER); break;
    case 8: ag_rp_8(PCB_POINTER); break;
    case 9: ag_rp_9(PCB_POINTER); break;
    case 10: ag_rp_10(PCB_POINTER); break;
    case 11: ag_rp_11(PCB_POINTER); break;
    case 12: ag_rp_12(PCB_POINTER); break;
    case 13: ag_rp_13(PCB_POINTER); break;
    case 14: ag_rp_14(PCB_POINTER); break;
    case 15: ag_rp_15(PCB_POINTER); break;
    case 16: V(0,ParserBasicToken) = ag_rp_16(PCB_POINTER); break;
    case 17: V(0,ParserBasicToken) = ag_rp_17(PCB_POINTER); break;
    case 18: V(0,ParserBasicToken) = ag_rp_18(PCB_POINTER); break;
    case 19: V(0,ParserBasicToken) = ag_rp_19(PCB_POINTER); break;
    case 20: V(0,ParserQuotedString) = ag_rp_20(PCB_POINTER); break;
    case 21: V(0,ParserBasicToken) = ag_rp_21(PCB_POINTER); break;
    case 22: V(0,ParserMethod) = ag_rp_22(PCB_POINTER); break;
    case 23: V(0,ParserMethod) = ag_rp_23(PCB_POINTER); break;
    case 24: V(0,ParserMethod) = ag_rp_24(PCB_POINTER); break;
    case 25: V(0,ParserMethod) = ag_rp_25(PCB_POINTER); break;
    case 26: V(0,ParserMethod) = ag_rp_26(PCB_POINTER); break;
    case 27: V(0,ParserMethod) = ag_rp_27(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 28: ag_rp_28(PCB_POINTER, V(0,ParserCSeq)); break;
    case 29: V(0,ParserCSeq) = ag_rp_29(PCB_POINTER, V(0,ParserSequenceNumber), V(2,ParserMethod)); break;
    case 30: V(0,ParserSequenceNumber) = ag_rp_30(PCB_POINTER); break;
    case 31: ag_rp_31(PCB_POINTER, V(0,ParserContentLength)); break;
    case 32: V(0,ParserContentLength) = ag_rp_32(PCB_POINTER); break;
    case 33: ag_rp_33(PCB_POINTER, V(0,ParserCallId)); break;
    case 34: V(0,ParserCallId) = ag_rp_34(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 35: V(0,ParserCallId) = ag_rp_35(PCB_POINTER, V(0,ParserBasicToken), V(2,ParserBasicToken)); break;
    case 36: ag_rp_36(PCB_POINTER); break;
    case 37: ag_rp_37(PCB_POINTER, V(0,ParserMediaType), V(2,ParserMediaSubType)); break;
    case 38: ag_rp_38(PCB_POINTER); break;
    case 39: ag_rp_39(PCB_POINTER); break;
    case 40: V(0,ParserMediaType) = ag_rp_40(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 41: V(0,ParserMediaSubType) = ag_rp_41(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 42: ag_rp_42(PCB_POINTER, V(0,ParserBasicToken), V(2,ParserGenericParamDataVal)); break;
    case 43: V(0,ParserGenericParamDataVal) = ag_rp_43(PCB_POINTER, V(2,ParserBasicToken)); break;
    case 44: V(0,ParserGenericParamDataVal) = ag_rp_44(PCB_POINTER, V(2,ParserQuotedString)); break;
    case 45: ag_rp_45(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 46: ag_rp_46(PCB_POINTER, V(0,ParserBasicToken), V(2,ParserGenericParamDataVal)); break;
    case 47: V(0,ParserGenericParamDataVal) = ag_rp_47(PCB_POINTER, V(2,ParserBasicToken)); break;
    case 48: V(0,ParserGenericParamDataVal) = ag_rp_48(PCB_POINTER, V(2,ParserQuotedString)); break;
    case 49: ag_rp_49(PCB_POINTER, V(0,ParserSipVersion), V(2,ParserStatusCode), V(4,ParserReasonPhrase)); break;
    case 50: ag_rp_50(PCB_POINTER, V(0,ParserSipVersion), V(2,ParserStatusCode)); break;
    case 51: V(0,ParserSipVersion) = ag_rp_51(PCB_POINTER); break;
    case 52: V(0,ParserReasonPhrase) = ag_rp_52(PCB_POINTER); break;
    case 53: V(0,ParserStatusCode) = ag_rp_53(PCB_POINTER, V(0,int), V(1,int), V(2,int)); break;
    case 54: ag_rp_54(PCB_POINTER); break;
    case 55: ag_rp_55(PCB_POINTER); break;
    case 56: ag_rp_56(PCB_POINTER); break;
    case 57: ag_rp_57(PCB_POINTER); break;
    case 58: ag_rp_58(PCB_POINTER, V(3,ParserViaParam)); break;
    case 59: ag_rp_59(PCB_POINTER, V(0,ParserProtocolName), V(4,ParserProtocolVersion), V(8,ParserTransport)); break;
    case 60: V(0,ParserProtocolName) = ag_rp_60(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 61: V(0,ParserProtocolName) = ag_rp_61(PCB_POINTER); break;
    case 62: V(0,ParserProtocolVersion) = ag_rp_62(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 63: V(0,ParserTransport) = ag_rp_63(PCB_POINTER, V(0,ParserTransport)); break;
    case 64: V(0,ParserTransport) = ag_rp_64(PCB_POINTER); break;
    case 65: V(0,ParserTransport) = ag_rp_65(PCB_POINTER); break;
    case 66: V(0,ParserTransport) = ag_rp_66(PCB_POINTER); break;
    case 67: V(0,ParserTransport) = ag_rp_67(PCB_POINTER); break;
    case 68: V(0,ParserTransport) = ag_rp_68(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 69: V(0,ParserPort) = ag_rp_69(PCB_POINTER); break;
    case 70: ag_rp_70(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 71: ag_rp_71(PCB_POINTER, V(0,ParserBasicToken), V(4,ParserPort)); break;
    case 72: V(0,ParserViaParam) = ag_rp_72(PCB_POINTER); break;
    case 73: V(0,ParserViaParam) = ag_rp_73(PCB_POINTER); break;
    case 74: V(0,ParserViaParam) = ag_rp_74(PCB_POINTER, V(0,ParserTTLParam)); break;
    case 75: V(0,ParserViaParam) = ag_rp_75(PCB_POINTER, V(0,ParserMaddrParam)); break;
    case 76: V(0,ParserViaParam) = ag_rp_76(PCB_POINTER, V(0,ParserReceived)); break;
    case 77: V(0,ParserViaParam) = ag_rp_77(PCB_POINTER, V(0,ParserBranch)); break;
    case 78: V(0,ParserViaParam) = ag_rp_78(PCB_POINTER, V(0,ParserPort)); break;
    case 79: V(0,ParserViaParam) = ag_rp_79(PCB_POINTER, V(0,ParserCompParam)); break;
    case 80: V(0,ParserViaParam) = ag_rp_80(PCB_POINTER, V(0,ParserSigCompIdParam)); break;
    case 81: V(0,ParserViaParam) = ag_rp_81(PCB_POINTER); break;
    case 82: ag_rp_82(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 83: ag_rp_83(PCB_POINTER, V(0,ParserBasicToken), V(4,ParserBasicToken)); break;
    case 84: ag_rp_84(PCB_POINTER, V(0,ParserBasicToken), V(4,ParserQuotedString)); break;
    case 85: ag_rp_85(PCB_POINTER, V(0,ParserBasicToken), V(4,ParserBasicToken)); break;
    case 86: V(0,ParserTTLParam) = ag_rp_86(PCB_POINTER, V(0,ParserTtlParam)); break;
    case 87: V(0,ParserTtlParam) = ag_rp_87(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 88: V(0,ParserBasicToken) = ag_rp_88(PCB_POINTER); break;
    case 89: V(0,ParserBasicToken) = ag_rp_89(PCB_POINTER); break;
    case 90: V(0,ParserBasicToken) = ag_rp_90(PCB_POINTER); break;
    case 91: V(0,ParserPort) = ag_rp_91(PCB_POINTER, V(1,ParserPort)); break;
    case 92: V(0,ParserPort) = ag_rp_92(PCB_POINTER); break;
    case 93: V(0,ParserPort) = ag_rp_93(PCB_POINTER, V(2,ParserPort)); break;
    case 94: V(0,ParserMaddrParam) = ag_rp_94(PCB_POINTER, V(0,ParserMaddrParam)); break;
    case 95: V(0,ParserMaddrParam) = ag_rp_95(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 96: V(0,ParserReceived) = ag_rp_96(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 97: V(0,ParserBranch) = ag_rp_97(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 98: V(0,ParserCompParam) = ag_rp_98(PCB_POINTER, V(4,ParserCompParam)); break;
    case 99: V(0,ParserCompParam) = ag_rp_99(PCB_POINTER); break;
    case 100: V(0,ParserCompParam) = ag_rp_100(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 101: V(0,ParserSigCompIdParam) = ag_rp_101(PCB_POINTER, V(5,ParserBasicToken)); break;
    case 102: V(0,ParserBasicToken) = ag_rp_102(PCB_POINTER); break;
    case 103: ag_rp_103(PCB_POINTER, V(0,ParserAbsoluteUri)); break;
    case 104: V(0,ParserAbsoluteUri) = ag_rp_104(PCB_POINTER, V(0,ParserBasicToken), V(3,ParserBasicToken)); break;
    case 105: V(0,ParserBasicToken) = ag_rp_105(PCB_POINTER); break;
    case 106: V(0,ParserDisplayName) = ag_rp_106(PCB_POINTER, V(0,ParserDisplayName)); break;
    case 107: V(0,ParserBasicToken) = ag_rp_107(PCB_POINTER); break;
    case 108: V(0,ParserBasicToken) = ag_rp_108(PCB_POINTER); break;
    case 109: ag_rp_109(PCB_POINTER); break;
    case 110: ag_rp_110(PCB_POINTER); break;
    case 111: ag_rp_111(PCB_POINTER); break;
    case 112: ag_rp_112(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 113: ag_rp_113(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 114: V(0,ParserBasicToken) = ag_rp_114(PCB_POINTER); break;
    case 115: V(0,ParserBasicToken) = ag_rp_115(PCB_POINTER); break;
    case 116: ag_rp_116(PCB_POINTER); break;
    case 117: ag_rp_117(PCB_POINTER); break;
    case 118: ag_rp_118(PCB_POINTER); break;
    case 119: ag_rp_119(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 120: ag_rp_120(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 121: ag_rp_121(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 122: ag_rp_122(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 123: ag_rp_123(PCB_POINTER, V(0,ParserEnumdiParamType)); break;
    case 124: ag_rp_124(PCB_POINTER); break;
    case 125: V(0,ParserBasicToken) = ag_rp_125(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 126: V(0,ParserBasicToken) = ag_rp_126(PCB_POINTER); break;
    case 127: V(0,ParserBasicToken) = ag_rp_127(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 128: V(0,ParserBasicToken) = ag_rp_128(PCB_POINTER); break;
    case 129: V(0,ParserBasicToken) = ag_rp_129(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 130: V(0,ParserBasicToken) = ag_rp_130(PCB_POINTER); break;
    case 131: V(0,ParserBasicToken) = ag_rp_131(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 132: V(0,ParserBasicToken) = ag_rp_132(PCB_POINTER); break;
    case 133: ag_rp_133(PCB_POINTER); break;
    case 134: ag_rp_134(PCB_POINTER); break;
    case 135: ag_rp_135(PCB_POINTER); break;
    case 136: V(0,ParserEnumdiParamType) = ag_rp_136(PCB_POINTER); break;
    case 137: ag_rp_137(PCB_POINTER); break;
    case 138: ag_rp_138(PCB_POINTER, V(0,ParserAbsoluteUri)); break;
    case 139: ag_rp_139(PCB_POINTER, V(0,ParserAbsoluteUri)); break;
    case 140: ag_rp_140(PCB_POINTER); break;
    case 141: ag_rp_141(PCB_POINTER); break;
    case 142: V(0,ParserAbsoluteUri) = ag_rp_142(PCB_POINTER, V(2,ParserAbsoluteUri)); break;
    case 143: V(0,ParserAbsoluteUri) = ag_rp_143(PCB_POINTER); break;
    case 144: ag_rp_144(PCB_POINTER); break;
    case 145: ag_rp_145(PCB_POINTER); break;
    case 146: V(0,ParserAbsoluteUri) = ag_rp_146(PCB_POINTER, V(2,ParserAbsoluteUri)); break;
    case 147: V(0,ParserAbsoluteUri) = ag_rp_147(PCB_POINTER); break;
    case 148: ag_rp_148(PCB_POINTER); break;
    case 149: ag_rp_149(PCB_POINTER, V(2,ParserOptionalHeaders)); break;
    case 150: ag_rp_150(PCB_POINTER, V(2,ParserOptionalHeaders)); break;
    case 151: ag_rp_151(PCB_POINTER, V(3,ParserOptionalHeaders)); break;
    case 152: ag_rp_152(PCB_POINTER, V(3,ParserOptionalHeaders)); break;
    case 153: ag_rp_153(PCB_POINTER); break;
    case 154: ag_rp_154(PCB_POINTER, V(3,ParserOptionalHeaders)); break;
    case 155: ag_rp_155(PCB_POINTER, V(3,ParserOptionalHeaders)); break;
    case 156: ag_rp_156(PCB_POINTER, V(3,ParserOptionalHeaders)); break;
    case 157: ag_rp_157(PCB_POINTER, V(3,ParserOptionalHeaders)); break;
    case 158: ag_rp_158(PCB_POINTER, V(0,ParserDisplayName), V(2,ParserBasicToken)); break;
    case 159: ag_rp_159(PCB_POINTER); break;
    case 160: ag_rp_160(PCB_POINTER); break;
    case 161: ag_rp_161(PCB_POINTER); break;
    case 162: ag_rp_162(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 163: ag_rp_163(PCB_POINTER, V(0,ParserBasicToken), V(2,ParserBasicToken)); break;
    case 164: V(0,ParserBasicToken) = ag_rp_164(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 165: V(0,ParserBasicToken) = ag_rp_165(PCB_POINTER); break;
    case 166: V(0,ParserBasicToken) = ag_rp_166(PCB_POINTER); break;
    case 167: ag_rp_167(PCB_POINTER); break;
    case 168: ag_rp_168(PCB_POINTER); break;
    case 169: ag_rp_169(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 170: ag_rp_170(PCB_POINTER, V(0,ParserBasicToken), V(2,ParserPort)); break;
    case 171: V(0,ParserBasicToken) = ag_rp_171(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 172: V(0,ParserBasicToken) = ag_rp_172(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 173: V(0,ParserBasicToken) = ag_rp_173(PCB_POINTER); break;
    case 174: V(0,ParserBasicToken) = ag_rp_174(PCB_POINTER); break;
    case 175: V(0,ParserBasicToken) = ag_rp_175(PCB_POINTER); break;
    case 176: ag_rp_176(PCB_POINTER); break;
    case 177: ag_rp_177(PCB_POINTER); break;
    case 178: ag_rp_178(PCB_POINTER); break;
    case 179: ag_rp_179(PCB_POINTER); break;
    case 180: ag_rp_180(PCB_POINTER, V(0,ParserTransport)); break;
    case 181: ag_rp_181(PCB_POINTER, V(0,ParserUserParam)); break;
    case 182: ag_rp_182(PCB_POINTER, V(0,ParserTtlParam)); break;
    case 183: ag_rp_183(PCB_POINTER, V(0,ParserMaddrParam)); break;
    case 184: ag_rp_184(PCB_POINTER, V(0,ParserMethod)); break;
    case 185: ag_rp_185(PCB_POINTER, V(0,ParserLrParamType)); break;
    case 186: ag_rp_186(PCB_POINTER, V(0,ParserCompParam)); break;
    case 187: ag_rp_187(PCB_POINTER, V(0,ParserSigCompIdParam)); break;
    case 188: ag_rp_188(PCB_POINTER, V(0,ParserTokenizedByParam)); break;
    case 189: ag_rp_189(PCB_POINTER, V(0,RvBool)); break;
    case 190: ag_rp_190(PCB_POINTER); break;
    case 191: V(0,ParserTransport) = ag_rp_191(PCB_POINTER, V(4,ParserTransport)); break;
    case 192: V(0,ParserUserParam) = ag_rp_192(PCB_POINTER, V(4,RvSipUserParam)); break;
    case 193: V(0,ParserUserParam) = ag_rp_193(PCB_POINTER, V(4,ParserOtherUser)); break;
    case 194: V(0,RvSipUserParam) = ag_rp_194(PCB_POINTER); break;
    case 195: V(0,RvSipUserParam) = ag_rp_195(PCB_POINTER); break;
    case 196: V(0,ParserOtherUser) = ag_rp_196(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 197: V(0,ParserTtlParam) = ag_rp_197(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 198: V(0,ParserMaddrParam) = ag_rp_198(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 199: V(0,ParserMethod) = ag_rp_199(PCB_POINTER, V(4,ParserMethod)); break;
    case 200: V(0,ParserLrParamType) = ag_rp_200(PCB_POINTER); break;
    case 201: V(0,ParserLrParamType) = ag_rp_201(PCB_POINTER); break;
    case 202: V(0,ParserLrParamType) = ag_rp_202(PCB_POINTER); break;
    case 203: V(0,ParserLrParamType) = ag_rp_203(PCB_POINTER); break;
    case 204: V(0,ParserCompParam) = ag_rp_204(PCB_POINTER, V(4,ParserCompParam)); break;
    case 205: V(0,ParserSigCompIdParam) = ag_rp_205(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 206: V(0,ParserBasicToken) = ag_rp_206(PCB_POINTER); break;
    case 207: V(0,ParserTokenizedByParam) = ag_rp_207(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 208: V(0,RvBool) = ag_rp_208(PCB_POINTER); break;
    case 209: ag_rp_209(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 210: ag_rp_210(PCB_POINTER, V(0,ParserBasicToken), V(1,ParserBasicToken)); break;
    case 211: V(0,ParserBasicToken) = ag_rp_211(PCB_POINTER); break;
    case 212: V(0,ParserBasicToken) = ag_rp_212(PCB_POINTER, V(2,ParserBasicToken)); break;
    case 213: V(0,ParserOptionalHeaders) = ag_rp_213(PCB_POINTER); break;
    case 214: V(0,ParserOptionalHeaders) = ag_rp_214(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 215: V(0,ParserBasicToken) = ag_rp_215(PCB_POINTER); break;
    case 216: V(0,ParserBasicToken) = ag_rp_216(PCB_POINTER); break;
    case 217: V(0,ParserBasicToken) = ag_rp_217(PCB_POINTER); break;
    case 218: V(0,ParserBasicToken) = ag_rp_218(PCB_POINTER); break;
    case 219: V(0,ParserBasicToken) = ag_rp_219(PCB_POINTER); break;
    case 220: ag_rp_220(PCB_POINTER); break;
    case 221: ag_rp_221(PCB_POINTER, V(0,ParserMethod), V(4,ParserSipVersion)); break;
    case 222: V(0,ParserMethod) = ag_rp_222(PCB_POINTER); break;
    case 223: V(0,ParserMethod) = ag_rp_223(PCB_POINTER); break;
    case 224: V(0,ParserMethod) = ag_rp_224(PCB_POINTER); break;
    case 225: V(0,ParserMethod) = ag_rp_225(PCB_POINTER); break;
    case 226: V(0,ParserMethod) = ag_rp_226(PCB_POINTER); break;
    case 227: V(0,ParserMethod) = ag_rp_227(PCB_POINTER); break;
    case 228: V(0,ParserMethod) = ag_rp_228(PCB_POINTER); break;
    case 229: V(0,ParserMethod) = ag_rp_229(PCB_POINTER); break;
    case 230: V(0,ParserMethod) = ag_rp_230(PCB_POINTER); break;
    case 231: V(0,ParserMethod) = ag_rp_231(PCB_POINTER); break;
    case 232: V(0,ParserMethod) = ag_rp_232(PCB_POINTER); break;
    case 233: V(0,ParserMethod) = ag_rp_233(PCB_POINTER); break;
    case 234: V(0,ParserMethod) = ag_rp_234(PCB_POINTER); break;
    case 235: V(0,ParserMethod) = ag_rp_235(PCB_POINTER, V(0,ParserMethod)); break;
    case 236: ag_rp_236(PCB_POINTER); break;
    case 237: ag_rp_237(PCB_POINTER, V(0,ParserAbsoluteUri)); break;
    case 238: ag_rp_238(PCB_POINTER, V(0,ParserAbsoluteUri)); break;
    case 239: ag_rp_239(PCB_POINTER); break;
    case 240: ag_rp_240(PCB_POINTER, V(0,ParserAbsoluteUri)); break;
    case 241: ag_rp_241(PCB_POINTER); break;
    case 242: ag_rp_242(PCB_POINTER); break;
    case 243: ag_rp_243(PCB_POINTER, V(2,ParserPartyParams)); break;
    case 244: ag_rp_244(PCB_POINTER); break;
    case 245: ag_rp_245(PCB_POINTER, V(1,ParserPartyParams)); break;
    case 246: ag_rp_246(PCB_POINTER, V(0,ParserDisplayName)); break;
    case 247: ag_rp_247(PCB_POINTER); break;
    case 248: V(0,ParserDisplayName) = ag_rp_248(PCB_POINTER, V(0,ParserQuotedString)); break;
    case 249: V(0,ParserDisplayName) = ag_rp_249(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 250: V(0,ParserBasicToken) = ag_rp_250(PCB_POINTER); break;
    case 251: V(0,ParserBasicToken) = ag_rp_251(PCB_POINTER); break;
    case 252: V(0,ParserTagParam) = ag_rp_252(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 253: V(0,ParserPartyParams) = ag_rp_253(PCB_POINTER, V(2,ParserPartyParams)); break;
    case 254: V(0,ParserPartyParams) = ag_rp_254(PCB_POINTER, V(0,ParserPartyParams), V(3,ParserPartyParams)); break;
    case 255: V(0,ParserPartyParams) = ag_rp_255(PCB_POINTER, V(0,ParserTagParam)); break;
    case 256: V(0,ParserPartyParams) = ag_rp_256(PCB_POINTER); break;
    case 257: ag_rp_257(PCB_POINTER); break;
    case 258: ag_rp_258(PCB_POINTER); break;
    case 259: ag_rp_259(PCB_POINTER); break;
    case 260: ag_rp_260(PCB_POINTER); break;
    case 261: ag_rp_261(PCB_POINTER); break;
    case 262: ag_rp_262(PCB_POINTER); break;
    case 263: ag_rp_263(PCB_POINTER); break;
    case 264: ag_rp_264(PCB_POINTER, V(0,ParserQVal)); break;
    case 265: ag_rp_265(PCB_POINTER, V(0,ParserContactActionType)); break;
    case 266: ag_rp_266(PCB_POINTER); break;
    case 267: ag_rp_267(PCB_POINTER); break;
    case 268: V(0,ParserQVal) = ag_rp_268(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 269: V(0,ParserBasicToken) = ag_rp_269(PCB_POINTER); break;
    case 270: V(0,ParserBasicToken) = ag_rp_270(PCB_POINTER); break;
    case 271: V(0,ParserContactActionType) = ag_rp_271(PCB_POINTER, V(4,ParserContactActionType)); break;
    case 272: V(0,ParserContactActionType) = ag_rp_272(PCB_POINTER); break;
    case 273: V(0,ParserContactActionType) = ag_rp_273(PCB_POINTER); break;
    case 274: ag_rp_274(PCB_POINTER); break;
    case 275: ag_rp_275(PCB_POINTER, V(0,ParserDeltaSeconds)); break;
    case 276: ag_rp_276(PCB_POINTER, V(2,ParserSipDate)); break;
    case 277: ag_rp_277(PCB_POINTER, V(0,ParserDeltaSeconds)); break;
    case 278: ag_rp_278(PCB_POINTER, V(0,ParserSipDate)); break;
    case 279: V(0,ParserDeltaSeconds) = ag_rp_279(PCB_POINTER); break;
    case 280: ag_rp_280(PCB_POINTER, V(0,ParserSipDate)); break;
    case 281: V(0,ParserSipDate) = ag_rp_281(PCB_POINTER, V(0,ParserDDMMYY), V(2,ParserTime)); break;
    case 282: V(0,ParserSipDate) = ag_rp_282(PCB_POINTER, V(0,ParserSipDate)); break;
    case 283: V(0,ParserSipDate) = ag_rp_283(PCB_POINTER, V(0,RvSipDateWeekDay), V(4,ParserDDMMYY), V(6,ParserTime)); break;
    case 284: V(0,RvSipDateWeekDay) = ag_rp_284(PCB_POINTER); break;
    case 285: V(0,RvSipDateWeekDay) = ag_rp_285(PCB_POINTER); break;
    case 286: V(0,RvSipDateWeekDay) = ag_rp_286(PCB_POINTER); break;
    case 287: V(0,RvSipDateWeekDay) = ag_rp_287(PCB_POINTER); break;
    case 288: V(0,RvSipDateWeekDay) = ag_rp_288(PCB_POINTER); break;
    case 289: V(0,RvSipDateWeekDay) = ag_rp_289(PCB_POINTER); break;
    case 290: V(0,RvSipDateWeekDay) = ag_rp_290(PCB_POINTER); break;
    case 291: V(0,ParserDDMMYY) = ag_rp_291(PCB_POINTER, V(0,ParserInt32), V(2,RvSipDateMonth), V(4,ParserInt32)); break;
    case 292: V(0,ParserInt32) = ag_rp_292(PCB_POINTER, V(0,int)); break;
    case 293: V(0,ParserInt32) = ag_rp_293(PCB_POINTER, V(0,int), V(1,int)); break;
    case 294: V(0,RvSipDateMonth) = ag_rp_294(PCB_POINTER); break;
    case 295: V(0,RvSipDateMonth) = ag_rp_295(PCB_POINTER); break;
    case 296: V(0,RvSipDateMonth) = ag_rp_296(PCB_POINTER); break;
    case 297: V(0,RvSipDateMonth) = ag_rp_297(PCB_POINTER); break;
    case 298: V(0,RvSipDateMonth) = ag_rp_298(PCB_POINTER); break;
    case 299: V(0,RvSipDateMonth) = ag_rp_299(PCB_POINTER); break;
    case 300: V(0,RvSipDateMonth) = ag_rp_300(PCB_POINTER); break;
    case 301: V(0,RvSipDateMonth) = ag_rp_301(PCB_POINTER); break;
    case 302: V(0,RvSipDateMonth) = ag_rp_302(PCB_POINTER); break;
    case 303: V(0,RvSipDateMonth) = ag_rp_303(PCB_POINTER); break;
    case 304: V(0,RvSipDateMonth) = ag_rp_304(PCB_POINTER); break;
    case 305: V(0,RvSipDateMonth) = ag_rp_305(PCB_POINTER); break;
    case 306: V(0,ParserInt32) = ag_rp_306(PCB_POINTER, V(0,int), V(1,int)); break;
    case 307: V(0,ParserInt32) = ag_rp_307(PCB_POINTER, V(0,int), V(1,int), V(2,int)); break;
    case 308: V(0,ParserInt32) = ag_rp_308(PCB_POINTER, V(0,int), V(1,int), V(2,int), V(3,int)); break;
    case 309: V(0,ParserTime) = ag_rp_309(PCB_POINTER, V(0,ParserInt32), V(2,ParserInt32), V(4,ParserInt32), V(6,ParserTimeZone)); break;
    case 310: V(0,ParserInt32) = ag_rp_310(PCB_POINTER, V(0,int), V(1,int)); break;
    case 311: V(0,ParserInt32) = ag_rp_311(PCB_POINTER, V(0,int), V(1,int)); break;
    case 312: V(0,ParserInt32) = ag_rp_312(PCB_POINTER, V(0,int), V(1,int)); break;
    case 313: V(0,ParserTimeZone) = ag_rp_313(PCB_POINTER); break;
    case 314: V(0,ParserTimeZone) = ag_rp_314(PCB_POINTER); break;
    case 315: V(0,ParserTimeZone) = ag_rp_315(PCB_POINTER); break;
    case 316: ag_rp_316(PCB_POINTER, V(0,ParserMethod)); break;
    case 317: ag_rp_317(PCB_POINTER, V(0,ParserMethod), V(4,ParserBasicToken)); break;
    case 318: ag_rp_318(PCB_POINTER); break;
    case 319: ag_rp_319(PCB_POINTER); break;
    case 320: ag_rp_320(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 321: ag_rp_321(PCB_POINTER); break;
    case 322: ag_rp_322(PCB_POINTER); break;
    case 323: ag_rp_323(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 324: ag_rp_324(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 325: ag_rp_325(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 326: ag_rp_326(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 327: ag_rp_327(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 328: ag_rp_328(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 329: ag_rp_329(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 330: ag_rp_330(PCB_POINTER, V(0,RvSipAuthStale)); break;
    case 331: ag_rp_331(PCB_POINTER, V(0,ParserAlgorithm)); break;
    case 332: ag_rp_332(PCB_POINTER, V(0,ParserQopValue)); break;
    case 333: ag_rp_333(PCB_POINTER); break;
    case 334: V(0,ParserBasicToken) = ag_rp_334(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 335: V(0,ParserBasicToken) = ag_rp_335(PCB_POINTER, V(0,ParserQuotedString)); break;
    case 336: V(0,ParserBasicToken) = ag_rp_336(PCB_POINTER); break;
    case 337: V(0,ParserBasicToken) = ag_rp_337(PCB_POINTER); break;
    case 338: ag_rp_338(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 339: V(0,ParserBasicToken) = ag_rp_339(PCB_POINTER); break;
    case 340: ag_rp_340(PCB_POINTER); break;
    case 341: ag_rp_341(PCB_POINTER); break;
    case 342: V(0,ParserBasicToken) = ag_rp_342(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 343: V(0,ParserBasicToken) = ag_rp_343(PCB_POINTER, V(0,ParserQuotedString)); break;
    case 344: V(0,ParserBasicToken) = ag_rp_344(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 345: V(0,ParserBasicToken) = ag_rp_345(PCB_POINTER, V(0,ParserQuotedString)); break;
    case 346: V(0,RvSipAuthStale) = ag_rp_346(PCB_POINTER); break;
    case 347: V(0,RvSipAuthStale) = ag_rp_347(PCB_POINTER); break;
    case 348: V(0,ParserAlgorithm) = ag_rp_348(PCB_POINTER, V(1,ParserAlgorithm)); break;
    case 349: ag_rp_349(PCB_POINTER); break;
    case 350: ag_rp_350(PCB_POINTER); break;
    case 351: ag_rp_351(PCB_POINTER); break;
    case 352: ag_rp_352(PCB_POINTER); break;
    case 353: V(0,ParserAlgorithm) = ag_rp_353(PCB_POINTER, V(2,ParserBasicToken), V(4,ParserAlgorithm)); break;
    case 354: V(0,ParserAlgorithm) = ag_rp_354(PCB_POINTER, V(1,ParserAlgorithm)); break;
    case 355: ag_rp_355(PCB_POINTER); break;
    case 356: V(0,ParserAlgorithm) = ag_rp_356(PCB_POINTER); break;
    case 357: V(0,ParserAlgorithm) = ag_rp_357(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 358: ag_rp_358(PCB_POINTER); break;
    case 359: ag_rp_359(PCB_POINTER); break;
    case 360: V(0,ParserBasicToken) = ag_rp_360(PCB_POINTER); break;
    case 361: V(0,ParserQopValue) = ag_rp_361(PCB_POINTER, V(5,ParserQopValue)); break;
    case 362: V(0,ParserQopValue) = ag_rp_362(PCB_POINTER, V(0,ParserAuthQopOptions)); break;
    case 363: V(0,ParserQopValue) = ag_rp_363(PCB_POINTER, V(0,ParserQopValue), V(4,ParserAuthQopOptions)); break;
    case 364: V(0,ParserAuthQopOptions) = ag_rp_364(PCB_POINTER); break;
    case 365: V(0,ParserAuthQopOptions) = ag_rp_365(PCB_POINTER); break;
    case 366: V(0,ParserAuthQopOptions) = ag_rp_366(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 367: V(0,ParserBasicToken) = ag_rp_367(PCB_POINTER, V(3,ParserBasicToken)); break;
    case 368: V(0,ParserBasicToken) = ag_rp_368(PCB_POINTER, V(3,ParserBasicToken)); break;
    case 369: V(0,ParserBasicToken) = ag_rp_369(PCB_POINTER); break;
    case 370: V(0,ParserBasicToken) = ag_rp_370(PCB_POINTER); break;
    case 371: V(0,ParserBasicToken) = ag_rp_371(PCB_POINTER); break;
    case 372: V(0,ParserBasicToken) = ag_rp_372(PCB_POINTER); break;
    case 373: V(0,ParserBasicToken) = ag_rp_373(PCB_POINTER); break;
    case 374: ag_rp_374(PCB_POINTER, V(0,ParserBasicToken), V(2,ParserGenericParamDataVal)); break;
    case 375: ag_rp_375(PCB_POINTER); break;
    case 376: ag_rp_376(PCB_POINTER); break;
    case 377: ag_rp_377(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 378: ag_rp_378(PCB_POINTER); break;
    case 379: ag_rp_379(PCB_POINTER); break;
    case 380: ag_rp_380(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 381: ag_rp_381(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 382: ag_rp_382(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 383: ag_rp_383(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 384: ag_rp_384(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 385: ag_rp_385(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 386: ag_rp_386(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 387: ag_rp_387(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 388: ag_rp_388(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 389: ag_rp_389(PCB_POINTER, V(0,RvSipAuthIntegrityProtected)); break;
    case 390: ag_rp_390(PCB_POINTER, V(0,RvSipAuthQopOption)); break;
    case 391: ag_rp_391(PCB_POINTER); break;
    case 392: ag_rp_392(PCB_POINTER, V(0,ParserAlgorithm)); break;
    case 393: V(0,ParserBasicToken) = ag_rp_393(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 394: V(0,ParserBasicToken) = ag_rp_394(PCB_POINTER, V(0,ParserQuotedString)); break;
    case 395: ag_rp_395(PCB_POINTER); break;
    case 396: ag_rp_396(PCB_POINTER); break;
    case 397: V(0,RvSipAuthQopOption) = ag_rp_397(PCB_POINTER, V(5,ParserAuthQopOptions)); break;
    case 398: V(0,ParserBasicToken) = ag_rp_398(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 399: V(0,ParserBasicToken) = ag_rp_399(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 400: V(0,ParserBasicToken) = ag_rp_400(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 401: V(0,ParserBasicToken) = ag_rp_401(PCB_POINTER); break;
    case 402: V(0,ParserBasicToken) = ag_rp_402(PCB_POINTER); break;
    case 403: V(0,ParserBasicToken) = ag_rp_403(PCB_POINTER); break;
    case 404: V(0,ParserBasicToken) = ag_rp_404(PCB_POINTER); break;
    case 405: V(0,ParserBasicToken) = ag_rp_405(PCB_POINTER); break;
    case 406: V(0,ParserBasicToken) = ag_rp_406(PCB_POINTER); break;
    case 407: V(0,ParserBasicToken) = ag_rp_407(PCB_POINTER); break;
    case 408: V(0,ParserBasicToken) = ag_rp_408(PCB_POINTER); break;
    case 409: V(0,ParserBasicToken) = ag_rp_409(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 410: V(0,ParserBasicToken) = ag_rp_410(PCB_POINTER); break;
    case 411: V(0,ParserBasicToken) = ag_rp_411(PCB_POINTER); break;
    case 412: V(0,ParserBasicToken) = ag_rp_412(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 413: V(0,ParserBasicToken) = ag_rp_413(PCB_POINTER); break;
    case 414: V(0,RvSipAuthIntegrityProtected) = ag_rp_414(PCB_POINTER); break;
    case 415: V(0,RvSipAuthIntegrityProtected) = ag_rp_415(PCB_POINTER); break;
    case 416: ag_rp_416(PCB_POINTER); break;
    case 417: ag_rp_417(PCB_POINTER); break;
    case 418: ag_rp_418(PCB_POINTER); break;
    case 419: ag_rp_419(PCB_POINTER); break;
    case 420: ag_rp_420(PCB_POINTER); break;
    case 421: ag_rp_421(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 422: ag_rp_422(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 423: ag_rp_423(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 424: ag_rp_424(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 425: ag_rp_425(PCB_POINTER); break;
    case 426: ag_rp_426(PCB_POINTER, V(5,ParserAuthQopOptions)); break;
    case 427: V(0,ParserBasicToken) = ag_rp_427(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 428: V(0,ParserBasicToken) = ag_rp_428(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 429: V(0,ParserBasicToken) = ag_rp_429(PCB_POINTER); break;
  }
  (PCB).la_ptr = (PCB).pointer;
}
static int ag_action_1_r_proc(PCB_DECL);
static int ag_action_2_r_proc(PCB_DECL);
static int ag_action_3_r_proc(PCB_DECL);
static int ag_action_4_r_proc(PCB_DECL);
static int ag_action_1_s_proc(PCB_DECL);
static int ag_action_3_s_proc(PCB_DECL);
static int ag_action_1_proc(PCB_DECL);
static int ag_action_2_proc(PCB_DECL);
static int ag_action_3_proc(PCB_DECL);
static int ag_action_4_proc(PCB_DECL);
static int ag_action_5_proc(PCB_DECL);
static int ag_action_6_proc(PCB_DECL);
static int ag_action_7_proc(PCB_DECL);
static int ag_action_8_proc(PCB_DECL);
static int ag_action_9_proc(PCB_DECL);
static int ag_action_10_proc(PCB_DECL);
static int ag_action_11_proc(PCB_DECL);
static int ag_action_8_proc(PCB_DECL);
static int (*const  ag_r_procs_scan[])(PCB_DECL) = {
  ag_action_1_r_proc,
  ag_action_2_r_proc,
  ag_action_3_r_proc,
  ag_action_4_r_proc
};
static int (*const  ag_s_procs_scan[])(PCB_DECL) = {
  ag_action_1_s_proc,
  ag_action_2_r_proc,
  ag_action_3_s_proc,
  ag_action_4_r_proc
};
static int (*const  ag_gt_procs_scan[])(PCB_DECL) = {
  ag_action_1_proc,
  ag_action_2_proc,
  ag_action_3_proc,
  ag_action_4_proc,
  ag_action_5_proc,
  ag_action_6_proc,
  ag_action_7_proc,
  ag_action_8_proc,
  ag_action_9_proc,
  ag_action_10_proc,
  ag_action_11_proc,
  ag_action_8_proc
};
static int ag_action_10_proc(PCB_DECL) {
  int ag_t = (PCB).token_number;
  do {
    ag_track(PCB_POINTER);
    (PCB).token_number = (SipParser_token_type) AG_TCV(INPUT_CODE(*(PCB).la_ptr));
    (PCB).la_ptr++;
    if (ag_key_index[(PCB).sn]) {
      unsigned ag_k = ag_key_index[(PCB).sn];
      int ag_ch = CONVERT_CASE(INPUT_CODE(*(PCB).pointer));
      while (ag_key_ch[ag_k] < ag_ch) ag_k++;
      if (ag_key_ch[ag_k] == ag_ch) ag_get_key_word((PCB_TYPE *)PCB_POINTER, ag_k);
    }
  } while ((PCB).token_number == (SipParser_token_type) ag_t);
  (PCB).la_ptr =  (PCB).pointer;
  return 1;
}
static int ag_action_11_proc(PCB_DECL) {
  int ag_t = (PCB).token_number;
  do {
    (*(int *) &(PCB).vs[(PCB).ssx]) = *(PCB).pointer;
    (PCB).ssx--;
    ag_track(PCB_POINTER);
    ag_ra(PCB_POINTER);
    if ((PCB).exit_flag != AG_RUNNING_CODE) return 0;
    (PCB).ssx++;
    (PCB).token_number = (SipParser_token_type) AG_TCV(INPUT_CODE(*(PCB).la_ptr));
    (PCB).la_ptr++;
    if (ag_key_index[(PCB).sn]) {
      unsigned ag_k = ag_key_index[(PCB).sn];
      int ag_ch = CONVERT_CASE(INPUT_CODE(*(PCB).pointer));
      while (ag_key_ch[ag_k] < ag_ch) ag_k++;
      if (ag_key_ch[ag_k] == ag_ch) ag_get_key_word((PCB_TYPE *)PCB_POINTER, ag_k);
    }
  }
  while ((PCB).token_number == (SipParser_token_type) ag_t);
  (PCB).la_ptr =  (PCB).pointer;
  return 1;
}
static int ag_action_3_r_proc(PCB_DECL) {
  int ag_sd = ag_fl[(PCB).ag_ap] - 1;
  if (ag_sd) (PCB).sn = (PCB).ss[(PCB).ssx -= ag_sd];
  (PCB).reduction_token = (SipParser_token_type) ag_ptt[(PCB).ag_ap];
  ag_ra(PCB_POINTER);
  return (PCB).exit_flag == AG_RUNNING_CODE;
}
static int ag_action_3_s_proc(PCB_DECL) {
  int ag_sd = ag_fl[(PCB).ag_ap] - 1;
  if (ag_sd) (PCB).sn = (PCB).ss[(PCB).ssx -= ag_sd];
  (PCB).reduction_token = (SipParser_token_type) ag_ptt[(PCB).ag_ap];
  ag_ra(PCB_POINTER);
  return (PCB).exit_flag == AG_RUNNING_CODE;;
}
static int ag_action_4_r_proc(PCB_DECL) {
  int ag_sd = ag_fl[(PCB).ag_ap] - 1;
  if (ag_sd) (PCB).sn = (PCB).ss[(PCB).ssx -= ag_sd];
  (PCB).reduction_token = (SipParser_token_type) ag_ptt[(PCB).ag_ap];
  return 1;
}
static int ag_action_2_proc(PCB_DECL) {
  if ((PCB).ssx >= 43) {
    (PCB).exit_flag = AG_STACK_ERROR_CODE;
    PARSER_STACK_OVERFLOW;
  }
  (*(int *) &(PCB).vs[(PCB).ssx]) = *(PCB).pointer;
  (PCB).ss[(PCB).ssx] = (PCB).sn;
  (PCB).ssx++;
  (PCB).sn = (PCB).ag_ap;
  ag_track(PCB_POINTER);
  return 0;
}
static int ag_action_9_proc(PCB_DECL) {
  if ((PCB).ssx >= 43) {
    (PCB).exit_flag = AG_STACK_ERROR_CODE;
    PARSER_STACK_OVERFLOW;
  }
  (PCB).vs[(PCB).ssx] = ag_null_value;
  (PCB).ss[(PCB).ssx] = (PCB).sn;
  (PCB).ssx++;
  (PCB).sn = (PCB).ag_ap;
  (PCB).la_ptr =  (PCB).pointer;
  return (PCB).exit_flag == AG_RUNNING_CODE;
}
static int ag_action_2_r_proc(PCB_DECL) {
  (PCB).ssx++;
  (PCB).sn = (PCB).ag_ap;
  return 0;
}
static int ag_action_7_proc(PCB_DECL) {
  --(PCB).ssx;
  (PCB).la_ptr =  (PCB).pointer;
  (PCB).exit_flag = AG_SUCCESS_CODE;
  return 0;
}
static int ag_action_1_proc(PCB_DECL) {
  ag_track(PCB_POINTER);
  (PCB).exit_flag = AG_SUCCESS_CODE;
  return 0;
}
static int ag_action_1_r_proc(PCB_DECL) {
  (PCB).exit_flag = AG_SUCCESS_CODE;
  return 0;
}
static int ag_action_1_s_proc(PCB_DECL) {
  (PCB).exit_flag = AG_SUCCESS_CODE;
  return 0;
}
static int ag_action_4_proc(PCB_DECL) {
  int ag_sd = ag_fl[(PCB).ag_ap] - 1;
  (PCB).reduction_token = (SipParser_token_type) ag_ptt[(PCB).ag_ap];
  (*(int *) &(PCB).vs[(PCB).ssx]) = *(PCB).pointer;
  if (ag_sd) (PCB).sn = (PCB).ss[(PCB).ssx -= ag_sd];
  else (PCB).ss[(PCB).ssx] = (PCB).sn;
  ag_track(PCB_POINTER);
  while ((PCB).exit_flag == AG_RUNNING_CODE) {
    unsigned ag_t1 = ag_sbe[(PCB).sn] + 1;
    unsigned ag_t2 = ag_sbt[(PCB).sn+1] - 1;
    do {
      unsigned ag_tx = (ag_t1 + ag_t2)/2;
      if (ag_tstt[ag_tx] < (const unsigned short)(PCB).reduction_token) ag_t1 = ag_tx + 1;
      else ag_t2 = ag_tx;
    } while (ag_t1 < ag_t2);
    (PCB).ag_ap = ag_pstt[ag_t1];
    if ((ag_s_procs_scan[ag_astt[ag_t1]])(PCB_POINTER) == 0) break;
  }
  return 0;
}
static int ag_action_3_proc(PCB_DECL) {
  int ag_sd = ag_fl[(PCB).ag_ap] - 1;
  (*(int *) &(PCB).vs[(PCB).ssx]) = *(PCB).pointer;
  if (ag_sd) (PCB).sn = (PCB).ss[(PCB).ssx -= ag_sd];
  else (PCB).ss[(PCB).ssx] = (PCB).sn;
  ag_track(PCB_POINTER);
  (PCB).reduction_token = (SipParser_token_type) ag_ptt[(PCB).ag_ap];
  ag_ra(PCB_POINTER);
  while ((PCB).exit_flag == AG_RUNNING_CODE) {
    unsigned ag_t1 = ag_sbe[(PCB).sn] + 1;
    unsigned ag_t2 = ag_sbt[(PCB).sn+1] - 1;
    do {
      unsigned ag_tx = (ag_t1 + ag_t2)/2;
      if (ag_tstt[ag_tx] < (const unsigned short)(PCB).reduction_token) ag_t1 = ag_tx + 1;
      else ag_t2 = ag_tx;
    } while (ag_t1 < ag_t2);
    (PCB).ag_ap = ag_pstt[ag_t1];
    if ((ag_s_procs_scan[ag_astt[ag_t1]])(PCB_POINTER) == 0) break;
  }
  return 0;
}
static int ag_action_8_proc(PCB_DECL) {
  (PCB).la_ptr =  (PCB).pointer;
  (PCB).exit_flag = AG_SYNTAX_ERROR_CODE;
  SYNTAX_ERROR;
  {(PCB).la_ptr = (PCB).pointer + 1; ag_track(PCB_POINTER);}
  return (PCB).exit_flag == AG_RUNNING_CODE;
}
static int ag_action_5_proc(PCB_DECL) {
  int ag_sd = ag_fl[(PCB).ag_ap];
  if (ag_sd) (PCB).sn = (PCB).ss[(PCB).ssx -= ag_sd];
  else {
    if ((PCB).ssx >= 43) {
      (PCB).exit_flag = AG_STACK_ERROR_CODE;
      PARSER_STACK_OVERFLOW;
    }
    (PCB).ss[(PCB).ssx] = (PCB).sn;
  }
  (PCB).la_ptr =  (PCB).pointer;
  (PCB).reduction_token = (SipParser_token_type) ag_ptt[(PCB).ag_ap];
  ag_ra(PCB_POINTER);
  while ((PCB).exit_flag == AG_RUNNING_CODE) {
    unsigned ag_t1 = ag_sbe[(PCB).sn] + 1;
    unsigned ag_t2 = ag_sbt[(PCB).sn+1] - 1;
    do {
      unsigned ag_tx = (ag_t1 + ag_t2)/2;
      if (ag_tstt[ag_tx] < (const unsigned short)(PCB).reduction_token) ag_t1 = ag_tx + 1;
      else ag_t2 = ag_tx;
    } while (ag_t1 < ag_t2);
    (PCB).ag_ap = ag_pstt[ag_t1];
    if ((ag_r_procs_scan[ag_astt[ag_t1]])(PCB_POINTER) == 0) break;
  }
  return (PCB).exit_flag == AG_RUNNING_CODE;
}
static int ag_action_6_proc(PCB_DECL) {
  int ag_sd = ag_fl[(PCB).ag_ap];
  (PCB).reduction_token = (SipParser_token_type) ag_ptt[(PCB).ag_ap];
  if (ag_sd) {
    (PCB).sn = (PCB).ss[(PCB).ssx -= ag_sd];
  }
  else {
    if ((PCB).ssx >= 43) {
      (PCB).exit_flag = AG_STACK_ERROR_CODE;
      PARSER_STACK_OVERFLOW;
    }
    (PCB).vs[(PCB).ssx] = ag_null_value;
    (PCB).ss[(PCB).ssx] = (PCB).sn;
  }
  (PCB).la_ptr =  (PCB).pointer;
  while ((PCB).exit_flag == AG_RUNNING_CODE) {
    unsigned ag_t1 = ag_sbe[(PCB).sn] + 1;
    unsigned ag_t2 = ag_sbt[(PCB).sn+1] - 1;
    do {
      unsigned ag_tx = (ag_t1 + ag_t2)/2;
      if (ag_tstt[ag_tx] < (const unsigned short)(PCB).reduction_token) ag_t1 = ag_tx + 1;
      else ag_t2 = ag_tx;
    } while (ag_t1 < ag_t2);
    (PCB).ag_ap = ag_pstt[ag_t1];
    if ((ag_r_procs_scan[ag_astt[ag_t1]])(PCB_POINTER) == 0) break;
  }
  return (PCB).exit_flag == AG_RUNNING_CODE;
}
void init_SipParser(SipParser_pcb_type *PCB_POINTER) {
  (PCB).la_ptr = (PCB).pointer;
  (PCB).error_message = "Syntax Error";
  (PCB).ss[0] = (PCB).sn = (PCB).ssx = 0;
  (PCB).exit_flag = AG_RUNNING_CODE;
  (PCB).line = FIRST_LINE;
  (PCB).column = FIRST_COLUMN;
}
void SipParser(SipParser_pcb_type *PCB_POINTER) {
  init_SipParser(PCB_POINTER);
  (PCB).exit_flag = AG_RUNNING_CODE;
  while ((PCB).exit_flag == AG_RUNNING_CODE) {
    unsigned ag_t1 = ag_sbt[(PCB).sn];
    if (ag_tstt[ag_t1]) {
      unsigned ag_t2 = ag_sbe[(PCB).sn] - 1;
      (PCB).token_number = (SipParser_token_type) AG_TCV(INPUT_CODE(*(PCB).la_ptr));
      (PCB).la_ptr++;
      if (ag_key_index[(PCB).sn]) {
        unsigned ag_k = ag_key_index[(PCB).sn];
        int ag_ch = CONVERT_CASE(INPUT_CODE(*(PCB).pointer));
        while (ag_key_ch[ag_k] < ag_ch) ag_k++;
        if (ag_key_ch[ag_k] == ag_ch) ag_get_key_word((PCB_TYPE *)PCB_POINTER, ag_k);
      }
      do {
        unsigned ag_tx = (ag_t1 + ag_t2)/2;
        if (ag_tstt[ag_tx] > (const unsigned short)(PCB).token_number)
          ag_t1 = ag_tx + 1;
        else ag_t2 = ag_tx;
      } while (ag_t1 < ag_t2);
      if (ag_tstt[ag_t1] != (const unsigned short)(PCB).token_number)
        ag_t1 = ag_sbe[(PCB).sn];
    }
    (PCB).ag_ap = ag_pstt[ag_t1];
    (ag_gt_procs_scan[ag_astt[ag_t1]])((PCB_TYPE *)PCB_POINTER);
  }
}
#elif defined(RV_SIP_AUTH_ON) && defined(RV_SIP_PRIMITIVES) && !defined(RV_SIP_IMS_DCS_HEADER_SUPPORT) && !defined(RV_SIP_IMS_HEADER_SUPPORT) && !defined(RV_SIP_EXTENDED_HEADER_SUPPORT) && !defined(RV_SIP_JSR32_SUPPORT) && !defined(RV_SIP_LIGHT) && !defined(RV_SIP_TEL_URI_SUPPORT) && !defined(RV_SIP_SUBS_ON)
/*
 AnaGram Parsing Engine
 Copyright (c) 1993-1999, Parsifal Software.
 All Rights Reserved.
 Serial number 2P20671
 Registered to:
   Meir Fuchs
   RADVision Ltd.
*/
#ifndef PARSERENGINESIPPRIMITIVES_H_1248033514
#include ".\ParserEngineSipPrimitives.h"
#endif
#ifndef PARSERENGINESIPPRIMITIVES_H_1248033514
#error Mismatched header file
#endif
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define RULE_CONTEXT (&((PCB).cs[(PCB).ssx]))
#define ERROR_CONTEXT ((PCB).cs[(PCB).error_frame_ssx])
#define CONTEXT ((PCB).cs[(PCB).ssx])
#ifndef PCB_TYPE
#define PCB_TYPE SipParser_pcb_type
#endif
#define PCB (*pcb_pointer)
#define PCB_DECL PCB_TYPE *pcb_pointer
#define PCB_POINTER pcb_pointer
#ifndef CONVERT_CASE
static const char agCaseTable[31] = {
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,    0,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
};
static int agConvertCase(int c) {
  if (c >= 'a' && c <= 'z') return c ^= 0x20;
  if (c >= 0xe0 && c < 0xff) c ^= agCaseTable[c-0xe0];
  return c;
}
#define CONVERT_CASE(c) agConvertCase(c)
#endif
#ifndef TAB_SPACING
#define TAB_SPACING 8
#endif
#define ag_rp_1(PCB_POINTER) (SET_HEADER_VALUE())
#define ag_rp_2(PCB_POINTER) (SIP_BEGIN_TOKEN())
#define ag_rp_3(PCB_POINTER) (SIP_BEGIN_TOKEN())
static void ag_rp_4(PCB_DECL) {
/* Line 318, RV SIP Stack */
        SIP_BEGIN_TOKEN();
        REMOVE_COMMA_BETWEEN_HEADERS();
    
}
#define ag_rp_5(PCB_POINTER) (SIP_BEGIN_TOKEN())
#define ag_rp_6(PCB_POINTER) (SIP_BEGIN_TOKEN())
#define ag_rp_7(PCB_POINTER) (SIP_BEGIN_TOKEN())
#define ag_rp_8(PCB_POINTER) (SIP_BEGIN_TOKEN())
#define ag_rp_9(PCB_POINTER) (SIP_BEGIN_TOKEN())
#define ag_rp_10(PCB_POINTER) (SIP_BEGIN_TOKEN())
#define ag_rp_11(PCB_POINTER) (SIP_BEGIN_TOKEN())
static void ag_rp_12(PCB_DECL) {
/* Line 364, RV SIP Stack */
                PCB.isWithinAngleBrackets = RV_TRUE;
                SIP_BEGIN_TOKEN();
            
}
static void ag_rp_13(PCB_DECL) {
/* Line 371, RV SIP Stack */
                PCB.isWithinAngleBrackets = RV_FALSE;
            
}
#define ag_rp_14(PCB_POINTER) (SIP_BEGIN_TOKEN())
#define ag_rp_15(PCB_POINTER) (SIP_BEGIN_TOKEN())
#define ag_rp_16(PCB_POINTER) (CUR_STRING())
#define ag_rp_17(PCB_POINTER) (CUR_STRING())
#define ag_rp_18(PCB_POINTER) (CUR_STRING())
#define ag_rp_19(PCB_POINTER) (CUR_STRING())
#define ag_rp_20(PCB_POINTER) (CUR_STRING())
#define ag_rp_21(PCB_POINTER) (CUR_STRING())
#define ag_rp_22(PCB_POINTER) (CUR_METHOD(PARSER_METHOD_TYPE_INVITE))
#define ag_rp_23(PCB_POINTER) (CUR_METHOD(PARSER_METHOD_TYPE_ACK))
#define ag_rp_24(PCB_POINTER) (CUR_METHOD(PARSER_METHOD_TYPE_BYE))
#define ag_rp_25(PCB_POINTER) (CUR_METHOD(PARSER_METHOD_TYPE_CANCEL))
#define ag_rp_26(PCB_POINTER) (CUR_METHOD(PARSER_METHOD_TYPE_REGISTER))
static ParserMethod ag_rp_27(PCB_DECL, ParserBasicToken t) {
/* Line 515, RV SIP Stack */
 ParserMethod method;
                            method.type = PARSER_METHOD_TYPE_OTHER;
                            method.other = t;
                            RV_UNUSED_ARG(pcb_pointer);
                            return method;
                          
}
static void ag_rp_28(PCB_DECL, ParserCSeq cseq) {
/* Line 531, RV SIP Stack */
   
       RvStatus status;
       status = ParserInitCSeq(PCB.pParserMgr, PCB.pointer,&cseq,PCB.eHeaderType,PCB.pSipObject);
       if(RV_ERROR_ILLEGAL_SYNTAX == status)
       {
           PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
           SYNTAX_ERROR;
       }
       else if (RV_OK != status)
       {
           
           PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
           PCB.eStat = status;
       }
    
}
static ParserCSeq ag_rp_29(PCB_DECL, ParserSequenceNumber sequenceNumber, ParserMethod method) {
/* Line 552, RV SIP Stack */
        ParserCSeq cseq;
        cseq.method = method;
		cseq.sequenceNumber = sequenceNumber;
        RV_UNUSED_ARG(pcb_pointer);
        return(cseq);
	 
}
#define ag_rp_30(PCB_POINTER) (CUR_STRING())
static void ag_rp_31(PCB_DECL, ParserContentLength contentLength) {
/* Line 574, RV SIP Stack */
          RvStatus status;
          contentLength.isCompact = PCB.isCompactForm;
          status = ParserInitContentLength(PCB.pParserMgr,PCB.pointer,&contentLength,PCB.pSipObject);
          if(RV_ERROR_ILLEGAL_SYNTAX == status)
          {
              PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
              SYNTAX_ERROR;
          }
          else if (RV_OK != status)
          {
            
            PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
            PCB.eStat = status;
          }
     
}
static ParserContentLength ag_rp_32(PCB_DECL) {
/* Line 597, RV SIP Stack */
        ParserContentLength contentLength;
        contentLength.contentLenVal = CUR_STRING();
        contentLength.isCompact = RV_FALSE;
        return contentLength;
    
}
static void ag_rp_33(PCB_DECL, ParserCallId callId) {
/* Line 613, RV SIP Stack */
  
          RvStatus status;
          callId.isCompact = PCB.isCompactForm;
          status = ParserInitCallId(PCB.pParserMgr,PCB.pointer,&callId,PCB.pSipObject);
          if(RV_ERROR_ILLEGAL_SYNTAX == status)
          {
              PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
              SYNTAX_ERROR;
          }
          else if(RV_OK != status)
          {
            
            PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
            PCB.eStat = status;
          }
    
}
static ParserCallId ag_rp_34(PCB_DECL, ParserBasicToken token) {
/* Line 635, RV SIP Stack */
         ParserCallId callId;
         callId.callIdVal = token;
         callId.isCompact = PCB.isCompactForm;
         return callId;
    
}
static ParserCallId ag_rp_35(PCB_DECL, ParserBasicToken token, ParserBasicToken token2) {
/* Line 642, RV SIP Stack */
        ParserCallId callId;
        token.len = (token2.len +token.len + 1);
        callId.callIdVal = token;
        callId.isCompact = PCB.isCompactForm;
        return callId;
    
}
static void ag_rp_36(PCB_DECL) {
/* Line 660, RV SIP Stack */
        RvStatus status;
        PCB.contentType.isCompactForm = PCB.isCompactForm;
        status = ParserInitContentType(PCB.pParserMgr,PCB.pointer,&(PCB.contentType),PCB.pSipObject);
        if(RV_ERROR_ILLEGAL_SYNTAX == status)
        {
             PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
             SYNTAX_ERROR;
        }
        else if (RV_OK != status) 
        {
            
            PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
            PCB.eStat = status;
        }
        ParserCleanExtParams (PCB.pExtParams);
		ParserCleanExtParams (PCB.pUrlExtParams);
     
}
static void ag_rp_37(PCB_DECL, ParserMediaType t, ParserMediaSubType st) {
/* Line 689, RV SIP Stack */
 
        PCB.contentType.mediaType    = t;
        PCB.contentType.mediaSubType = st;
        PCB.contentType.genericParamList = NULL;
        
        if (0 != ((ParserExtensionString *)PCB.pExtParams)->size)
        {
             PCB.contentType.genericParamList=PCB.pExtParams;
        }
    
}
static void ag_rp_38(PCB_DECL) {
/* Line 702, RV SIP Stack */
 
               RV_UNUSED_ARG(pcb_pointer);
            
}
static void ag_rp_39(PCB_DECL) {
/* Line 766, RV SIP Stack */
        
        
        RV_UNUSED_ARG(pcb_pointer);
    
}
#define ag_rp_40(PCB_POINTER, t) (t)
#define ag_rp_41(PCB_POINTER, t) (t)
static void ag_rp_42(PCB_DECL, ParserBasicToken token1, ParserGenericParamDataVal token2) {
/* Line 931, RV SIP Stack */
      if (0!=((ParserExtensionString *)PCB.pExtParams)->size)
      {
          
          APPEND_DATA(";", 1,PCB.pExtParams);
          APPEND_DATA(token1.buf, token1.len,PCB.pExtParams);
          APPEND_DATA("=" , 1,PCB.pExtParams);
          APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);            
      }
      else
      {      
          
          APPEND_DATA(token1.buf, token1.len,PCB.pExtParams);
          APPEND_DATA("=" , 1,PCB.pExtParams);
          APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);            
      }
   
}
#define ag_rp_43(PCB_POINTER, token) (token)
#define ag_rp_44(PCB_POINTER, qs) (qs)
static void ag_rp_45(PCB_DECL, ParserBasicToken token) {
/* Line 1026, RV SIP Stack */
      if (0!=((ParserExtensionString *)PCB.pExtParams)->size)
      {
          
          APPEND_DATA(";", 1,PCB.pExtParams);
          APPEND_DATA(token.buf,token.len, PCB.pExtParams);
      }
      else
      {      
          
          APPEND_DATA(token.buf,token.len,PCB.pExtParams);
      }
    
}
static void ag_rp_46(PCB_DECL, ParserBasicToken token1, ParserGenericParamDataVal token2) {
/* Line 1040, RV SIP Stack */
        if (0!=((ParserExtensionString *)PCB.pExtParams)->size)
        {
          
          APPEND_DATA(";", 1,PCB.pExtParams);
          APPEND_DATA(token1.buf, token1.len,PCB.pExtParams);
          APPEND_DATA("=" , 1,PCB.pExtParams);
          APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);            
        }
        else
        {      
          
          APPEND_DATA(token1.buf, token1.len,PCB.pExtParams);
          APPEND_DATA("=" , 1,PCB.pExtParams);
          APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);            
        }
    
}
#define ag_rp_47(PCB_POINTER, token) (token)
#define ag_rp_48(PCB_POINTER, qs) (qs)
static void ag_rp_49(PCB_DECL, ParserSipVersion ver, ParserStatusCode status, ParserReasonPhrase reason) {
/* Line 1074, RV SIP Stack */
        RvStatus        stat;
        ParserStatusLine statusLine;
        statusLine.sipVersion = ver;
        statusLine.statusCode = status;
        statusLine.reasonPhrase = reason;
        stat = ParserInitStatusLine(PCB.pParserMgr,&PCB, PCB.pointer, &statusLine, PCB.pSipObject);
        if (RV_ERROR_ILLEGAL_SYNTAX == stat)
        {
            PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
            SYNTAX_ERROR;
        }
        else if (RV_OK != stat)
        {
            
            PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
            PCB.eStat = stat;
        }
    
}
static void ag_rp_50(PCB_DECL, ParserSipVersion ver, ParserStatusCode status) {
/* Line 1094, RV SIP Stack */
        RvStatus        stat;
        ParserStatusLine statusLine;
        statusLine.sipVersion = ver;
        statusLine.statusCode = status;       
        statusLine.reasonPhrase.buf=NULL;
        statusLine.reasonPhrase.len=0;
        stat = ParserInitStatusLine(PCB.pParserMgr,&PCB, PCB.pointer, &statusLine, PCB.pSipObject);
        if(RV_ERROR_ILLEGAL_SYNTAX == stat)
        {
            PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
            SYNTAX_ERROR;
        }
        else if (RV_OK != stat)
        {
            
            PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
            PCB.eStat = stat;
        }
    
}
#define ag_rp_51(PCB_POINTER) (CUR_STRING())
#define ag_rp_52(PCB_POINTER) (CUR_STRING())
static ParserStatusCode ag_rp_53(PCB_DECL, int D100, int D10, int D1) {
/* Line 1132, RV SIP Stack */
        RvUint16 code;
        RV_UNUSED_ARG(pcb_pointer);
        code =  (RvUint16)((D100 - '0')*100 + (D10 - '0')*10 + (D1 - '0'));
        return code;
    
}
static void ag_rp_54(PCB_DECL) {
/* Line 1196, RV SIP Stack */
 
       RvStatus status;
       PCB.singleVia.isCompact    = PCB.isCompactForm;
       status = ParserInitVia(PCB.pParserMgr, &PCB, PCB.pointer, &(PCB.singleVia),PCB.eHeaderType, PCB.pSipObject);
       if(RV_ERROR_ILLEGAL_SYNTAX == status)
       {
           PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
           SYNTAX_ERROR;
       }
       else if (RV_OK != status && RV_ERROR_NOTSUPPORTED != status)
       {
            
            PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
            PCB.eStat = status;
       }
        
       ParserCleanExtParams (PCB.pExtParams);
    
}
static void ag_rp_55(PCB_DECL) {
/* Line 1216, RV SIP Stack */
 
       RvStatus status;
       PCB.singleVia.isCompact    = PCB.isCompactForm;
       status = ParserInitVia(PCB.pParserMgr,&PCB, PCB.pointer,&(PCB.singleVia),PCB.eHeaderType, PCB.pSipObject);
       if(RV_ERROR_ILLEGAL_SYNTAX == status)
       {
           PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
           SYNTAX_ERROR;
       }
       else if (RV_OK != status && RV_ERROR_NOTSUPPORTED != status)
       {
            
            PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
            PCB.eStat = status;
       }
        
       ParserCleanExtParams (PCB.pExtParams);
    
}
static void ag_rp_56(PCB_DECL) {
/* Line 1237, RV SIP Stack */
 
        PCB.singleVia.sentProtocol = PCB.sentProtocol;
        PCB.singleVia.sentBy       = PCB.viaSentBy;
         
        if (((ParserExtensionString *)PCB.pExtParams)->size !=0)
        {
            if (PCB.singleVia.params.isOtherParams==RV_FALSE)
            {
                PCB.singleVia.params.isOtherParams = RV_TRUE;
                PCB.singleVia.params.otherParam = PCB.pExtParams;
            }
        }                
    
}
static void ag_rp_57(PCB_DECL) {
/* Line 1254, RV SIP Stack */
 
        PCB.singleVia.params.hidden     = RV_FALSE;
        PCB.singleVia.params.alias     = RV_FALSE;
        PCB.singleVia.params.isTtl      = RV_FALSE;
        PCB.singleVia.params.isMaddr    = RV_FALSE;
	    PCB.singleVia.params.isReceived = RV_FALSE;
        PCB.singleVia.params.isBranch   = RV_FALSE;
        PCB.singleVia.params.isOtherParams=RV_FALSE;  
        PCB.singleVia.params.isRport	= RV_FALSE;
        PCB.singleVia.params.isComp     = RV_FALSE;
	PCB.singleVia.params.isSigCompId= RV_FALSE;
    
}
static void ag_rp_58(PCB_DECL, ParserViaParam p) {
/* Line 1269, RV SIP Stack */
        switch( p.paramType )
        {
            case PARSER_PARAM_TYPE_HIDDEN :
                PCB.singleVia.params.hidden = RV_TRUE; 
                break;
            case PARSER_PARAM_TYPE_ALIAS:
                PCB.singleVia.params.alias = RV_TRUE; 
                break;
            case PARSER_PARAM_TYPE_TTL :
                PCB.singleVia.params.isTtl = RV_TRUE;
                PCB.singleVia.params.ttl   = p.param.ttl;
                break;
            
            case PARSER_PARAM_TYPE_MADDR :
                PCB.singleVia.params.isMaddr = RV_TRUE;
                PCB.singleVia.params.maddr   = p.param.maddr;
                break;
            
            case PARSER_PARAM_TYPE_RECEIVED :
                PCB.singleVia.params.isReceived = RV_TRUE;
                PCB.singleVia.params.received   = p.param.received;
                break;
            
            case PARSER_PARAM_TYPE_BRANCH :
                PCB.singleVia.params.isBranch = RV_TRUE;
                PCB.singleVia.params.branch   = p.param.branch;
                break;
            case PARSER_PARAM_TYPE_RPORT:
                PCB.singleVia.params.isRport = RV_TRUE;
                PCB.singleVia.params.rPort   = p.param.rport;
                break;
                 
            case PARSER_PARAM_TYPE_COMP :
                PCB.singleVia.params.isComp  = RV_TRUE;
                PCB.singleVia.params.comp    = p.param.comp;
                break;   
	case PARSER_PARAM_TYPE_SIGCOMPID:
		PCB.singleVia.params.isSigCompId = RV_TRUE;
		PCB.singleVia.params.sigCompId   = p.param.sigCompId;
		break;
            default:
                if (p.isOther == RV_TRUE)
                {
                     PCB.singleVia.params.isOtherParams = RV_TRUE;
                     PCB.singleVia.params.otherParam   = p.otherParam;
                }                   
                break;
        }       
    
}
static void ag_rp_59(PCB_DECL, ParserProtocolName pn, ParserProtocolVersion pv, ParserTransport t) {
/* Line 1327, RV SIP Stack */
 
        PCB.sentProtocol.protocolName    = pn;
        PCB.sentProtocol.protocolVersion = pv;
        PCB.sentProtocol.transport       = t;
    
}
#define ag_rp_60(PCB_POINTER, t) (t)
#define ag_rp_61(PCB_POINTER) (CUR_STRING())
#define ag_rp_62(PCB_POINTER, t) (t)
#define ag_rp_63(PCB_POINTER, t) (t)
static ParserTransport ag_rp_64(PCB_DECL) {
/* Line 1354, RV SIP Stack */
        ParserTransport t;
        t.transport    = RVSIP_TRANSPORT_UDP;
        t.strTransport = CUR_STRING();
        return t;
    
}
static ParserTransport ag_rp_65(PCB_DECL) {
/* Line 1361, RV SIP Stack */
        ParserTransport t;
        t.transport    = RVSIP_TRANSPORT_TCP;
        t.strTransport = CUR_STRING();
        return t;
    
}
static ParserTransport ag_rp_66(PCB_DECL) {
/* Line 1368, RV SIP Stack */
        ParserTransport t;
        t.transport    = RVSIP_TRANSPORT_SCTP;
        t.strTransport = CUR_STRING();
        return t;
    
}
static ParserTransport ag_rp_67(PCB_DECL) {
/* Line 1375, RV SIP Stack */
        ParserTransport t;
        t.transport    = RVSIP_TRANSPORT_TLS;
        t.strTransport = CUR_STRING();
        return t;
    
}
static ParserTransport ag_rp_68(PCB_DECL, ParserBasicToken token) {
/* Line 1382, RV SIP Stack */
        ParserTransport t;
        t.transport    = RVSIP_TRANSPORT_OTHER;
        t.strTransport = token;
        RV_UNUSED_ARG(pcb_pointer);
        return t;
    
}
#define ag_rp_69(PCB_POINTER) (CUR_STRING())
static void ag_rp_70(PCB_DECL, ParserBasicToken host) {
/* Line 1401, RV SIP Stack */
 
        
        PCB.viaSentBy.isPort   = RV_FALSE;
        PCB.viaSentBy.hostName = host;
     
}
static void ag_rp_71(PCB_DECL, ParserBasicToken host, ParserPort port) {
/* Line 1407, RV SIP Stack */
 
        
        PCB.viaSentBy.isPort   = RV_TRUE;
        PCB.viaSentBy.hostName = host;
        PCB.viaSentBy.port     = port;
      
}
static ParserViaParam ag_rp_72(PCB_DECL) {
/* Line 1421, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_HIDDEN;                   
          viaParam.param.hidden = RV_TRUE;
	      viaParam.isOther = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_73(PCB_DECL) {
/* Line 1430, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_ALIAS;                   
          viaParam.param.alias = RV_TRUE;
	      viaParam.isOther = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_74(PCB_DECL, ParserTTLParam t) {
/* Line 1439, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_TTL;                   
          viaParam.param.ttl = t;
	      viaParam.isOther = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_75(PCB_DECL, ParserMaddrParam ma) {
/* Line 1448, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_MADDR;                   
          viaParam.param.maddr = ma;
	      viaParam.isOther = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_76(PCB_DECL, ParserReceived r) {
/* Line 1457, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_RECEIVED;                   
          viaParam.param.received = r;
	      viaParam.isOther = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_77(PCB_DECL, ParserBranch b) {
/* Line 1466, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_BRANCH;                   
          viaParam.param.branch = b;
	      viaParam.isOther = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_78(PCB_DECL, ParserPort rport) {
/* Line 1476, RV SIP Stack */
          ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_RPORT;                   
          viaParam.param.rport = rport;
	      viaParam.isOther = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_79(PCB_DECL, ParserCompParam comp) {
/* Line 1485, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType  = PARSER_PARAM_TYPE_COMP;                   
          viaParam.param.comp = comp;
	      viaParam.isOther    = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_80(PCB_DECL, ParserSigCompIdParam sigcompid) {
/* Line 1495, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType  = PARSER_PARAM_TYPE_SIGCOMPID;                   
          viaParam.param.sigCompId = sigcompid;
	      viaParam.isOther    = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_81(PCB_DECL) {
/* Line 1505, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_OTHER; 
	      viaParam.isOther = RV_TRUE;
          viaParam.otherParam = PCB.pExtParams;
          return viaParam;    
        
}
static void ag_rp_82(PCB_DECL, ParserBasicToken token) {
/* Line 1522, RV SIP Stack */
        if (((ParserExtensionString *)PCB.pExtParams)->size !=0)
        {
            
            APPEND_DATA(";", 1, PCB.pExtParams);
            APPEND_DATA(token.buf, token.len,PCB.pExtParams);
        }
        else
        {      
            
            APPEND_DATA(token.buf, token.len,PCB.pExtParams);
        }
    
}
static void ag_rp_83(PCB_DECL, ParserBasicToken token, ParserBasicToken token2) {
/* Line 1537, RV SIP Stack */
        if (0!=((ParserExtensionString *)PCB.pExtParams)->size)
        {
            
            APPEND_DATA(";", 1,PCB.pExtParams);
            APPEND_DATA(token.buf, token.len,PCB.pExtParams);
            APPEND_DATA("=", 1,PCB.pExtParams);
            APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);
        }
        else
        {      
            
            APPEND_DATA(token.buf, token.len,PCB.pExtParams);
            APPEND_DATA("=", 1,PCB.pExtParams);
            APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);
        }
    
}
static void ag_rp_84(PCB_DECL, ParserBasicToken token, ParserQuotedString token2) {
/* Line 1556, RV SIP Stack */
        if (((ParserExtensionString *)PCB.pExtParams)->size !=0)
        {
            
            APPEND_DATA(";", 1,PCB.pExtParams);
            APPEND_DATA(token.buf, token.len,PCB.pExtParams);
            APPEND_DATA("=", 1,PCB.pExtParams);
            APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);
        }
        else
        {      
            
            APPEND_DATA(token.buf, token.len,PCB.pExtParams);
            APPEND_DATA("=", 1,PCB.pExtParams);
            APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);
        }
    
}
static void ag_rp_85(PCB_DECL, ParserBasicToken token, ParserBasicToken token2) {
/* Line 1579, RV SIP Stack */
        if (((ParserExtensionString *)PCB.pExtParams)->size !=0)
        {
            
            APPEND_DATA(";", 1,PCB.pExtParams);
            APPEND_DATA(token.buf, token.len,PCB.pExtParams);
            APPEND_DATA("=", 1,PCB.pExtParams);
            APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);
        }
        else
        {      
            
            APPEND_DATA(token.buf, token.len,PCB.pExtParams);
            APPEND_DATA("=", 1,PCB.pExtParams);
            APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);
        }
    
}
#define ag_rp_86(PCB_POINTER, t) (t)
#define ag_rp_87(PCB_POINTER, ttl) (ttl)
#define ag_rp_88(PCB_POINTER) (CUR_STRING())
#define ag_rp_89(PCB_POINTER) (CUR_STRING())
#define ag_rp_90(PCB_POINTER) (CUR_STRING())
#define ag_rp_91(PCB_POINTER, rport) (rport)
static ParserPort ag_rp_92(PCB_DECL) {
/* Line 1634, RV SIP Stack */
        ParserPort port;
        port.len = 0;
		port.buf = NULL;
        RV_UNUSED_ARG(pcb_pointer);
        return port;
    
}
#define ag_rp_93(PCB_POINTER, port) (port)
#define ag_rp_94(PCB_POINTER, ma) (ma)
#define ag_rp_95(PCB_POINTER, host) (host)
#define ag_rp_96(PCB_POINTER, host) (host)
#define ag_rp_97(PCB_POINTER, t) (t)
#define ag_rp_98(PCB_POINTER, comp) (comp)
static ParserCompParam ag_rp_99(PCB_DECL) {
/* Line 1671, RV SIP Stack */
        ParserCompParam comp;
        comp.compType     = RVSIP_COMP_SIGCOMP;
        comp.strCompParam = CUR_STRING();
        return comp;
    
}
static ParserCompParam ag_rp_100(PCB_DECL, ParserBasicToken token) {
/* Line 1678, RV SIP Stack */
        ParserCompParam comp;
        comp.compType     = RVSIP_COMP_OTHER;
        comp.strCompParam = CUR_STRING();
        RV_UNUSED_ARG(token.buf);
        return comp;
    
}
#define ag_rp_101(PCB_POINTER, sigCompId) (sigCompId)
#define ag_rp_102(PCB_POINTER) (CUR_STRING())
static void ag_rp_103(PCB_DECL, ParserAbsoluteUri uri) {
/* Line 1927, RV SIP Stack */
       RvStatus status;
       status = ParserInitAbsUri(PCB.pParserMgr, &PCB, &uri, PCB.eHeaderType,PCB.pSipObject);
       if(RV_ERROR_ILLEGAL_SYNTAX == status)
       {
           PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
           SYNTAX_ERROR;
       }
       else if (status != RV_OK)
       {
            
            PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
            PCB.eStat = status;
       }
    
}
static ParserAbsoluteUri ag_rp_104(PCB_DECL, ParserBasicToken scheme, ParserBasicToken id) {
/* Line 1947, RV SIP Stack */
        ParserAbsoluteUri uri;
        uri.scheme = scheme;
        uri.idefntifier = id;
        RV_UNUSED_ARG(pcb_pointer);
        return uri;
    
}
#define ag_rp_105(PCB_POINTER) (CUR_STRING())
#define ag_rp_106(PCB_POINTER, displayName) (displayName)
#define ag_rp_107(PCB_POINTER) (CUR_STRING())
#define ag_rp_108(PCB_POINTER) (CUR_STRING())
static void ag_rp_109(PCB_DECL) {
/* Line 2567, RV SIP Stack */
        RvStatus status;
        status = ParserInitUrl(PCB.pParserMgr,&PCB,&(PCB.sipUrl), PCB.eHeaderType,PCB.pSipObject);
        if(RV_ERROR_ILLEGAL_SYNTAX == status)
        {
            PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
            SYNTAX_ERROR;
        }
        else if (RV_OK != status)
        {
            
            PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
            PCB.eStat = status;
        }
    
}
#define ag_rp_110(PCB_POINTER, uri) (INIT_PRES_IM_STRUCT(uri))
#define ag_rp_111(PCB_POINTER, uri) (INIT_PRES_IM_STRUCT(uri))
static void ag_rp_112(PCB_DECL) {
/* Line 2614, RV SIP Stack */
        PCB.sipUrl.bIsSecure = RV_FALSE;
        PCB.sipUrl.bOldAddrSpec = RV_FALSE;
        SIP_BEGIN_TOKEN();
        DEFINE_URL_PREFIX();
    
}
static void ag_rp_113(PCB_DECL) {
/* Line 2621, RV SIP Stack */
        PCB.sipUrl.bIsSecure = RV_TRUE;
        PCB.sipUrl.bOldAddrSpec = RV_FALSE;
        SIP_BEGIN_TOKEN();
        DEFINE_URL_PREFIX();
    
}
#define ag_rp_114(PCB_POINTER, uri) (uri)
static ParserAbsoluteUri ag_rp_115(PCB_DECL) {
/* Line 2635, RV SIP Stack */
        ParserAbsoluteUri uri;
        
        uri.scheme.len = 0;
		uri.scheme.buf = NULL;
        uri.idefntifier.len = 0;
		uri.idefntifier.buf = NULL;
        RV_UNUSED_ARG(pcb_pointer)
        return uri;
    
}
#define ag_rp_116(PCB_POINTER) (SET_PARSER_PCB_BACK_TO_BUFF(RV_SIPPARSER_PREFIX_RV_ABS))
static void ag_rp_117(PCB_DECL) {
/* Line 2651, RV SIP Stack */
        SIP_BEGIN_TOKEN();
        DEFINE_PRES_IM_PREFIX(RV_FALSE);
    
}
#define ag_rp_118(PCB_POINTER, uri) (uri)
static ParserAbsoluteUri ag_rp_119(PCB_DECL) {
/* Line 2661, RV SIP Stack */
        ParserAbsoluteUri uri;
        
        uri.scheme.len = 0;
        uri.scheme.buf = NULL;
        uri.idefntifier.len = 0;
		uri.idefntifier.buf = NULL;
        RV_UNUSED_ARG(pcb_pointer)
        return uri;
    
}
static void ag_rp_120(PCB_DECL) {
/* Line 2674, RV SIP Stack */
        SIP_BEGIN_TOKEN();
        DEFINE_PRES_IM_PREFIX(RV_TRUE);
    
}
static void ag_rp_121(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2682, RV SIP Stack */
  
           
         PCB.sipUrl.optionalHeaders = hs;
         
         if (0 != ((ParserExtensionString *)PCB.pUrlExtParams)->size)
         {
              PCB.sipUrl.urlParameters.genericParamList= PCB.pUrlExtParams;
              PCB.sipUrl.urlParameters.isOtherParams   = RV_TRUE;
         }          
  	
}
static void ag_rp_122(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2693, RV SIP Stack */
  
         
       PCB.sipUrl.optionalHeaders = hs;
       
       if (0 != ((ParserExtensionString *)PCB.pUrlExtParams)->size)
       {
            PCB.sipUrl.urlParameters.genericParamList=PCB.pUrlExtParams;
            PCB.sipUrl.urlParameters.isOtherParams   = RV_TRUE;
       }          
	
}
static void ag_rp_123(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2726, RV SIP Stack */
 
        RESET_URI_PARAMS();
        PCB.sipUrl.optionalHeaders.isSpecified = RV_FALSE;
        PCB.sipUrl.optionalHeaders = hs;
        PCB.exUri.uriType = PARSER_ADDR_PARAM_TYPE_SIP_URL;
        PCB.exUri.ExUriInfo.SipUrl = PCB.sipUrl;
      
}
static void ag_rp_124(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2735, RV SIP Stack */
 
        RESET_URI_PARAMS();
        PCB.sipUrl.optionalHeaders.isSpecified = RV_FALSE;
        PCB.sipUrl.optionalHeaders = hs;
        PCB.exUri.uriType = PARSER_ADDR_PARAM_TYPE_SIP_URL;
        PCB.exUri.ExUriInfo.SipUrl = PCB.sipUrl;
      
}
static void ag_rp_125(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2775, RV SIP Stack */
 
        RESET_URI_PARAMS();
        PCB.sipUrl.optionalHeaders.isSpecified = RV_FALSE;
        PCB.sipUrl.optionalHeaders = hs;
        PCB.exUri.uriType = PARSER_ADDR_PARAM_TYPE_PRES_URI;
        PCB.exUri.ExUriInfo.SipUrl = PCB.sipUrl;
      
}
static void ag_rp_126(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2784, RV SIP Stack */
 
        RESET_URI_PARAMS();
        PCB.sipUrl.optionalHeaders.isSpecified = RV_FALSE;
        PCB.sipUrl.optionalHeaders = hs;
        
        PCB.exUri.uriType = PARSER_ADDR_PARAM_TYPE_PRES_URI;
        PCB.exUri.ExUriInfo.SipUrl = PCB.sipUrl;
      
}
static void ag_rp_127(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2796, RV SIP Stack */
 
        RESET_URI_PARAMS();
        PCB.sipUrl.optionalHeaders.isSpecified = RV_FALSE;
        PCB.sipUrl.optionalHeaders = hs;
        PCB.exUri.uriType = PARSER_ADDR_PARAM_TYPE_IM_URI;
        PCB.exUri.ExUriInfo.SipUrl = PCB.sipUrl;
      
}
static void ag_rp_128(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2805, RV SIP Stack */
 
        RESET_URI_PARAMS();
        PCB.sipUrl.optionalHeaders.isSpecified = RV_FALSE;
        PCB.sipUrl.optionalHeaders = hs;
        
        PCB.exUri.uriType = PARSER_ADDR_PARAM_TYPE_IM_URI;
        PCB.exUri.ExUriInfo.SipUrl = PCB.sipUrl;
      
}
static void ag_rp_129(PCB_DECL, ParserDisplayName scheme, ParserBasicToken id) {
/* Line 2819, RV SIP Stack */
          PCB.exUri.ExUriInfo.absUri.scheme = scheme;
          PCB.exUri.ExUriInfo.absUri.idefntifier = id;
          PCB.exUri.uriType = PARSER_ADDR_PARAM_TYPE_ABS_URI;
      
}
static void ag_rp_130(PCB_DECL) {
/* Line 2829, RV SIP Stack */
        PCB.sipUrl.bIsUserInfo = RV_TRUE;
    
}
static void ag_rp_131(PCB_DECL) {
/* Line 2834, RV SIP Stack */
        PCB.sipUrl.bIsUserInfo = RV_FALSE;
    
}
static void ag_rp_132(PCB_DECL) {
/* Line 2841, RV SIP Stack */
        SET_PARSER_PCB_BACK_TO_BUFF(RV_SIPPARSER_PREFIX_USERINFO);
    
}
static void ag_rp_133(PCB_DECL, ParserBasicToken user) {
/* Line 2847, RV SIP Stack */
        PCB.sipUrl.urlUserInfo.userName   = user;
        PCB.sipUrl.urlUserInfo.isPassword = RV_FALSE;
    
}
static void ag_rp_134(PCB_DECL, ParserBasicToken user, ParserBasicToken pass) {
/* Line 2852, RV SIP Stack */
        PCB.sipUrl.urlUserInfo.userName   = user;
        PCB.sipUrl.urlUserInfo.isPassword = RV_TRUE;
        PCB.sipUrl.urlUserInfo.password   = pass;
     
}
#define ag_rp_135(PCB_POINTER, t) (t)
#define ag_rp_136(PCB_POINTER) (CUR_STRING())
#define ag_rp_137(PCB_POINTER) (CUR_STRING())
static void ag_rp_138(PCB_DECL) {
/* Line 2885, RV SIP Stack */
        PCB.sipUrl.bIsUserInfo = RV_FALSE;
    
}
static void ag_rp_139(PCB_DECL) {
/* Line 2891, RV SIP Stack */
        SET_PARSER_PCB_BACK_TO_BUFF(RV_SIPPARSER_PREFIX_HOST);
    
}
static void ag_rp_140(PCB_DECL, ParserBasicToken host) {
/* Line 2899, RV SIP Stack */
        PCB.sipUrl.urlHost.hostName = host;
        PCB.sipUrl.urlHost.isPort   = RV_FALSE;
    
}
static void ag_rp_141(PCB_DECL, ParserBasicToken host, ParserPort port) {
/* Line 2905, RV SIP Stack */
        PCB.sipUrl.urlHost.hostName = host;
        PCB.sipUrl.urlHost.isPort   = RV_TRUE;
        PCB.sipUrl.urlHost.port     = port;
    
}
#define ag_rp_142(PCB_POINTER, t) (t)
#define ag_rp_143(PCB_POINTER, ip6ref) (ip6ref)
#define ag_rp_144(PCB_POINTER) (CUR_STRING())
#define ag_rp_145(PCB_POINTER) (CUR_STRING())
#define ag_rp_146(PCB_POINTER) (CUR_STRING())
#define ag_rp_147(PCB_POINTER) (RESET_URI_PARAMS())
static void ag_rp_148(PCB_DECL) {
/* Line 2959, RV SIP Stack */
	
       PCB.sipUrl.urlParameters.isValid = RV_TRUE;
    
}
static void ag_rp_149(PCB_DECL) {
/* Line 2972, RV SIP Stack */
 
        RESET_URI_PARAMS();  
         								
        switch( PCB.urlParameter.paramType )
        {
            case PARSER_TRANSPORT_PARAM: 
	            PCB.sipUrl.urlParameters.isTransport        = RV_TRUE;
	            PCB.sipUrl.urlParameters.transport          = PCB.urlParameter.param.transport; 
	            break;
            case PARSER_USER_PARAM: 
	            PCB.sipUrl.urlParameters.isUserParam        = RV_TRUE;
	            PCB.sipUrl.urlParameters.userParam          = PCB.urlParameter.param.userParam; 
	            break;		
            case PARSER_TTL_PARAM: 
	            PCB.sipUrl.urlParameters.isTtlParam         = RV_TRUE;
	            PCB.sipUrl.urlParameters.ttlParam           = PCB.urlParameter.param.ttlParam; 
	            break;
            case PARSER_MADDR_PARAM: 
	            PCB.sipUrl.urlParameters.isMaddrParam       = RV_TRUE;
	            PCB.sipUrl.urlParameters.maddrParam         = PCB.urlParameter.param.maddrParam; 
	            break;
            case PARSER_METHOD_PARAM: 
	            PCB.sipUrl.urlParameters.isMethodParam       = RV_TRUE;
	            PCB.sipUrl.urlParameters.methodParam         = PCB.urlParameter.param.methodParam; 
	            break;
            case PARSER_LR_PARAM: 
	            PCB.sipUrl.urlParameters.lrParamType         = PCB.urlParameter.param.lrParamType;
	            break;
            case PARSER_COMP_PARAM:
                PCB.sipUrl.urlParameters.isCompParam         = RV_TRUE;
                PCB.sipUrl.urlParameters.compParam           = PCB.urlParameter.param.compParam;
                break;
            case PARSER_SIGCOMPID_PARAM:
                PCB.sipUrl.urlParameters.isSigCompIdParam    = RV_TRUE;
                PCB.sipUrl.urlParameters.sigCompIdParam      = PCB.urlParameter.param.sigCompIdParam;
                break;
			case PARSER_TOKENIZED_BY_PARAM: 
		        PCB.sipUrl.urlParameters.isTokenizedByParam	= RV_TRUE;
		        PCB.sipUrl.urlParameters.tokenizedByParam	= PCB.urlParameter.param.tokenizedByParam; 
		        break;
			case PARSER_ORIG_PARAM: 
		        PCB.sipUrl.urlParameters.bOrigParam			= PCB.urlParameter.param.bOrigParam;
		        break;
            case PARSER_OTHER_PARAM:    
	            break;
             default:
                 PCB.exit_flag = AG_SEMANTIC_ERROR_CODE;
                 PCB.eStat = RV_ERROR_ILLEGAL_SYNTAX;            
        } 	 														
    
}
static void ag_rp_150(PCB_DECL) {
/* Line 3033, RV SIP Stack */
 
         
        switch( PCB.urlParameter.paramType )
        {
	        case PARSER_TRANSPORT_PARAM: 
		        PCB.sipUrl.urlParameters.isTransport        = RV_TRUE;
		        PCB.sipUrl.urlParameters.transport          = PCB.urlParameter.param.transport; 
		        break;
	        case PARSER_USER_PARAM: 
		        PCB.sipUrl.urlParameters.isUserParam        = RV_TRUE;
		        PCB.sipUrl.urlParameters.userParam          = PCB.urlParameter.param.userParam; 
		        break;	
	        case PARSER_TTL_PARAM: 
		        PCB.sipUrl.urlParameters.isTtlParam         = RV_TRUE;
		        PCB.sipUrl.urlParameters.ttlParam           = PCB.urlParameter.param.ttlParam; 
		        break;
	        case PARSER_MADDR_PARAM: 
		        PCB.sipUrl.urlParameters.isMaddrParam       = RV_TRUE;
		        PCB.sipUrl.urlParameters.maddrParam         = PCB.urlParameter.param.maddrParam; 
		        break;  
            case PARSER_METHOD_PARAM: 
		        PCB.sipUrl.urlParameters.isMethodParam		= RV_TRUE;
		        PCB.sipUrl.urlParameters.methodParam		= PCB.urlParameter.param.methodParam; 
		        break; 
            case PARSER_LR_PARAM: 
		        PCB.sipUrl.urlParameters.lrParamType		= PCB.urlParameter.param.lrParamType;
		        break; 
            case PARSER_COMP_PARAM:
                PCB.sipUrl.urlParameters.isCompParam		= RV_TRUE;
                PCB.sipUrl.urlParameters.compParam			= PCB.urlParameter.param.compParam;
                break;
            case PARSER_SIGCOMPID_PARAM:
                PCB.sipUrl.urlParameters.isSigCompIdParam   = RV_TRUE;
                PCB.sipUrl.urlParameters.sigCompIdParam     = PCB.urlParameter.param.sigCompIdParam;
                break;
			case PARSER_TOKENIZED_BY_PARAM: 
		        PCB.sipUrl.urlParameters.isTokenizedByParam	= RV_TRUE;
		        PCB.sipUrl.urlParameters.tokenizedByParam	= PCB.urlParameter.param.tokenizedByParam; 
		        break;
			case PARSER_ORIG_PARAM: 
		        PCB.sipUrl.urlParameters.bOrigParam			= PCB.urlParameter.param.bOrigParam;
		        break;
            case PARSER_OTHER_PARAM:    
		        PCB.sipUrl.urlParameters.isOtherParams		= RV_TRUE;
		        break; 
            default:
                PCB.exit_flag = AG_SEMANTIC_ERROR_CODE;
                PCB.eStat = RV_ERROR_ILLEGAL_SYNTAX;                  
        }	 
    
}
static void ag_rp_151(PCB_DECL, ParserTransport t) {
/* Line 3097, RV SIP Stack */
 
         PCB.urlParameter.paramType = PARSER_TRANSPORT_PARAM;
         PCB.urlParameter.param.transport = t;
    
}
static void ag_rp_152(PCB_DECL, ParserUserParam u) {
/* Line 3102, RV SIP Stack */
 
	     PCB.urlParameter.paramType = PARSER_USER_PARAM;
		 PCB.urlParameter.param.userParam = u;
	
}
static void ag_rp_153(PCB_DECL, ParserTtlParam t) {
/* Line 3107, RV SIP Stack */
 
	     PCB.urlParameter.paramType = PARSER_TTL_PARAM;
		 PCB.urlParameter.param.ttlParam = t;
	
}
static void ag_rp_154(PCB_DECL, ParserMaddrParam ma) {
/* Line 3112, RV SIP Stack */
 
	     PCB.urlParameter.paramType = PARSER_MADDR_PARAM;
		 PCB.urlParameter.param.maddrParam = ma;
	
}
static void ag_rp_155(PCB_DECL, ParserMethod met) {
/* Line 3118, RV SIP Stack */
	     PCB.urlParameter.paramType = PARSER_METHOD_PARAM;
		 PCB.urlParameter.param.methodParam =met;
    
}
static void ag_rp_156(PCB_DECL, ParserLrParamType lrParam) {
/* Line 3123, RV SIP Stack */
	     PCB.urlParameter.paramType = PARSER_LR_PARAM;
		 PCB.urlParameter.param.lrParamType   = lrParam;
    
}
static void ag_rp_157(PCB_DECL, ParserCompParam comp) {
/* Line 3128, RV SIP Stack */
        PCB.urlParameter.paramType       = PARSER_COMP_PARAM;
        PCB.urlParameter.param.compParam = comp;
    
}
static void ag_rp_158(PCB_DECL, ParserSigCompIdParam sigCompId) {
/* Line 3133, RV SIP Stack */
		PCB.urlParameter.paramType = PARSER_SIGCOMPID_PARAM;
		PCB.urlParameter.param.sigCompIdParam = sigCompId;
	
}
static void ag_rp_159(PCB_DECL, ParserTokenizedByParam tokenizedBy) {
/* Line 3138, RV SIP Stack */
        PCB.urlParameter.paramType       = PARSER_TOKENIZED_BY_PARAM;
        PCB.urlParameter.param.tokenizedByParam = tokenizedBy;
    
}
static void ag_rp_160(PCB_DECL, RvBool orig) {
/* Line 3143, RV SIP Stack */
        PCB.urlParameter.paramType        = PARSER_ORIG_PARAM;
        PCB.urlParameter.param.bOrigParam = orig;
    
}
static void ag_rp_161(PCB_DECL) {
/* Line 3165, RV SIP Stack */
	     PCB.urlParameter.paramType = PARSER_OTHER_PARAM;
    
}
#define ag_rp_162(PCB_POINTER, t) (t)
static ParserUserParam ag_rp_163(PCB_DECL, RvSipUserParam pt) {
/* Line 3180, RV SIP Stack */
 
        ParserUserParam userParam;
	    userParam.paramType = pt;
		userParam.otherUser.buf = NULL;
		userParam.otherUser.len = 0;
        RV_UNUSED_ARG(pcb_pointer);
	    return userParam;
    
}
static ParserUserParam ag_rp_164(PCB_DECL, ParserOtherUser ou) {
/* Line 3189, RV SIP Stack */
 
        ParserUserParam userParam;
        userParam.paramType = RVSIP_USERPARAM_OTHER;
	    userParam.otherUser = ou;
        RV_UNUSED_ARG(pcb_pointer);
	    return userParam;
    
}
#define ag_rp_165(PCB_POINTER) (RVSIP_USERPARAM_PHONE)
#define ag_rp_166(PCB_POINTER) (RVSIP_USERPARAM_IP)
#define ag_rp_167(PCB_POINTER, t) (t)
#define ag_rp_168(PCB_POINTER, ttl) (ttl)
#define ag_rp_169(PCB_POINTER, host) (host)
#define ag_rp_170(PCB_POINTER, met) (met)
#define ag_rp_171(PCB_POINTER) (ParserLrParam1)
#define ag_rp_172(PCB_POINTER) (ParserLrParamTrue)
#define ag_rp_173(PCB_POINTER) (ParserLrParamOn)
#define ag_rp_174(PCB_POINTER) (ParserLrParamEmpty)
#define ag_rp_175(PCB_POINTER, comp) (comp)
#define ag_rp_176(PCB_POINTER, sigCompId) (sigCompId)
#define ag_rp_177(PCB_POINTER) (CUR_STRING())
#define ag_rp_178(PCB_POINTER, hostname) (hostname)
#define ag_rp_179(PCB_POINTER) (RV_TRUE)
static void ag_rp_180(PCB_DECL, ParserBasicToken token) {
/* Line 3325, RV SIP Stack */
          if (0!=((ParserExtensionString *)PCB.pUrlExtParams)->size)
          {
              
              APPEND_DATA(";", 1,PCB.pUrlExtParams);
              APPEND_DATA(token.buf,token.len, PCB.pUrlExtParams);
          }
          else
          {      
              
              APPEND_DATA(token.buf,token.len,PCB.pUrlExtParams);
          }
    
}
static void ag_rp_181(PCB_DECL, ParserBasicToken token1, ParserBasicToken token2) {
/* Line 3339, RV SIP Stack */
          if (0!=((ParserExtensionString *)PCB.pUrlExtParams)->size)
          {
              
              APPEND_DATA(";", 1,PCB.pUrlExtParams);
              APPEND_DATA(token1.buf, token1.len,PCB.pUrlExtParams);
              APPEND_DATA("=" , 1,PCB.pUrlExtParams);
              APPEND_DATA(token2.buf, token2.len,PCB.pUrlExtParams);            
          }
          else
          {      
              
              APPEND_DATA(token1.buf, token1.len,PCB.pUrlExtParams);
              APPEND_DATA("=" , 1,PCB.pUrlExtParams);
              APPEND_DATA(token2.buf, token2.len,PCB.pUrlExtParams);            
          }
    
}
#define ag_rp_182(PCB_POINTER) (CUR_STRING())
#define ag_rp_183(PCB_POINTER, token) (token)
static ParserOptionalHeaders ag_rp_184(PCB_DECL) {
/* Line 3382, RV SIP Stack */
 
        ParserOptionalHeaders optionalHeaders;
        optionalHeaders.isSpecified = RV_FALSE;
		optionalHeaders.body.buf = NULL;
		optionalHeaders.body.len = 0;
        RV_UNUSED_ARG(pcb_pointer);
        return optionalHeaders;
    
}
static ParserOptionalHeaders ag_rp_185(PCB_DECL, ParserBasicToken headersList) {
/* Line 3392, RV SIP Stack */
 
        ParserOptionalHeaders optionalHeaders;
        optionalHeaders.isSpecified = RV_TRUE;
        optionalHeaders.body=headersList;
        RV_UNUSED_ARG(pcb_pointer);
        return optionalHeaders;
    
}
#define ag_rp_186(PCB_POINTER) (CUR_STRING())
#define ag_rp_187(PCB_POINTER) (CUR_STRING())
#define ag_rp_188(PCB_POINTER) (CUR_STRING())
#define ag_rp_189(PCB_POINTER) (CUR_STRING())
#define ag_rp_190(PCB_POINTER) (CUR_STRING())
static void ag_rp_191(PCB_DECL) {
/* Line 3428, RV SIP Stack */
        RvStatus status;
        status = ParserInitRequsetLine(PCB.pParserMgr,&PCB,PCB.pointer,&(PCB.requestLine), PCB.pSipObject);
        if(RV_ERROR_ILLEGAL_SYNTAX == status)
        {
            PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
            SYNTAX_ERROR;
        }
        else if (RV_OK != status)
        {
            
            PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
            PCB.eStat = status;
        }
    
}
static void ag_rp_192(PCB_DECL, ParserMethod met, ParserSipVersion ver) {
/* Line 3447, RV SIP Stack */
         PCB.requestLine.method = met;
         PCB.requestLine.sipVer = ver;
		 PCB.requestLine.exUri = PCB.exUri;         
    
}
#define ag_rp_193(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("m"))
#define ag_rp_194(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("i"))
#define ag_rp_195(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("c"))
#define ag_rp_196(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("v"))
#define ag_rp_197(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("t"))
#define ag_rp_198(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("f"))
#define ag_rp_199(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("r"))
#define ag_rp_200(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("b"))
#define ag_rp_201(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("k"))
#define ag_rp_202(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("x"))
#define ag_rp_203(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("o"))
#define ag_rp_204(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("u"))
#define ag_rp_205(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("l"))
#define ag_rp_206(PCB_POINTER, met) (met)
#define ag_rp_207(PCB_POINTER) (SET_URI_TYPE(PARSER_ADDR_PARAM_TYPE_SIP_URL, PCB.exUri.ExUriInfo.absUri))
#define ag_rp_208(PCB_POINTER, uri) (SET_URI_TYPE(PARSER_ADDR_PARAM_TYPE_PRES_URI, uri))
#define ag_rp_209(PCB_POINTER, uri) (SET_URI_TYPE(PARSER_ADDR_PARAM_TYPE_IM_URI, uri))
#define ag_rp_210(PCB_POINTER, uri) (SET_URI_TYPE(PARSER_ADDR_PARAM_TYPE_ABS_URI, uri))
static void ag_rp_211(PCB_DECL) {
/* Line 3505, RV SIP Stack */
 
         RvStatus status;
         status = ParserInitParty(PCB.pParserMgr,PCB.pointer,&(PCB.party), PCB.eHeaderType, PCB.eWhichHeader, PCB.pSipObject);
         if(RV_ERROR_ILLEGAL_SYNTAX == status)
         {
             PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
             SYNTAX_ERROR;
         }
         else if (RV_OK != status)
         {
             
             PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
             PCB.eStat = status;
         }        
	 
}
static void ag_rp_212(PCB_DECL) {
/* Line 3524, RV SIP Stack */
 
	  PCB.party.nameAddr = PCB.nameAddr;
	  PCB.party.partyParams.isTag = RV_FALSE;
	  PCB.party.partyParams.isAddrParams = RV_FALSE;
      PCB.party.isCompact = PCB.isCompactForm;
	
}
static void ag_rp_213(PCB_DECL, ParserPartyParams partyParams) {
/* Line 3533, RV SIP Stack */
 
	  PCB.party.nameAddr = PCB.nameAddr;
	  PCB.party.partyParams = partyParams; 
      PCB.party.isCompact = PCB.isCompactForm;
	
}
static void ag_rp_214(PCB_DECL) {
/* Line 3542, RV SIP Stack */
 
	  PCB.party.nameAddr.isDisplayName = RV_FALSE;
      PCB.party.partyParams.isTag = RV_FALSE;
	  PCB.party.partyParams.isAddrParams = RV_FALSE;
	  PCB.party.nameAddr.exUri = PCB.exUri;
      PCB.party.isCompact = PCB.isCompactForm;
	
}
static void ag_rp_215(PCB_DECL, ParserPartyParams partyParams) {
/* Line 3552, RV SIP Stack */
 
        PCB.party.nameAddr.isDisplayName = RV_FALSE;
        PCB.party.partyParams = partyParams;
		PCB.party.nameAddr.exUri = PCB.exUri;       
        PCB.party.isCompact = PCB.isCompactForm;
      
	
}
static void ag_rp_216(PCB_DECL, ParserDisplayName name) {
/* Line 3564, RV SIP Stack */
 
		PCB.nameAddr.exUri = PCB.exUri;    
        PCB.nameAddr.isDisplayName = RV_TRUE;  
	    PCB.nameAddr.name = name;
	
}
static void ag_rp_217(PCB_DECL) {
/* Line 3571, RV SIP Stack */
 
		PCB.nameAddr.exUri = PCB.exUri;
        PCB.nameAddr.isDisplayName = RV_FALSE;  
    
}
#define ag_rp_218(PCB_POINTER, qs) (qs)
#define ag_rp_219(PCB_POINTER, mtot) (mtot)
#define ag_rp_220(PCB_POINTER) (CUR_STRING())
#define ag_rp_221(PCB_POINTER) (CUR_STRING())
#define ag_rp_222(PCB_POINTER, t) (t)
#define ag_rp_223(PCB_POINTER, ppd) (ppd)
static ParserPartyParams ag_rp_224(PCB_DECL, ParserPartyParams ppl, ParserPartyParams ppd) {
/* Line 3602, RV SIP Stack */
        if (RV_TRUE == ppd.isTag)
        {
            ppl.isTag = RV_TRUE;
            ppl.tag = ppd.tag;
        }
        if (RV_TRUE == ppd.isAddrParams) 
        {
            ppl.isAddrParams = RV_TRUE;
            ppl.addrParams = ppd.addrParams;
        }
        RV_UNUSED_ARG(pcb_pointer);
        return ppl;
    
}
static ParserPartyParams ag_rp_225(PCB_DECL, ParserTagParam tag) {
/* Line 3621, RV SIP Stack */
        ParserPartyParams partyParams;
        partyParams.isTag = RV_TRUE;
        partyParams.tag = tag;
        partyParams.isAddrParams = RV_FALSE;
        partyParams.addrParams = NULL;
        RV_UNUSED_ARG(pcb_pointer);
        return partyParams; 
    
}
static ParserPartyParams ag_rp_226(PCB_DECL) {
/* Line 3631, RV SIP Stack */
        ParserPartyParams partyParams;
        partyParams.isAddrParams = RV_TRUE;
        partyParams.isTag = RV_FALSE;
        partyParams.addrParams = PCB.pExtParams;
		partyParams.tag.buf = NULL;
		partyParams.tag.len = 0;
        return partyParams; 
    
}
static void ag_rp_227(PCB_DECL) {
/* Line 3657, RV SIP Stack */
         RvStatus status;
         ParserContactHeaderValues contactHeaderValues;
         contactHeaderValues.isStar = RV_TRUE; 
         contactHeaderValues.isCompact = PCB.isCompactForm;
         status = ParserInitContactHeader(PCB.pParserMgr, &PCB, PCB.pointer,
                                          &contactHeaderValues,
                                          PCB.eHeaderType,
                                          PCB.pSipObject);
         if(RV_ERROR_ILLEGAL_SYNTAX == status)
         {
             PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
             SYNTAX_ERROR;
         }
         else if (RV_OK != status)
         {
             
             PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
             PCB.eStat = status;
         }
    
}
static void ag_rp_228(PCB_DECL) {
/* Line 3690, RV SIP Stack */
         RvStatus status;
         ParserContactHeaderValues  contactHeaderValues;
         contactHeaderValues.isStar        = RV_FALSE;
         contactHeaderValues.header        = PCB.contact;
         contactHeaderValues.isCompact     = PCB.isCompactForm;
         status = ParserInitContactHeader(PCB.pParserMgr,&PCB,PCB.pointer,
                                          &contactHeaderValues,
                                          PCB.eHeaderType,
                                          PCB.pSipObject);
         if(RV_ERROR_ILLEGAL_SYNTAX == status)
         {
             PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
             SYNTAX_ERROR;
         }
         else if (RV_OK != status)
         {
             
             PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
             PCB.eStat = status;
         }
         
    
}
static void ag_rp_229(PCB_DECL) {
/* Line 3715, RV SIP Stack */
         RvStatus                 status;
         ParserContactHeaderValues contactHeaderValues;
         contactHeaderValues.isStar = RV_FALSE;
         contactHeaderValues.header = PCB.contact;
         contactHeaderValues.isCompact = PCB.isCompactForm;
         status = ParserInitContactHeader(PCB.pParserMgr,&PCB, PCB.pointer,
                                          &contactHeaderValues,
                                          PCB.eHeaderType,
                                          PCB.pSipObject);
         if(RV_ERROR_ILLEGAL_SYNTAX == status)
         {
             PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
             SYNTAX_ERROR;
         }
         else if (RV_OK != status)
         {
             
             PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
             PCB.eStat = status;
         }
    
}
static void ag_rp_230(PCB_DECL) {
/* Line 3742, RV SIP Stack */
 
	  PCB.contact.nameAddr = PCB.nameAddr;
      PCB.contact.isParams = RV_FALSE;
	
}
static void ag_rp_231(PCB_DECL) {
/* Line 3748, RV SIP Stack */
 
	  PCB.contact.nameAddr = PCB.nameAddr;
      PCB.contact.isParams = RV_TRUE;
      
	
}
static void ag_rp_232(PCB_DECL) {
/* Line 3758, RV SIP Stack */
 
	  PCB.contact.nameAddr.isDisplayName = RV_FALSE;  
      PCB.contact.isParams = RV_FALSE;
	  PCB.contact.nameAddr.exUri = PCB.exUri;      
	
}
static void ag_rp_233(PCB_DECL) {
/* Line 3765, RV SIP Stack */
 
      PCB.contact.nameAddr.isDisplayName = RV_FALSE;
	  PCB.contact.isParams = RV_TRUE;      
      PCB.contact.nameAddr.exUri = PCB.exUri;
	
}
static void ag_rp_234(PCB_DECL, ParserQVal q) {
/* Line 3785, RV SIP Stack */
 
        PCB.contact.params.isQ = RV_TRUE;
        PCB.contact.params.q = q;
    
}
static void ag_rp_235(PCB_DECL, ParserContactActionType action) {
/* Line 3791, RV SIP Stack */
 
        PCB.contact.params.isAction = RV_TRUE;
        PCB.contact.params.action = action;
    
}
static void ag_rp_236(PCB_DECL) {
/* Line 3797, RV SIP Stack */
 
        PCB.contact.params.isExpires = RV_TRUE;
        PCB.contact.params.expires = PCB.expires;
    
}
static void ag_rp_237(PCB_DECL) {
/* Line 3971, RV SIP Stack */
  
        PCB.contact.params.isExtention = RV_TRUE;
		PCB.contact.params.exten = PCB.pExtParams;
    
}
#define ag_rp_238(PCB_POINTER, qVal) (qVal)
#define ag_rp_239(PCB_POINTER) (CUR_STRING())
#define ag_rp_240(PCB_POINTER) (CUR_STRING())
#define ag_rp_241(PCB_POINTER, actionVal) (actionVal)
#define ag_rp_242(PCB_POINTER) (PARSER_CONTACT_ACTION_PROXY)
#define ag_rp_243(PCB_POINTER) (PARSER_CONTACT_ACTION_REDIRECT)
static void ag_rp_244(PCB_DECL) {
/* Line 4144, RV SIP Stack */
       RvStatus status;
       status = ParserInitExpiresHeader(PCB.pParserMgr,PCB.pointer,&(PCB.expires),PCB.eHeaderType,PCB.pSipObject);
       if(RV_ERROR_ILLEGAL_SYNTAX == status)
       {
           PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
           SYNTAX_ERROR;
       }
       else if (RV_OK != status)
       {
           
           PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
           PCB.eStat = status;
       }
    
}
static void ag_rp_245(PCB_DECL, ParserDeltaSeconds delta) {
/* Line 4165, RV SIP Stack */
  
		PCB.expires.eFormat = RVSIP_EXPIRES_FORMAT_DELTA_SECONDS;
		PCB.expires.deltaSeconds = delta;
    
}
static void ag_rp_246(PCB_DECL, ParserSipDate date) {
/* Line 4171, RV SIP Stack */
   
	    PCB.expires.eFormat = RVSIP_EXPIRES_FORMAT_DATE;
	    PCB.expires.sipDate = date;
	
}
static void ag_rp_247(PCB_DECL, ParserDeltaSeconds delta) {
/* Line 4180, RV SIP Stack */
   
		PCB.expires.eFormat = RVSIP_EXPIRES_FORMAT_DELTA_SECONDS;
		PCB.expires.deltaSeconds = delta;
    
}
static void ag_rp_248(PCB_DECL, ParserSipDate date) {
/* Line 4186, RV SIP Stack */
   
	    PCB.expires.eFormat = RVSIP_EXPIRES_FORMAT_DATE;
	    PCB.expires.sipDate = date;
	
}
#define ag_rp_249(PCB_POINTER) (CUR_STRING())
static void ag_rp_250(PCB_DECL, ParserSipDate date) {
/* Line 4211, RV SIP Stack */
   
          RvStatus status;
          status = ParserInitDateHeader(PCB.pParserMgr,PCB.pointer,&date,PCB.eHeaderType,PCB.pSipObject);
          if(RV_ERROR_ILLEGAL_SYNTAX == status)
          {
              PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
              SYNTAX_ERROR;
          }
          else if (RV_OK != status)
          {
                
              PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
              PCB.eStat = status;
          }
    
}
static ParserSipDate ag_rp_251(PCB_DECL, ParserDDMMYY date, ParserTime t) {
/* Line 4233, RV SIP Stack */
  
        ParserSipDate sipDate;
        sipDate.eWeekDay = RVSIP_WEEKDAY_UNDEFINED;
		sipDate.ddmmyy   = date;
        sipDate.time     = t;
        RV_UNUSED_ARG(pcb_pointer);
        return sipDate;
    
}
#define ag_rp_252(PCB_POINTER, date) (date)
static ParserSipDate ag_rp_253(PCB_DECL, RvSipDateWeekDay day, ParserDDMMYY date, ParserTime t) {
/* Line 4248, RV SIP Stack */
   
        ParserSipDate sipDate;
        sipDate.eWeekDay = RVSIP_WEEKDAY_UNDEFINED;
	    sipDate.eWeekDay = day; 
		sipDate.ddmmyy   = date;
        sipDate.time     = t;
        RV_UNUSED_ARG(pcb_pointer);
        return sipDate;
    
}
#define ag_rp_254(PCB_POINTER) (RVSIP_WEEKDAY_SUN)
#define ag_rp_255(PCB_POINTER) (RVSIP_WEEKDAY_MON)
#define ag_rp_256(PCB_POINTER) (RVSIP_WEEKDAY_TUE)
#define ag_rp_257(PCB_POINTER) (RVSIP_WEEKDAY_WED)
#define ag_rp_258(PCB_POINTER) (RVSIP_WEEKDAY_THU)
#define ag_rp_259(PCB_POINTER) (RVSIP_WEEKDAY_FRI)
#define ag_rp_260(PCB_POINTER) (RVSIP_WEEKDAY_SAT)
static ParserDDMMYY ag_rp_261(PCB_DECL, ParserInt32 dd, RvSipDateMonth mm, ParserInt32 yy) {
/* Line 4273, RV SIP Stack */
        ParserDDMMYY ddmmyy;
        ddmmyy.day = dd;
        ddmmyy.eMonth = mm;
        ddmmyy.year = yy;
        RV_UNUSED_ARG(pcb_pointer);
        return ddmmyy;
    
}
static ParserInt32 ag_rp_262(PCB_DECL, int d) {
/* Line 4285, RV SIP Stack */
    	RV_UNUSED_ARG(pcb_pointer);
    	return d - '0';	
    
}
static ParserInt32 ag_rp_263(PCB_DECL, int d1, int d2) {
/* Line 4290, RV SIP Stack */
		RV_UNUSED_ARG(pcb_pointer);
	    return 10 * (d1-'0') + (d2-'0');
    
}
#define ag_rp_264(PCB_POINTER) (RVSIP_MONTH_JAN)
#define ag_rp_265(PCB_POINTER) (RVSIP_MONTH_FEB)
#define ag_rp_266(PCB_POINTER) (RVSIP_MONTH_MAR)
#define ag_rp_267(PCB_POINTER) (RVSIP_MONTH_APR)
#define ag_rp_268(PCB_POINTER) (RVSIP_MONTH_MAY)
#define ag_rp_269(PCB_POINTER) (RVSIP_MONTH_JUN)
#define ag_rp_270(PCB_POINTER) (RVSIP_MONTH_JUL)
#define ag_rp_271(PCB_POINTER) (RVSIP_MONTH_AUG)
#define ag_rp_272(PCB_POINTER) (RVSIP_MONTH_SEP)
#define ag_rp_273(PCB_POINTER) (RVSIP_MONTH_OCT)
#define ag_rp_274(PCB_POINTER) (RVSIP_MONTH_NOV)
#define ag_rp_275(PCB_POINTER) (RVSIP_MONTH_DEC)
#define ag_rp_276(PCB_POINTER, d1, d2) (2000 + 10 * (d1-'0') + (d2-'0'))
#define ag_rp_277(PCB_POINTER, d1, d2, d3) (2000 + 100 * (d1-'0') + 10 * (d2-'0') + (d3-'0'))
#define ag_rp_278(PCB_POINTER, d1, d2, d3, d4) (1000 * (d1-'0') + 100*(d2-'0') + 10*(d3-'0') + (d4-'0'))
static ParserTime ag_rp_279(PCB_DECL, ParserInt32 hour, ParserInt32 min, ParserInt32 second, ParserTimeZone zone) {
/* Line 4320, RV SIP Stack */
        ParserTime t;
        t.hour   = hour;
        t.minute = min;
        t.second = second;
        t.zone   = zone;
        RV_UNUSED_ARG(pcb_pointer);
        return t;        
    
}
#define ag_rp_280(PCB_POINTER, d1, d2) (10*(d1-'0') + (d2-'0'))
#define ag_rp_281(PCB_POINTER, d1, d2) (10*(d1-'0') + (d2-'0'))
#define ag_rp_282(PCB_POINTER, d1, d2) (10*(d1-'0') + (d2-'0'))
#define ag_rp_283(PCB_POINTER) (SIPPARSER_TIME_ZONE_GMT)
#define ag_rp_284(PCB_POINTER) (SIPPARSER_TIME_ZONE_UTC)
#define ag_rp_285(PCB_POINTER) (SIPPARSER_TIME_ZONE_GMST)
static void ag_rp_286(PCB_DECL, ParserMethod method) {
/* Line 4367, RV SIP Stack */
        RvStatus         status;
        ParserOtherHeader otherHeader;
        otherHeader.name = method.other;
        otherHeader.value.buf = NULL;
        otherHeader.value.len = 0;
        ParserMethodInOtherHeader(&method, &otherHeader);
        status = ParserInitOtherHeader(PCB.pParserMgr,&PCB,PCB.pointer,
                                       &otherHeader,
                                       PCB.eHeaderType,
                                       PCB.pSipObject);
        if(RV_ERROR_ILLEGAL_SYNTAX == status)
        {
             PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
             SYNTAX_ERROR;
        }
        else if (RV_OK != status)
        {
             
             PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
             PCB.eStat = status;
        }
        
    
}
static void ag_rp_287(PCB_DECL, ParserMethod method, ParserBasicToken fv) {
/* Line 4392, RV SIP Stack */
        RvStatus         status;
        ParserOtherHeader otherHeader;
        otherHeader.name = method.other;
        otherHeader.value = fv;
        ParserMethodInOtherHeader(&method, &otherHeader);
        status = ParserInitOtherHeader(PCB.pParserMgr,&PCB,PCB.pointer,
                                       &otherHeader,
                                       PCB.eHeaderType,
                                       PCB.pSipObject);
        if(RV_ERROR_ILLEGAL_SYNTAX == status)
        {
            PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
            SYNTAX_ERROR;
        }
        else if (RV_OK != status)
        {
            
            PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
            PCB.eStat = status;
        }
    
}
static void ag_rp_288(PCB_DECL) {
/* Line 4422, RV SIP Stack */
          RvStatus status; 
          status = ParserInitAuthentication(PCB.pParserMgr,&PCB,PCB.pointer,&(PCB.authentication),PCB.eHeaderType,PCB.eWhichHeader,PCB.pSipObject);
          if(RV_ERROR_ILLEGAL_SYNTAX == status)
          {
              PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
              SYNTAX_ERROR;
          }
          else if (RV_OK != status)
          {
               
               PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
               PCB.eStat = status;
          }
    
}
static void ag_rp_289(PCB_DECL) {
/* Line 4445, RV SIP Stack */
    
        PCB.authentication.eAuthScheme = RVSIP_AUTH_SCHEME_DIGEST;
        PCB.authentication.isAuthParamList = RV_FALSE;
    
}
static void ag_rp_290(PCB_DECL, ParserBasicToken authScheme) {
/* Line 4451, RV SIP Stack */
        
        PCB.authentication.eAuthScheme = RVSIP_AUTH_SCHEME_OTHER;
        PCB.authentication.authScheme  = authScheme;
        PCB.authentication.isRealm =     RV_FALSE;
        PCB.authentication.isDomain =    RV_FALSE;
        PCB.authentication.isNonce =     RV_FALSE;
        PCB.authentication.isOpaque =    RV_FALSE;
        PCB.authentication.isAuthParam = RV_FALSE;
        PCB.authentication.eStale =      RVSIP_AUTH_STALE_UNDEFINED;
        PCB.authentication.algorithm.eAlgorithm = RVSIP_AUTH_ALGORITHM_UNDEFINED;
		PCB.authentication.algorithm.isAKAVersion = RV_FALSE;
		PCB.authentication.qopValue.eAuthQop    = RVSIP_AUTH_QOP_UNDEFINED;
        PCB.authentication.qopValue.isOther = RV_FALSE;
        PCB.authentication.isAuthParamList = RV_TRUE;
		PCB.authentication.isIntegrityKey =	RV_FALSE;
		PCB.authentication.isCipherKey =	RV_FALSE;
        PCB.authentication.authParamList = PCB.pExtParams;
    
}
static void ag_rp_291(PCB_DECL) {
/* Line 4478, RV SIP Stack */
        PCB.authentication.eAuthScheme = RVSIP_AUTH_SCHEME_UNDEFINED;
        PCB.authentication.isRealm =     RV_FALSE;
        PCB.authentication.isDomain =    RV_FALSE;
        PCB.authentication.isNonce =     RV_FALSE;
        PCB.authentication.isOpaque =    RV_FALSE;
        PCB.authentication.isAuthParam = RV_FALSE;
        PCB.authentication.eStale =      RVSIP_AUTH_STALE_UNDEFINED;
        PCB.authentication.algorithm.eAlgorithm = RVSIP_AUTH_ALGORITHM_UNDEFINED;
		PCB.authentication.algorithm.isAKAVersion = RV_FALSE;
        PCB.authentication.qopValue.eAuthQop    = RVSIP_AUTH_QOP_UNDEFINED;
        PCB.authentication.qopValue.isOther = RV_FALSE;
		PCB.authentication.isIntegrityKey =	RV_FALSE;
		PCB.authentication.isCipherKey =	RV_FALSE;
        switch (PCB.digestChallenge.eType)
        {
            case PARSER_DIGEST_TYPE_REALM:
            {
                PCB.authentication.isRealm = RV_TRUE;
                PCB.authentication.realm = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_DOMAIN:
            {
                PCB.authentication.isDomain = RV_TRUE;
                PCB.authentication.domain = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_NONCE:
            {
                PCB.authentication.isNonce = RV_TRUE;
                PCB.authentication.nonce = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_OPAQUE:
            {
                PCB.authentication.isOpaque = RV_TRUE;
                PCB.authentication.opaque = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_STALE:
            {
                PCB.authentication.eStale = PCB.digestChallenge.eStale;
                break;
            }
            case PARSER_DIGEST_TYPE_ALGORITHM:
            {
                PCB.authentication.algorithm = PCB.digestChallenge.algorithm;
                break;
            }
			case PARSER_DIGEST_TYPE_INTEGRITY_KEY:
            {
                PCB.authentication.integrityKey = PCB.digestChallenge.challenge;
                break;
            }
			case PARSER_DIGEST_TYPE_CIPHER_KEY:
            {
                PCB.authentication.cipherKey = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_OPQ_OPTIONS:
            {
                PCB.authentication.qopValue = PCB.digestChallenge.qopVal;
                break;
            }
            case PARSER_DIGEST_TYPE_AUTH_PARAM:
            {
                PCB.authentication.isAuthParam = RV_TRUE;
                PCB.authentication.authParam = PCB.digestChallenge.challenge;
                break;
            }
            default:
                    PCB.exit_flag = AG_SEMANTIC_ERROR_CODE;
                    PCB.eStat = RV_ERROR_ILLEGAL_SYNTAX;
        }
    
}
static void ag_rp_292(PCB_DECL) {
/* Line 4556, RV SIP Stack */
        if (PCB.isNewChallenge == RV_FALSE)
        {
            switch (PCB.digestChallenge.eType)
            {
                case PARSER_DIGEST_TYPE_REALM:
                {
                    PCB.authentication.isRealm = RV_TRUE;
                    PCB.authentication.realm = PCB.digestChallenge.challenge;
                    break;
                }
                case PARSER_DIGEST_TYPE_DOMAIN:
                {
                    PCB.authentication.isDomain = RV_TRUE;
                    PCB.authentication.domain = PCB.digestChallenge.challenge;
                    break;
                }
                case PARSER_DIGEST_TYPE_NONCE:
                {
                    PCB.authentication.isNonce = RV_TRUE;
                    PCB.authentication.nonce = PCB.digestChallenge.challenge;
                    break;
                }
                case PARSER_DIGEST_TYPE_OPAQUE:
                {
                    PCB.authentication.isOpaque = RV_TRUE;
                    PCB.authentication.opaque = PCB.digestChallenge.challenge;
                    break;
                }
                case PARSER_DIGEST_TYPE_STALE:
                {
                    PCB.authentication.eStale = PCB.digestChallenge.eStale;
                    break;
                }
                case PARSER_DIGEST_TYPE_ALGORITHM:
                {
                    PCB.authentication.algorithm = PCB.digestChallenge.algorithm;
                    break;
                }
				case PARSER_DIGEST_TYPE_INTEGRITY_KEY:
				{
					PCB.authentication.isIntegrityKey = RV_TRUE;
					PCB.authentication.integrityKey = PCB.digestChallenge.challenge;
					break;
				}
				case PARSER_DIGEST_TYPE_CIPHER_KEY:
				{
					PCB.authentication.isCipherKey = RV_TRUE;
					PCB.authentication.cipherKey = PCB.digestChallenge.challenge;
					break;
				}
                case PARSER_DIGEST_TYPE_OPQ_OPTIONS:
                {
                    PCB.authentication.qopValue = PCB.digestChallenge.qopVal;
                    break;
                }
                case PARSER_DIGEST_TYPE_AUTH_PARAM:
                {
                    PCB.authentication.isAuthParam = RV_TRUE;
                    PCB.authentication.authParam = PCB.digestChallenge.challenge;
                    break;
                }
                default:
                        PCB.exit_flag = AG_SEMANTIC_ERROR_CODE;
                            
                        PCB.eStat = RV_ERROR_ILLEGAL_SYNTAX;
            }
        }
    
}
#define ag_rp_293(PCB_POINTER, realm) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_REALM, realm))
#define ag_rp_294(PCB_POINTER, domain) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_DOMAIN, domain))
#define ag_rp_295(PCB_POINTER, nonce) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_NONCE, nonce))
#define ag_rp_296(PCB_POINTER, opaque) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_OPAQUE, opaque))
#define ag_rp_297(PCB_POINTER, ik) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_INTEGRITY_KEY, ik))
#define ag_rp_298(PCB_POINTER, ck) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_CIPHER_KEY, ck))
#define ag_rp_299(PCB_POINTER, authParam) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_AUTH_PARAM, authParam))
static void ag_rp_300(PCB_DECL, RvSipAuthStale stale) {
/* Line 4637, RV SIP Stack */
        PCB.digestChallenge.eType = PARSER_DIGEST_TYPE_STALE;
        PCB.digestChallenge.eStale = stale;
    
}
static void ag_rp_301(PCB_DECL, ParserAlgorithm algorithm) {
/* Line 4642, RV SIP Stack */
        PCB.digestChallenge.eType = PARSER_DIGEST_TYPE_ALGORITHM;
        
        if(PCB.digestChallenge.algorithm.eAlgorithm == RVSIP_AUTH_ALGORITHM_UNDEFINED)
        {
            PCB.digestChallenge.algorithm = algorithm;
        }
    
}
static void ag_rp_302(PCB_DECL, ParserQopValue qopOptions) {
/* Line 4651, RV SIP Stack */
        PCB.digestChallenge.eType = PARSER_DIGEST_TYPE_OPQ_OPTIONS;
        PCB.digestChallenge.qopVal = qopOptions;
    
}
static void ag_rp_303(PCB_DECL) {
/* Line 4657, RV SIP Stack */
        PCB.digestChallenge.eType = PARSER_DIGEST_TYPE_NEW_CHALLENGE;
        PCB.digestChallenge.challenge.buf = "NewChallenge";
        PCB.isNewChallenge = RV_TRUE;
    
}
#define ag_rp_304(PCB_POINTER, t) (t)
#define ag_rp_305(PCB_POINTER, t) (t)
#define ag_rp_306(PCB_POINTER) (CUR_LINE())
#define ag_rp_307(PCB_POINTER) (CUR_LINE())
static void ag_rp_308(PCB_DECL, ParserBasicToken token) {
/* Line 4693, RV SIP Stack */
        RV_UNUSED_ARG(pcb_pointer);
        RV_UNUSED_ARG(token.buf);
    
}
#define ag_rp_309(PCB_POINTER) (CUR_STRING())
#define ag_rp_310(PCB_POINTER) (SIP_BEGIN_LINE())
#define ag_rp_311(PCB_POINTER) (SIP_BEGIN_LINE())
#define ag_rp_312(PCB_POINTER, t) (t)
#define ag_rp_313(PCB_POINTER, t) (t)
#define ag_rp_314(PCB_POINTER, t) (t)
#define ag_rp_315(PCB_POINTER, t) (t)
#define ag_rp_316(PCB_POINTER) (RVSIP_AUTH_STALE_TRUE)
#define ag_rp_317(PCB_POINTER) (RVSIP_AUTH_STALE_FALSE)
#define ag_rp_318(PCB_POINTER, algo) (algo)
static void ag_rp_319(PCB_DECL) {
/* Line 4761, RV SIP Stack */
		DEFINE_AKAV_PREFIX();
		SIP_BEGIN_TOKEN();			
	
}
static void ag_rp_320(PCB_DECL) {
/* Line 4766, RV SIP Stack */
		DEFINE_AKAV_PREFIX();
		SIP_BEGIN_TOKEN();			
	
}
static void ag_rp_321(PCB_DECL) {
/* Line 4771, RV SIP Stack */
		DEFINE_AKAV_PREFIX();
		SIP_BEGIN_TOKEN();			
	
}
static void ag_rp_322(PCB_DECL) {
/* Line 4776, RV SIP Stack */
		DEFINE_AKAV_PREFIX();
		SIP_BEGIN_TOKEN();			
	
}
static ParserAlgorithm ag_rp_323(PCB_DECL, ParserBasicToken version, ParserAlgorithm algo) {
/* Line 4784, RV SIP Stack */
		algo.isAKAVersion = RV_TRUE;
		algo.nAKAVersion  = version;
		RV_UNUSED_ARG(pcb_pointer);
		return algo;
	
}
static ParserAlgorithm ag_rp_324(PCB_DECL, ParserAlgorithm algo) {
/* Line 4791, RV SIP Stack */
		algo.isAKAVersion = RV_FALSE;
		RV_UNUSED_ARG(pcb_pointer);
		return algo;
	
}
#define ag_rp_325(PCB_POINTER) (SIP_BEGIN_TOKEN())
static ParserAlgorithm ag_rp_326(PCB_DECL) {
/* Line 4803, RV SIP Stack */
        ParserAlgorithm algo;
        algo.eAlgorithm = RVSIP_AUTH_ALGORITHM_MD5;
		algo.algorithm.buf = NULL;
		algo.algorithm.len = 0;
		algo.nAKAVersion.buf = NULL;
		algo.nAKAVersion.len = 0;
		algo.isAKAVersion = RV_FALSE;
        RV_UNUSED_ARG(pcb_pointer);
        return algo;
    
}
static ParserAlgorithm ag_rp_327(PCB_DECL, ParserBasicToken t) {
/* Line 4815, RV SIP Stack */
        ParserAlgorithm algo;
        algo.eAlgorithm = RVSIP_AUTH_ALGORITHM_OTHER;
        algo.algorithm = t;
		algo.nAKAVersion.buf = NULL;
		algo.nAKAVersion.len = 0;
		algo.isAKAVersion = RV_FALSE;
        RV_UNUSED_ARG(pcb_pointer);
        return algo;
    
}
#define ag_rp_328(PCB_POINTER) (SET_PARSER_PCB_BACK_TO_BUFF(RV_SIPPARSER_PREFIX_AKAV))
#define ag_rp_329(PCB_POINTER) (SET_PARSER_PCB_BACK_TO_BUFF(RV_SIPPARSER_PREFIX_ALGORITHM))
#define ag_rp_330(PCB_POINTER) (CUR_STRING())
#define ag_rp_331(PCB_POINTER, qvList) (qvList)
static ParserQopValue ag_rp_332(PCB_DECL, ParserAuthQopOptions val) {
/* Line 4855, RV SIP Stack */
        ParserQopValue qop;
        qop.eAuthQop = RVSIP_AUTH_QOP_OTHER;
        qop.isOther  = RV_FALSE;
		qop.qop = NULL;
		
        if (RV_TRUE == val.isAuth)
        {
            qop.eAuthQop = RVSIP_AUTH_QOP_AUTH_ONLY;    
        }
        else if (RV_TRUE == val.isAuthInt)
        {
            qop.eAuthQop = RVSIP_AUTH_QOP_AUTHINT_ONLY;
        }
        else if (RV_TRUE == val.isOther)
        {
            if (0!=((ParserExtensionString *)PCB.pExtParams)->size)
            {
                APPEND_DATA(",", 1,PCB.pExtParams);
                
                APPEND_DATA(val.strOther.buf, val.strOther.len,PCB.pExtParams);
                qop.isOther = RV_TRUE;
                qop.qop =  PCB.pExtParams;
            }
            else
            {
                
                APPEND_DATA(val.strOther.buf, val.strOther.len,PCB.pExtParams);
                qop.isOther = RV_TRUE;
                qop.qop = PCB.pExtParams;
            }        
        }      
        return qop;        
    
}
static ParserQopValue ag_rp_333(PCB_DECL, ParserQopValue qop, ParserAuthQopOptions val) {
/* Line 4891, RV SIP Stack */
        if (RV_TRUE == val.isAuth && 
            RVSIP_AUTH_QOP_AUTHINT_ONLY == qop.eAuthQop)
        {
            qop.eAuthQop = RVSIP_AUTH_QOP_AUTH_AND_AUTHINT;    
        }
        else if (RV_TRUE == val.isAuth)
        {
            qop.eAuthQop = RVSIP_AUTH_QOP_AUTH_ONLY;    
        }
        else if (RV_TRUE == val.isAuthInt &&
                 RVSIP_AUTH_QOP_AUTH_ONLY == qop.eAuthQop)
        {
            qop.eAuthQop = RVSIP_AUTH_QOP_AUTH_AND_AUTHINT;
        }
        else if (RV_TRUE == val.isAuthInt)
        {
            qop.eAuthQop = RVSIP_AUTH_QOP_AUTHINT_ONLY;
        }
        else if (RV_TRUE == val.isOther)
        {
            if (0!=((ParserExtensionString *)PCB.pExtParams)->size)
            {
                APPEND_DATA(",", 1,PCB.pExtParams);
                
                APPEND_DATA(val.strOther.buf, val.strOther.len,PCB.pExtParams);
                qop.isOther = RV_TRUE;
                qop.qop =  PCB.pExtParams;
            }
            else
            {
                
                APPEND_DATA(val.strOther.buf, val.strOther.len,PCB.pExtParams);
                qop.isOther = RV_TRUE;
                qop.qop = PCB.pExtParams;
            }        
        }
        return qop;
    
}
static ParserAuthQopOptions ag_rp_334(PCB_DECL) {
/* Line 4936, RV SIP Stack */
        ParserAuthQopOptions qopOption;
        qopOption.isAuthInt = RV_TRUE;
        qopOption.isAuth    = RV_FALSE; 
        qopOption.isOther   = RV_FALSE;
		qopOption.strOther.buf = NULL;
		qopOption.strOther.len = 0;
        RV_UNUSED_ARG(pcb_pointer);
        return qopOption;
    
}
static ParserAuthQopOptions ag_rp_335(PCB_DECL) {
/* Line 4947, RV SIP Stack */
        ParserAuthQopOptions qopOption;
        qopOption.isAuth = RV_TRUE;
        qopOption.isAuthInt = RV_FALSE; 
        qopOption.isOther   = RV_FALSE;
		qopOption.strOther.buf = NULL;
		qopOption.strOther.len = 0;
        RV_UNUSED_ARG(pcb_pointer);
        return qopOption;
    
}
static ParserAuthQopOptions ag_rp_336(PCB_DECL, ParserBasicToken t) {
/* Line 4959, RV SIP Stack */
        ParserAuthQopOptions qopOption;
        qopOption.isOther = RV_TRUE;
        qopOption.isAuthInt = RV_FALSE;
        qopOption.isAuth    = RV_FALSE; 
        qopOption.strOther = t;
        RV_UNUSED_ARG(pcb_pointer);
        return qopOption;
    
}
#define ag_rp_337(PCB_POINTER, ik) (ik)
#define ag_rp_338(PCB_POINTER, ck) (ck)
#define ag_rp_339(PCB_POINTER) (CUR_STRING())
#define ag_rp_340(PCB_POINTER) (CUR_STRING())
#define ag_rp_341(PCB_POINTER) (CUR_STRING())
#define ag_rp_342(PCB_POINTER) (CUR_STRING())
#define ag_rp_343(PCB_POINTER) (CUR_STRING())
static void ag_rp_344(PCB_DECL, ParserBasicToken token1, ParserGenericParamDataVal token2) {
/* Line 5001, RV SIP Stack */
      if (0!=((ParserExtensionString *)PCB.pExtParams)->size)
      {
          
          APPEND_DATA(",", 1,PCB.pExtParams);
          APPEND_DATA(token1.buf, token1.len,PCB.pExtParams);
          APPEND_DATA("=" , 1,PCB.pExtParams);
          APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);            
      }
      else
      {      
          
          APPEND_DATA(token1.buf, token1.len,PCB.pExtParams);
          APPEND_DATA("=" , 1,PCB.pExtParams);
          APPEND_DATA(token2.buf, token2.len,PCB.pExtParams);            
      }
   
}
static void ag_rp_345(PCB_DECL) {
/* Line 5028, RV SIP Stack */
   
          RvStatus status; 
          status = ParserInitAuthorization(PCB.pParserMgr,&PCB,PCB.pointer,&(PCB.authorization),PCB.eHeaderType,PCB.eWhichHeader,PCB.pSipObject);
          if(RV_ERROR_ILLEGAL_SYNTAX == status)
          {
              PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
              SYNTAX_ERROR;
          }
          else if (RV_OK != status)
          {
               
               PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
               PCB.eStat = status;
          }
    
}
static void ag_rp_346(PCB_DECL) {
/* Line 5053, RV SIP Stack */
        
        PCB.authorization.eAuthScheme = RVSIP_AUTH_SCHEME_DIGEST;  
        PCB.authorization.isAuthParamList = RV_FALSE;
    
}
static void ag_rp_347(PCB_DECL, ParserBasicToken authScheme) {
/* Line 5059, RV SIP Stack */
        
        PCB.authorization.eAuthScheme			= RVSIP_AUTH_SCHEME_OTHER;
        PCB.authorization.authScheme			= authScheme; 
        PCB.authorization.isUsername			= RV_FALSE;
        PCB.authorization.isRealm				= RV_FALSE;
        PCB.authorization.isNonce				= RV_FALSE;
        PCB.authorization.isResponse			= RV_FALSE;
        PCB.authorization.algorithm.eAlgorithm	= RVSIP_AUTH_ALGORITHM_UNDEFINED;   
        PCB.authorization.isCnonce				= RV_FALSE;
        PCB.authorization.isOpaque				= RV_FALSE;
        PCB.authorization.isNonceCount			= RV_FALSE;
        PCB.authorization.eAuthQop				= RVSIP_AUTH_QOP_UNDEFINED;
        PCB.authorization.isAuthParam			= RV_FALSE;
		PCB.authorization.isAuts				= RV_FALSE;
		PCB.authorization.algorithm.isAKAVersion= RV_FALSE;
		PCB.authorization.eProtected			= RVSIP_AUTH_INTEGRITY_PROTECTED_UNDEFINED;
        PCB.authorization.isAuthParamList = RV_TRUE;
        PCB.authorization.authParamList = PCB.pExtParams;
    
}
static void ag_rp_348(PCB_DECL) {
/* Line 5089, RV SIP Stack */
      
        PCB.authorization.isUsername			= RV_FALSE;
        PCB.authorization.isRealm				= RV_FALSE;
        PCB.authorization.isNonce				= RV_FALSE;
        PCB.authorization.isResponse			= RV_FALSE;
        PCB.authorization.algorithm.eAlgorithm	= RVSIP_AUTH_ALGORITHM_UNDEFINED;   
        PCB.authorization.isCnonce				= RV_FALSE;
        PCB.authorization.isOpaque				= RV_FALSE;
        PCB.authorization.isNonceCount			= RV_FALSE;
        PCB.authorization.eAuthQop				= RVSIP_AUTH_QOP_UNDEFINED;
        PCB.authorization.isAuthParam			= RV_FALSE;
		PCB.authorization.isAuts				= RV_FALSE;
		PCB.authorization.algorithm.isAKAVersion= RV_FALSE;
		PCB.authorization.eProtected			= RVSIP_AUTH_INTEGRITY_PROTECTED_UNDEFINED;
        switch (PCB.digestChallenge.eType)
        {
             case PARSER_DIGEST_TYPE_USERNAME:
            {
                PCB.authorization.isUsername = RV_TRUE;
                PCB.authorization.userName = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_REALM:
            {
                PCB.authorization.isRealm = RV_TRUE;
                PCB.authorization.realm = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_NONCE:
            {
                PCB.authorization.isNonce = RV_TRUE;
                PCB.authorization.nonce = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_DIGEST_URI:
            {
                PCB.authorization.isUri = RV_TRUE;
                PCB.authorization.hDigestUri = PCB.digestChallenge.uri;
                break;
            }
            case PARSER_DIGEST_TYPE_DIGEST_RESPONSE:
            {
                PCB.authorization.isResponse = RV_TRUE;
                PCB.authorization.response = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_ALGORITHM:
            {
                PCB.authorization.algorithm = PCB.digestChallenge.algorithm;
                break;
            }
            case PARSER_DIGEST_TYPE_CNONCE:
            {
                PCB.authorization.isCnonce = RV_TRUE;
                PCB.authorization.cnonce = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_OPAQUE:
            {
                PCB.authorization.isOpaque = RV_TRUE;
                PCB.authorization.opaque = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_NONCE_COUNT:
            {
                PCB.authorization.isNonceCount = RV_TRUE;
                PCB.authorization.nonceCount = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_MESSAGE_QOP:
            {
                PCB.authorization.eAuthQop = PCB.digestChallenge.qopVal.eAuthQop;
                break;
            }
            case PARSER_DIGEST_TYPE_AUTH_PARAM:
            {
                PCB.authorization.isAuthParam = RV_TRUE;
                PCB.authorization.authParam = PCB.digestChallenge.challenge;
                break;
            }
			case PARSER_DIGEST_TYPE_AUTS:
            {
                PCB.authorization.isAuts = RV_TRUE;
                PCB.authorization.auts = PCB.digestChallenge.challenge;
                break;
            }
			case PARSER_DIGEST_TYPE_INTEGRITY_PROTECTED:
            {
                PCB.authorization.eProtected = PCB.digestChallenge.eProtected;
                break;
            }
            default:
				PCB.exit_flag = AG_SEMANTIC_ERROR_CODE;
				PCB.eStat = RV_ERROR_ILLEGAL_SYNTAX;
        }
    
}
static void ag_rp_349(PCB_DECL) {
/* Line 5188, RV SIP Stack */
        switch (PCB.digestChallenge.eType)
        {
             case PARSER_DIGEST_TYPE_USERNAME:
            {
                PCB.authorization.isUsername = RV_TRUE;
                PCB.authorization.userName = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_REALM:
            {
                PCB.authorization.isRealm = RV_TRUE;
                PCB.authorization.realm = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_NONCE:
            {
                PCB.authorization.isNonce = RV_TRUE;
                PCB.authorization.nonce = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_DIGEST_URI:
            {
                PCB.authorization.isUri = RV_TRUE;
                PCB.authorization.hDigestUri = PCB.digestChallenge.uri;
                break;
            }
            case PARSER_DIGEST_TYPE_DIGEST_RESPONSE:
            {
                PCB.authorization.isResponse = RV_TRUE;
                PCB.authorization.response = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_ALGORITHM:
            {
                PCB.authorization.algorithm = PCB.digestChallenge.algorithm;
                break;
            }
            case PARSER_DIGEST_TYPE_CNONCE:
            {
                PCB.authorization.isCnonce = RV_TRUE;
                PCB.authorization.cnonce = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_OPAQUE:
            {
                PCB.authorization.isOpaque = RV_TRUE;
                PCB.authorization.opaque = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_NONCE_COUNT:
            {
                PCB.authorization.isNonceCount = RV_TRUE;
                PCB.authorization.nonceCount = PCB.digestChallenge.challenge;
                break;
            }
            case PARSER_DIGEST_TYPE_MESSAGE_QOP:
            {
                PCB.authorization.eAuthQop = PCB.digestChallenge.qopVal.eAuthQop;
                break;
            }
            case PARSER_DIGEST_TYPE_AUTH_PARAM:
            {
                PCB.authorization.isAuthParam = RV_TRUE;
                PCB.authorization.authParam = PCB.digestChallenge.challenge;
                break;
            }
			case PARSER_DIGEST_TYPE_AUTS:
            {
                PCB.authorization.isAuts = RV_TRUE;
                PCB.authorization.auts = PCB.digestChallenge.challenge;
                break;
            }
			case PARSER_DIGEST_TYPE_INTEGRITY_PROTECTED:
            {
                PCB.authorization.eProtected = PCB.digestChallenge.eProtected;
                break;
            }
            default:
                    PCB.exit_flag = AG_SEMANTIC_ERROR_CODE;
                    PCB.eStat = RV_ERROR_ILLEGAL_SYNTAX;
        }
    
}
#define ag_rp_350(PCB_POINTER, useranme) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_USERNAME, useranme))
#define ag_rp_351(PCB_POINTER, realm) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_REALM, realm))
#define ag_rp_352(PCB_POINTER, nonce) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_NONCE, nonce))
#define ag_rp_353(PCB_POINTER, response) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_DIGEST_RESPONSE, response))
#define ag_rp_354(PCB_POINTER, cnonce) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_CNONCE, cnonce))
#define ag_rp_355(PCB_POINTER, opaque) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_OPAQUE, opaque))
#define ag_rp_356(PCB_POINTER, nonceCount) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_NONCE_COUNT, nonceCount))
#define ag_rp_357(PCB_POINTER, authParam) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_AUTH_PARAM, authParam))
#define ag_rp_358(PCB_POINTER, auts) (CUR_AUTH_CHALLENGE(PARSER_DIGEST_TYPE_AUTS, auts))
static void ag_rp_359(PCB_DECL, RvSipAuthIntegrityProtected protected) {
/* Line 5283, RV SIP Stack */
        PCB.digestChallenge.eType = PARSER_DIGEST_TYPE_INTEGRITY_PROTECTED;
        PCB.digestChallenge.eProtected = protected;
    
}
static void ag_rp_360(PCB_DECL, RvSipAuthQopOption messageQop) {
/* Line 5288, RV SIP Stack */
        PCB.digestChallenge.eType = PARSER_DIGEST_TYPE_MESSAGE_QOP;
        PCB.digestChallenge.qopVal.eAuthQop = messageQop;
    
}
static void ag_rp_361(PCB_DECL) {
/* Line 5293, RV SIP Stack */
        PCB.digestChallenge.eType = PARSER_DIGEST_TYPE_DIGEST_URI;
        PCB.digestChallenge.uri = PCB.exUri;
    
}
static void ag_rp_362(PCB_DECL, ParserAlgorithm algorithm) {
/* Line 5298, RV SIP Stack */
        PCB.digestChallenge.eType = PARSER_DIGEST_TYPE_ALGORITHM;
        
        if(PCB.digestChallenge.algorithm.eAlgorithm == RVSIP_AUTH_ALGORITHM_UNDEFINED)
        {
            PCB.digestChallenge.algorithm = algorithm;
        }
    
}
#define ag_rp_363(PCB_POINTER, username) (username)
#define ag_rp_364(PCB_POINTER, t) (t)
static void ag_rp_365(PCB_DECL) {
/* Line 5330, RV SIP Stack */
PCB.authorization.uriEnd = (RvChar *)PCB.pointer;
}
static void ag_rp_366(PCB_DECL) {
/* Line 5336, RV SIP Stack */
 
                SIP_BEGIN_TOKEN();
                PCB.authorization.uriBegin = (RvChar *)PCB.pointer;
              
}
static RvSipAuthQopOption ag_rp_367(PCB_DECL, ParserAuthQopOptions qopVal) {
/* Line 5347, RV SIP Stack */
        RV_UNUSED_ARG(pcb_pointer);
        if (RV_TRUE == qopVal.isAuth)
        {
            return RVSIP_AUTH_QOP_AUTH_ONLY;
        }
        else if (RV_TRUE == qopVal.isAuthInt)
        {
            return RVSIP_AUTH_QOP_AUTHINT_ONLY;
        }
        else if (RV_TRUE == qopVal.isOther)
        {
            return RVSIP_AUTH_QOP_OTHER;
        }
        else 
        {
             return RVSIP_AUTH_QOP_UNDEFINED;
        }
    
}
#define ag_rp_368(PCB_POINTER, cnonceVal) (cnonceVal)
#define ag_rp_369(PCB_POINTER, nonceVal) (nonceVal)
#define ag_rp_370(PCB_POINTER, ncVal) (ncVal)
#define ag_rp_371(PCB_POINTER) (CUR_STRING())
#define ag_rp_372(PCB_POINTER) (CUR_STRING())
#define ag_rp_373(PCB_POINTER) (CUR_STRING())
#define ag_rp_374(PCB_POINTER) (CUR_STRING())
#define ag_rp_375(PCB_POINTER) (CUR_STRING())
#define ag_rp_376(PCB_POINTER) (CUR_STRING())
#define ag_rp_377(PCB_POINTER) (CUR_STRING())
#define ag_rp_378(PCB_POINTER) (CUR_STRING())
#define ag_rp_379(PCB_POINTER, requestDigest) (requestDigest)
#define ag_rp_380(PCB_POINTER) (CUR_STRING())
#define ag_rp_381(PCB_POINTER) (CUR_STRING())
#define ag_rp_382(PCB_POINTER, autsValue) (autsValue)
#define ag_rp_383(PCB_POINTER) (CUR_STRING())
#define ag_rp_384(PCB_POINTER) (RVSIP_AUTH_INTEGRITY_PROTECTED_NO)
#define ag_rp_385(PCB_POINTER) (RVSIP_AUTH_INTEGRITY_PROTECTED_YES)
static void ag_rp_386(PCB_DECL) {
/* Line 5466, RV SIP Stack */
         RvStatus status;
         status = ParserInitRoute(PCB.pParserMgr,&PCB,PCB.pointer,&(PCB.route), PCB.eHeaderType, PCB.pSipObject);
         if(RV_ERROR_ILLEGAL_SYNTAX == status)
         {
             PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
             SYNTAX_ERROR;
         }
         else if (RV_OK != status)
         {
             
             PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
             PCB.eStat = status;
         }
    
}
static void ag_rp_387(PCB_DECL) {
/* Line 5482, RV SIP Stack */
 
        RvStatus status;
        status = ParserInitRoute(PCB.pParserMgr,&PCB,PCB.pointer,&(PCB.route), PCB.eHeaderType, PCB.pSipObject);
        if(RV_ERROR_ILLEGAL_SYNTAX == status)
        {
             PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
             SYNTAX_ERROR;
        }
        else if (RV_OK != status)
        {
             
             PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
             PCB.eStat = status;
        }
    
}
static void ag_rp_388(PCB_DECL) {
/* Line 5502, RV SIP Stack */
        PCB.route.nameAddr = PCB.nameAddr;
        PCB.route.isParams = RV_FALSE;
        PCB.route.eRouteType = PCB.eWhichHeader;
    
}
static void ag_rp_389(PCB_DECL) {
/* Line 5507, RV SIP Stack */
 
        PCB.route.nameAddr = PCB.nameAddr;
        PCB.route.isParams = RV_TRUE;
        PCB.route.eRouteType = PCB.eWhichHeader;
          
         if (((ParserExtensionString *)PCB.pExtParams)->size !=0)
         {
             PCB.route.routeParams = PCB.pExtParams;
         }                
     
}
static void ag_rp_390(PCB_DECL) {
/* Line 6728, RV SIP Stack */
   
		RvStatus status; 
		status = ParserInitAuthenticationInfo(PCB.pParserMgr,PCB.pointer,&(PCB.auth_info),PCB.eHeaderType,PCB.pSipObject);
		if(RV_ERROR_ILLEGAL_SYNTAX == status)
        {
            PCB.exit_flag = AG_SYNTAX_ERROR_CODE;
            SYNTAX_ERROR;
        }
        else if (RV_OK != status)
		{
			
			PCB.exit_flag = AG_REDUCTION_ERROR_CODE;
			PCB.eStat = status;
		}
		
    
}
static void ag_rp_391(PCB_DECL, ParserBasicToken nc) {
/* Line 6752, RV SIP Stack */
		PCB.auth_info.isNonceCount  = RV_TRUE;
		PCB.auth_info.strNonceCount = nc;
	
}
static void ag_rp_392(PCB_DECL, ParserBasicToken nv) {
/* Line 6757, RV SIP Stack */
		PCB.auth_info.isNextNonce   = RV_TRUE;
		PCB.auth_info.strNextNonce  = nv;
	
}
static void ag_rp_393(PCB_DECL, ParserBasicToken cn) {
/* Line 6762, RV SIP Stack */
		PCB.auth_info.isCNonce      = RV_TRUE;
		PCB.auth_info.strCNonce     = cn;
	
}
static void ag_rp_394(PCB_DECL, ParserBasicToken rd) {
/* Line 6767, RV SIP Stack */
		PCB.auth_info.isRspAuth     = RV_TRUE;
		PCB.auth_info.strRspAuth    = rd;
	
}
static void ag_rp_395(PCB_DECL) {
/* Line 6772, RV SIP Stack */
		PCB.auth_info.isMsgQop      = RV_TRUE;
	
}
static void ag_rp_396(PCB_DECL, ParserAuthQopOptions qopVal) {
/* Line 6780, RV SIP Stack */
		if (RV_TRUE == qopVal.isAuth)
        {
            PCB.auth_info.eMsgQop = RVSIP_AUTH_QOP_AUTH_ONLY;
        }
        else if (RV_TRUE == qopVal.isAuthInt)
        {
            PCB.auth_info.eMsgQop = RVSIP_AUTH_QOP_AUTHINT_ONLY;
        }
        else if (RV_TRUE == qopVal.isOther)
        {
            PCB.auth_info.eMsgQop = RVSIP_AUTH_QOP_OTHER;
        }
        else 
        {
	    PCB.auth_info.eMsgQop = RVSIP_AUTH_QOP_UNDEFINED;
        }
		
		PCB.auth_info.strMsgQop = qopVal.strOther;
	
}
#define ag_rp_397(PCB_POINTER, nv) (nv)
#define ag_rp_398(PCB_POINTER, val) (val)
#define ag_rp_399(PCB_POINTER) (CUR_STRING())
#define READ_COUNTS 
#define WRITE_COUNTS 
#undef V
#define V(i,t) (*(t *) (&(PCB).vs[(PCB).ssx + i]))
#undef VS
#define VS(i) (PCB).vs[(PCB).ssx + i]
#ifndef GET_CONTEXT
#define GET_CONTEXT CONTEXT = (PCB).input_context
#endif
typedef enum {
  ag_action_1,
  ag_action_2,
  ag_action_3,
  ag_action_4,
  ag_action_5,
  ag_action_6,
  ag_action_7,
  ag_action_8,
  ag_action_9,
  ag_action_10,
  ag_action_11,
  ag_action_12
} ag_parser_action;
#ifndef NULL_VALUE_INITIALIZER
#define NULL_VALUE_INITIALIZER = { 0 }
#endif
static SipParser_vs_type const ag_null_value NULL_VALUE_INITIALIZER;
static const unsigned short ag_rpx[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  1,  2,  3,  4,  0,  0,  5,  6,  7,  8,  9, 10, 11, 12,
   13, 14, 15,  0,  0, 16,  0,  0, 17,  0,  0, 18,  0,  0,  0,  0,  0, 19,
    0,  0,  0,  0,  0, 20,  0,  0,  0,  0, 21, 22, 23, 24, 25, 26, 27, 28,
   29,  0,  0, 30, 31, 32, 33, 34, 35, 36, 37, 38,  0, 39,  0, 40, 41, 42,
   43, 44,  0,  0, 45, 46, 47, 48,  0,  0, 49, 50, 51,  0,  0,  0,  0,  0,
    0, 52, 53,  0, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
   68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81,  0, 82, 83, 84,
   85,  0,  0, 86, 87, 88, 89, 90, 91,  0, 92, 93, 94, 95, 96, 97, 98, 99,
  100,101,102,  0,103,104,  0,  0,105,106,  0,  0,107,  0,  0,108,109,110,
  111,  0,112,113,114,115,116,117,118,119,120,121,122,  0,  0,  0,  0,123,
  124,125,126,  0,127,128,  0,129,130,131,132,133,134,135,  0,  0,136,137,
    0,  0,  0,  0,  0,  0,138,139,140,141,142,143,144,  0,  0,145,146,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,
  163,164,165,166,167,168,169,170,171,172,173,  0,  0,  0,  0,174,175,176,
    0,  0,  0,  0,  0,177,178,179,180,181,182,183,184,185,186,187,188,  0,
    0,189,  0,  0,  0,  0,  0,190,191,192,193,194,195,196,197,198,199,200,
  201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,
  219,  0,  0,  0,  0,  0,220,221,222,223,224,225,226,  0,227,  0,  0,228,
  229,230,231,232,233,  0,  0,234,235,236,237,238,  0,  0,239,240,  0,  0,
    0,  0,241,242,243,244,  0,245,246,247,248,249,250,251,252,253,254,255,
  256,257,258,259,260,261,262,263,264,265,266,267,268,269,270,271,272,273,
  274,275,276,277,278,279,280,281,282,283,284,285,286,287,288,  0,289,290,
  291,292,293,294,295,296,297,298,299,300,301,302,303,304,305,  0,  0,306,
  307,  0,  0,308,  0,  0,309,310,311,  0,  0,  0,312,313,314,315,316,317,
    0,  0,318,319,320,321,322,323,324,325,326,327,328,329,330,331,332,333,
  334,335,336,337,338,  0,  0,339,340,341,342,343,  0,  0,  0,  0,344,345,
    0,346,347,348,349,350,351,352,353,354,355,356,357,358,359,360,361,362,
  363,364,  0,365,366,  0,  0,367,368,369,370,371,372,373,374,375,376,377,
  378,379,380,381,  0,  0,  0,382,  0,  0,  0,  0,383,  0,  0,384,385,  0,
  386,387,388,389,390,  0,  0,391,392,393,394,395,396,397,398,  0,  0,  0,
    0,  0,399
};
static const unsigned char ag_key_itt[] = {
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0
};
static const unsigned short ag_key_pt[] = {
  0,  2,  0,  6,  0,  8,  0, 10,  0, 12,  0, 14,  0, 16,  0, 18,
  0, 20,  0, 22,  0, 24,  0, 26,  0, 30,  0, 92,  0, 93,  0, 94,
  0, 95,  0, 96,  0,133,  0,135,  0,136,  0,137,  0,138,  0,153,
  0,154,  0,156,  0,160,  0,162,  0,163,  0,165,  0,166,  0,168,
  0,169,  0,254,  0,255,  0,258,  0,259,  0,260,  0,261,  0,262,
  0,263,  0,264,  0,271,  0,272,  0,288,  0,289,  0,290,  0,291,
  0,292,  0,293,  0,294,  0,295,  0,296,  0,297,  0,298,  0,299,
  0,300,  0,309,  0,314,  0,321,  0,327,  0,329,  0,330,  0,332,
  0,340,  0,341,  0,342,  0,343,  0,344,  0,345,  0,346,  0,350,
  0,351,  0,352,  0,353,  0,354,  0,355,  0,356,  0,357,  0,358,
  0,359,  0,360,  0,361,  0,366,  0,367,  0,368,  0,371,  0,386,
  0,388,  0,400,  0,402,  0,404,  0,405,  0,409,  0,418,  0,421,
  0,424,  0,425,  0,426,  0,428,  0,443,  0,445,  0,449,  0,451,
  0,454,  0,459,  0,467,  0,468,  0,469,  0,477,  0,478,0
};
static const unsigned char ag_key_ch[] = {
    0, 49, 50, 67, 73,255, 89,255, 65, 73, 76, 79, 83,255, 77, 78,255, 69,
  255, 82,255, 69, 72,255, 47, 58, 83,255, 80,255, 73,255, 82,255, 73,255,
   65, 66, 67, 68, 69, 70, 73, 75, 76, 77, 79, 80, 82, 83, 84, 85, 86, 88,
  255, 32,255, 72, 85,255, 72, 82, 85,255, 75, 84,255, 58, 82,255, 79,255,
   71, 73,255, 45,255, 72, 83,255, 71, 84,255, 67, 75, 76, 80, 85,255, 82,
   89,255, 65, 75, 78, 79,255, 69, 73, 79,255, 65, 69, 82,255, 83, 84,255,
   77,255, 84, 86,255, 75, 77, 78, 80,255, 76, 78,255, 65, 85,255, 68, 82,
   89,255, 65, 69, 79,255, 78, 86,255, 67, 69, 79,255, 67, 80, 82,255, 69,
   79,255, 72, 82,255, 79,255, 65, 67, 68, 71, 83,255, 69, 80, 83,255, 45,
  255, 80,255, 77,255, 79,255, 67,255, 58, 83,255, 71, 80,255, 65, 67, 69,
   73, 84, 85,255, 65, 85,255, 65, 67, 72, 76, 79, 82, 84, 85,255, 78,255,
   82,255, 69,255, 68, 82, 83, 84,255, 42, 58, 65, 66, 67, 68, 69, 70, 71,
   72, 73, 74, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 87, 89,255, 58, 83,
  255, 80,255, 73,255, 73, 80, 83,255, 67, 69,255, 58, 83,255, 80,255, 65,
   73, 85,255, 72, 85,255, 42, 67, 68, 70, 73, 77, 78, 80, 81, 82, 83, 84,
   87,255, 67, 69,255, 67, 78, 81, 82,255, 68,255, 83,255, 58, 83,255, 80,
  255, 73,255, 42, 73, 80, 83,255, 65, 85,255, 72, 85,255, 70, 77, 83, 84,
   87,255, 58,255, 65, 69,255, 79, 82, 84,255, 67, 76, 77, 79, 83, 84, 85,
  255, 83,255, 76, 85,255, 67, 79,255, 65, 83,255, 69,255, 82, 83,255, 65,
   67, 73, 78, 79, 81, 82, 85,255, 65, 67, 68, 73, 78, 79, 81, 82, 83,255,
   65, 69, 81,255, 80, 85,255, 76, 78,255, 65, 85,255, 82, 89,255, 65,255,
   65, 68, 70, 74, 77, 78, 79, 83,255, 84,255, 65, 66, 67, 73, 82,255, 45,
  255, 72,255, 84,255, 85,255, 65,255, 75, 76,255, 65,255, 32, 49, 79, 84,
  255, 77,255, 83,255, 73, 80,255, 67, 76,255, 83, 84, 85,255, 78, 89,255,
   70, 84,255, 69, 80,255, 65, 66, 67, 72, 77, 82, 83, 84,255, 80, 82,255,
   58, 83,255, 80,255, 73,255, 83,255, 61,255, 61,255, 83, 84,255, 77,255,
   71, 85,255
};
static const unsigned char ag_key_act[] = {
  0,5,5,7,5,4,7,4,7,5,5,5,5,4,3,7,4,3,4,6,4,7,5,4,3,0,3,4,2,4,2,4,5,4,5,
  4,2,6,6,7,7,5,6,5,5,5,5,2,6,2,5,6,6,5,4,0,4,3,3,4,3,3,3,4,5,7,4,0,7,4,
  2,4,2,7,4,7,4,6,5,4,5,2,4,2,3,2,7,2,4,7,7,4,7,5,7,7,4,7,7,7,4,7,7,7,4,
  7,5,4,2,4,7,7,4,5,3,2,5,4,5,5,4,7,2,4,7,5,5,4,2,7,7,4,7,5,4,5,7,6,4,7,
  7,7,4,3,7,4,7,2,4,7,4,7,7,7,7,7,4,2,7,7,4,7,4,6,4,2,4,2,4,2,4,0,3,4,2,
  6,4,7,7,7,2,7,7,4,7,7,4,7,7,7,7,7,2,7,7,4,7,4,6,4,2,4,7,7,2,7,4,5,3,2,
  2,2,2,7,2,2,7,2,2,7,2,2,2,2,6,2,2,2,2,7,7,4,0,3,4,2,4,2,4,3,3,2,4,5,7,
  4,0,3,4,6,4,7,2,7,4,7,7,4,5,7,7,7,3,7,2,3,7,7,2,2,7,4,5,7,4,7,2,7,7,4,
  7,4,7,4,0,3,4,2,4,2,4,5,3,3,2,4,7,7,4,7,7,4,7,7,2,2,7,4,3,4,7,7,4,7,7,
  7,4,7,7,2,7,7,2,7,4,3,4,7,7,4,5,7,4,7,7,4,2,4,7,7,4,2,7,7,2,7,7,2,2,4,
  7,7,7,7,7,7,7,7,7,4,7,7,5,4,7,7,4,5,5,4,7,2,4,5,5,4,2,4,2,7,7,2,2,7,7,
  7,4,7,4,7,7,7,7,7,4,7,4,6,4,2,4,2,4,2,4,3,3,4,2,4,0,5,7,7,4,7,4,7,4,7,
  7,4,7,7,4,7,2,7,4,7,7,4,7,7,4,7,7,4,7,7,7,7,7,2,7,7,4,7,7,4,0,3,4,2,4,
  2,4,2,4,0,4,1,4,7,5,4,2,4,2,7,4
};
static const unsigned short ag_key_parm[] = {
    0, 18, 20, 28, 22,  0, 30,  0, 32, 10, 12,  8,  0,  0,198, 26,  0,196,
    0,  2,  0, 34, 16,  0,118,191,192,  0,  0,  0,  0,  0, 24,  0, 14,  0,
    0,102, 92,  6,  4, 98, 90,104,112, 88,108,  0,100,  0, 96,110, 94,106,
    0,399,  0,223,211,  0,223,195,211,  0, 28,120,  0,420,186,  0,  0,  0,
    0, 48,  0,192,  0,194,210,  0,156,  0,  0,  0,419,  0,148,  0,  0, 58,
   30,  0, 32,198,204, 60,  0,164,172,176,  0,184,144,138,  0,170,166,  0,
    0,  0,212, 26,  0,196,198,  0, 72,  0,154,152,  0,142,  0,  0, 54,146,
  150,  0,  0, 74,130,  0,178,162,  0,206,218,214,  0,160,180, 86,  0,196,
  122,  0, 70,  0,  0,190,  0,174, 56,124, 34,208,  0,  0, 52,220,  0, 64,
    0, 62,  0,  0,  0,  0,  0,  0,  0,191,192,  0,  0, 36,  0,140, 42,158,
    0,182,128,  0, 66, 80,  0,114, 40,136, 44, 84,  0, 50,132,  0,200,  0,
   68,  0,  0,  0, 38,202,  0,168,  0,116,235,  0,  0,  0,  0,126,  0,  0,
   46,  0,  0, 76,  0,  0,  0,  0,118,  0,  0,  0,  0,134,216,  0,191,192,
    0,  0,  0,  0,  0,198,196,  0,  0,206,218,  0,191,192,  0, 36,  0,140,
    0,128,  0,136,132,  0,116,204,172,138,198,130,  0,196,190,220,  0,  0,
  134,  0,206,218,  0,204,  0,190,220,  0,172,  0, 36,  0,191,192,  0,  0,
    0,  0,  0,116,198,196,  0,  0,140,128,  0,136,132,  0,138,130,  0,  0,
  134,  0,235,  0, 54, 74,  0, 84, 66, 50,  0, 60, 76,  0, 86, 64,  0, 68,
    0,118,  0,186,210,  0,206,178,  0,174,208,  0,  0,  0,202,200,  0,  0,
  204,212,  0,180,190,  0,  0,  0,186,198,176,196,178,180,190,174,182,  0,
  120,126,118,  0,148,156,  0,154,152,  0,142,  0,  0,146,150,  0,  0,  0,
    0,164,144,  0,  0,162,160,158,  0,114,  0, 28, 30, 32, 26, 34,  0,192,
    0,194,  0,  0,  0,  0,  0,  0,  0,419,420,  0,  0,  0,399, 78, 82, 80,
    0,188,  0, 62,  0, 72, 70,  0, 40, 44,  0, 42,  0, 38,  0,214,216,  0,
  184, 80,  0, 56, 52,  0, 48, 58, 60, 46, 54,  0, 64, 50,  0,122,124,  0,
  191,192,  0,  0,  0,  0,  0,  0,  0,466,  0,465,  0,170,166,  0,  0,  0,
    0,168,  0
};
static const unsigned short ag_key_jmp[] = {
    0,  0,  0,  0,  0,  0,  2,  0,  4,  0,  0,  0,  0,  0, 13, 15,  0, 20,
    0, 17,  0, 23,  0,  0, 30,  0, 34,  0, 24,  0, 28,  0,  0,  0,  0,  0,
    1,  6,  8,  9, 11,  0, 14,  0,  0,  0,  0, 19, 21, 30,  0, 32, 34,  0,
    0,  0,  0, 36, 41,  0, 50, 55, 62,  0,  0, 73,  0,  0, 83,  0, 67,  0,
   70, 88,  0, 93,  0, 75,  0,  0,  0, 77,  0, 64, 77, 72, 91, 80,  0, 97,
  102,  0,104,  0,109,114,  0,117,119,124,  0,136,140,142,  0,144,  0,  0,
  105,  0,154,171,  0,  0,152,110,  0,  0,  0,  0,  0,175,118,  0,179,  0,
    0,  0,124,182,187,  0,197,  0,  0,  0,189,132,  0,200,202,207,  0,214,
  217,  0,210,143,  0,220,  0,222,225,231,237,243,  0,151,249,253,  0,266,
    0,161,  0,163,  0,165,  0,167,  0,  0,269,  0,169,171,  0,259,261,264,
  174,271,275,  0,296,303,  0,277,279,281,283,285,184,305,307,  0,313,  0,
  196,  0,198,  0,309,311,200,317,  0,  0, 71, 83, 89, 92, 97,129,101,108,
  146,113,121,177,128,135,139,146,149,157,177,187,202,319,322,  0,  0,333,
    0,232,  0,235,  0,325,328,237,  0,  0,356,  0,  0,381,  0,246,  0,379,
  249,383,  0,385,387,  0,  0,335,341,347,350,353,243,364,369,372,251,255,
  389,  0,  0,398,  0,392,272,406,409,  0,416,  0,422,  0,  0,433,  0,284,
    0,287,  0,  0,425,428,289,  0,441,443,  0,445,447,  0,435,438,296,299,
  449,  0,452,  0,460,464,  0,483,494,502,  0,454,458,310,469,473,313,504,
    0,508,  0,515,523,  0,  0,551,  0,564,567,  0,333,  0,573,575,  0,327,
  526,532,330,555,561,336,338,  0,582,591,593,599,601,606,612,615,620,  0,
  625,631,  0,  0,638,640,  0,  0,  0,  0,648,367,  0,  0,  0,  0,373,  0,
  364,642,645,370,376,650,653,656,  0,659,  0,662,665,668,674,680,  0,688,
    0,395,  0,397,  0,399,  0,401,  0,692,698,  0,405,  0,  0,  0,702,704,
    0,708,  0,711,  0,718,720,  0,729,731,  0,725,422,733,  0,736,738,  0,
  741,746,  0,776,783,  0,750,755,761,765,771,435,787,797,  0,800,805,  0,
    0,813,  0,450,  0,453,  0,455,  0,  0,  0,459,  0,815,  0,  0,463,  0,
  466,817,  0
};
static const unsigned short ag_key_index[] = {
   36, 55, 57, 60, 60,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 55, 55,  0,  0,207,  0,239,239,
    0,258,  0,275,280,280,  0,282,  0,  0,291,302,302,239,  0,  0, 55,308,
   55,  0,  0,  0,  0,317, 55, 55, 55,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,239,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 60, 60, 57,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0, 55,308,308,  0,308,  0,  0,  0,  0,  0,317,317,  0, 55,  0,  0,
  325,  0,  0,  0,  0,  0,  0,  0,  0,  0,341,  0,350,  0,  0,  0,  0,  0,
  239,239,  0,  0,  0,  0,282,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,360,
    0,  0,239,  0,378,  0,387,  0,  0,389,308,  0,308,  0,  0, 55,  0, 55,
   55,  0,  0,  0, 55,  0,  0,  0,  0,  0,317,  0,  0,  0,  0,  0,  0,  0,
  403,  0,  0,  0,  0,275,  0,  0,  0,408,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,239,  0,  0,  0,
    0,  0,  0,282,  0,  0,  0,  0,  0,  0,  0,  0,360,360,  0,239,  0,  0,
    0,  0,387,387,  0,308,  0,  0, 55,  0,  0,  0,  0, 55,410,  0,  0,  0,
    0,  0, 55,  0,  0,403,  0,  0,  0,  0,275,  0,  0,  0,415,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,341,  0,  0,  0,  0,  0,  0,350,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,360,  0,  0,  0,
    0,  0,387,  0,  0,  0,  0,  0,417,389,  0,  0,419,425, 55,403,  0,  0,
    0,  0,408,  0,  0,  0,403,429,  0,  0,  0,  0,  0,  0,  0,  0,341,  0,
  432,  0,  0,  0,  0,350,  0,  0,  0,  0,  0,438,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,417,389,  0,  0,419,425,  0,  0,  0,  0,  0,408,
    0,  0,  0,  0,403,429,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,432,
    0,  0,  0,  0,  0,  0,  0,438,  0,  0,447,  0,  0,  0,  0,  0,  0, 55,
   55,  0,  0,  0,  0,415,239,403,  0,  0,  0,  0,  0,403,  0,  0,457,  0,
  425,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,447,  0,  0,  0,
    0,  0, 55,  0,  0,  0,461,  0,  0,  0,  0,  0,457,457,  0,425,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,302,  0,  0,468,  0,  0,  0,  0,  0, 55,
    0,  0,  0,  0,  0,  0,417,  0,  0,308,302,  0,  0,  0,403,457,  0,  0,
    0,  0,  0,417,  0,308,  0,  0,  0,  0,403,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};
static const unsigned char ag_key_ends[] = {
75,0, 69,0, 78,67,69,76,0, 84,0, 88,0, 58,0, 86,73,84,69,0, 
83,58,0, 71,73,83,84,69,82,0, 50,46,48,0, 58,0, 79,83,84,58,0, 
83,69,82,73,78,70,79,58,0, 79,83,84,58,0, 86,45,65,66,83,58,0, 
83,69,82,73,78,70,79,58,0, 58,0, 73,79,78,0, 65,86,69,82,58,0, 
73,84,72,77,0, 65,83,0, 82,0, 73,78,84,0, 65,78,67,72,0, 69,0, 
78,67,69,76,0, 79,78,67,69,0, 77,80,0, 67,0, 71,69,83,84,0, 
77,65,73,78,0, 88,80,73,82,69,83,0, 76,83,69,0, 66,0, 73,0, 
84,0, 73,68,68,69,78,0, 58,0, 
69,71,82,73,84,89,45,80,82,79,84,69,67,84,69,68,0, 73,84,69,0, 
78,0, 82,0, 68,82,0, 84,72,79,68,0, 78,0, 
88,84,78,79,78,67,69,0, 67,69,0, 84,0, 65,81,85,69,0, 73,71,0, 
79,78,69,0, 83,58,0, 88,89,0, 80,0, 76,77,0, 69,73,86,69,68,0, 
73,82,69,67,84,0, 73,83,84,69,82,0, 80,79,78,83,69,0, 79,82,84,0, 
80,65,85,84,72,0, 84,0, 84,80,0, 80,0, 73,68,0, 58,0, 
65,76,69,0, 78,0, 71,0, 80,0, 85,0, 83,0, 
75,69,78,73,90,69,68,45,66,89,0, 78,83,80,79,82,84,0, 69,0, 76,0, 
69,0, 80,0, 73,0, 65,77,69,0, 67,0, 69,68,0, 69,83,0, 77,58,0, 
82,69,83,58,0, 58,0, 78,79,78,67,69,0, 73,71,69,83,84,0, 82,73,0, 
77,58,0, 79,78,0, 88,84,78,79,78,67,69,0, 82,69,83,58,0, 79,80,0, 
83,80,65,85,84,72,0, 84,0, 58,0, 78,0, 85,0, 69,0, 69,68,0, 
78,79,78,67,69,0, 88,84,78,79,78,67,69,0, 79,80,0, 
83,80,65,85,84,72,0, 73,71,69,83,84,0, 73,80,0, 77,58,0, 
82,69,83,58,0, 58,0, 82,73,0, 79,78,0, 84,0, 78,0, 85,0, 69,0, 
69,68,0, 58,0, 79,77,80,0, 82,0, 68,68,82,0, 84,72,79,68,0, 
82,73,71,0, 73,71,67,79,77,80,45,73,68,0, 
75,69,78,73,90,69,68,45,66,89,0, 65,78,83,80,79,82,84,0, 76,0, 
83,69,82,0, 73,80,47,50,46,48,0, 71,79,82,73,84,72,77,0, 84,83,0, 
78,79,78,67,69,0, 
78,84,69,71,82,73,84,89,45,80,82,79,84,69,67,84,69,68,0, 78,67,69,0, 
80,65,81,85,69,0, 79,80,0, 76,77,0, 80,79,78,83,69,0, 73,0, 
69,82,78,65,77,69,0, 76,71,79,82,73,84,72,77,0, 75,0, 
79,77,65,73,78,0, 75,0, 79,78,67,69,0, 80,65,81,85,69,0, 79,80,0, 
69,65,76,77,0, 84,65,76,69,0, 67,84,73,79,78,0, 
88,80,73,82,69,83,0, 82,0, 71,0, 69,67,0, 69,66,0, 78,0, 
79,86,0, 67,84,0, 69,80,0, 65,71,0, 67,75,0, 89,69,0, 
65,78,67,69,76,0, 78,86,73,84,69,0, 69,71,73,83,84,69,82,0, 
73,78,84,0, 65,86,69,82,58,0, 71,79,58,0, 78,0, 82,85,69,0, 
68,53,0, 73,71,67,79,77,80,0, 80,0, 72,79,78,69,0, 67,84,80,0, 
80,0, 83,0, 68,80,0, 79,0, 69,83,0, 65,76,83,69,0, 82,85,69,0, 
76,73,65,83,0, 82,65,78,67,72,0, 79,77,80,0, 73,68,68,69,78,0, 
65,68,68,82,0, 67,69,73,86,69,68,0, 79,82,84,0, 
73,71,67,79,77,80,45,73,68,0, 84,76,0, 82,79,88,89,0, 
69,68,73,82,69,67,84,0, 58,0, 84,0, 84,67,0, 
};
#define AG_TCV(x) ag_tcv[(x)]
static const unsigned short ag_tcv[] = {
    5,497,497,497,497,497,497,497,497,483,  0,497,497,  0,497,497,497,497,
  497,497,497,497,497,497,497,497,497,497,497,497,497,497, 40,484, 62,494,
  495,484,277,484,491,491,484,485, 42, 64,232, 56,323,326,486,486,486,486,
  486,486,486,486, 36, 52, 58, 47, 60, 54, 49,415,487,487,487,487,487,488,
  488,488,488,416,488,488,488,488,488,488,488,488,488,488,417,488,488,488,
  488,228, 86,229,496,489,490,415,487,487,487,487,487,488,488,488,488,416,
  488,488,488,488,488,488,488,488,488,488,417,488,488,488,488,492,496,492,
  484,497,496,496,496,496,496,496,496,496,496,496,496,496,496,496,496,496,
  496,493,493,496,496,496,496,496,493,496,496,496,496,496,496,496,496,496,
  496,496,496,496,496,496,496,496,496,496,496,496,496,496,496,496,496,496,
  496,496,496,496,496,496,496,496,496,496,496,496,496,496,496,496,496,496,
  496,496,496,496,496,496,496,496,496,496,496,496,496,496,496,496,496,496,
  496,496,496,496,496,496,496,496,496,496,496,496,496,496,496,496,496,496,
  496,496,496,496,496,496,496,496,496,496,496,496,496,496,496,496,496,496,
  496,496,496,496
};
#ifndef SYNTAX_ERROR
#define SYNTAX_ERROR fprintf(stderr,"%s, line %d, column %d\n", \
  (PCB).error_message, (PCB).line, (PCB).column)
#endif
#ifndef FIRST_LINE
#define FIRST_LINE 1
#endif
#ifndef FIRST_COLUMN
#define FIRST_COLUMN 1
#endif
#ifndef PARSER_STACK_OVERFLOW
#define PARSER_STACK_OVERFLOW {fprintf(stderr, \
   "\nParser stack overflow, line %d, column %d\n",\
   (PCB).line, (PCB).column);}
#endif
#ifndef REDUCTION_TOKEN_ERROR
#define REDUCTION_TOKEN_ERROR {fprintf(stderr, \
    "\nReduction token error, line %d, column %d\n", \
    (PCB).line, (PCB).column);}
#endif
#ifndef INPUT_CODE
#define INPUT_CODE(T) (T)
#endif
typedef enum
  {ag_accept_key, ag_set_key, ag_jmp_key, ag_end_key, ag_no_match_key,
   ag_cf_accept_key, ag_cf_set_key, ag_cf_end_key} key_words;
static void ag_get_key_word(PCB_DECL, int ag_k) {
  int ag_save = (int) ((PCB).la_ptr - (PCB).pointer);
  const  unsigned char *ag_p;
  int ag_ch;
  while (1) {
    switch (ag_key_act[ag_k]) {
    case ag_cf_end_key: {
      const  unsigned char *sp = ag_key_ends + ag_key_jmp[ag_k];
      do {
        if ((ag_ch = *sp++) == 0) {
          int ag_k1 = ag_key_parm[ag_k];
          int ag_k2 = ag_key_pt[ag_k1];
          if (ag_key_itt[ag_k2 + CONVERT_CASE(*(PCB).la_ptr)]) goto ag_fail;
          (PCB).token_number = (SipParser_token_type) ag_key_pt[ag_k1 + 1];
          return;
        }
      } while (CONVERT_CASE(*(PCB).la_ptr++) == ag_ch);
      goto ag_fail;
    }
    case ag_end_key: {
      const  unsigned char *sp = ag_key_ends + ag_key_jmp[ag_k];
      do {
        if ((ag_ch = *sp++) == 0) {
          (PCB).token_number = (SipParser_token_type) ag_key_parm[ag_k];
          return;
        }
      } while (CONVERT_CASE(*(PCB).la_ptr++) == ag_ch);
    }
    case ag_no_match_key:
ag_fail:
      (PCB).la_ptr = (PCB).pointer + ag_save;
      return;
    case ag_cf_set_key: {
      int ag_k1 = ag_key_parm[ag_k];
      int ag_k2 = ag_key_pt[ag_k1];
      ag_k = ag_key_jmp[ag_k];
      if (ag_key_itt[ag_k2 + CONVERT_CASE(*(PCB).la_ptr)]) break;
      ag_save = (int) ((PCB).la_ptr - (PCB).pointer);
      (PCB).token_number = (SipParser_token_type) ag_key_pt[ag_k1+1];
      break;
    }
    case ag_set_key:
      ag_save = (int) ((PCB).la_ptr - (PCB).pointer);
      (PCB).token_number = (SipParser_token_type) ag_key_parm[ag_k];
    case ag_jmp_key:
      ag_k = ag_key_jmp[ag_k];
      break;
    case ag_accept_key:
      (PCB).token_number = (SipParser_token_type) ag_key_parm[ag_k];
      return;
    case ag_cf_accept_key: {
      int ag_k1 = ag_key_parm[ag_k];
      int ag_k2 = ag_key_pt[ag_k1];
      if (ag_key_itt[ag_k2 + CONVERT_CASE(*(PCB).la_ptr)])
        (PCB).la_ptr = (PCB).pointer + ag_save;
      else (PCB).token_number = (SipParser_token_type) ag_key_pt[ag_k1+1];
      return;
    }
    }
    ag_ch = CONVERT_CASE(*(PCB).la_ptr++);
    ag_p = &ag_key_ch[ag_k];
    while (*ag_p < ag_ch) ag_p++;
    if (*ag_p != ag_ch) {
      (PCB).la_ptr = (PCB).pointer + ag_save;
      return;
    }
    ag_k = (int) (ag_p - ag_key_ch);
  }
}
#ifndef AG_NEWLINE
#define AG_NEWLINE 10
#endif
#ifndef AG_RETURN
#define AG_RETURN 13
#endif
#ifndef AG_FORMFEED
#define AG_FORMFEED 12
#endif
#ifndef AG_TABCHAR
#define AG_TABCHAR 9
#endif
static void ag_track(PCB_DECL) {
  int ag_k = (int) ((PCB).la_ptr - (PCB).pointer);
  while (ag_k--) {
    switch (*(PCB).pointer++) {
    case AG_NEWLINE:
      (PCB).column = 1, (PCB).line++;
    case AG_RETURN:
    case AG_FORMFEED:
      break;
    case AG_TABCHAR:
      (PCB).column += (TAB_SPACING) - ((PCB).column - 1) % (TAB_SPACING);
      break;
    default:
      (PCB).column++;
    }
  }
}
static const unsigned short ag_tstt[] = {
490,489,488,487,486,485,484,417,416,415,326,323,300,299,298,297,296,295,294,
  293,292,291,290,289,288,232,198,196,192,191,118,96,95,94,93,92,64,30,26,
  24,22,20,18,16,14,12,10,8,6,2,0,1,28,29,32,33,34,35,65,67,91,114,186,
  187,188,189,193,197,286,287,
490,489,488,487,486,485,484,417,416,415,326,323,232,64,0,
223,211,0,190,199,202,208,222,
223,211,195,0,190,194,199,202,208,222,
223,211,195,0,190,194,199,202,208,222,
483,40,36,0,37,38,39,45,
40,0,39,115,
5,0,
5,0,
5,0,
5,0,
36,0,3,
5,0,
5,0,
36,0,3,
36,0,3,
36,0,3,
36,0,3,
36,0,3,
36,0,3,
36,0,3,
36,0,3,
36,0,3,
36,0,3,
36,0,3,
36,0,3,
489,488,487,486,417,416,415,326,323,232,228,64,0,140,152,210,224,
495,494,491,489,488,487,486,485,484,417,416,415,326,323,277,232,64,56,54,52,
  49,47,42,0,48,209,212,214,216,
483,399,62,60,54,52,40,5,0,51,200,241,
483,399,62,60,54,52,40,5,0,51,200,241,
488,487,486,485,417,416,415,326,323,232,64,0,174,175,178,
488,487,486,485,417,416,415,326,323,232,64,0,174,175,178,
483,40,0,
36,0,50,
488,487,486,485,483,417,416,415,326,323,232,198,196,192,191,64,40,36,0,
488,487,486,485,417,416,415,326,323,232,198,196,192,191,64,0,172,173,174,
  175,178,186,187,188,189,193,197,
486,326,323,40,0,39,116,119,
493,492,491,490,489,488,487,486,485,484,483,478,477,451,449,421,417,416,415,
  371,346,345,344,343,342,341,340,326,323,314,232,229,228,198,196,192,191,
  133,86,64,62,60,58,56,54,40,36,0,37,38,45,
488,487,486,485,417,416,415,326,323,232,64,0,31,174,175,178,
478,477,451,449,421,0,27,437,438,472,473,474,475,476,
490,489,488,487,486,485,484,417,416,415,371,326,323,232,64,0,25,65,67,431,
  432,
490,489,488,487,486,485,484,417,416,415,371,326,323,232,64,0,23,65,67,369,
  370,
490,489,488,487,486,485,484,417,416,415,326,323,232,64,62,58,0,21,57,80,180,
  302,304,470,471,
490,489,488,487,486,485,484,417,416,415,326,323,232,133,64,0,19,65,67,124,
  125,126,130,
486,326,323,0,17,100,101,
493,492,491,490,489,488,487,486,485,484,417,416,415,326,323,232,229,228,86,
  64,62,60,58,56,54,36,0,15,68,70,102,
490,489,488,487,486,485,484,417,416,415,326,323,314,232,198,196,192,191,64,
  62,58,0,13,57,80,179,180,189,193,197,203,204,205,206,207,302,304,311,
  312,313,315,
486,346,345,344,343,342,341,340,326,323,0,11,335,336,337,339,347,
486,346,345,344,343,342,341,340,326,323,0,9,100,331,334,335,339,
490,489,488,487,486,485,484,417,416,415,326,323,232,198,196,192,191,64,62,
  58,0,7,57,80,179,180,189,193,197,203,204,205,206,207,301,302,304,
486,326,323,0,4,97,98,100,
489,488,487,486,417,416,415,326,323,232,64,0,227,
489,488,487,486,417,416,415,326,323,232,64,0,227,
487,486,415,326,323,235,0,171,233,234,237,
36,0,50,
489,488,487,486,417,416,415,326,323,232,228,64,0,140,152,210,224,
495,494,491,489,488,487,486,485,484,417,416,415,326,323,277,232,64,56,54,52,
  47,42,0,
36,0,50,
49,0,48,
495,491,489,488,487,486,485,484,483,417,416,415,326,323,277,272,271,261,260,
  255,254,232,229,228,169,166,162,156,64,56,40,36,0,37,38,45,
52,0,51,
483,399,62,60,54,40,5,0,53,201,275,
483,399,62,60,54,40,5,0,53,201,275,
488,487,486,485,417,416,415,326,323,232,64,0,
36,0,50,
497,496,495,494,493,492,491,490,489,488,487,486,485,484,483,417,416,415,326,
  323,277,232,229,228,86,64,62,60,58,56,54,52,49,47,42,40,36,5,0,37,38,45,
40,0,39,115,
486,326,323,0,
40,0,39,
5,0,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
483,42,40,5,0,37,38,45,
5,0,
483,40,0,37,45,
483,40,0,37,45,
5,0,
483,40,0,37,45,
483,40,0,37,45,
5,0,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,64,58,40,36,0,305,
  306,307,
497,496,495,494,493,492,491,490,489,488,487,486,485,484,483,417,416,415,326,
  323,277,232,229,228,86,64,62,60,58,56,54,52,49,47,42,40,36,5,0,81,82,83,
  84,
490,489,488,487,486,485,484,417,416,415,326,323,232,64,0,
483,58,40,36,0,37,38,45,
488,487,486,485,483,417,416,415,326,323,232,198,196,192,191,64,40,0,37,38,
  45,
58,0,57,
483,52,42,40,5,0,37,38,45,
42,0,43,
5,0,
483,56,40,0,37,38,45,
483,40,0,37,45,
42,0,43,
5,0,
486,326,323,0,
483,40,5,0,37,38,45,
5,0,
493,492,491,490,489,488,487,486,485,484,417,416,415,326,323,232,229,228,86,
  64,62,60,58,56,54,36,0,
49,0,48,
483,40,5,0,37,38,45,
5,0,
36,0,50,
223,211,195,0,194,199,202,208,222,
223,211,195,0,194,199,202,208,222,
223,211,0,199,202,208,222,
58,0,57,
52,0,51,316,
483,52,42,40,5,0,37,38,45,
42,0,43,
5,0,
486,326,323,0,
483,42,40,0,37,38,45,
483,40,0,37,45,
483,40,0,37,45,
483,40,5,0,37,38,45,
5,0,
486,326,323,0,
483,40,5,0,37,38,45,
5,0,
52,0,51,303,
483,52,40,5,0,37,38,45,
5,0,
486,326,323,0,
483,40,0,37,45,
483,40,5,0,37,38,45,
5,0,
489,488,487,486,417,416,415,326,323,232,64,0,
487,486,415,326,323,0,
483,235,232,229,52,42,40,36,5,0,238,239,240,
487,486,483,415,326,323,232,229,52,42,40,5,0,234,236,237,
235,0,
232,0,
229,0,
486,326,323,0,100,139,
495,491,489,488,487,486,485,484,417,416,415,326,323,277,232,64,49,47,42,0,
  213,217,219,220,221,
489,488,487,486,417,416,415,326,323,232,228,64,0,140,152,210,224,
495,491,489,488,487,486,485,484,417,416,415,326,323,277,272,271,261,260,255,
  254,232,229,228,169,166,162,156,64,56,36,0,242,243,244,245,246,247,248,
  249,250,251,252,253,273,
495,491,489,488,487,486,485,484,483,417,416,415,326,323,277,272,271,261,260,
  255,254,232,229,228,169,166,162,156,64,56,40,36,0,37,38,45,
495,491,489,488,487,486,485,484,417,416,415,326,323,232,229,228,64,56,54,36,
  0,276,278,281,
277,0,
495,494,491,489,488,487,486,485,484,483,417,416,415,326,323,277,232,229,228,
  64,56,54,52,49,47,42,40,36,0,37,38,45,
497,496,495,494,493,492,491,490,489,488,487,486,485,484,417,416,415,326,323,
  277,232,229,228,86,64,62,60,58,56,54,52,49,47,42,36,0,74,
118,40,0,39,114,
486,326,323,0,
496,495,494,493,492,491,490,489,488,487,486,485,484,417,416,415,326,323,277,
  232,229,228,86,64,62,60,58,56,54,52,49,47,42,40,36,5,0,117,121,122,123,
47,0,46,
47,0,46,
47,0,46,
47,0,46,
47,0,46,
42,0,41,
490,489,488,487,486,485,484,417,416,415,326,323,232,64,0,65,67,373,430,
490,489,488,487,486,485,484,467,459,454,451,449,445,443,421,417,416,415,409,
  402,400,386,326,323,232,64,0,67,375,377,378,381,383,406,433,434,435,436,
  437,438,439,440,441,442,
490,489,488,487,486,485,484,417,416,415,326,323,232,64,0,65,67,373,430,
490,489,488,487,486,485,484,428,426,421,417,416,415,409,404,402,400,388,386,
  326,323,232,64,0,67,372,374,375,376,377,378,379,380,381,382,383,384,406,
483,40,0,305,
497,496,495,494,493,492,491,490,489,488,487,486,485,484,483,417,416,415,326,
  323,277,232,229,228,86,64,62,60,58,56,54,52,49,47,42,40,36,5,0,
497,496,495,494,493,492,491,490,489,488,487,486,485,484,483,417,416,415,326,
  323,277,232,229,228,86,64,60,58,56,54,52,49,47,42,40,36,5,0,81,82,
62,0,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,64,58,40,36,0,305,
  306,307,
488,487,486,485,417,416,415,326,323,232,198,196,192,191,64,0,173,174,175,
  178,186,187,188,189,193,197,
488,487,486,485,483,417,416,415,326,323,232,198,196,192,191,64,40,0,37,38,
  45,
52,0,51,111,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,64,62,58,40,0,37,38,
  45,
56,0,55,
489,488,487,486,417,416,415,326,323,232,228,64,0,127,140,152,224,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,133,64,40,0,37,38,
  45,
493,492,491,490,489,488,487,486,485,484,417,416,415,326,323,232,229,228,86,
  64,62,60,58,56,54,36,0,68,70,
495,494,491,489,488,487,486,485,484,417,416,415,326,323,277,232,229,228,64,
  56,49,47,36,0,183,185,
490,489,488,487,486,485,484,417,416,415,326,323,232,64,62,0,80,179,180,207,
  304,
483,54,52,42,40,5,0,37,38,45,
483,54,52,42,40,5,0,37,38,45,
490,489,488,487,486,485,484,417,416,415,326,323,232,64,62,0,80,179,180,207,
  304,
483,54,52,42,40,5,0,37,38,45,
483,54,52,42,40,5,0,37,38,45,
483,54,52,42,40,5,0,37,38,45,
483,54,52,42,40,5,0,37,38,45,
490,489,488,487,486,485,484,483,417,416,415,332,327,326,323,321,232,64,40,0,
  37,38,45,
52,0,51,
52,0,51,316,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,198,196,192,191,64,
  62,58,40,0,37,38,45,
42,0,41,
361,360,359,358,357,356,355,354,353,352,351,350,0,348,
486,326,323,0,338,362,
490,489,488,487,486,485,484,483,417,416,415,326,323,309,232,64,40,0,37,38,
  45,
52,0,51,
52,0,51,303,
490,489,488,487,486,485,484,417,416,415,326,323,232,96,95,94,93,92,64,0,65,
  67,91,
487,486,415,326,323,0,
487,486,415,326,323,0,237,
36,0,238,
487,486,483,415,326,323,232,229,52,42,40,5,0,234,236,237,
486,326,323,0,230,231,
486,326,323,0,
495,491,489,488,487,486,485,484,417,416,415,326,323,277,232,64,47,42,0,219,
495,491,489,488,487,486,485,484,483,417,416,415,399,326,323,277,232,229,228,
  64,62,60,56,54,52,47,40,36,5,0,268,269,270,
47,0,46,274,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
490,489,488,487,486,485,484,483,417,416,415,399,326,323,232,64,62,60,54,52,
  47,40,5,0,46,265,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
495,491,489,488,487,486,485,484,417,416,415,326,323,277,272,271,261,260,255,
  254,232,229,228,169,166,162,156,64,56,36,0,242,243,244,245,246,247,248,
  249,250,251,252,253,273,
495,491,489,488,487,486,485,484,417,416,415,326,323,232,229,228,64,56,54,36,
  0,
47,0,
495,491,489,488,487,486,485,484,417,416,415,326,323,232,229,228,64,56,54,36,
  0,276,278,281,
495,494,491,489,488,487,486,485,484,417,416,415,326,323,277,232,229,228,64,
  56,54,52,49,47,42,36,0,176,182,
497,496,495,494,493,492,491,490,489,488,487,486,485,484,483,417,416,415,326,
  323,277,232,229,228,86,64,62,60,58,56,54,52,49,47,42,40,36,5,0,77,78,79,
483,40,5,0,37,38,45,
496,495,494,493,492,491,490,489,488,487,486,485,484,417,416,415,326,323,277,
  232,229,228,86,64,62,60,58,56,54,52,49,47,42,40,36,0,121,
490,489,488,487,486,485,484,483,425,424,417,416,415,326,323,232,64,62,40,0,
  37,38,45,
483,62,40,0,37,38,45,
483,62,40,0,37,38,45,
483,62,40,0,37,38,45,
487,486,483,415,326,323,40,0,37,38,45,
483,478,477,451,449,421,40,0,37,38,45,
483,47,40,0,37,38,45,
42,0,41,
483,47,40,0,37,38,45,
420,419,0,407,410,414,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,64,47,40,0,45,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
483,42,40,5,0,37,38,45,
42,0,41,
42,0,41,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,64,47,40,0,45,
483,42,40,5,0,37,38,45,
42,0,41,
483,60,40,0,37,38,45,
488,487,486,485,417,416,415,326,323,232,198,196,192,191,64,0,173,174,175,
  178,186,187,188,189,193,197,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,64,40,0,37,38,45,
52,0,51,
490,489,488,487,486,485,484,417,416,415,326,323,232,64,62,58,0,57,80,180,
  302,304,471,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,64,40,0,37,38,45,
483,52,42,40,36,5,0,37,38,45,
52,42,5,0,128,
490,489,488,487,486,485,484,417,416,415,326,323,232,133,64,0,65,67,125,126,
  130,
495,494,491,489,488,487,486,485,484,417,416,415,326,323,277,232,229,228,64,
  56,49,47,36,0,
483,52,42,40,5,0,37,38,45,
54,52,42,5,0,53,201,275,
54,52,42,5,0,53,201,275,
54,52,42,5,0,53,201,275,
54,52,42,5,0,53,201,275,
54,52,42,5,0,53,201,275,
54,52,42,5,0,53,201,275,
490,489,488,487,486,485,484,417,416,415,332,327,326,323,321,232,64,0,65,67,
  151,317,318,319,320,
490,489,488,487,486,485,484,483,417,416,415,332,327,326,323,321,232,64,40,0,
  37,38,45,
52,0,51,
490,489,488,487,486,485,484,417,416,415,326,323,232,198,196,192,191,64,62,
  58,0,57,80,179,180,189,193,197,203,204,205,206,207,302,304,315,
486,483,326,323,40,0,37,38,45,
483,40,0,37,45,
486,326,323,0,
36,0,50,
490,489,488,487,486,485,484,417,416,415,326,323,309,232,64,0,65,67,151,308,
  310,
490,489,488,487,486,485,484,483,417,416,415,326,323,309,232,64,40,0,37,38,
  45,
52,0,51,
487,486,415,326,323,0,
486,326,323,0,
232,0,
495,491,489,488,487,486,485,484,417,416,415,326,323,277,232,229,228,64,56,
  36,0,268,
495,491,489,488,487,486,485,484,483,417,416,415,326,323,277,232,229,228,64,
  56,40,36,0,37,38,45,
47,0,46,
47,0,46,
47,0,46,
490,489,488,487,486,485,484,483,417,416,415,399,326,323,232,64,62,60,54,52,
  40,5,0,65,67,266,
264,263,262,0,
47,0,46,
47,0,46,
47,0,46,
47,0,46,
47,0,46,
495,491,489,488,487,486,485,484,483,417,416,415,399,326,323,277,232,229,228,
  64,62,60,56,54,52,49,42,40,36,5,0,279,283,284,285,
495,494,491,489,488,487,486,485,484,417,416,415,326,323,277,232,229,228,64,
  56,54,52,49,47,42,36,0,
497,496,495,494,493,492,491,490,489,488,487,486,485,484,483,417,416,415,326,
  323,277,232,229,228,86,64,62,60,58,56,54,52,49,47,42,40,36,0,77,
490,489,488,487,486,485,484,425,424,417,416,415,326,323,232,64,62,0,61,448,
62,0,479,
62,0,80,401,450,
62,0,80,401,
487,486,415,326,323,0,452,453,458,
478,477,451,449,421,0,437,438,472,473,474,475,476,
47,0,46,113,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,64,40,0,37,38,45,
47,0,46,
490,489,488,487,486,485,484,418,417,416,415,326,323,232,64,0,65,67,413,
415,0,411,
483,62,42,40,5,0,408,
47,0,46,
47,0,46,
47,0,46,
47,0,46,
483,47,40,0,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,64,62,40,0,305,306,
  307,
47,0,46,
47,0,46,
47,0,46,
47,0,46,
47,0,46,
490,489,488,487,486,485,484,483,467,459,454,451,449,445,443,421,417,416,415,
  409,402,400,386,326,323,232,64,40,0,37,38,45,
47,0,46,
47,0,46,
47,0,46,
47,0,46,
47,0,46,389,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,64,47,40,0,67,385,
490,489,488,487,486,485,484,483,428,426,421,417,416,415,409,404,402,400,388,
  386,326,323,232,64,40,0,37,38,45,
60,0,59,
483,60,40,0,37,38,45,
490,489,488,487,486,485,484,417,416,415,326,323,232,64,0,65,67,112,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,64,40,0,37,38,45,
490,489,488,487,486,485,484,417,416,415,326,323,232,64,0,65,67,131,
36,0,50,
52,0,51,
483,52,47,42,40,5,0,37,38,45,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
483,52,42,40,5,0,37,38,45,
483,52,42,40,5,0,37,38,45,
483,52,42,40,5,0,37,38,45,
490,489,488,487,486,485,484,417,416,415,332,327,326,323,321,232,64,0,65,67,
  151,317,318,319,320,
486,326,323,0,337,347,
486,326,323,0,349,
486,326,323,0,363,
483,47,40,0,37,38,45,
483,52,40,5,0,37,38,45,
490,489,488,487,486,485,484,417,416,415,326,323,309,232,64,0,65,67,151,308,
  310,
486,326,323,0,
486,326,323,0,231,
495,491,489,488,487,486,485,484,417,416,415,326,323,277,232,229,228,64,56,
  36,0,273,
489,488,487,486,483,417,416,415,326,323,232,228,64,40,0,37,38,45,
495,491,489,488,487,486,485,484,483,417,416,415,326,323,277,232,229,228,64,
  56,40,36,0,37,38,45,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,168,64,40,0,37,38,
  45,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,96,95,94,93,92,64,
  40,0,37,38,45,
489,488,487,486,483,417,416,415,326,323,232,228,64,40,0,37,38,45,
486,483,326,323,40,0,37,38,45,
490,489,488,487,486,485,484,483,417,416,415,326,323,259,258,232,64,40,0,37,
  38,45,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,138,137,136,135,64,
  40,0,37,38,45,
495,491,489,488,487,486,485,484,417,416,415,326,323,232,229,228,64,56,54,49,
  36,0,283,
490,489,488,487,486,485,484,425,424,417,416,415,326,323,232,64,0,65,67,423,
487,486,415,326,323,62,0,453,458,480,481,482,
487,486,415,326,323,0,453,458,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,64,62,40,0,37,38,45,
490,489,488,487,486,485,484,417,416,415,326,323,232,64,0,65,67,430,
483,62,40,0,37,45,61,
416,0,
486,326,323,0,100,412,
483,62,40,0,37,38,45,
490,489,488,487,486,485,484,483,425,424,417,416,415,326,323,232,64,62,40,0,
  37,38,45,
483,469,468,40,0,37,38,45,
483,62,40,0,37,38,45,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,64,62,40,0,305,306,
  307,
490,489,488,487,486,485,484,417,416,415,326,323,232,64,62,0,67,
483,62,40,0,37,38,45,
483,62,40,0,37,38,45,
483,62,40,0,37,38,45,
483,62,40,0,37,38,45,
483,62,40,0,37,38,45,
490,489,488,487,486,485,484,467,459,454,451,449,445,443,421,417,416,415,409,
  402,400,386,326,323,232,64,0,67,375,377,378,381,383,406,434,435,436,437,
  438,439,440,441,442,
483,62,40,0,37,38,45,
483,405,263,40,0,37,38,45,
62,0,427,
62,0,427,
483,62,40,0,37,38,45,390,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,64,47,40,0,305,306,
  307,
490,489,488,487,486,485,484,428,426,421,417,416,415,409,404,402,400,388,386,
  326,323,232,64,0,67,374,375,376,377,378,379,380,381,382,383,384,406,
60,0,59,
483,52,47,42,40,5,0,37,38,45,
490,489,488,487,486,485,484,417,416,415,326,323,232,64,0,65,67,112,
483,56,40,0,37,38,45,
486,483,326,323,40,0,37,38,45,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,169,166,165,163,162,
  160,156,154,153,64,40,0,37,38,45,
47,0,46,
47,0,46,
47,0,46,
47,0,46,
483,40,0,37,45,
486,326,323,0,
486,326,323,0,
36,0,50,
47,0,46,
232,0,
489,488,487,486,417,416,415,326,323,232,228,64,0,140,152,224,
495,491,489,488,487,486,485,484,417,416,415,326,323,277,232,229,228,64,56,
  36,0,170,
490,489,488,487,486,485,484,417,416,415,326,323,232,168,64,0,65,67,167,
490,489,488,487,486,485,484,417,416,415,326,323,232,96,95,94,93,92,64,0,65,
  67,91,
489,488,487,486,417,416,415,326,323,232,228,64,0,140,152,224,
486,326,323,0,157,
490,489,488,487,486,485,484,417,416,415,326,323,259,258,232,64,0,65,67,256,
  257,
490,489,488,487,486,485,484,417,416,415,326,323,232,138,137,136,135,64,0,65,
  67,134,
483,62,42,40,5,0,408,
487,486,415,326,323,0,453,458,480,
62,0,
487,486,415,326,323,0,453,458,
490,489,488,487,486,485,484,417,416,415,326,323,232,64,62,0,65,67,80,
62,0,61,
415,0,
486,326,323,0,
64,0,63,
62,0,446,447,
490,489,488,487,486,485,484,425,424,417,416,415,326,323,232,64,62,0,61,448,
469,468,0,
62,0,460,
490,489,488,487,486,485,484,417,416,415,326,323,232,64,62,0,67,
497,496,495,494,493,492,491,490,489,488,487,486,485,484,483,417,416,415,326,
  323,277,232,229,228,86,64,62,60,58,56,54,52,49,47,42,40,36,5,0,81,82,83,
  84,
490,489,488,487,486,485,484,417,416,415,326,323,232,64,0,
62,0,80,403,
62,0,455,
62,0,80,401,
62,0,80,387,
62,0,80,444,
483,42,40,5,0,37,38,45,
62,0,61,
405,263,0,
487,486,415,326,323,62,0,88,90,429,
62,0,
47,0,
483,42,40,5,0,37,38,45,
47,0,46,113,
56,0,55,
486,326,323,0,100,139,
490,489,488,487,486,485,484,417,416,415,326,323,232,169,166,165,163,162,160,
  156,154,153,64,0,65,67,129,141,142,143,144,145,146,147,148,149,150,151,
  155,158,161,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,228,64,62,40,0,37,
  38,45,
486,483,326,323,62,40,0,37,38,45,
483,330,329,40,0,37,38,45,
483,326,323,40,0,37,38,45,
486,326,323,0,338,362,
486,326,323,0,
486,326,323,0,364,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,64,40,0,37,38,45,
486,326,323,0,231,
495,491,489,488,487,486,485,484,483,417,416,415,399,326,323,277,232,229,228,
  64,62,60,56,54,52,40,36,5,0,268,269,270,
486,326,323,0,
487,486,415,326,323,0,453,458,
483,52,42,40,5,0,37,38,45,
483,52,42,40,5,0,37,38,45,
417,0,
490,489,488,487,486,485,484,418,417,416,415,326,323,232,64,0,65,67,413,
488,487,486,485,417,416,415,326,323,232,198,196,192,191,64,0,172,173,174,
  175,178,186,187,188,189,193,197,
490,489,488,487,486,485,484,425,424,417,416,415,326,323,232,64,0,65,67,423,
488,487,486,485,417,416,415,326,323,56,0,462,
497,496,495,494,493,492,491,490,489,488,487,486,485,484,483,417,416,415,326,
  323,277,232,229,228,86,64,62,60,58,56,54,52,49,47,42,40,36,5,0,81,82,83,
  84,
490,489,488,487,486,485,484,417,416,415,326,323,232,64,0,
62,0,
487,486,483,415,326,323,62,40,0,37,38,45,453,456,458,
490,489,488,487,486,485,484,425,424,417,416,415,326,323,232,64,0,65,67,422,
  423,
487,486,415,326,323,0,
62,0,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,229,228,192,191,64,
  62,40,36,0,45,391,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,64,62,40,0,305,306,
  307,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,138,137,136,135,64,
  40,0,37,38,45,
483,52,42,40,5,0,37,38,45,
483,52,47,42,40,5,0,37,38,45,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
52,47,42,5,0,46,159,
483,47,40,0,37,38,45,
483,47,40,0,37,38,45,
483,52,42,40,5,0,37,38,45,
483,52,42,40,5,0,37,38,45,
490,489,488,487,486,485,484,417,416,415,326,323,232,228,64,62,0,65,67,80,
  152,
486,326,323,62,0,61,100,333,334,
330,329,0,328,
326,323,0,322,
486,326,323,0,
486,326,323,0,
483,40,0,37,45,
490,489,488,487,486,485,484,417,416,415,326,323,232,64,0,65,67,
486,326,323,0,
487,486,415,326,323,0,453,458,
62,0,
483,62,42,40,5,0,408,
488,487,486,485,466,465,417,416,415,326,323,62,56,0,463,464,
62,0,
62,0,
487,486,415,326,323,0,453,458,
62,0,
483,62,42,40,0,37,38,45,
483,40,0,
490,489,488,487,486,485,484,417,416,415,326,323,232,229,228,192,191,64,62,
  36,0,186,189,392,393,394,395,397,
490,489,488,487,486,485,484,417,416,415,326,323,232,64,62,0,67,
490,489,488,487,486,485,484,417,416,415,326,323,232,138,137,136,135,64,0,65,
  67,132,134,
47,0,46,
47,0,46,
47,0,46,
47,0,46,
486,483,326,323,40,0,37,38,45,
47,0,46,
47,0,46,
483,52,42,40,5,0,37,38,45,
483,52,42,40,5,0,37,38,45,
483,52,42,40,5,0,37,38,45,
483,346,345,344,343,342,341,340,40,0,37,38,45,
483,232,52,42,40,5,0,325,
483,232,52,42,40,5,0,325,
368,367,366,0,365,
487,486,415,326,323,0,453,458,
62,0,
487,486,415,326,323,0,453,458,
42,0,41,
490,489,488,487,486,485,484,417,416,415,326,323,232,229,228,64,36,0,
399,0,398,
483,62,40,0,45,391,
497,496,495,494,493,492,491,490,489,488,487,486,485,484,483,417,416,415,326,
  323,277,232,229,228,86,64,62,60,58,56,54,52,49,47,42,40,36,5,0,81,82,83,
  84,
490,489,488,487,486,485,484,417,416,415,326,323,232,64,0,
489,488,487,486,483,417,416,415,326,323,232,228,64,40,0,37,38,45,
486,483,326,323,40,0,37,38,45,
483,62,40,0,37,38,45,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,168,64,40,0,37,38,
  45,
486,326,323,0,100,139,
490,489,488,487,486,485,484,483,417,416,415,326,323,232,64,40,0,37,38,45,
487,486,483,415,326,323,235,40,0,37,38,45,
346,345,344,343,342,341,340,0,335,339,
486,483,326,323,52,42,40,5,0,324,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
490,489,488,487,486,485,484,483,425,424,417,416,415,326,323,232,64,40,0,37,
  38,45,
192,191,0,186,189,
62,0,
62,0,
489,488,487,486,417,416,415,326,323,232,228,64,0,140,152,224,
486,326,323,0,157,
62,0,61,
490,489,488,487,486,485,484,417,416,415,326,323,232,168,64,0,65,67,167,
490,489,488,487,486,485,484,417,416,415,326,323,232,64,0,65,67,
487,486,415,326,323,235,0,164,171,233,234,237,
483,62,40,0,37,38,45,
486,326,323,0,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
490,489,488,487,486,485,484,425,424,417,416,415,326,323,232,64,0,65,67,423,
483,52,42,40,5,0,37,38,45,
483,52,42,40,5,0,37,38,45,
495,491,489,488,487,486,485,484,417,416,415,326,323,277,232,229,228,64,56,
  36,0,170,
483,52,42,40,5,0,37,38,45,
483,52,42,40,5,0,37,38,45,
483,52,42,40,5,0,37,38,45,
62,0,
486,326,323,0,
487,486,415,326,323,0,453,458,
62,0,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
487,486,415,326,323,0,453,458,
};
static unsigned const char ag_astt[6464] = {
  1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,7,0,1,1,1,1,1,1,2,1,2,1,2,2,2,1,1,1,2,1,9,9,9,9,
  9,9,9,9,9,9,9,9,9,9,4,2,2,7,3,1,1,1,1,2,2,2,7,2,1,1,1,1,1,2,2,2,7,2,1,1,1,
  1,1,1,1,8,7,1,1,1,1,2,7,1,1,3,7,3,7,3,7,3,7,1,7,1,3,7,3,7,1,7,1,1,7,1,1,7,
  1,1,7,1,1,7,1,1,7,1,1,7,1,1,7,1,1,7,1,1,7,1,1,7,1,1,7,1,1,1,1,1,1,1,1,1,1,
  1,1,1,7,1,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,7,1,1,1,2,1,
  4,4,4,4,4,2,4,4,7,1,1,1,4,4,4,4,4,2,4,4,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,2,
  1,1,1,1,1,1,1,1,1,1,1,1,1,7,2,1,1,9,9,4,2,7,1,4,4,4,4,5,4,4,4,4,4,4,4,4,4,
  4,4,5,5,7,1,1,1,1,1,1,1,1,1,1,2,2,2,2,1,7,1,1,2,1,1,2,2,2,1,1,1,1,1,1,2,7,
  3,1,1,5,5,5,5,5,5,5,5,5,5,1,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
  5,5,5,5,5,5,5,5,5,5,5,1,5,7,2,2,1,1,1,1,1,1,1,1,1,1,1,1,7,1,2,1,1,1,1,1,1,
  1,7,1,2,2,2,1,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,2,2,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,7,1,1,1,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,7,1,1,1,1,1,
  1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,7,1,2,1,1,2,1,1,1,1,1,7,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,2,1,2,2,2,2,1,1,2,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,2,1,2,2,2,2,2,
  2,2,1,1,7,1,2,1,1,1,1,1,2,2,2,2,2,2,2,1,1,7,1,1,1,2,2,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,2,2,2,2,1,1,2,7,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,7,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,1,1,1,1,4,1,1,1,1,1,1,1,7,1,1,1,1,2,
  4,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,2,2,2,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9,9,9,9,4,2,4,1,2,7,1,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,1,8,7,1,1,1,2,4,1,4,4,4,4,2,4,4,7,1,2,1,4,4,4,4,2,4,4,7,1,2,1,9,9,
  9,9,9,9,9,9,9,9,9,4,2,7,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,1,8,5,7,1,1,1,2,7,1,1,1,1,1,7,2,4,1,3,7,1,8,1,7,1,1,
  1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,5,7,1,1,1,
  3,7,1,1,7,1,1,1,1,7,1,1,3,7,1,1,7,1,1,1,1,7,1,1,3,7,5,5,5,5,5,5,5,1,5,5,5,
  5,5,5,5,5,1,5,7,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  8,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,4,1,5,1,5,7,
  2,2,1,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,2,7,1,1,8,5,1,5,7,1,1,1,2,
  5,1,3,7,1,8,1,7,1,1,1,1,1,7,1,1,2,5,1,3,7,9,9,9,4,1,1,5,7,2,2,1,3,7,9,9,9,
  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,4,2,4,1,1,1,5,7,2,2,1,3,7,2,
  7,1,2,2,2,7,1,1,1,1,1,2,2,2,7,1,1,1,1,1,2,2,7,1,1,1,1,2,4,1,2,4,1,1,1,8,5,
  1,5,7,1,1,1,2,5,1,3,7,2,2,2,4,1,8,1,7,1,1,1,1,1,7,1,1,1,1,7,1,1,1,1,5,7,2,
  2,1,3,7,9,9,9,4,1,1,5,7,2,2,1,3,7,2,4,1,1,1,8,1,5,7,1,1,1,3,7,9,9,9,4,1,1,
  7,1,1,1,1,5,7,2,2,1,3,7,9,9,9,9,9,9,9,9,9,9,9,4,1,1,1,1,1,5,5,5,5,5,5,5,5,
  1,5,7,1,1,3,1,1,5,1,1,1,5,5,5,5,5,5,7,3,3,1,1,5,1,5,2,7,1,1,1,7,1,2,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,5,1,1,7,2,2,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,7,1,2,
  2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,2,2,2,2,
  2,2,2,2,2,2,2,2,1,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,1,8,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,2,1,1,1,4,8,8,8,
  8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,8,7,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,4,2,2,2,7,3,1,2,2,2,
  7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,5,
  7,2,1,1,2,2,7,1,2,7,1,2,7,1,2,7,1,2,7,1,2,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,2,2,2,2,
  2,1,1,1,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,2,2,2,2,2,2,2,2,2,2,1,9,9,5,3,3,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  7,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,1,9,9,9,9,9,9,9,9,9,9,9,
  9,5,3,3,2,7,5,5,5,5,5,5,5,1,5,5,5,5,5,5,5,5,1,5,7,1,1,2,1,1,1,1,1,1,1,1,1,
  1,2,2,2,2,1,7,1,2,1,1,2,2,2,1,1,1,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,
  1,2,4,1,1,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,1,7,1,1,1,2,7,1,1,1,1,1,1,1,1,
  1,1,1,1,1,7,1,1,2,2,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,1,7,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,2,3,1,1,8,5,5,
  1,5,7,1,1,1,1,8,5,5,1,5,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,2,3,1,
  1,8,5,5,1,5,7,1,1,1,1,8,5,5,1,5,7,1,1,1,1,8,5,5,1,5,7,1,1,1,1,8,5,5,1,5,7,
  1,1,1,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,2,4,1,2,4,1,1,8,8,8,8,
  8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,2,7,1,2,2,2,2,2,2,2,2,2,2,2,2,
  7,1,1,1,1,7,2,1,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,1,7,1,1,1,2,4,1,2,4,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,1,7,2,1,2,1,1,1,1,1,5,1,1,1,1,1,7,3,1,5,
  3,1,1,5,1,1,1,5,5,5,5,5,5,7,3,3,1,1,1,1,7,3,1,9,9,9,4,9,9,9,9,9,9,9,9,9,9,
  9,9,9,9,9,9,9,9,5,3,1,1,1,1,1,1,1,1,5,1,1,1,5,1,1,1,1,1,1,1,5,5,1,5,5,5,5,
  1,5,7,1,1,2,2,4,1,2,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,8,8,8,8,8,8,
  8,5,8,8,8,5,8,8,8,8,5,5,5,5,2,5,5,7,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,
  7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,7,2,2,2,2,2,2,2,2,2,2,2,2,1,9,9,9,9,9,9,9,9,9,9,9,9,
  9,9,9,9,9,9,9,9,4,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,2,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,2,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,5,7,1,1,2,1,1,5,7,2,
  2,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  5,3,5,5,5,5,5,5,5,1,5,5,5,5,5,5,5,5,5,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,
  1,1,1,8,1,7,1,1,1,8,8,1,8,8,8,1,7,1,1,1,1,8,8,8,8,8,1,7,1,1,1,1,8,1,7,1,1,
  1,2,4,1,1,8,1,7,1,1,1,2,2,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,
  1,1,8,1,7,1,1,1,9,9,9,9,9,9,9,1,9,9,9,9,9,9,9,1,1,7,1,1,8,1,7,1,1,1,1,8,1,
  7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,5,1,5,7,2,2,1,2,4,1,2,
  4,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,
  9,9,9,9,9,9,9,1,9,9,9,9,9,9,9,1,1,7,1,1,5,1,5,7,2,2,1,2,4,1,1,8,1,7,1,1,1,
  1,1,1,1,1,1,1,1,1,1,2,2,2,2,1,7,1,2,1,1,2,2,2,1,1,1,8,8,8,8,8,8,8,1,8,8,8,
  8,8,8,8,1,7,1,1,1,2,4,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,7,1,1,1,1,1,2,8,8,
  8,8,8,8,8,1,8,8,8,8,8,8,8,1,7,1,1,1,1,5,5,1,8,5,7,1,1,1,4,4,4,7,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,2,1,7,2,1,2,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9,9,9,4,1,5,5,1,5,7,2,2,1,2,4,4,4,7,1,2,1,2,4,4,4,7,1,2,1,2,4,4,4,7,1,2,1,
  2,4,4,4,7,1,2,1,2,4,4,4,7,1,2,1,2,4,4,4,7,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,7,1,1,2,3,1,1,1,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,2,4,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,1,1,2,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,
  8,1,8,8,1,7,1,1,1,1,1,7,1,1,2,2,2,7,2,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,
  1,1,2,1,2,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,1,7,1,1,1,2,4,1,3,3,3,3,3,5,1,1,
  1,5,1,7,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,5,3,8,8,8,8,8,8,8,8,1,8,8,
  8,8,8,8,8,8,8,8,8,1,8,7,1,1,1,2,7,1,2,7,1,2,7,1,1,1,1,1,1,1,1,5,1,1,1,5,1,
  1,1,1,5,5,5,5,5,5,7,2,1,2,2,2,2,5,2,7,1,2,7,1,2,7,1,2,7,1,2,7,1,1,1,1,1,1,
  1,1,1,5,1,1,1,5,1,1,5,1,1,1,1,5,5,1,1,5,1,5,5,1,5,7,2,1,1,2,9,9,9,9,9,9,9,
  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,4,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,5,3,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,2,7,1,1,1,7,2,1,7,2,2,2,1,7,2,2,1,1,1,1,1,7,2,1,1,1,1,1,1,1,7,2,2,3,1,2,
  2,2,2,7,1,2,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,1,7,1,1,1,2,7,1,1,1,1,1,1,1,1,2,
  1,1,1,1,1,1,1,7,2,1,2,1,7,1,5,2,5,5,5,7,2,2,7,1,2,7,1,2,7,1,2,7,1,9,1,9,7,
  8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,1,7,1,1,1,2,7,1,2,7,1,2,7,1,2,7,1,2,7,1,8,
  8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,2,7,1,2,7,1,
  2,7,1,2,7,1,2,7,2,1,1,1,1,1,1,1,1,9,1,1,1,1,1,1,1,1,9,7,1,2,8,8,8,8,8,8,8,
  1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,2,7,2,1,8,1,7,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,7,1,1,3,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,1,7,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,7,2,1,1,2,4,1,2,4,1,1,5,8,5,1,5,7,1,1,1,1,8,1,7,1,1,1,
  1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,5,5,1,5,7,2,2,1,1,5,5,1,5,7,2,2,1,1,5,5,1,5,
  7,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,2,3,1,1,1,1,1,1,7,1,1,1,1,
  1,7,2,1,1,1,7,1,1,8,1,7,1,1,1,1,5,1,5,7,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,7,1,1,2,1,2,3,3,3,5,1,1,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,
  2,8,8,8,8,1,8,8,8,8,8,8,8,8,1,7,1,1,1,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,
  8,8,1,8,7,1,1,1,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,1,7,1,1,1,8,8,8,8,8,8,8,1,
  8,8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,8,8,8,8,1,8,8,8,8,8,8,8,8,1,7,1,1,1,8,1,
  8,8,1,7,1,1,1,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,1,7,1,1,1,8,8,8,8,8,8,8,1,
  8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  5,3,1,1,1,1,1,1,1,2,2,1,1,1,1,1,1,1,7,2,1,1,1,1,1,1,1,8,7,1,1,1,1,1,1,1,1,
  1,1,4,1,1,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,7,1,1,3,1,2,1,4,1,1,2,1,7,1,1,1,7,1,1,1,8,1,7,1,1,1,5,5,5,5,5,5,5,1,
  5,5,5,5,5,5,5,5,5,8,1,7,1,1,1,1,8,8,1,7,1,1,1,1,8,1,7,1,1,1,8,8,8,8,8,8,8,
  1,8,8,8,8,8,8,8,8,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,8,1,7,1,1,
  1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,1,8,
  1,7,1,1,1,1,8,8,1,7,1,1,1,1,7,2,1,7,2,1,5,1,7,2,2,1,1,9,9,9,9,9,9,9,1,9,9,
  9,9,9,9,9,8,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,
  2,2,2,2,2,2,2,2,2,2,1,2,7,2,1,5,8,5,1,5,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,7,1,1,3,1,8,1,7,1,1,1,8,1,8,8,1,7,1,1,1,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,1,7,1,1,1,2,4,1,2,7,1,2,7,1,2,7,1,1,1,7,1,1,1,1,1,7,2,2,2,
  7,2,7,1,2,7,1,1,7,1,1,1,1,1,1,1,1,1,1,1,1,7,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,7,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,7,2,1,2,1,1,1,1,1,1,1,1,
  1,1,1,1,1,2,2,2,2,2,1,7,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,7,2,2,2,1,1,1,7,2,1,
  1,1,1,1,1,1,1,1,1,1,1,2,2,1,1,7,2,1,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,
  1,7,2,1,2,5,2,5,5,5,7,2,9,9,9,9,9,5,3,3,3,2,7,1,1,1,1,1,4,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,7,1,1,1,2,4,2,1,7,9,9,9,4,2,7,1,2,7,3,1,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,2,7,1,1,2,2,7,1,7,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,8,1,1,1,1,1,1,1,1,1,1,1,7,
  1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,4,1,7,2,2,1,7,2,1,7,2,2,1,7,2,2,1,7,2,
  2,1,5,1,5,7,2,2,1,2,7,1,2,2,7,1,1,1,1,1,8,7,1,1,1,1,7,1,7,1,5,1,5,7,2,2,1,
  2,4,1,2,2,7,1,1,1,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,
  1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,1,7,1,
  1,1,8,1,8,8,8,1,7,1,1,1,1,8,8,1,7,1,1,1,1,8,8,1,7,1,1,1,1,1,1,7,2,1,1,1,1,
  4,1,1,1,7,1,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,1,7,1,1,1,1,1,1,7,3,1,1,1,1,1,1,
  1,1,5,1,1,1,5,1,1,1,1,1,1,1,5,5,1,5,5,5,1,5,7,1,1,2,1,1,1,4,1,1,1,1,1,4,1,
  1,1,5,5,1,5,7,2,2,1,1,5,5,1,5,7,2,2,1,2,7,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,7,
  2,1,2,1,1,1,1,1,1,1,1,1,1,2,2,2,2,1,7,1,1,2,1,1,2,2,2,1,1,1,1,1,1,1,1,1,1,
  2,2,1,1,1,1,1,1,1,7,2,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,8,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,1,9,9,9,9,9,9,
  9,9,9,9,9,9,9,9,4,2,7,1,1,1,1,1,1,8,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,1,1,
  1,1,1,1,1,7,2,1,1,2,9,9,9,9,9,4,2,7,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,
  8,1,8,7,1,1,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,1,7,1,1,1,8,8,8,8,8,8,8,1,8,8,
  8,8,8,8,8,8,8,8,8,1,7,1,1,1,1,5,5,1,5,7,2,2,1,1,5,5,5,1,5,7,3,3,1,1,8,1,7,
  1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,4,2,4,4,7,1,2,1,8,1,7,1,1,
  1,1,8,1,7,1,1,1,1,5,5,1,5,7,3,3,1,1,5,5,1,5,7,3,3,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,7,1,1,1,1,1,1,1,2,7,1,1,3,2,2,2,7,2,1,1,7,2,2,2,2,4,2,2,2,7,1,1,
  7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,2,1,2,2,2,4,1,1,1,1,1,4,1,1,2,7,5,2,5,
  5,5,7,2,9,9,9,9,1,1,9,9,9,9,9,8,9,7,1,1,2,7,2,7,1,1,1,1,1,7,1,1,2,7,1,2,8,
  1,7,1,1,1,9,9,5,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,1,2,1,7,1,1,1,1,1,2,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,1,7,2,1,
  2,2,2,7,1,2,7,1,2,7,1,2,7,1,8,1,8,8,1,7,1,1,1,2,7,1,2,7,1,1,5,5,1,5,7,2,2,
  1,1,5,5,1,5,7,2,2,1,1,5,5,1,5,7,2,2,1,1,8,8,8,8,8,8,8,1,7,1,1,1,5,1,5,5,5,
  5,7,2,5,1,5,5,5,5,7,2,2,2,2,7,2,1,1,1,1,1,4,1,1,2,7,1,1,1,1,1,7,1,1,2,7,1,
  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,4,1,5,1,1,8,1,7,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,8,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,1,9,9,9,9,
  9,9,9,9,9,9,9,9,9,9,5,8,8,8,8,1,8,8,8,8,8,8,8,8,1,7,1,1,1,8,1,8,8,1,7,1,1,
  1,1,8,1,7,1,1,1,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,1,7,1,1,1,1,1,1,7,1,2,8,8,
  8,8,8,8,8,1,8,8,8,8,8,8,8,1,7,1,1,1,8,8,1,8,8,8,8,1,7,1,1,1,2,2,2,2,2,2,2,
  7,1,1,1,5,1,1,5,5,5,5,7,3,1,1,1,1,1,4,1,1,1,1,1,1,1,7,1,1,8,8,8,8,8,8,8,1,
  8,8,8,8,8,8,8,8,8,1,7,1,1,1,2,2,7,3,1,2,7,3,7,1,1,1,1,1,1,1,1,1,1,1,1,7,1,
  2,2,1,1,1,7,1,2,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,7,2,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,7,1,2,1,1,1,1,8,1,7,1,1,1,1,1,1,5,2,2,2,2,2,
  4,2,2,1,1,1,1,1,7,1,1,1,1,1,1,1,1,1,2,2,1,1,1,1,1,1,1,7,2,1,2,1,5,5,1,5,7,
  2,2,1,1,5,5,1,5,7,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,5,5,
  1,5,7,2,2,1,1,5,5,1,5,7,2,2,1,1,5,5,1,5,7,2,2,1,2,7,3,3,3,5,1,1,1,1,1,7,1,
  1,2,7,1,1,1,1,1,7,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,7,1,1,1,1,
  1,1,1,7,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,7,1,
  1,1,1,1,1,1,7,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,7,1,1,1,1,1,1,
  1,7,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,7,1,1,1,
  1,1,1,1,7,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,7,
  1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,7,1,1,3,3,3,3,3,7,3,3
};
static const unsigned short ag_pstt[] = {
1,1,1,1,1,1,1,1,1,1,1,1,328,327,326,325,324,323,322,321,320,319,318,317,316,
  1,190,187,183,182,102,69,68,67,66,65,1,11,14,15,16,17,18,19,20,21,22,23,
  24,25,0,0,13,12,10,9,8,7,70,1,329,6,178,179,180,2,4,3,314,5,
40,40,40,40,40,40,40,40,40,40,40,40,40,40,41,
223,208,2,181,29,28,27,26,
223,208,186,3,189,30,29,28,27,26,
223,208,186,4,185,31,29,28,27,26,
32,34,33,5,33,33,35,32,
23,6,36,36,
19,7,
18,8,
17,9,
16,10,
37,11,38,
14,12,
13,13,
37,14,39,
37,15,40,
37,16,41,
37,17,42,
37,18,43,
37,19,44,
37,20,45,
37,21,46,
37,22,47,
37,23,48,
37,24,49,
37,25,50,
51,52,52,52,52,52,52,52,52,51,53,51,26,54,227,222,226,
56,56,56,56,56,56,56,56,56,56,56,56,56,56,56,56,56,56,56,56,30,56,56,27,55,
  58,57,211,56,
254,254,254,254,254,32,254,254,28,59,61,60,
254,254,254,254,254,32,254,254,29,59,62,60,
63,63,63,63,63,63,63,63,63,63,63,30,188,64,63,
63,63,63,63,63,63,63,63,63,63,63,31,184,64,63,
27,27,28,
31,33,65,
23,23,23,23,26,23,23,23,23,23,23,23,23,23,23,23,26,26,34,
63,63,63,63,63,63,63,63,63,63,190,187,183,182,63,35,66,66,333,64,63,330,331,
  332,2,4,3,
67,67,67,23,36,99,68,68,
20,20,20,20,20,20,20,20,20,20,32,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
  20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,32,20,37,22,
  22,32,
63,63,63,63,63,63,63,63,63,63,63,38,69,166,64,63,
71,73,74,72,70,39,76,567,565,562,75,566,568,569,
1,1,1,1,1,1,1,1,1,1,78,1,1,1,1,40,79,77,1,503,503,
1,1,1,1,1,1,1,1,1,1,81,1,1,1,1,41,82,80,1,428,428,
83,83,83,83,83,83,83,83,83,83,83,83,83,83,84,35,42,91,87,86,88,89,85,90,558,
1,1,1,1,1,1,1,1,1,1,1,1,1,119,1,43,95,118,1,94,112,93,92,
96,96,96,44,98,96,97,
99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
  99,45,102,100,99,101,
83,83,83,83,83,83,83,83,83,83,83,83,356,83,190,187,183,182,83,84,35,46,111,
  87,86,103,107,106,105,104,108,108,108,108,108,109,85,111,356,110,359,
112,400,399,398,397,396,395,394,112,112,47,117,392,116,115,113,114,
118,400,399,398,397,396,395,394,118,118,48,120,118,119,387,388,113,
83,83,83,83,83,83,83,83,83,83,83,83,83,190,187,183,182,83,84,35,49,123,87,
  86,103,107,106,105,104,121,121,121,121,121,334,122,85,
124,124,124,50,127,126,125,124,
128,128,128,128,128,128,128,128,128,128,128,51,128,
128,128,128,128,128,128,128,128,128,128,128,228,128,
129,129,129,129,129,131,53,134,133,132,130,
31,224,135,
51,52,52,52,52,52,52,52,52,51,53,51,55,54,227,207,226,
213,213,213,213,213,213,213,213,213,213,213,213,213,213,213,213,213,213,213,
  213,213,213,214,
31,209,136,
30,58,137,
138,138,138,138,138,138,138,138,32,138,138,138,138,138,138,138,138,138,138,
  138,138,138,138,138,138,138,138,138,138,138,32,138,59,138,138,32,
32,255,139,
300,300,300,300,33,300,300,61,140,192,141,
300,300,300,300,33,300,300,62,140,191,141,
169,169,169,169,169,169,169,169,169,169,169,170,
31,64,142,
143,143,143,143,143,143,143,143,143,143,143,143,143,143,32,143,143,143,143,
  143,143,143,143,143,143,143,143,143,143,143,143,143,143,143,143,32,143,
  20,65,143,143,32,
23,66,144,144,
145,145,145,67,
23,101,146,
15,69,
32,147,32,70,147,147,32,
32,148,32,71,148,148,32,
32,149,32,72,149,149,32,
32,150,32,73,150,150,32,
32,151,32,74,151,151,32,
32,152,32,563,75,152,152,32,
12,76,
32,32,77,153,32,
32,32,78,154,32,
11,79,
32,32,80,155,32,
32,32,81,156,32,
10,82,
346,346,346,346,346,346,346,157,346,346,346,346,346,346,346,346,157,346,83,
  157,157,349,
159,159,159,159,159,159,159,159,159,159,159,159,159,159,159,159,159,159,159,
  159,159,159,159,159,158,159,160,159,159,159,159,159,159,159,159,159,159,
  159,84,159,159,159,160,
161,161,161,161,161,161,161,161,161,161,161,161,161,161,342,
32,20,32,20,86,341,341,32,
162,162,162,162,32,162,162,162,162,162,162,162,162,162,162,162,32,87,162,
  162,32,
35,88,163,
32,164,20,32,20,89,164,164,32,
25,557,165,
9,91,
32,166,32,92,166,166,32,
32,32,93,167,32,
25,111,168,
8,95,
74,74,74,77,
32,32,20,97,76,76,32,
7,98,
43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,43,
  43,44,
30,79,169,
32,32,20,101,78,78,32,
6,102,
31,103,170,
223,208,186,104,171,173,172,27,26,
223,208,186,105,174,176,175,27,26,
223,208,106,178,177,27,26,
35,171,163,
32,363,179,180,
32,181,20,32,20,109,181,181,32,
25,357,182,
5,111,
403,403,403,402,
32,183,32,113,183,183,32,
32,32,114,184,32,
32,32,115,185,32,
32,32,20,116,390,390,32,
4,117,
74,74,74,389,
32,32,20,119,383,383,32,
3,120,
32,337,186,187,
32,188,32,20,122,188,188,32,
2,123,
74,74,74,75,
32,32,125,189,32,
32,32,20,126,71,71,32,
1,127,
230,230,230,230,230,230,230,230,230,230,230,231,
190,190,190,190,190,250,
247,247,247,247,247,247,247,191,247,130,192,192,249,
129,129,240,129,129,129,240,240,240,240,240,240,131,243,243,130,
193,239,
194,237,
232,134,
195,195,195,135,195,225,
196,196,196,196,196,196,196,196,196,196,196,196,196,196,196,196,219,196,196,
  136,210,215,196,196,215,
51,52,52,52,52,52,52,52,52,51,53,51,137,54,227,206,226,
197,197,197,197,197,197,197,197,197,197,197,197,197,197,295,199,202,203,206,
  207,197,197,197,200,201,204,205,197,197,197,138,256,258,259,260,261,262,
  263,264,265,266,267,268,198,
208,208,208,208,208,208,208,208,32,208,208,208,208,208,208,208,208,208,208,
  208,208,208,208,208,208,208,208,208,208,208,32,208,139,208,208,32,
209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,
  209,140,302,210,209,
211,301,
212,212,212,212,212,212,212,212,212,32,212,212,212,212,212,212,212,212,212,
  212,212,212,212,212,212,212,32,212,142,212,212,32,
213,213,213,213,213,213,213,213,213,213,213,213,213,213,213,213,213,213,213,
  213,213,213,213,213,213,213,213,213,213,213,213,213,213,213,213,426,427,
102,23,144,99,214,
110,110,110,145,
215,215,215,215,215,215,215,215,215,215,215,215,215,215,215,215,215,215,215,
  215,215,215,215,215,215,215,215,215,215,215,215,215,215,215,215,107,146,
  100,215,215,109,
29,147,216,
29,148,217,
29,149,218,
29,150,219,
29,151,220,
24,152,221,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,153,222,1,223,223,
230,230,230,230,230,230,230,228,229,232,74,72,226,235,227,230,230,230,224,
  231,233,234,230,230,230,230,154,230,510,511,514,516,521,225,237,236,509,
  512,513,515,517,518,519,520,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,155,222,1,238,238,
244,244,244,244,244,244,244,241,242,239,244,244,244,224,240,231,233,243,234,
  244,244,244,244,156,244,246,245,434,435,436,437,438,439,440,441,442,443,
  225,
345,345,347,345,
61,61,61,61,61,61,61,61,61,61,61,61,61,61,61,61,61,61,61,61,61,61,61,61,61,
  61,61,61,61,61,61,61,61,61,61,61,61,61,158,
56,56,56,56,56,56,56,56,56,56,56,56,56,56,56,56,56,56,56,56,56,56,56,56,158,
  56,56,56,56,56,56,56,56,56,56,56,56,58,56,56,
59,160,
346,346,346,346,346,346,346,157,346,346,346,346,346,346,346,346,157,346,161,
  157,157,348,
63,63,63,63,63,63,63,63,63,63,190,187,183,182,63,162,247,333,64,63,330,331,
  332,2,4,3,
248,248,248,248,32,248,248,248,248,248,248,248,248,248,248,248,32,163,248,
  248,32,
32,560,249,250,
251,251,251,251,251,251,251,32,251,251,251,251,251,251,251,251,251,32,165,
  251,251,32,
34,166,252,
51,52,52,52,52,52,52,52,52,51,53,51,167,254,253,227,226,
255,255,255,255,255,255,255,32,255,255,255,255,255,255,255,255,32,168,255,
  255,32,
99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
  99,169,80,99,
256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,
  256,256,256,256,170,257,256,
83,83,83,83,83,83,83,83,83,83,83,83,83,83,84,171,86,103,171,204,85,
32,258,20,20,32,20,172,258,258,32,
32,259,20,20,32,20,173,259,259,32,
83,83,83,83,83,83,83,83,83,83,83,83,83,83,84,174,86,103,171,201,85,
32,260,20,20,32,20,175,260,260,32,
32,261,20,20,32,20,176,261,261,32,
32,262,20,20,32,20,177,262,262,32,
32,263,20,20,32,20,178,263,263,32,
264,264,264,264,264,264,264,32,264,264,264,264,264,264,264,264,264,264,32,
  179,264,264,32,
32,364,265,
32,361,179,266,
267,267,267,267,267,267,267,32,267,267,267,267,267,267,267,267,267,267,267,
  267,267,32,182,267,267,32,
24,183,268,
415,414,413,412,411,410,409,408,407,406,405,404,184,269,
270,270,270,185,391,271,
272,272,272,272,272,272,272,32,272,272,272,272,272,272,272,272,32,186,272,
  272,32,
32,338,273,
32,335,186,274,
1,1,1,1,1,1,1,1,1,1,1,1,1,69,68,67,66,65,1,189,70,1,72,
275,275,275,275,275,251,
129,129,129,129,129,191,244,
191,248,246,
129,129,240,129,129,129,240,240,240,240,240,240,193,242,242,130,
276,276,276,194,238,277,
74,74,74,127,
218,218,218,218,218,218,218,218,218,218,218,218,218,218,218,218,218,218,220,
  218,
278,278,278,278,278,278,278,278,291,278,278,278,291,278,278,278,278,278,278,
  278,291,291,278,291,291,291,291,278,291,197,278,278,298,
29,296,279,297,
32,280,32,199,280,280,32,
32,281,32,200,281,281,32,
32,282,32,201,282,282,32,
283,283,283,283,283,283,283,281,283,283,283,281,283,283,283,283,281,281,281,
  281,29,281,281,202,284,283,
32,285,32,203,285,285,32,
32,286,32,204,286,286,32,
32,287,32,205,287,287,32,
32,288,32,206,288,288,32,
32,289,32,207,289,289,32,
197,197,197,197,197,197,197,197,197,197,197,197,197,197,295,199,202,203,206,
  207,197,197,197,200,201,204,205,197,197,197,208,257,258,259,260,261,262,
  263,264,265,266,267,268,198,
306,306,306,306,306,306,306,306,306,306,306,306,306,306,306,306,306,306,306,
  306,307,
290,210,
209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,
  209,211,303,210,209,
291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,
  291,291,291,291,291,291,291,212,167,291,
292,292,292,292,292,292,292,292,292,292,292,292,292,292,292,292,292,292,292,
  292,292,292,292,292,292,292,292,292,292,292,292,292,292,292,292,292,292,
  51,213,292,292,53,
32,32,20,214,315,315,32,
106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,
  106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,108,106,
20,20,20,20,20,20,20,32,20,20,20,20,20,20,20,20,20,293,32,216,293,293,32,
32,294,32,217,294,294,32,
32,295,32,218,295,295,32,
32,296,32,219,296,296,32,
297,297,32,297,297,297,32,220,297,297,32,
32,298,298,298,298,298,32,221,298,298,32,
32,299,32,222,299,299,32,
24,506,300,
32,301,32,224,301,301,32,
481,480,225,304,303,302,
32,305,32,226,305,305,32,
32,306,32,227,306,306,32,
32,307,32,228,307,307,32,
32,308,32,229,308,308,32,
40,40,40,40,40,40,40,309,40,40,40,40,40,40,40,310,309,230,309,
32,311,32,231,311,311,32,
32,312,32,232,312,312,32,
32,313,32,233,313,313,32,
32,314,32,234,314,314,32,
32,315,32,235,315,315,32,
32,20,32,20,236,507,507,32,
24,505,316,
24,431,300,
32,317,32,239,317,317,32,
32,318,32,240,318,318,32,
32,319,32,241,319,319,32,
32,320,32,242,320,320,32,
32,321,32,243,321,321,32,
40,40,40,40,40,40,40,322,40,40,40,40,40,40,40,310,322,244,322,
32,20,32,20,245,432,432,32,
24,430,323,
32,324,32,247,324,324,32,
63,63,63,63,63,63,63,63,63,63,190,187,183,182,63,248,325,333,64,63,330,331,
  332,2,4,3,
326,326,326,326,326,326,326,32,326,326,326,326,326,326,326,32,249,326,326,
  32,
32,561,327,
83,83,83,83,83,83,83,83,83,83,83,83,83,83,84,35,251,87,86,88,89,85,559,
328,328,328,328,328,328,328,32,328,328,328,328,328,328,328,32,252,328,328,
  32,
32,20,20,32,329,20,253,329,329,32,
115,115,115,254,330,
1,1,1,1,1,1,1,1,1,1,1,1,1,119,1,255,118,1,113,93,92,
176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,
  176,176,176,176,177,
32,20,20,32,20,257,205,205,32,
33,300,300,300,258,140,203,141,
33,300,300,300,259,140,202,141,
33,300,300,300,260,140,200,141,
33,300,300,300,261,140,199,141,
33,300,300,300,262,140,198,141,
33,300,300,300,263,140,197,141,
1,1,1,1,1,1,1,1,1,1,332,333,1,1,334,1,1,264,331,1,370,365,337,336,335,
338,338,338,338,338,338,338,32,338,338,338,338,338,338,338,338,338,338,32,
  265,338,338,32,
32,362,265,
83,83,83,83,83,83,83,83,83,83,83,83,83,190,187,183,182,83,84,35,267,87,86,
  103,107,106,105,104,108,108,108,108,108,109,85,360,
339,32,339,339,32,268,339,339,32,
32,32,269,340,32,
420,420,420,270,
31,271,341,
1,1,1,1,1,1,1,1,1,1,1,1,342,1,1,272,331,1,354,343,351,
344,344,344,344,344,344,344,32,344,344,344,344,344,344,344,344,32,273,344,
  344,32,
32,336,273,
253,253,253,253,253,252,
345,345,345,234,
346,277,
290,290,290,290,290,290,290,290,290,290,290,290,290,290,290,290,290,290,290,
  290,292,290,
347,347,347,347,347,347,347,347,32,347,347,347,347,347,347,347,347,347,347,
  347,32,347,279,347,347,32,
29,280,348,
29,281,349,
29,282,350,
1,1,1,1,1,1,1,283,1,1,1,283,1,1,1,1,283,283,283,283,283,283,283,285,1,285,
280,279,278,282,
29,285,351,
29,286,352,
29,287,353,
29,288,354,
29,289,355,
356,356,356,356,356,356,356,356,311,356,356,356,311,356,356,311,356,356,356,
  356,311,311,356,356,311,356,311,311,356,311,290,304,356,356,313,
173,173,173,173,173,173,173,173,173,173,173,173,173,173,173,173,173,173,173,
  173,173,173,173,173,173,173,174,
50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,
  50,50,50,50,50,50,50,50,50,50,50,50,52,50,
357,357,357,357,357,357,357,357,357,357,357,357,357,357,357,357,37,293,357,
  357,
358,294,572,
84,295,463,531,530,
84,296,463,571,
359,359,359,359,359,297,532,359,359,
71,73,74,72,70,298,567,565,564,75,566,568,569,
29,299,360,502,
361,361,361,361,361,361,361,32,361,361,361,361,361,361,361,32,300,361,361,
  32,
29,301,362,
1,1,1,1,1,1,1,478,1,1,1,1,1,1,1,302,479,1,476,
363,303,364,
468,470,468,468,468,304,470,
29,305,365,
29,306,366,
29,307,367,
29,308,368,
27,369,27,309,
370,370,370,370,370,370,370,157,370,370,370,370,370,370,370,370,157,310,157,
  157,370,
29,311,371,
29,312,372,
29,313,373,
29,314,374,
29,315,375,
376,376,376,376,376,376,376,32,376,376,376,376,376,376,376,376,376,376,376,
  376,376,376,376,376,376,376,376,32,316,376,376,32,
29,317,377,
29,318,378,
29,319,379,
29,320,380,
29,321,458,381,
382,382,382,382,382,382,382,27,382,382,382,382,382,382,382,369,27,322,382,
  444,
383,383,383,383,383,383,383,32,383,383,383,383,383,383,383,383,383,383,383,
  383,383,383,383,383,32,323,383,383,32,
36,324,340,
32,384,32,325,384,384,32,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,326,385,1,92,
386,386,386,386,386,386,386,32,386,386,386,386,386,386,386,32,327,386,386,
  32,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,328,120,1,387,
31,128,388,
32,114,389,
32,20,390,20,32,20,331,390,390,32,
32,391,32,332,391,391,32,
32,392,32,333,392,392,32,
32,393,32,334,393,393,32,
32,20,20,32,20,335,369,369,32,
32,20,20,32,20,336,368,368,32,
32,20,20,32,20,337,367,367,32,
1,1,1,1,1,1,1,1,1,1,332,333,1,1,334,1,1,338,331,1,370,366,337,336,335,
112,112,112,339,394,114,
395,395,395,340,401,
396,396,396,341,397,
32,398,32,342,398,398,32,
32,20,32,20,343,353,353,32,
1,1,1,1,1,1,1,1,1,1,1,1,342,1,1,344,331,1,354,343,352,
236,236,236,235,
276,276,276,346,399,
197,197,197,197,197,197,197,197,197,197,197,197,197,197,197,197,197,197,197,
  197,347,299,
400,400,400,400,32,400,400,400,400,400,400,400,400,32,348,400,400,32,
401,401,401,401,401,401,401,401,32,401,401,401,401,401,401,401,401,401,401,
  401,32,401,349,401,401,32,
402,402,402,402,402,402,402,32,402,402,402,402,402,402,402,402,32,350,402,
  402,32,
403,403,403,403,403,403,403,32,403,403,403,403,403,403,403,403,403,403,403,
  403,32,351,403,403,32,
404,404,404,404,32,404,404,404,404,404,404,404,404,32,352,404,404,32,
405,32,405,405,32,353,405,405,32,
406,406,406,406,406,406,406,32,406,406,406,406,406,406,406,406,406,32,354,
  406,406,32,
407,407,407,407,407,407,407,32,407,407,407,407,407,407,407,407,407,407,407,
  32,355,407,407,32,
310,310,310,310,310,310,310,310,310,310,310,310,310,310,310,310,310,310,310,
  310,310,312,310,
1,1,1,1,1,1,1,487,486,1,1,1,1,1,1,1,357,488,1,408,
409,409,409,409,409,410,358,409,409,409,409,410,
411,411,411,411,411,533,411,411,
412,412,412,412,412,412,412,32,412,412,412,412,412,412,412,412,32,360,412,
  412,32,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,361,222,1,501,
32,37,32,471,413,32,472,
414,363,
415,415,415,364,415,416,
32,417,32,365,417,417,32,
20,20,20,20,20,20,20,32,20,20,20,20,20,20,20,20,20,418,32,366,418,418,32,
32,419,419,32,367,419,419,32,
32,420,32,368,420,420,32,
421,421,421,421,421,421,421,157,421,421,421,421,421,421,421,421,157,369,157,
  157,421,
423,423,423,423,423,423,423,423,423,423,423,423,423,423,422,370,423,
32,424,32,371,424,424,32,
32,425,32,372,425,425,32,
32,426,32,373,426,426,32,
32,427,32,374,427,427,32,
32,428,32,375,428,428,32,
230,230,230,230,230,230,230,228,229,232,74,72,226,235,227,230,230,230,224,
  231,233,234,230,230,230,230,376,230,510,511,514,516,521,225,429,509,512,
  513,515,517,518,519,520,
32,430,32,377,430,430,32,
32,431,431,32,378,431,431,32,
432,379,490,
432,380,489,
32,20,32,381,457,457,32,433,
40,40,40,40,40,40,40,157,40,40,40,40,40,40,40,434,157,382,157,157,434,
244,244,244,244,244,244,244,241,242,239,244,244,244,224,240,231,233,243,234,
  244,244,244,244,383,244,435,434,435,436,437,438,439,440,441,442,443,225,
36,384,339,
32,20,436,20,32,20,385,436,436,32,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,386,385,1,93,
32,437,32,387,437,437,32,
438,32,438,438,32,388,438,438,32,
439,439,439,439,439,439,439,32,439,439,439,439,439,439,439,439,439,439,439,
  439,439,439,439,439,32,389,439,439,32,
29,141,440,
29,391,441,
29,392,442,
29,393,443,
32,32,394,444,32,
445,445,445,395,
421,421,421,396,
31,397,446,
29,398,447,
448,399,
51,52,52,52,52,52,52,52,52,51,53,51,400,294,227,226,
449,449,449,449,449,449,449,449,449,449,449,449,449,449,449,449,449,449,449,
  449,401,287,
1,1,1,1,1,1,1,1,1,1,1,1,1,161,1,402,162,1,286,
1,1,1,1,1,1,1,1,1,1,1,1,1,69,68,67,66,65,1,403,70,1,277,
51,52,52,52,52,52,52,52,52,51,53,51,404,276,227,226,
450,450,450,405,275,
1,1,1,1,1,1,1,1,1,1,1,1,273,272,1,1,406,274,1,270,271,
1,1,1,1,1,1,1,1,1,1,1,1,1,125,124,123,122,1,407,126,1,269,
468,570,468,468,468,408,570,
575,575,575,575,575,577,575,575,575,
578,410,
451,451,451,451,451,534,451,451,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,84,412,453,1,452,
37,473,474,
454,414,
74,74,74,482,
38,416,455,
526,417,524,456,
457,457,457,457,457,457,457,457,457,457,457,457,457,457,457,457,37,418,457,
  457,
556,555,419,
458,420,547,
460,460,460,460,460,460,460,460,460,460,460,460,460,460,459,421,460,
159,159,159,159,159,159,159,159,159,159,159,159,159,159,159,159,159,159,159,
  159,159,159,159,159,158,159,461,159,159,159,159,159,159,159,159,159,159,
  159,422,159,159,159,461,
40,40,40,40,40,40,40,40,40,40,40,40,40,40,494,
84,424,465,464,
462,425,541,
84,426,463,462,
84,427,446,445,
84,428,523,522,
32,20,32,20,429,508,508,32,
37,430,463,
467,466,431,
464,464,464,464,464,465,432,465,464,465,
466,433,
467,434,
32,20,32,20,435,433,433,32,
29,94,360,95,
34,437,468,
195,195,195,438,195,469,
1,1,1,1,1,1,1,1,1,1,1,1,1,473,474,476,477,471,470,472,478,479,1,439,331,1,
  116,130,131,132,133,134,135,136,137,138,139,139,147,475,156,
480,480,480,480,480,480,480,32,480,480,480,480,480,480,480,480,480,32,440,
  480,480,32,
481,32,481,481,481,32,441,481,481,32,
32,482,482,32,442,482,482,32,
32,483,483,32,443,483,483,32,
270,270,270,444,393,271,
484,484,484,416,
485,485,485,446,486,
487,487,487,487,487,487,487,32,487,487,487,487,487,487,487,32,447,487,487,
  32,
276,276,276,448,233,
278,278,278,278,278,278,278,278,291,278,278,278,291,278,278,278,278,278,278,
  278,291,291,278,291,291,291,278,291,449,278,278,293,
488,488,488,149,
489,489,489,489,489,535,489,489,
32,20,20,32,20,452,97,97,32,
32,20,20,32,20,453,96,96,32,
477,454,
1,1,1,1,1,1,1,478,1,1,1,1,1,1,1,455,479,1,475,
63,63,63,63,63,63,63,63,63,63,190,187,183,182,63,456,490,490,333,64,63,330,
  331,332,2,4,3,
1,1,1,1,1,1,1,487,486,1,1,1,1,1,1,1,457,488,1,491,
492,492,492,492,492,492,492,492,492,492,458,492,
159,159,159,159,159,159,159,159,159,159,159,159,159,159,159,159,159,159,159,
  159,159,159,159,159,158,159,493,159,159,159,159,159,159,159,159,159,159,
  159,459,159,159,159,493,
40,40,40,40,40,40,40,40,40,40,40,40,40,40,496,
495,461,
495,495,32,495,495,495,494,32,462,494,494,32,495,496,495,
1,1,1,1,1,1,1,487,486,1,1,1,1,1,1,1,463,488,1,497,484,
63,63,63,63,63,64,
493,465,
499,499,499,499,499,499,499,498,499,499,499,499,499,499,499,499,499,499,499,
  499,498,499,466,498,499,
500,500,500,500,500,500,500,157,500,500,500,500,500,500,500,500,157,467,157,
  157,500,
501,501,501,501,501,501,501,32,501,501,501,501,501,501,501,501,501,501,501,
  32,468,501,501,32,
32,20,20,32,20,469,129,129,32,
32,20,20,20,32,20,470,153,153,32,
32,502,32,471,502,502,32,
32,503,32,472,503,503,32,
32,504,32,473,504,504,32,
32,505,32,474,505,505,32,
154,29,154,154,475,506,152,
32,507,32,476,507,507,32,
32,508,32,477,508,508,32,
32,20,20,32,20,478,146,146,32,
32,20,20,32,20,479,145,145,32,
1,1,1,1,1,1,1,1,1,1,1,1,1,53,1,84,480,511,1,510,509,
118,118,118,37,481,512,118,384,385,
382,381,482,380,
513,514,483,371,
418,418,418,417,
422,422,422,485,
32,32,486,515,32,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,487,350,1,
151,151,151,150,
516,516,516,516,516,536,516,516,
525,490,
468,529,468,468,468,491,529,
549,549,549,549,517,517,549,549,549,549,549,517,549,492,517,517,
497,493,
543,494,
518,518,518,518,518,495,518,518,
542,496,
32,483,519,32,497,519,519,32,
27,27,448,
520,520,520,520,520,520,520,520,520,520,520,520,520,520,520,183,182,520,449,
  520,499,521,2,522,521,522,453,520,
524,524,524,524,524,524,524,524,524,524,524,524,524,524,523,500,524,
1,1,1,1,1,1,1,1,1,1,1,1,1,125,124,123,122,1,501,126,1,117,121,
29,502,525,
29,503,526,
29,504,527,
29,505,528,
529,32,529,529,32,506,529,529,32,
29,507,530,
29,508,531,
32,20,20,32,20,509,144,144,32,
32,20,20,32,20,510,143,143,32,
32,20,20,32,20,511,142,142,32,
32,532,532,532,532,532,532,532,32,512,532,532,32,
372,533,372,372,372,372,513,375,
372,533,372,372,372,372,514,374,
425,424,423,515,419,
534,534,534,534,534,537,534,534,
552,517,
535,535,535,535,535,518,535,535,
24,519,536,
455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,456,
537,451,537,
498,538,498,522,498,538,
159,159,159,159,159,159,159,159,159,159,159,159,159,159,159,159,159,159,159,
  159,159,159,159,159,158,159,539,159,159,159,159,159,159,159,159,159,159,
  159,523,159,159,159,539,
40,40,40,40,40,40,40,40,40,40,40,40,40,40,498,
540,540,540,540,32,540,540,540,540,540,540,540,540,32,525,540,540,32,
541,32,541,541,32,526,541,541,32,
32,542,32,527,542,542,32,
543,543,543,543,543,543,543,32,543,543,543,543,543,543,543,543,32,528,543,
  543,32,
195,195,195,529,195,155,
544,544,544,544,544,544,544,32,544,544,544,544,544,544,544,32,530,544,544,
  32,
545,545,32,545,545,545,545,32,531,545,545,32,
400,399,398,397,396,395,394,532,546,113,
547,376,547,547,376,376,376,376,533,373,
548,548,548,548,548,538,548,548,
549,549,549,549,549,535,549,549,
550,550,550,550,550,550,550,32,550,550,550,550,550,550,550,550,550,32,536,
  550,550,32,
183,182,537,460,2,
450,538,
499,539,
51,52,52,52,52,52,52,52,52,51,53,51,540,551,227,226,
450,450,450,541,552,
37,542,553,
1,1,1,1,1,1,1,1,1,1,1,1,1,161,1,543,162,1,554,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,544,555,1,
129,129,129,129,129,131,545,556,164,133,132,130,
32,557,32,546,557,557,32,
558,558,558,377,
540,540,540,540,540,539,540,540,
559,559,559,559,559,549,559,559,
1,1,1,1,1,1,1,487,486,1,1,1,1,1,1,1,550,488,1,485,
32,20,20,32,20,551,157,157,32,
32,20,20,32,20,552,148,148,32,
449,449,449,449,449,449,449,449,449,449,449,449,449,449,449,449,449,449,449,
  449,553,560,
32,20,20,32,20,554,160,160,32,
32,20,20,32,20,555,159,159,32,
32,20,20,32,20,556,158,158,32,
386,557,
379,379,379,378,
561,561,561,561,561,559,561,561,
163,560,
562,562,562,562,562,561,562,562,
563,563,563,563,563,562,563,563,
564,564,564,564,564,563,564,564,
565,565,565,565,565,564,565,565,
566,566,566,566,566,565,566,566,
567,567,567,567,567,566,567,567,
568,568,568,568,568,567,568,568,
569,569,569,569,569,568,569,569,
570,570,570,570,570,569,570,570,
571,571,571,571,571,570,571,571,
572,572,572,572,572,571,572,572,
573,573,573,573,573,572,573,573,
574,574,574,574,574,573,574,574,
575,575,575,575,575,574,575,575,
576,576,576,576,576,575,576,576,
577,577,577,577,577,576,577,577,
578,578,578,578,578,577,578,578,
579,579,579,579,579,578,579,579,
580,580,580,580,580,579,580,580,
581,581,581,581,581,580,581,581,
582,582,582,582,582,581,582,582,
583,583,583,583,583,582,583,583,
584,584,584,584,584,583,584,584,
585,585,585,585,585,584,585,585,
586,586,586,586,586,585,586,586,
544,544,544,544,544,586,544,544,
};
static const unsigned short ag_sbt[] = {
     0,  70,  85,  93, 103, 113, 121, 125, 127, 129, 131, 133, 136, 138,
   140, 143, 146, 149, 152, 155, 158, 161, 164, 167, 170, 173, 176, 193,
   222, 234, 246, 261, 276, 279, 282, 301, 328, 336, 387, 403, 417, 438,
   459, 484, 507, 514, 545, 586, 603, 620, 657, 665, 678, 691, 702, 705,
   722, 745, 748, 751, 787, 790, 801, 812, 824, 827, 869, 873, 877, 880,
   882, 889, 896, 903, 910, 917, 925, 927, 932, 937, 939, 944, 949, 951,
   973,1016,1031,1039,1060,1063,1072,1075,1077,1084,1089,1092,1094,1098,
  1105,1107,1134,1137,1144,1146,1149,1158,1167,1174,1177,1181,1190,1193,
  1195,1199,1206,1211,1216,1223,1225,1229,1236,1238,1242,1250,1252,1256,
  1261,1268,1270,1282,1288,1301,1317,1319,1321,1323,1329,1354,1371,1415,
  1451,1475,1477,1509,1546,1551,1555,1596,1599,1602,1605,1608,1611,1614,
  1633,1677,1696,1734,1738,1777,1817,1819,1841,1867,1888,1892,1914,1917,
  1934,1955,1984,2010,2031,2041,2051,2072,2082,2092,2102,2112,2135,2138,
  2142,2168,2171,2185,2191,2212,2215,2219,2242,2248,2255,2258,2274,2280,
  2284,2304,2337,2341,2348,2355,2362,2388,2395,2402,2409,2416,2423,2467,
  2488,2490,2514,2543,2585,2592,2629,2652,2659,2666,2673,2684,2695,2702,
  2705,2712,2718,2725,2732,2739,2746,2765,2772,2779,2786,2793,2800,2808,
  2811,2814,2821,2828,2835,2842,2849,2868,2876,2879,2886,2912,2932,2935,
  2958,2978,2988,2993,3014,3038,3047,3055,3063,3071,3079,3087,3095,3120,
  3143,3146,3182,3191,3196,3200,3203,3224,3245,3248,3254,3258,3260,3282,
  3308,3311,3314,3317,3343,3347,3350,3353,3356,3359,3362,3397,3424,3463,
  3483,3486,3491,3495,3504,3517,3521,3541,3544,3563,3566,3573,3576,3579,
  3582,3585,3589,3610,3613,3616,3619,3622,3625,3657,3660,3663,3666,3669,
  3673,3693,3722,3725,3732,3750,3770,3788,3791,3794,3804,3811,3818,3825,
  3834,3843,3852,3877,3883,3888,3893,3900,3908,3929,3933,3938,3960,3978,
  4004,4025,4050,4068,4077,4099,4123,4146,4166,4178,4186,4207,4225,4232,
  4234,4240,4247,4270,4278,4285,4306,4323,4330,4337,4344,4351,4358,4401,
  4408,4416,4419,4422,4430,4451,4488,4491,4501,4519,4526,4535,4564,4567,
  4570,4573,4576,4581,4585,4589,4592,4595,4597,4613,4635,4654,4677,4693,
  4698,4719,4741,4748,4757,4759,4767,4786,4789,4791,4795,4798,4802,4822,
  4825,4828,4845,4888,4903,4907,4910,4914,4918,4922,4930,4933,4936,4946,
  4948,4950,4958,4962,4965,4971,5012,5034,5044,5052,5060,5066,5070,5075,
  5095,5100,5132,5136,5144,5153,5162,5164,5183,5210,5230,5242,5285,5300,
  5302,5317,5338,5344,5346,5371,5392,5416,5425,5435,5442,5449,5456,5463,
  5470,5477,5484,5493,5502,5523,5532,5536,5540,5544,5548,5553,5570,5574,
  5582,5584,5591,5607,5609,5611,5619,5621,5629,5632,5660,5677,5700,5703,
  5706,5709,5712,5721,5724,5727,5736,5745,5754,5767,5775,5783,5788,5796,
  5798,5806,5809,5827,5830,5836,5879,5894,5912,5921,5928,5949,5955,5975,
  5987,5997,6007,6015,6023,6045,6050,6052,6054,6070,6075,6078,6097,6114,
  6126,6133,6137,6145,6153,6173,6182,6191,6213,6222,6231,6240,6242,6246,
  6254,6256,6264,6272,6280,6288,6296,6304,6312,6320,6328,6336,6344,6352,
  6360,6368,6376,6384,6392,6400,6408,6416,6424,6432,6440,6448,6456,6464
};
static const unsigned short ag_sbe[] = {
    50,  84,  87,  96, 106, 116, 122, 126, 128, 130, 132, 134, 137, 139,
   141, 144, 147, 150, 153, 156, 159, 162, 165, 168, 171, 174, 188, 216,
   230, 242, 257, 272, 278, 280, 300, 316, 332, 383, 398, 408, 432, 453,
   475, 499, 510, 540, 566, 596, 613, 640, 660, 676, 689, 697, 703, 717,
   744, 746, 749, 783, 788, 797, 808, 823, 825, 865, 870, 876, 878, 881,
   885, 892, 899, 906, 913, 921, 926, 929, 934, 938, 941, 946, 950, 969,
  1011,1030,1035,1056,1061,1068,1073,1076,1080,1086,1090,1093,1097,1101,
  1106,1133,1135,1140,1145,1147,1152,1161,1169,1175,1178,1186,1191,1194,
  1198,1202,1208,1213,1219,1224,1228,1232,1237,1239,1246,1251,1255,1258,
  1264,1269,1281,1287,1297,1313,1318,1320,1322,1326,1348,1366,1401,1447,
  1471,1476,1505,1544,1548,1554,1591,1597,1600,1603,1606,1609,1612,1628,
  1659,1691,1719,1736,1776,1814,1818,1837,1856,1884,1889,1910,1915,1929,
  1951,1981,2007,2025,2037,2047,2066,2078,2088,2098,2108,2131,2136,2139,
  2164,2169,2183,2188,2208,2213,2216,2238,2247,2253,2256,2270,2277,2283,
  2302,2333,2338,2344,2351,2358,2385,2391,2398,2405,2412,2419,2453,2487,
  2489,2510,2540,2581,2588,2627,2648,2655,2662,2669,2680,2691,2698,2703,
  2708,2714,2721,2728,2735,2742,2763,2768,2775,2782,2789,2796,2804,2809,
  2812,2817,2824,2831,2838,2845,2866,2872,2877,2882,2901,2928,2933,2951,
  2974,2984,2991,3008,3037,3043,3051,3059,3067,3075,3083,3091,3112,3139,
  3144,3166,3187,3193,3199,3201,3218,3241,3246,3253,3257,3259,3280,3304,
  3309,3312,3315,3339,3346,3348,3351,3354,3357,3360,3392,3423,3461,3480,
  3484,3487,3492,3500,3509,3518,3537,3542,3559,3564,3571,3574,3577,3580,
  3583,3588,3606,3611,3614,3617,3620,3623,3653,3658,3661,3664,3667,3670,
  3690,3718,3723,3728,3746,3766,3784,3789,3792,3800,3807,3814,3821,3830,
  3839,3848,3869,3880,3886,3891,3896,3904,3923,3932,3936,3958,3974,4000,
  4021,4046,4064,4073,4095,4119,4144,4162,4172,4183,4203,4221,4228,4233,
  4237,4243,4266,4274,4281,4302,4321,4326,4333,4340,4347,4354,4384,4404,
  4412,4417,4420,4425,4447,4474,4489,4497,4515,4522,4531,4560,4565,4568,
  4571,4574,4578,4584,4588,4590,4593,4596,4609,4633,4650,4673,4689,4696,
  4714,4737,4746,4753,4758,4764,4782,4787,4790,4794,4796,4799,4819,4824,
  4826,4843,4883,4902,4904,4908,4911,4915,4919,4926,4931,4935,4942,4947,
  4949,4954,4959,4963,4968,4994,5030,5040,5048,5056,5063,5069,5073,5091,
  5098,5128,5135,5141,5149,5158,5163,5179,5198,5226,5240,5280,5299,5301,
  5310,5333,5343,5345,5368,5388,5412,5421,5431,5438,5445,5452,5459,5467,
  5473,5480,5489,5498,5518,5527,5534,5538,5543,5547,5550,5567,5573,5579,
  5583,5589,5604,5608,5610,5616,5620,5625,5631,5652,5675,5695,5701,5704,
  5707,5710,5717,5722,5725,5732,5741,5750,5763,5773,5781,5786,5793,5797,
  5803,5807,5826,5828,5833,5874,5893,5908,5917,5924,5945,5952,5971,5983,
  5994,6005,6012,6020,6041,6047,6051,6053,6066,6073,6076,6093,6111,6120,
  6129,6136,6142,6150,6169,6178,6187,6211,6218,6227,6236,6241,6245,6251,
  6255,6261,6269,6277,6285,6293,6301,6309,6317,6325,6333,6341,6349,6357,
  6365,6373,6381,6389,6397,6405,6413,6421,6429,6437,6445,6453,6461,6464
};
static const unsigned char ag_fl[] = {
  1,4,4,4,4,4,4,4,4,4,4,4,4,2,2,4,2,2,2,2,0,1,2,1,1,1,1,2,1,1,1,1,1,1,1,
  1,1,1,1,1,2,1,1,2,1,1,2,1,1,1,2,0,1,2,1,1,2,0,1,3,1,2,1,2,1,1,1,1,1,1,
  1,2,3,1,2,1,2,1,2,1,3,1,5,0,1,3,4,1,1,3,4,4,3,4,2,3,4,4,1,2,5,3,1,1,1,
  1,2,0,1,1,3,1,1,4,4,0,4,9,1,1,1,1,1,1,1,1,1,1,2,6,1,1,1,1,1,1,1,1,1,1,
  1,2,6,6,6,2,2,1,6,1,2,3,2,2,0,3,1,6,6,6,6,1,1,7,1,1,1,4,1,2,1,1,1,2,1,
  1,2,1,1,1,1,2,1,1,3,2,1,1,3,2,1,3,3,1,1,1,1,4,4,4,4,3,4,4,3,4,4,3,1,1,
  3,1,1,2,1,1,1,1,2,0,1,1,2,1,1,3,1,1,1,1,2,2,3,5,1,2,3,1,3,1,0,1,3,2,2,
  1,2,0,1,2,1,2,3,4,0,1,3,4,1,1,1,1,1,1,1,1,1,1,1,5,5,5,1,1,1,5,5,5,3,3,
  3,0,1,0,1,3,5,5,1,1,2,0,1,2,5,1,1,2,2,3,0,1,2,3,3,1,2,1,1,1,2,0,1,1,1,
  6,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,1,2,6,5,2,1,1,1,2,0,1,3,2,
  5,3,4,2,1,1,1,1,1,1,4,2,3,1,2,3,4,2,2,2,1,5,0,2,2,2,0,1,2,3,5,1,1,2,5,
  1,5,1,1,1,2,3,1,7,1,1,1,1,1,1,1,5,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,4,7,
  2,2,2,1,1,1,4,5,1,1,3,3,2,5,1,1,1,1,1,1,1,1,1,1,3,5,1,0,1,7,9,1,1,1,1,
  2,1,1,1,1,3,1,5,1,5,1,5,5,0,1,3,3,4,4,5,5,2,4,1,1,1,1,1,7,1,5,1,1,1,4,
  4,0,1,3,4,6,5,7,5,7,1,4,3,1,1,3,3,2,5,1,1,1,1,1,1,1,1,1,1,1,1,1,5,1,5,
  3,1,0,1,7,5,1,5,1,2,3,4,5,6,7,8,5,3,3,32,1,1,5,1,2,0,1,4,1,1,5,5,1,1,4,
  2,3,1,1,5,1,1,1,1,1,7,5,5,1,1,2,0,1,3
};
static const unsigned short ag_ptt[] = {
    0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1, 38, 38,  3, 39, 41, 43, 45, 45, 37, 46, 48, 50, 51, 53, 55, 57,
   59, 61, 63, 67, 67, 65, 70, 70, 68, 73, 73, 71, 77, 78, 78, 79, 79, 74,
   82, 83, 83, 84, 84, 80, 81, 81, 90, 90, 88, 91, 91, 91, 91, 91, 91,  4,
   97,100,100, 98, 17,101, 15,102,102,103,104,107,107,108,108,105,106,109,
  110,110,111,111,112,112,113,113,115,115, 28, 28,114,116,121,122,122,123,
  123,117,119, 19,124,124,125,128,128,126,130,130,131,132,134,134,134,134,
  134,139,127,127,129,129,129,129,129,129,129,129,129,129,150,151,151,151,
  151,141,142,143,155,157,157,157,147,158,159,159,144,161,145,146,148,167,
  167,149,164,172, 31,174,178,178,175,179,182,182,176,185,185,183, 34, 32,
   33,186,189,189,187,187,194,193,188,188,197,190,190,203,203,203,203,204,
  204,205,205,205,206,206,206,207,199,199,208,209,209,212,216,216,214,213,
  219,220,220,221,221,217,202,222,210,210,140,140,224,227,227,224,152,230,
  231,231,231,171,171,233,236,236,233,233,238,239,239,240,240,234,237,237,
  237,237,200,200,241,241,242,242,242,242,242,242,242,242,242,242,242,243,
  244,244,256,256,257,245,246,247,248,248,248,265,265,266,266,248,249,250,
  268,269,269,270,270,170,251,252,253,253,273,274,201,201,275,275,276,281,
  281,278,283,284,284,285,285,279, 29,286,287,287,287,287,287,287,287,287,
  287,287,287,287,287,287,173,173,173,173,  7,301,301,301,301,302,302,180,
  180,305,306,306,307,307,304,304,308,303,303,310,310, 13,311,311,312,313,
  313,315,315,315,315,316,316,317,317,317,317,318,325,325,322,322,324,324,
  324,324,319,328,328,  9,320,333,333,331,331,334, 11,336,336,335,339,339,
  339,339,339,339,339,337,347,347,348,348,348,348,348,348,348,348,348,348,
  348,348,349,349,349,338,362,363,364,365,365,365, 35, 35, 23,369,370,370,
  372,372,374,374,374,374,374,374,374,374,374,374,374,375,387,391,391,376,
  376,392,392,394,397,397,395,390,389,393,393,398,377,401,378,403,382,382,
  408,408,383,406,406,406,406,407,407,411,413,413,410,414,412,384,422,422,
  423,423,423,379,380,429,429,427,381,381,381,381,385,385,373,373,430, 25,
  431,432,432,433,433,434,434,434,434,434,434,434,434,434,434,434,434,434,
  435,444,442,446,447,448,448,441,437,450,438,452,452,452,452,452,452,452,
  452,436,455,455,456,458,453,439,462,462,464,464,460,463,463,440,440, 21,
  470,470,471,471, 27,472,472,473,473,473,473,473,476,474,475,480,481,481,
  482,482,479
};
static void ag_ra(PCB_DECL)
{
  switch(ag_rpx[(PCB).ag_ap]) {
    case 1: ag_rp_1(PCB_POINTER); break;
    case 2: ag_rp_2(PCB_POINTER); break;
    case 3: ag_rp_3(PCB_POINTER); break;
    case 4: ag_rp_4(PCB_POINTER); break;
    case 5: ag_rp_5(PCB_POINTER); break;
    case 6: ag_rp_6(PCB_POINTER); break;
    case 7: ag_rp_7(PCB_POINTER); break;
    case 8: ag_rp_8(PCB_POINTER); break;
    case 9: ag_rp_9(PCB_POINTER); break;
    case 10: ag_rp_10(PCB_POINTER); break;
    case 11: ag_rp_11(PCB_POINTER); break;
    case 12: ag_rp_12(PCB_POINTER); break;
    case 13: ag_rp_13(PCB_POINTER); break;
    case 14: ag_rp_14(PCB_POINTER); break;
    case 15: ag_rp_15(PCB_POINTER); break;
    case 16: V(0,ParserBasicToken) = ag_rp_16(PCB_POINTER); break;
    case 17: V(0,ParserBasicToken) = ag_rp_17(PCB_POINTER); break;
    case 18: V(0,ParserBasicToken) = ag_rp_18(PCB_POINTER); break;
    case 19: V(0,ParserBasicToken) = ag_rp_19(PCB_POINTER); break;
    case 20: V(0,ParserQuotedString) = ag_rp_20(PCB_POINTER); break;
    case 21: V(0,ParserBasicToken) = ag_rp_21(PCB_POINTER); break;
    case 22: V(0,ParserMethod) = ag_rp_22(PCB_POINTER); break;
    case 23: V(0,ParserMethod) = ag_rp_23(PCB_POINTER); break;
    case 24: V(0,ParserMethod) = ag_rp_24(PCB_POINTER); break;
    case 25: V(0,ParserMethod) = ag_rp_25(PCB_POINTER); break;
    case 26: V(0,ParserMethod) = ag_rp_26(PCB_POINTER); break;
    case 27: V(0,ParserMethod) = ag_rp_27(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 28: ag_rp_28(PCB_POINTER, V(0,ParserCSeq)); break;
    case 29: V(0,ParserCSeq) = ag_rp_29(PCB_POINTER, V(0,ParserSequenceNumber), V(2,ParserMethod)); break;
    case 30: V(0,ParserSequenceNumber) = ag_rp_30(PCB_POINTER); break;
    case 31: ag_rp_31(PCB_POINTER, V(0,ParserContentLength)); break;
    case 32: V(0,ParserContentLength) = ag_rp_32(PCB_POINTER); break;
    case 33: ag_rp_33(PCB_POINTER, V(0,ParserCallId)); break;
    case 34: V(0,ParserCallId) = ag_rp_34(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 35: V(0,ParserCallId) = ag_rp_35(PCB_POINTER, V(0,ParserBasicToken), V(2,ParserBasicToken)); break;
    case 36: ag_rp_36(PCB_POINTER); break;
    case 37: ag_rp_37(PCB_POINTER, V(0,ParserMediaType), V(2,ParserMediaSubType)); break;
    case 38: ag_rp_38(PCB_POINTER); break;
    case 39: ag_rp_39(PCB_POINTER); break;
    case 40: V(0,ParserMediaType) = ag_rp_40(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 41: V(0,ParserMediaSubType) = ag_rp_41(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 42: ag_rp_42(PCB_POINTER, V(0,ParserBasicToken), V(2,ParserGenericParamDataVal)); break;
    case 43: V(0,ParserGenericParamDataVal) = ag_rp_43(PCB_POINTER, V(2,ParserBasicToken)); break;
    case 44: V(0,ParserGenericParamDataVal) = ag_rp_44(PCB_POINTER, V(2,ParserQuotedString)); break;
    case 45: ag_rp_45(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 46: ag_rp_46(PCB_POINTER, V(0,ParserBasicToken), V(2,ParserGenericParamDataVal)); break;
    case 47: V(0,ParserGenericParamDataVal) = ag_rp_47(PCB_POINTER, V(2,ParserBasicToken)); break;
    case 48: V(0,ParserGenericParamDataVal) = ag_rp_48(PCB_POINTER, V(2,ParserQuotedString)); break;
    case 49: ag_rp_49(PCB_POINTER, V(0,ParserSipVersion), V(2,ParserStatusCode), V(4,ParserReasonPhrase)); break;
    case 50: ag_rp_50(PCB_POINTER, V(0,ParserSipVersion), V(2,ParserStatusCode)); break;
    case 51: V(0,ParserSipVersion) = ag_rp_51(PCB_POINTER); break;
    case 52: V(0,ParserReasonPhrase) = ag_rp_52(PCB_POINTER); break;
    case 53: V(0,ParserStatusCode) = ag_rp_53(PCB_POINTER, V(0,int), V(1,int), V(2,int)); break;
    case 54: ag_rp_54(PCB_POINTER); break;
    case 55: ag_rp_55(PCB_POINTER); break;
    case 56: ag_rp_56(PCB_POINTER); break;
    case 57: ag_rp_57(PCB_POINTER); break;
    case 58: ag_rp_58(PCB_POINTER, V(3,ParserViaParam)); break;
    case 59: ag_rp_59(PCB_POINTER, V(0,ParserProtocolName), V(4,ParserProtocolVersion), V(8,ParserTransport)); break;
    case 60: V(0,ParserProtocolName) = ag_rp_60(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 61: V(0,ParserProtocolName) = ag_rp_61(PCB_POINTER); break;
    case 62: V(0,ParserProtocolVersion) = ag_rp_62(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 63: V(0,ParserTransport) = ag_rp_63(PCB_POINTER, V(0,ParserTransport)); break;
    case 64: V(0,ParserTransport) = ag_rp_64(PCB_POINTER); break;
    case 65: V(0,ParserTransport) = ag_rp_65(PCB_POINTER); break;
    case 66: V(0,ParserTransport) = ag_rp_66(PCB_POINTER); break;
    case 67: V(0,ParserTransport) = ag_rp_67(PCB_POINTER); break;
    case 68: V(0,ParserTransport) = ag_rp_68(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 69: V(0,ParserPort) = ag_rp_69(PCB_POINTER); break;
    case 70: ag_rp_70(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 71: ag_rp_71(PCB_POINTER, V(0,ParserBasicToken), V(4,ParserPort)); break;
    case 72: V(0,ParserViaParam) = ag_rp_72(PCB_POINTER); break;
    case 73: V(0,ParserViaParam) = ag_rp_73(PCB_POINTER); break;
    case 74: V(0,ParserViaParam) = ag_rp_74(PCB_POINTER, V(0,ParserTTLParam)); break;
    case 75: V(0,ParserViaParam) = ag_rp_75(PCB_POINTER, V(0,ParserMaddrParam)); break;
    case 76: V(0,ParserViaParam) = ag_rp_76(PCB_POINTER, V(0,ParserReceived)); break;
    case 77: V(0,ParserViaParam) = ag_rp_77(PCB_POINTER, V(0,ParserBranch)); break;
    case 78: V(0,ParserViaParam) = ag_rp_78(PCB_POINTER, V(0,ParserPort)); break;
    case 79: V(0,ParserViaParam) = ag_rp_79(PCB_POINTER, V(0,ParserCompParam)); break;
    case 80: V(0,ParserViaParam) = ag_rp_80(PCB_POINTER, V(0,ParserSigCompIdParam)); break;
    case 81: V(0,ParserViaParam) = ag_rp_81(PCB_POINTER); break;
    case 82: ag_rp_82(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 83: ag_rp_83(PCB_POINTER, V(0,ParserBasicToken), V(4,ParserBasicToken)); break;
    case 84: ag_rp_84(PCB_POINTER, V(0,ParserBasicToken), V(4,ParserQuotedString)); break;
    case 85: ag_rp_85(PCB_POINTER, V(0,ParserBasicToken), V(4,ParserBasicToken)); break;
    case 86: V(0,ParserTTLParam) = ag_rp_86(PCB_POINTER, V(0,ParserTtlParam)); break;
    case 87: V(0,ParserTtlParam) = ag_rp_87(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 88: V(0,ParserBasicToken) = ag_rp_88(PCB_POINTER); break;
    case 89: V(0,ParserBasicToken) = ag_rp_89(PCB_POINTER); break;
    case 90: V(0,ParserBasicToken) = ag_rp_90(PCB_POINTER); break;
    case 91: V(0,ParserPort) = ag_rp_91(PCB_POINTER, V(1,ParserPort)); break;
    case 92: V(0,ParserPort) = ag_rp_92(PCB_POINTER); break;
    case 93: V(0,ParserPort) = ag_rp_93(PCB_POINTER, V(2,ParserPort)); break;
    case 94: V(0,ParserMaddrParam) = ag_rp_94(PCB_POINTER, V(0,ParserMaddrParam)); break;
    case 95: V(0,ParserMaddrParam) = ag_rp_95(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 96: V(0,ParserReceived) = ag_rp_96(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 97: V(0,ParserBranch) = ag_rp_97(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 98: V(0,ParserCompParam) = ag_rp_98(PCB_POINTER, V(4,ParserCompParam)); break;
    case 99: V(0,ParserCompParam) = ag_rp_99(PCB_POINTER); break;
    case 100: V(0,ParserCompParam) = ag_rp_100(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 101: V(0,ParserSigCompIdParam) = ag_rp_101(PCB_POINTER, V(5,ParserBasicToken)); break;
    case 102: V(0,ParserBasicToken) = ag_rp_102(PCB_POINTER); break;
    case 103: ag_rp_103(PCB_POINTER, V(0,ParserAbsoluteUri)); break;
    case 104: V(0,ParserAbsoluteUri) = ag_rp_104(PCB_POINTER, V(0,ParserBasicToken), V(3,ParserBasicToken)); break;
    case 105: V(0,ParserBasicToken) = ag_rp_105(PCB_POINTER); break;
    case 106: V(0,ParserDisplayName) = ag_rp_106(PCB_POINTER, V(0,ParserDisplayName)); break;
    case 107: V(0,ParserBasicToken) = ag_rp_107(PCB_POINTER); break;
    case 108: V(0,ParserBasicToken) = ag_rp_108(PCB_POINTER); break;
    case 109: ag_rp_109(PCB_POINTER); break;
    case 110: ag_rp_110(PCB_POINTER, V(0,ParserAbsoluteUri)); break;
    case 111: ag_rp_111(PCB_POINTER, V(0,ParserAbsoluteUri)); break;
    case 112: ag_rp_112(PCB_POINTER); break;
    case 113: ag_rp_113(PCB_POINTER); break;
    case 114: V(0,ParserAbsoluteUri) = ag_rp_114(PCB_POINTER, V(2,ParserAbsoluteUri)); break;
    case 115: V(0,ParserAbsoluteUri) = ag_rp_115(PCB_POINTER); break;
    case 116: ag_rp_116(PCB_POINTER); break;
    case 117: ag_rp_117(PCB_POINTER); break;
    case 118: V(0,ParserAbsoluteUri) = ag_rp_118(PCB_POINTER, V(2,ParserAbsoluteUri)); break;
    case 119: V(0,ParserAbsoluteUri) = ag_rp_119(PCB_POINTER); break;
    case 120: ag_rp_120(PCB_POINTER); break;
    case 121: ag_rp_121(PCB_POINTER, V(2,ParserOptionalHeaders)); break;
    case 122: ag_rp_122(PCB_POINTER, V(2,ParserOptionalHeaders)); break;
    case 123: ag_rp_123(PCB_POINTER, V(3,ParserOptionalHeaders)); break;
    case 124: ag_rp_124(PCB_POINTER, V(3,ParserOptionalHeaders)); break;
    case 125: ag_rp_125(PCB_POINTER, V(3,ParserOptionalHeaders)); break;
    case 126: ag_rp_126(PCB_POINTER, V(3,ParserOptionalHeaders)); break;
    case 127: ag_rp_127(PCB_POINTER, V(3,ParserOptionalHeaders)); break;
    case 128: ag_rp_128(PCB_POINTER, V(3,ParserOptionalHeaders)); break;
    case 129: ag_rp_129(PCB_POINTER, V(0,ParserDisplayName), V(2,ParserBasicToken)); break;
    case 130: ag_rp_130(PCB_POINTER); break;
    case 131: ag_rp_131(PCB_POINTER); break;
    case 132: ag_rp_132(PCB_POINTER); break;
    case 133: ag_rp_133(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 134: ag_rp_134(PCB_POINTER, V(0,ParserBasicToken), V(2,ParserBasicToken)); break;
    case 135: V(0,ParserBasicToken) = ag_rp_135(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 136: V(0,ParserBasicToken) = ag_rp_136(PCB_POINTER); break;
    case 137: V(0,ParserBasicToken) = ag_rp_137(PCB_POINTER); break;
    case 138: ag_rp_138(PCB_POINTER); break;
    case 139: ag_rp_139(PCB_POINTER); break;
    case 140: ag_rp_140(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 141: ag_rp_141(PCB_POINTER, V(0,ParserBasicToken), V(2,ParserPort)); break;
    case 142: V(0,ParserBasicToken) = ag_rp_142(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 143: V(0,ParserBasicToken) = ag_rp_143(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 144: V(0,ParserBasicToken) = ag_rp_144(PCB_POINTER); break;
    case 145: V(0,ParserBasicToken) = ag_rp_145(PCB_POINTER); break;
    case 146: V(0,ParserBasicToken) = ag_rp_146(PCB_POINTER); break;
    case 147: ag_rp_147(PCB_POINTER); break;
    case 148: ag_rp_148(PCB_POINTER); break;
    case 149: ag_rp_149(PCB_POINTER); break;
    case 150: ag_rp_150(PCB_POINTER); break;
    case 151: ag_rp_151(PCB_POINTER, V(0,ParserTransport)); break;
    case 152: ag_rp_152(PCB_POINTER, V(0,ParserUserParam)); break;
    case 153: ag_rp_153(PCB_POINTER, V(0,ParserTtlParam)); break;
    case 154: ag_rp_154(PCB_POINTER, V(0,ParserMaddrParam)); break;
    case 155: ag_rp_155(PCB_POINTER, V(0,ParserMethod)); break;
    case 156: ag_rp_156(PCB_POINTER, V(0,ParserLrParamType)); break;
    case 157: ag_rp_157(PCB_POINTER, V(0,ParserCompParam)); break;
    case 158: ag_rp_158(PCB_POINTER, V(0,ParserSigCompIdParam)); break;
    case 159: ag_rp_159(PCB_POINTER, V(0,ParserTokenizedByParam)); break;
    case 160: ag_rp_160(PCB_POINTER, V(0,RvBool)); break;
    case 161: ag_rp_161(PCB_POINTER); break;
    case 162: V(0,ParserTransport) = ag_rp_162(PCB_POINTER, V(4,ParserTransport)); break;
    case 163: V(0,ParserUserParam) = ag_rp_163(PCB_POINTER, V(4,RvSipUserParam)); break;
    case 164: V(0,ParserUserParam) = ag_rp_164(PCB_POINTER, V(4,ParserOtherUser)); break;
    case 165: V(0,RvSipUserParam) = ag_rp_165(PCB_POINTER); break;
    case 166: V(0,RvSipUserParam) = ag_rp_166(PCB_POINTER); break;
    case 167: V(0,ParserOtherUser) = ag_rp_167(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 168: V(0,ParserTtlParam) = ag_rp_168(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 169: V(0,ParserMaddrParam) = ag_rp_169(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 170: V(0,ParserMethod) = ag_rp_170(PCB_POINTER, V(4,ParserMethod)); break;
    case 171: V(0,ParserLrParamType) = ag_rp_171(PCB_POINTER); break;
    case 172: V(0,ParserLrParamType) = ag_rp_172(PCB_POINTER); break;
    case 173: V(0,ParserLrParamType) = ag_rp_173(PCB_POINTER); break;
    case 174: V(0,ParserLrParamType) = ag_rp_174(PCB_POINTER); break;
    case 175: V(0,ParserCompParam) = ag_rp_175(PCB_POINTER, V(4,ParserCompParam)); break;
    case 176: V(0,ParserSigCompIdParam) = ag_rp_176(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 177: V(0,ParserBasicToken) = ag_rp_177(PCB_POINTER); break;
    case 178: V(0,ParserTokenizedByParam) = ag_rp_178(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 179: V(0,RvBool) = ag_rp_179(PCB_POINTER); break;
    case 180: ag_rp_180(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 181: ag_rp_181(PCB_POINTER, V(0,ParserBasicToken), V(1,ParserBasicToken)); break;
    case 182: V(0,ParserBasicToken) = ag_rp_182(PCB_POINTER); break;
    case 183: V(0,ParserBasicToken) = ag_rp_183(PCB_POINTER, V(2,ParserBasicToken)); break;
    case 184: V(0,ParserOptionalHeaders) = ag_rp_184(PCB_POINTER); break;
    case 185: V(0,ParserOptionalHeaders) = ag_rp_185(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 186: V(0,ParserBasicToken) = ag_rp_186(PCB_POINTER); break;
    case 187: V(0,ParserBasicToken) = ag_rp_187(PCB_POINTER); break;
    case 188: V(0,ParserBasicToken) = ag_rp_188(PCB_POINTER); break;
    case 189: V(0,ParserBasicToken) = ag_rp_189(PCB_POINTER); break;
    case 190: V(0,ParserBasicToken) = ag_rp_190(PCB_POINTER); break;
    case 191: ag_rp_191(PCB_POINTER); break;
    case 192: ag_rp_192(PCB_POINTER, V(0,ParserMethod), V(4,ParserSipVersion)); break;
    case 193: V(0,ParserMethod) = ag_rp_193(PCB_POINTER); break;
    case 194: V(0,ParserMethod) = ag_rp_194(PCB_POINTER); break;
    case 195: V(0,ParserMethod) = ag_rp_195(PCB_POINTER); break;
    case 196: V(0,ParserMethod) = ag_rp_196(PCB_POINTER); break;
    case 197: V(0,ParserMethod) = ag_rp_197(PCB_POINTER); break;
    case 198: V(0,ParserMethod) = ag_rp_198(PCB_POINTER); break;
    case 199: V(0,ParserMethod) = ag_rp_199(PCB_POINTER); break;
    case 200: V(0,ParserMethod) = ag_rp_200(PCB_POINTER); break;
    case 201: V(0,ParserMethod) = ag_rp_201(PCB_POINTER); break;
    case 202: V(0,ParserMethod) = ag_rp_202(PCB_POINTER); break;
    case 203: V(0,ParserMethod) = ag_rp_203(PCB_POINTER); break;
    case 204: V(0,ParserMethod) = ag_rp_204(PCB_POINTER); break;
    case 205: V(0,ParserMethod) = ag_rp_205(PCB_POINTER); break;
    case 206: V(0,ParserMethod) = ag_rp_206(PCB_POINTER, V(0,ParserMethod)); break;
    case 207: ag_rp_207(PCB_POINTER); break;
    case 208: ag_rp_208(PCB_POINTER, V(0,ParserAbsoluteUri)); break;
    case 209: ag_rp_209(PCB_POINTER, V(0,ParserAbsoluteUri)); break;
    case 210: ag_rp_210(PCB_POINTER, V(0,ParserAbsoluteUri)); break;
    case 211: ag_rp_211(PCB_POINTER); break;
    case 212: ag_rp_212(PCB_POINTER); break;
    case 213: ag_rp_213(PCB_POINTER, V(2,ParserPartyParams)); break;
    case 214: ag_rp_214(PCB_POINTER); break;
    case 215: ag_rp_215(PCB_POINTER, V(1,ParserPartyParams)); break;
    case 216: ag_rp_216(PCB_POINTER, V(0,ParserDisplayName)); break;
    case 217: ag_rp_217(PCB_POINTER); break;
    case 218: V(0,ParserDisplayName) = ag_rp_218(PCB_POINTER, V(0,ParserQuotedString)); break;
    case 219: V(0,ParserDisplayName) = ag_rp_219(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 220: V(0,ParserBasicToken) = ag_rp_220(PCB_POINTER); break;
    case 221: V(0,ParserBasicToken) = ag_rp_221(PCB_POINTER); break;
    case 222: V(0,ParserTagParam) = ag_rp_222(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 223: V(0,ParserPartyParams) = ag_rp_223(PCB_POINTER, V(2,ParserPartyParams)); break;
    case 224: V(0,ParserPartyParams) = ag_rp_224(PCB_POINTER, V(0,ParserPartyParams), V(3,ParserPartyParams)); break;
    case 225: V(0,ParserPartyParams) = ag_rp_225(PCB_POINTER, V(0,ParserTagParam)); break;
    case 226: V(0,ParserPartyParams) = ag_rp_226(PCB_POINTER); break;
    case 227: ag_rp_227(PCB_POINTER); break;
    case 228: ag_rp_228(PCB_POINTER); break;
    case 229: ag_rp_229(PCB_POINTER); break;
    case 230: ag_rp_230(PCB_POINTER); break;
    case 231: ag_rp_231(PCB_POINTER); break;
    case 232: ag_rp_232(PCB_POINTER); break;
    case 233: ag_rp_233(PCB_POINTER); break;
    case 234: ag_rp_234(PCB_POINTER, V(0,ParserQVal)); break;
    case 235: ag_rp_235(PCB_POINTER, V(0,ParserContactActionType)); break;
    case 236: ag_rp_236(PCB_POINTER); break;
    case 237: ag_rp_237(PCB_POINTER); break;
    case 238: V(0,ParserQVal) = ag_rp_238(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 239: V(0,ParserBasicToken) = ag_rp_239(PCB_POINTER); break;
    case 240: V(0,ParserBasicToken) = ag_rp_240(PCB_POINTER); break;
    case 241: V(0,ParserContactActionType) = ag_rp_241(PCB_POINTER, V(4,ParserContactActionType)); break;
    case 242: V(0,ParserContactActionType) = ag_rp_242(PCB_POINTER); break;
    case 243: V(0,ParserContactActionType) = ag_rp_243(PCB_POINTER); break;
    case 244: ag_rp_244(PCB_POINTER); break;
    case 245: ag_rp_245(PCB_POINTER, V(0,ParserDeltaSeconds)); break;
    case 246: ag_rp_246(PCB_POINTER, V(2,ParserSipDate)); break;
    case 247: ag_rp_247(PCB_POINTER, V(0,ParserDeltaSeconds)); break;
    case 248: ag_rp_248(PCB_POINTER, V(0,ParserSipDate)); break;
    case 249: V(0,ParserDeltaSeconds) = ag_rp_249(PCB_POINTER); break;
    case 250: ag_rp_250(PCB_POINTER, V(0,ParserSipDate)); break;
    case 251: V(0,ParserSipDate) = ag_rp_251(PCB_POINTER, V(0,ParserDDMMYY), V(2,ParserTime)); break;
    case 252: V(0,ParserSipDate) = ag_rp_252(PCB_POINTER, V(0,ParserSipDate)); break;
    case 253: V(0,ParserSipDate) = ag_rp_253(PCB_POINTER, V(0,RvSipDateWeekDay), V(4,ParserDDMMYY), V(6,ParserTime)); break;
    case 254: V(0,RvSipDateWeekDay) = ag_rp_254(PCB_POINTER); break;
    case 255: V(0,RvSipDateWeekDay) = ag_rp_255(PCB_POINTER); break;
    case 256: V(0,RvSipDateWeekDay) = ag_rp_256(PCB_POINTER); break;
    case 257: V(0,RvSipDateWeekDay) = ag_rp_257(PCB_POINTER); break;
    case 258: V(0,RvSipDateWeekDay) = ag_rp_258(PCB_POINTER); break;
    case 259: V(0,RvSipDateWeekDay) = ag_rp_259(PCB_POINTER); break;
    case 260: V(0,RvSipDateWeekDay) = ag_rp_260(PCB_POINTER); break;
    case 261: V(0,ParserDDMMYY) = ag_rp_261(PCB_POINTER, V(0,ParserInt32), V(2,RvSipDateMonth), V(4,ParserInt32)); break;
    case 262: V(0,ParserInt32) = ag_rp_262(PCB_POINTER, V(0,int)); break;
    case 263: V(0,ParserInt32) = ag_rp_263(PCB_POINTER, V(0,int), V(1,int)); break;
    case 264: V(0,RvSipDateMonth) = ag_rp_264(PCB_POINTER); break;
    case 265: V(0,RvSipDateMonth) = ag_rp_265(PCB_POINTER); break;
    case 266: V(0,RvSipDateMonth) = ag_rp_266(PCB_POINTER); break;
    case 267: V(0,RvSipDateMonth) = ag_rp_267(PCB_POINTER); break;
    case 268: V(0,RvSipDateMonth) = ag_rp_268(PCB_POINTER); break;
    case 269: V(0,RvSipDateMonth) = ag_rp_269(PCB_POINTER); break;
    case 270: V(0,RvSipDateMonth) = ag_rp_270(PCB_POINTER); break;
    case 271: V(0,RvSipDateMonth) = ag_rp_271(PCB_POINTER); break;
    case 272: V(0,RvSipDateMonth) = ag_rp_272(PCB_POINTER); break;
    case 273: V(0,RvSipDateMonth) = ag_rp_273(PCB_POINTER); break;
    case 274: V(0,RvSipDateMonth) = ag_rp_274(PCB_POINTER); break;
    case 275: V(0,RvSipDateMonth) = ag_rp_275(PCB_POINTER); break;
    case 276: V(0,ParserInt32) = ag_rp_276(PCB_POINTER, V(0,int), V(1,int)); break;
    case 277: V(0,ParserInt32) = ag_rp_277(PCB_POINTER, V(0,int), V(1,int), V(2,int)); break;
    case 278: V(0,ParserInt32) = ag_rp_278(PCB_POINTER, V(0,int), V(1,int), V(2,int), V(3,int)); break;
    case 279: V(0,ParserTime) = ag_rp_279(PCB_POINTER, V(0,ParserInt32), V(2,ParserInt32), V(4,ParserInt32), V(6,ParserTimeZone)); break;
    case 280: V(0,ParserInt32) = ag_rp_280(PCB_POINTER, V(0,int), V(1,int)); break;
    case 281: V(0,ParserInt32) = ag_rp_281(PCB_POINTER, V(0,int), V(1,int)); break;
    case 282: V(0,ParserInt32) = ag_rp_282(PCB_POINTER, V(0,int), V(1,int)); break;
    case 283: V(0,ParserTimeZone) = ag_rp_283(PCB_POINTER); break;
    case 284: V(0,ParserTimeZone) = ag_rp_284(PCB_POINTER); break;
    case 285: V(0,ParserTimeZone) = ag_rp_285(PCB_POINTER); break;
    case 286: ag_rp_286(PCB_POINTER, V(0,ParserMethod)); break;
    case 287: ag_rp_287(PCB_POINTER, V(0,ParserMethod), V(4,ParserBasicToken)); break;
    case 288: ag_rp_288(PCB_POINTER); break;
    case 289: ag_rp_289(PCB_POINTER); break;
    case 290: ag_rp_290(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 291: ag_rp_291(PCB_POINTER); break;
    case 292: ag_rp_292(PCB_POINTER); break;
    case 293: ag_rp_293(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 294: ag_rp_294(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 295: ag_rp_295(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 296: ag_rp_296(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 297: ag_rp_297(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 298: ag_rp_298(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 299: ag_rp_299(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 300: ag_rp_300(PCB_POINTER, V(0,RvSipAuthStale)); break;
    case 301: ag_rp_301(PCB_POINTER, V(0,ParserAlgorithm)); break;
    case 302: ag_rp_302(PCB_POINTER, V(0,ParserQopValue)); break;
    case 303: ag_rp_303(PCB_POINTER); break;
    case 304: V(0,ParserBasicToken) = ag_rp_304(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 305: V(0,ParserBasicToken) = ag_rp_305(PCB_POINTER, V(0,ParserQuotedString)); break;
    case 306: V(0,ParserBasicToken) = ag_rp_306(PCB_POINTER); break;
    case 307: V(0,ParserBasicToken) = ag_rp_307(PCB_POINTER); break;
    case 308: ag_rp_308(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 309: V(0,ParserBasicToken) = ag_rp_309(PCB_POINTER); break;
    case 310: ag_rp_310(PCB_POINTER); break;
    case 311: ag_rp_311(PCB_POINTER); break;
    case 312: V(0,ParserBasicToken) = ag_rp_312(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 313: V(0,ParserBasicToken) = ag_rp_313(PCB_POINTER, V(0,ParserQuotedString)); break;
    case 314: V(0,ParserBasicToken) = ag_rp_314(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 315: V(0,ParserBasicToken) = ag_rp_315(PCB_POINTER, V(0,ParserQuotedString)); break;
    case 316: V(0,RvSipAuthStale) = ag_rp_316(PCB_POINTER); break;
    case 317: V(0,RvSipAuthStale) = ag_rp_317(PCB_POINTER); break;
    case 318: V(0,ParserAlgorithm) = ag_rp_318(PCB_POINTER, V(1,ParserAlgorithm)); break;
    case 319: ag_rp_319(PCB_POINTER); break;
    case 320: ag_rp_320(PCB_POINTER); break;
    case 321: ag_rp_321(PCB_POINTER); break;
    case 322: ag_rp_322(PCB_POINTER); break;
    case 323: V(0,ParserAlgorithm) = ag_rp_323(PCB_POINTER, V(2,ParserBasicToken), V(4,ParserAlgorithm)); break;
    case 324: V(0,ParserAlgorithm) = ag_rp_324(PCB_POINTER, V(1,ParserAlgorithm)); break;
    case 325: ag_rp_325(PCB_POINTER); break;
    case 326: V(0,ParserAlgorithm) = ag_rp_326(PCB_POINTER); break;
    case 327: V(0,ParserAlgorithm) = ag_rp_327(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 328: ag_rp_328(PCB_POINTER); break;
    case 329: ag_rp_329(PCB_POINTER); break;
    case 330: V(0,ParserBasicToken) = ag_rp_330(PCB_POINTER); break;
    case 331: V(0,ParserQopValue) = ag_rp_331(PCB_POINTER, V(5,ParserQopValue)); break;
    case 332: V(0,ParserQopValue) = ag_rp_332(PCB_POINTER, V(0,ParserAuthQopOptions)); break;
    case 333: V(0,ParserQopValue) = ag_rp_333(PCB_POINTER, V(0,ParserQopValue), V(4,ParserAuthQopOptions)); break;
    case 334: V(0,ParserAuthQopOptions) = ag_rp_334(PCB_POINTER); break;
    case 335: V(0,ParserAuthQopOptions) = ag_rp_335(PCB_POINTER); break;
    case 336: V(0,ParserAuthQopOptions) = ag_rp_336(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 337: V(0,ParserBasicToken) = ag_rp_337(PCB_POINTER, V(3,ParserBasicToken)); break;
    case 338: V(0,ParserBasicToken) = ag_rp_338(PCB_POINTER, V(3,ParserBasicToken)); break;
    case 339: V(0,ParserBasicToken) = ag_rp_339(PCB_POINTER); break;
    case 340: V(0,ParserBasicToken) = ag_rp_340(PCB_POINTER); break;
    case 341: V(0,ParserBasicToken) = ag_rp_341(PCB_POINTER); break;
    case 342: V(0,ParserBasicToken) = ag_rp_342(PCB_POINTER); break;
    case 343: V(0,ParserBasicToken) = ag_rp_343(PCB_POINTER); break;
    case 344: ag_rp_344(PCB_POINTER, V(0,ParserBasicToken), V(2,ParserGenericParamDataVal)); break;
    case 345: ag_rp_345(PCB_POINTER); break;
    case 346: ag_rp_346(PCB_POINTER); break;
    case 347: ag_rp_347(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 348: ag_rp_348(PCB_POINTER); break;
    case 349: ag_rp_349(PCB_POINTER); break;
    case 350: ag_rp_350(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 351: ag_rp_351(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 352: ag_rp_352(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 353: ag_rp_353(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 354: ag_rp_354(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 355: ag_rp_355(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 356: ag_rp_356(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 357: ag_rp_357(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 358: ag_rp_358(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 359: ag_rp_359(PCB_POINTER, V(0,RvSipAuthIntegrityProtected)); break;
    case 360: ag_rp_360(PCB_POINTER, V(0,RvSipAuthQopOption)); break;
    case 361: ag_rp_361(PCB_POINTER); break;
    case 362: ag_rp_362(PCB_POINTER, V(0,ParserAlgorithm)); break;
    case 363: V(0,ParserBasicToken) = ag_rp_363(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 364: V(0,ParserBasicToken) = ag_rp_364(PCB_POINTER, V(0,ParserQuotedString)); break;
    case 365: ag_rp_365(PCB_POINTER); break;
    case 366: ag_rp_366(PCB_POINTER); break;
    case 367: V(0,RvSipAuthQopOption) = ag_rp_367(PCB_POINTER, V(5,ParserAuthQopOptions)); break;
    case 368: V(0,ParserBasicToken) = ag_rp_368(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 369: V(0,ParserBasicToken) = ag_rp_369(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 370: V(0,ParserBasicToken) = ag_rp_370(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 371: V(0,ParserBasicToken) = ag_rp_371(PCB_POINTER); break;
    case 372: V(0,ParserBasicToken) = ag_rp_372(PCB_POINTER); break;
    case 373: V(0,ParserBasicToken) = ag_rp_373(PCB_POINTER); break;
    case 374: V(0,ParserBasicToken) = ag_rp_374(PCB_POINTER); break;
    case 375: V(0,ParserBasicToken) = ag_rp_375(PCB_POINTER); break;
    case 376: V(0,ParserBasicToken) = ag_rp_376(PCB_POINTER); break;
    case 377: V(0,ParserBasicToken) = ag_rp_377(PCB_POINTER); break;
    case 378: V(0,ParserBasicToken) = ag_rp_378(PCB_POINTER); break;
    case 379: V(0,ParserBasicToken) = ag_rp_379(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 380: V(0,ParserBasicToken) = ag_rp_380(PCB_POINTER); break;
    case 381: V(0,ParserBasicToken) = ag_rp_381(PCB_POINTER); break;
    case 382: V(0,ParserBasicToken) = ag_rp_382(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 383: V(0,ParserBasicToken) = ag_rp_383(PCB_POINTER); break;
    case 384: V(0,RvSipAuthIntegrityProtected) = ag_rp_384(PCB_POINTER); break;
    case 385: V(0,RvSipAuthIntegrityProtected) = ag_rp_385(PCB_POINTER); break;
    case 386: ag_rp_386(PCB_POINTER); break;
    case 387: ag_rp_387(PCB_POINTER); break;
    case 388: ag_rp_388(PCB_POINTER); break;
    case 389: ag_rp_389(PCB_POINTER); break;
    case 390: ag_rp_390(PCB_POINTER); break;
    case 391: ag_rp_391(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 392: ag_rp_392(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 393: ag_rp_393(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 394: ag_rp_394(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 395: ag_rp_395(PCB_POINTER); break;
    case 396: ag_rp_396(PCB_POINTER, V(5,ParserAuthQopOptions)); break;
    case 397: V(0,ParserBasicToken) = ag_rp_397(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 398: V(0,ParserBasicToken) = ag_rp_398(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 399: V(0,ParserBasicToken) = ag_rp_399(PCB_POINTER); break;
  }
  (PCB).la_ptr = (PCB).pointer;
}
static int ag_action_1_r_proc(PCB_DECL);
static int ag_action_2_r_proc(PCB_DECL);
static int ag_action_3_r_proc(PCB_DECL);
static int ag_action_4_r_proc(PCB_DECL);
static int ag_action_1_s_proc(PCB_DECL);
static int ag_action_3_s_proc(PCB_DECL);
static int ag_action_1_proc(PCB_DECL);
static int ag_action_2_proc(PCB_DECL);
static int ag_action_3_proc(PCB_DECL);
static int ag_action_4_proc(PCB_DECL);
static int ag_action_5_proc(PCB_DECL);
static int ag_action_6_proc(PCB_DECL);
static int ag_action_7_proc(PCB_DECL);
static int ag_action_8_proc(PCB_DECL);
static int ag_action_9_proc(PCB_DECL);
static int ag_action_10_proc(PCB_DECL);
static int ag_action_11_proc(PCB_DECL);
static int ag_action_8_proc(PCB_DECL);
static int (*const  ag_r_procs_scan[])(PCB_DECL) = {
  ag_action_1_r_proc,
  ag_action_2_r_proc,
  ag_action_3_r_proc,
  ag_action_4_r_proc
};
static int (*const  ag_s_procs_scan[])(PCB_DECL) = {
  ag_action_1_s_proc,
  ag_action_2_r_proc,
  ag_action_3_s_proc,
  ag_action_4_r_proc
};
static int (*const  ag_gt_procs_scan[])(PCB_DECL) = {
  ag_action_1_proc,
  ag_action_2_proc,
  ag_action_3_proc,
  ag_action_4_proc,
  ag_action_5_proc,
  ag_action_6_proc,
  ag_action_7_proc,
  ag_action_8_proc,
  ag_action_9_proc,
  ag_action_10_proc,
  ag_action_11_proc,
  ag_action_8_proc
};
static int ag_action_10_proc(PCB_DECL) {
  int ag_t = (PCB).token_number;
  do {
    ag_track(PCB_POINTER);
    (PCB).token_number = (SipParser_token_type) AG_TCV(INPUT_CODE(*(PCB).la_ptr));
    (PCB).la_ptr++;
    if (ag_key_index[(PCB).sn]) {
      unsigned ag_k = ag_key_index[(PCB).sn];
      int ag_ch = CONVERT_CASE(INPUT_CODE(*(PCB).pointer));
      while (ag_key_ch[ag_k] < ag_ch) ag_k++;
      if (ag_key_ch[ag_k] == ag_ch) ag_get_key_word((PCB_TYPE *)PCB_POINTER, ag_k);
    }
  } while ((PCB).token_number == (SipParser_token_type) ag_t);
  (PCB).la_ptr =  (PCB).pointer;
  return 1;
}
static int ag_action_11_proc(PCB_DECL) {
  int ag_t = (PCB).token_number;
  do {
    (*(int *) &(PCB).vs[(PCB).ssx]) = *(PCB).pointer;
    (PCB).ssx--;
    ag_track(PCB_POINTER);
    ag_ra(PCB_POINTER);
    if ((PCB).exit_flag != AG_RUNNING_CODE) return 0;
    (PCB).ssx++;
    (PCB).token_number = (SipParser_token_type) AG_TCV(INPUT_CODE(*(PCB).la_ptr));
    (PCB).la_ptr++;
    if (ag_key_index[(PCB).sn]) {
      unsigned ag_k = ag_key_index[(PCB).sn];
      int ag_ch = CONVERT_CASE(INPUT_CODE(*(PCB).pointer));
      while (ag_key_ch[ag_k] < ag_ch) ag_k++;
      if (ag_key_ch[ag_k] == ag_ch) ag_get_key_word((PCB_TYPE *)PCB_POINTER, ag_k);
    }
  }
  while ((PCB).token_number == (SipParser_token_type) ag_t);
  (PCB).la_ptr =  (PCB).pointer;
  return 1;
}
static int ag_action_3_r_proc(PCB_DECL) {
  int ag_sd = ag_fl[(PCB).ag_ap] - 1;
  if (ag_sd) (PCB).sn = (PCB).ss[(PCB).ssx -= ag_sd];
  (PCB).reduction_token = (SipParser_token_type) ag_ptt[(PCB).ag_ap];
  ag_ra(PCB_POINTER);
  return (PCB).exit_flag == AG_RUNNING_CODE;
}
static int ag_action_3_s_proc(PCB_DECL) {
  int ag_sd = ag_fl[(PCB).ag_ap] - 1;
  if (ag_sd) (PCB).sn = (PCB).ss[(PCB).ssx -= ag_sd];
  (PCB).reduction_token = (SipParser_token_type) ag_ptt[(PCB).ag_ap];
  ag_ra(PCB_POINTER);
  return (PCB).exit_flag == AG_RUNNING_CODE;;
}
static int ag_action_4_r_proc(PCB_DECL) {
  int ag_sd = ag_fl[(PCB).ag_ap] - 1;
  if (ag_sd) (PCB).sn = (PCB).ss[(PCB).ssx -= ag_sd];
  (PCB).reduction_token = (SipParser_token_type) ag_ptt[(PCB).ag_ap];
  return 1;
}
static int ag_action_2_proc(PCB_DECL) {
  if ((PCB).ssx >= 43) {
    (PCB).exit_flag = AG_STACK_ERROR_CODE;
    PARSER_STACK_OVERFLOW;
  }
  (*(int *) &(PCB).vs[(PCB).ssx]) = *(PCB).pointer;
  (PCB).ss[(PCB).ssx] = (PCB).sn;
  (PCB).ssx++;
  (PCB).sn = (PCB).ag_ap;
  ag_track(PCB_POINTER);
  return 0;
}
static int ag_action_9_proc(PCB_DECL) {
  if ((PCB).ssx >= 43) {
    (PCB).exit_flag = AG_STACK_ERROR_CODE;
    PARSER_STACK_OVERFLOW;
  }
  (PCB).vs[(PCB).ssx] = ag_null_value;
  (PCB).ss[(PCB).ssx] = (PCB).sn;
  (PCB).ssx++;
  (PCB).sn = (PCB).ag_ap;
  (PCB).la_ptr =  (PCB).pointer;
  return (PCB).exit_flag == AG_RUNNING_CODE;
}
static int ag_action_2_r_proc(PCB_DECL) {
  (PCB).ssx++;
  (PCB).sn = (PCB).ag_ap;
  return 0;
}
static int ag_action_7_proc(PCB_DECL) {
  --(PCB).ssx;
  (PCB).la_ptr =  (PCB).pointer;
  (PCB).exit_flag = AG_SUCCESS_CODE;
  return 0;
}
static int ag_action_1_proc(PCB_DECL) {
  ag_track(PCB_POINTER);
  (PCB).exit_flag = AG_SUCCESS_CODE;
  return 0;
}
static int ag_action_1_r_proc(PCB_DECL) {
  (PCB).exit_flag = AG_SUCCESS_CODE;
  return 0;
}
static int ag_action_1_s_proc(PCB_DECL) {
  (PCB).exit_flag = AG_SUCCESS_CODE;
  return 0;
}
static int ag_action_4_proc(PCB_DECL) {
  int ag_sd = ag_fl[(PCB).ag_ap] - 1;
  (PCB).reduction_token = (SipParser_token_type) ag_ptt[(PCB).ag_ap];
  (*(int *) &(PCB).vs[(PCB).ssx]) = *(PCB).pointer;
  if (ag_sd) (PCB).sn = (PCB).ss[(PCB).ssx -= ag_sd];
  else (PCB).ss[(PCB).ssx] = (PCB).sn;
  ag_track(PCB_POINTER);
  while ((PCB).exit_flag == AG_RUNNING_CODE) {
    unsigned ag_t1 = ag_sbe[(PCB).sn] + 1;
    unsigned ag_t2 = ag_sbt[(PCB).sn+1] - 1;
    do {
      unsigned ag_tx = (ag_t1 + ag_t2)/2;
      if (ag_tstt[ag_tx] < (const unsigned short)(PCB).reduction_token) ag_t1 = ag_tx + 1;
      else ag_t2 = ag_tx;
    } while (ag_t1 < ag_t2);
    (PCB).ag_ap = ag_pstt[ag_t1];
    if ((ag_s_procs_scan[ag_astt[ag_t1]])(PCB_POINTER) == 0) break;
  }
  return 0;
}
static int ag_action_3_proc(PCB_DECL) {
  int ag_sd = ag_fl[(PCB).ag_ap] - 1;
  (*(int *) &(PCB).vs[(PCB).ssx]) = *(PCB).pointer;
  if (ag_sd) (PCB).sn = (PCB).ss[(PCB).ssx -= ag_sd];
  else (PCB).ss[(PCB).ssx] = (PCB).sn;
  ag_track(PCB_POINTER);
  (PCB).reduction_token = (SipParser_token_type) ag_ptt[(PCB).ag_ap];
  ag_ra(PCB_POINTER);
  while ((PCB).exit_flag == AG_RUNNING_CODE) {
    unsigned ag_t1 = ag_sbe[(PCB).sn] + 1;
    unsigned ag_t2 = ag_sbt[(PCB).sn+1] - 1;
    do {
      unsigned ag_tx = (ag_t1 + ag_t2)/2;
      if (ag_tstt[ag_tx] < (const unsigned short)(PCB).reduction_token) ag_t1 = ag_tx + 1;
      else ag_t2 = ag_tx;
    } while (ag_t1 < ag_t2);
    (PCB).ag_ap = ag_pstt[ag_t1];
    if ((ag_s_procs_scan[ag_astt[ag_t1]])(PCB_POINTER) == 0) break;
  }
  return 0;
}
static int ag_action_8_proc(PCB_DECL) {
  (PCB).la_ptr =  (PCB).pointer;
  (PCB).exit_flag = AG_SYNTAX_ERROR_CODE;
  SYNTAX_ERROR;
  {(PCB).la_ptr = (PCB).pointer + 1; ag_track(PCB_POINTER);}
  return (PCB).exit_flag == AG_RUNNING_CODE;
}
static int ag_action_5_proc(PCB_DECL) {
  int ag_sd = ag_fl[(PCB).ag_ap];
  if (ag_sd) (PCB).sn = (PCB).ss[(PCB).ssx -= ag_sd];
  else {
    if ((PCB).ssx >= 43) {
      (PCB).exit_flag = AG_STACK_ERROR_CODE;
      PARSER_STACK_OVERFLOW;
    }
    (PCB).ss[(PCB).ssx] = (PCB).sn;
  }
  (PCB).la_ptr =  (PCB).pointer;
  (PCB).reduction_token = (SipParser_token_type) ag_ptt[(PCB).ag_ap];
  ag_ra(PCB_POINTER);
  while ((PCB).exit_flag == AG_RUNNING_CODE) {
    unsigned ag_t1 = ag_sbe[(PCB).sn] + 1;
    unsigned ag_t2 = ag_sbt[(PCB).sn+1] - 1;
    do {
      unsigned ag_tx = (ag_t1 + ag_t2)/2;
      if (ag_tstt[ag_tx] < (const unsigned short)(PCB).reduction_token) ag_t1 = ag_tx + 1;
      else ag_t2 = ag_tx;
    } while (ag_t1 < ag_t2);
    (PCB).ag_ap = ag_pstt[ag_t1];
    if ((ag_r_procs_scan[ag_astt[ag_t1]])(PCB_POINTER) == 0) break;
  }
  return (PCB).exit_flag == AG_RUNNING_CODE;
}
static int ag_action_6_proc(PCB_DECL) {
  int ag_sd = ag_fl[(PCB).ag_ap];
  (PCB).reduction_token = (SipParser_token_type) ag_ptt[(PCB).ag_ap];
  if (ag_sd) {
    (PCB).sn = (PCB).ss[(PCB).ssx -= ag_sd];
  }
  else {
    if ((PCB).ssx >= 43) {
      (PCB).exit_flag = AG_STACK_ERROR_CODE;
      PARSER_STACK_OVERFLOW;
    }
    (PCB).vs[(PCB).ssx] = ag_null_value;
    (PCB).ss[(PCB).ssx] = (PCB).sn;
  }
  (PCB).la_ptr =  (PCB).pointer;
  while ((PCB).exit_flag == AG_RUNNING_CODE) {
    unsigned ag_t1 = ag_sbe[(PCB).sn] + 1;
    unsigned ag_t2 = ag_sbt[(PCB).sn+1] - 1;
    do {
      unsigned ag_tx = (ag_t1 + ag_t2)/2;
      if (ag_tstt[ag_tx] < (const unsigned short)(PCB).reduction_token) ag_t1 = ag_tx + 1;
      else ag_t2 = ag_tx;
    } while (ag_t1 < ag_t2);
    (PCB).ag_ap = ag_pstt[ag_t1];
    if ((ag_r_procs_scan[ag_astt[ag_t1]])(PCB_POINTER) == 0) break;
  }
  return (PCB).exit_flag == AG_RUNNING_CODE;
}
void init_SipParser(SipParser_pcb_type *PCB_POINTER) {
  (PCB).la_ptr = (PCB).pointer;
  (PCB).error_message = "Syntax Error";
  (PCB).ss[0] = (PCB).sn = (PCB).ssx = 0;
  (PCB).exit_flag = AG_RUNNING_CODE;
  (PCB).line = FIRST_LINE;
  (PCB).column = FIRST_COLUMN;
}
void SipParser(SipParser_pcb_type *PCB_POINTER) {
  init_SipParser(PCB_POINTER);
  (PCB).exit_flag = AG_RUNNING_CODE;
  while ((PCB).exit_flag == AG_RUNNING_CODE) {
    unsigned ag_t1 = ag_sbt[(PCB).sn];
    if (ag_tstt[ag_t1]) {
      unsigned ag_t2 = ag_sbe[(PCB).sn] - 1;
      (PCB).token_number = (SipParser_token_type) AG_TCV(INPUT_CODE(*(PCB).la_ptr));
      (PCB).la_ptr++;
      if (ag_key_index[(PCB).sn]) {
        unsigned ag_k = ag_key_index[(PCB).sn];
        int ag_ch = CONVERT_CASE(INPUT_CODE(*(PCB).pointer));
        while (ag_key_ch[ag_k] < ag_ch) ag_k++;
        if (ag_key_ch[ag_k] == ag_ch) ag_get_key_word((PCB_TYPE *)PCB_POINTER, ag_k);
      }
      do {
        unsigned ag_tx = (ag_t1 + ag_t2)/2;
        if (ag_tstt[ag_tx] > (const unsigned short)(PCB).token_number)
          ag_t1 = ag_tx + 1;
        else ag_t2 = ag_tx;
      } while (ag_t1 < ag_t2);
      if (ag_tstt[ag_t1] != (const unsigned short)(PCB).token_number)
        ag_t1 = ag_sbe[(PCB).sn];
    }
    (PCB).ag_ap = ag_pstt[ag_t1];
    (ag_gt_procs_scan[ag_astt[ag_t1]])((PCB_TYPE *)PCB_POINTER);
  }
}
#endif
