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
#include "dealers.h"

#include "probing.h"
#include "exception.h"

/*
 *  \brief Read the value of a symbolic link.
 *
 *  It tries to emulate <em>readlink</em> system call.
 *
 *  \param path path to the symbolic link
 *  \param buff pointer to the buffer where data to be read is to be stored
 *  \param size buffer size in bytes
 *
 *  \return 0 on success;
 *      -errno in case of error, being errno the system error that better represents the cause of failure
 */
int soReadlink(const char *path, char *buff, size_t size)
{
    soProbe(236, "soReadlink(\"%s\", %p, %u)\n", path, buff, size);

    try
    {
        char* xpath = strdupa(path);

        if (strlen(path) > SOFS16_MAX_PATH)
            throw SOException(ENAMETOOLONG,__FUNCTION__);

        uint32_t in; soTraversePath(xpath, &in);
        int ih = iOpen(in);
        SOInode *ip = iGetPointer(ih);

        /* Check if it's not a symlink */
        if((ip->mode & S_IFLNK) != S_IFLNK) {
            iClose(ih);
            throw SOException(EINVAL, __FUNCTION__);
        }

        /* Check if we have write permissions */
        if(!iCheckAccess(ih, R_OK)) {
            iClose(ih);
            throw SOException(EACCES, __FUNCTION__);
        }

        /* Read link BPC bytes at a time */
        uint32_t BPC = soGetBPC();
        uint8_t data[BPC];
        uint32_t lastFcn = (size-1)/BPC;
        uint32_t i;
        for (i = 0; i < lastFcn; i++)
        {
            soReadFileCluster(ih, i, data);
            memcpy(buff + i*BPC, data, BPC);
        }
        memcpy(buff + i*BPC, data, size%BPC);

        iClose(ih);

        return 0;
    }
    catch(SOException & err)
    {
        return -err.en;
    }
}
