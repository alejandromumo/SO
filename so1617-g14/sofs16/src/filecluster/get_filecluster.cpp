/**
 *  \author Inês Moreira
 *  \tester Inês Moreira
 */

#include "filecluster.h"

#include "probing.h"
#include "exception.h"
#include "inode.h"
#include "core.h"

#include <errno.h>
#include <stdint.h>
#include <czdealer.h>
#include <itdealer.h>

static void soGetIndirectFileCluster(SOInode * ip, uint32_t fcn, uint32_t * cnp);
static void soGetDoubleIndirectFileCluster(SOInode * ip, uint32_t fcn, uint32_t * cnp);

void soGetFileCluster(int ih, uint32_t fcn, uint32_t * cnp)
{
    soProbe(600, "soGetFileCluster(%d, %u, %p)\n", ih, fcn, cnp);

    uint32_t RPC = soGetRPC();

    /* Open the file's inode */
    SOInode *inode = iGetPointer(ih);

    /* fcn is out of bounds */
    if(fcn < 0 || fcn >= N_DIRECT + N_INDIRECT*RPC + RPC*RPC)
        throw SOException(ENOSYS, __FUNCTION__);

    /* fcn is in d */
    if(fcn < N_DIRECT)
        *cnp = inode->d[fcn];
    /* fcn is in i1 */
    else if (fcn < N_DIRECT + N_INDIRECT*RPC)
        soGetIndirectFileCluster(inode, fcn, cnp);
    /* fcn is in i2 */
    else
        soGetDoubleIndirectFileCluster(inode, fcn, cnp);
}

static void soGetIndirectFileCluster(SOInode * ip, uint32_t afcn, uint32_t * cnp)
{
    soProbe(600, "soGetIndirectFileCluster(%p, %u, %p)\n", ip, afcn, cnp);

    /* get references per cluster */
    uint32_t RPC = soGetRPC();
    /* get i1 correct index */
    uint32_t clusterIndex = (afcn - N_DIRECT)/RPC;
    /* check if the position is null */
    if(ip->i1[clusterIndex] == NULL_REFERENCE)
    {
        *cnp = NULL_REFERENCE;
    }
    else
    {
        /* create array for the references inside the cluster */
        uint32_t refs[RPC];
        /* get index of the reference inside of the cluster */
        uint32_t referenceIndex = (afcn - N_DIRECT)%RPC;
        /* read the cluster inside of i1[clusterIndex] and collect data to ref */
        soReadCluster(ip->i1[clusterIndex], &refs);
        *cnp = refs[referenceIndex];
    }
}

static void soGetDoubleIndirectFileCluster(SOInode * ip, uint32_t afcn, uint32_t * cnp)
{
    soProbe(600, "soGetDoubleIndirectFileCluster(%p, %u, %p)\n", ip, afcn, cnp);

    /* check if the position is null */
    if(ip->i2 == NULL_REFERENCE)
    {
        *cnp = NULL_REFERENCE;
    }
    else
    {
        /* create array for the references inside of the first cluster */
        uint32_t RPC = soGetRPC();
        uint32_t refs[RPC];
        soReadCluster(ip->i2, &refs);

        /* get the position of the reference that we want */
        uint32_t clusterIndex = (afcn - N_DIRECT - N_INDIRECT*RPC)/RPC;
        if(refs[clusterIndex] == NULL_REFERENCE)
        {
            *cnp = NULL_REFERENCE;
        }
        else
        {
            /* Array of references inside of the second cluster */
            uint32_t refsOfRefs[RPC];
            /*  corresponding cluster inside of the first cluster */
            soReadCluster(refs[clusterIndex], &refsOfRefs);

            /* Get the referenceIndex */
            uint32_t referenceIndex = (afcn - N_DIRECT - N_INDIRECT*RPC)%RPC;
            *cnp = refsOfRefs[referenceIndex];
        }
    }
}
