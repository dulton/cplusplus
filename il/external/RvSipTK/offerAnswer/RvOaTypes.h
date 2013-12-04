/*
*******************************************************************************
*                                                                             *
* NOTICE:                                                                     *
* This document contains information that is confidential and proprietary to  *
* RADVision LTD. No part of this publication may be reproduced in any form   *
* whatsoever without written prior approval by RADVision LTD.                *
*                                                                             *
* RADVision LTD. reserves the right to revise this publication and make       *
* changes without obligation to notify any person of such revisions or changes*
*******************************************************************************
*/


/******************************************************************************
 *                              <RvOaTypes.h>
 *
 *
 *    Author                        Date
 *    ------                        --------
 *    Igor							Aug 2006
 *****************************************************************************/

/*@****************************************************************************
 * Module: Offer-Answer
 * ----------------------------------------------------------------------------
 * Title: Offer-Answer Type Definitions
 *
 * The RvOaTypes.h file contains all type definitions and callback function
 * type definitions for the Offer-Answer Module.
 * This includes handle types, callback functions and other definitions for:
 *  1.Offer-Answer Manager
 *  2.Offer-Answer Session
 *  3.Offer-Answer Stream
 *
 * The Offer-Answer (OA) Module implements the Offer-Answer Model of negotiation
 * of media capabilities by SIP endpoints, described in RFC 3264.
 * The Offer-Answer Module implements the abstractions and their behavior, as
 * it is defined by RFC 3264.
 ***************************************************************************@*/

#ifndef _RV_OA_TYPES_H
#define _RV_OA_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*                          INCLUDE HEADER FILES                             */
/*---------------------------------------------------------------------------*/
#include "RV_ADS_DEF.h"
#include "rvsdp.h"

/*---------------------------------------------------------------------------*/
/*                       MANAGER TYPE DEFINITIONS                            */
/*---------------------------------------------------------------------------*/

/*@****************************************************************************
 * RvOaSessionMgrHandle (Offer-Answer Manager)
 * ----------------------------------------------------------------------------
 * The declaration of a handle to the instance of a SDP Session Manager.
 * The handle of the SDP Session Manager object should be provided while
 * calling the Offer-Answer Manager functions, such as RvOaCreateSession().
 ***************************************************************************@*/
RV_DECLARE_HANDLE(RvOaMgrHandle);

/*@****************************************************************************
 * RvOaSessionHandle (Offer-Answer Manager)
 * ----------------------------------------------------------------------------
 * The declaration of a handle to the instance of a SDP Session.
 * The handle of the SDP session object should be provided while calling the
 * Offer-Answer Session and Offer-Answer Stream functions, such as
 * RvOaSessionTerminate().
 ***************************************************************************@*/
RV_DECLARE_HANDLE(RvOaSessionHandle);

/*@****************************************************************************
 * RvOaAppSessionHandle (Offer-Answer Manager)
 * ----------------------------------------------------------------------------
 * The declaration of an application handle to the instance of a SDP Session.
 * The application should use the application handle as a reference
 * to the SDP session object of the application. This handle is
 * set into the Offer-Answer session upon construction, and it is
 * provided by the Offer-Answer Module in callback functions.
 ***************************************************************************@*/
RV_DECLARE_HANDLE(RvOaAppSessionHandle);

/*@****************************************************************************
 * RvOaStreamHandle (Offer-Answer Manager)
 * ----------------------------------------------------------------------------
 * The declaration of a handle to the instance of a stream (see RFC 3264).
 * The handle of the stream should be provided while calling the
 * Offer-Answer Stream API functions, such as RvOaStreamGetStatus().
 ***************************************************************************@*/
RV_DECLARE_HANDLE(RvOaStreamHandle);

/*@****************************************************************************
 * RvOaAppStreamHandle (Offer-Answer Manager)
 * ----------------------------------------------------------------------------
 * The declaration of an application handle to the instance of a SDP Stream.
 * The application should use the application handle as a reference
 * to the application SDP stream, such as the RTP channel handle.
 * This handle can be set into an Offer-Answer stream at any time after
 * the object was constructed. The Offer-Answer streams are constructed automatically
 * on RvOaSessionGenerateOffer() or RvOaSessionSetRcvdMsg() calls.
 ***************************************************************************@*/
RV_DECLARE_HANDLE(RvOaAppStreamHandle);

