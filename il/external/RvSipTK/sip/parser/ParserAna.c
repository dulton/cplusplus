/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                           ParserAna.c
 *
 * The file contains rules which parse sip encoded message or sip header and
 * reduction methods which build the message or the header.
 *
 *      Author           Date
 *     ------           ------------
 *  Michal Mashiach      Nov.2000
 ******************************************************************************/


/*----------------------- C DECLARATIONS ----------------------*/

#include "RV_SIP_DEF.h"

#include <stdio.h>
#include "ParserTypes.h"
#include "ParserProcess.h"
#include "ParserUtils.h"
#include "_SipCommonUtils.h"

#ifdef RV_SIP_IMS_HEADER_SUPPORT
#include "RvSipPChargingVectorHeader.h"
#include "_SipPChargingVectorHeader.h"
#include "RvSipPChargingFunctionAddressesHeader.h"
#include "_SipPChargingFunctionAddressesHeader.h"
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 

/***************************************************************************
 * This macros is used in the parser to handle with tokens created.
 * In general, PCB.pointer is data member of the parser which always pointes
 * to the next input character to be read.
 ***************************************************************************/
/* =====================
    GENERAL MACROS
   =====================*/

/* SIP_BEGIN_LINE() is used to save the PCB.pointer in a begining of a line*/
#define SIP_BEGIN_LINE()     (PCB.pCurLine=(RvChar *)PCB.pointer)

/* SIP_BEGIN_TOKEN() is used to save the PCB.pointer in a begining of a token*/
#define SIP_BEGIN_TOKEN()     (PCB.pCurToken=(RvChar *)PCB.pointer)

/* SIP_TOKEN_START is used to save pointer to the begining of a token */
#define SIP_TOKEN_START          (PCB.pCurToken)

/* SIP_LINE_START is used to save pointer to the begining of a Line */
#define SIP_LINE_START          (PCB.pCurLine)

/* SIP_TOKEN_LENGTH is used to save the length of the current token */
#define SIP_TOKEN_LENGTH      (RvUint32)((RvChar * )PCB.pointer-SIP_TOKEN_START)

/* CUR_STRING() create ParserBasicToken structure from the current token */
#define CUR_STRING()          ParserCurString(SIP_TOKEN_START,SIP_TOKEN_LENGTH)

#define SYNTAX_ERROR          ParserSetSyntaxErr(&(PCB))

/* SIP_LINE_LENGTH is used to save the length of the current token (used for the
   domain parameter inside the authentication header */
#define SIP_LINE_LENGTH      (RvUint32)((RvChar * )PCB.pointer-SIP_LINE_START)

#define REMOVE_COMMA_BETWEEN_HEADERS()    (*(PCB.pointer-1)=' ')

/* CUR_STRING() create ParserBasicToken structure from the current token
   using SIP_LINE_START macro */
#define CUR_LINE()          ParserCurString(SIP_LINE_START,SIP_LINE_LENGTH)

/* SET_HEADER_VALUE() is used to set the value of the header to the PCB.pointer and to
   the PCB.pCurToken */
#define SET_HEADER_VALUE()  ParserSetHeaderValue(&(PCB))

#define SET_PARSER_PCB_BACK_TO_BUFF(prefix)        ParserSetBackPcbToMsgBuffer(&(PCB), prefix)


/* APPEND_DATA() is used to append data (usually, extended params) to a page */
#define APPEND_DATA(pData, sizeToCopy, pExtParams)    ParserAppend(pData, sizeToCopy, pExtParams, &(PCB));


/* =====================
    URI MACROS
   =====================*/
#define DEFINE_URL_PREFIX()               ParserDefineUrlPrefix(&(PCB))

#define DEFINE_PRES_IM_PREFIX(bIm)        ParserDefinePresImPrefix(&(PCB), bIm)

#define DEFINE_OLD_NEW_ADDR_SPEC_PREFIX() ParserDefineOldNewAddrSpecPrefix(&(PCB))

#define INIT_PRES_IM_STRUCT(uri)          ParserInitPresImStruct(&(PCB), uri)

#define SET_URI_TYPE(uriType, uri)        ParserSetUriType(&(PCB), uriType, uri)

#define RESET_URI_PARAMS()                ParserResetUrlParams(&(PCB))

#ifdef RV_SIP_TEL_URI_SUPPORT
/* check whether the last char that was read was '-' */
#define LAST_CHAR_IS_HYPHEN() (*(SIP_TOKEN_START+SIP_TOKEN_LENGTH-1)=='-')
#endif

/* =====================
    SPECIFIC MACROS - for reducing parser footprint
   =====================*/

#define CUR_METHOD(eMethod)          ParserCurMethod(eMethod)

#define CUR_REQUEST_COMPACT_METHOD(strMethod)         ParserCurRequestCompactMethod(strMethod)

#ifndef RV_SIP_PRIMITIVES
#define CUR_MEDIA_TYPE(eMedia)  ParserCurMediaType(eMedia)

#define CUR_MEDIA_SUB_TYPE(eMediaSubType)  ParserCurMediaSubType(eMediaSubType)
#endif /*#ifndef RV_SIP_PRIMITIVES*/

#ifdef RV_SIP_SUBS_ON
#define CUR_SUBS_STATE_REASON(eType) ParserCurSubsStateReason(eType)
#endif

#ifndef RV_SIP_PRIMITIVES
#define CUR_DISPOSITION_TYPE(eType)  ParserCurDispositionType(eType)
#endif

#ifdef RV_SIP_AUTH_ON
#define CUR_AUTH_CHALLENGE(eType, challenge) ParserSetDigestChallenge(&(PCB), eType, challenge)
#endif

#define DEFINE_AKAV_PREFIX()			ParserDefineAKAvPrefix(&(PCB))

