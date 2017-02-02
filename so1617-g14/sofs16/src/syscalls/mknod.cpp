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
 *  \brief Create a regular file with size 0.
 *
 *  It tries to emulate <em>mknod</em> system call.
 *
 *  \param path path to the file
 *  \param mode type and permissions to be set:
 *                    a bitwise combination of S_IFREG, S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH,
 *                    S_IWOTH, S_IXOTH
 *
 *  \return 0 on success;
 *      -errno in case of error, being errno the system error that better represents the cause of failure
 */
int soMknod(const char *path, mode_t mode)
{
    soProbe(228, "soMknod(\"%s\", %u)\n", path, mode);

    try
    {
        char *xpath = strdupa(path);
        char *bn = strdupa(basename(xpath));
        char *dn = dirname(xpath);

        /* Check if we have kernel memory to copy the strings */
        if (!xpath || !bn)
            throw SOException(ENOMEM, __FUNCTION__);

        /* Check if the path's basename is empty */
        if(strlen(bn) == 0)
            throw SOException(EINVAL, __FUNCTION__);

        /* Check if the path's basename is too long */
        if(strlen(bn) > SOFS16_MAX_NAME)
            throw SOException(ENAMETOOLONG, __FUNCTION__);

        /* Check if the path is too long */
        if(strlen(path) > SOFS16_MAX_PATH)
            throw SOException(ENAMETOOLONG,__FUNCTION__);

        /* Get parent inode handler */
        uint32_t pin; soTraversePath(dn, &pin);
        int pih = iOpen(pin);

        /* Check execute permissions */
        if(!iCheckAccess(pih, X_OK))
            throw SOException(EACCES, __FUNCTION__);

        /* Check if the parent inode is not a directory */
        SOInode *pip = iGetPointer(pih);
        if (!S_ISDIR(pip->mode))
            throw SOException(ENOTDIR, __FUNCTION__);

        /* Check write permissions */
        if(!iCheckAccess(pih, W_OK))
        	throw SOException(EACCES, __FUNCTION__);

        /* Allocate a new inode for the file */
        uint32_t cin; soAllocInode(mode | S_IFREG, &cin);
        int cih = iOpen(cin);

        /* Add dir entry to parent */
        soAddDirEntry(pih, bn, cin);

        /* Increase the file's refcount */
        iIncRefcount(cih);

        return 0;
    }
    catch(SOException & err)
    {
        return -err.en;
    }
}
