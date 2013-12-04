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
 *                              <OaCodecHash.h>
 *
 * The OaCodecHash.h file defines Hash of Codecs.
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

#ifndef _OA_CODECHASH_H
#define _OA_CODECHASH_H

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                            */
/*---------------------------------------------------------------------------*/
#include "AdsHash.h"
#include "AdsRlist.h"
#include "rvlog.h"
#include "rvsdp.h"
#include "rvmutex.h"
#include "RvOaTypes.h"

/*---------------------------------------------------------------------------*/
/*                              TYPE DEFINITIONS                             */
/*---------------------------------------------------------------------------*/
/******************************************************************************
 * OaCodecHash
 * ----------------------------------------------------------------------------
 * OaCodecHash represents the Codec Hash object.
 * The Codec Hash holds pointers to Media Descriptors that include codecs.
 * For more details see description of file.
 *****************************************************************************/
typedef struct {
    RvUint32           size;          /* Size of Hash Table in elements */
    HASH_HANDLE        hHash;         /* Hash Table */
    RvLogSource*       pLogSource;
    RvLogMgr*          pLogMgr;
    RvMutex*           pLock;
    RLIST_POOL_HANDLE  hHashElemPool; /* Pool of pointers to hash elements */
} OaCodecHash;

/******************************************************************************
 * OaCodecHashKey
 * ----------------------------------------------------------------------------
 * OaCodecHashKey contains parameters that describe codec.
 * OaCodecHashKey is used in two purposes:
 *      1. Key for hashing (while adding element to hash).
 *         Offset of hash bucket, into which the hash element should be added,
 *         is calculated using the codec parameters.
 *         This offset is called hash value and is calculated by HashFunction.
 *         Hash bucket is an entry in Hash Table, which contains elements with
 *         same hash value, but different keys.
 *      2. Criteria for comparison (while searching element in hash).
 *         The codec parameters are stored in the hash element and are used
 *         when hash bucket is searched for requested element.
 *         The hash bucket itself is found using offset, calculated by
 *         HashFunction.
 *****************************************************************************/
typedef struct {
    void*  pCodecOwner;
        /* Pointer to codec owner.
           It is used by Offer-Answer in order to store pointer to
           Session object or to module Manager. */

    const RvChar*  strMediaType;
        /* Type of media, which uses codec.
           This string appears in the m-line.
           For example, if the m-line is "m=video 50000 RTP/AVP 0 113",
           the media type is "video". */

    const RvChar*  strMediaFormat;
        /* Name of codec, if it is not used as a dynamic payload type.
           This name appears at the end of m-line.
           For example, if the m-line is "m=video 50000 RTP/AVP 0 113",
           the media format is "0". */

    const RvChar*  strCodecName;
        /* Name of codec, if it is used as a dynamic payload type.
           This name appears in RTPMAP attribute.
           For example, if the attribute is "a=rtpmap:113 DV/90000",
           the codec name is "DV" (digital video). */

    RvOaCodec eCodec;
        /* Codec, supported by the module derivation algorithm. */

    OaCodecHash*  pOaCodecHash;
        /* Pointer to the Codec Hash, where the element is stored,
           if the key is stored in hash element.
           Should not be used if the key is used for search. */

} OaCodecHashKey;

/*---------------------------------------------------------------------------*/
/*                            FUNCTION DEFINITIONS                           */
/*---------------------------------------------------------------------------*/
/******************************************************************************
 * OaCodecHashConstruct
 * ----------------------------------------------------------------------------
 * General:
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
                                OUT OaCodecHash* pCodecHash);

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
void RVCALLCONV OaCodecHashDestruct(IN OaCodecHash* pCodecHash);

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
 * Input:  pCodecHash         - Codec Hash.
 *         pSdpMsg            - SDP Message to be searched for codecs.
 *         pCodecOwner        - Codec Owner (Session or Module Manager).
 * Output: hListHashElements  - List of inserted hash elements.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaCodecHashInsertSdpMsgCodecs(
                                IN   OaCodecHash*  pCodecHash,
                                IN   RvSdpMsg*     pSdpMsg,
                                IN   void*         pCodecOwner,
                                OUT  RLIST_HANDLE  hListHashElements);

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
                                    IN RLIST_HANDLE  hHashElemList);

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
                                IN  OaCodecHash*           pCodecHash,
                                IN  OaCodecHashKey*        pCodecHashKey);

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
                                OUT OaCodecHashKey*        pCodecHashKey);

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef _OA_CODECHASH_H */


/*nl for linux */