#ifdef RV_SIP_IMS_HEADER_SUPPORT
#define CUR_EMPTY_FEATURE_TAG()			ParserCurEmptyContactFeatureTag()
#define CUR_FEATURE_TAG(value)		    ParserCurContactFeatureTag(value)
#define CUR_ACCESS(eAccessType)			ParserCurAccess(eAccessType)
#define CUR_MECHANISM(eMechanismType)	ParserCurMechanism(eMechanismType)
#define CUR_CPC(eCPCType)	            ParserCurCPC(eCPCType)
#define INFO_LIST_ADD_ELEMENT(element)	ParserInfoListAddElement(&(PCB), element)
#define CUR_ANSWER(eAnswerType)			ParserCurAnswer(eAnswerType)
#define P_CHARGING_FUNCTION_ADDRESSES_LIST_ADD_ELEMENT(element)	ParserPChargingFunctionAddressesListAddElement(&(PCB), element)
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 

#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
#define CUR_OSPS_TAG(eOSPSTagType)		ParserCurOSPSTag(eOSPSTagType)
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 

/***************************************************************************
 * ParseCurString
 * ------------------------------------------------------------------------
 * General: This function is used only in the parser to construct a ParserBasicToken
 *          structure which include a string and length integer.
 * Return Value: ParserBasicToken structure.
 * ------------------------------------------------------------------------
 * Arguments:
 *    str - The string to be set in the structure.
 *    len - The integer length to be set in the structure.
 ***************************************************************************/
static ParserBasicToken ParserCurString(RvChar * str,RvUint32 len)
{
    ParserBasicToken basicToken;
    basicToken.buf = str;
    basicToken.len = len;
    return basicToken;
}

/***************************************************************************
 * ParserCurMethod
 * ------------------------------------------------------------------------
 * General: returns a method struct
 * Return Value: ParserMethod structure.
 * ------------------------------------------------------------------------
 * Arguments:
 *    eMethod - enumeration of the method.
 ***************************************************************************/
static ParserMethod ParserCurMethod(ParserMethodType eMethod)
{
    ParserMethod method;
    method.type = eMethod;
	method.other.buf = NULL;
	method.other.len = 0;
    return method;
}

/***************************************************************************
 * ParserCurRequestCompactMethod
 * ------------------------------------------------------------------------
 * General: returns a method struct
 * Return Value: ParserMethod structure.
 * ------------------------------------------------------------------------
 * Arguments:
 *    eMethod - enumeration of the method.
 ***************************************************************************/
static ParserMethod ParserCurRequestCompactMethod(RvChar* strMethod)
{
    ParserMethod met;
    met.other.buf = strMethod;
    met.other.len = 1;
    met.type = PARSER_METHOD_TYPE_OTHER;
    return met;
}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * ParserCurMediaType
 * ------------------------------------------------------------------------
 * General: returns a method struct
 * Return Value: ParserMethod structure.
 * ------------------------------------------------------------------------
 * Arguments:
 *    eMethod - enumeration of the method.
 ***************************************************************************/
static ParserMediaType ParserCurMediaType(RvSipMediaType  eMedia)
{
    ParserMediaType mediaType;
    mediaType.type = eMedia;
    mediaType.other.buf = NULL;
    mediaType.other.len = 0;
	return mediaType;
}

/***************************************************************************
 * ParserCurMediaSubType
 * ------------------------------------------------------------------------
 * General: returns a method struct
 * Return Value: ParserMethod structure.
 * ------------------------------------------------------------------------
 * Arguments:
 *    eMediaSubType - enumeration of the method.
 ***************************************************************************/
static ParserMediaSubType ParserCurMediaSubType(RvSipMediaSubType   eMediaSubType)
{
    ParserMediaSubType mediaSubType;
    mediaSubType.type = eMediaSubType;
    mediaSubType.other.buf = NULL;
    mediaSubType.other.len = 0;
    return mediaSubType;
}
#endif /*#ifndef RV_SIP_PRIMITIVES*/ 

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * ParserCurSubsStateReason
 * ------------------------------------------------------------------------
 * General: returns a method struct
 * Return Value: ParserMethod structure.
 * ------------------------------------------------------------------------
 * Arguments:
 *    eMediaSubType - enumeration of the method.
 ***************************************************************************/
static ParserSubsStateReason ParserCurSubsStateReason(ParserSubsStateReasonTypes   eType)
{
    ParserSubsStateReason reason;
    reason.substateReason = eType;
    reason.otherSubstateReason.buf = NULL;
    reason.otherSubstateReason.len = 0;
    return reason;
}
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * ParserCurDispositionType
 * ------------------------------------------------------------------------
 * General: returns a method struct
 * Return Value: ParserMethod structure.
 * ------------------------------------------------------------------------
 * Arguments:
 *    eMediaSubType - enumeration of the method.
 ***************************************************************************/
static ParserDisposition ParserCurDispositionType(ParserDispositionType eType)
{
    ParserDisposition disposition;
    disposition.eDispositionType = eType;
    disposition.otherDispositionType.buf = NULL;
    disposition.otherDispositionType.len = 0;
    return disposition;
}
#endif /*#ifndef RV_SIP_PRIMITIVES*/

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * ParserSetDigestChallenge
 * ------------------------------------------------------------------------
 * General: This function sets the challenge results in the PCB structure.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pcb_ - pointer to the PCB structure of the parser.
 ***************************************************************************/
void ParserSetDigestChallenge(IN void * pcb_, ParserDigestChallengeType eType, ParserBasicToken challenge)
{
    SipParser_pcb_type * pcb = (SipParser_pcb_type*)pcb_;

    pcb->digestChallenge.eType = eType;
    pcb->digestChallenge.challenge = challenge;
}
#endif

/***************************************************************************
 * ParseAppend
 * ------------------------------------------------------------------------
 * General: This function is used only in the parser to append data(usually
 *          for extended parameters).
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pData      - The string to be appended.
 *    sizeToCopy - The integer length of the appended string.
 *  pExtParams - The string that pData is going to be appended to.
 ***************************************************************************/
RvStatus ParserAppend(const RvChar * pData,RvUint32 sizeToCopy, void * pExtParams, void * pcb_)
{
    RvStatus          status;
    SipParser_pcb_type * pcb = (SipParser_pcb_type*)pcb_;

    status = ParserAppendData(pData, sizeToCopy, pExtParams);
    if (status != RV_OK)
    {
        /* Error occurred in while appending data */
        pcb->exit_flag = AG_REDUCTION_ERROR_CODE;
        pcb->eStat = RV_SIPPARSER_STOP_REDUCTION_ERROR;
    }
    return status;
}

