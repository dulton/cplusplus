/*
*******************************************************************************
*                                                                             *
* NOTICE:                                                                     *
* This document contains information that is confidential and proprietary to  *
* RADVision LTD.. No part of this publication may be reproduced in any form   *
* whatsoever without written prior approval by RADVision LTD..                *
*                                                                             *
* RADVision LTD. reserves the right to revise this publication and make       *
* changes without obligation to notify any person of such revisions or changes*
*******************************************************************************
*/


/******************************************************************************
 *                              <OaCodecHash.c>
 *
 * The OaCodecHash.c file implements Hash of Codecs, using Hash Table.
 * The Offer-Answer module uses it to store the Application capabilities.
 *
 * Codec Hash stores pointers to Media Descriptor objects.
 * Codec Hash enables to find a Media Descriptor object, which contains
 * specified codec, according to following search criteria:
 *          1. Type of media, contained in m-line of Media Descriptor.
 *          2. Name of codec, contained in m-line or in RTPMAP attribute of
 *             Media Descriptor.
 *             If media packet payload is of static type, or media packets are
 *             delivered over not RTP protocol, Format Name parameter of m-line
 *             is used as a name of codec.
 *             If payload is dynamic, Codec Name parameter of RTPMAP attribute
 *             is used as a name of codec.
 *             For more information about static and dynamic payloads (codecs)
 *             see RFC 2327 and RFC 1890.
 *          3. Clock rate, contained in RTPMAP attribute of Media Descriptor.
 *          4. Number of channels, contained in RTPMAP attribute.
 *          5. Other parameters defined as a suffix string of RTPMAP attribute.
 *
 * Example. Following Media Descriptions contains 3 codecs: 0, 8 and L8:
 *            m=audio 49170 RTP/AVP 0 8 102
 *            a=rtpmap:102 L8/22050/2
 *
 *          In order to find this Media Descriptor, as a descriptor that
 *          includes codec L8, following search criteria should be provided
 *              {audio, L8, 22050, 2, NULL}
 *
 *          In order to find this Media Descriptor, as a descriptor that
 *          includes format 8, following search criteria should be provided
 *              {audio, 8, 0, 0, NULL}
 *
 *
 * Codec Hash serves the Offer-Answer module in following way:
 * The Offer-Answer module enables the application to set Default Capabilities,
 * using RvOaSetCapabilities() API, or per session capabilities, using
 * RvOaSessionSetCapabilities() API. These capabilities are kept in format of
 * RADVISION SDP Stack Message objects.
 * The SDP Stack messages include Media Descriptor objects that in turn contain
 * codec descriptions.
 * When the Application set capabilities, the Media Descriptor of the capabili-
 * ties messages are inserted into the Hash in according to the codecs they
 * include.
 * In order to prepare ANSWER to be sent in context of specific SDP session,
 * the Offer-Answer Module should find subset of codecs, supported by both
 * local and remote session participants.
 * The remote codecs are taken from the received OFFER.
 * The local codecs are taken from the Capabilities messages, stored in
 * Offer-Answer Session object (per session capabilities) or in Offer-Answer
 * Manager object (default capabilities).
 * The intersection of these sets is found by search the Codec Hash for
 * Media Descriptors (in local capabilities), using codecs from OFFER as a
 * search criteria.
 *
 *
 *    Author                        Date
 *    ------                        ----
 *    Igor                          Aug 2006
 *****************************************************************************/

/*---------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                            */
/*---------------------------------------------------------------------------*/
#include "stdlib.h"

#include "OaCodecHash.h"
#include "rvsdp.h"
#include "rvansi.h"
#include "rvstrutils.h"
#include "OaUtils.h"

/*---------------------------------------------------------------------------*/
/*                              TYPE DEFINITIONS                             */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                          STATIC FUNCTION DECLARATIONS                     */
/*---------------------------------------------------------------------------*/
static RvUint HashFunction(void *pHashKey);

static RvBool HashCompareFunction(IN void *param1, IN void *param2);

static RvStatus RVCALLCONV HashInsertCodecByIndex(
                                IN  OaCodecHash*           pCodecHash ,
                                IN  const RvSdpMediaDescr* pMediaDescr,
                                IN  RvSize_t               idx,
                                IN  void*                  pCodecOwner,
                                IN  const char*            strMediaType,
                                OUT RLIST_HANDLE           hListHashElements);

