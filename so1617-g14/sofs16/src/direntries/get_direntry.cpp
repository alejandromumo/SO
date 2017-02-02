/**
 *  \author Francisco Cunha
 *  \tester Francisco Cunha
 */

#include <string.h> /* added */
#include <sys/stat.h> /* added */
#include <unistd.h> /* added */

#include "direntries.h"

#include "probing.h"
#include "exception.h"

#include <errno.h>

#include <itdealer.h>
#include "dealers.h"
#include "filecluster.h"
#include "core.h"

/*
param pih inode handler of the parent directory
param name the name entry to be searched for
param cinp Pointer to the variable where inode number associated to the entry is to be stored
 */
void soGetDirEntry(int pih, const char *name, uint32_t * cinp)
{
    soProbe(500, "soGetDirEntry(%d, %s, %p)\n", pih, name, cinp);

    SOInode *pip = iGetPointer(pih);
    uint32_t DPC = soGetDPC();
    uint32_t numDirEntries = pip->size/sizeof(SODirEntry);
    SODirEntry dirEntries[DPC];

    /* Check if we have permission to execute on the parent inode */
    if(!iCheckAccess(pih, X_OK))
        throw SOException(EACCES, __FUNCTION__);

    /* Check if parent inode is a directory */
    if(!S_ISDIR(pip->mode))
        throw SOException(ENOTDIR, __FUNCTION__);

    /* Search the dirEntries */
    for (uint32_t i = 0; i < numDirEntries; i++)
    {
        uint32_t fcn = i / DPC;
        uint32_t idx = i % DPC;

        if (idx == 0)
        {
            soReadFileCluster(pih, fcn, dirEntries);

            for (uint32_t j = 0; j < DPC; j++)
            {
                if (strcmp(dirEntries[j].name, name) == 0)
                {
                    if (cinp) *cinp = dirEntries[j].in;
                    return;
                }
            }
        }
    }

    /* When the path is not found, it should return a null reference */
    if(cinp) *cinp = NULL_REFERENCE;
}
