#if defined(RV_SIP_LIGHT) && defined(RV_SIP_PRIMITIVES) && !defined(RV_SIP_IMS_DCS_HEADER_SUPPORT) && !defined(RV_SIP_IMS_HEADER_SUPPORT) && !defined(RV_SIP_EXTENDED_HEADER_SUPPORT) && !defined(RV_SIP_JSR32_SUPPORT) && !defined(RV_SIP_TEL_URI_SUPPORT) && !defined(RV_SIP_AUTH_ON) && !defined(RV_SIP_SUBS_ON)
/*
 AnaGram Parsing Engine
 Copyright (c) 1993-1999, Parsifal Software.
 All Rights Reserved.
 Serial number 2P20671
 Registered to:
   Meir Fuchs
   RADVision Ltd.
*/
#ifndef PARSERENGINESIPLIGHT_H_1248033498
#include ".\ParserEngineSipLight.h"
#endif
#ifndef PARSERENGINESIPLIGHT_H_1248033498
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
#define ag_rp_15(PCB_POINTER) (CUR_STRING())
#define ag_rp_16(PCB_POINTER) (CUR_STRING())
#define ag_rp_17(PCB_POINTER) (CUR_STRING())
#define ag_rp_18(PCB_POINTER) (CUR_STRING())
#define ag_rp_19(PCB_POINTER) (CUR_STRING())
#define ag_rp_20(PCB_POINTER) (CUR_METHOD(PARSER_METHOD_TYPE_INVITE))
#define ag_rp_21(PCB_POINTER) (CUR_METHOD(PARSER_METHOD_TYPE_ACK))
#define ag_rp_22(PCB_POINTER) (CUR_METHOD(PARSER_METHOD_TYPE_BYE))
#define ag_rp_23(PCB_POINTER) (CUR_METHOD(PARSER_METHOD_TYPE_CANCEL))
#define ag_rp_24(PCB_POINTER) (CUR_METHOD(PARSER_METHOD_TYPE_REGISTER))
static ParserMethod ag_rp_25(PCB_DECL, ParserBasicToken t) {
/* Line 515, RV SIP Stack */
 ParserMethod method;
                            method.type = PARSER_METHOD_TYPE_OTHER;
                            method.other = t;
                            RV_UNUSED_ARG(pcb_pointer);
                            return method;
                          
}
static void ag_rp_26(PCB_DECL, ParserContentLength contentLength) {
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
static ParserContentLength ag_rp_27(PCB_DECL) {
/* Line 597, RV SIP Stack */
        ParserContentLength contentLength;
        contentLength.contentLenVal = CUR_STRING();
        contentLength.isCompact = RV_FALSE;
        return contentLength;
    
}
static void ag_rp_28(PCB_DECL, ParserBasicToken token) {
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
static void ag_rp_29(PCB_DECL, ParserBasicToken token1, ParserGenericParamDataVal token2) {
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
#define ag_rp_30(PCB_POINTER, token) (token)
#define ag_rp_31(PCB_POINTER, qs) (qs)
static void ag_rp_32(PCB_DECL, ParserSipVersion ver, ParserStatusCode status, ParserReasonPhrase reason) {
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
static void ag_rp_33(PCB_DECL, ParserSipVersion ver, ParserStatusCode status) {
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
#define ag_rp_34(PCB_POINTER) (CUR_STRING())
#define ag_rp_35(PCB_POINTER) (CUR_STRING())
static ParserStatusCode ag_rp_36(PCB_DECL, int D100, int D10, int D1) {
/* Line 1132, RV SIP Stack */
        RvUint16 code;
        RV_UNUSED_ARG(pcb_pointer);
        code =  (RvUint16)((D100 - '0')*100 + (D10 - '0')*10 + (D1 - '0'));
        return code;
    
}
static void ag_rp_37(PCB_DECL) {
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
static void ag_rp_38(PCB_DECL) {
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
static void ag_rp_39(PCB_DECL) {
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
static void ag_rp_40(PCB_DECL) {
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
static void ag_rp_41(PCB_DECL, ParserViaParam p) {
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
static void ag_rp_42(PCB_DECL, ParserProtocolName pn, ParserProtocolVersion pv, ParserTransport t) {
/* Line 1327, RV SIP Stack */
 
        PCB.sentProtocol.protocolName    = pn;
        PCB.sentProtocol.protocolVersion = pv;
        PCB.sentProtocol.transport       = t;
    
}
#define ag_rp_43(PCB_POINTER, t) (t)
#define ag_rp_44(PCB_POINTER) (CUR_STRING())
#define ag_rp_45(PCB_POINTER, t) (t)
#define ag_rp_46(PCB_POINTER, t) (t)
static ParserTransport ag_rp_47(PCB_DECL) {
/* Line 1354, RV SIP Stack */
        ParserTransport t;
        t.transport    = RVSIP_TRANSPORT_UDP;
        t.strTransport = CUR_STRING();
        return t;
    
}
static ParserTransport ag_rp_48(PCB_DECL) {
/* Line 1361, RV SIP Stack */
        ParserTransport t;
        t.transport    = RVSIP_TRANSPORT_TCP;
        t.strTransport = CUR_STRING();
        return t;
    
}
static ParserTransport ag_rp_49(PCB_DECL) {
/* Line 1368, RV SIP Stack */
        ParserTransport t;
        t.transport    = RVSIP_TRANSPORT_SCTP;
        t.strTransport = CUR_STRING();
        return t;
    
}
static ParserTransport ag_rp_50(PCB_DECL) {
/* Line 1375, RV SIP Stack */
        ParserTransport t;
        t.transport    = RVSIP_TRANSPORT_TLS;
        t.strTransport = CUR_STRING();
        return t;
    
}
static ParserTransport ag_rp_51(PCB_DECL, ParserBasicToken token) {
/* Line 1382, RV SIP Stack */
        ParserTransport t;
        t.transport    = RVSIP_TRANSPORT_OTHER;
        t.strTransport = token;
        RV_UNUSED_ARG(pcb_pointer);
        return t;
    
}
#define ag_rp_52(PCB_POINTER) (CUR_STRING())
static void ag_rp_53(PCB_DECL, ParserBasicToken host) {
/* Line 1401, RV SIP Stack */
 
        
        PCB.viaSentBy.isPort   = RV_FALSE;
        PCB.viaSentBy.hostName = host;
     
}
static void ag_rp_54(PCB_DECL, ParserBasicToken host, ParserPort port) {
/* Line 1407, RV SIP Stack */
 
        
        PCB.viaSentBy.isPort   = RV_TRUE;
        PCB.viaSentBy.hostName = host;
        PCB.viaSentBy.port     = port;
      
}
static ParserViaParam ag_rp_55(PCB_DECL) {
/* Line 1421, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_HIDDEN;                   
          viaParam.param.hidden = RV_TRUE;
	      viaParam.isOther = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_56(PCB_DECL) {
/* Line 1430, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_ALIAS;                   
          viaParam.param.alias = RV_TRUE;
	      viaParam.isOther = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_57(PCB_DECL, ParserTTLParam t) {
/* Line 1439, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_TTL;                   
          viaParam.param.ttl = t;
	      viaParam.isOther = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_58(PCB_DECL, ParserMaddrParam ma) {
/* Line 1448, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_MADDR;                   
          viaParam.param.maddr = ma;
	      viaParam.isOther = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_59(PCB_DECL, ParserReceived r) {
/* Line 1457, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_RECEIVED;                   
          viaParam.param.received = r;
	      viaParam.isOther = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_60(PCB_DECL, ParserBranch b) {
/* Line 1466, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_BRANCH;                   
          viaParam.param.branch = b;
	      viaParam.isOther = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_61(PCB_DECL, ParserPort rport) {
/* Line 1476, RV SIP Stack */
          ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_RPORT;                   
          viaParam.param.rport = rport;
	      viaParam.isOther = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_62(PCB_DECL, ParserCompParam comp) {
/* Line 1485, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType  = PARSER_PARAM_TYPE_COMP;                   
          viaParam.param.comp = comp;
	      viaParam.isOther    = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_63(PCB_DECL, ParserSigCompIdParam sigcompid) {
/* Line 1495, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType  = PARSER_PARAM_TYPE_SIGCOMPID;                   
          viaParam.param.sigCompId = sigcompid;
	      viaParam.isOther    = RV_FALSE;
          viaParam.otherParam = NULL;
          RV_UNUSED_ARG(pcb_pointer);
          return viaParam;
        
}
static ParserViaParam ag_rp_64(PCB_DECL) {
/* Line 1505, RV SIP Stack */
 ParserViaParam viaParam;
          viaParam.paramType = PARSER_PARAM_TYPE_OTHER; 
	      viaParam.isOther = RV_TRUE;
          viaParam.otherParam = PCB.pExtParams;
          return viaParam;    
        
}
static void ag_rp_65(PCB_DECL, ParserBasicToken token) {
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
static void ag_rp_66(PCB_DECL, ParserBasicToken token, ParserBasicToken token2) {
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
static void ag_rp_67(PCB_DECL, ParserBasicToken token, ParserQuotedString token2) {
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
static void ag_rp_68(PCB_DECL, ParserBasicToken token, ParserBasicToken token2) {
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
#define ag_rp_69(PCB_POINTER, t) (t)
#define ag_rp_70(PCB_POINTER, ttl) (ttl)
#define ag_rp_71(PCB_POINTER) (CUR_STRING())
#define ag_rp_72(PCB_POINTER) (CUR_STRING())
#define ag_rp_73(PCB_POINTER) (CUR_STRING())
#define ag_rp_74(PCB_POINTER, rport) (rport)
static ParserPort ag_rp_75(PCB_DECL) {
/* Line 1634, RV SIP Stack */
        ParserPort port;
        port.len = 0;
		port.buf = NULL;
        RV_UNUSED_ARG(pcb_pointer);
        return port;
    
}
#define ag_rp_76(PCB_POINTER, port) (port)
#define ag_rp_77(PCB_POINTER, ma) (ma)
#define ag_rp_78(PCB_POINTER, host) (host)
#define ag_rp_79(PCB_POINTER, host) (host)
#define ag_rp_80(PCB_POINTER, t) (t)
#define ag_rp_81(PCB_POINTER, comp) (comp)
static ParserCompParam ag_rp_82(PCB_DECL) {
/* Line 1671, RV SIP Stack */
        ParserCompParam comp;
        comp.compType     = RVSIP_COMP_SIGCOMP;
        comp.strCompParam = CUR_STRING();
        return comp;
    
}
static ParserCompParam ag_rp_83(PCB_DECL, ParserBasicToken token) {
/* Line 1678, RV SIP Stack */
        ParserCompParam comp;
        comp.compType     = RVSIP_COMP_OTHER;
        comp.strCompParam = CUR_STRING();
        RV_UNUSED_ARG(token.buf);
        return comp;
    
}
#define ag_rp_84(PCB_POINTER, sigCompId) (sigCompId)
#define ag_rp_85(PCB_POINTER) (CUR_STRING())
static void ag_rp_86(PCB_DECL, ParserAbsoluteUri uri) {
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
static ParserAbsoluteUri ag_rp_87(PCB_DECL, ParserBasicToken scheme, ParserBasicToken id) {
/* Line 1947, RV SIP Stack */
        ParserAbsoluteUri uri;
        uri.scheme = scheme;
        uri.idefntifier = id;
        RV_UNUSED_ARG(pcb_pointer);
        return uri;
    
}
#define ag_rp_88(PCB_POINTER) (CUR_STRING())
#define ag_rp_89(PCB_POINTER, displayName) (displayName)
#define ag_rp_90(PCB_POINTER) (CUR_STRING())
#define ag_rp_91(PCB_POINTER) (CUR_STRING())
static void ag_rp_92(PCB_DECL) {
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
#define ag_rp_93(PCB_POINTER, uri) (INIT_PRES_IM_STRUCT(uri))
#define ag_rp_94(PCB_POINTER, uri) (INIT_PRES_IM_STRUCT(uri))
static void ag_rp_95(PCB_DECL) {
/* Line 2614, RV SIP Stack */
        PCB.sipUrl.bIsSecure = RV_FALSE;
        PCB.sipUrl.bOldAddrSpec = RV_FALSE;
        SIP_BEGIN_TOKEN();
        DEFINE_URL_PREFIX();
    
}
static void ag_rp_96(PCB_DECL) {
/* Line 2621, RV SIP Stack */
        PCB.sipUrl.bIsSecure = RV_TRUE;
        PCB.sipUrl.bOldAddrSpec = RV_FALSE;
        SIP_BEGIN_TOKEN();
        DEFINE_URL_PREFIX();
    
}
#define ag_rp_97(PCB_POINTER, uri) (uri)
static ParserAbsoluteUri ag_rp_98(PCB_DECL) {
/* Line 2635, RV SIP Stack */
        ParserAbsoluteUri uri;
        
        uri.scheme.len = 0;
		uri.scheme.buf = NULL;
        uri.idefntifier.len = 0;
		uri.idefntifier.buf = NULL;
        RV_UNUSED_ARG(pcb_pointer)
        return uri;
    
}
#define ag_rp_99(PCB_POINTER) (SET_PARSER_PCB_BACK_TO_BUFF(RV_SIPPARSER_PREFIX_RV_ABS))
static void ag_rp_100(PCB_DECL) {
/* Line 2651, RV SIP Stack */
        SIP_BEGIN_TOKEN();
        DEFINE_PRES_IM_PREFIX(RV_FALSE);
    
}
#define ag_rp_101(PCB_POINTER, uri) (uri)
static ParserAbsoluteUri ag_rp_102(PCB_DECL) {
/* Line 2661, RV SIP Stack */
        ParserAbsoluteUri uri;
        
        uri.scheme.len = 0;
        uri.scheme.buf = NULL;
        uri.idefntifier.len = 0;
		uri.idefntifier.buf = NULL;
        RV_UNUSED_ARG(pcb_pointer)
        return uri;
    
}
static void ag_rp_103(PCB_DECL) {
/* Line 2674, RV SIP Stack */
        SIP_BEGIN_TOKEN();
        DEFINE_PRES_IM_PREFIX(RV_TRUE);
    
}
static void ag_rp_104(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2682, RV SIP Stack */
  
           
         PCB.sipUrl.optionalHeaders = hs;
         
         if (0 != ((ParserExtensionString *)PCB.pUrlExtParams)->size)
         {
              PCB.sipUrl.urlParameters.genericParamList= PCB.pUrlExtParams;
              PCB.sipUrl.urlParameters.isOtherParams   = RV_TRUE;
         }          
  	
}
static void ag_rp_105(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2693, RV SIP Stack */
  
         
       PCB.sipUrl.optionalHeaders = hs;
       
       if (0 != ((ParserExtensionString *)PCB.pUrlExtParams)->size)
       {
            PCB.sipUrl.urlParameters.genericParamList=PCB.pUrlExtParams;
            PCB.sipUrl.urlParameters.isOtherParams   = RV_TRUE;
       }          
	
}
static void ag_rp_106(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2726, RV SIP Stack */
 
        RESET_URI_PARAMS();
        PCB.sipUrl.optionalHeaders.isSpecified = RV_FALSE;
        PCB.sipUrl.optionalHeaders = hs;
        PCB.exUri.uriType = PARSER_ADDR_PARAM_TYPE_SIP_URL;
        PCB.exUri.ExUriInfo.SipUrl = PCB.sipUrl;
      
}
static void ag_rp_107(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2735, RV SIP Stack */
 
        RESET_URI_PARAMS();
        PCB.sipUrl.optionalHeaders.isSpecified = RV_FALSE;
        PCB.sipUrl.optionalHeaders = hs;
        PCB.exUri.uriType = PARSER_ADDR_PARAM_TYPE_SIP_URL;
        PCB.exUri.ExUriInfo.SipUrl = PCB.sipUrl;
      
}
static void ag_rp_108(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2775, RV SIP Stack */
 
        RESET_URI_PARAMS();
        PCB.sipUrl.optionalHeaders.isSpecified = RV_FALSE;
        PCB.sipUrl.optionalHeaders = hs;
        PCB.exUri.uriType = PARSER_ADDR_PARAM_TYPE_PRES_URI;
        PCB.exUri.ExUriInfo.SipUrl = PCB.sipUrl;
      
}
static void ag_rp_109(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2784, RV SIP Stack */
 
        RESET_URI_PARAMS();
        PCB.sipUrl.optionalHeaders.isSpecified = RV_FALSE;
        PCB.sipUrl.optionalHeaders = hs;
        
        PCB.exUri.uriType = PARSER_ADDR_PARAM_TYPE_PRES_URI;
        PCB.exUri.ExUriInfo.SipUrl = PCB.sipUrl;
      
}
static void ag_rp_110(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2796, RV SIP Stack */
 
        RESET_URI_PARAMS();
        PCB.sipUrl.optionalHeaders.isSpecified = RV_FALSE;
        PCB.sipUrl.optionalHeaders = hs;
        PCB.exUri.uriType = PARSER_ADDR_PARAM_TYPE_IM_URI;
        PCB.exUri.ExUriInfo.SipUrl = PCB.sipUrl;
      
}
static void ag_rp_111(PCB_DECL, ParserOptionalHeaders hs) {
/* Line 2805, RV SIP Stack */
 
        RESET_URI_PARAMS();
        PCB.sipUrl.optionalHeaders.isSpecified = RV_FALSE;
        PCB.sipUrl.optionalHeaders = hs;
        
        PCB.exUri.uriType = PARSER_ADDR_PARAM_TYPE_IM_URI;
        PCB.exUri.ExUriInfo.SipUrl = PCB.sipUrl;
      
}
static void ag_rp_112(PCB_DECL, ParserDisplayName scheme, ParserBasicToken id) {
/* Line 2819, RV SIP Stack */
          PCB.exUri.ExUriInfo.absUri.scheme = scheme;
          PCB.exUri.ExUriInfo.absUri.idefntifier = id;
          PCB.exUri.uriType = PARSER_ADDR_PARAM_TYPE_ABS_URI;
      
}
static void ag_rp_113(PCB_DECL) {
/* Line 2829, RV SIP Stack */
        PCB.sipUrl.bIsUserInfo = RV_TRUE;
    
}
static void ag_rp_114(PCB_DECL) {
/* Line 2834, RV SIP Stack */
        PCB.sipUrl.bIsUserInfo = RV_FALSE;
    
}
static void ag_rp_115(PCB_DECL) {
/* Line 2841, RV SIP Stack */
        SET_PARSER_PCB_BACK_TO_BUFF(RV_SIPPARSER_PREFIX_USERINFO);
    
}
static void ag_rp_116(PCB_DECL, ParserBasicToken user) {
/* Line 2847, RV SIP Stack */
        PCB.sipUrl.urlUserInfo.userName   = user;
        PCB.sipUrl.urlUserInfo.isPassword = RV_FALSE;
    
}
static void ag_rp_117(PCB_DECL, ParserBasicToken user, ParserBasicToken pass) {
/* Line 2852, RV SIP Stack */
        PCB.sipUrl.urlUserInfo.userName   = user;
        PCB.sipUrl.urlUserInfo.isPassword = RV_TRUE;
        PCB.sipUrl.urlUserInfo.password   = pass;
     
}
#define ag_rp_118(PCB_POINTER, t) (t)
#define ag_rp_119(PCB_POINTER) (CUR_STRING())
#define ag_rp_120(PCB_POINTER) (CUR_STRING())
static void ag_rp_121(PCB_DECL) {
/* Line 2885, RV SIP Stack */
        PCB.sipUrl.bIsUserInfo = RV_FALSE;
    
}
static void ag_rp_122(PCB_DECL) {
/* Line 2891, RV SIP Stack */
        SET_PARSER_PCB_BACK_TO_BUFF(RV_SIPPARSER_PREFIX_HOST);
    
}
static void ag_rp_123(PCB_DECL, ParserBasicToken host) {
/* Line 2899, RV SIP Stack */
        PCB.sipUrl.urlHost.hostName = host;
        PCB.sipUrl.urlHost.isPort   = RV_FALSE;
    
}
static void ag_rp_124(PCB_DECL, ParserBasicToken host, ParserPort port) {
/* Line 2905, RV SIP Stack */
        PCB.sipUrl.urlHost.hostName = host;
        PCB.sipUrl.urlHost.isPort   = RV_TRUE;
        PCB.sipUrl.urlHost.port     = port;
    
}
#define ag_rp_125(PCB_POINTER, t) (t)
#define ag_rp_126(PCB_POINTER, ip6ref) (ip6ref)
#define ag_rp_127(PCB_POINTER) (CUR_STRING())
#define ag_rp_128(PCB_POINTER) (CUR_STRING())
#define ag_rp_129(PCB_POINTER) (CUR_STRING())
#define ag_rp_130(PCB_POINTER) (RESET_URI_PARAMS())
static void ag_rp_131(PCB_DECL) {
/* Line 2959, RV SIP Stack */
	
       PCB.sipUrl.urlParameters.isValid = RV_TRUE;
    
}
static void ag_rp_132(PCB_DECL) {
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
static void ag_rp_133(PCB_DECL) {
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
static void ag_rp_134(PCB_DECL, ParserTransport t) {
/* Line 3097, RV SIP Stack */
 
         PCB.urlParameter.paramType = PARSER_TRANSPORT_PARAM;
         PCB.urlParameter.param.transport = t;
    
}
static void ag_rp_135(PCB_DECL, ParserUserParam u) {
/* Line 3102, RV SIP Stack */
 
	     PCB.urlParameter.paramType = PARSER_USER_PARAM;
		 PCB.urlParameter.param.userParam = u;
	
}
static void ag_rp_136(PCB_DECL, ParserTtlParam t) {
/* Line 3107, RV SIP Stack */
 
	     PCB.urlParameter.paramType = PARSER_TTL_PARAM;
		 PCB.urlParameter.param.ttlParam = t;
	
}
static void ag_rp_137(PCB_DECL, ParserMaddrParam ma) {
/* Line 3112, RV SIP Stack */
 
	     PCB.urlParameter.paramType = PARSER_MADDR_PARAM;
		 PCB.urlParameter.param.maddrParam = ma;
	
}
static void ag_rp_138(PCB_DECL, ParserMethod met) {
/* Line 3118, RV SIP Stack */
	     PCB.urlParameter.paramType = PARSER_METHOD_PARAM;
		 PCB.urlParameter.param.methodParam =met;
    
}
static void ag_rp_139(PCB_DECL, ParserLrParamType lrParam) {
/* Line 3123, RV SIP Stack */
	     PCB.urlParameter.paramType = PARSER_LR_PARAM;
		 PCB.urlParameter.param.lrParamType   = lrParam;
    
}
static void ag_rp_140(PCB_DECL, ParserCompParam comp) {
/* Line 3128, RV SIP Stack */
        PCB.urlParameter.paramType       = PARSER_COMP_PARAM;
        PCB.urlParameter.param.compParam = comp;
    
}
static void ag_rp_141(PCB_DECL, ParserSigCompIdParam sigCompId) {
/* Line 3133, RV SIP Stack */
		PCB.urlParameter.paramType = PARSER_SIGCOMPID_PARAM;
		PCB.urlParameter.param.sigCompIdParam = sigCompId;
	
}
static void ag_rp_142(PCB_DECL, ParserTokenizedByParam tokenizedBy) {
/* Line 3138, RV SIP Stack */
        PCB.urlParameter.paramType       = PARSER_TOKENIZED_BY_PARAM;
        PCB.urlParameter.param.tokenizedByParam = tokenizedBy;
    
}
static void ag_rp_143(PCB_DECL, RvBool orig) {
/* Line 3143, RV SIP Stack */
        PCB.urlParameter.paramType        = PARSER_ORIG_PARAM;
        PCB.urlParameter.param.bOrigParam = orig;
    
}
static void ag_rp_144(PCB_DECL) {
/* Line 3165, RV SIP Stack */
	     PCB.urlParameter.paramType = PARSER_OTHER_PARAM;
    
}
#define ag_rp_145(PCB_POINTER, t) (t)
static ParserUserParam ag_rp_146(PCB_DECL, RvSipUserParam pt) {
/* Line 3180, RV SIP Stack */
 
        ParserUserParam userParam;
	    userParam.paramType = pt;
		userParam.otherUser.buf = NULL;
		userParam.otherUser.len = 0;
        RV_UNUSED_ARG(pcb_pointer);
	    return userParam;
    
}
static ParserUserParam ag_rp_147(PCB_DECL, ParserOtherUser ou) {
/* Line 3189, RV SIP Stack */
 
        ParserUserParam userParam;
        userParam.paramType = RVSIP_USERPARAM_OTHER;
	    userParam.otherUser = ou;
        RV_UNUSED_ARG(pcb_pointer);
	    return userParam;
    
}
#define ag_rp_148(PCB_POINTER) (RVSIP_USERPARAM_PHONE)
#define ag_rp_149(PCB_POINTER) (RVSIP_USERPARAM_IP)
#define ag_rp_150(PCB_POINTER, t) (t)
#define ag_rp_151(PCB_POINTER, ttl) (ttl)
#define ag_rp_152(PCB_POINTER, host) (host)
#define ag_rp_153(PCB_POINTER, met) (met)
#define ag_rp_154(PCB_POINTER) (ParserLrParam1)
#define ag_rp_155(PCB_POINTER) (ParserLrParamTrue)
#define ag_rp_156(PCB_POINTER) (ParserLrParamOn)
#define ag_rp_157(PCB_POINTER) (ParserLrParamEmpty)
#define ag_rp_158(PCB_POINTER, comp) (comp)
#define ag_rp_159(PCB_POINTER, sigCompId) (sigCompId)
#define ag_rp_160(PCB_POINTER) (CUR_STRING())
#define ag_rp_161(PCB_POINTER, hostname) (hostname)
#define ag_rp_162(PCB_POINTER) (RV_TRUE)
static void ag_rp_163(PCB_DECL, ParserBasicToken token) {
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
static void ag_rp_164(PCB_DECL, ParserBasicToken token1, ParserBasicToken token2) {
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
#define ag_rp_165(PCB_POINTER) (CUR_STRING())
#define ag_rp_166(PCB_POINTER, token) (token)
static ParserOptionalHeaders ag_rp_167(PCB_DECL) {
/* Line 3382, RV SIP Stack */
 
        ParserOptionalHeaders optionalHeaders;
        optionalHeaders.isSpecified = RV_FALSE;
		optionalHeaders.body.buf = NULL;
		optionalHeaders.body.len = 0;
        RV_UNUSED_ARG(pcb_pointer);
        return optionalHeaders;
    
}
static ParserOptionalHeaders ag_rp_168(PCB_DECL, ParserBasicToken headersList) {
/* Line 3392, RV SIP Stack */
 
        ParserOptionalHeaders optionalHeaders;
        optionalHeaders.isSpecified = RV_TRUE;
        optionalHeaders.body=headersList;
        RV_UNUSED_ARG(pcb_pointer);
        return optionalHeaders;
    
}
#define ag_rp_169(PCB_POINTER) (CUR_STRING())
#define ag_rp_170(PCB_POINTER) (CUR_STRING())
#define ag_rp_171(PCB_POINTER) (CUR_STRING())
#define ag_rp_172(PCB_POINTER) (CUR_STRING())
#define ag_rp_173(PCB_POINTER) (CUR_STRING())
static void ag_rp_174(PCB_DECL) {
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
static void ag_rp_175(PCB_DECL, ParserMethod met, ParserSipVersion ver) {
/* Line 3447, RV SIP Stack */
         PCB.requestLine.method = met;
         PCB.requestLine.sipVer = ver;
		 PCB.requestLine.exUri = PCB.exUri;         
    
}
#define ag_rp_176(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("m"))
#define ag_rp_177(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("i"))
#define ag_rp_178(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("c"))
#define ag_rp_179(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("v"))
#define ag_rp_180(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("t"))
#define ag_rp_181(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("f"))
#define ag_rp_182(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("r"))
#define ag_rp_183(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("b"))
#define ag_rp_184(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("k"))
#define ag_rp_185(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("x"))
#define ag_rp_186(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("o"))
#define ag_rp_187(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("u"))
#define ag_rp_188(PCB_POINTER) (CUR_REQUEST_COMPACT_METHOD("l"))
#define ag_rp_189(PCB_POINTER, met) (met)
#define ag_rp_190(PCB_POINTER) (SET_URI_TYPE(PARSER_ADDR_PARAM_TYPE_SIP_URL, PCB.exUri.ExUriInfo.absUri))
#define ag_rp_191(PCB_POINTER, uri) (SET_URI_TYPE(PARSER_ADDR_PARAM_TYPE_PRES_URI, uri))
#define ag_rp_192(PCB_POINTER, uri) (SET_URI_TYPE(PARSER_ADDR_PARAM_TYPE_IM_URI, uri))
#define ag_rp_193(PCB_POINTER, uri) (SET_URI_TYPE(PARSER_ADDR_PARAM_TYPE_ABS_URI, uri))
static void ag_rp_194(PCB_DECL, ParserDisplayName name) {
/* Line 3564, RV SIP Stack */
 
		PCB.nameAddr.exUri = PCB.exUri;    
        PCB.nameAddr.isDisplayName = RV_TRUE;  
	    PCB.nameAddr.name = name;
	
}
static void ag_rp_195(PCB_DECL) {
/* Line 3571, RV SIP Stack */
 
		PCB.nameAddr.exUri = PCB.exUri;
        PCB.nameAddr.isDisplayName = RV_FALSE;  
    
}
#define ag_rp_196(PCB_POINTER, qs) (qs)
#define ag_rp_197(PCB_POINTER, mtot) (mtot)
#define ag_rp_198(PCB_POINTER) (CUR_STRING())
#define ag_rp_199(PCB_POINTER) (CUR_STRING())
#define ag_rp_200(PCB_POINTER, t) (t)
#define ag_rp_201(PCB_POINTER, ppd) (ppd)
static ParserPartyParams ag_rp_202(PCB_DECL, ParserPartyParams ppl, ParserPartyParams ppd) {
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
static ParserPartyParams ag_rp_203(PCB_DECL, ParserTagParam tag) {
/* Line 3621, RV SIP Stack */
        ParserPartyParams partyParams;
        partyParams.isTag = RV_TRUE;
        partyParams.tag = tag;
        partyParams.isAddrParams = RV_FALSE;
        partyParams.addrParams = NULL;
        RV_UNUSED_ARG(pcb_pointer);
        return partyParams; 
    
}
static ParserPartyParams ag_rp_204(PCB_DECL) {
/* Line 3631, RV SIP Stack */
        ParserPartyParams partyParams;
        partyParams.isAddrParams = RV_TRUE;
        partyParams.isTag = RV_FALSE;
        partyParams.addrParams = PCB.pExtParams;
		partyParams.tag.buf = NULL;
		partyParams.tag.len = 0;
        return partyParams; 
    
}
static void ag_rp_205(PCB_DECL, ParserMethod method) {
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
static void ag_rp_206(PCB_DECL, ParserMethod method, ParserBasicToken fv) {
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
static void ag_rp_207(PCB_DECL) {
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
static void ag_rp_208(PCB_DECL) {
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
static void ag_rp_209(PCB_DECL) {
/* Line 5502, RV SIP Stack */
        PCB.route.nameAddr = PCB.nameAddr;
        PCB.route.isParams = RV_FALSE;
        PCB.route.eRouteType = PCB.eWhichHeader;
    
}
static void ag_rp_210(PCB_DECL) {
/* Line 5507, RV SIP Stack */
 
        PCB.route.nameAddr = PCB.nameAddr;
        PCB.route.isParams = RV_TRUE;
        PCB.route.eRouteType = PCB.eWhichHeader;
          
         if (((ParserExtensionString *)PCB.pExtParams)->size !=0)
         {
             PCB.route.routeParams = PCB.pExtParams;
         }                
     
}
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
static const unsigned char ag_rpx[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  0,  0,
    5,  6,  7,  8,  9, 10, 11, 12, 13, 14,  0,  0, 15,  0,  0, 16,  0,  0,
   17,  0,  0,  0,  0,  0, 18,  0,  0,  0,  0,  0, 19,  0,  0, 20, 21, 22,
   23, 24, 25, 26,  0,  0, 27,  0,  0, 28, 29, 30, 31,  0,  0, 32, 33, 34,
    0,  0,  0,  0,  0,  0, 35, 36,  0, 37, 38, 39, 40, 41, 42, 43, 44, 45,
   46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
   64,  0, 65, 66, 67, 68,  0,  0, 69, 70, 71, 72, 73, 74,  0, 75, 76, 77,
   78, 79, 80, 81, 82, 83, 84, 85,  0, 86, 87,  0,  0, 88, 89,  0,  0, 90,
    0,  0, 91, 92, 93, 94,  0, 95, 96, 97, 98, 99,100,101,102,103,104,105,
    0,  0,  0,  0,106,107,108,109,  0,110,111,  0,112,113,114,115,116,117,
  118,  0,  0,119,120,  0,  0,  0,  0,  0,  0,121,122,123,124,125,126,127,
    0,  0,128,129,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,130,131,132,133,134,135,136,137,138,139,140,
  141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,  0,  0,
    0,  0,157,158,159,  0,  0,  0,  0,  0,160,161,162,163,164,165,166,167,
  168,169,170,171,  0,  0,172,  0,  0,  0,  0,  0,173,174,175,176,177,178,
  179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,
  197,  0,  0,  0,  0,  0,198,199,200,201,202,203,204,205,206,  0,207,208,
  209,210
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
  0,  2,  0,  6,  0, 10,  0, 67,  0, 68,  0, 69,  0, 70,  0, 71,
  0, 98,  0,100,  0,101,  0,102,  0,103,  0,118,  0,119,  0,121,
  0,125,  0,127,  0,128,  0,130,  0,131,  0,133,  0,134,  0,220,
  0,221,  0,224,  0,225,  0,226,  0,227,  0,228,  0,229,  0,230,
  0,237,  0,238,  0,254,  0,255,  0,256,  0,257,  0,258,  0,259,
  0,260,  0,261,  0,262,  0,263,  0,264,  0,265,  0,266,  0,273,
0
};
static const unsigned char ag_key_ch[] = {
    0, 89,255, 65,255, 77, 78,255, 69, 72,255, 47, 58, 83,255, 80,255, 73,
  255, 82,255, 73,255, 65, 66, 67, 70, 73, 75, 76, 77, 79, 80, 82, 83, 84,
   85, 86, 88,255, 72, 85,255, 72, 82, 85,255, 67, 76,255, 82, 89,255, 65,
   79,255, 77, 78, 80,255, 65, 69,255, 72, 82,255, 67, 71,255, 69, 80,255,
   45,255, 80,255, 77,255, 79,255, 67,255, 58, 83,255, 71, 80,255, 67, 73,
  255, 67, 76, 79, 82, 84,255, 68, 83,255, 58, 65, 66, 67, 72, 73, 76, 77,
   79, 80, 82, 83, 84, 85,255, 58, 83,255, 80,255, 73,255, 73, 80, 83,255,
   83,255, 58,255, 65, 69,255, 79, 82, 84,255, 67, 76, 77, 79, 83, 84, 85,
  255, 83,255, 49, 79, 84,255, 83,255, 65, 66, 67, 73, 82,255, 73, 80,255,
   67, 76,255, 83, 84, 85,255, 69, 80,255, 65, 66, 67, 72, 77, 82, 83, 84,
  255
};
static const unsigned char ag_key_act[] = {
  0,7,4,7,4,3,7,4,7,5,4,3,0,3,4,2,4,2,4,5,4,5,4,7,6,6,5,6,5,5,5,5,3,6,2,
  5,6,6,5,4,3,3,4,3,3,3,4,7,7,4,7,7,4,7,7,4,3,7,5,4,7,7,4,7,3,4,7,7,4,2,
  7,4,7,4,6,4,2,4,2,4,2,4,0,3,4,2,6,4,7,2,4,7,7,7,7,7,4,7,7,4,3,2,2,2,7,
  2,7,2,7,2,2,2,2,2,4,0,3,4,2,4,2,4,3,3,2,4,7,4,3,4,7,7,4,7,7,7,4,7,7,2,
  7,7,2,7,4,3,4,5,7,7,4,7,4,7,7,7,7,7,4,7,7,4,7,7,4,7,2,7,4,7,7,4,7,7,7,
  7,7,2,7,7,4
};
static const unsigned short ag_key_parm[] = {
    0, 10,  0, 12,  0,163,  6,  0, 14,  2,  0, 83,156,157,  0,  0,  0,  0,
    0,  4,  0,  0,  0,  8, 82, 72, 78, 70, 84, 92, 68, 88,161, 80,  0, 76,
   90, 74, 86,  0,188,176,  0,188,160,176,  0,  8, 28,  0, 38, 10,  0, 12,
   40,  0,163,  6, 52,  0, 34, 54,  0, 50,161,  0, 36, 14,  0,  0, 32,  0,
   44,  0, 42,  0,  0,  0,  0,  0,  0,  0,156,157,  0,  0, 16,  0, 22,  0,
    0, 20, 24, 64, 46, 30,  0, 18, 48,  0,200,  0,  0,  0, 26,  0, 56,  0,
   66,  0,  0,  0,  0,  0,  0,156,157,  0,  0,  0,  0,  0,163,161,  0,  0,
   16,  0,200,  0, 34, 54,  0, 64, 46, 30,  0, 40, 56,  0, 66, 44,  0, 48,
    0, 83,  0, 58, 62, 60,  0, 42,  0,  8, 10, 12,  6, 14,  0, 52, 50,  0,
   20, 24,  0, 22,  0, 18,  0, 36, 32,  0, 28, 38, 40, 26, 34,  0, 44, 30,
    0
};
static const unsigned short ag_key_jmp[] = {
    0,  3,  0,  5,  0, 10, 12,  0, 22,  0,  0, 29,  0, 33,  0, 11,  0, 15,
    0,  0,  0,  0,  0,  0,  1,  3,  0,  5,  0,  0,  0,  0, 17,  8, 17,  0,
   19, 21,  0,  0, 35, 40,  0, 49, 54, 61,  0, 72, 74,  0, 78, 83,  0, 85,
   90,  0, 99,101,  0,  0,108,112,  0,121,125,  0,129,135,  0, 66,141,  0,
  148,  0, 72,  0, 74,  0, 76,  0, 78,  0,  0,151,  0, 80, 82,  0,145, 85,
    0,153,155,157,168,176,  0,178,180,  0, 70, 47, 50, 53, 93, 56,106, 60,
  117, 63, 69, 88, 91, 97,  0,  0,191,  0,115,  0,118,  0,183,186,120,  0,
  193,  0,196,  0,204,208,  0,227,238,246,  0,198,202,130,213,217,133,248,
    0,252,  0,  0,259,261,  0,265,  0,272,275,278,284,290,  0,298,300,  0,
  309,311,  0,305,162,313,  0,342,349,  0,316,321,327,331,337,169,353,363,
    0
};
static const unsigned short ag_key_index[] = {
   23,  0, 40, 43, 43,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,100,  0,122,122,  0,126,  0,  0,126,  0,  0,128,  0,  0,
    0,  0,  0,137,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  122,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,128,  0,128,  0,  0,  0,  0,
    0,137,137,  0,  0,  0,  0,145,  0,  0,  0,  0,  0,  0,  0,122,122,  0,
    0,  0,  0,126,128,  0,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,137,  0,  0,  0,  0,  0,  0,  0,  0,122,  0,  0,  0,  0,
    0,  0,126,128,  0,  0,  0,  0,  0,  0,  0,  0,147,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,151,153,  0,
    0,159,165,  0,  0,  0,  0,  0,  0,172,  0,  0,  0,151,153,  0,  0,159,
  165,  0,  0,  0,172,  0,  0,  0,  0,165,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,165,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,151,  0,  0,128,  0,  0,  0,  0,151,  0,128,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0
};
static const unsigned char ag_key_ends[] = {
67,75,0, 69,0, 78,67,69,76,0, 58,0, 86,73,84,69,0, 
82,69,83,58,0, 71,73,83,84,69,82,0, 50,46,48,0, 58,0, 
79,83,84,58,0, 83,69,82,73,78,70,79,58,0, 79,83,84,58,0, 
86,45,65,66,83,58,0, 83,69,82,73,78,70,79,58,0, 58,0, 75,0, 
73,65,83,0, 65,78,67,72,0, 69,0, 78,67,69,76,0, 77,80,0, 
73,68,68,69,78,0, 58,0, 86,73,84,69,0, 82,0, 68,68,82,0, 
84,72,79,68,0, 82,73,71,0, 79,78,69,0, 69,83,58,0, 
69,73,86,69,68,0, 73,83,84,69,82,0, 79,82,84,0, 84,80,0, 73,68,0, 
58,0, 80,0, 83,0, 75,69,78,73,90,69,68,45,66,89,0, 
65,78,83,80,79,82,84,0, 76,0, 80,0, 69,82,0, 77,58,0, 
82,69,83,58,0, 58,0, 73,80,0, 58,0, 79,77,80,0, 82,0, 
68,68,82,0, 84,72,79,68,0, 82,73,71,0, 
73,71,67,79,77,80,45,73,68,0, 75,69,78,73,90,69,68,45,66,89,0, 
65,78,83,80,79,82,84,0, 76,0, 83,69,82,0, 73,80,47,50,46,48,0, 
78,0, 82,85,69,0, 73,71,67,79,77,80,0, 67,75,0, 89,69,0, 
65,78,67,69,76,0, 78,86,73,84,69,0, 69,71,73,83,84,69,82,0, 80,0, 
72,79,78,69,0, 67,84,80,0, 80,0, 83,0, 68,80,0, 76,73,65,83,0, 
82,65,78,67,72,0, 79,77,80,0, 73,68,68,69,78,0, 65,68,68,82,0, 
67,69,73,86,69,68,0, 79,82,84,0, 73,71,67,79,77,80,45,73,68,0, 
84,76,0, 
};
#define AG_TCV(x) ag_tcv[(x)]
static const unsigned short ag_tcv[] = {
    5,292,292,292,292,292,292,292,292,278,  0,292,292,  0,292,292,292,292,
  292,292,292,292,292,292,292,292,292,292,292,292,292,292, 20,279, 42,289,
  290,279,243,279,286,286,279,280, 22,281,197, 36, 74, 74, 74, 74, 74, 74,
   74, 74, 74, 74, 16, 32, 38, 27, 40, 34, 29,282,282,282,282,282,282,283,
  283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,
  283,193, 64,194,291,284,285,282,282,282,282,282,282,283,283,283,283,283,
  283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,287,291,287,
  279,292,291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,
  291,288,288,291,291,291,291,291,288,291,291,291,291,291,291,291,291,291,
  291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,
  291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,
  291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,
  291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,
  291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,291,
  291,291,291,291
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
285,284,283,282,281,280,279,266,265,264,263,262,261,260,259,258,257,256,255,
  254,197,163,161,157,156,83,74,71,70,69,68,67,10,6,2,0,1,8,9,12,13,14,15,
  43,45,66,79,151,152,153,154,158,162,252,253,
285,284,283,282,281,280,279,197,74,0,
188,176,0,155,164,167,173,187,
188,176,160,0,155,159,164,167,173,187,
188,176,160,0,155,159,164,167,173,187,
278,20,16,0,17,18,19,25,
20,0,19,80,
5,0,
5,0,
5,0,
5,0,
16,0,3,
5,0,
5,0,
16,0,3,
16,0,3,
284,283,282,281,197,193,74,0,105,117,175,189,
290,289,286,284,283,282,281,280,279,243,197,74,36,34,32,29,27,22,0,28,174,
  177,179,181,
278,40,34,32,20,5,0,31,165,207,
278,40,34,32,20,5,0,31,165,207,
283,282,281,280,197,74,0,139,140,143,
283,282,281,280,197,74,0,139,140,143,
278,20,0,
16,0,30,
283,282,281,280,278,197,163,161,157,156,74,20,16,0,
283,282,281,280,197,163,161,157,156,74,0,137,138,139,140,143,151,152,153,
  154,158,162,
74,20,0,19,81,84,
285,284,283,282,281,280,279,278,197,98,74,42,38,20,0,17,18,25,
283,282,281,280,197,74,0,11,139,140,143,
285,284,283,282,281,280,279,197,74,42,38,0,7,37,58,145,267,268,276,277,
285,284,283,282,281,280,279,197,98,74,0,4,43,45,89,90,91,95,
284,283,282,281,197,74,0,192,
284,283,282,281,197,74,0,192,
282,200,74,0,136,198,199,202,
16,0,30,
284,283,282,281,197,193,74,0,105,117,175,189,
290,289,286,284,283,282,281,280,279,243,197,74,36,34,32,27,22,0,
16,0,30,
29,0,28,
290,286,284,283,282,281,280,279,278,243,238,237,227,226,221,220,197,194,193,
  134,131,127,121,74,36,20,16,0,17,18,25,
32,0,31,
278,40,34,20,5,0,33,166,241,
278,40,34,20,5,0,33,166,241,
283,282,281,280,197,74,0,
16,0,30,
292,291,290,289,288,287,286,285,284,283,282,281,280,279,278,243,197,194,193,
  74,64,42,40,38,36,34,32,29,27,22,20,16,5,0,17,18,25,
20,0,19,80,
74,0,
20,0,19,
5,0,
285,284,283,282,281,280,279,278,197,74,38,20,0,269,270,271,
292,291,290,289,288,287,286,285,284,283,282,281,280,279,278,243,197,194,193,
  74,64,42,40,38,36,34,32,29,27,22,20,16,5,0,59,60,61,62,
285,284,283,282,281,280,279,197,74,0,
278,38,20,0,17,18,25,
283,282,281,280,278,197,163,161,157,156,74,20,0,17,18,25,
38,0,37,
278,32,22,20,5,0,17,18,25,
22,0,23,
5,0,
278,36,20,0,17,18,25,
278,20,0,17,25,
22,0,23,
5,0,
284,283,282,281,197,74,0,
282,74,0,
278,200,197,194,32,22,20,16,5,0,203,204,205,
282,278,197,194,74,32,22,20,5,0,199,201,202,
200,0,
197,0,
194,0,
74,0,75,104,
290,286,284,283,282,281,280,279,243,197,74,29,27,22,0,178,182,184,185,186,
284,283,282,281,197,193,74,0,105,117,175,189,
290,286,284,283,282,281,280,279,243,238,237,227,226,221,220,197,194,193,134,
  131,127,121,74,36,16,0,208,209,210,211,212,213,214,215,216,217,218,219,
  239,
290,286,284,283,282,281,280,279,278,243,238,237,227,226,221,220,197,194,193,
  134,131,127,121,74,36,20,16,0,17,18,25,
290,286,284,283,282,281,280,279,197,194,193,74,36,34,16,0,242,244,247,
243,0,
290,289,286,284,283,282,281,280,279,278,243,197,194,193,74,36,34,32,29,27,
  22,20,16,0,17,18,25,
292,291,290,289,288,287,286,285,284,283,282,281,280,279,243,197,194,193,74,
  64,42,40,38,36,34,32,29,27,22,16,0,52,
83,20,0,19,79,
74,0,
291,290,289,288,287,286,285,284,283,282,281,280,279,243,197,194,193,74,64,
  42,40,38,36,34,32,29,27,22,20,16,5,0,82,86,87,88,
278,20,0,269,
292,291,290,289,288,287,286,285,284,283,282,281,280,279,278,243,197,194,193,
  74,64,42,40,38,36,34,32,29,27,22,20,16,5,0,
292,291,290,289,288,287,286,285,284,283,282,281,280,279,278,243,197,194,193,
  74,64,40,38,36,34,32,29,27,22,20,16,5,0,59,60,
42,0,
285,284,283,282,281,280,279,278,197,74,38,20,0,269,270,271,
283,282,281,280,197,163,161,157,156,74,0,138,139,140,143,151,152,153,154,
  158,162,
283,282,281,280,278,197,163,161,157,156,74,20,0,17,18,25,
32,0,31,76,
285,284,283,282,281,280,279,278,197,74,42,38,20,0,17,18,25,
36,0,35,
284,283,282,281,197,193,74,0,92,105,117,189,
285,284,283,282,281,280,279,278,197,98,74,20,0,17,18,25,
282,74,0,
282,74,0,202,
16,0,203,
282,278,197,194,74,32,22,20,5,0,199,201,202,
74,0,195,196,
74,0,
290,286,284,283,282,281,280,279,243,197,74,27,22,0,184,
290,286,284,283,282,281,280,279,278,243,197,194,193,74,40,36,34,32,27,20,16,
  5,0,234,235,236,
27,0,26,240,
278,27,20,0,17,18,25,
278,27,20,0,17,18,25,
278,27,20,0,17,18,25,
285,284,283,282,281,280,279,278,197,74,40,34,32,27,20,5,0,26,231,
278,27,20,0,17,18,25,
278,27,20,0,17,18,25,
278,27,20,0,17,18,25,
278,27,20,0,17,18,25,
278,27,20,0,17,18,25,
290,286,284,283,282,281,280,279,243,238,237,227,226,221,220,197,194,193,134,
  131,127,121,74,36,16,0,208,209,210,211,212,213,214,215,216,217,218,219,
  239,
290,286,284,283,282,281,280,279,197,194,193,74,36,34,16,0,
27,0,
290,286,284,283,282,281,280,279,197,194,193,74,36,34,16,0,242,244,247,
290,289,286,284,283,282,281,280,279,243,197,194,193,74,36,34,32,29,27,22,16,
  0,141,147,
292,291,290,289,288,287,286,285,284,283,282,281,280,279,278,243,197,194,193,
  74,64,42,40,38,36,34,32,29,27,22,20,16,5,0,55,56,57,
278,20,5,0,17,18,25,
291,290,289,288,287,286,285,284,283,282,281,280,279,243,197,194,193,74,64,
  42,40,38,36,34,32,29,27,22,20,16,0,86,
278,40,20,0,17,18,25,
283,282,281,280,197,163,161,157,156,74,0,138,139,140,143,151,152,153,154,
  158,162,
285,284,283,282,281,280,279,278,197,74,20,0,17,18,25,
32,0,31,
285,284,283,282,281,280,279,197,74,42,38,0,37,58,145,267,268,277,
285,284,283,282,281,280,279,278,197,74,20,0,17,18,25,
278,32,22,20,16,5,0,17,18,25,
32,22,5,0,93,
285,284,283,282,281,280,279,197,98,74,0,43,45,90,91,95,
282,74,0,
74,0,
197,0,
290,286,284,283,282,281,280,279,243,197,194,193,74,36,16,0,234,
290,286,284,283,282,281,280,279,278,243,197,194,193,74,36,20,16,0,17,18,25,
27,0,26,
27,0,26,
27,0,26,
285,284,283,282,281,280,279,278,197,74,40,34,32,20,5,0,43,45,232,
230,229,228,0,
27,0,26,
27,0,26,
27,0,26,
27,0,26,
27,0,26,
290,286,284,283,282,281,280,279,278,243,197,194,193,74,40,36,34,29,20,16,5,
  0,245,249,250,251,
290,289,286,284,283,282,281,280,279,243,197,194,193,74,36,34,32,29,27,22,16,
  0,
292,291,290,289,288,287,286,285,284,283,282,281,280,279,278,243,197,194,193,
  74,64,42,40,38,36,34,32,29,27,22,20,16,0,55,
40,0,39,
278,40,20,0,17,18,25,
285,284,283,282,281,280,279,197,74,0,43,45,77,
285,284,283,282,281,280,279,278,197,74,20,0,17,18,25,
285,284,283,282,281,280,279,197,74,0,43,45,96,
16,0,30,
32,0,31,
74,0,
74,0,196,
290,286,284,283,282,281,280,279,243,197,194,193,74,36,16,0,239,
284,283,282,281,278,197,193,74,20,0,17,18,25,
290,286,284,283,282,281,280,279,278,243,197,194,193,74,36,20,16,0,17,18,25,
285,284,283,282,281,280,279,278,197,133,74,20,0,17,18,25,
285,284,283,282,281,280,279,278,197,74,71,70,69,68,67,20,0,17,18,25,
284,283,282,281,278,197,193,74,20,0,17,18,25,
278,74,20,0,17,18,25,
285,284,283,282,281,280,279,278,225,224,197,74,20,0,17,18,25,
285,284,283,282,281,280,279,278,197,103,102,101,100,74,20,0,17,18,25,
290,286,284,283,282,281,280,279,197,194,193,74,36,34,29,16,0,249,
40,0,39,
278,32,27,22,20,5,0,17,18,25,
285,284,283,282,281,280,279,197,74,0,43,45,77,
278,36,20,0,17,18,25,
278,74,20,0,17,18,25,
285,284,283,282,281,280,279,278,197,134,131,130,128,127,125,121,119,118,74,
  20,0,17,18,25,
197,0,
284,283,282,281,197,193,74,0,105,117,189,
290,286,284,283,282,281,280,279,243,197,194,193,74,36,16,0,135,
285,284,283,282,281,280,279,197,133,74,0,43,45,132,
285,284,283,282,281,280,279,197,74,71,70,69,68,67,0,43,45,66,
284,283,282,281,197,193,74,0,105,117,189,
74,0,122,
285,284,283,282,281,280,279,225,224,197,74,0,43,45,222,223,
285,284,283,282,281,280,279,197,103,102,101,100,74,0,43,45,99,
27,0,26,78,
36,0,35,
74,0,75,104,
285,284,283,282,281,280,279,197,134,131,130,128,127,125,121,119,118,74,0,43,
  45,94,106,107,108,109,110,111,112,113,114,115,116,120,123,126,
74,0,196,
290,286,284,283,282,281,280,279,278,243,197,194,193,74,42,40,36,34,32,20,16,
  5,0,234,235,236,
74,0,
285,284,283,282,281,280,279,278,197,74,42,20,0,17,18,25,
285,284,283,282,281,280,279,278,197,103,102,101,100,74,20,0,17,18,25,
278,32,22,20,5,0,17,18,25,
278,32,27,22,20,5,0,17,18,25,
278,32,27,22,20,5,0,17,18,25,
278,27,20,0,17,18,25,
278,27,20,0,17,18,25,
278,27,20,0,17,18,25,
278,27,20,0,17,18,25,
32,27,22,5,0,26,124,
278,27,20,0,17,18,25,
278,27,20,0,17,18,25,
278,32,22,20,5,0,17,18,25,
278,32,22,20,5,0,17,18,25,
74,0,
285,284,283,282,281,280,279,197,74,42,0,43,45,58,
285,284,283,282,281,280,279,197,103,102,101,100,74,0,43,45,97,99,
27,0,26,
27,0,26,
27,0,26,
27,0,26,
27,0,26,
278,74,20,0,17,18,25,
27,0,26,
27,0,26,
278,32,22,20,5,0,17,18,25,
278,32,22,20,5,0,17,18,25,
285,284,283,282,281,280,279,278,197,193,74,42,20,0,17,18,25,
284,283,282,281,278,197,193,74,20,0,17,18,25,
278,74,20,0,17,18,25,
278,42,20,0,17,18,25,
285,284,283,282,281,280,279,278,197,133,74,20,0,17,18,25,
74,0,75,104,
285,284,283,282,281,280,279,278,197,74,20,0,17,18,25,
282,278,200,74,20,0,17,18,25,
285,284,283,282,281,280,279,197,193,74,42,0,43,45,58,117,
284,283,282,281,197,193,74,0,105,117,189,
74,0,122,
42,0,41,
285,284,283,282,281,280,279,197,133,74,0,43,45,132,
285,284,283,282,281,280,279,197,74,0,43,45,
282,200,74,0,129,136,198,199,202,
278,32,22,20,5,0,17,18,25,
278,32,22,20,5,0,17,18,25,
278,32,22,20,5,0,17,18,25,
278,32,22,20,5,0,17,18,25,
278,32,22,20,5,0,17,18,25,
290,286,284,283,282,281,280,279,243,197,194,193,74,36,16,0,135,
278,32,22,20,5,0,17,18,25,
278,32,22,20,5,0,17,18,25,
278,32,22,20,5,0,17,18,25,
42,0,
};
static unsigned const char ag_astt[2603] = {
  1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,1,2,2,2,2,2,1,1,1,7,0,
  1,1,1,1,1,1,2,1,2,1,2,2,2,1,1,1,2,1,9,9,9,9,9,9,9,9,9,4,2,2,7,3,1,1,1,1,2,
  2,2,7,2,1,1,1,1,1,2,2,2,7,2,1,1,1,1,1,1,1,8,7,1,1,1,1,2,7,1,1,3,7,3,7,3,7,
  3,7,1,7,1,3,7,3,7,1,7,1,1,7,1,1,1,1,1,1,1,1,7,1,2,2,2,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,2,1,1,7,1,1,1,2,1,4,4,4,2,4,4,7,1,1,1,4,4,4,2,4,4,7,1,1,1,1,1,1,
  1,1,1,7,2,1,1,1,1,1,1,1,1,7,2,1,1,9,9,4,2,7,1,4,4,4,4,5,4,4,4,4,4,4,5,5,7,
  1,1,1,1,1,2,2,2,2,1,7,1,1,2,1,1,2,2,2,1,1,1,1,2,7,3,1,1,5,5,5,5,5,5,5,1,5,
  5,5,5,5,1,7,2,2,1,1,1,1,1,1,1,7,1,2,1,1,1,1,1,1,1,1,1,1,1,1,2,7,1,1,1,1,1,
  1,1,2,1,1,1,1,1,1,1,1,2,1,7,1,2,1,1,2,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,4,1,
  1,1,1,7,1,1,1,1,2,4,1,1,1,1,1,1,1,1,7,1,2,2,2,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9,9,9,4,2,4,1,2,7,1,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,8,
  7,1,1,1,2,4,1,4,4,2,4,4,7,1,2,1,4,4,2,4,4,7,1,2,1,9,9,9,9,9,9,4,2,7,1,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,8,5,7,1,1,1,2,7,
  1,1,1,7,2,4,1,3,7,5,5,5,5,5,5,5,1,5,5,5,1,7,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,8,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,4,1,
  5,1,7,2,2,1,8,8,8,8,1,8,8,8,8,8,8,1,7,1,1,1,2,7,1,1,8,5,1,5,7,1,1,1,2,5,1,
  3,7,1,8,1,7,1,1,1,1,1,7,1,1,2,5,1,3,7,9,9,9,9,9,9,4,1,1,5,5,5,5,5,5,5,5,1,
  5,7,1,1,3,1,5,5,5,1,5,5,5,5,7,3,3,1,1,5,1,5,2,7,1,7,1,2,1,1,1,1,1,1,1,1,1,
  1,1,5,1,1,7,2,2,1,1,2,1,1,1,1,1,1,1,7,1,2,2,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,7,2,2,2,2,2,2,2,2,2,2,2,2,1,8,8,8,8,8,8,8,8,1,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,1,8,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,2,1,
  1,1,4,8,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,1,8,7,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,4,2,2,2,7,3,1,2,7,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,5,7,2,1,1,2,9,9,5,3,3,3,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,7,9,9,9,9,9,
  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,1,9,9,9,9,9,9,9,9,9,9,9,5,3,3,2,7,5,5,5,5,5,
  5,5,1,5,5,5,1,7,1,1,2,1,1,1,1,1,2,2,2,2,1,7,1,2,1,1,2,2,2,1,1,1,8,8,8,8,1,
  8,8,8,8,8,8,1,7,1,1,1,2,4,1,1,8,8,8,8,8,8,8,1,8,8,8,8,1,7,1,1,1,2,7,1,1,1,
  1,1,1,1,1,7,1,1,2,2,8,8,8,8,8,8,8,1,8,8,8,1,7,1,1,1,1,1,5,1,1,7,3,1,5,3,1,
  5,5,5,1,5,5,5,5,7,3,3,1,1,7,3,1,9,4,9,9,9,9,9,9,9,9,9,9,9,9,9,5,3,1,1,1,1,
  1,1,1,1,5,1,1,1,1,1,5,1,5,5,5,5,1,5,7,1,1,2,2,4,1,2,1,8,1,7,1,1,1,1,8,1,7,
  1,1,1,1,8,1,7,1,1,1,8,8,8,8,8,8,8,5,8,8,5,5,5,2,5,5,7,1,1,1,8,1,7,1,1,1,1,
  8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,1,1,1,1,1,1,1,1,1,2,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,2,2,2,2,2,2,2,2,2,2,2,2,1,9,9,9,9,9,9,9,9,
  9,9,9,9,9,9,9,4,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,2,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,7,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,5,7,1,1,2,1,1,5,7,2,2,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,5,3,1,8,1,7,1,1,1,1,1,1,1,1,2,2,2,2,1,7,1,
  2,1,1,2,2,2,1,1,1,8,8,8,8,8,8,8,1,8,8,1,7,1,1,1,2,4,1,1,1,1,1,1,1,1,1,1,1,
  2,7,1,1,1,1,1,2,8,8,8,8,8,8,8,1,8,8,1,7,1,1,1,1,5,5,1,8,5,7,1,1,1,4,4,4,7,
  1,1,1,1,1,1,1,1,1,2,1,7,2,1,2,1,1,3,3,5,1,5,1,7,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9,9,5,3,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,1,8,7,1,1,1,2,7,1,2,7,1,2,7,1,1,1,1,
  1,1,1,1,5,1,1,5,5,5,5,5,7,2,1,2,2,2,2,5,2,7,1,2,7,1,2,7,1,2,7,1,2,7,1,1,1,
  1,1,1,1,1,1,5,5,1,1,1,1,5,1,1,1,5,1,5,7,2,1,1,2,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9,9,9,9,9,9,9,9,4,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9,9,9,9,5,3,2,7,2,1,8,1,7,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,3,8,8,8,8,8,8,8,1,
  8,8,1,7,1,1,1,1,1,1,1,1,1,1,1,1,7,2,1,1,2,4,1,2,4,1,3,5,1,7,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,7,2,8,8,8,8,1,8,8,8,1,7,1,1,1,8,8,8,8,8,8,8,8,1,8,8,8,8,
  8,8,1,8,7,1,1,1,8,8,8,8,8,8,8,1,8,8,8,1,7,1,1,1,8,8,8,8,8,8,8,1,8,8,8,8,8,
  8,8,1,7,1,1,1,8,8,8,8,1,8,8,8,1,7,1,1,1,1,8,1,7,1,1,1,8,8,8,8,8,8,8,1,8,8,
  8,8,1,7,1,1,1,8,8,8,8,8,8,8,1,8,8,8,8,8,8,1,7,1,1,1,9,9,9,9,9,9,9,9,9,9,9,
  9,9,9,9,9,5,3,2,7,2,1,5,8,5,1,5,7,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,3,1,8,1,7,
  1,1,1,1,8,1,7,1,1,1,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,1,7,1,1,1,1,7,1,
  1,1,1,1,1,1,7,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,2,1,1,1,1,1,1,1,1,2,1,
  7,2,1,2,1,1,1,1,1,1,1,1,1,2,2,2,2,2,7,2,1,2,1,1,1,1,1,1,1,7,2,2,2,1,7,2,1,
  1,1,1,1,1,1,2,2,1,1,7,2,1,2,2,1,1,1,1,1,1,1,1,2,2,2,2,1,7,2,1,2,2,4,1,2,2,
  7,1,1,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,2,2,2,2,2,2,2,2,2,2,
  2,2,2,1,2,1,7,3,1,1,1,1,1,1,1,1,5,1,1,1,1,1,5,5,1,5,5,5,1,5,7,1,1,2,1,4,8,
  8,8,8,8,8,8,1,8,8,8,1,7,1,1,1,8,8,8,8,8,8,8,1,8,8,8,8,8,8,1,7,1,1,1,1,5,5,
  1,5,7,2,2,1,1,5,8,5,1,5,7,1,1,1,1,5,5,5,1,5,7,3,3,1,1,8,1,7,1,1,1,1,8,1,7,
  1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,4,2,4,4,7,1,2,1,8,1,7,1,1,1,1,8,1,7,1,1,
  1,1,5,5,1,5,7,3,3,1,1,5,5,1,5,7,3,3,1,2,4,1,1,1,1,1,1,1,1,1,1,7,1,1,1,1,1,
  1,1,1,1,1,1,2,2,2,2,1,7,2,1,2,2,2,4,1,2,7,1,2,7,1,2,7,1,2,7,1,1,8,1,7,1,1,
  1,2,7,1,2,7,1,1,5,5,1,5,7,2,2,1,1,5,5,1,5,7,2,2,1,8,8,8,8,8,8,8,1,8,8,8,8,
  1,7,1,1,1,8,8,8,8,1,8,8,8,1,7,1,1,1,1,8,1,7,1,1,1,1,8,1,7,1,1,1,8,8,8,8,8,
  8,8,1,8,8,8,1,7,1,1,1,1,7,1,2,8,8,8,8,8,8,8,1,8,8,1,7,1,1,1,8,1,8,8,1,7,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,1,1,1,1,1,1,1,1,7,1,2,2,1,7,1,2,7,1,1,1,
  1,1,1,1,1,1,2,1,7,2,1,1,1,1,1,1,1,1,1,1,1,7,1,1,1,1,1,7,1,2,1,1,1,1,5,5,1,
  5,7,2,2,1,1,5,5,1,5,7,2,2,1,1,5,5,1,5,7,2,2,1,1,5,5,1,5,7,2,2,1,1,5,5,1,5,
  7,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,7,1,1,5,5,1,5,7,2,2,1,1,5,5,1,5,7,2,
  2,1,1,5,5,1,5,7,2,2,1,2,7
};
static const unsigned short ag_pstt[] = {
1,1,1,1,1,1,1,297,296,295,294,293,292,291,290,289,288,287,286,285,1,159,156,
  152,151,71,1,55,54,53,52,51,11,14,15,0,0,13,12,10,9,8,7,56,1,298,6,147,
  148,149,2,4,3,283,5,
29,29,29,29,29,29,29,29,29,30,
192,177,2,150,19,18,17,16,
192,177,155,3,158,20,19,18,17,16,
192,177,155,4,154,21,19,18,17,16,
22,24,23,5,23,23,25,22,
13,6,26,26,
9,7,
8,8,
7,9,
6,10,
27,11,28,
4,12,
3,13,
27,14,29,
27,15,30,
31,32,32,31,31,33,32,16,34,196,191,195,
36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,20,36,36,17,35,38,37,180,36,
223,223,223,22,223,223,18,39,41,40,
223,223,223,22,223,223,19,39,42,40,
43,43,43,43,43,43,20,157,44,43,
43,43,43,43,43,43,21,153,44,43,
17,17,18,
21,23,45,
13,13,13,13,16,13,13,13,13,13,13,16,16,24,
43,43,43,43,43,159,156,152,151,43,25,46,46,302,44,43,299,300,301,2,4,3,
47,13,26,68,48,48,
10,10,10,10,10,10,10,22,10,10,10,10,10,22,27,12,12,22,
43,43,43,43,43,43,28,49,135,44,43,
50,50,50,50,50,50,50,50,50,51,25,29,58,54,53,55,56,52,57,322,
1,1,1,1,1,1,1,1,88,1,30,62,87,1,61,81,60,59,
63,63,63,63,63,63,31,63,
63,63,63,63,63,63,197,63,
64,66,64,33,69,68,67,65,
21,193,70,
31,32,32,31,31,33,32,35,34,196,176,195,
182,182,182,182,182,182,182,182,182,182,182,182,182,182,182,182,182,183,
21,178,71,
20,38,72,
73,73,73,73,73,73,73,73,22,73,73,73,73,73,73,73,73,73,73,73,73,73,73,73,73,
  22,73,39,73,73,22,
22,224,74,
269,269,23,269,269,41,75,161,76,
269,269,23,269,269,42,75,160,76,
138,138,138,138,138,138,139,
21,44,77,
78,78,78,78,78,78,78,78,78,78,78,78,78,78,22,78,78,78,78,78,78,78,78,78,78,
  78,78,78,78,78,22,78,10,45,78,78,22,
13,46,79,79,
80,47,
13,70,81,
5,49,
310,310,310,310,310,310,310,82,310,310,310,82,50,82,82,313,
84,84,84,84,84,84,84,84,84,84,84,84,84,84,84,84,84,84,84,84,83,85,84,84,84,
  84,84,84,84,84,84,84,84,51,84,84,84,85,
86,86,86,86,86,86,86,86,86,306,
22,10,22,53,305,305,22,
87,87,87,87,22,87,87,87,87,87,87,22,54,87,87,22,
25,55,88,
22,89,10,22,10,56,89,89,22,
15,321,90,
2,58,
22,91,22,59,91,91,22,
22,22,60,92,22,
15,80,93,
1,62,
199,199,199,199,199,199,200,
94,94,219,
216,216,216,216,216,216,216,95,216,65,96,96,218,
64,209,209,209,64,209,209,209,209,66,212,212,65,
97,208,
98,206,
201,69,
99,70,99,194,
100,100,100,100,100,100,100,100,100,100,100,188,100,100,71,179,184,100,100,
  184,
31,32,32,31,31,33,32,72,34,196,175,195,
101,101,101,101,101,101,101,101,101,264,103,106,107,110,111,101,101,101,104,
  105,108,109,101,101,101,73,225,227,228,229,230,231,232,233,234,235,236,
  237,102,
112,112,112,112,112,112,112,112,22,112,112,112,112,112,112,112,112,112,112,
  112,112,112,112,112,112,22,112,74,112,112,22,
113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,75,271,114,113,
115,270,
116,116,116,116,116,116,116,116,116,22,116,116,116,116,116,116,116,116,116,
  116,116,22,116,77,116,116,22,
117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,
  117,117,117,117,117,117,117,117,117,117,117,319,320,
71,13,79,68,118,
79,80,
119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,
  119,119,119,119,119,119,119,119,119,119,119,76,81,69,119,119,78,
309,309,311,309,
50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,
  50,50,50,50,50,50,50,50,83,
45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,45,83,45,45,45,45,
  45,45,45,45,45,45,45,47,45,45,
48,85,
310,310,310,310,310,310,310,82,310,310,310,82,86,82,82,312,
43,43,43,43,43,159,156,152,151,43,87,120,302,44,43,299,300,301,2,4,3,
121,121,121,121,22,121,121,121,121,121,121,22,88,121,121,22,
22,324,122,123,
124,124,124,124,124,124,124,22,124,124,124,124,22,90,124,124,22,
24,91,125,
31,32,32,31,31,33,32,92,127,126,196,195,
128,128,128,128,128,128,128,22,128,128,128,22,93,128,128,22,
129,129,220,
64,64,95,213,
95,217,215,
64,209,209,209,64,209,209,209,209,97,211,211,65,
130,98,207,131,
59,96,
187,187,187,187,187,187,187,187,187,187,187,187,187,189,187,
132,132,132,132,132,132,132,132,260,132,132,132,132,132,260,132,260,260,260,
  260,132,260,101,132,132,267,
19,265,133,266,
22,134,22,103,134,134,22,
22,135,22,104,135,135,22,
22,136,22,105,136,136,22,
137,137,137,137,137,137,137,250,137,137,250,250,250,19,250,250,106,138,137,
22,139,22,107,139,139,22,
22,140,22,108,140,140,22,
22,141,22,109,141,141,22,
22,142,22,110,142,142,22,
22,143,22,111,143,143,22,
101,101,101,101,101,101,101,101,101,264,103,106,107,110,111,101,101,101,104,
  105,108,109,101,101,101,112,226,227,228,229,230,231,232,233,234,235,236,
  237,102,
275,275,275,275,275,275,275,275,275,275,275,275,275,275,275,276,
144,114,
113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,115,272,114,113,
145,145,145,145,145,145,145,145,145,145,145,145,145,145,145,145,145,145,145,
  145,145,116,136,145,
146,146,146,146,146,146,146,146,146,146,146,146,146,146,146,146,146,146,146,
  146,146,146,146,146,146,146,146,146,146,146,146,146,40,117,146,146,42,
22,22,10,118,284,284,22,
75,75,75,75,75,75,75,75,75,75,75,75,75,75,75,75,75,75,75,75,75,75,75,75,75,
  75,75,75,75,75,77,75,
22,147,22,120,147,147,22,
43,43,43,43,43,159,156,152,151,43,121,148,302,44,43,299,300,301,2,4,3,
149,149,149,149,149,149,149,22,149,149,22,122,149,149,22,
22,325,150,
50,50,50,50,50,50,50,50,50,51,25,124,54,53,55,56,52,323,
151,151,151,151,151,151,151,22,151,151,22,125,151,151,22,
22,10,10,22,152,10,126,152,152,22,
84,84,84,127,153,
1,1,1,1,1,1,1,1,88,1,128,87,1,82,60,59,
222,222,221,
154,203,
155,131,
259,259,259,259,259,259,259,259,259,259,259,259,259,259,259,261,259,
156,156,156,156,156,156,156,156,22,156,156,156,156,156,156,22,156,133,156,
  156,22,
19,134,157,
19,135,158,
19,136,159,
1,1,1,1,1,1,1,252,1,1,252,252,252,252,252,137,254,1,254,
249,248,247,251,
19,139,160,
19,140,161,
19,141,162,
19,142,163,
19,143,164,
165,165,165,165,165,165,165,165,280,280,165,165,165,165,280,165,165,165,280,
  165,280,144,273,165,165,282,
142,142,142,142,142,142,142,142,142,142,142,142,142,142,142,142,142,142,142,
  142,142,143,
39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,
  39,39,39,39,39,39,39,41,39,
26,147,304,
22,166,22,148,166,166,22,
1,1,1,1,1,1,1,1,1,149,167,1,61,
168,168,168,168,168,168,168,22,168,168,22,150,168,168,22,
1,1,1,1,1,1,1,1,1,151,89,1,169,
21,97,170,
22,83,171,
205,204,
130,155,172,
101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,156,268,
173,173,173,173,22,173,173,173,22,157,173,173,22,
174,174,174,174,174,174,174,174,22,174,174,174,174,174,174,22,174,158,174,
  174,22,
175,175,175,175,175,175,175,22,175,175,175,22,159,175,175,22,
176,176,176,176,176,176,176,22,176,176,176,176,176,176,176,22,160,176,176,
  22,
177,177,177,177,22,177,177,177,22,161,177,177,22,
22,178,22,162,178,178,22,
179,179,179,179,179,179,179,22,179,179,179,179,22,163,179,179,22,
180,180,180,180,180,180,180,22,180,180,180,180,180,180,22,164,180,180,22,
279,279,279,279,279,279,279,279,279,279,279,279,279,279,279,279,281,279,
26,166,303,
22,10,181,10,22,10,167,181,181,22,
1,1,1,1,1,1,1,1,1,168,167,1,62,
22,182,22,169,182,182,22,
22,183,22,170,183,183,22,
184,184,184,184,184,184,184,22,184,184,184,184,184,184,184,184,184,184,184,
  22,171,184,184,22,
185,172,
31,32,32,31,31,33,32,173,263,196,195,
186,186,186,186,186,186,186,186,186,186,186,186,186,186,186,174,256,
1,1,1,1,1,1,1,1,130,1,175,131,1,255,
1,1,1,1,1,1,1,1,1,55,54,53,52,51,176,56,1,246,
31,32,32,31,31,33,32,177,245,196,195,
187,178,244,
1,1,1,1,1,1,1,242,241,1,1,179,243,1,239,240,
1,1,1,1,1,1,1,1,94,93,92,91,1,180,95,1,238,
19,63,188,64,
24,182,189,
99,183,99,190,
1,1,1,1,1,1,1,1,195,196,198,199,193,192,194,200,201,1,184,191,1,85,99,100,
  101,102,103,104,105,106,107,108,108,116,197,125,
130,185,202,
132,132,132,132,132,132,132,132,260,132,132,132,132,132,260,260,132,260,260,
  260,132,260,186,132,132,262,
202,118,
203,203,203,203,203,203,203,22,203,203,203,22,188,203,203,22,
204,204,204,204,204,204,204,22,204,204,204,204,204,204,22,189,204,204,22,
22,10,10,22,10,190,98,98,22,
22,10,205,10,22,10,191,205,205,22,
22,10,10,10,22,10,192,122,122,22,
22,206,22,193,206,206,22,
22,207,22,194,207,207,22,
22,208,22,195,208,208,22,
22,209,22,196,209,209,22,
123,19,123,123,197,210,121,
22,211,22,198,211,211,22,
22,212,22,199,212,212,22,
22,10,10,22,10,200,115,115,22,
22,10,10,22,10,201,114,114,22,
120,119,
1,1,1,1,1,1,1,1,1,51,203,214,1,213,
1,1,1,1,1,1,1,1,94,93,92,91,1,204,95,1,86,90,
19,110,215,
19,206,216,
19,207,217,
19,208,218,
19,209,219,
22,220,22,210,220,220,22,
19,211,221,
19,212,222,
22,10,10,22,10,213,66,66,22,
22,10,10,22,10,214,65,65,22,
223,223,223,223,223,223,223,22,223,223,223,223,22,215,223,223,22,
224,224,224,224,22,224,224,224,22,216,224,224,22,
22,225,22,217,225,225,22,
22,226,22,218,226,226,22,
227,227,227,227,227,227,227,22,227,227,227,22,219,227,227,22,
99,220,99,124,
228,228,228,228,228,228,228,22,228,228,22,221,228,228,22,
229,22,229,229,22,222,229,229,22,
1,1,1,1,1,1,1,1,33,1,51,223,232,1,231,230,
31,32,32,31,31,33,32,224,233,196,195,
187,225,234,
27,226,235,
1,1,1,1,1,1,1,1,130,1,227,131,1,236,
1,1,1,1,1,1,1,1,1,228,237,1,
64,66,64,229,238,133,68,67,65,
22,10,10,22,10,230,113,113,22,
22,10,10,22,10,231,112,112,22,
22,10,10,22,10,232,111,111,22,
22,10,10,22,10,233,126,126,22,
22,10,10,22,10,234,117,117,22,
186,186,186,186,186,186,186,186,186,186,186,186,186,186,186,235,239,
22,10,10,22,10,236,129,129,22,
22,10,10,22,10,237,128,128,22,
22,10,10,22,10,238,127,127,22,
132,239,
};
static const unsigned short ag_sbt[] = {
     0,  55,  65,  73,  83,  93, 101, 105, 107, 109, 111, 113, 116, 118,
   120, 123, 126, 138, 162, 172, 182, 192, 202, 205, 208, 222, 244, 250,
   268, 279, 299, 317, 325, 333, 341, 344, 356, 374, 377, 380, 411, 414,
   423, 432, 439, 442, 479, 483, 485, 488, 490, 506, 544, 554, 561, 577,
   580, 589, 592, 594, 601, 606, 609, 611, 618, 621, 634, 647, 649, 651,
   653, 657, 677, 689, 728, 759, 778, 780, 807, 839, 844, 846, 882, 886,
   920, 955, 957, 973, 994,1010,1014,1031,1034,1046,1062,1065,1069,1072,
  1085,1089,1091,1106,1132,1136,1143,1150,1157,1176,1183,1190,1197,1204,
  1211,1250,1266,1268,1287,1311,1348,1355,1387,1394,1415,1430,1433,1451,
  1466,1476,1481,1497,1500,1502,1504,1521,1542,1545,1548,1551,1570,1574,
  1577,1580,1583,1586,1589,1615,1637,1671,1674,1681,1694,1709,1722,1725,
  1728,1730,1733,1750,1763,1784,1800,1820,1833,1840,1857,1876,1894,1897,
  1907,1920,1927,1934,1958,1960,1971,1988,2002,2020,2031,2034,2050,2067,
  2071,2074,2078,2114,2117,2143,2145,2161,2180,2189,2199,2209,2216,2223,
  2230,2237,2244,2251,2258,2267,2276,2278,2292,2310,2313,2316,2319,2322,
  2325,2332,2335,2338,2347,2356,2373,2386,2393,2400,2416,2420,2435,2444,
  2460,2471,2474,2477,2491,2503,2512,2521,2530,2539,2548,2557,2574,2583,
  2592,2601,2603
};
static const unsigned short ag_sbe[] = {
    35,  64,  67,  76,  86,  96, 102, 106, 108, 110, 112, 114, 117, 119,
   121, 124, 133, 156, 168, 178, 188, 198, 204, 206, 221, 232, 246, 264,
   274, 290, 309, 323, 331, 336, 342, 351, 373, 375, 378, 407, 412, 419,
   428, 438, 440, 475, 480, 484, 486, 489, 502, 539, 553, 557, 573, 578,
   585, 590, 593, 597, 603, 607, 610, 617, 620, 630, 643, 648, 650, 652,
   654, 671, 684, 714, 755, 774, 779, 803, 837, 841, 845, 877, 884, 919,
   952, 956, 969, 983,1006,1011,1027,1032,1041,1058,1064,1067,1070,1081,
  1086,1090,1104,1128,1133,1139,1146,1153,1173,1179,1186,1193,1200,1207,
  1236,1265,1267,1283,1308,1344,1351,1385,1390,1404,1426,1431,1444,1462,
  1472,1479,1491,1499,1501,1503,1519,1538,1543,1546,1549,1566,1573,1575,
  1578,1581,1584,1587,1610,1636,1669,1672,1677,1690,1705,1718,1723,1726,
  1729,1731,1748,1759,1780,1796,1816,1829,1836,1853,1872,1892,1895,1903,
  1916,1923,1930,1954,1959,1967,1986,1998,2016,2027,2032,2045,2063,2068,
  2072,2075,2096,2115,2139,2144,2157,2176,2185,2195,2205,2212,2219,2226,
  2233,2241,2247,2254,2263,2272,2277,2288,2305,2311,2314,2317,2320,2323,
  2328,2333,2336,2343,2352,2369,2382,2389,2396,2412,2417,2431,2440,2455,
  2467,2472,2475,2487,2500,2506,2517,2526,2535,2544,2553,2572,2579,2588,
  2597,2602,2603
};
static const unsigned char ag_fl[] = {
  1,4,4,2,2,4,2,2,2,2,0,1,2,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,1,1,2,1,1,
  2,1,1,1,2,0,1,2,1,1,2,0,1,3,1,2,1,1,1,1,1,1,2,1,2,1,3,4,2,3,4,4,1,2,5,
  3,1,1,1,1,2,0,1,1,3,1,1,4,4,0,4,9,1,1,1,1,1,1,1,1,1,1,2,6,1,1,1,1,1,1,
  1,1,1,1,1,2,6,6,6,2,2,1,6,1,2,3,2,2,0,3,1,6,6,6,6,1,1,7,1,1,1,4,1,2,1,
  1,1,2,1,1,2,1,1,1,1,2,1,1,3,2,1,1,3,2,1,3,3,1,1,1,1,4,4,4,4,3,4,4,3,4,
  4,3,1,1,3,1,1,2,1,1,1,1,2,0,1,1,2,1,1,3,1,1,1,1,2,2,3,5,1,2,3,1,3,1,0,
  1,3,2,2,1,2,0,1,2,1,2,3,4,0,1,3,4,1,1,1,1,1,1,1,1,1,1,1,5,5,5,1,1,1,5,
  5,5,3,3,3,0,1,0,1,3,5,5,1,1,2,0,1,2,5,1,1,2,2,3,0,1,2,3,3,1,2,1,1,1,2,
  0,1,1,1,6,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,6,5,2,1,1,1,2,0,1,3,2,5,
  3,4,2,1,4,5,1,1,4,2,3
};
static const unsigned short ag_ptt[] = {
    0,  1,  1,  1,  1,  1,  1,  1,  1,  1, 18, 18,  3, 19, 21, 23, 25, 25,
   17, 26, 28, 30, 31, 33, 35, 37, 39, 41, 45, 45, 43, 48, 48, 46, 51, 51,
   49, 55, 56, 56, 57, 57, 52, 60, 61, 61, 62, 62, 58, 59, 59, 66, 66, 66,
   66, 66, 66, 72, 75, 75, 73, 76, 76, 77, 77, 78, 78, 80, 80,  8,  8, 79,
   81, 86, 87, 87, 88, 88, 82, 84,  4, 89, 89, 90, 93, 93, 91, 95, 95, 96,
   97, 99, 99, 99, 99, 99,104, 92, 92, 94, 94, 94, 94, 94, 94, 94, 94, 94,
   94,115,116,116,116,116,106,107,108,120,122,122,122,112,123,124,124,109,
  126,110,111,113,132,132,114,129,137, 11,139,143,143,140,144,147,147,141,
  150,150,148, 14, 12, 13,151,154,154,152,152,159,158,153,153,162,155,155,
  168,168,168,168,169,169,170,170,170,171,171,171,172,164,164,173,174,174,
  177,181,181,179,178,184,185,185,186,186,182,167,187,175,175,105,105,189,
  192,192,189,117,195,196,196,196,136,136,198,201,201,198,198,203,204,204,
  205,205,199,202,202,202,202,165,165,207,207,208,208,208,208,208,208,208,
  208,208,208,208,209,210,210,222,222,223,211,212,213,214,214,214,231,231,
  232,232,214,215,216,234,235,235,236,236,135,217,218,219,219,239,240,166,
  166,241,241,242,247,247,244,249,250,250,251,251,245,  9,252,253,253,253,
  253,253,253,253,253,253,253,253,253,253,253,138,138,138,138,267,267,145,
  145,269,270,270,271,271,268,268,272,274,274,275,275, 15, 15,  7,276,276,
  277,277
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
    case 15: V(0,ParserBasicToken) = ag_rp_15(PCB_POINTER); break;
    case 16: V(0,ParserBasicToken) = ag_rp_16(PCB_POINTER); break;
    case 17: V(0,ParserBasicToken) = ag_rp_17(PCB_POINTER); break;
    case 18: V(0,ParserBasicToken) = ag_rp_18(PCB_POINTER); break;
    case 19: V(0,ParserQuotedString) = ag_rp_19(PCB_POINTER); break;
    case 20: V(0,ParserMethod) = ag_rp_20(PCB_POINTER); break;
    case 21: V(0,ParserMethod) = ag_rp_21(PCB_POINTER); break;
    case 22: V(0,ParserMethod) = ag_rp_22(PCB_POINTER); break;
    case 23: V(0,ParserMethod) = ag_rp_23(PCB_POINTER); break;
    case 24: V(0,ParserMethod) = ag_rp_24(PCB_POINTER); break;
    case 25: V(0,ParserMethod) = ag_rp_25(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 26: ag_rp_26(PCB_POINTER, V(0,ParserContentLength)); break;
    case 27: V(0,ParserContentLength) = ag_rp_27(PCB_POINTER); break;
    case 28: ag_rp_28(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 29: ag_rp_29(PCB_POINTER, V(0,ParserBasicToken), V(2,ParserGenericParamDataVal)); break;
    case 30: V(0,ParserGenericParamDataVal) = ag_rp_30(PCB_POINTER, V(2,ParserBasicToken)); break;
    case 31: V(0,ParserGenericParamDataVal) = ag_rp_31(PCB_POINTER, V(2,ParserQuotedString)); break;
    case 32: ag_rp_32(PCB_POINTER, V(0,ParserSipVersion), V(2,ParserStatusCode), V(4,ParserReasonPhrase)); break;
    case 33: ag_rp_33(PCB_POINTER, V(0,ParserSipVersion), V(2,ParserStatusCode)); break;
    case 34: V(0,ParserSipVersion) = ag_rp_34(PCB_POINTER); break;
    case 35: V(0,ParserReasonPhrase) = ag_rp_35(PCB_POINTER); break;
    case 36: V(0,ParserStatusCode) = ag_rp_36(PCB_POINTER, V(0,int), V(1,int), V(2,int)); break;
    case 37: ag_rp_37(PCB_POINTER); break;
    case 38: ag_rp_38(PCB_POINTER); break;
    case 39: ag_rp_39(PCB_POINTER); break;
    case 40: ag_rp_40(PCB_POINTER); break;
    case 41: ag_rp_41(PCB_POINTER, V(3,ParserViaParam)); break;
    case 42: ag_rp_42(PCB_POINTER, V(0,ParserProtocolName), V(4,ParserProtocolVersion), V(8,ParserTransport)); break;
    case 43: V(0,ParserProtocolName) = ag_rp_43(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 44: V(0,ParserProtocolName) = ag_rp_44(PCB_POINTER); break;
    case 45: V(0,ParserProtocolVersion) = ag_rp_45(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 46: V(0,ParserTransport) = ag_rp_46(PCB_POINTER, V(0,ParserTransport)); break;
    case 47: V(0,ParserTransport) = ag_rp_47(PCB_POINTER); break;
    case 48: V(0,ParserTransport) = ag_rp_48(PCB_POINTER); break;
    case 49: V(0,ParserTransport) = ag_rp_49(PCB_POINTER); break;
    case 50: V(0,ParserTransport) = ag_rp_50(PCB_POINTER); break;
    case 51: V(0,ParserTransport) = ag_rp_51(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 52: V(0,ParserPort) = ag_rp_52(PCB_POINTER); break;
    case 53: ag_rp_53(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 54: ag_rp_54(PCB_POINTER, V(0,ParserBasicToken), V(4,ParserPort)); break;
    case 55: V(0,ParserViaParam) = ag_rp_55(PCB_POINTER); break;
    case 56: V(0,ParserViaParam) = ag_rp_56(PCB_POINTER); break;
    case 57: V(0,ParserViaParam) = ag_rp_57(PCB_POINTER, V(0,ParserTTLParam)); break;
    case 58: V(0,ParserViaParam) = ag_rp_58(PCB_POINTER, V(0,ParserMaddrParam)); break;
    case 59: V(0,ParserViaParam) = ag_rp_59(PCB_POINTER, V(0,ParserReceived)); break;
    case 60: V(0,ParserViaParam) = ag_rp_60(PCB_POINTER, V(0,ParserBranch)); break;
    case 61: V(0,ParserViaParam) = ag_rp_61(PCB_POINTER, V(0,ParserPort)); break;
    case 62: V(0,ParserViaParam) = ag_rp_62(PCB_POINTER, V(0,ParserCompParam)); break;
    case 63: V(0,ParserViaParam) = ag_rp_63(PCB_POINTER, V(0,ParserSigCompIdParam)); break;
    case 64: V(0,ParserViaParam) = ag_rp_64(PCB_POINTER); break;
    case 65: ag_rp_65(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 66: ag_rp_66(PCB_POINTER, V(0,ParserBasicToken), V(4,ParserBasicToken)); break;
    case 67: ag_rp_67(PCB_POINTER, V(0,ParserBasicToken), V(4,ParserQuotedString)); break;
    case 68: ag_rp_68(PCB_POINTER, V(0,ParserBasicToken), V(4,ParserBasicToken)); break;
    case 69: V(0,ParserTTLParam) = ag_rp_69(PCB_POINTER, V(0,ParserTtlParam)); break;
    case 70: V(0,ParserTtlParam) = ag_rp_70(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 71: V(0,ParserBasicToken) = ag_rp_71(PCB_POINTER); break;
    case 72: V(0,ParserBasicToken) = ag_rp_72(PCB_POINTER); break;
    case 73: V(0,ParserBasicToken) = ag_rp_73(PCB_POINTER); break;
    case 74: V(0,ParserPort) = ag_rp_74(PCB_POINTER, V(1,ParserPort)); break;
    case 75: V(0,ParserPort) = ag_rp_75(PCB_POINTER); break;
    case 76: V(0,ParserPort) = ag_rp_76(PCB_POINTER, V(2,ParserPort)); break;
    case 77: V(0,ParserMaddrParam) = ag_rp_77(PCB_POINTER, V(0,ParserMaddrParam)); break;
    case 78: V(0,ParserMaddrParam) = ag_rp_78(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 79: V(0,ParserReceived) = ag_rp_79(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 80: V(0,ParserBranch) = ag_rp_80(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 81: V(0,ParserCompParam) = ag_rp_81(PCB_POINTER, V(4,ParserCompParam)); break;
    case 82: V(0,ParserCompParam) = ag_rp_82(PCB_POINTER); break;
    case 83: V(0,ParserCompParam) = ag_rp_83(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 84: V(0,ParserSigCompIdParam) = ag_rp_84(PCB_POINTER, V(5,ParserBasicToken)); break;
    case 85: V(0,ParserBasicToken) = ag_rp_85(PCB_POINTER); break;
    case 86: ag_rp_86(PCB_POINTER, V(0,ParserAbsoluteUri)); break;
    case 87: V(0,ParserAbsoluteUri) = ag_rp_87(PCB_POINTER, V(0,ParserBasicToken), V(3,ParserBasicToken)); break;
    case 88: V(0,ParserBasicToken) = ag_rp_88(PCB_POINTER); break;
    case 89: V(0,ParserDisplayName) = ag_rp_89(PCB_POINTER, V(0,ParserDisplayName)); break;
    case 90: V(0,ParserBasicToken) = ag_rp_90(PCB_POINTER); break;
    case 91: V(0,ParserBasicToken) = ag_rp_91(PCB_POINTER); break;
    case 92: ag_rp_92(PCB_POINTER); break;
    case 93: ag_rp_93(PCB_POINTER, V(0,ParserAbsoluteUri)); break;
    case 94: ag_rp_94(PCB_POINTER, V(0,ParserAbsoluteUri)); break;
    case 95: ag_rp_95(PCB_POINTER); break;
    case 96: ag_rp_96(PCB_POINTER); break;
    case 97: V(0,ParserAbsoluteUri) = ag_rp_97(PCB_POINTER, V(2,ParserAbsoluteUri)); break;
    case 98: V(0,ParserAbsoluteUri) = ag_rp_98(PCB_POINTER); break;
    case 99: ag_rp_99(PCB_POINTER); break;
    case 100: ag_rp_100(PCB_POINTER); break;
    case 101: V(0,ParserAbsoluteUri) = ag_rp_101(PCB_POINTER, V(2,ParserAbsoluteUri)); break;
    case 102: V(0,ParserAbsoluteUri) = ag_rp_102(PCB_POINTER); break;
    case 103: ag_rp_103(PCB_POINTER); break;
    case 104: ag_rp_104(PCB_POINTER, V(2,ParserOptionalHeaders)); break;
    case 105: ag_rp_105(PCB_POINTER, V(2,ParserOptionalHeaders)); break;
    case 106: ag_rp_106(PCB_POINTER, V(3,ParserOptionalHeaders)); break;
    case 107: ag_rp_107(PCB_POINTER, V(3,ParserOptionalHeaders)); break;
    case 108: ag_rp_108(PCB_POINTER, V(3,ParserOptionalHeaders)); break;
    case 109: ag_rp_109(PCB_POINTER, V(3,ParserOptionalHeaders)); break;
    case 110: ag_rp_110(PCB_POINTER, V(3,ParserOptionalHeaders)); break;
    case 111: ag_rp_111(PCB_POINTER, V(3,ParserOptionalHeaders)); break;
    case 112: ag_rp_112(PCB_POINTER, V(0,ParserDisplayName), V(2,ParserBasicToken)); break;
    case 113: ag_rp_113(PCB_POINTER); break;
    case 114: ag_rp_114(PCB_POINTER); break;
    case 115: ag_rp_115(PCB_POINTER); break;
    case 116: ag_rp_116(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 117: ag_rp_117(PCB_POINTER, V(0,ParserBasicToken), V(2,ParserBasicToken)); break;
    case 118: V(0,ParserBasicToken) = ag_rp_118(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 119: V(0,ParserBasicToken) = ag_rp_119(PCB_POINTER); break;
    case 120: V(0,ParserBasicToken) = ag_rp_120(PCB_POINTER); break;
    case 121: ag_rp_121(PCB_POINTER); break;
    case 122: ag_rp_122(PCB_POINTER); break;
    case 123: ag_rp_123(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 124: ag_rp_124(PCB_POINTER, V(0,ParserBasicToken), V(2,ParserPort)); break;
    case 125: V(0,ParserBasicToken) = ag_rp_125(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 126: V(0,ParserBasicToken) = ag_rp_126(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 127: V(0,ParserBasicToken) = ag_rp_127(PCB_POINTER); break;
    case 128: V(0,ParserBasicToken) = ag_rp_128(PCB_POINTER); break;
    case 129: V(0,ParserBasicToken) = ag_rp_129(PCB_POINTER); break;
    case 130: ag_rp_130(PCB_POINTER); break;
    case 131: ag_rp_131(PCB_POINTER); break;
    case 132: ag_rp_132(PCB_POINTER); break;
    case 133: ag_rp_133(PCB_POINTER); break;
    case 134: ag_rp_134(PCB_POINTER, V(0,ParserTransport)); break;
    case 135: ag_rp_135(PCB_POINTER, V(0,ParserUserParam)); break;
    case 136: ag_rp_136(PCB_POINTER, V(0,ParserTtlParam)); break;
    case 137: ag_rp_137(PCB_POINTER, V(0,ParserMaddrParam)); break;
    case 138: ag_rp_138(PCB_POINTER, V(0,ParserMethod)); break;
    case 139: ag_rp_139(PCB_POINTER, V(0,ParserLrParamType)); break;
    case 140: ag_rp_140(PCB_POINTER, V(0,ParserCompParam)); break;
    case 141: ag_rp_141(PCB_POINTER, V(0,ParserSigCompIdParam)); break;
    case 142: ag_rp_142(PCB_POINTER, V(0,ParserTokenizedByParam)); break;
    case 143: ag_rp_143(PCB_POINTER, V(0,RvBool)); break;
    case 144: ag_rp_144(PCB_POINTER); break;
    case 145: V(0,ParserTransport) = ag_rp_145(PCB_POINTER, V(4,ParserTransport)); break;
    case 146: V(0,ParserUserParam) = ag_rp_146(PCB_POINTER, V(4,RvSipUserParam)); break;
    case 147: V(0,ParserUserParam) = ag_rp_147(PCB_POINTER, V(4,ParserOtherUser)); break;
    case 148: V(0,RvSipUserParam) = ag_rp_148(PCB_POINTER); break;
    case 149: V(0,RvSipUserParam) = ag_rp_149(PCB_POINTER); break;
    case 150: V(0,ParserOtherUser) = ag_rp_150(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 151: V(0,ParserTtlParam) = ag_rp_151(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 152: V(0,ParserMaddrParam) = ag_rp_152(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 153: V(0,ParserMethod) = ag_rp_153(PCB_POINTER, V(4,ParserMethod)); break;
    case 154: V(0,ParserLrParamType) = ag_rp_154(PCB_POINTER); break;
    case 155: V(0,ParserLrParamType) = ag_rp_155(PCB_POINTER); break;
    case 156: V(0,ParserLrParamType) = ag_rp_156(PCB_POINTER); break;
    case 157: V(0,ParserLrParamType) = ag_rp_157(PCB_POINTER); break;
    case 158: V(0,ParserCompParam) = ag_rp_158(PCB_POINTER, V(4,ParserCompParam)); break;
    case 159: V(0,ParserSigCompIdParam) = ag_rp_159(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 160: V(0,ParserBasicToken) = ag_rp_160(PCB_POINTER); break;
    case 161: V(0,ParserTokenizedByParam) = ag_rp_161(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 162: V(0,RvBool) = ag_rp_162(PCB_POINTER); break;
    case 163: ag_rp_163(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 164: ag_rp_164(PCB_POINTER, V(0,ParserBasicToken), V(1,ParserBasicToken)); break;
    case 165: V(0,ParserBasicToken) = ag_rp_165(PCB_POINTER); break;
    case 166: V(0,ParserBasicToken) = ag_rp_166(PCB_POINTER, V(2,ParserBasicToken)); break;
    case 167: V(0,ParserOptionalHeaders) = ag_rp_167(PCB_POINTER); break;
    case 168: V(0,ParserOptionalHeaders) = ag_rp_168(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 169: V(0,ParserBasicToken) = ag_rp_169(PCB_POINTER); break;
    case 170: V(0,ParserBasicToken) = ag_rp_170(PCB_POINTER); break;
    case 171: V(0,ParserBasicToken) = ag_rp_171(PCB_POINTER); break;
    case 172: V(0,ParserBasicToken) = ag_rp_172(PCB_POINTER); break;
    case 173: V(0,ParserBasicToken) = ag_rp_173(PCB_POINTER); break;
    case 174: ag_rp_174(PCB_POINTER); break;
    case 175: ag_rp_175(PCB_POINTER, V(0,ParserMethod), V(4,ParserSipVersion)); break;
    case 176: V(0,ParserMethod) = ag_rp_176(PCB_POINTER); break;
    case 177: V(0,ParserMethod) = ag_rp_177(PCB_POINTER); break;
    case 178: V(0,ParserMethod) = ag_rp_178(PCB_POINTER); break;
    case 179: V(0,ParserMethod) = ag_rp_179(PCB_POINTER); break;
    case 180: V(0,ParserMethod) = ag_rp_180(PCB_POINTER); break;
    case 181: V(0,ParserMethod) = ag_rp_181(PCB_POINTER); break;
    case 182: V(0,ParserMethod) = ag_rp_182(PCB_POINTER); break;
    case 183: V(0,ParserMethod) = ag_rp_183(PCB_POINTER); break;
    case 184: V(0,ParserMethod) = ag_rp_184(PCB_POINTER); break;
    case 185: V(0,ParserMethod) = ag_rp_185(PCB_POINTER); break;
    case 186: V(0,ParserMethod) = ag_rp_186(PCB_POINTER); break;
    case 187: V(0,ParserMethod) = ag_rp_187(PCB_POINTER); break;
    case 188: V(0,ParserMethod) = ag_rp_188(PCB_POINTER); break;
    case 189: V(0,ParserMethod) = ag_rp_189(PCB_POINTER, V(0,ParserMethod)); break;
    case 190: ag_rp_190(PCB_POINTER); break;
    case 191: ag_rp_191(PCB_POINTER, V(0,ParserAbsoluteUri)); break;
    case 192: ag_rp_192(PCB_POINTER, V(0,ParserAbsoluteUri)); break;
    case 193: ag_rp_193(PCB_POINTER, V(0,ParserAbsoluteUri)); break;
    case 194: ag_rp_194(PCB_POINTER, V(0,ParserDisplayName)); break;
    case 195: ag_rp_195(PCB_POINTER); break;
    case 196: V(0,ParserDisplayName) = ag_rp_196(PCB_POINTER, V(0,ParserQuotedString)); break;
    case 197: V(0,ParserDisplayName) = ag_rp_197(PCB_POINTER, V(0,ParserBasicToken)); break;
    case 198: V(0,ParserBasicToken) = ag_rp_198(PCB_POINTER); break;
    case 199: V(0,ParserBasicToken) = ag_rp_199(PCB_POINTER); break;
    case 200: V(0,ParserTagParam) = ag_rp_200(PCB_POINTER, V(4,ParserBasicToken)); break;
    case 201: V(0,ParserPartyParams) = ag_rp_201(PCB_POINTER, V(2,ParserPartyParams)); break;
    case 202: V(0,ParserPartyParams) = ag_rp_202(PCB_POINTER, V(0,ParserPartyParams), V(3,ParserPartyParams)); break;
    case 203: V(0,ParserPartyParams) = ag_rp_203(PCB_POINTER, V(0,ParserTagParam)); break;
    case 204: V(0,ParserPartyParams) = ag_rp_204(PCB_POINTER); break;
    case 205: ag_rp_205(PCB_POINTER, V(0,ParserMethod)); break;
    case 206: ag_rp_206(PCB_POINTER, V(0,ParserMethod), V(4,ParserBasicToken)); break;
    case 207: ag_rp_207(PCB_POINTER); break;
    case 208: ag_rp_208(PCB_POINTER); break;
    case 209: ag_rp_209(PCB_POINTER); break;
    case 210: ag_rp_210(PCB_POINTER); break;
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
  if ((PCB).ssx >= 26) {
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
  if ((PCB).ssx >= 26) {
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
    if ((PCB).ssx >= 26) {
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
    if ((PCB).ssx >= 26) {
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