/*@****************************************************************************
 * RvOaLogFilters (Offer-Answer Manager)
 * ----------------------------------------------------------------------------
 * RvOaLogFilters defines log filters to be used by the Offer-Answer Module.
 ***************************************************************************@*/
typedef enum {
    RVOA_LOG_NONE_FILTER    = 0x00,
    RVOA_LOG_DEBUG_FILTER   = 0x01,
    RVOA_LOG_INFO_FILTER    = 0x02,
    RVOA_LOG_WARN_FILTER    = 0x04,
    RVOA_LOG_ERROR_FILTER   = 0x08,
    RVOA_LOG_EXCEP_FILTER   = 0x10,
    RVOA_LOG_LOCKDBG_FILTER = 0x20
} RvOaLogFilter;

/*@****************************************************************************
 * RvOaCodec (Offer-Answer Manager)
 * ----------------------------------------------------------------------------
 * RvOaCodec defines codecs, parameters of that, acceptable by both local and
 * remote sides, can be derived by Offer-Answer Module.
 * For G7XX codecs the ptime and silenceSupp are derived.
 * For H26X codecs the framerate, QCIF and CIF are derived.
 * In addition to this for H261 bandwidth is derived, for H263 (RFC_2190)
 * MaxBR is derived.
 * Derivation algorithm is as follow:
 *
 *   ptime:         minimal of offered and capabilities values is taken.
 *                  Is set into ANSWER even if it doesn't appear in OFFER
 *                  or in Capabilities.
 *
 *   silenceSupp:   "on" iff "on" in both OFFER and Capabilities
 *                  Is set into ANSWER only if appears in Capabilities.
 *                  Rest of Silence Suppression parameters are not derived.
 *                  They are simply copied from capabilities.
 *
 *   framerate:     minimal of offered and capabilities values is taken.
 *                  Is set into ANSWER even if it doesn't appear in OFFER
 *                  or in Capabilities.
 *
 *   QCIF:          maximal of offered and capabilities values is taken.
 *                  Is set into ANSWER even if it doesn't appear in OFFER.
 *                  Must appear in Capabilities, otherwise the codec will
 *                  be rejected.
 *
 *   CIF:           maximal of offered and capabilities values is taken.
 *                  Is set into ANSWER even if it doesn't appear in OFFER.
 *                  Must appear in Capabilities, otherwise the codec will
 *                  be rejected.
 *
 *   MaxBR:         minimal of offered and capabilities values is taken.
 *                  Is set into ANSWER even if it doesn't appear in OFFER
 *                  or in Capabilities.
 *
 *  Rest FMTP parameters are copied from capabilities.
 *
 * Parameters of rest codecs are simply copied from the local Capabilities,
 * set by the application. Therefore the application has to implement its own
 * parameter derivation code. Using RvOaDeriveFormatParams callback
 * the application can inspect the result of Offer-Answer module derivation
 * and overwrite it.
 ***************************************************************************@*/
typedef enum {
    RVOA_CODEC_UNKNOWN = -1,
    RVOA_CODEC_G711,     /*0*/
    RVOA_CODEC_G722,     /*1*/
    RVOA_CODEC_G729,     /*2*/
    RVOA_CODEC_G7231,    /*3*/
    RVOA_CODEC_H261,     /*4*/
    RVOA_CODEC_H263_2190 /* RFC 2190 */
} RvOaCodec;

/*@****************************************************************************
 * RvOaLogSource (Offer-Answer Manager)
 * ----------------------------------------------------------------------------
 * Defines modules that produce logs and that have their own
 * log filters.
 ***************************************************************************@*/
typedef enum {
    RVOA_LOG_SRC_GENERAL,  /* All the modules not specified by this enumeration. */
    RVOA_LOG_SRC_CAPS,     /* The capabilities-related code. */
    RVOA_LOG_SRC_MSGS      /* Received/To Be Sent message-related code.
                              The information about messages that were received
                              and are about to be sent is logged using this source
                              and the INFO log filter. */
} RvOaLogSource;

/*@****************************************************************************
 * RvOaLogEntryPrint (Offer-Answer Manager)
 * ----------------------------------------------------------------------------
 * General:
 *  Notifies the application each time a line should be printed to the log.
 *  The application can decide whether to print the line to the screen, file
 *  or other output device. You set this callback in the RvOaCfg structure
 *  before initializing the Offer-Answer Module. If you do not implement this function,
 *  a default logging will be used and the line will be written to the Oa.txt file.
 *
 * Arguments:
 * Input:  context       - The context that was given in the callback
 *                         registration process.
 *         filter        - The filter that this message is using (such as INFO)
 *         formattedText - The text to be printed to the log. The text
 *                         is formatted as follows:
 *                         <filer> - <module> - <message>
 *                         For example:
 *                         "INFO - OA - OA was constructed successfully."
 ***************************************************************************@*/
