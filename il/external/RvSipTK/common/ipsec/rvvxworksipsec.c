#include "rvtypes.h"
#include "rvaddress.h"
#include "rvalloc.h"
#include "rvlog.h"
#include "rvlock.h"

/* #define RV_IMS_IPSEC_DEBUG_MESSAGES 1 */

#if (RV_OS_TYPE == RV_OS_TYPE_VXWORKS)

#if (RV_IMS_IPSEC_ENABLED == RV_YES)

#include "rvstdio.h"

#if (RV_OS_VERSION == RV_OS_VXWORKS_2_2_PNE)

STATUS spdSetESPXform (char* pConfStr);
STATUS spdSetProp (char* pConfStr);
STATUS spdSetPropAttrib (char* pConfStr);
STATUS spdSetSA (char* pConfStr);
STATUS spdAddTransport (char* pConfStr);
STATUS spdDeletePolicy (char* pConfStr);

STATUS mkmDeleteSA (char* cptr_mkm_sa);
STATUS mkmAddTransport (char* cptr_mkm_sa);
STATUS mkmSetInboundESP (char* cptr_value_string);
STATUS mkmSetOutboundESP (char* cptr_value_string);
STATUS mkmCommitAll (void);
STATUS mkmDeleteSA (char* cptr_mkm_sa);


#elif (RV_OS_VERSION == RV_OS_VXWORKS_3_1)
#include "wrn/spd/spd.h"
#include "wrn/ipsec/ipsec.h"
#include "wrn/ipsec/ipsec_mkm.h"
#else
#error IPSec on VxWorks is supported for Tornado 2.2 with PNE and Workbench only
#endif

#include "rvimsipsecprivate.h"

const RvChar* gsESPTransforms[4][4];
const RvChar* gsEncrAlgNames[4];
const RvChar* gsAuthAlgNames[4];

#define RV_TRANSFORM_NAME_NULL_MD5  "rv_esp_null_md5"
#define RV_TRANSFORM_NAME_3DES_MD5  "rv_esp_3des_md5"
#define RV_TRANSFORM_NAME_AES_MD5   "rv_esp_aes_md5"

#define RV_TRANSFORM_NAME_NULL_SHA  "rv_esp_null_sha"
#define RV_TRANSFORM_NAME_3DES_SHA  "rv_esp_3des_sha"
#define RV_TRANSFORM_NAME_AES_SHA   "rv_esp_aes_sha"


RvStatus rvAdIpsecInit()
{
    STATUS status;
    RvChar txt[100];
    const RvChar* xformName;
    RvInt c1,c2;

#define SET_ESP_XFORM(ENCR_TXT,AUTH_TXT,NAME,ENCR,AUTH) \
    { \
    sprintf(txt,"%s,%s,%s",NAME,ENCR_TXT,AUTH_TXT);\
    status = spdSetESPXform(txt);\
    if (status)\
        return RV_ERROR_UNKNOWN;\
    gsESPTransforms[ENCR][AUTH] = NAME;\
}

    bzero((void*)gsESPTransforms,sizeof(gsESPTransforms));

    /* we create all possible combinations of Transforms already at the initialize stage 
       there are only 6 possible combiantions, beacuse there are three encryption algorithms
       and two for authentications.
    */

    SET_ESP_XFORM("ESPNULL","MD5",RV_TRANSFORM_NAME_NULL_MD5,RvImsEncrNull,RvImsAuthMd5);
    SET_ESP_XFORM("ESP3DES","MD5",RV_TRANSFORM_NAME_3DES_MD5,RvImsEncr3Des,RvImsAuthMd5);
    SET_ESP_XFORM("ESPAES-CBC","MD5",RV_TRANSFORM_NAME_AES_MD5,RvImsEncrAes,RvImsAuthMd5);

    SET_ESP_XFORM("ESPNULL","SHA",RV_TRANSFORM_NAME_NULL_SHA,RvImsEncrNull,RvImsAuthSha1);
    SET_ESP_XFORM("ESP3DES","SHA",RV_TRANSFORM_NAME_3DES_SHA,RvImsEncr3Des,RvImsAuthSha1);
    SET_ESP_XFORM("ESPAES-CBC","SHA",RV_TRANSFORM_NAME_AES_SHA,RvImsEncrAes,RvImsAuthSha1);

    for (c1 = RvImsEncrNull; c1 <= RvImsEncrAes; c1++)
    {
        for (c2 = RvImsAuthMd5; c2 <= RvImsAuthSha1; c2++)
        {
            xformName = gsESPTransforms[c1][c2];
            sprintf(txt,"prop_%s,%s",xformName,xformName);
            status = spdSetProp(txt);
            if (status)
            {
                /* we don't have logging yet, thus print it out */
                printf("Failed with %d in spdSetProp for %s\n",status,txt);
            }
    
            sprintf(txt,"prop_%s,ANTIREPLAY,DISABLED,ENCAP,TRANSPORT,HARDLIFETIME,4294967295,HARDLIFESIZE,4294967295",
                xformName);
            status = spdSetPropAttrib(txt);
            if (status)
            {
                /* we don't have logging yet, thus print it out */
                printf("Failed with %d in spdSetPropAttrib for %s\n",status,txt);
            }
        }
    }
        
    bzero((void*)gsEncrAlgNames,sizeof(gsEncrAlgNames));
    gsEncrAlgNames[RvImsEncrNull] = "ESPNULL";
    gsEncrAlgNames[RvImsEncr3Des] = "ESP3DES";
    gsEncrAlgNames[RvImsEncrAes] = "ESPAES-CBC"; 

    bzero((void*)gsAuthAlgNames,sizeof(gsAuthAlgNames));
    gsAuthAlgNames[RvImsAuthMd5] = "MD5";
    gsAuthAlgNames[RvImsAuthSha1] = "SHA";

    return RV_OK;
}

