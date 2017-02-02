/*
 * \author Inês Moreira
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

#include "syscalls.h"
#include "direntries.h"
#include "filecluster.h"
#include "freelists.h"
#include "dealers.h"
#include "core.h"

#include "probing.h"
#include "exception.h"

/*
 *  \brief Creates a symbolic link which contains the given path.
 *
 *  It tries to emulate <em>symlink</em> system call.
 *
 *  \remark The permissions set for the symbolic link should have read (r), write (w) and execution (x) permissions for
 *          both <em>user</em>, <em>group</em> and <em>other</em>.
 *
 *  \param effPath path to be stored in the symbolic link file
 *  \param path path to the symbolic link to be created
 *
 *  \return 0 on success;
 *      -errno in case of error, being errno the system error that better represents the cause of failure
 */
int soSymlink(const char *effPath, const char *path)
{
    soProbe(235, "soSymlink(\"%s\", \"%s\")\n", effPath, path);

    try
    {
        char *xeffPath = strdupa(effPath);

        char* xpath = strdupa(path);
        char* sbn = strdupa(basename(xpath));
        char* sdn = dirname(xpath);

        if (strlen(effPath) == 0)
            throw SOException(EINVAL, __FUNCTION__);

        if (strlen(effPath) > SOFS16_MAX_PATH)
            throw SOException(ENAMETOOLONG, __FUNCTION__);

        if (strlen(path) > SOFS16_MAX_PATH)
            throw SOException(ENAMETOOLONG, __FUNCTION__);

        if (strlen(sbn) > SOFS16_MAX_NAME)
            throw SOException(ENAMETOOLONG, __FUNCTION__);

        /* Get the symlink's parent inode number */
        uint32_t spin; soTraversePath(sdn, &spin);
        uint32_t spih = iOpen(spin);

        /* Check if we have write permissions on the symlink's parent inode */
        if(!iCheckAccess(spih, W_OK)) {
            iClose(spih);
            throw SOException(EACCES, __FUNCTION__);
        }

        /* Allocate an inode for the symlink and get its child inode number */
        uint32_t scin; soAllocInode(S_IFLNK | S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH, &scin);
        uint32_t scih = iOpen(scin);
        SOInode *scip = iGetPointer(scih);

        /* Add the dir entry and write the path of the symlink */
        soAddDirEntry(spih, sbn, scin);

        uint32_t BPC = soGetBPC();
        uint32_t lastFcn = (strlen(xeffPath)-1)/BPC;
        for (uint32_t i = 0; i <= lastFcn; i++)
            soWriteFileCluster(scih, i, xeffPath + i*BPC);

        /* Adjust refcount and size in bytes of the symlink */

        iIncRefcount(scih);
        scip->size = strlen(effPath);
        iSave(scih);

        iClose(scih);
        iClose(spih);

        return 0;
    }
    catch(SOException & err)
    {
        return -err.en;
    }
}
