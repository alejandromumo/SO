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
#include "filecluster.h"

#include "syscalls.h"
#include "probing.h"
#include "exception.h"

/*
 *  \brief Truncate a regular file to a specified length.
 *
 *  It tries to emulate <em>truncate</em> system call.
 *
 *  \param path path to the file
 *  \param length new size for the regular file
 *
 *  \return 0 on success;
 *      -errno in case of error, being errno the system error that better represents the cause of failure
 */
int soTruncate(const char *path, off_t length)
{
    soProbe(231, "soTruncate(\"%s\", %u)\n", path, length);

    try
    {
        /* Check if the length is negative */
        if (length < 0)
            throw SOException(EINVAL, __FUNCTION__);

        /* Check if the length is larger than the maximum file size */
        if (length > soGetMaxFileSize())
            throw SOException(EFBIG, __FUNCTION__);

        char *xpath = strdupa(path);
        uint32_t in; soTraversePath(xpath, &in);
        int ih = iOpen(in);
        SOInode *ip = iGetPointer(ih);

        /* Check if the path is a directory */
        if (S_ISDIR(ip->mode))
            throw SOException(EISDIR, __FUNCTION__);

        /* Check if we need to erase file contents */
        if (length <= ip->size)
        {
            uint32_t BPC = soGetBPC();
            uint32_t fcn = length / BPC;
            uint32_t pos = length % BPC;

            /* Free the exceeding clusters */
            if (pos == 0)
            {
                soFreeFileClusters(ih, fcn);
            }
            else
            {
                soFreeFileClusters(ih, fcn + 1);

                /* Zero out exceeding bytes */
                uint8_t buf[BPC]; soReadFileCluster(ih, fcn, buf);
                memset(buf + pos, 0x00, BPC - pos);
                soWriteFileCluster(ih, fcn, buf);
            }
        }

        ip->size = length;
        iSave(ih);

        return 0;
    }
    catch(SOException & err)
    {
        return -err.en;
    }
}
