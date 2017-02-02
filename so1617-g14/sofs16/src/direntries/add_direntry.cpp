/**
 *  \author Fábio Maia
 *  \tester Fábio Maia
 */
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "direntries.h"

#include "probing.h"
#include "exception.h"
#include "filecluster.h"
#include "dealers.h"
#include "core.h"

#include <errno.h>

/* assumes that:
 * - pih is a valid inode handler of a directory where the user has write access
 * - name can already exist or not (that should be tested)
 * - cin is a valid inodo handler
 */
void soAddDirEntry(int pih, const char *name, uint32_t cin)
{
    soProbe(500, "soAddDirEntry(%d, %s, %u)\n", pih, name, cin);

    /* Check if a dir entry with the given name already exists */
    uint32_t ein; soGetDirEntry(pih, name, &ein);
    if (ein != NULL_REFERENCE)
        throw SOException(EEXIST, __FUNCTION__);

    /* Check if the parent inode is not a directory */
    SOInode *parentInode = iGetPointer(pih);

    uint32_t DPC = soGetDPC();
    uint32_t numDirEntries = parentInode->size / sizeof(SODirEntry);
    uint32_t fcn = numDirEntries / DPC;
    uint32_t idx = numDirEntries % DPC;

    /* Write new dir entry to last cluster */
    SODirEntry dirEntries[DPC];
    soReadFileCluster(pih, fcn, dirEntries);
    strncpy(dirEntries[idx].name, name, SOFS16_MAX_NAME + 1);
    dirEntries[idx].in = cin;
    for (uint32_t i = idx+1; i < DPC; i++)
        dirEntries[i].in = NULL_REFERENCE;
    soWriteFileCluster(pih, fcn, dirEntries);

    /* Update parent inode */
    parentInode->size += sizeof(SODirEntry);
    parentInode->atime = parentInode->ctime = parentInode->mtime = time(NULL);
    iSave(pih);
}
