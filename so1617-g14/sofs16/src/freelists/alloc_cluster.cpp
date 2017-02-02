/**
 *  \author Inês Moreira
 *  \tester Inês Moreira
 */

#include "freelists.h"

#include "probing.h"
#include "exception.h"
#include "sbdealer.h"
#include "core.h"
#include <errno.h>

/*
 * Dictates to be obeyed by the implementation:
 * - error ENOSPC should be thrown if there is no free clusters
 * - after the reference is removed,
 *      its location should be filled with NULL_REFERENCE
 */
void soAllocCluster(uint32_t * cnp)
{
    soProbe(713, "soAllocCluster(%u)\n", cnp);

    SOSuperBlock *sbp = sbGetPointer();

    /* check if there are free clusters available */
    if(sbp->cfree == 0)
        throw SOException(ENOSPC, __FUNCTION__);

    /* if the head cache is empty, fill it with free clusters */
    if(sbp->chead.cache.ref[sbp->chead.cache.out] == NULL_REFERENCE)
        soReplenish();

    /* alloc the first free cluster and change its reference in the head cache to null */
    *cnp = sbp->chead.cache.ref[sbp->chead.cache.out];
    sbp->chead.cache.ref[sbp->chead.cache.out] = NULL_REFERENCE;

    /* decrease the number of free clusters */
    sbp->cfree--;

    /* resize the head cache */
    sbp->chead.cache.out = (sbp->chead.cache.out + 1) % FCT_CACHE_SIZE;

    sbSave();
}
