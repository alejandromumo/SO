/*
 * \author ...
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
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
#include <direntries.h>
#include <inode.h>
#include <dealers.h>
#include <core.h>

#include "syscalls.h"

#include "probing.h"
#include "exception.h"

/*
 *  \brief Make a new link to a file.
 *
 *  It tries to emulate <em>link</em> system call.
 *
 *  \param path path to an existing file
 *  \param newPath new path to the same file
 *
 *  \return 0 on success; 
 *      -errno in case of error, being errno the system error that better represents the cause of failure
 */
int soLink(const char *path, const char *newPath)
{
    soProbe(225, "soLink(\"%s\", \"%s\")\n", path, newPath);

      /*
        ERRORS
           (V) EACCES Write  access  to the directory containing newpath is denied, or
                  search permission is denied for one of the  directories  in  the
                  path  prefix  of  oldpath  or  newpath.   (See also path_resolu‐
                  tion(7).)

           EDQUOT The user's quota of disk  blocks  on  the  filesystem  has  been
                  exhausted.

           (V) EEXIST newpath already exists.

           EFAULT oldpath or newpath points outside your accessible address space.

           EIO    An I/O error occurred.

           ELOOP  Too many symbolic links were encountered in resolving oldpath or
              newpath.

           EMLINK The file referred to by oldpath already has the  maximum  number
                  of links to it.

           (V) ENAMETOOLONG
                  oldpath or newpath was too long.

           ENOENT A directory component in oldpath or newpath does not exist or is
                  a dangling symbolic link.

           ENOMEM Insufficient kernel memory was available.

           ENOSPC The device containing the file has no room for the new directory
                  entry.

           (V) ENOTDIR
                  A component used as a directory in oldpath or newpath is not, in
                  fact, a directory.

           (V) EPERM  oldpath is a directory.

           EPERM  The filesystem containing oldpath and newpath does  not  support
                  the creation of hard links.

           EPERM (since Linux 3.6)
                  The  caller  does  not  have permission to create a hard link to
                  this   file   (see   the   description   of    /proc/sys/fs/pro‐
                  tected_hardlink in proc(5)).

           EROFS  The file is on a read-only filesystem.

           EXDEV  oldpath  and  newpath  are  not  on the same mounted filesystem.
                  (Linux permits a filesystem to be mounted  at  multiple  points,
                  but  link() does not work across different mount points, even if
                  the same filesystem is mounted on both.
      */

    try
    {

        char* xpath = strdupa(newPath); 
        char* bn = strdupa(basename(xpath)); 
        char* dn = dirname(xpath); 
        uint32_t path_inp,newpath_inp;

        
        /* Check name from newpath */
        if(strlen(bn) == 0 || bn == NULL)
            throw SOException(EINVAL, __FUNCTION__);
        /* Check if name from newpath exceeds max len */
        if(strlen(bn) > SOFS16_MAX_NAME)
            throw SOException(ENAMETOOLONG, __FUNCTION__);

        
        /* I-nodes (Traverse)*/
        
        /* New path */
        soTraversePath(dn, &newpath_inp);
        /* Original path */
        char* originalpath = strdupa(path);
        soTraversePath(originalpath,&path_inp);

        
        /* Handlers */
        
        /* New i-node handler */ 
        int icopy_handler = iOpen(newpath_inp);
        /* Original i-node handler */
        int ioriginal_handler = iOpen(path_inp);
        
        
        /* Verifications */
        
        /* Check if original file is not a directory */
        SOInode *original_inode = iGetPointer(ioriginal_handler);
        if(S_ISDIR(original_inode -> mode)) throw SOException(EPERM,__FUNCTION__);
        /* Check if original file is readable by user */
        if(iCheckAccess(ioriginal_handler, R_OK) == false)
            throw SOException(EACCES,__FUNCTION__);
        /* Check if new dir is readable and writable by user */
        if(iCheckAccess(icopy_handler, R_OK | W_OK) == false)
          throw SOException(EACCES,__FUNCTION__);
        
        
        /* Updates*/
        
        /* Update number of links from original file i-node */
        iIncRefcount(ioriginal_handler);
        /* Save updated original i-node */
        iSave(ioriginal_handler);
        /* Add DirEntry in newpath i-node */
        soAddDirEntry(icopy_handler,bn,path_inp);

        return 0;
    }
    catch(SOException & err)
    {
        return -err.en;
    }

}