RvStatus rvAdIpsecEnd()
{
    return RV_OK;
}


/********************************************************************************************
 * rvAdSetOSSpecific
 *
 * Set OS specific data for SA/SP manipulations
 *
 * INPUT:
 *    sad       - contains data needed for SA/SP creation;
 *  logMgr      - handle of the log manager for this instance
 *
 * RETURNS:
 *  RV_OK on success, other values on failure
 ********************************************************************************************/
RvStatus rvAdSetOSSpecific(
    IN RvImsSAData *sad,
    IN RvLogMgr *logMgr)
{
    RV_UNUSED_ARG(sad);
    RV_UNUSED_ARG(logMgr);
    return RV_OK;
}


/********************************************************************************************
 * encodeKey
 *
 * Encode the encryption/authentication key within the provided string as HEX numbers 
 *
 * INPUT:
 *    k - the key to encode
 * OUTPUT
 *    pp - at the end will point to the position next to the last used within the 'txt'
 *    txt - the string where encoding will be performed.
 *
 * RETURNS:
 *  None
 ********************************************************************************************/
static void encodeKey(RvChar** pp, RvImsCryptoKey* k, char* txt)
{
    RvInt up,cnt;
    RvUint8 *v, c;
    RvChar *p;

    p = *pp;
    sprintf(p,",%s,",txt);
    p += strlen(p);
    up = k->iKeyBits>>3;
    for (cnt = 0, v = k->iKeyValue; cnt < up; cnt++,v++)
    {
        /* take upper and lower 4 bits of each byte and print it as HEX number */
        c = (RvChar)(*v>>4);
        *p++ = (RvUint8)((c<10)?'0'+c:'A'+c-10);
        c = (RvChar)(0x0F&*v);
        *p++ = (RvUint8)((c<10)?'0'+c:'A'+c-10);
    }
    *pp = p;
}


/********************************************************************************************
 * rvAdIPSecAddSASP
 *
 * OS specific function to add 'sapdNum' of security associations and security policies
 *
 * INPUT:
 *    sad       - array of SP data
 * sapdNum       - number of elements in 'sapd'
 *  logMgr      - handle of the log manager for this instance
 *
 * RETURNS:
 *  RV_OK on success, other values on failure
 ********************************************************************************************/
