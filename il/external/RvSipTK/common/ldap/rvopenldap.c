/* rvopenldap.c - Interface functions for OpenLDAP services */
/************************************************************************
      Copyright (c) 2001,2002 RADVISION Inc. and RADVISION Ltd.
************************************************************************
NOTICE:
This document contains information that is confidential and proprietary
to RADVISION Inc. and RADVISION Ltd.. No part of this document may be
reproduced in any form whatsoever without written prior approval by
RADVISION Inc. or RADVISION Ltd..

RADVISION Inc. and RADVISION Ltd. reserve the right to revise this
publication and make changes without obligation to notify any person of
such revisions or changes.
***********************************************************************/

#include "rvldapcb.h"

#if (RV_LDAP_TYPE == RV_LDAP_OPENLDAP)
#include "rvopenldap.h"
#include <string.h>

#if (RV_OS_TYPE == RV_OS_TYPE_WIN32)
#include <winsock2.h>

#else
#if (RV_OS_TYPE == RV_OS_TYPE_SOLARIS)
#include <lber.h>
#endif

#include <sys/time.h>
#endif

#include <ldap.h>


#define RV_LDAP_MAX_ATTRIBUTES 20


static RvLdapErrorType rvLdapErrorCode2RvLdapErrorType(int ldapErrorCode)
{
    switch(ldapErrorCode)
    {
    case(LDAP_SUCCESS):                  return RvLdapErrorTypeSuccess;
    case(LDAP_OPERATIONS_ERROR):         return RvLdapErrorTypeOperationsError;
    case(LDAP_PROTOCOL_ERROR):           return RvLdapErrorTypeProtocolError;
    case(LDAP_TIMELIMIT_EXCEEDED):       return RvLdapErrorTypeTimeLimitExceeded;
    case(LDAP_SIZELIMIT_EXCEEDED):       return RvLdapErrorTypeSizeLimitExceeded;
    case(LDAP_COMPARE_FALSE):            return RvLdapErrorTypeCompareFalse;
    case(LDAP_COMPARE_TRUE):             return RvLdapErrorTypeCompareTrue;
    case(LDAP_AUTH_METHOD_NOT_SUPPORTED):return RvLdapErrorTypeAuthMethodNotSupported;
    case(LDAP_STRONG_AUTH_REQUIRED):     return RvLdapErrorTypeStrongAuthRequired;
    case(LDAP_PARTIAL_RESULTS):          return RvLdapErrorTypePartialResults;
    case(LDAP_NO_SUCH_ATTRIBUTE):        return RvLdapErrorTypeNoSuchAttribute;
    case(LDAP_UNDEFINED_TYPE):           return RvLdapErrorTypeUndefinedType;
    case(LDAP_INAPPROPRIATE_MATCHING):   return RvLdapErrorTypeInappropriateMatching;
    case(LDAP_CONSTRAINT_VIOLATION):     return RvLdapErrorTypeConstraintViolation;
    case(LDAP_TYPE_OR_VALUE_EXISTS):     return RvLdapErrorTypeTypeOrValueExists;
    case(LDAP_INVALID_SYNTAX):           return RvLdapErrorTypeInvalidSyntax;
    case(LDAP_NO_SUCH_OBJECT):           return RvLdapErrorTypeNoSuchObject;
    case(LDAP_ALIAS_PROBLEM):            return RvLdapErrorTypeAliasProblem;
    case(LDAP_INVALID_DN_SYNTAX):        return RvLdapErrorTypeInvalidDnSyntax;
    case(LDAP_IS_LEAF):                  return RvLdapErrorTypeIsLeaf;
    case(LDAP_ALIAS_DEREF_PROBLEM):      return RvLdapErrorTypeAliasDerefProblem;
    case(LDAP_INAPPROPRIATE_AUTH):       return RvLdapErrorTypeInappropriate_auth;
    case(LDAP_INVALID_CREDENTIALS):      return RvLdapErrorTypeInvalidCredentials;
    case(LDAP_INSUFFICIENT_ACCESS):      return RvLdapErrorTypeInsufficientAccess;
    case(LDAP_BUSY):                     return RvLdapErrorTypeBusy;
    case(LDAP_UNAVAILABLE):              return RvLdapErrorTypeUnavailable;
    case(LDAP_UNWILLING_TO_PERFORM):     return RvLdapErrorTypeUnwillingToPerform;
    case(LDAP_LOOP_DETECT):              return RvLdapErrorTypeLoopDetect;
    case(LDAP_NAMING_VIOLATION):         return RvLdapErrorTypeNamingViolation;
    case(LDAP_OBJECT_CLASS_VIOLATION):   return RvLdapErrorTypeObjectClassViolation;
    case(LDAP_NOT_ALLOWED_ON_NONLEAF):   return RvLdapErrorTypeNotAllowedOnNonleaf;
    case(LDAP_NOT_ALLOWED_ON_RDN):       return RvLdapErrorTypeNotAllowedOnRdn;
    case(LDAP_ALREADY_EXISTS):           return RvLdapErrorTypeAlreadyExists;
    case(LDAP_NO_OBJECT_CLASS_MODS):     return RvLdapErrorTypeNoObjectClassMods;
    case(LDAP_OTHER):                    return RvLdapErrorTypeOther;
    case(LDAP_SERVER_DOWN):              return RvLdapErrorTypeServerDown;
    case(LDAP_LOCAL_ERROR):              return RvLdapErrorTypeLocalError;
    case(LDAP_ENCODING_ERROR):           return RvLdapErrorTypeEncodingError;
    case(LDAP_DECODING_ERROR):           return RvLdapErrorTypeDecodingError;
    case(LDAP_TIMEOUT):                  return RvLdapErrorTypeTimeout;
    case(LDAP_AUTH_UNKNOWN):             return RvLdapErrorTypeAuthUnknown;
    case(LDAP_FILTER_ERROR):             return RvLdapErrorTypeFilterError;
    case(LDAP_PARAM_ERROR):              return RvLdapErrorTypeParamError;
    case(LDAP_NO_MEMORY):                return RvLdapErrorTypeNoMemory;
    default:                             return RvLdapErrorTypeOther;
    }
}