/*---------------------------------------------------------------------------*/
/*                          FUNCTION IMPLEMENTATIONS                         */
/*---------------------------------------------------------------------------*/
/******************************************************************************
 * OaCodecHashConstruct
 * ----------------------------------------------------------------------------
 * General:
 *  Constructs Hash of Codecs object, while allocating memory for it.
 *  Constructs also pool of codec owners. Codec owner can be Offer-Answer
 *  Session object or Offer-Answer module Manager object. Session and Manager
 *  keep lists of hash elements that hold session or manager codecs.These lists
 *  are used for efficient removal owner's codecs from the hash.
 *
 * Arguments:
 * Input:  maxNumOfCodecs - hash capacity.
 *         maxNumOfOwners - number of owners that own hashed codecs.
 *         pLogMgr        - Log Manager, used by the Offer-Answer module.
 *         pLogSource     - Log Source, serving the Offer-Answer module.
 *         pLock          - Lock to be used for multi thread safety.
 * Output: pCodecHash     - Codec Hash object.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaCodecHashConstruct(
                                         IN  RvUint32     maxNumOfCodecs,
                                         IN  RvUint32     maxNumOfOwners,
                                         IN  RvLogMgr*    pLogMgr,
                                         IN  RvLogSource* pLogSource,
                                         IN  RvMutex*     pLock,
                                         OUT OaCodecHash* pCodecHash)
{
    memset(pCodecHash, 0, sizeof(OaCodecHash));

    /* Construct Hash Table */
    pCodecHash->hHash = HASH_Construct(HASH_DEFAULT_TABLE_SIZE(maxNumOfCodecs),
                                       maxNumOfCodecs, HashFunction,
                                       sizeof(RvSdpMediaDescr*),
                                       sizeof(OaCodecHashKey),
                                       pLogMgr, "OA_CodecHash");
    if (NULL == pCodecHash->hHash)
    {
        RvLogError(pLogSource ,(pLogSource,
            "OaCodecHashConstruct - failed to construct hash"));
        return RV_ERROR_OUTOFRESOURCES;
    }

    /* Construct pool of pointers to elements inserted into Hash Table */
    pCodecHash->hHashElemPool = RLIST_PoolListConstruct(
        maxNumOfCodecs, maxNumOfOwners/*Total Number of lists*/,
        sizeof(void*), pLogMgr, "OA_PointersToCodecHashPool");
    if (NULL == pCodecHash->hHashElemPool)
    {
        RvLogError(pLogSource ,(pLogSource,
            "OaCodecHashConstruct - failed to construct pool of hash elements"));
        return RV_ERROR_UNKNOWN;
    }


    pCodecHash->size  = HASH_DEFAULT_TABLE_SIZE(maxNumOfCodecs);
    pCodecHash->pLogSource = pLogSource;
    pCodecHash->pLogMgr = pLogMgr;
    pCodecHash->pLock = pLock;

    return RV_OK;
}

/******************************************************************************
 * OaCodecHashDestruct
 * ----------------------------------------------------------------------------
 * General:
 *  Destructs Hash of Codecs object, while freeing its memory.
 *
 * Arguments:
 * Input:  pCodecHash - hash capacity.
 * Output: none.
 *
 * Return Value: none.
 *****************************************************************************/
void RVCALLCONV OaCodecHashDestruct(IN OaCodecHash* pCodecHash)
{
    if(NULL != pCodecHash->hHash)
    {
        HASH_Destruct(pCodecHash->hHash);
    }
    if (NULL != pCodecHash->hHashElemPool)
    {
        RLIST_PoolListDestruct(pCodecHash->hHashElemPool);
    }
    memset(pCodecHash, 0, sizeof(OaCodecHash));
}