RvStatus rvAdIPSecAddSASP(
        IN RvImsSAData *sad,
        IN int sapdNum,
        IN RvLogMgr *logMgr)
{
    STATUS status;
    RvStatus sts;
    RvChar txt[1024], *p;
    const RvChar *xformName;
    static RvUint sCnt = 0;
    RvInt cnt;
    RvChar peerAddrTxt[256], lclAddrTxt[256], saCfg[256];

    RV_UNUSED_ARG(sapdNum);

    #define FUNCTION_NAME "rvAdIPSecAddSASP"
    IMSIPSEC_LOG_ENTER;

    RvAddressGetString(&sad->iPeerAddress,sizeof(peerAddrTxt),peerAddrTxt);
    RvAddressGetString(&sad->iLclAddress,sizeof(lclAddrTxt),lclAddrTxt);

    sts = RV_OK;

    /* find the name of the transform (as the combination of encryption and authentication 
       algorithms */
    xformName = gsESPTransforms[sad->iEncrAlg][sad->iAuthAlg];
    if (xformName == NULL)
    {
        IMSIPSEC_LOG_ERROR_2("Trying to use unsupported algorithm combination (%d,%d)",
            sad->iEncrAlg,sad->iAuthAlg);
        goto failure;
    }

    for (cnt = 0; cnt < 2; cnt++)
    {
        ++sCnt;

        /* create the SA for the needed transform */
        sprintf(txt,"sa_%d_%s,prop_%s,1",sCnt,xformName,xformName);
        status = spdSetSA(txt);
        if (status)
        {
            IMSIPSEC_LOG_ERROR_2("Failed with %d in spdSetSA for %s",status,txt);
            goto failure;
        }
        sad->iIpSecSpecific.iVWState[cnt] = RvImsVWSACreated;

        if (cnt == 0)
            sprintf(saCfg,"%d/%d/%d,%s,%s,OUT",(sad->iProto==RvImsProtoUdp)?17:6,
            sad->iPeerPrtClnt,sad->iLclPrtSrv,peerAddrTxt,lclAddrTxt);
        else
            sprintf(saCfg,"%d/%d/%d,%s,%s,OUT",(sad->iProto==RvImsProtoUdp)?17:6,
                sad->iPeerPrtSrv,sad->iLclPrtClnt,peerAddrTxt,lclAddrTxt);
        
        sprintf(txt,"%s,PACKET,MANUAL,sa_%d_%s",saCfg,sCnt,xformName);
#if RV_IMS_IPSEC_DEBUG_MESSAGES                
        printf("spdAddTransport %s\n",txt);
#endif
        status = spdAddTransport(txt);
        if (status)
        {
            IMSIPSEC_LOG_ERROR_2("Failed with %d in spdAddTransport for %s",status,txt);
            goto failure;
        }

        sprintf(txt,"%d,%s,%s",sCnt,saCfg,lclAddrTxt);
#if RV_IMS_IPSEC_DEBUG_MESSAGES        
        printf("mkmAddTransport %s\n",txt);
#endif
        status = mkmAddTransport(txt);
        if (status)
        {
            IMSIPSEC_LOG_ERROR_2("Failed with %d in mkmAddTransport for %s",status,txt);
            goto failure;
        }
        sad->iIpSecSpecific.iVWState[cnt] = RvImsVWMkmAdded;

        if (cnt == 0)
            sprintf(txt,"%d,%d,%s",sCnt,sad->iSpiOutSrv,gsEncrAlgNames[sad->iEncrAlg]);
        else
            sprintf(txt,"%d,%d,%s",sCnt,sad->iSpiOutClnt,gsEncrAlgNames[sad->iEncrAlg]);

        p = txt+strlen(txt);
        if (sad->iEncrAlg != RvImsEncrNull)
            encodeKey(&p,&sad->iEncrKey,"ENCKEY");
        strcpy(p,",AUTHALG,");
        p += 9;
        strcpy(p,gsAuthAlgNames[sad->iAuthAlg]);
        p += strlen(p);
        encodeKey(&p,&sad->iAuthKey,"AUTHKEY");
        *p++ = 0;

#if RV_IMS_IPSEC_DEBUG_MESSAGES        
        printf("mkmSetOutboundESP %s\n",txt);
#endif
        status = mkmSetOutboundESP(txt);
        if (status)
        {
            txt[5] = 0;
            IMSIPSEC_LOG_ERROR_2("Failed with %d in mkmSetOutboundESP for %s",status,txt);
            goto failure;
        }

        /* now have to replace the value of SPI */
        {
            RvChar ttt[15], *pp1, *pp2;
            RvInt oldL, newL;
            if (cnt == 0)
                sprintf(ttt,"%d",sad->iSpiInSrv);
            else
                sprintf(ttt,"%d",sad->iSpiInClnt);

            newL = strlen(ttt);
            pp1 = strchr(txt,',')+1;
            pp2 = strchr(pp1,',');
            oldL = pp2 - pp1;
            if (oldL > newL)
                memcpy(pp2-(oldL-newL),pp2,p-pp2);
            else if (newL > oldL)
                memmove(pp2+(newL-oldL),pp2,p-pp2);
            memcpy(pp1,ttt,newL);

            pp1 = strstr(txt,",ENCKEY,");
            if (pp1)
                strncpy(pp1+1,"DEC",3);
        }

#if RV_IMS_IPSEC_DEBUG_MESSAGES        
        printf("mkmSetInboundESP %s\n",txt);
#endif
        status = mkmSetInboundESP(txt);
        if (status)
        {
            txt[5] = 0;
            IMSIPSEC_LOG_ERROR_2("Failed with %d in mkmSetInboundESP for %s",status,txt);
            goto failure;
        }
    }
    
    status = mkmCommitAll();
    if (status)
    {
        IMSIPSEC_LOG_ERROR_1("Failed with %d in mkmCommitAll",status);
        goto failure;
    }

    goto good_end;

failure:

    sts = RV_ERROR_UNKNOWN;
    rvAdIPSecDeleteSASP(sad,sapdNum,logMgr);
    
good_end:

    IMSIPSEC_LOG_LEAVE_1("Completed with %s",(sts == RV_OK) ? "success" : "failure");
    return sts;
    #undef FUNCTION_NAME
}