static int rvLdapErrorType2LdapErrorCode(RvLdapErrorType rvLdapErrorType)
{
    switch(rvLdapErrorType)
    {
    case(RvLdapErrorTypeSuccess):               return LDAP_SUCCESS;
    case(RvLdapErrorTypeOperationsError):       return LDAP_OPERATIONS_ERROR;
    case(RvLdapErrorTypeProtocolError):         return LDAP_PROTOCOL_ERROR;
    case(RvLdapErrorTypeTimeLimitExceeded):     return LDAP_TIMELIMIT_EXCEEDED;
    case(RvLdapErrorTypeSizeLimitExceeded):     return LDAP_SIZELIMIT_EXCEEDED;
    case(RvLdapErrorTypeCompareFalse):          return LDAP_COMPARE_FALSE;
    case(RvLdapErrorTypeCompareTrue):           return LDAP_COMPARE_TRUE;
    case(RvLdapErrorTypeAuthMethodNotSupported):return LDAP_AUTH_METHOD_NOT_SUPPORTED;
    case(RvLdapErrorTypeStrongAuthRequired):    return LDAP_STRONG_AUTH_REQUIRED;
    case(RvLdapErrorTypePartialResults):        return LDAP_PARTIAL_RESULTS;
    case(RvLdapErrorTypeNoSuchAttribute):       return LDAP_NO_SUCH_ATTRIBUTE;
    case(RvLdapErrorTypeUndefinedType):         return LDAP_UNDEFINED_TYPE;
    case(RvLdapErrorTypeInappropriateMatching): return LDAP_INAPPROPRIATE_MATCHING;
    case(RvLdapErrorTypeConstraintViolation):   return LDAP_CONSTRAINT_VIOLATION;
    case(RvLdapErrorTypeTypeOrValueExists):     return LDAP_TYPE_OR_VALUE_EXISTS;
    case(RvLdapErrorTypeInvalidSyntax):         return LDAP_INVALID_SYNTAX;
    case(RvLdapErrorTypeNoSuchObject):          return LDAP_NO_SUCH_OBJECT;
    case(RvLdapErrorTypeAliasProblem):          return LDAP_ALIAS_PROBLEM;
    case(RvLdapErrorTypeInvalidDnSyntax):       return LDAP_INVALID_DN_SYNTAX;
    case(RvLdapErrorTypeIsLeaf):                return LDAP_IS_LEAF;
    case(RvLdapErrorTypeAliasDerefProblem):     return LDAP_ALIAS_DEREF_PROBLEM;
    case(RvLdapErrorTypeInappropriate_auth):    return LDAP_INAPPROPRIATE_AUTH;
    case(RvLdapErrorTypeInvalidCredentials):    return LDAP_INVALID_CREDENTIALS;
    case(RvLdapErrorTypeInsufficientAccess):    return LDAP_INSUFFICIENT_ACCESS;
    case(RvLdapErrorTypeBusy):                  return LDAP_BUSY;
    case(RvLdapErrorTypeUnavailable):           return LDAP_UNAVAILABLE;
    case(RvLdapErrorTypeUnwillingToPerform):    return LDAP_UNWILLING_TO_PERFORM;
    case(RvLdapErrorTypeLoopDetect):            return LDAP_LOOP_DETECT;
    case(RvLdapErrorTypeNamingViolation):       return LDAP_NAMING_VIOLATION;
    case(RvLdapErrorTypeObjectClassViolation):  return LDAP_OBJECT_CLASS_VIOLATION;
    case(RvLdapErrorTypeNotAllowedOnNonleaf):   return LDAP_NOT_ALLOWED_ON_NONLEAF;
    case(RvLdapErrorTypeNotAllowedOnRdn):       return LDAP_NOT_ALLOWED_ON_RDN;
    case(RvLdapErrorTypeAlreadyExists):         return LDAP_ALREADY_EXISTS;
    case(RvLdapErrorTypeNoObjectClassMods):     return LDAP_NO_OBJECT_CLASS_MODS;
    case(RvLdapErrorTypeOther):                 return LDAP_OTHER;
    case(RvLdapErrorTypeServerDown):            return LDAP_SERVER_DOWN;
    case(RvLdapErrorTypeLocalError):            return LDAP_LOCAL_ERROR;
    case(RvLdapErrorTypeEncodingError):         return LDAP_ENCODING_ERROR;
    case(RvLdapErrorTypeDecodingError):         return LDAP_DECODING_ERROR;
    case(RvLdapErrorTypeTimeout):               return LDAP_TIMEOUT;
    case(RvLdapErrorTypeAuthUnknown):           return LDAP_AUTH_UNKNOWN;
    case(RvLdapErrorTypeFilterError):           return LDAP_FILTER_ERROR;
    case(RvLdapErrorTypeParamError):            return LDAP_PARAM_ERROR;
    case(RvLdapErrorTypeNoMemory):              return LDAP_NO_MEMORY;
    default:                                    return LDAP_OTHER;
    }
}


