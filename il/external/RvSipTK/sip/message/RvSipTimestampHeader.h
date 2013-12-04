/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipTimestampHeader.h								  *
 *                                                                            *
 * The file defines the methods of the Timestamp header object:				  *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *      Author                 Date                                           *
 *     ------               ------------                                      *
 *    Galit Edri Domani      May 2005                                         *
 *                                                                            *
 ******************************************************************************/

#ifndef RVSIPTIMESTAMPHEADER_H
#define RVSIPTIMESTAMPHEADER_H

#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT

#include "RvSipMsgTypes.h"
#include "rpool_API.h"

/*-----------------------------------------------------------------------*/
/*                   DECLERATIONS                                        */
/*-----------------------------------------------------------------------*/

/*
 * RvSipTimestampHeaderStringName
 * ----------------------
 * This enum defines all the header's strings (for getting it's size)
 * Defines all Warning header object fields that are kept in the
 * object in a string format.
 */

typedef enum
{
    RVSIP_TIMESTAMP_BAD_SYNTAX

}RvSipTimestampHeaderStringName;

#ifdef RV_SIP_JSR32_SUPPORT
/*
 * RvSipTimestampHeaderFieldName
 * ----------------------------
 * This enum defines all the Timestamp header fields.
 * It is used for getting and setting via RvSipHeader interface.
 */
typedef enum
{
	RVSIP_TIMESTAMP_FIELD_TIMESTAMP_VAL = 0,
	RVSIP_TIMESTAMP_FIELD_DELAY_VAL     = 1,
	RVSIP_TIMESTAMP_FIELD_BAD_SYNTAX    = 2
}RvSipTimestampHeaderFieldName;
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * RvSipTimestampHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Timestamp Header 
 *          object. The header is constructed on a given page taken from a 
 *          specified pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Timestamp header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderConstruct(
                                         IN  RvSipMsgMgrHandle           hMsgMgr,
                                         IN  HRPOOL                      hPool,
                                         IN  HPAGE                       hPage,
                                         OUT RvSipTimestampHeaderHandle *hHeader);



