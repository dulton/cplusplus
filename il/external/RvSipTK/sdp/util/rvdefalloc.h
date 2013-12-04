#ifndef RV_DEFALLOC_H
#define RV_DEFALLOC_H

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#   include "rvalloc.h"
#   ifndef rvDefaultAlloc
#   define rvDefaultAlloc (*(RvAlloc *)(rvAllocGetDefaultAllocator()))
#   endif
#endif
/* SPIRENT_END */

#endif