RVAPI RvStatus RvLdapInitialize(
    IN RvChar * strServerAddress,
    OUT HLDAPCb * pHLdapCb)
{
    LDAP* oLdap;

#if (RV_OS_TYPE != RV_OS_TYPE_SOLARIS)
    if (strlen(strServerAddress) > 4 && strncmp(strServerAddress, "ldap", 4) == 0)
    {
        int result = ldap_initialize(&oLdap, strServerAddress);
        if (result != LDAP_SUCCESS)
            oLdap = NULL;
    }
    else
#endif
    {
        oLdap = ldap_init(strServerAddress, LDAP_PORT);
    }
    
    if (oLdap == NULL)
        return RV_ERROR_UNKNOWN;
    
    *pHLdapCb = (HLDAPCb)oLdap;
    
    return RV_OK;
}


RVAPI RvStatus RvLdapSetOption(
    IN HLDAPCb hLdapCb,
    IN RvLdapOption ldapOption,
    IN void* ldapData)
{
    int result, option;

    switch(ldapOption)
    {
    case ldapOptApiInfo:
        option = LDAP_OPT_API_INFO;
    	break;
    case ldapOptProtocolVersion:
        option = LDAP_OPT_PROTOCOL_VERSION;
        break;
    default:
        return RV_ERROR_BADPARAM;
    }

    result = ldap_set_option((LDAP*)hLdapCb, option, ldapData);
    if (result < 0)
        return RV_ERROR_UNKNOWN;

    return RV_OK;
}


