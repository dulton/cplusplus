/***********************************************************************
Copyright (c) 2003 RADVISION Ltd.
************************************************************************
NOTICE:
This document contains information that is confidential and proprietary
to RADVISION Ltd.. No part of this document may be reproduced in any
form whatsoever without written prior approval by RADVISION Ltd..
RADVISION Ltd. reserve the right to revise this publication and make
changes without obligation to notify any person of such revisions or
changes.
***********************************************************************/

#include "rtptObj.h"
/********************************************************************************************
 * rtptStackInit
 * purpose : inits the test application RTPT object
 * input   : pointer to rtptObj
 * output  : none
 * return  : RV_OK
 ********************************************************************************************/
RvStatus rtptStackInit(
    IN rtptObj       *ta)
{
	RvUint32 sessionCount = 0;
	RvStatus status = RV_OK;

	memset(ta, 0, sizeof(rtptObj));
    
	status = RvRtpInit();
	if (status == RV_OK)
		status = RvRtcpInit();
	for (sessionCount=0; sessionCount<RTP_TEST_APPLICATION_SESSIONS_MAX; sessionCount++)
		if (status == RV_OK)
		{
			status = rtptSessionObjInit(ta, &ta->session[sessionCount], &ta->encryptors);
            (ta->session)[sessionCount].id = sessionCount;
		}
	rvRtpDesEncryptionConstruct(&ta->encryptors.desEncryptor);
	rvRtp3DesEncryptionConstruct(&ta->encryptors.tripleDesEncryptor);

    ta->cb.rtptEventIndication = NULL;

	return status;
}
/********************************************************************************************
 * rtptStackEnd
 * purpose : endsworking with the RTPT object
 * input   : pointer to rtptObj
 * output  : none
 * return  : RV_OK
 ********************************************************************************************/
RvStatus rtptStackEnd(
    IN rtptObj       *ta)
{
	RvUint32 sessionCount = 0;
	RvStatus status = RV_OK;
	
	status = RvRtcpEnd();
	if (status == RV_OK)
		RvRtpEnd();
	for (sessionCount=0;sessionCount<RTP_TEST_APPLICATION_SESSIONS_MAX;sessionCount++)
		if (status == RV_OK)
			status = rtptSessionObjInit(ta, &ta->session[sessionCount], &ta->encryptors);
		
	rvRtpDesEncryptionDestruct(&ta->encryptors.desEncryptor);
	rvRtp3DesEncryptionDestruct(&ta->encryptors.tripleDesEncryptor);
		
	return status;
}


/********************************************************************************************
 * rtptGetLogSource
 * purpose : Accesor to the TA log source
 * input   : none
 * output  : none
 * return  : pointer to the application log source
 ********************************************************************************************/
RvLogSource* rtptGetLogSource(rtptObj* rtptPtr)
{
    if (rtptPtr->appLogSourceInited)
        return &rtptPtr->appLogSource;
    else
        return NULL;
}