/***************************************************************************
 * ParserSetHeaderValue
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the header to the
 *          PCB.pointer and to the PCB.pCurToken.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pcb_ - pointer to the PCB structure of the parser.
 ***************************************************************************/
void ParserSetHeaderValue(IN void * pcb_)
{
    SipParser_pcb_type * pcb = (SipParser_pcb_type*)pcb_;

    pcb->pointer   = (RvUint8*)pcb->pBuffer;
    pcb->pCurToken = (RvChar*)pcb->pBuffer;

     /* need to decrease the length of prefix from the column calculation */
    pcb->column -= HEADER_PREFIX_LEN;

}

/***************************************************************************
 * ParserSetUriType
 * ------------------------------------------------------------------------
 * General: This function sets the uri results in the PCB structure.
 *          The purpose of this function is to remove the RV_SIP_OTHER_URI_SUPPORT
 *          compilation flag in the syn file.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pcb_ - pointer to the PCB structure of the parser.
 ***************************************************************************/
void ParserSetUriType(IN void * pcb_, IN ParserTypeAddr uriType, ParserAbsoluteUri  absUri )
{
    SipParser_pcb_type * pcb = (SipParser_pcb_type*)pcb_;
    switch (uriType)
    {
    case PARSER_ADDR_PARAM_TYPE_SIP_URL:
        pcb->exUri.uriType = uriType;
        pcb->exUri.ExUriInfo.SipUrl = pcb->sipUrl;
        break;
    case PARSER_ADDR_PARAM_TYPE_ABS_URI:
        pcb->exUri.uriType = uriType;
        pcb->exUri.ExUriInfo.absUri = absUri;
        break;
#ifdef RV_SIP_TEL_URI_SUPPORT
    case PARSER_ADDR_PARAM_TYPE_TEL_URI:
        pcb->exUri.uriType = uriType;
        pcb->exUri.ExUriInfo.telUri = pcb->telUri;
        break;
#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */        
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    case PARSER_ADDR_PARAM_TYPE_DIAMETER_URI:
        pcb->exUri.uriType = uriType;
        pcb->exUri.ExUriInfo.diameterUri = pcb->diameterUri;
        break;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
    case PARSER_ADDR_PARAM_TYPE_IM_URI:
    case PARSER_ADDR_PARAM_TYPE_PRES_URI:
#ifdef RV_SIP_OTHER_URI_SUPPORT
        pcb->exUri.uriType = uriType;
        pcb->exUri.ExUriInfo.SipUrl = pcb->sipUrl;
#else
        pcb->exUri.uriType = PARSER_ADDR_PARAM_TYPE_ABS_URI;
        pcb->exUri.ExUriInfo.absUri = absUri;
#endif /*RV_SIP_OTHER_URI_SUPPORT*/
        break;
    default:
        break;
    }
}

/***************************************************************************
 * ParserSetBackPcbToMsgBuffer
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the header to the
 *          PCB.pointer and to the PCB.pCurToken.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pcb_ - pointer to the PCB structure of the parser.
 ***************************************************************************/
void ParserSetBackPcbToMsgBuffer(IN void * pcb_,
                                IN ParseMarkPrefix ePrefix)
{
    SipParser_pcb_type * pcb = (SipParser_pcb_type*)pcb_;
    pcb->pointer   = (RvUint8*)pcb->pBuffer;
    pcb->pCurToken = (RvChar*)pcb->pBuffer;

    /* need to decrease the length of prefix from the column calculation */
    switch (ePrefix)
    {
    case RV_SIPPARSER_PREFIX_USERINFO:
        pcb->column -= 9; /* 9 is the length of "USERINFO:" */
        break;
    case RV_SIPPARSER_PREFIX_HOST:
        pcb->column -= 5; /* 5 is the length of "HOST:" */
        break;
    case RV_SIPPARSER_PREFIX_RV_ABS:
        pcb->column -= 7; /* 8 is the length of "RV-ABS:" */
        break;
	case RV_SIPPARSER_PREFIX_AKAV:
		pcb->column -= 7; /* 7 is the length of "AKAVER:" */
		break;
	case RV_SIPPARSER_PREFIX_ALGORITHM:
		pcb->column -= 5; /* 5 is the length of "ALGO:" */
		break;
	case RV_SIPPARSER_PREFIX_OLD_ADDR_SPEC:
		pcb->column -= 12; /* 12 is the length of "OLDADDRSPEC:" */
		break;
	case RV_SIPPARSER_PREFIX_NEW_ADDR_SPEC:
		pcb->column -= 12; /* 12 is the length of "NEWADDRSPEC:" */
		break;
    default:
        break;
    }
}

/***************************************************************************
 * ParserDefineUrlPrefix
 * ------------------------------------------------------------------------
 * General: This function is called when a sip or sips prefix of url is found.
 *          The function checks if there is a '@' in this url. (checks until
 *          finding a CRLF).
 *          if there is a @, set the pcb pointer to point to a 'userinfo:'
 *          else set it to point to a 'host:' prefix.
 *          when returning from this function, parser will simply continue
 *          according to these prefixes, this way we won't have the ambiguity
 *          of HOST characters and USER_INFO characters.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pcb_ - pointer to the PCB structure of the parser.
 ***************************************************************************/
