/*
 * \author ...
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
#include <dealers.h>
#include <direntries.h>
#include <freelists.h>
#include <filecluster.h>

#include "syscalls.h"

#include "probing.h"
#include "exception.h"

/*
 *  \brief Delete a link to a file from a directory and possibly the file it refers to from the file system.
 *
 *  It tries to emulate <em>unlink</em> system call.
 *
 *  \param path path to the file to be deleted
 *
 *  \return 0 on success; 
 *      -errno in case of error, being errno the system error that better represents the cause of failure
 */
int soUnlink(const char *path)
{
    soProbe(226, "soUnlink(\"%s\")\n", path);

     /*
       EACCES Write access to the directory containing pathname is not allowed
              for  the  process's  effective UID, or one of the directories in
              pathname did not allow search permission.  (See also  path_reso‐
              lution(7).)

       EBUSY  The file pathname cannot be unlinked because it is being used by
              the system or another process; for example, it is a mount  point
              or the NFS client software created it to represent an active but
              otherwise nameless inode ("NFS silly renamed").

       EFAULT pathname points outside your accessible address space.

       EIO    An I/O error occurred.

       EISDIR pathname refers to a directory.  (This is  the  non-POSIX  value
              returned by Linux since 2.1.132.)

       ELOOP  Too  many  symbolic  links were encountered in translating path‐
              name.

       ENAMETOOLONG
              pathname was too long.

       ENOENT A component in pathname does not exist or is a dangling symbolic
              link, or pathname is empty.

       ENOMEM Insufficient kernel memory was available.

       ENOTDIR
              A  component  used as a directory in pathname is not, in fact, a
              directory.

       EPERM  The system does not allow unlinking of directories, or unlinking
              of  directories  requires  privileges  that  the calling process
              doesn't have.  (This is the POSIX prescribed  error  return;  as
              noted above, Linux returns EISDIR for this case.)

               EPERM (Linux only)
              The filesystem does not allow unlinking of files.

       EPERM or EACCES
              The  directory  containing pathname has the sticky bit (S_ISVTX)
              set and the process's effective UID is neither the  UID  of  the
              file  to be deleted nor that of the directory containing it, and
              the  process  is  not  privileged  (Linux:  does  not  have  the
              CAP_FOWNER capability).

       EROFS  pathname refers to a file on a read-only filesystem.

*/ 

    try
    {
        char* xpath = strdupa(path); 
        char* bn = strdupa(basename(xpath)); 
        char* dn = dirname(xpath); 
        uint32_t dir_inp,file_inp;

        /* Check name from path */
        if(strlen(bn) == 0 || bn == NULL)
            throw SOException(EINVAL, __FUNCTION__);
        /* Check if name from path exceeds max len */
        if(strlen(bn) > SOFS16_MAX_NAME)
            throw SOException(ENAMETOOLONG, __FUNCTION__);


        /* Dir I-node (Traverse) */

        soTraversePath(dn,&dir_inp);
        //soTraversePath(bn,&file_inp);


        /* Handlers */

        int dir_inode_handler = iOpen(dir_inp);
        soGetDirEntry(dir_inp,bn,&file_inp);
        int file_inode_handler = iOpen(file_inp);


        /* Verifications */

        /* Check if file is not a directory */
        SOInode *file_inode = iGetPointer(file_inode_handler);
        if(S_ISDIR(file_inode -> mode)) throw SOException(EPERM,__FUNCTION__);
        /* Check if file is readable and writable by user */
        if(iCheckAccess(file_inode_handler, R_OK | W_OK) == false)
            throw SOException(EACCES,__FUNCTION__);


        /* Updates */

        /* Update RefCount */
        iDecRefcount(file_inode_handler);
        if(file_inode -> refcount == 0){
         soFreeFileClusters(file_inode_handler,0);
         soFreeInode(file_inp);
        }
        /* Delete DirEntry */
        uint32_t cinp;
        soDeleteDirEntry(dir_inode_handler,bn,&cinp);
        iSave(dir_inode_handler);
        return 0;
    }
    catch(SOException & err)
    {
        return -err.en;
    }
}
