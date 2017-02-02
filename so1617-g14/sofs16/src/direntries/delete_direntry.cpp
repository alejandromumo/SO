/**
 *  \author Luís Leira
 *  \tester Luís Leira
 */

#include <string.h> /* added */
#include <sys/stat.h> /* added */
#include <unistd.h> /* added */

#include "direntries.h"

#include "probing.h"
#include "exception.h"
#include "dealers.h" /* added */
#include "filecluster.h" /* added */
#include "core.h" /* added */

#include <errno.h>

void soDeleteDirEntry(int pih, const char *name, uint32_t * cinp)
{
    soProbe(500, "soDeleteDirEntry(%d, %s, %p)\n", pih, name, cinp);

    SOInode *p_pinode = iGetPointer(pih);
    uint32_t DPC = soGetDPC();
    uint32_t n = p_pinode->size/sizeof(SODirEntry);
    uint32_t lastcl = (n-1)/DPC;
    uint32_t lastidx = (n-1)%DPC;
    uint32_t pos, i, cin;
    SODirEntry direntries[DPC], direntries2[DPC];
    SODirEntry last;
    bool found = false; /* Variable that indicates if direntry was found */

    /* Check if exists dir with (that) name */
    soGetDirEntry(pih, name, &cin);
    if(cin == NULL_REFERENCE)
        throw SOException(ENOENT, __FUNCTION__);

    /* Check if exists permission to write and execute */
    if(!iCheckAccess(pih, X_OK))
    	throw SOException(EACCES, __FUNCTION__);

    if(!iCheckAccess(pih, W_OK))
    	throw SOException(EPERM, __FUNCTION__);

    
	/* Get last dir */
    soReadFileCluster(pih, lastcl, direntries2);
    last = direntries2[lastidx];

    /* Checks each cluster */
    for(i = 0; i < n; i++){

        pos = i/DPC;

        if(i % DPC == 0){
	        soReadFileCluster(pih, pos, direntries);
	        
	        /* Checks each dir */
	        for(uint32_t j = 0; j < DPC; j++){

	            if(strcmp(direntries[j].name, name) == 0){
	                if(cinp) *cinp = direntries[j].in;

	                /* if dir isn't the last, replace it for the last */
	                if(!(lastcl == pos && lastidx == i % DPC)){
	                    strncpy(direntries[j].name, last.name, SOFS16_MAX_NAME+1);
	                    direntries[j].in = last.in;

	                    /* if last is in another cluster */
		                if(lastcl != pos){
		                	/* clean last one */
		                	strncpy(direntries2[lastidx].name, "\0", SOFS16_MAX_NAME+1);
		                	direntries2[lastidx].in = NULL_REFERENCE;
		                	soWriteFileCluster(pih, lastcl, direntries2);
		                }else{
		                	/* clean last one */
		                	strncpy(direntries[lastidx].name, "\0", SOFS16_MAX_NAME+1);
		                	direntries[lastidx].in = NULL_REFERENCE;
		                }
	                }else{
	                	/* clean last one */
	                	strncpy(direntries[lastidx].name, "\0", SOFS16_MAX_NAME+1);
	                	direntries[lastidx].in = NULL_REFERENCE;
	                }

	                p_pinode->size -= sizeof(SODirEntry);

	                found = true;
	                break;
	            }
	        }
	    }
        
        /* when it is found, write and break */
        if(found){
            soWriteFileCluster(pih, pos, direntries);
            break;
        }
    }
    
    /* free cluster if it hasn't more direntries */
    if(lastidx == 0){
        soFreeFileClusters(pih ,lastcl);
    }

}