void ParserDefineUrlPrefix(IN void * pcb_)
{
    SipParser_pcb_type* pcb                    = (SipParser_pcb_type*)pcb_;
    RvChar*             tempPointer            = (RvChar*)pcb->pointer;
    RvBool              bFound                 = RV_FALSE;
    static const RvChar URL_USERINFO_PREFIX[9] = {'U','S','E','R','I','N','F','O',':'}; /*"userinfo:"*/
    static const RvChar URL_HOST_PREFIX[5]     = {'H','O','S','T',':'}; /*"host:"*/
    RvBool              bStartLine             = (pcb->eWhichHeader==SIP_PARSETYPE_UNDEFINED)?RV_TRUE:RV_FALSE;

    pcb->pBuffer = (RvChar*)pcb->pointer;

    /* search the @. from version 4.0, each line is ended with NULL */
    while(*tempPointer != '\0' && 
          *tempPointer != '>' && *tempPointer != '"')
    {
        /* , should end the search, since it is the end of a header when 
           concatenating several headers to a single line */
        if(*tempPointer == ',')
        {
            if(pcb->isWithinAngleBrackets == RV_FALSE)
            {
                if(bStartLine == RV_FALSE) /* start-line can't be end with , */
                {
                    bFound = RV_FALSE;
                    break;
                }
            }
        }
        if(*tempPointer == '@')
        {
            bFound = RV_TRUE;
            break;
        }
        ++tempPointer;
    }

    if(bFound == RV_TRUE)
    {
        /* url with userinfo string */
        /* pcb->pointer will point to the static URL_USERINFO_PREFIX prefix */
        pcb->pointer = (RvUint8 *)&URL_USERINFO_PREFIX;
    }
    else
    {
        /* only host string */
        /* pcb->pointer will point to the static URL_HOST_PREFIX prefix */
        pcb->pointer = (RvUint8 *)&URL_HOST_PREFIX;
    }
}

/***************************************************************************
 * ParserDefineOldNewAddrSpecPrefix
 * ------------------------------------------------------------------------
 * General: This function is called when we need to distinguish between old 
 *          (RFC 822) and new addr spec. The function checks if there is a 
 *          ':' in this url. (checks until finding a CRLF).
 *          if there is a ':', set the pcb pointer to point to a 'NEWADDRSPEC:'
 *          else set it to point to a 'OLDADDRSPEC:' prefix.
 *          when returning from this function, parser will simply continue
 *          according to these prefixes, this way we won't have the ambiguity
 *          of new addr_spec characters and old addr_spec characters.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pcb_ - pointer to the PCB structure of the parser.
 ***************************************************************************/
void ParserDefineOldNewAddrSpecPrefix(IN void * pcb_)
{
    SipParser_pcb_type  *pcb                     = (SipParser_pcb_type*)pcb_;
    RvChar*             tempPointer              = (RvChar*)pcb->pointer;
    RvBool              bFound                   = RV_FALSE;
    static const RvChar OLD_ADDR_SPEC_PREFIX[12] = {'O','L','D','A','D','D','R','S','P','E','C',':'}; 
    static const RvChar NEW_ADDR_SPEC_PREFIX[12] = {'N','E','W','A','D','D','R','S','P','E','C',':'}; 
    RvBool              bStartLine = (pcb->eWhichHeader==SIP_PARSETYPE_UNDEFINED)?RV_TRUE:RV_FALSE;

    pcb->pBuffer = (RvChar*)pcb->pointer;

    /* search the ':', from version 4.0, each line is ended with NULL */
    while(*tempPointer != '\0' && 
          *tempPointer != '>' && *tempPointer != '"')
    {
        /* , should end the search, since it is the end oh a header when 
           concatenating several headers to a single line */
        if(*tempPointer == ',')
        {
            if(pcb->isWithinAngleBrackets == RV_FALSE)
            {
                if(bStartLine == RV_FALSE) /* start-line can't be end with , */
                {
                    bFound = RV_FALSE;
                    break;
                }
            }
        }
        if(*tempPointer == ':')
        {
            bFound = RV_TRUE;
            break;
        }
        ++tempPointer;
    }

    if(bFound == RV_TRUE)
    {
        /* New Addr Spec */
        /* pcb->pointer will point to the static NEW_ADDR_SPEC_PREFIX prefix */
        pcb->pointer = (RvUint8 *)&NEW_ADDR_SPEC_PREFIX;
    }
    else
    {
        /* Old Addr Spec */
        /* pcb->pointer will point to the static OLD_ADDR_SPEC_PREFIX prefix */
        pcb->pointer = (RvUint8 *)&OLD_ADDR_SPEC_PREFIX;
    }
}

/***************************************************************************
 * ParserDefineAKAvPrefix
 * ------------------------------------------------------------------------
 * General: This function is called when an algorithm prefix of Authenticate is found.
 *          The function checks if the field is formatted as: "AKAV" Digits "-" Algorithm.
 *          if so, set the pcb pointer to point to a 'AKAVER:'
 *          else set it to point to a 'ALGO:' prefix.
 *          when returning from this function, parser will simply continue
 *          according to these prefixes, this way we won't have the ambiguity
 *          of AKAV verses a proprietary Algorithm.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pcb_ - pointer to the PCB structure of the parser.
 ***************************************************************************/
void ParserDefineAKAvPrefix(IN void * pcb_)
{
    SipParser_pcb_type *pcb				= (SipParser_pcb_type*)pcb_;
    RvChar*            tempPointer		= (RvChar*)pcb->pointer;
    RvBool             bAKAv			= RV_TRUE;
    static const RvChar AKAV_PREFIX[7]		= {'A','K','A','V','E','R',':'};/*"akaver:"*/
    static const RvChar ALGORITHM_PREFIX[5]	= {'A','L','G','O',':'};		/*"algo:"*/
	RvChar AKAV[4]							= {'A','K','A','V'};			/*"akav"*/
    int i;

	pcb->pBuffer = (RvChar*)pcb->pointer;

    /* search for the format: akav, digits, -  */
	if(SipCommonStricmpByLen(tempPointer, (RvChar*)AKAV, 4) != RV_TRUE) 
	{
		bAKAv = RV_FALSE;
	}
	
    if(bAKAv == RV_TRUE)
    {
		bAKAv = RV_FALSE;
		i=4;
		while (i < 1000)
		{
			if(tempPointer[i] > '0' && tempPointer[i] < '9')
			{
				bAKAv = RV_TRUE;
			}
			else if(tempPointer[i] == '-'  ||
					tempPointer[i] == '\0' || 
					tempPointer[i] == ','  ||
					tempPointer[i] == '"')
			{
				break;
			}
			else
			{
				bAKAv = RV_FALSE;
				break;
			}
			
			++i;
		}

		if(bAKAv == RV_TRUE)
		{
			if(tempPointer[i] == '-')
			{
				/* the aka version was specified: */
				/* pcb->pointer will point to the static AKAV_PREFIX prefix */
				pcb->pointer = (RvUint8 *)&AKAV_PREFIX;
				return;
			}
		}
    }
    
    /* only algorithm string */
    /* pcb->pointer will point to the static ALGORITHM_PREFIX prefix */
    pcb->pointer = (RvUint8 *)&ALGORITHM_PREFIX;
}