typedef void (RVCALLCONV * RvOaLogEntryPrint)(
                                        IN void*           context,
                                        IN RvOaLogFilter   filter,
                                        IN const RvChar*   formattedText);

/*@****************************************************************************
 * Type: RvOaMediaFormatInfo (Offer-Answer Manager)
 * ----------------------------------------------------------------------------
 * Contains the information regarding Media Format, that migth be need
 * to the application in order to derive parameters of media format,
 * acceptable by both sides of Offer-Answer session.
 * This information is provided to the application with RvOaDeriveFormatParams
 * callback.
 ***************************************************************************@*/
typedef struct {
    RvUint32 formatOffer;
        /* The offered format. Can be considered as a format payload type,
           presenting in format list of m-line, contained in OFFER message.
           Note the offered format is used in the ANSWER message. */

    RvUint32 formatIdxOffer;
        /* 0-based index of the format in format list of m-line, contained
           in OFFER message, i.e. index of offered format in m-line.
           The format index is required by some SDP Stack API functions. */

    RvUint32 formatCaps;
        /* The capabilities format. Can be considered as a format payload type,
           presenting in format list of m-line, contained in capabilities.
           May differ from the formatOffer, if the different dynamic payload
           types were chose by the offerer and answerer for the same codec.*/

    RvUint32 formatIdxCaps;
        /* 0-based index of the format in format list of m-line, contained
           in capabilities.
           This index is required by some SDP Stack API functions. */

    RvOaCodec eCodec;
        /* Codec, specified by the media format, if the derivation of format
           parameters was performed by the Offer-Answer module.
           If eCodec value is RVOA_CODEC_UNKNOWN, the Offer-Answer simply
           copies format parameters from local Capabilities. */

} RvOaMediaFormatInfo;

/*@****************************************************************************
 * RvOaDeriveFormatParams (Offer-Answer Manager)
 * ----------------------------------------------------------------------------
 * General:
 *  Provides the application with the data, needed in order to derive
 *  parameters of media format or codec, that are acceptable by both sides.
 *  This callback is called for each media format, supported by both sides,
 *  when the ANSWER message is built.
 *
 *  The application should derive the parameters and set them into the provided
 *  Media Descriptor object, using SDP Stack API. This Media Descriptor is a
 *  part of ANSWER message, that will be delivered to the remote side.
 *
 *  The application can choose the default behavior of Offer-Answer module.
 *  In this case the Offer-Answer module will derive the parameters and set
 *  them into the ANSWER media descriptor.
 *  The default behavior depends on media format and can be:
 *         1. Simple copying of format related parameters and all attributes
 *            from local Capabilities;
 *         2. Sophisticated derivation of parameters acceptable by both sides. 
 *  The sophisticated derivation is supported for codecs, enumerated by
 *  RvOaCodec enumeration.
 *  For details of derivation algorithm see documentation for RvOaCodec type.
 *
 *  IMPORTANT:
 *  The application implementation of this callback should be short as possible
 *  and should not call Offer-Answer module API functions in order to prevent
 *  deadlocks, since Session and Stream objects still locked while calling this
 *  callback.
 *
 * Arguments:
 * Input:  hSession          - The handle to the Session object.
 *         hAppSession       - The handle to the application Session object.
 *         pMediaDescrOffer  - The pointer to the Media Descriptor object,
 *                             which contains offered media format and
 *                             format parameters.
 *         pMediaDescrCaps   - The pointer to the Media Descriptor object,
 *                             which contains the media format and format
 *                             parameters, set by the application as a local
 *                             capabilities.
 *         pMediaDescrAnswer - The pointer to the Media Descriptor object,
 *                             which contains the media format and
 *                             format parameters, derived by the Offer-Answer
 *                             module using pMediaDescrOffer and
 *                             pMediaDescrCaps media descriptors.
 *         pFormatInfo       - Auxiliary information about media format.
 *                             It uses the pMediaDescrCaps descriptor.
 *         pSdpMsgOffer      - The pointer to SDP Stack Message object,
 *                             which contains offered media format.
 *                             pMediaDescrOffer is a part of this object.
 *         pSdpMsgAnswer     - The pointer to SDP Stack Message object,
 *                             which contains derived media format.
 *                             pMediaDescrAnswer is a part of this object.
 * Output: pbRemove          - RV_TRUE, if the application wants module to
 *                             remove the media format from the ANSWER.
 *                             The application may want to remove format for
 *                             example due to lack of resources.
 ***************************************************************************@*/