RVAPI RvStatus RvLdapBind(
    IN HLDAPCb hLdapCb,
    IN RvLdapConnectionMethod eConnectionType,
    IN const RvChar * strName,
    IN const RvChar * strPassword,
    OUT HLDAPRequestCb * pHRequestCb)
{
    int method, request;

    switch (eConnectionType)
    {
    case (ldapConnectionMethodKRBV41):
        method = LDAP_AUTH_KRBV41;
        break;
    case (ldapConnectionMethodKRBV42):
        method = LDAP_AUTH_KRBV42;
        break;
    case (ldapConnectionMethodSimple):
    default:
        method = LDAP_AUTH_SIMPLE;
        break;
    }
    request = ldap_bind((LDAP*)hLdapCb, (char*)strName, (char*)strPassword, method);
    if (request < 0)
        return RV_ERROR_UNKNOWN;
    *pHRequestCb = (HLDAPRequestCb)request;
    return RV_OK;
}


RVAPI RvStatus RvLdapUnbind(
    IN HLDAPCb hLdapCb)
{
    ldap_unbind((LDAP*)hLdapCb);
    return RV_OK;
}


RVAPI RvStatus RvLdapGetAllResults(
    IN HLDAPCb hLdapCb,
    INOUT HLDAPRequestCb * pHRequestCb,
    OUT RvLdapErrorType * errorCode,
    OUT RvInt32 * resultCount,
    OUT HLDAPMessageCb * pHResultCb)
{
    int msgid;
    struct timeval timeout;
    LDAPMessage *result;
    int resType;

    memset(&timeout, 0, sizeof(timeout));

    if (*pHRequestCb != NULL)
        msgid = (int)*pHRequestCb;
    else
        msgid = LDAP_RES_ANY;

    /* (Eli) On Solaris a timeout parameter doesn't work. Only NULL works.
     * Needs investigation.
     */
    resType = ldap_result((LDAP*)hLdapCb, msgid, RV_TRUE, &timeout, &result);

    if (resType <= 0)
        return RV_ERROR_UNKNOWN;

    *errorCode = rvLdapErrorCode2RvLdapErrorType(ldap_result2error((LDAP*)hLdapCb, result, 0));

    *pHResultCb = (HLDAPMessageCb)result;
    *resultCount = ldap_count_messages((LDAP*)hLdapCb, result);

    if (*pHRequestCb == NULL)
    {
        *pHRequestCb = (HLDAPRequestCb)ldap_msgid(result);
    }

    return RV_OK;
}


RVAPI RvStatus RvLdapGetNextResult(
    IN HLDAPCb hLdapCb,
    IN HLDAPMessageCb hResultCb,
    INOUT HLDAPEntryCb * pHEntryCb)
{
    if (*pHEntryCb == NULL)
    {
        *pHEntryCb = (HLDAPEntryCb)ldap_first_message((LDAP*)hLdapCb, (LDAPMessage*)hResultCb);
    }
    else
    {
        *pHEntryCb = (HLDAPEntryCb)ldap_next_message((LDAP*)hLdapCb, (LDAPMessage*)*pHEntryCb);
    }
    return RV_OK;
}