/********************************************************************************************
 * rvAdIPSecDeleteSASP
 *
 * OS specific function to delete 'sapdNum' of security associations and security policies
 *
 * INPUT:
 *    sad       - array of SP data
 * sapdNum       - number of elements in 'sapd'
 *  logMgr      - handle of the log manager for this instance
 *
 * RETURNS:
 *  RV_OK on success, other values on failure
 ********************************************************************************************/
RvStatus rvAdIPSecDeleteSASP(
        IN RvImsSAData *sad,
        IN int sapdNum,
        IN RvLogMgr *logMgr)
{
    STATUS status;
    RvStatus sts;
    const RvChar *xformName;
    RvInt cnt;
    RvChar peerAddrTxt[256], lclAddrTxt[256], txt[1024];

    RV_UNUSED_ARG(sapdNum);

    #define FUNCTION_NAME "rvAdIPSecDeleteSASP"
    IMSIPSEC_LOG_ENTER;

    RvAddressGetString(&sad->iPeerAddress,sizeof(peerAddrTxt),peerAddrTxt);
    RvAddressGetString(&sad->iLclAddress,sizeof(lclAddrTxt),lclAddrTxt);

    sts = RV_OK;

    xformName = gsESPTransforms[sad->iEncrAlg][sad->iAuthAlg];
    if (xformName == NULL)
    {
        IMSIPSEC_LOG_ERROR_2("Trying to use unsupported algorithm combination (%d,%d)",
            sad->iEncrAlg,sad->iAuthAlg);
        return RV_ERROR_UNKNOWN;
    }

    for (cnt = 0; cnt < 2; cnt++)
    {
        if (sad->iIpSecSpecific.iVWState[cnt] < RvImsVWSACreated)
            continue;

        if (cnt == 0)
            sprintf(txt,"%d/%d/%d,%s,%s,OUT,MIRRORED",(sad->iProto==RvImsProtoUdp)?17:6,
                                sad->iPeerPrtSrv,sad->iLclPrtClnt,peerAddrTxt,lclAddrTxt);
        else
            sprintf(txt,"%d/%d/%d,%s,%s,OUT,MIRRORED",(sad->iProto==RvImsProtoUdp)?17:6,
                                sad->iPeerPrtClnt,sad->iLclPrtSrv,peerAddrTxt,lclAddrTxt);

        switch (sad->iIpSecSpecific.iVWState[cnt])
        {
        case RvImsVWMkmAdded:           
            status = mkmDeleteSA(txt);
            if (status)
            {
                IMSIPSEC_LOG_ERROR_2("Failed with %d in mkmDeleteSA for %s",status,txt);
                sts = RV_ERROR_UNKNOWN;    
                continue;
            }
        case RvImsVWSACreated:
            status = spdDeletePolicy(txt);
            if (status)
            {
                IMSIPSEC_LOG_ERROR_2("Failed with %d in spdDeletePolicy for %s",status,txt);
                sts = RV_ERROR_UNKNOWN;    
                continue;
            }
        default:
            break;
        }
    }
    
    IMSIPSEC_LOG_LEAVE_1("Completed with %s",(sts == RV_OK) ? "success" : "failure");
    memset(&sad->iIpSecSpecific,0,sizeof(RvImsIpSecSpecific));
    return sts;
    #undef FUNCTION_NAME
}

/********************************************************************************************
 * rvAdIPSecCleanAll
 *
 * Removes all SA/SP set on this host.
 * Not supported on VxWorks.
 *
 * INPUT:
 *  logMgr      - handle of the log manager for this instance
 *
 * RETURNS:
 *  RV_OK on success, other values on failure
 ********************************************************************************************/
RvStatus rvAdIPSecCleanAll(
    IN RvLogMgr *logMgr)
{
    RV_UNUSED_ARG(logMgr);
    return RV_OK;
}

#endif /*(RV_IMS_IPSEC_ENABLED == RV_YES) */

#else
int prevent_warning_of_ranlib_has_no_symbols_rvvxworksipsec=0;
#endif /*(RV_OS_TYPE == RV_OS_TYPE_VXWORKS)*/
