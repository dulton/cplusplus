/************************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/

/********************************************************************************************
 *                                defs.h
 *
 * This file provides definitions for the entire test application.
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *      Oren Libis                          04-Mar-2000
 *
 ********************************************************************************************/

#ifndef _DEFS_H
#define _DEFS_H

#ifdef __cplusplus
extern "C" {
#endif


#include "rtptObj.h"

#include "RTPT_general.h"


#define BUFFER_SIZE 1024


/* TCL resources */
extern Tcl_Interp*  interp;



/********************************************************************************************
 *                             struct ConnectionInfo
 *
 * This struct holds the application's information about a TCP connection.
 *
 ********************************************************************************************/
typedef struct 
{
    int     counter; /* Counter value of this connection */
    int     color; /* Color assigned for this connection host */
} ConnectionInfo;




#ifdef __cplusplus
}
#endif

#endif /* _DEFS_H */