typedef void (RVCALLCONV * RvOaDeriveFormatParams)(
                                    IN  RvOaSessionHandle    hSession,
                                    IN  RvOaAppSessionHandle hAppSession,
                                    IN  RvSdpMediaDescr*     pMediaDescrOffer,
                                    IN  RvSdpMediaDescr*     pMediaDescrCaps,
                                    IN  RvSdpMediaDescr*     pMediaDescrAnswer,
                                    IN  RvOaMediaFormatInfo* pFormatInfo,
                                    IN  RvSdpMsg*            pSdpMsgOffer,
                                    IN  RvSdpMsg*            pSdpMsgAnswer,
                                    OUT RvBool*              pbRemove);

/*@****************************************************************************
 * Type: RvOaCfg (Offer-Answer Manager)
 * ----------------------------------------------------------------------------
 * Contains the configuration parameters of the Offer-Answer Module.
 * This structure can be initialized with default values, using RvOaInitCfg().
 ***************************************************************************@*/
typedef struct {
    RvUint32  maxSessions;
        /*  The maximum number of concurrent SDP Sessions.
            Minimal value: 1
            Default value: DEFAULT_CFG_MAX_SESSIONS (10) */

    RvUint32  maxStreams;
        /*  The maximum number of concurrent Streams.
            Minimal value: 1
            Default value: DEFAULT_CFG_MAX_STREAMS (20) */

    RvUint32  messagePoolPageSize;
        /*  The size of the page in the message pool.
            Minimal value: 1.
            Default value: DEFAULT_CFG_MESSAGEPAGE_SIZE (4096) */

    /* ---- Capabilities-related values ---- */
    RvUint32  maxSessionsWithLocalCaps;
        /*  The maximum number of concurrent SDP Sessions,
            for which the application sets the local capabilities,
            using RvOaSessionSetCapabilties().
            This parameter should not be affected by the default capabilities,
            that the application can set using RvOaSetCapabilties().
            Minimal value: 0.
            Default value: DEFAULT_CFG_MAX_SESSIONS_WITH_LOCAL_CAPS (0) */

    RvUint32  capPoolPageSize;
        /*  The size of the page in the capabilities pool.
            The capabilities, both the local capabilities set per session and
            default capabilities set per module, are kept in the format of
            the RADVISION SDP Stack message. Therefore, this parameter represents
            the size of the message.
            Minimal value: 1.
            Default value: DEFAULT_CFG_MESSAGEPAGE_SIZE (4096) */

    RvUint32  avgNumOfMediaFormatsPerCaps;
        /*  The average number of media formats for all types of media
            that can be found in the local or default capabilities.
            Minimal value: 1.
            Default value: DEFAULT_CFG_AVG_FORMATS_PER_CAPS (10) */

    RvBool  bSetOneCodecPerMediaDescr;
        /*  This flag is taken into account while building the ANSWER message.
            If its value is RV_TRUE, the subset of media formats that is
            supported by both sides will be limited to one codec only.
            Default value: RV_FALSE */

    RvOaDeriveFormatParams pfnDeriveFormatParams;
        /*  The application implementation of parameter derivation algorithm,
            used in order to get codec parameters, acceptable by both sides of
            the Offer-Answer session, while building ANSWER.
            For more details see documentation for RvOaDeriveFormatParams type.
            Default value: NULL */

    /* ---- Log-related variables ---- */
    RV_LOG_Handle  logHandle;
        /*  The LOG handle to use for logging messages.
            This handle should point to the Log Manager object.
            that the application provides.
            Default value: NULL */

    RvOaLogEntryPrint  pfnLogEntryPrint;
        /*  See the RvOaLogEntryPrint() callback function for details.
            Default value: NULL */

    void*  logEntryPrintContext;
        /*  See the RvOaLogEntryPrint() callback function for details.
            Default value: NULL */

    RvUint32  logFilter;
        /*  Filters for logged information, produced by the Offer-Answer
            Module.
            See the RvOaLogFilters type definition and the RvOaLogEntryPrint()
            callback function for more details.
            The default value is set by RvOaInitCfg():
                INFO + ERROR + DEBUG + EXCEP + WARN */

    RvUint32  logFilterCaps;
        /*  Filters for logged information, produced by the capabilities-
            related code of the Offer-Answer Module.
            See the RvOaLogFilters type definition and the RvOaLogEntryPrint()
            callback function for more details.
            The default value is set by RvOaInitCfg:
                INFO + ERROR + DEBUG + EXCEP + WARN */

    RvUint32  logFilterMsgs;
        /*  Filters for logged information produced by the message-related
            code of the add-on [tbd]. This parameter should be used for monitoring
            SDP messages that the Offer-Answer Module provided or generated.
            See the RvOaLogFilters type definition and the RvOaLogEntryPrint()
            callback function for more details.
            The default value is set by
                INFO + ERROR + DEBUG + EXCEP + WARN */

} RvOaCfg;

