/**
 *  \author Fábio Maia
 *  \tester Fábio Maia
 */

#include "freelists.h"
#include "czdealer.h"

#include "probing.h"
#include "exception.h"
#include "core.h"
#include "superblock.h"
#include "sbdealer.h"
#include <errno.h>

/*
 * Even if some of them are not functionally necessary,
 * the following dictates must be obyed by the implementation:
 * - if crefs is equal to zero,
 *      first transfer as much as possible to head cache
 * -
 */
void soDeplete(void)
{
    soProbe(722, "soDeplete()\n");

    SOSuperBlock *sbp = sbGetPointer();
    uint32_t RPC = soGetRPC();

    /* Check if the FCT is empty */
    if (sbp->crefs == 0)
    {
        /* Check if chead is full */
        if(sbp->chead.cache.ref[sbp->chead.cache.in] != NULL_REFERENCE)
        {
            /* Allocate the tail cluster for the new FCT */
            uint32_t newCluster[RPC];
            uint32_t newClusterNumber;
            soAllocCluster(&newClusterNumber);
            memset(newCluster, NULL_REFERENCE, RPC*sizeof(uint32_t));

            /* Adjust links */
            sbp->chead.cluster_number = sbp->ctail.cluster_number = newClusterNumber;
            sbp->chead.cluster_idx = sbp->ctail.cluster_idx = 0;
            sbp->crefs++;

            /* Copy tail cache references to the new FCT */
            while(sbp->ctail.cache.ref[sbp->ctail.cache.out] != NULL_REFERENCE)
            {
                newCluster[sbp->ctail.cluster_idx] = sbp->ctail.cache.ref[sbp->ctail.cache.out];
                sbp->ctail.cache.ref[sbp->ctail.cache.out] = NULL_REFERENCE;
                sbp->ctail.cache.out = (sbp->ctail.cache.out + 1) % FCT_CACHE_SIZE;
                sbp->ctail.cluster_idx++;
            }

            soWriteCluster(newClusterNumber, newCluster);
        }
        else
        {
            /* Copy tail cache references to head cache */
            while(sbp->chead.cache.ref[sbp->chead.cache.in] == NULL_REFERENCE &&    /* chead is not full */
                  sbp->ctail.cache.ref[sbp->ctail.cache.out] != NULL_REFERENCE)     /* ctail is not empty */
            {
                sbp->chead.cache.ref[sbp->chead.cache.in] = sbp->ctail.cache.ref[sbp->ctail.cache.out];
                sbp->ctail.cache.ref[sbp->ctail.cache.out] = NULL_REFERENCE;
                sbp->chead.cache.in = (sbp->chead.cache.in + 1) % FCT_CACHE_SIZE;
                sbp->ctail.cache.out = (sbp->ctail.cache.out + 1) % FCT_CACHE_SIZE;
            }
        }
    }
    else
    {
        /* Read tail cluster into memory */
        uint32_t tailCluster[RPC];
        soReadCluster(sbp->ctail.cluster_number, tailCluster);

        /* Transfer while tail cache is not empty */
        while (sbp->ctail.cache.ref[sbp->ctail.cache.out] != NULL_REFERENCE)
        {
            /* Check if we need to allocate a new tail cluster */
            if (sbp->ctail.cluster_idx == RPC-1)
            {
                uint32_t oldTailClusterNumber = sbp->ctail.cluster_number;
                soAllocCluster(&sbp->ctail.cluster_number);
                sbp->crefs++;
                tailCluster[RPC-1] = sbp->ctail.cluster_number;
                soWriteCluster(oldTailClusterNumber, tailCluster);
                soReadCluster(sbp->ctail.cluster_number, &tailCluster);
                memset(tailCluster, NULL_REFERENCE, RPC*sizeof(uint32_t));
                sbp->ctail.cluster_idx = 0;
            }

            /* Copy tail cache references to the tail cluster */
            tailCluster[sbp->ctail.cluster_idx] = sbp->ctail.cache.ref[sbp->ctail.cache.out];
            sbp->ctail.cache.ref[sbp->ctail.cache.out] = NULL_REFERENCE;
            sbp->ctail.cache.out = (sbp->ctail.cache.out + 1) % FCT_CACHE_SIZE;
            sbp->ctail.cluster_idx++;
        }

        soWriteCluster(sbp->ctail.cluster_number, tailCluster);
    }

    sbSave();
}