#ifndef RV_SIP_OTHER_URI_SUPPORT
/***************************************************************************
 * moveBufferBackToAbsScheme
 * ------------------------------------------------------------------------
 * General: 
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pcb_ - pointer to the PCB structure of the parser.
 ***************************************************************************/
static void moveBufferBackToAbsScheme(IN SipParser_pcb_type * pcb, IN RvBool bIm)
{
    /*1. go back to the colon */
    while (*(pcb->pBuffer) != ':')
    {
        --(pcb->pBuffer);
    }

    /*2. go back over spaces */
    while (*(pcb->pBuffer) == ' ')
    {
        --(pcb->pBuffer);
    }

    /* 3. go over IM or PRES */
    if(bIm == RV_TRUE)
    {
        pcb->pBuffer -= 2; /* length of IM */
    }
    else
    {
        pcb->pBuffer -= 4; /* length of PRES */
    }
}
#endif /*#ifndef RV_SIP_OTHER_URI_SUPPORT*/

/***************************************************************************
 * ParserDefinePresImPrefix
 * ------------------------------------------------------------------------
 * General: if the stack is compiled with RV_SIP_OTHER_URI_SUPPORT:
 *          we parse the im and pres urls the same as sip url. (because
 *          it has the same syntax).
 *          if NOT compiled with RV_SIP_OTHER_URI_SUPPORT:
 *          We want the im and pres to be parsed as abs-uri.
 *          The purpose of this function is to omit the compilation flag
 *          from the syn file.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pcb_ - pointer to the PCB structure of the parser.
 ***************************************************************************/
void ParserDefinePresImPrefix(IN void * pcb_, IN RvBool bIm)
{
    SipParser_pcb_type * pcb              = (SipParser_pcb_type*)pcb_;
    static RvChar RV_ABS_PREFIX[7] = {'R','V','-','A','B','S',':'}; /*"RV-ABS:"*/

    pcb->pBuffer = (RvChar*)pcb->pointer;
#ifdef RV_SIP_OTHER_URI_SUPPORT
    ParserDefineUrlPrefix(pcb);
    RV_UNUSED_ARG(bIm)
    RV_UNUSED_ARGS(RV_ABS_PREFIX);
#else
    /* handle pres as abs-uri */
    /* pcb->pointer will point to the static RV_ABS_PREFIX prefix */
    pcb->pointer = (RvUint8 *)&RV_ABS_PREFIX;
    /* need to move the pcb buffer back to the beginning of the scheme */
    moveBufferBackToAbsScheme(pcb, bIm);
#endif
}

/***************************************************************************
 * ParserInitPresImStruct
 * ------------------------------------------------------------------------
 * General:
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pcb_ - pointer to the PCB structure of the parser.
 ***************************************************************************/
void ParserInitPresImStruct(IN void * pcb_, IN ParserAbsoluteUri uri)
{
    SipParser_pcb_type * pcb              = (SipParser_pcb_type*)pcb_;
    RvStatus status;

#ifdef RV_SIP_OTHER_URI_SUPPORT
    status = ParserInitUrl(pcb->pParserMgr,pcb,&(pcb->sipUrl), pcb->eHeaderType, pcb->pSipObject);
    ParserCleanExtParams (pcb->pUrlExtParams);
    ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SIP_URL, pcb);
    ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_URL_PARAMETER, pcb);
    RV_UNUSED_ARGS(&uri);
#else
    status = ParserInitAbsUri(pcb->pParserMgr, pcb, &uri, pcb->eHeaderType, pcb->pSipObject);
#endif
    if (RV_OK != status)
    {
        /* Error occurred in the reduction function*/
        pcb->exit_flag = AG_REDUCTION_ERROR_CODE;
        pcb->eStat = status;
    }
}

/***************************************************************************
 * ParserResetUrlParams
 * ------------------------------------------------------------------------
 * General:
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pcb_ - pointer to the PCB structure of the parser.
 ***************************************************************************/
void ParserResetUrlParams(IN void * pcb_)
{
    SipParser_pcb_type * pcb = (SipParser_pcb_type*)pcb_;

    pcb->sipUrl.urlParameters.isValid       = RV_FALSE;
    pcb->sipUrl.urlParameters.isMaddrParam  = RV_FALSE;
    pcb->sipUrl.urlParameters.isTransport   = RV_FALSE;
    pcb->sipUrl.urlParameters.isUserParam   = RV_FALSE;
    pcb->sipUrl.urlParameters.isOtherParams = RV_FALSE;
    pcb->sipUrl.urlParameters.isTtlParam    = RV_FALSE;
    pcb->sipUrl.urlParameters.isMethodParam = RV_FALSE;
    pcb->sipUrl.urlParameters.isCompParam   = RV_FALSE;
	pcb->sipUrl.urlParameters.isSigCompIdParam = RV_FALSE;
    pcb->sipUrl.urlParameters.lrParamType   = ParserLrParamUndefined;
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    pcb->sipUrl.urlParameters.isCpcParam    = RV_FALSE;
    pcb->sipUrl.urlParameters.isGrParam     = RV_FALSE;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
}

#ifdef RV_SIP_IMS_HEADER_SUPPORT
/***************************************************************************
 * ParserCurCPC
 * ------------------------------------------------------------------------
 * General: returns an CPC struct
 * Return Value: ParserCPC structure.
 * ------------------------------------------------------------------------
 * Arguments:
 *    eCPCType - enumeration of the method.
 ***************************************************************************/
