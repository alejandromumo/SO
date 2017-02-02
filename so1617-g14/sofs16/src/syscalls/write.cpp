/*
 * \author Luís Leira
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
#include "dealers.h" /* added */
#include "core.h" /* added */
#include "freelists.h" /* added */
#include "filecluster.h" /* added */
#include "direntries.h" /* added */

/*
 *  \brief Write data into an open regular file.
 *
 *  It tries to emulate <em>write</em> system call.
 *
 *  \param path path to the file
 *  \param buff pointer to the buffer where data to be written is stored
 *  \param count number of bytes to be written
 *  \param pos starting [byte] position in the file data continuum where data is to be written into
 *
 *  \return 0 on success; 
 *      -errno in case of error, being errno the system error that better represents the cause of failure
 */
int soWrite(const char *path, void *buff, uint32_t count, int32_t pos)
{
    soProbe(230, "soWrite(\"%s\", %p, %u, %u)\n", path, buff, count, pos);

    try
    {
        /* Check if pos is negative */
        if(pos < 0)
            throw SOException(EINVAL, __FUNCTION__);

        uint32_t cinp, BPC = soGetBPC();
        uint32_t fcn = pos/BPC, idx = pos%BPC;
        uint32_t fcn_last, idx_last, cih, nbytes, bytestowrite;
        SOInode *inode;
        char data[BPC];

        char* xpath = strdupa(path);

        /* Get the inode index */
        soTraversePath(xpath, &cinp);

        /* Get inode handler */
    	cih = iOpen(cinp);

        /* Check access */
        if(!iCheckAccess(cih, W_OK))
            throw SOException(EPERM, __FUNCTION__);

        /* Get pointer */
        inode = iGetPointer(cih);

    	nbytes = 0;
        bytestowrite = count;
        fcn_last = (pos+count-1)/BPC;
        idx_last = (pos+count-1)%BPC;

        for(uint32_t i = fcn; i <= fcn_last; i++){

            /* read cluster */
            soReadFileCluster(cih, i, data);
            
            /* if it is the first cluster */
            if(i == fcn){
                if(count+idx <= BPC){
                    memcpy(data+idx, buff, idx_last-idx+1);
                    nbytes += idx_last-idx+1;
                }else{
                    memcpy(data+idx, buff, BPC-idx);
                    nbytes += (BPC-idx);
                }
            /* if it is the last cluster but not the first */
            }else if(i == fcn_last){
                memcpy(data, (uint8_t *)buff+nbytes, bytestowrite-nbytes);
                nbytes = bytestowrite;
            /* intermediate cluster */
            }else{
                memcpy(data, (uint8_t *)buff+nbytes, BPC);
                nbytes += BPC;
            }

            soWriteFileCluster(cih, i, data);

        }

        inode->size += nbytes;

        iSave(cih);
        iClose(cih);

    	return nbytes;
    }
    catch(SOException & err)
    {
        return -err.en;
    }
}
