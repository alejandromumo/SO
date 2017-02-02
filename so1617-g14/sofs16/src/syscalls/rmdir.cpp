/*
 * \author Fábio Maia
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <time.h>
#include <utime.h>
#include <libgen.h>
#include <string.h>

#include "dealers.h"
#include "direntries.h"
#include "freelists.h"

#include "syscalls.h"
#include "probing.h"
#include "exception.h"

/*
 *  \brief Delete a directory.
 *
 *  It tries to emulate <em>rmdir</em> system call.
 *
 *  The directory should be empty, ie. only containing the '.' and '..' entries.
 *
 *  \param path path to the directory to be deleted
 *
 *  \return 0 on success;
 *      -errno in case of error, being errno the system error that better represents the cause of failure
 */
int soRmdir(const char *path)
{
    soProbe(233, "soRmdir(\"%s\")\n", path);

    try
    {
        char *pathcopy = strdupa(path);
        char *xpath = strdupa(path);
        char *bn = strdupa(basename(xpath));
        char *dn = dirname(xpath);

        /* Check if we have kernel memory to copy the strings */
        if (!xpath || !bn)
            throw SOException(ENOMEM, __FUNCTION__);

        /* Check if name is null */
        if(strlen(bn) == 0)
            throw SOException(EINVAL, __FUNCTION__);

        /* Check if the path's basename is too long */
        if(strlen(bn) > SOFS16_MAX_NAME)
            throw SOException(ENAMETOOLONG, __FUNCTION__);

        /* Check if the path is too long */
        if(strlen(path) > SOFS16_MAX_PATH)
            throw SOException(ENAMETOOLONG,__FUNCTION__);

        /* Get child inode handler */
        uint32_t cin; soTraversePath(pathcopy, &cin);
        int cih = iOpen(cin);
        SOInode *childInode = iGetPointer(cih);

        /* Check if the directory is empty */
        if (childInode->size != 2*sizeof(SODirEntry))
            throw SOException(ENOTEMPTY, __FUNCTION__);

        /* Get parent inode handler */
        uint32_t pin; soTraversePath(dn, &pin);
        int pih = iOpen(pin);

        /* Check execute permissions */
        if(!iCheckAccess(pih, X_OK))
            throw SOException(EACCES, __FUNCTION__);

        /* Delete dir entries from parent */
        soDeleteDirEntry(pih, bn, NULL);
        iDecRefcount(pih);

        /* Delete dir entries from child */
        soDeleteDirEntry(cih, ".", NULL);
        iDecRefcount(cih);
        soDeleteDirEntry(cih, "..", NULL);
        iDecRefcount(cih);

        /* Free the child inode */
        soFreeInode(cin);

        iClose(pih);
        iClose(cih);
        return 0;
    }
    catch(SOException & err)
    {
        return -err.en;
    }
}