static ParserCPCParam ParserCurCPC(RvSipUriCPCType eCPCType)
{
    ParserCPCParam cpc;
    cpc.cpcType = eCPCType;
	cpc.strCpcParam.buf = NULL;
	cpc.strCpcParam.len = 0;
    return cpc;
}

/***************************************************************************
 * ParserCurEmptyContactFeatureTag
 * ------------------------------------------------------------------------
 * General: returns an empty ContactFeatureTag struct
 * Return Value: ParserContactFeatureTag structure.
 * ------------------------------------------------------------------------
 * Arguments:
 *    
 ***************************************************************************/
static ParserContactFeatureTag ParserCurEmptyContactFeatureTag()
{
    ParserContactFeatureTag featureTag;
    featureTag.value.buf = NULL;
    featureTag.value.len = 0;
    featureTag.isValue = RV_FALSE;
    return featureTag;
}

/***************************************************************************
 * ParserCurContactFeatureTag
 * ------------------------------------------------------------------------
 * General: returns an empty ContactFeatureTag struct
 * Return Value: ContactFeatureTag structure.
 * ------------------------------------------------------------------------
 * Arguments:
 *    value - string value of the method.
 ***************************************************************************/
static ParserContactFeatureTag ParserCurContactFeatureTag(ParserBasicToken value)
{
    ParserContactFeatureTag featureTag;
    featureTag.value = value;
    featureTag.isValue = RV_TRUE;
    return featureTag;
}

/***************************************************************************
 * ParserCurAccess
 * ------------------------------------------------------------------------
 * General: returns an access struct
 * Return Value: ParserAccess structure.
 * ------------------------------------------------------------------------
 * Arguments:
 *    eAccessType - enumeration of the method.
 ***************************************************************************/
static ParserAccess ParserCurAccess(ParserAccessType eAccessType)
{
    ParserAccess access;
    access.type = eAccessType;
    access.other.buf = NULL;
    access.other.len = 0;
    return access;
}

/***************************************************************************
 * ParserCurMechanism
 * ------------------------------------------------------------------------
 * General: returns an Mechanism struct
 * Return Value: ParserMechanism structure.
 * ------------------------------------------------------------------------
 * Arguments:
 *    eMechanismType - enumeration of the method.
 ***************************************************************************/
static ParserMechanism ParserCurMechanism(ParserMechanismType eMechanismType)
{
    ParserMechanism mechanism;
    mechanism.type = eMechanismType;
	mechanism.other.buf = NULL;
	mechanism.other.len = 0;
    return mechanism;
}

/***************************************************************************
 * ParserCurAnswer
 * ------------------------------------------------------------------------
 * General: returns an answer struct
 * Return Value: ParserAnswer structure.
 * ------------------------------------------------------------------------
 * Arguments:
 *    eAnswerType - enumeration of the method.
 ***************************************************************************/
static ParserAnswer ParserCurAnswer(ParserAnswerType eAnswerType)
{
    ParserAnswer answer;
    answer.type = eAnswerType;
    answer.other.buf = NULL;
    answer.other.len = 0;
    return answer;
}

/***************************************************************************
 * ParserInfoListAddElement
 * ------------------------------------------------------------------------
 * General: add an element to the info list in PChargingVectorHeader
 * Return Value: None.
 * ------------------------------------------------------------------------
 * Arguments:
 *    element - .
 *    element - .
 ***************************************************************************/
