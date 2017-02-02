/**
 *  \author Francisco Cunha
 *  \tester Francisco Cunha
 */

#include "freelists.h"

#include "probing.h"
#include "exception.h"

#include "core.h"
#include "itdealer.h"
#include "sbdealer.h"

#include <unistd.h> /* added */
#include <sys/stat.h> /* added */

#include <errno.h>
#include <inttypes.h>

/*
 * Dictates to be obeyed by the implementation:
 * - parameter in must be validated,
 *      throwing a proper error if necessary
  param in number of the inode to be freed
 */
void soFreeInode(uint32_t in)
{
    soProbe(712, "soFreeInode (%u)\n", in);

    SOSuperBlock *sbp = sbGetPointer();

    /* Check if the inode number is out of range */
    if(in > sbp->itotal || in == 0)
        throw SOException(EINVAL, __FUNCTION__);

    /* Open freed inode */
    int fih = iOpen(in);
    SOInode *freedInode = iGetPointer(fih);
    
    /* Silently return if the inode is already free */ 
    if ((freedInode->mode & INODE_FREE) == INODE_FREE)
    {
        iClose(fih);
        return;
    }

    /* Check if we need to consider a tail inode or not */
    if (sbp->ifree == 0)
    {
        /* Adjust links when there isn't a tail inode */
        sbp->ihead = sbp->itail = in;
        freedInode->next = NULL_REFERENCE;
    }
    else
    {
        /* Open tail inode */
        int tih = iOpen(sbp->itail);
        SOInode *tailInode = iGetPointer(tih);

        /* Adjust links when there's a tail inode */
        sbp->itail = in;
        tailInode->next = in;
        freedInode->next = NULL_REFERENCE;

        /* Save and close tail inode */
        iSave(tih);
        iClose(tih);
    }

    /* Mark freed inode as free */
    freedInode->mode |= INODE_FREE;
    sbp->ifree++;

    /* Save and close freed inode */
    iSave(fih);
    iClose(fih);

    /* Save the superblock */
    sbSave();
}
