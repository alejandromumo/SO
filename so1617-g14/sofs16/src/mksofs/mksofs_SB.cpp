#include "mksofs.h"

#include "superblock.h"
#include "exception.h"
#include "inode.h"
#include "core.h"
#include "cluster.h"

#include <errno.h>

int nearest_multiple(int n, int m)
{
    if (m == 0) return n;
    if (n == 0) return m;
    int r = n % m;
    if (r == 0) return n;
    return n + m - r;
}

int ceil_integer_division(int x, int y)
{
    return (x + y - 1) / y;
}

/* filling in the superblock fields:
   *   magic number should be set presently to 0xFFFF,
   *   this enables that if something goes wrong during formating, the
   *   device can never be mounted later on
   */
void fillInSuperBlock(SOSuperBlock *sbp, const char *name, uint32_t ntotal, uint32_t itotal, uint32_t bpc)
{
    // General metadata

    sbp->magic = 0xFFFF;
    sbp->version = VERSION_NUMBER;
    strncpy(sbp->name, name, PARTITION_NAME_SIZE + 1);
    sbp->name[PARTITION_NAME_SIZE + 1] = '\0'; // Truncate partition name
    sbp->mstat = PRU; // PRoperly Unmounted by default
    sbp->csize = bpc; // Blocks per cluster
    sbp->ntotal = (ntotal > 3 ) ? ntotal : 4; // Total number of blocks


    // Inode table

    sbp->itstart = 1; // inode table starts at block 1
    sbp->itotal = nearest_multiple(itotal, IPB); // starting number of inodes
    sbp->itotal += IPB*((sbp->ntotal - 1 - ceil_integer_division(sbp->itotal, IPB)) % sbp->csize); // add cluster orphan block(s) to inode table
    sbp->itsize = ceil_integer_division(sbp->itotal, IPB);
    sbp->ifree = sbp->itotal-1; // all inodes are free except the root dir
    sbp->ihead = 1; // head is inode 1, root dir is presumably filled in
    sbp->itail = sbp->ifree; // tail is the last free inode

    // Cluster zone

    sbp->czstart = sbp->itsize + 1; // cluster zone starts after inode table
    sbp->ctotal = (sbp->ntotal - 1 - sbp->itsize) / sbp->csize;
    sbp->cfree = (sbp-> ctotal > 1 ) ? sbp->ctotal - 1 : 0; // initial cfree
    sbp->crefs = ceil_integer_division(sbp->cfree, RPB*sbp->csize); // solve for crefs: (n-1)*crefs >= cfree-crefs
    sbp->cfree = (sbp -> ctotal > 1) ? sbp->ctotal - 1 - sbp->crefs : 0; // final cfree

    // Head of the caches of references to free clusters

    sbp->chead.cluster_number = (sbp->crefs > 0) ? 1: NULL_REFERENCE;
    sbp->chead.cluster_idx = 0;
    memset(sbp->chead.cache.ref, NULL_REFERENCE, FCT_CACHE_SIZE*sizeof(uint32_t));
    sbp->chead.cache.in = 0;
    sbp->chead.cache.out = 0;

    // Tail of the caches of references to free clusters

    sbp->ctail.cluster_number = (sbp->crefs > 0) ? sbp->crefs : NULL_REFERENCE;
    sbp->ctail.cluster_idx = (sbp->cfree) % (RPB * sbp->csize - 1);
    memset(sbp->ctail.cache.ref, NULL_REFERENCE, FCT_CACHE_SIZE*sizeof(uint32_t));
    sbp->ctail.cache.in = 0;
    sbp->ctail.cache.out = 0;
}