static RvStatus ParserInfoListAddElement(IN void								 *pcb_,
										 IN ParserPChargingVectorInfoListElement element)
{
	SipParser_pcb_type						*pcb  = (SipParser_pcb_type*)pcb_;
	RvSipCommonListHandle					hList = NULL;
    RvStatus								status; 
    HRPOOL									hPool;
    HPAGE									hPage;
	RvSipPChargingVectorInfoListElemHandle	hElement;
	RvSipCommonListElemHandle				hRelative = NULL;
	RvUint32								value;
	RvInt32									offset = UNDEFINED;

	if(element.eListType == PARSER_INFO_LIST_TYPE_PDP)
	{
		hList = pcb->pchargingVectorHeader.pdpInfoList;
	}
	else if(element.eListType == PARSER_INFO_LIST_TYPE_DSL_BEARER)
	{
		hList = pcb->pchargingVectorHeader.dslBearerInfoList;
	}
	else
	{
		return RV_ERROR_BADPARAM;
	}

	if(hList == NULL)
	{
		if (SIP_PARSETYPE_P_CHARGING_VECTOR == pcb->eHeaderType)
		{
			/* The parser is used to parse PChargingVector header */
			RvSipPChargingVectorHeaderHandle hHeader = (RvSipPChargingVectorHeaderHandle)pcb->pSipObject;
			hPool = SipPChargingVectorHeaderGetPool(hHeader);
			hPage = SipPChargingVectorHeaderGetPage(hHeader);
		}
		else if(SIP_PARSETYPE_MSG == pcb->eHeaderType)
		{
			RvSipMsgHandle hMsg = (RvSipMsgHandle)pcb->pSipObject;
			hPool = SipMsgGetPool(hMsg);
			hPage = SipMsgGetPage(hMsg);
		}
		else if(SIP_PARSETYPE_URL_HEADERS_LIST == pcb->eHeaderType)
		{
			RvSipCommonListHandle hHeadersList =(RvSipCommonListHandle)pcb->pSipObject;
			hPool = RvSipCommonListGetPool(hHeadersList);
			hPage = RvSipCommonListGetPage(hHeadersList);
		}
		else
		{
			pcb->exit_flag = AG_REDUCTION_ERROR_CODE;
			pcb->eStat = RV_SIPPARSER_STOP_REDUCTION_ERROR;
			return RV_ERROR_BADPARAM;
		}

		status = RvSipCommonListConstructOnPage(hPool, hPage, (RV_LOG_Handle)pcb->pParserMgr->pLogMgr, &hList);
		if(RV_OK != status)
		{
			/* Error occurred in while constructing the List */
			pcb->exit_flag = AG_REDUCTION_ERROR_CODE;
			pcb->eStat = RV_SIPPARSER_STOP_REDUCTION_ERROR;
			return status;
		}
	}
	else
	{
		hPool = RvSipCommonListGetPool(hList);
		hPage = RvSipCommonListGetPage(hList);
	}
	
	status = RvSipPChargingVectorInfoListElementConstruct(pcb->pParserMgr->hMsgMgr, hPool, hPage, &hElement);
	if(RV_OK != status)
	{
		pcb->exit_flag = AG_REDUCTION_ERROR_CODE;
		pcb->eStat = RV_SIPPARSER_STOP_REDUCTION_ERROR;
		return status;
	}
	
	/* Change the token step into a number */
	status = ParserGetUINT32FromString(pcb->pParserMgr, element.nItem.buf,
								   element.nItem.len, &value);
	if (RV_OK != status)
	{
		if (RV_ERROR_ILLEGAL_SYNTAX == status)
		{
			pcb->exit_flag = AG_SYNTAX_ERROR_CODE;
			pcb->eStat = RV_SIPPARSER_STOP_ERROR;
		}
		else 
		{
			pcb->exit_flag = AG_REDUCTION_ERROR_CODE;
			pcb->eStat = RV_SIPPARSER_STOP_REDUCTION_ERROR;
		}
		RvLogError(pcb->pParserMgr->pLogSrc ,(pcb->pParserMgr->pLogSrc ,
					"ParserInfoListAddElement - error in ParserGetUINT32FromString"));
		return status;
	}

	RvSipPChargingVectorInfoListElementSetItem(hElement, value);

	RvSipPChargingVectorInfoListElementSetSig(hElement, element.bSig);

    status = ParserAllocFromObjPage(pcb->pParserMgr,
									&offset,
									hPool,
									hPage,
									element.cid.buf,
									element.cid.len);
    if (RV_OK != status)
    {
        RvLogError(pcb->pParserMgr->pLogSrc ,(pcb->pParserMgr->pLogSrc ,
          "ParserInfoListAddElement - error in ParserAllocFromObjPage"));
		pcb->exit_flag = AG_REDUCTION_ERROR_CODE;
		pcb->eStat = RV_SIPPARSER_STOP_REDUCTION_ERROR;
        return status;
    }

    /* Set the cid in the element */
    status = SipPChargingVectorInfoListElementSetStrCid(hElement, NULL, hPool, hPage, offset);
    if (RV_OK != status)
    {
        RvLogError(pcb->pParserMgr->pLogSrc,(pcb->pParserMgr->pLogSrc,
          "ParserInfoListAddElement - error in SipPChargingVectorInfoListElementSetStrCid"));
		pcb->exit_flag = AG_REDUCTION_ERROR_CODE;
		pcb->eStat = RV_SIPPARSER_STOP_REDUCTION_ERROR;
        return status;
    }

	if(RV_TRUE == element.isFlowID)
	{
		status = ParserAllocFromObjPage(pcb->pParserMgr,
										&offset,
										hPool,
										hPage,
										element.flowID.buf,
										element.flowID.len);
		if (RV_OK != status)
		{
			RvLogError(pcb->pParserMgr->pLogSrc ,(pcb->pParserMgr->pLogSrc ,
			  "ParserInfoListAddElement - error in ParserAllocFromObjPage"));
			pcb->exit_flag = AG_REDUCTION_ERROR_CODE;
			pcb->eStat = RV_SIPPARSER_STOP_REDUCTION_ERROR;
			return status;
		}

		/* Set the FlowID in the element */
		status = SipPChargingVectorInfoListElementSetStrFlowID(hElement, NULL, hPool, hPage, offset);
		if (RV_OK != status)
		{
			RvLogError(pcb->pParserMgr->pLogSrc,(pcb->pParserMgr->pLogSrc,
			  "ParserInfoListAddElement - error in SipPChargingVectorInfoListElementSetStrFlowID"));
			pcb->exit_flag = AG_REDUCTION_ERROR_CODE;
			pcb->eStat = RV_SIPPARSER_STOP_REDUCTION_ERROR;
			return status;
		}
	}
	
	status = RvSipCommonListPushElem(hList, 
									 RVSIP_P_CHARGING_VECTOR_INFO_LIST_ELEMENT_TYPE_INFO, 
									 (void*)hElement, 
									 RVSIP_LAST_ELEMENT,
									 NULL, 
									 &hRelative);
	if(RV_OK != status)
	{
		pcb->exit_flag = AG_REDUCTION_ERROR_CODE;
		pcb->eStat = RV_SIPPARSER_STOP_REDUCTION_ERROR;
	}

	if(element.eListType == PARSER_INFO_LIST_TYPE_PDP)
	{
		pcb->pchargingVectorHeader.pdpInfoList = hList;
	}
	else if(element.eListType == PARSER_INFO_LIST_TYPE_DSL_BEARER)
	{
		pcb->pchargingVectorHeader.dslBearerInfoList = hList;
	}
	
	return status;
}

/***************************************************************************
 * ParserPChargingFunctionAddressesListAddElement
 * ------------------------------------------------------------------------
 * General: add an element to the info list in PChargingFunctionAddressesHeader
 * Return Value: None.
 * ------------------------------------------------------------------------
 * Arguments:
 *    element - .
 *    element - .
 ***************************************************************************/