/******************************************************************************
 * OaCodecHashInsertSdpMsgCodecs
 * ----------------------------------------------------------------------------
 * General:
 *  Inserts pointers to Media Descriptors found in the SDP Message into
 *  the Codec Hash.
 *  Each Media Descriptor is inserted number of times exact as a number of
 *  codecs it contains. Number of codecs is found as a number of Media Formats
 *  that appear in the Descriptor's m-line.
 *  The place for the Media Descriptor in hash is calculated in accordance to
 *  codec, found in the Descriptor.
 *  This function also fills the provided list of hash elements with pointers
 *  to the newly inserted elements
 *
 * Arguments:
 * Input:  pCodecHash        - Codec Hash.
 *         pSdpMsg           - SDP Message to be searched for codecs.
 *         pCodecOwner       - Codec Owner (Session or Module Manager).
 * Output: hListHashElements - List of inserted hash elements.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaCodecHashInsertSdpMsgCodecs(
                                IN  OaCodecHash*  pCodecHash,
                                IN  RvSdpMsg*     pSdpMsg,
                                IN  void*         pCodecOwner,
                                OUT RLIST_HANDLE  hListHashElements)
{
    const RvSdpMediaDescr* pMediaDescr;
    RvSdpListIter          mdIterator; /* Media Descriptor iterator */
    RvSize_t               numOfFormats, i;
    const char*            strMediaType;
    RvStatus               rv, crv;

    memset(&mdIterator, 0, sizeof(RvSdpListIter));
    pMediaDescr = rvSdpMsgGetFirstMediaDescr(pSdpMsg, &mdIterator);
    
    crv = RvMutexLock(pCodecHash->pLock, pCodecHash->pLogMgr);
    if (RV_OK != crv)
    {
        rv = OA_CRV2RV(crv);
        RvLogError(pCodecHash->pLogSource,(pCodecHash->pLogSource,
            "OaCodecHashInsertSdpMsgCodecs - failed to lock(crv=%d,rv=%d:%s)",
            RvErrorGetCode(crv), rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    while (NULL != pMediaDescr)
    {
        strMediaType = rvSdpMediaDescrGetMediaTypeStr(pMediaDescr);

        numOfFormats = rvSdpMediaDescrGetNumOfFormats(pMediaDescr);
        for (i=0; i<numOfFormats; i++)
        {
            rv = HashInsertCodecByIndex(pCodecHash, pMediaDescr, i,
                                        pCodecOwner, strMediaType,
                                        hListHashElements);
            if (RV_OK != rv)
            {
                RvMutexUnlock(pCodecHash->pLock, pCodecHash->pLogMgr);
                RvLogError(pCodecHash->pLogSource,(pCodecHash->pLogSource,
                    "OaCodecHashInsertSdpMsgCodecs - failed to insert element into hash(rv=%d:%s)",
                    rv, OaUtilsConvertEnum2StrStatus(rv)));
                return rv;
            }
        } /* ENDOF:   for (i=0; i<numOfFormats; i++) */

        pMediaDescr = rvSdpMsgGetNextMediaDescr(&mdIterator);
    } /*  ENDOF:   while (NULL != pMediaDescr)*/

    RvMutexUnlock(pCodecHash->pLock, pCodecHash->pLogMgr);

    return RV_OK;
}

/******************************************************************************
 * OaCodecHashRemoveElements
 * ----------------------------------------------------------------------------
 * General:
 *  Goes through list of hash elements and remove them from the Codec Hash.
 *
 * Arguments:
 * Input:  pCodecHash    - Codec Hash.
 *         hHashElemList - list of elements to be removed from hash.
 * Output: none.
 *
 * Return Value: none.
 *****************************************************************************/
void RVCALLCONV OaCodecHashRemoveElements(
                                    IN OaCodecHash*  pCodecHash,
                                    IN RLIST_HANDLE  hHashElemList)
{
    RLIST_ITEM_HANDLE  listElement = NULL;
    RLIST_ITEM_HANDLE  nextListElement = NULL;
    void*              hashElement;
    RvStatus           crv;

    /* Lock pool, list and hash by same lock ;-) */
    crv = RvMutexLock(pCodecHash->pLock, pCodecHash->pLogMgr);
    if (RV_OK != crv)
    {
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
        RvStatus rv = OA_CRV2RV(crv);
        RvLogError(pCodecHash->pLogSource,(pCodecHash->pLogSource,
            "OaCodecHashRemoveElements - failed to lock(crv=%d,rv=%d:%s)",
            RvErrorGetCode(crv), rv, OaUtilsConvertEnum2StrStatus(rv)));
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
        return;
    }

    /* Remove elements contained in list from hash */
    RLIST_GetHead(pCodecHash->hHashElemPool, hHashElemList, &listElement);
    while (NULL != listElement)
    {
        hashElement = *((void**)listElement);
        HASH_RemoveElement(pCodecHash->hHash, hashElement);

        RLIST_GetNext(pCodecHash->hHashElemPool, hHashElemList, listElement,
                      &nextListElement);

        /* Remove element reference from list */
        RLIST_Remove(pCodecHash->hHashElemPool, hHashElemList, listElement);

        listElement = nextListElement;
    }

    RvMutexUnlock(pCodecHash->pLock, pCodecHash->pLogMgr);
}

/******************************************************************************
 * OaCodecHashFindCodec
 * ----------------------------------------------------------------------------
 * General:
 *  Searches Codec Hash for Media Descriptor that contains codec with
 *  parameters united in hash key.
 *
 * Arguments:
 * Input:  pCodecHash    - Codec Hash.
 *         pCodecHashKey - key for search.
 * Output: none.
 *
 * Return Value: found Media Descriptor or NULL.
 *****************************************************************************/
RvSdpMediaDescr* RVCALLCONV OaCodecHashFindCodec(
                                IN  OaCodecHash*       pCodecHash,
                                IN  OaCodecHashKey*    pCodecHashKey)
{
    RvBool           bWasFound;
    RvStatus         rv, crv;
    RvSdpMediaDescr* pMediaDescr;
    void*            pHashElement;


    crv = RvMutexLock(pCodecHash->pLock, pCodecHash->pLogMgr);
    if (RV_OK != crv)
    {
        rv = OA_CRV2RV(crv);
        RvLogError(pCodecHash->pLogSource,(pCodecHash->pLogSource,
            "OaCodecHashFindCodec - failed to lock(crv=%d,rv=%d:%s)",
            RvErrorGetCode(crv), rv, OaUtilsConvertEnum2StrStatus(rv)));
        return NULL;
    }

    bWasFound = HASH_FindElement(pCodecHash->hHash, (void*)pCodecHashKey,
                                 HashCompareFunction, (void**)&pHashElement);
    if (RV_FALSE == bWasFound)
    {
        RvMutexUnlock(pCodecHash->pLock, pCodecHash->pLogMgr);
        if (NULL == pCodecHashKey->strCodecName)
        {
            RvLogDebug(pCodecHash->pLogSource,(pCodecHash->pLogSource,
                "OaCodecHashFindCodec - media format %s was not found",
                pCodecHashKey->strMediaFormat));
        }
        else
        {
            RvLogDebug(pCodecHash->pLogSource,(pCodecHash->pLogSource,
                "OaCodecHashFindCodec - codec %s was not found",
                pCodecHashKey->strCodecName));
        }
        return NULL;
    }
    rv = HASH_GetUserElement(pCodecHash->hHash,pHashElement,(void*)&pMediaDescr);
    if(rv != RV_OK)
    {
        RvMutexUnlock(pCodecHash->pLock, pCodecHash->pLogMgr);
        RvLogError(pCodecHash->pLogSource,(pCodecHash->pLogSource,
            "OaCodecHashFindCodec - failed to get user data (pMediaDescr) for hash element %p(rv=%d:%s)",
            pHashElement, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return NULL;
    }

    RvMutexUnlock(pCodecHash->pLock, pCodecHash->pLogMgr);
    return pMediaDescr;
}

/******************************************************************************
 * OaCodecHashInitializeKey
 * ----------------------------------------------------------------------------
 * General:
 *  Fills the Codec Hash key with codec data.
 *  The codec data is taken from provided Media Descriptor.
 *  The codec data describes the codec (or media format) with provided 0-based
 *  index.
 *
 * Arguments:
 * Input:  pCodecHash    - Codec Hash.
 *         pMediaDescr   - Media Descriptor.
 *         idx           - index of media format in the Media Descriptor.
 *         pCodecOwner   - codec owner.
 *         strMediaType  - type of media utilizing codec.
 * Output: pCodecHashKey - initialized key.
 *
 * Return Value: none.
 *****************************************************************************/
void RVCALLCONV OaCodecHashInitializeKey(
                                IN  OaCodecHash*           pCodecHash,
                                IN  const RvSdpMediaDescr* pMediaDescr,
                                IN  RvSize_t               idx,
                                IN  void*                  pCodecOwner,
                                IN  const char*            strMediaType,
                                OUT OaCodecHashKey*        pCodecHashKey)
{
    const RvSdpRtpMap*  pRtpMap;
    RvSdpListIter       rmIterator; /* RTPMAP iterator */
    const char*         strMediaFormat;
    int                 payloadType;
    RvInt32             mediaFormat;

    /* Prepare key based on Media Format or Codec */
    memset(pCodecHashKey, 0 , sizeof(OaCodecHashKey));
    pCodecHashKey->pCodecOwner  = pCodecOwner;
    pCodecHashKey->pOaCodecHash = pCodecHash;
    pCodecHashKey->strMediaType = strMediaType;

    /* If Media Format is represented by integer in range [96-127],
    that means, that the Media Format is a dynamic payload type,
    codec of which is described by the correspondent RTPMAP attribute.
    In this case the codec itself (value of RTPMAP) should be inserted
    into the hash. Otherwise - the Media Format should be inserted.
    See RFC1890 for more details. */

    strMediaFormat = rvSdpMediaDescrGetFormat(pMediaDescr, idx);
    mediaFormat = atoi(strMediaFormat);
    if (96 <= mediaFormat  &&  mediaFormat <= 127)
    /* Use codec name for hashing */
    {
        pRtpMap = rvSdpMediaDescrGetFirstRtpMap(
            (RvSdpMediaDescr*)pMediaDescr, &rmIterator);
        while (NULL != pRtpMap)
        {
            payloadType = rvSdpRtpMapGetPayload(pRtpMap);
            if (payloadType == mediaFormat)
            {
                pCodecHashKey->strCodecName = rvSdpRtpMapGetEncodingName(pRtpMap);
                /* Find the codec, supported by the module derivation algorithm */
                pCodecHashKey->eCodec = OaUtilsConvertStr2EnumCodec(pCodecHashKey->strCodecName);
                break;
            }
            pRtpMap = rvSdpMediaDescrGetNextRtpMap(&rmIterator);
        }
    }
    else
    /* Use media format for hashing */
    {
        pCodecHashKey->strMediaFormat = strMediaFormat;
        /* Find the codec, supported by the module derivation algorithm */
        pCodecHashKey->eCodec = OaUtilsConvertStr2EnumCodec(strMediaFormat);
    }
}


/*---------------------------------------------------------------------------*/
/*                       STATIC FUNCTION IMPLEMENTATIONS                     */
/*---------------------------------------------------------------------------*/
/******************************************************************************
 * HashFunction
 * ----------------------------------------------------------------------------
 * General:
 *  Calculates offset of the element in hash table, based on the hash key.
 *
 * Arguments:
 * Input:  pHashKey - key.
 * Output: none.
 *
 * Return Value: calculated offset.
 *****************************************************************************/
static RvUint HashFunction(void *pHashKey)
{
    OaCodecHashKey* pKey = (OaCodecHashKey*)pHashKey;
    RvUint      s = 0;
    RvInt32     mod = pKey->pOaCodecHash->size;
    RvInt32     ch;
    const char* pCh;
    RvChar      buffer[64];
    RvChar      deltaLowUpperCase = 'a'-'A';
    static const int base = 1 << (sizeof(char)*8);

    /* Use Owner value for hashing */
    RvSprintf(buffer, "%p", pKey->pCodecOwner);
    pCh = buffer;
    while ('\0' != *pCh)
    {
        s = (s*base + *pCh)%mod;
        pCh++;
    }

    /* Use Media Type for hashing */
    pCh = pKey->strMediaType;
    while ('\0' != *pCh)
    {
        /* Use lowercase while calculating hash value */
        if (*pCh >= 'A')
        {
            ch = *pCh - deltaLowUpperCase;
        }
        else
        {
            ch = *pCh;
        }
        s = (s*base+ch)%mod;
        pCh++;
    }

    /* If Supported Codec is set, use it and don't use Format or Codec */
    if (RVOA_CODEC_UNKNOWN != pKey->eCodec)
    {
        s = (s*base + pKey->eCodec)%mod;
        return s;
    }

    /* If Media Format is set, use it and don't use Codec */
    if (NULL != pKey->strMediaFormat)
    {
        pCh = pKey->strMediaFormat;
        while ('\0' != *pCh)
        {
            s = (s*base+(*pCh))%mod;
            pCh++;
        }
        return s;
    }

    /* At this point no Media Format was found. Use Codec. */
    /* Use Codec Name for hashing */
    if (NULL != pKey->strCodecName)
    {
        pCh = pKey->strCodecName;
        while ('\0' != *pCh)
        {
            /* Don't use dots, while hashing */
            while (*pCh=='.')
            {
                pCh++;
            }
            if ('\0' == *pCh)
            {
                break;
            }
            /* Use lowercase while calculating hash value */
            if (*pCh > 'Z')
            {
                ch = *pCh - deltaLowUpperCase;
            }
            else
            {
                ch = *pCh;
            }
            s = (s*base+ch)%mod;
            pCh++;
        }
    }
    return s;
}

/******************************************************************************
 * HashCompareFunction
 * ----------------------------------------------------------------------------
 * General:
 *  Compares parameters of two hash elements.
 *  The Codec Hash uses key as a parameter for comparing.
 *  Each hash element holds it's key.
 *
 * Arguments:
 * Input:  param1 - parameters of the first element to be compared.
 *         param2 - parameters of the other element to be compared.
 * Output: none.
 *
 * Return Value: RV_TRUE - if the elements are equal, RV_FALSE - otherwise.
 *****************************************************************************/
static RvBool HashCompareFunction(IN void *param1, IN void *param2)
{
    OaCodecHashKey* pKeyToBeFound = (OaCodecHashKey*)param1;
    OaCodecHashKey* pKeyInHash    = (OaCodecHashKey*)param2;
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    RvLogSource*    pLogSrc = pKeyInHash->pOaCodecHash->pLogSource;
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

    /* Check Owner */
    if (pKeyToBeFound->pCodecOwner != pKeyInHash->pCodecOwner)
    {
        RvLogDebug(pLogSrc,(pLogSrc,
            "HashCompareFunction - failed: owner mismatch(requested %p, found %p)",
            pKeyToBeFound->pCodecOwner, pKeyInHash->pCodecOwner));
        return RV_FALSE;
    }

    /* Check Media Type */
    if (NULL!=pKeyToBeFound->strMediaType && NULL!=pKeyInHash->strMediaType)
    {
        if (0 != RvStrcasecmp(pKeyToBeFound->strMediaType,pKeyInHash->strMediaType))
        {
            RvLogDebug(pLogSrc,(pLogSrc,
                "HashCompareFunction - failed: Media Type mismatch(requested %s, found %s)",
                pKeyToBeFound->strMediaType, pKeyInHash->strMediaType));
            return RV_FALSE;
        }
    }
    else
    {
        RvLogDebug(pLogSrc,(pLogSrc,
            "HashCompareFunction - failed: Media Type mismatch"));
        return RV_FALSE;
    }

    /* Check codec if supported */
    if (pKeyToBeFound->eCodec != RVOA_CODEC_UNKNOWN &&
        pKeyToBeFound->eCodec == pKeyInHash->eCodec)
    {
        RvLogDebug(pLogSrc,(pLogSrc,
            "HashCompareFunction - succeed (same codec %s, supported by derivation algorithm)",
            OaUtilsConvertEnum2StrCodec(pKeyToBeFound->eCodec)));
        return RV_TRUE;
    }

    /* If Media Format is requested to be found, don't check Codec. */
    if (NULL!=pKeyToBeFound->strMediaFormat && NULL!=pKeyInHash->strMediaFormat)
    {
        if (0 != RvStrcasecmp(pKeyToBeFound->strMediaFormat,pKeyInHash->strMediaFormat))
        {
            RvLogDebug(pLogSrc,(pLogSrc,
                "HashCompareFunction - failed: Media Format mismatch(requested %s, found %s)",
                pKeyToBeFound->strMediaFormat, pKeyInHash->strMediaFormat));
            return RV_FALSE;
        }
        RvLogDebug(pLogSrc,(pLogSrc,
            "HashCompareFunction - succeed (same format name %s)",
            pKeyToBeFound->strMediaFormat));
        return RV_TRUE;
    }
    else
    if (NULL!=pKeyToBeFound->strMediaFormat || NULL!=pKeyInHash->strMediaFormat)
    {
        RvLogDebug(pLogSrc,(pLogSrc,
            "HashCompareFunction - failed: Media Format mismatch"));
        return RV_FALSE;
    }

    /* At this point Media Format was not found, check the codec (Codec name)*/
    if (NULL!=pKeyToBeFound->strCodecName && NULL!=pKeyInHash->strCodecName)
    {
        if (RV_FALSE == OaUtilsCompareCodecNames(pKeyToBeFound->strCodecName,
                                                 pKeyInHash->strCodecName))
        {
            RvLogDebug(pLogSrc,(pLogSrc,
                "HashCompareFunction - failed: Codec Name mismatch(requested %s, found %s)",
                pKeyToBeFound->strCodecName, pKeyInHash->strCodecName));
            return RV_FALSE;
        }
    }
    else
    {
        RvLogDebug(pLogSrc,(pLogSrc,
            "HashCompareFunction - failed: Codec Name mismatch"));
        return RV_FALSE;
    }

    return RV_TRUE;
}

/******************************************************************************
 * HashInsertCodecByIndex
 * ----------------------------------------------------------------------------
 * General:
 *  Inserts pointers to Media Descriptor into the Codec Hash.
 *  The provided Media Descriptor is inserted number of times exact as a number
 *  of codecs it contains. Number of codecs is found as a number of
 *  Media Formats that appear in the Descriptor's m-line.
 *  The place for the Media Descriptor in hash is calculated in accordance to
 *  codec, found in the Descriptor.
 *  This function also fills the provided list of hash elements with pointers
 *  to the newly inserted elements
 *
 * Arguments:
 * Input:  pCodecHash         - Codec Hash.
 *         pMediaDescr        - Media Descriptor.
 *         idx                - index of codec in Media Descriptor.
 *         pCodecOwner        - Codec Owner (Session or Module Manager).
 *         strMediaType       - Type of media in string form, e.g. "audio".
 * Output: hListHashElements  - List of inserted hash elements.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
static RvStatus RVCALLCONV HashInsertCodecByIndex(
                                IN  OaCodecHash*           pCodecHash ,
                                IN  const RvSdpMediaDescr* pMediaDescr,
                                IN  RvSize_t               idx,
                                IN  void*                  pCodecOwner,
                                IN  const char*            strMediaType,
                                OUT RLIST_HANDLE           hListHashElements)
{
    RvStatus        rv;
    OaCodecHashKey  codecHashKey;
    RvBool          bWasInserted;
    void*           hashElement = NULL;

    OaCodecHashInitializeKey(pCodecHash, pMediaDescr, idx, pCodecOwner,
                             strMediaType,&codecHashKey);

    /* Insert key into the hash */
    rv = HASH_InsertElement(pCodecHash->hHash,
                            (void*)&codecHashKey,
                            (void*)&pMediaDescr,
                            RV_FALSE /*bLookupBeforeInsert*/,
                            &bWasInserted,
                            &hashElement,
                            HashCompareFunction);
    if (RV_OK != rv)
    {
        RvLogError(pCodecHash->pLogSource,(pCodecHash->pLogSource,
            "HashInsertCodecByIndex - failed to insert element into hash(rv=%d)",
            rv));
        return rv;
    }
    if (RV_FALSE==bWasInserted)
    {
        RvLogError(pCodecHash->pLogSource,(pCodecHash->pLogSource,
            "HashInsertCodecByIndex - element was not inserted"));
        return RV_ERROR_UNKNOWN;
    }

    /* Insert new hash element into the list of elements */
    if (NULL != hashElement)
    {
        RLIST_ITEM_HANDLE hListElement;

        /* Add element */
        rv = RLIST_InsertTail(pCodecHash->hHashElemPool,
                              hListHashElements, &hListElement);
        if (RV_OK != rv)
        {
            RvLogError(pCodecHash->pLogSource,(pCodecHash->pLogSource,
                "HashInsertCodecByIndex - failed to add hash elements to list(rv=%d:%s)",
                rv, OaUtilsConvertEnum2StrStatus(rv)));
            HASH_RemoveElement(pCodecHash->hHash, hashElement);
            return rv;
        }
        *((void**)hListElement) = hashElement;
    }

    return RV_OK;
}

/*nl for linux */