RVAPI RvStatus RvLdapResultGetNextAtribute(
    IN HLDAPCb hLdapCb,
    IN HLDAPEntryCb hEntryCb,
    INOUT HLDAPAttributeCb * pHAtributeCb,
    INOUT HLDAPValuesCb * pHValuesCb,
    OUT RvChar * strAttribute,
    INOUT RvInt32 * lenAttribute,
    OUT RvInt32 * numValues)
{
    RvChar * pStrAttrib;
    struct berval ** values;
    RvStatus res = RV_OK;

    if (*pHAtributeCb == NULL)
    {
        BerElement * ber;

        /* get the first attribute string and a new attribute pointer */
        pStrAttrib = ldap_first_attribute((LDAP*)hLdapCb, (LDAPMessage*)hEntryCb, &ber);
        if (pStrAttrib == NULL)
        {
            return RV_ERROR_NOT_FOUND; 
        }
        if (strlen(pStrAttrib) < (RvSize_t)*lenAttribute)
        {
            strcpy(strAttribute, pStrAttrib);
        }
        else
            res = RV_ERROR_INSUFFICIENT_BUFFER;
        *lenAttribute = strlen(pStrAttrib);
        *pHAtributeCb = (HLDAPAttributeCb)ber;
    }
    else
    {
        /* get the next attribute string */
        pStrAttrib = ldap_next_attribute((LDAP*)hLdapCb, (LDAPMessage*)hEntryCb, (BerElement*)*pHAtributeCb);
        if (pStrAttrib == NULL)
        {
            res = RV_ERROR_NOT_FOUND;
        }
        else
        {
            if (strlen(pStrAttrib) < (RvSize_t)*lenAttribute)
            {
                strcpy(strAttribute, pStrAttrib);
            }
            else
                res = RV_ERROR_INSUFFICIENT_BUFFER;
            *lenAttribute = strlen(pStrAttrib);
        }
    }

    if (*pHValuesCb != NULL)
    {
        ldap_value_free_len((struct berval**)*pHValuesCb);
    }

    if (pStrAttrib != NULL)
    {
        /* get the value of the attribute */
        values = ldap_get_values_len((LDAP*)hLdapCb, (LDAPMessage*)hEntryCb, pStrAttrib);
        *pHValuesCb = (HLDAPValuesCb)values;
        *numValues = ldap_count_values_len(values);
    }
    else
    {
        *pHValuesCb = (HLDAPValuesCb)NULL;
        *numValues = 0;
    }
    return res;
}


RVAPI RvStatus RvLdapGetAtributeValue(
    IN HLDAPCb hLdapCb,
    IN HLDAPAttributeCb hAtributeCb,
    IN HLDAPValuesCb hValuesCb,
    IN RvInt32 index, 
    INOUT RvInt32 * length,
    OUT RvChar ** strValue)
{
    struct berval ** values = (struct berval**)hValuesCb;

    RV_UNUSED_ARG(hLdapCb);
    RV_UNUSED_ARG(hAtributeCb);

    *length = (RvInt32)values[index]->bv_len;
    *strValue = values[index]->bv_val;
    return RV_OK;
}


RVAPI RvStatus RvLdapFreeResult(
    IN HLDAPCb hLdapCb,
    IN HLDAPMessageCb hResultCb,
    IN HLDAPAttributeCb hAtributeCb,
    IN HLDAPValuesCb hValuesCb)
{
    RV_UNUSED_ARG(hLdapCb);

    if (hValuesCb != NULL)
    {
        ldap_value_free_len((struct berval**)hValuesCb);
    }
    if (hAtributeCb != NULL)
    {
        ber_free((BerElement*)hAtributeCb, 0);
    }
    if (hResultCb != NULL)
    {
        ldap_msgfree((LDAPMessage*)hResultCb);
    }
    return RV_OK;
}


RVAPI RvStatus RvLdapEndRequest(
    IN HLDAPCb hLdapCb,
    IN HLDAPRequestCb hRequestCb)
{
    return ldap_abandon((LDAP*)hLdapCb, (int)hRequestCb);
}


RVAPI RvStatus RvLdapStartSearch(
    IN HLDAPCb hLdapCb,
    IN const RvChar * strBase, 
    IN RvLdapScope eScope, 
    IN const RvChar * strFilter, 
    IN RvChar ** strAttributesToReturn,
    OUT HLDAPRequestCb * pHRequestCb)
{
    int scope = 0, res;

    switch (eScope)
    {
    case ldapScopeBase:
        scope = LDAP_SCOPE_BASE;
        break;
    case ldapScopeOneLevel:
        scope = LDAP_SCOPE_ONELEVEL;
        break;
    case ldapScopeSubTree:
        scope = LDAP_SCOPE_SUBTREE;
        break;
    default:
        return RV_ERROR_BADPARAM;
    }

    res = ldap_search((LDAP*)hLdapCb, (char*)strBase, scope, (char*)strFilter, strAttributesToReturn, 0);

    if (res >= 0)
    {
        *pHRequestCb = (HLDAPRequestCb)res;
        return RV_OK;
    }

    return RV_ERROR_UNKNOWN;
}


