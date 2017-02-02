/*
 * \author Francisco Cunha
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
#include "probing.h" 
#include "exception.h"
#include "direntries.h" 
#include "dealers.h" 
#include "core.h"


/*
 *  \brief Change the name or the location of a file in the directory hierarchy of the file system.
 *
 *  It tries to emulate <em>rename</em> system call.
 *
 *  \param path path to an existing file
 *  \param newPath new path to the same file in replacement of the old one
 *
 *  \return 0 on success; 
 *      -errno in case of error, being errno the system error that better represents the cause of failure
 */

#include <stdio.h>
#include <fcntl.h>

int soRename(const char *path, const char *newPath)
{
    soProbe(227, "soRename(\"%s\", \"%s\")\n", path, newPath);

    try
    {
        uint32_t inode_parent1, inode_parent2;
        uint32_t renamed; // renamed inode
        uint32_t deleted; // deleted inode

        char* xpath = strdupa(path);
        char* bn = strdupa(basename(xpath));
        char* dn = dirname(xpath);

        char* Nxpath = strdupa(newPath); // New xpath
        char* Nbn = strdupa(basename(Nxpath)); // New basename
        char* Ndn = dirname(Nxpath); // New dirname 

        // Maximum number of bytes in a pathname, including the terminating null character.
        if (strlen(path) > SOFS16_MAX_NAME || strlen(newPath) > SOFS16_MAX_NAME)
        {
            throw SOException(ENAMETOOLONG, "Maximum number of path bytes exceeded");    
        }

        if (strlen(bn) > SOFS16_MAX_NAME || strlen(Nbn) > SOFS16_MAX_NAME)
        {
            throw SOException(ENAMETOOLONG, "Maximum number of basename bytes exceeded");    
        }

        // if newPath already exists it will be atomically replaced
        int equal = strcmp(strdupa(path), strdupa(newPath));

        if (equal == 0)
        {
            // Sucess
            return 0;
        }
        
        // Get inode parent1
        soTraversePath(strdupa(dn), &inode_parent1); // Get the inode associated to the given path
        int ih1 = iOpen(inode_parent1);

        // Get inode parent2
        soTraversePath(strdupa(Ndn), &inode_parent2); // Get the inode associated to the given path
        int ih2 = iOpen(inode_parent2);

        // Get an entry given a name
        soGetDirEntry(ih2, Nbn, &deleted);

        // if entry exists with the same name, it will be deleted
        if (deleted != NULL_REFERENCE)
        {
            soDeleteDirEntry(ih2, Nbn, &deleted);
        }

        /* Check if exists permission to write */
        if( !iCheckAccess(ih1, W_OK) || !iCheckAccess(ih2, W_OK))
        {   
            iSave(ih1);
            iSave(ih2);
            iClose(ih1);
            iClose(ih2);
            throw SOException(EACCES, __FUNCTION__);
        }

        soDeleteDirEntry(ih1, strdupa(bn), &renamed); // Remove an entry from a parent directory
        soAddDirEntry(ih2, Nbn, renamed); // Add a new entry to the parent directory

        uint32_t ih3 = iOpen(renamed); // open inode

        SOInode* inp = iGetPointer(ih3); // get pointer to an open inode

        // if is directory, must change the . and .. entries
        if ((inp -> mode & S_IFDIR) == S_IFDIR)
        {   
            /* Check if exists permission to write */
            if (iCheckAccess(ih3,W_OK) != 0)
            {
                iSave(ih1);
                iSave(ih2);
                iSave(ih3);
                iClose(ih1);
                iClose(ih2);
                iClose(ih3);
                throw SOException(EACCES, __FUNCTION__);
            }
            soDeleteDirEntry(ih3,"..", &deleted);
            soAddDirEntry(ih3, "..", inode_parent2);
            iIncRefcount(ih2); // increment, if possible, reference count of inode handler 2
            iDecRefcount(ih1); // decrement, if possible, reference count of inode handler 1
        }

        iSave(ih1);
        iSave(ih2);
        iSave(ih3);
        iClose(ih1);
        iClose(ih2);
        iClose(ih3);

        return 0;
    }
    catch(SOException & err)
    {
        return -err.en;
    }
}

/* 
        
    http://www.tutorialspoint.com/unix_system_calls/rename.htm

    if newPath already exists it will be atomically replaced, so that there 
    is no point at which another process attempting to acess newPath will find it missing
    However, there will probably be a window in both which old path and new path refer to the file being renamed

    If oldpath and newpath are existing hard links referring to the same
    file, then rename() does nothing, and returns a success status.

    If newpath exists but the operation fails for some reason, rename()
    guarantees to leave an instance of newpath in place.

    oldpath can specify a directory.  In this case, newpath must either
    not exist, or it must specify an empty directory.

    If oldpath refers to a symbolic link, the link is renamed; if newpath
    refers to a symbolic link, the link will be overwritten.

*/