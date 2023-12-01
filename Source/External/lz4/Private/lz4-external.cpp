// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Allocator/TrackingMalloc.h"
#include "Memory/MemoryDomain.h"

// From lz4.c:
 /* memory management functions can be customized by user project.
  * Below functions must exist somewhere in the Project
  * and be available at link time */
void* LZ4_malloc(size_t s) {
    return TRACKING_MALLOC(LZ4, s);
}
void* LZ4_calloc(size_t n, size_t s) {
    return TRACKING_CALLOC(LZ4, n, s);
}
void  LZ4_free(void* p) {
    TRACKING_FREE(LZ4, p);
}
