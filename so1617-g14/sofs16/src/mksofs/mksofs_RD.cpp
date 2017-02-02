#include "mksofs.h"

#include "core.h"
#include "superblock.h"
#include "exception.h"

#include "direntry.h"

#include <errno.h>
#include <string.h>

/*
 * filling in the contents of the root directory:
         the first 2 entries are filled in with "." and ".." references
         the other entries are empty
 */
void fillInRootDir(SOSuperBlock * sbp)
{
    uint32_t DPC = DPB * sbp->csize;
    SODirEntry dirEntries[DPC];
    memset(dirEntries, 0x00, DPC * sizeof(SODirEntry));

    /* Set name and inode of the . dir entry */
    strncpy(dirEntries[0].name, ".", 1+1);
    dirEntries[0].in = 0;

    /* Set name and inode of the .. dir entry */
    strncpy(dirEntries[1].name, "..", 2+1);
    dirEntries[1].in = 0;

    /* Set inode of remaining dir entries */
    for(uint32_t i = 2; i < DPC; i++)
    {
        dirEntries[i].in = NULL_REFERENCE;
    }

    /* Write dir entries to the first cluster */
    soWriteRawCluster(sbp->czstart, &dirEntries, sbp->csize);
}