/*@****************************************************************************
 * Type: RvOaResource (Offer-Answer Manager)
 * ----------------------------------------------------------------------------
 * A structure holding information about the memory resources.
 * This is a generic type used by other Offer-Answer specific types.
 ***************************************************************************@*/
typedef struct
{
    RvUint32 numOfAllocatedElements;
        /* The number of allocated elements in a structure, such as elements in
           a list, cells in an array, and so on. */

    RvUint32 currNumOfUsedElements;
        /* The number of elements currently used. */

    RvUint32 maxUsageOfElements;
        /* The maximal number elements that were used. */

} RvOaResource;

/*@****************************************************************************
 * Type: RvOaResources (Offer-Answer Manager)
 * ----------------------------------------------------------------------------
 * A structure holding information about the resources of the Offer-Answer Module.
 ***************************************************************************@*/
typedef struct {
    RvOaResource   sessions;
    RvOaResource   streams;
    RvOaResource   messagePoolPages;
    RvOaResource   capPoolPages; /* Capabilities */
} RvOaResources;


/*---------------------------------------------------------------------------*/
/*                        SESSION TYPE DEFINITIONS                           */
/*---------------------------------------------------------------------------*/

/*@****************************************************************************
 * RvOaSessionState (Offer-Answer Session)
 * ----------------------------------------------------------------------------
 * Represents the state of the SDP Session, while negotiating
 * media capabilities and addresses. The application can retrieve this state using
 * RvOaSessionGetState().
 ***************************************************************************@*/
typedef enum
{
    RVOA_SESSION_STATE_UNDEFINED = -1,
        /* Sessions that are not allocated currently stay in UNDEFINED
           state. Calling RvOaCreateSession() moves the session from this
           state to the IDLE state. Calling RvOaSessionTerminate() moves the
           session to the UNDEFINED state. */

    RVOA_SESSION_STATE_IDLE,        /*0*/
        /* The newly allocated session resides in the IDLE state.
           Calling RvOaSessionGenerateOffer() or RvOaSessionSetRcvdMsg()
           moves the session out of this state. Calling
           RvOaSessionGenerateOffer() moves the object to the
           OFFER_READY state. Calling RvOaSessionSetRcvdMsg() moves the
           object to the ANSWER_READY state. */

    RVOA_SESSION_STATE_OFFER_READY, /*1*/
        /* The session in the OFFER_READY state has an OFFER message that is
           ready to be sent to the remote side. This message can be gotten from
           the session using RvOaSessionGetMsgToBeSent(). Calling to
           RvOaSessionSetRcvdMsg() moves the session from
           this state to the ANSWER_RCVD state.
           Note that RvOaSessionGenerateOffer() can be called only for
           sessions that reside in the IDLE state. */

    RVOA_SESSION_STATE_ANSWER_RCVD, /*2*/
        /* The session in the ANSWER_RCVD state handled the received ANSWER
           message. The negotiation is done. The application can retrieve
           addresses and media parameters that are acceptable by both the local
           and remote sides.
           The application can provide the received ANSWER message
           to the session using RvOaSessionSetRcvdMsg().
           Calling RvOaSessionGenerateOffer() or RvOaSessionSetRcvdMsg()
           moves the session out of this state. If these functions
           are called for an object in the ANSWER_RCVD state, this means that
           the Session modification process began.
           Calling to RvOaSessionGenerateOffer() moves the object to
           the OFFER_READY state. Calling RvOaSessionSetRcvdMsg() moves the object
           to the ANSWER_READY state.
           Note that RvOaSessionSetRcvdMsg() can be called only for
           sessions that reside in the OFFER_READY or IDLE states. */

    RVOA_SESSION_STATE_ANSWER_READY /*3*/
        /* The session in the ANSWER_READY state has an ANSWER message
           that the negotiation is done. The application can retrieve
           addresses and media parameters that are acceptable by both the local
           and remote sides.
           The application can provide the received OFFER message
           to the session using RvOaSessionSetRcvdMsg().
           Calling RvOaSessionGenerateOffer() or RvOaSessionSetRcvdMsg()
           moves the session out of this state. If these functions
           are called for an object in the ANSWER_READY state, this means
           that the session modification process began. Calling
           RvOaSessionGenerateOffer() moves the object to the
           OFFER_READY state. Calling  RvOaSessionSetRcvdMsg() moves the
           object to the ANSWER_READY state.
           Note that RvOaSessionSetRcvdMsg() can be called only for
           sessions that reside in the OFFER_READY or IDLE states. */

} RvOaSessionState;

