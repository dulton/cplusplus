/*
 * Modified for use with MPlayer, detailed CVS changelog at
 * http://www.mplayerhq.hu/cgi-bin/cvsweb.cgi/main/
 * $Id: //TestCenter/integration/content/traffic/l4l7/il/external/vlc-0.8.6i/loader/driver.h#1 $
 */

#ifndef loader_driver_h
#define	loader_driver_h

#ifdef __cplusplus
extern "C" {
#endif

#include "wine/windef.h"
#include "wine/driver.h"

void SetCodecPath(const char* path);
void CodecAlloc(void);
void CodecRelease(void);

HDRVR DrvOpen(LPARAM lParam2);
void DrvClose(HDRVR hdrvr);

#ifdef __cplusplus
}
#endif

#endif