/***************************************************************************
 * RvSipTimestampHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs an Timestamp header object inside a given message 
 *          object. The header is kept in the header list of the message. You 
 *          can choose to insert the header either at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should 
 *                            be pushed to the head of the list (RV_TRUE), or to 
 *                            the tail (RV_FALSE).
 * output: hHeader          - Handle to the newly constructed Timestamp 
 *                            header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderConstructInMsg(
                                 IN  RvSipMsgHandle                       hSipMsg,
                                 IN  RvBool                               pushHeaderAtHead,
                                 OUT RvSipTimestampHeaderHandle           *hHeader);

/***************************************************************************
 * RvSipTimestampHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Timestamp header object to a destination 
 *          Timestamp header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Timestamp header object.
 *    hSource      - Handle to the source Timestamp header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderCopy
                                    (IN    RvSipTimestampHeaderHandle hDestination,
                                     IN    RvSipTimestampHeaderHandle hSource);

/***************************************************************************
 * RvSipTimestampHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes an Timestamp header object to a textual Timestamp header. The
 *          textual header is placed on a page taken from a specified pool. 
 *          In order to copy the textual header from the page to a consecutive 
 *          buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle to the Timestamp header object.
 *        hPool             - Handle to the specified memory pool.
 * output: hPage            - The memory page allocated to contain the encoded header.
 *         length           - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderEncode(
                                          IN    RvSipTimestampHeaderHandle    hHeader,
                                          IN    HRPOOL                     hPool,
                                          OUT   HPAGE*                     hPage,
                                          OUT   RvUint32*                  length);

/***************************************************************************
 * RvSipTimestampHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Timestamp header into a Timestamp header object.
 *          All the textual fields are placed inside the object.
 *          Note: Before performing the parse operation the stack 
 *          resets all the header fields. Therefore, if the parse 
 *          function fails, you will not be able to get the previous 
 *          header field values.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Timestamp header object.
 *    buffer    - Buffer containing a textual Timestamp header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderParse(
                                     IN RvSipTimestampHeaderHandle  hHeader,
                                     IN RvChar*                  buffer);


/***************************************************************************
 * RvSipTimestampHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Timestamp header value into an Timestamp header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipTimestampHeaderParse() function to parse strings that also
 *          include the header-name.
 *          Note: Before performing the parse operation the stack 
 *          resets all the header fields. Therefore, if the parse 
 *          function fails, you will not be able to get the previous 
 *          header field values.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Timestamp header object.
 *    buffer    - The buffer containing a textual Timestamp header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderParseValue(
                                     IN RvSipTimestampHeaderHandle  hHeader,
                                     IN RvChar*                  buffer);

/***************************************************************************
 * RvSipTimestampHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Timestamp header with bad-syntax information.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          Use this function to fix the header. This function parses a given
 *          correct header-value string to the supplied header object.
 *          If parsing succeeds, this function places all fields inside the
 *          object and removes the bad syntax string.
 *          If parsing fails, the bad-syntax string in the header remains as it was.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader      - The handle to the header object.
 *        pFixedBuffer - The buffer containing a legal header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderFix(
                                     IN RvSipTimestampHeaderHandle   hHeader,
                                     IN RvChar*                   pFixedBuffer);





/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/

/***************************************************************************
 * RvSipTimestampHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Timestamp header fields are kept in a string format, for
 *          example, the Timestamp header bad syntax string. In order to get such a field
 *          from the Timestamp header object, your application should supply an
 *          adequate buffer to where the string will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the Timestamp header object.
 *    eStringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipTimestampHeaderGetStringLength(
                                      IN  RvSipTimestampHeaderHandle     hHeader,
                                      IN  RvSipTimestampHeaderStringName eStringName);





/***************************************************************************
* RvSipTimestampHeaderSetTimestampInt
* ------------------------------------------------------------------------
* General: Sets the timestamp value from the Timestamp Header object.
* Return Value: RvStatus.
* ------------------------------------------------------------------------
* Arguments:
*    hHeader - Handle to the Timestamp header object.
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderSetTimestampVal(
												IN RvSipTimestampHeaderHandle hHeader,
												IN RvInt32         timestampIntPart,
												IN RvInt32         timestampDecPart);

/***************************************************************************
 * RvSipTimestampHeaderGetTimestampVal
 * ------------------------------------------------------------------------
 * General: Gets the timestamp value from the Timestamp Header object.
 * Return Value: Returns the timestamp value.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Timestamp header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderGetTimestampVal(
                                     IN RvSipTimestampHeaderHandle hHeader,
									 OUT RvInt32         *timestampIntPart,
									 OUT RvInt32         *timestampDecPart);

/***************************************************************************
 * RvSipTimestampHeaderGetDelayVal
 * ------------------------------------------------------------------------
 * General: Sets the delay value in the Timestamp Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader           - Handle to the Timestamp header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderGetDelayVal(
								   IN RvSipTimestampHeaderHandle hHeader,
								   OUT RvInt32         *delayIntPart,
								   OUT RvInt32         *delayDecPart);

/***************************************************************************
 * RvSipTimestampHeaderSetDelayVal
 * ------------------------------------------------------------------------
 * General: Gets the delay value from the Timestamp Header object.
 * Return Value: Returns the delay value.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Timestamp header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderSetDelayVal(
                                    IN RvSipTimestampHeaderHandle hHeader,
									IN RvInt32         delayIntPart,
									IN RvInt32         delayDecPart);

/***************************************************************************
 * RvSipTimestampHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Copies the bad-syntax string from the header object into a
 *          given buffer.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          You use this function to retrieve the bad-syntax string.
 *          If the value of bufferLen is adequate, this function copies
 *          the requested parameter into strBuffer. Otherwise, the function
 *          returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required
 *          buffer length.
 *          Use this function in the RvSipTransportBadSyntaxMsgEv() callback
 *          implementation if the message contains a bad Timestamp header,
 *          and you wish to see the header-value.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - The handle to the header object.
 *        strBuffer  - The buffer with which to fill the bad syntax string.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the bad syntax + 1, to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderGetStrBadSyntax(
                                     IN  RvSipTimestampHeaderHandle  hHeader,
                                     IN  RvChar*							  strBuffer,
                                     IN  RvUint								  bufferLen,
                                     OUT RvUint*							  actualLen);


/***************************************************************************
 * RvSipTimestampHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal
 *          Timestamp header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderSetStrBadSyntax(
                                  IN RvSipTimestampHeaderHandle hHeader,
                                  IN RvChar*							 strBadSyntax);

/***************************************************************************
 * RvSipTimestampHeaderGetRpoolString
 * ------------------------------------------------------------------------
 * General: Copy a string parameter from the Timestamp header into a given page
 *          from a specified pool. The copied string is not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hTimestampHeader  - Handle to the Timestamp header object.
 *           eStringName - The string the user wish to get
 *  Input/Output:
 *         pRpoolPtr     - pointer to a location inside an rpool. You need to
 *                         supply only the pool and page. The offset where the
 *                         returned string was located will be returned as an
 *                         output patameter.
 *                         If the function set the returned offset to
 *                         UNDEFINED is means that the parameter was not
 *                         set to the Timestamp header object.
 ***************************************************************************/
/*
RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderGetRpoolString(
                             IN    RvSipTimestampHeaderHandle      hTimestampHeader,
                             IN    RvSipTimestampHeaderStringName  eStringName,
                             INOUT RPOOL_Ptr                 *pRpoolPtr);

  
*/

/***************************************************************************
 * RvSipTimestampHeaderSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the Timestamp header
 *          object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader       - Handle to the Timestamp header object.
 *           eStringName   - The string the user wish to set
 *           pRpoolPtr     - pointer to a location inside an rpool where the
 *                           new string is located.
 ***************************************************************************/
/*

RVAPI RvStatus RVCALLCONV RvSipTimestampHeaderSetRpoolString(
                             IN RvSipTimestampHeaderHandle       hHeader,
                             IN RvSipTimestampHeaderStringName   eStringName,
                             IN RPOOL_Ptr                       *pRpoolPtr);


*/

#endif /* #ifdef RVSIP_ENHANCED_HEADER_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /*RVSIPTimestampHEADER_H*/
