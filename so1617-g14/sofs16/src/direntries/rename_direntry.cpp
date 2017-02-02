/**
 *  \author Manuel Costa
 *  \tester Manuel Costa
 */

#include "direntries.h"
#include "dealers.h"
#include "sys/stat.h"
#include "string.h"
#include "core.h"
#include "unistd.h"
#include "errno.h"
#include "filecluster.h"

#include "probing.h"
#include "exception.h"

#include <errno.h>

void soRenameDirEntry(int pih, const char *name, const char *newName)
{
    soProbe(500, "soRenameDirEntry(%d, %s, %s)\n", pih, name, newName);

    /* Check if target dir entry exists */
    uint32_t cin; soGetDirEntry(pih, name, &cin);
    if(cin == NULL_REFERENCE)
        throw SOException(ENOENT, __FUNCTION__);

    /* Check if new name exceeds size limit */
    if(strlen(newName) > SOFS16_MAX_NAME)
        throw SOException(ENAMETOOLONG, __FUNCTION__);

    /* Return early if the strings are the same */
    if(strcmp(name, newName) == 0)
        return;

    /* Check if parent inode is a directory */
    SOInode *pip = iGetPointer(pih);
    if(!S_ISDIR(pip->mode))
        throw SOException(ENOTDIR, __FUNCTION__);

    /* Check if we have read and write permissions on the parent inode */
    if(iCheckAccess(pih, R_OK | W_OK) == false)
        throw SOException(EACCES, __FUNCTION__);

    uint32_t DPC = soGetDPC();
    SODirEntry direntries[DPC];
    uint32_t fcn = 0, foundfcn, foundIdx;
    soReadFileCluster(pih, fcn, direntries);
    bool exists = false;

    for(uint32_t i = 0; i < DPC; i++)
    {
        if(direntries[i].in == NULL_REFERENCE)
            break;

        if(direntries[i].in == cin)
        {
            foundfcn = fcn;
            foundIdx = i;
        }

        /* If a direntry has the same name as newName and it's not the one we're trying to rename, then it's duplicated */
        if(strcmp(direntries[i].name, newName) == 0 && direntries[i].in != cin)
        {
            exists = true;
            break;
        }

        if(i == DPC-1)
        {
            fcn++;
            i = 0;
            soReadFileCluster(pih, fcn, direntries);
        }
    }

    /* If there isn't another direntry with newName and name is a direntry, we rename that direntry */
    if(exists == false)
    {
        soReadFileCluster(pih, foundfcn, direntries);
        strncpy(direntries[foundIdx].name, newName, SOFS16_MAX_NAME + 1);
        soWriteFileCluster(pih, foundfcn, direntries);
    }
    else
    {
        throw SOException(EEXIST, __FUNCTION__);
    }

    iSave(pih);
}