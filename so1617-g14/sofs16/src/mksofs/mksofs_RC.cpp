#include "mksofs.h"

#include "superblock.h"
#include "exception.h"
#include "rawdisk.h"
#include "cluster.h"

#include <errno.h>

void resetFreeCluster(SOSuperBlock *sbp)
{
    /* Skip the root dir cluster and one more cluster for each reference */
    uint32_t start = sbp->czstart + sbp->csize + sbp->crefs * sbp->csize;

    /* Create cluster of zeroed-out bytes */
    char zeros[BLOCK_SIZE*sbp->csize];
    memset(zeros, 0x00, sizeof(zeros));

    /* Overwrite all free clusters */
    for (uint32_t i = start; i < sbp->ntotal; i += sbp->csize)
        soWriteRawCluster(i, zeros, sbp->csize);
}