RVAPI RvStatus RvLdapDelete(
    IN HLDAPCb hLdapCb,
    IN const RvChar * strBase, 
    OUT HLDAPRequestCb * pHRequestCb)
{
    int res;

    res = ldap_delete((LDAP*)hLdapCb, (char*)strBase);

    if (res >= 0)
    {
        *pHRequestCb = (HLDAPRequestCb)res;
        return RV_OK;
    }

    return RV_ERROR_UNKNOWN;
}


RVAPI RvStatus RvLdapAdd(
    IN HLDAPCb hLdapCb,
    IN const RvChar * strBase, 
    IN LDAPAttrList* attrList,
    IN RvInt32 nAttrList,
    OUT HLDAPRequestCb * pHRequestCb)
{
    int i, res;
    LDAPMod  ldapAttrs[RV_LDAP_MAX_ATTRIBUTES];
    LDAPMod* ldapAttrList[RV_LDAP_MAX_ATTRIBUTES];

    for (i=0; i < nAttrList && i < RV_LDAP_MAX_ATTRIBUTES; ++i)
    {
        ldapAttrs[i].mod_op = 0;
        ldapAttrs[i].mod_type = attrList[i].attrName;
        ldapAttrs[i].mod_values = attrList[i].attrValues;

        ldapAttrList[i] = &ldapAttrs[i];
    }

    ldapAttrList[i] = NULL;

    res = ldap_add((LDAP*)hLdapCb, (char*)strBase, ldapAttrList);

    if (res >= 0)
    {
        *pHRequestCb = (HLDAPRequestCb)res;
        return RV_OK;
    }

    return RV_ERROR_UNKNOWN;
}


RVAPI RvStatus RvLdapModify(
    IN HLDAPCb hLdapCb,
    IN const RvChar * strBase,
    IN LDAPAttrList* attrList,
    IN RvInt32 nAttrList,
    OUT HLDAPRequestCb * pHRequestCb)
{
    int i, res;
    LDAPMod  ldapAttrs[RV_LDAP_MAX_ATTRIBUTES];
    LDAPMod* ldapAttrList[RV_LDAP_MAX_ATTRIBUTES];

    for (i=0; i < nAttrList && i < RV_LDAP_MAX_ATTRIBUTES; ++i)
    {
        ldapAttrs[i].mod_op = LDAP_MOD_REPLACE;
        ldapAttrs[i].mod_type = attrList[i].attrName;
        ldapAttrs[i].mod_values = attrList[i].attrValues;

        ldapAttrList[i] = &ldapAttrs[i];
    }

    ldapAttrList[i] = NULL;

    res = ldap_modify((LDAP*)hLdapCb, (char*)strBase, ldapAttrList);

    if (res >= 0)
    {
        *pHRequestCb = (HLDAPRequestCb)res;
        return RV_OK;
    }

    return RV_ERROR_UNKNOWN;
}


RVAPI RvStatus RvLdapGetFd(
    IN HLDAPCb hLdapCb,
    OUT void * pFileDesc)
{
    ldap_get_option((LDAP*)hLdapCb, LDAP_OPT_DESC, pFileDesc);

    return RV_OK;
}


RVAPI RvStatus RvLdapReceivedEvent(
    IN HLDAPCb hLdapCb,
    OUT RvBool *bPoll)
{
    RV_UNUSED_ARG(hLdapCb);
    *bPoll = RV_TRUE;
    return RV_OK;
}


RVAPI RvLdapErrorType RvLdapGetErrorNumber(
    IN HLDAPCb hLdapCb)
{
    int errorCode = LDAP_SUCCESS;

    ldap_get_option((LDAP*)hLdapCb, LDAP_OPT_ERROR_NUMBER, &errorCode);

    return rvLdapErrorCode2RvLdapErrorType(errorCode);
}


RVAPI const RvChar * RvLdapGetErrorString(
    IN RvLdapErrorType errorCode)
{
    return (const RvChar *)ldap_err2string(rvLdapErrorType2LdapErrorCode(errorCode));
}

#else
int prevent_warning_of_ranlib_has_no_symbols_rvopenldap=0;
#endif /* RV_LDAP_TYPE == RV_LDAP_OPENLDAP */
