/**
 *  \author Fábio Maia
 *  \tester Fábio Maia
 */

#include "freelists.h"

#include "core.h"
#include "probing.h"
#include "exception.h"

#include "itdealer.h"
#include "sbdealer.h"

#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

#include <errno.h>
#include <inttypes.h>

/*
 * Dictates to be obeyed by the implementation:
 * - error ENOSPC should be thrown if there is no free inodes
 * - the allocated inode must be properly initialized
 */
void soAllocInode(uint32_t type, uint32_t * inp)
{
    soProbe(711, "soAllocInode(%u, %p)\n", type, inp);

    SOSuperBlock *sbp = sbGetPointer();

    /* There must be free inodes */
    if (sbp->ifree == 0)
        throw SOException(ENOSPC, __FUNCTION__);

    /* The number of the allocated inode is the head of the list of free inodes */
    if(inp) *inp = sbp->ihead;

    /* Open head inode */
    int ih = iOpen(sbp->ihead);
    SOInode *inode = iGetPointer(ih);

    /* Update the number of free inodes and the head and tail on the superblock */
    sbp->ifree--;
    if(sbp->ifree == 0) sbp->ihead = sbp->itail = NULL_REFERENCE;
    else sbp->ihead = inode->next;
    sbSave();

    /* Initialize inode */
    memset(inode, 0x00, sizeof(SOInode));
    inode->mode = type;
    inode->owner = getuid();
    inode->group = getgid();
    inode->atime = inode->ctime = inode->mtime = time(NULL);
    memset(inode->d, NULL_REFERENCE, N_DIRECT*sizeof(uint32_t));
    memset(inode->i1, NULL_REFERENCE, N_INDIRECT*sizeof(uint32_t));
    inode->i2 = NULL_REFERENCE;

    /* Save and close inode */
    iSave(ih);
    iClose(ih);
}
