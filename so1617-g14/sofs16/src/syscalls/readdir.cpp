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
#include "filecluster.h" /* added */
#include "dealers.h" 
#include "core.h"

/*
 *  \brief Read a directory entry from a directory.
 *
 *  It tries to emulate <em>getdents</em> system call, but it reads a single directory entry at a time.
 *
 *  Only the field <em>name</em> is read.
 *
 *  \remark The returned value is the number of bytes read from the directory in order to get the next in use
 *          directory entry. 
 *          The point is that the system (through FUSE) uses the returned value to update file position.
 *
 *  \param path path to the file
 *  \param buff pointer to the buffer where data to be read is to be stored
 *  \param pos starting [byte] position in the file data continuum where data is to be read from
 *
 *  \return 0 on success; 
 *      -errno in case of error, being errno the system error that better represents the cause of failure
 */
int soReaddir(const char *path, void *buff, int32_t pos)
{
    
    soProbe(234, "soReaddir(\"%s\", %p, %u)\n", path, buff, pos);

    try
    {
        uint32_t inode_parent1;
        uint32_t BPC = soGetBPC();
        uint32_t DPC = soGetDPC();
        uint32_t directory;  
        SODirEntry data[DPC];
        
        char* xpath = strdupa(path);
        char* bn = basename(strdupa(xpath));
        char* dn = dirname(xpath);

        soTraversePath(dn, &inode_parent1);
        int ih = iOpen(inode_parent1);

        int cluster = pos / sizeof(SODirEntry) / BPC; // guedes
        int idx = pos / sizeof(SODirEntry) % BPC; // guedes

        soGetDirEntry(ih,bn,&directory);

        //  check if directory exists
        if (directory != NULL_REFERENCE)
        {
           throw SOException(ENOENT, __FUNCTION__);
        }

        // Check if does not exists permission to read 
        if (!iCheckAccess(ih, R_OK))
        {
            iSave(ih);
            iClose(ih);
            throw SOException(EPERM, __FUNCTION__);
        }
        
        soReadFileCluster(ih, cluster, data);

        if (strcmp(data[idx].name, "") == 0)
        {
            return 0;
        }

        memcpy(buff,&data[idx].name,sizeof(SODirEntry));

        pos += sizeof(SODirEntry);

        iSave(ih);
        iClose(ih);
        return sizeof(SODirEntry);
        
    }
    catch(SOException & err)
    {
        return -err.en;
    }

}