static RvStatus ParserPChargingFunctionAddressesListAddElement(
					IN void											*pcb_,
					IN ParserPChargingFunctionAddressesListElement	element)
{
	SipParser_pcb_type								*pcb  = (SipParser_pcb_type*)pcb_;
	RvSipCommonListHandle							hList = NULL;
    RvStatus										status; 
    HRPOOL											hPool;
    HPAGE											hPage;
	RvSipPChargingFunctionAddressesListElemHandle	hElement;
	RvSipCommonListElemHandle						hRelative = NULL;
	RvInt32											offset = UNDEFINED;

	if(element.eListType == PARSER_CHARGING_FUNCTION_ADDRESSES_LIST_TYPE_CCF)
	{
		hList = pcb->pchargingFunctionAddressesHeader.ccfList;
	}
	else if(element.eListType == PARSER_CHARGING_FUNCTION_ADDRESSES_LIST_TYPE_ECF)
	{
		hList = pcb->pchargingFunctionAddressesHeader.ecfList;
	}
	else
	{
		return RV_ERROR_BADPARAM;
	}

	if(hList == NULL)
	{
		if (SIP_PARSETYPE_P_CHARGING_FUNCTION_ADDRESSES == pcb->eHeaderType)
		{
			/* The parser is used to parse PChargingFunctionAddresses header */
			RvSipPChargingFunctionAddressesHeaderHandle hHeader = (RvSipPChargingFunctionAddressesHeaderHandle)pcb->pSipObject;
			hPool = SipPChargingFunctionAddressesHeaderGetPool(hHeader);
			hPage = SipPChargingFunctionAddressesHeaderGetPage(hHeader);
		}
		else if(SIP_PARSETYPE_MSG == pcb->eHeaderType)
		{
			RvSipMsgHandle hMsg = (RvSipMsgHandle)pcb->pSipObject;
			hPool = SipMsgGetPool(hMsg);
			hPage = SipMsgGetPage(hMsg);
		}
		else if(SIP_PARSETYPE_URL_HEADERS_LIST == pcb->eHeaderType)
		{
			RvSipCommonListHandle hHeadersList =(RvSipCommonListHandle)pcb->pSipObject;
			hPool = RvSipCommonListGetPool(hHeadersList);
			hPage = RvSipCommonListGetPage(hHeadersList);
		}
		else
		{
			pcb->exit_flag = AG_REDUCTION_ERROR_CODE;
			pcb->eStat = RV_SIPPARSER_STOP_REDUCTION_ERROR;
			return RV_ERROR_BADPARAM;
		}

		status = RvSipCommonListConstructOnPage(hPool, hPage, (RV_LOG_Handle)pcb->pParserMgr->pLogMgr, &hList);
		if(RV_OK != status)
		{
			/* Error occurred in while constructing the List */
			pcb->exit_flag = AG_REDUCTION_ERROR_CODE;
			pcb->eStat = RV_SIPPARSER_STOP_REDUCTION_ERROR;
			return status;
		}
	}
	else
	{
		hPool = RvSipCommonListGetPool(hList);
		hPage = RvSipCommonListGetPage(hList);
	}
	
	status = RvSipPChargingFunctionAddressesListElementConstruct(pcb->pParserMgr->hMsgMgr, hPool, hPage, &hElement);
	if(RV_OK != status)
	{
		pcb->exit_flag = AG_REDUCTION_ERROR_CODE;
		pcb->eStat = RV_SIPPARSER_STOP_REDUCTION_ERROR;
		return status;
	}
	
	status = ParserAllocFromObjPage(pcb->pParserMgr,
									&offset,
									hPool,
									hPage,
									element.value.buf,
									element.value.len);
    if (RV_OK != status)
    {
        RvLogError(pcb->pParserMgr->pLogSrc ,(pcb->pParserMgr->pLogSrc ,
          "ParserPChargingFunctionAddressesListAddElement - error in ParserAllocFromObjPage"));
		pcb->exit_flag = AG_REDUCTION_ERROR_CODE;
		pcb->eStat = RV_SIPPARSER_STOP_REDUCTION_ERROR;
        return status;
    }

    /* Set the value in the element */
    status = SipPChargingFunctionAddressesListElementSetStrValue(hElement, NULL, hPool, hPage, offset);
    if (RV_OK != status)
    {
        RvLogError(pcb->pParserMgr->pLogSrc,(pcb->pParserMgr->pLogSrc,
          "ParserPChargingFunctionAddressesListAddElement - error in SipPChargingFunctionAddressesListElementSetStrValue"));
		pcb->exit_flag = AG_REDUCTION_ERROR_CODE;
		pcb->eStat = RV_SIPPARSER_STOP_REDUCTION_ERROR;
        return status;
    }

	status = RvSipCommonListPushElem(hList, 
									 RVSIP_P_CHARGING_FUNCTION_ADDRESSES_LIST_ELEMENT_TYPE_CCF_ECF, 
									 (void*)hElement, 
									 RVSIP_LAST_ELEMENT,
									 NULL, 
									 &hRelative);
	if(RV_OK != status)
	{
		pcb->exit_flag = AG_REDUCTION_ERROR_CODE;
		pcb->eStat = RV_SIPPARSER_STOP_REDUCTION_ERROR;
	}

	if(element.eListType == PARSER_CHARGING_FUNCTION_ADDRESSES_LIST_TYPE_CCF)
	{
		pcb->pchargingFunctionAddressesHeader.ccfList = hList;
	}
	else if(element.eListType == PARSER_CHARGING_FUNCTION_ADDRESSES_LIST_TYPE_ECF)
	{
		pcb->pchargingFunctionAddressesHeader.ecfList = hList;
	}
	
	return status;
}
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
/***************************************************************************
 * ParserCurOSPSTag
 * ------------------------------------------------------------------------
 * General: returns a OSPSTag struct
 * Return Value: ParserOSPSTag structure.
 * ------------------------------------------------------------------------
 * Arguments:
 *    eOSPSTagType - enumeration of the OSPS Tag.
 ***************************************************************************/
static ParserOSPSTag ParserCurOSPSTag(ParserOSPSTagType eOSPSTagType)
{
    ParserOSPSTag tag;
    tag.type = eOSPSTagType;
	tag.other.buf = NULL;
	tag.other.len = 0;
    return tag;
}

#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 

#if defined RV_SIP_LIGHT
#include "ParserEngineSipLight2.h"
#elif defined RV_SIP_PRIMITIVES
#include "ParserEngineSipPrimitives2.h"
#elif defined RV_SIP_JSR32_SUPPORT
#include "ParserEngineJSR322.h"
#elif defined RV_SIP_IMS_HEADER_SUPPORT
#include "ParserEngineIMS2.h"
#elif defined RV_SIP_EXTENDED_HEADER_SUPPORT
#include "ParserEngineExtendedHeaders2.h"
#elif defined RV_SIP_TEL_URI_SUPPORT
#include "ParserEngineTel2.h"
#else
#include "ParserEngineClassic2.h"
#endif