/*---------------------------------------------------------------------------*/
/*                       Offer-Answer Stream TYPE DEFINITIONS                       */
/*---------------------------------------------------------------------------*/

/*@****************************************************************************
 * RvOaStreamCfg (Offer-Answer Stream API)
 * ----------------------------------------------------------------------------
 * Holds the media parameters required by the stream creation function,
 * RvOaSessionAddStream().
 ***************************************************************************@*/
typedef struct
{
    RvSdpMediaType eMediaType;
        /* m-line parameter: the type of media, such as audio or video. */

    RvInt32 port;
        /* m-line parameter: the port for incoming media. */

    RvSdpProtocol  eProtocol;
        /* m-line parameter: the protocol for media delivery, such as RTP/AVP. */

} RvOaStreamCfg;

/*@****************************************************************************
 * RvOaStreamState (Offer-Answer Stream)
 * ----------------------------------------------------------------------------
 * Represents the state of the media stream. The application can retrieve this
 * state using RvOaStreamGetStatus().
 ***************************************************************************@*/
typedef enum
{
    RVOA_STREAM_STATE_UNDEFINED = -1,
        /* The streams that are not allocated currently stay in the UNDEFINED
           state. */

    RVOA_STREAM_STATE_IDLE,    /*0*/
        /* The streams moves to the IDLE state upon allocation. */

    RVOA_STREAM_STATE_ACTIVE,  /*1*/
        /* The streams moves to the ACTIVE state as a result of OFFER
        generating or OFFER reception. */

    RVOA_STREAM_STATE_HELD,    /*2*/
        /* The stream moves to the HELD state as a result of the processing
           of the incoming holding OFFER. The procedures for hold and resume are
           described in RFC 3264. */

    RVOA_STREAM_STATE_RESUMED, /*3*/
        /* The stream moves to RESUMED state as a result of the processing
           of the incoming resuming OFFER. The procedures for hold and resume are
           described in RFC 3264. */

    RVOA_STREAM_STATE_REMOVED  /*4*/
        /* The stream moves to REMOVED state as a result of the application
           call to RvOaSessionResetStream(). */

} RvOaStreamState;

/*@****************************************************************************
 * RvOaStreamStatus (Offer-Answer Stream API)
 * ----------------------------------------------------------------------------
 * Holds various data about the state or status of the stream.The application
 * may want to request this data after an OFFER message was processed
 * because an OFFER message can create a new stream or modify an existing one.
 * The application can retrieve this data using RvOaStreamGetStatus().
 ***************************************************************************@*/
typedef struct
{
    RvOaStreamState eState;
        /* m-line parameter: The type of media, such as audio or video. */

    RvBool bNewOffered;
        /* If RV_TRUE, the stream was added by the remote side.
           This flag is set only for newly offered streams.
           It is reset on the reception or generation of the session modifying
           OFFER message. */

    RvBool bWasModified;
        /* If RV_TRUE, some parameters of the stream were modified by the
           remote side.
           This flag does not take into account the HOLD/RESUME parameters. */

    RvBool bWasClosed;
        /* If RV_TRUE, stream was closed / rejected by the remotre side.
           This flag is reset to RV_FALSE when the stream is closed locally. */

    RvBool bAddressWasModified;
        /* If RV_TRUE, address for media reception was modified by the remote
           side. The address can be contained in session level c-line or in
           media level c-line.
           The address modification in media level c-line sets also
           the bWasModified flag to RV_TRUE. */

} RvOaStreamStatus;

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef _RV_OA_TYPES_H */
