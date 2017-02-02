/**
 *  \author Luís Leira
 *  \tester Luís Leira
 */

#include "freelists.h"

#include "probing.h"
#include "exception.h"
#include "sbdealer.h" /* added */
#include "core.h" /* added */

#include <errno.h>
#include <inttypes.h>

/*
 * Dictates to be obeyed by the implementation:
 * - parameter cn must be validated, 
 *      throwing a proper error if necessary
 */
void soFreeCluster(uint32_t cn)
{
    soProbe(732, "soFreeCluster (%u)\n", cn);

    SOSuperBlock *p_sb;

    /* get superblock pointer */
    p_sb = sbGetPointer();

    /* check if cn is out of range */
    if(cn < 0 || cn >= p_sb->ctotal)
    	throw SOException(EINVAL, __FUNCTION__);

    /* if it's full, deplete it */
    if(p_sb->ctail.cache.ref[p_sb->ctail.cache.in] != NULL_REFERENCE)
    	soDeplete();

    p_sb->ctail.cache.ref[p_sb->ctail.cache.in] = cn;
    p_sb->ctail.cache.in = (p_sb->ctail.cache.in + 1) % FCT_CACHE_SIZE;
    p_sb->cfree++;

    /* save modifications */
    sbSave();
}