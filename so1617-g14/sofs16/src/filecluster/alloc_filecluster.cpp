/**
 *  \author Manuel Costa
 *  \tester Manuel Costa
 */

#include "filecluster.h"

#include "probing.h"
#include "exception.h"
#include "inode.h"
#include "itdealer.h"
#include "czdealer.h"
#include "freelists.h"
#include <core.h>

#include <errno.h>
#include <stdint.h>

static void soAllocIndirectFileCluster(SOInode * ip, uint32_t fcn, uint32_t * cnp);
static void soAllocDoubleIndirectFileCluster(SOInode * ip, uint32_t fcn, uint32_t * cnp);

/* ********************************************************* */

void soAllocFileCluster(int ih, uint32_t fcn, uint32_t * cnp)
{
    soProbe(600, "soAllocFileCluster(%d, %u, %p)\n", ih, fcn, cnp);
    uint32_t RPC = soGetRPC();

    if(cnp == NULL){
        throw SOException(EINVAL, __FUNCTION__);
    }
    if(fcn < 0 || fcn >= N_DIRECT + (N_INDIRECT*RPC) + (RPC*RPC)){
        throw SOException(EINVAL,__FUNCTION__); 
    }


    //i-node corresponding to our file
    SOInode *ip = iGetPointer(ih);

    //decision where the desired cluster is (d,i1 or i2)
    if(fcn < N_DIRECT){
        //Trabalha-se diretamente com d[fcn]
        if(ip->d[fcn] != NULL_REFERENCE)
            soFreeCluster(ip->d[fcn]);
        soAllocCluster(cnp);
        ip->d[fcn] = *cnp;
        ip->csize++;
    }
    else if(fcn - N_DIRECT < (N_INDIRECT*RPC)){
        //Trabalha-se na i1 Indirect 
        uint32_t afcn = fcn - N_DIRECT;
        soAllocIndirectFileCluster(ip,afcn,cnp);
    }
    else{ 
        //Trabalha-se na i2 Double Indirect
        uint32_t afcn = (fcn - N_DIRECT - (N_INDIRECT*RPC));
        soAllocDoubleIndirectFileCluster(ip,afcn,cnp);
    }
    iSave(ih);
}


/* ********************************************************* */


static void soAllocIndirectFileCluster(SOInode * ip, uint32_t afcn, uint32_t * cnp)
{
    soProbe(600, "soAllocIndirectFileCluster(%p, %u, %p)\n", ip, afcn, cnp);
    uint32_t RPC = soGetRPC();
    // i1x represents the index in the first layer. In this case, it's either 0 or 1
    uint32_t i1x = (int) afcn/RPC;
    // i1y represents the index in the second layer. In this case, it's an integer between 0 and RPC-1 
    uint32_t i1y =  afcn % RPC;
    uint32_t cluster_buffer[RPC];

    if(ip->i1[i1x] == NULL_REFERENCE){
        //Alloc the cluster
        soAllocCluster(cnp);
        ip->i1[i1x] = *cnp;
        ip->csize  ++;

        //Format the cluster
        soReadCluster(ip->i1[i1x],cluster_buffer);
        memset(cluster_buffer,NULL_REFERENCE,RPC*sizeof(uint32_t));
        soWriteCluster(ip->i1[i1x],cluster_buffer);
    }
    else 
        soReadCluster(ip->i1[i1x],cluster_buffer);


    if(cluster_buffer[i1y] != NULL_REFERENCE){
        //printf("Cluster already has info, trying to free cluster # %d\n",cluster_buffer[i1y]);
        soFreeCluster(cluster_buffer[i1y]);
    }
    soAllocCluster(cnp);
    cluster_buffer[i1y] = *cnp;
    soWriteCluster(ip->i1[i1x],cluster_buffer);

    // Update i-node info
    ip->csize++;

}

/* ********************************************************* */


static void soAllocDoubleIndirectFileCluster(SOInode * ip, uint32_t afcn, uint32_t * cnp)
{
    soProbe(600, "soAllocDoubleIndirectFileCluster(%p, %u, %p)\n", ip, afcn, cnp);
    uint32_t RPC = soGetRPC();
    // i2x represents the index in the second layer (first layer is a cluster). It's an integer between 0 and RPC-1 (i2[i2x])
    int i2x = (int) afcn / RPC;
    // i2y represents the index in the third layer. It's an integer between 0 and RPC-1 (i2[ii2x][i2y])
    int i2y = afcn % RPC;

    // Deal with first cluster containing refs
    uint32_t first_cluster_buffer[RPC];
    if(ip->i2 == NULL_REFERENCE){ 
        //Alloc the cluster
        soAllocCluster(cnp);
        ip->i2 = *cnp;
        ip->csize ++;

        //Format the cluster
        soReadCluster(ip->i2,first_cluster_buffer);
        memset(first_cluster_buffer,NULL_REFERENCE,RPC*sizeof(uint32_t));
        soWriteCluster(ip->i2,first_cluster_buffer);
    }
    else
        soReadCluster(ip->i2,first_cluster_buffer);
    
    

    // Deal with second cluster containing refs
    uint32_t second_cluster_buffer[RPC];
    if(first_cluster_buffer[i2x] == NULL_REFERENCE){
        //Alloc the cluster
        soAllocCluster(cnp);
        first_cluster_buffer[i2x] = *cnp;
        soWriteCluster(ip->i2,first_cluster_buffer);

        //Format it 
        soReadCluster(first_cluster_buffer[i2x],second_cluster_buffer);
        memset(second_cluster_buffer,NULL_REFERENCE,RPC*sizeof(uint32_t));
        soWriteCluster(first_cluster_buffer[i2x],second_cluster_buffer);
    }
    else
        soReadCluster(first_cluster_buffer[i2x],second_cluster_buffer);
    
 
    if(second_cluster_buffer[i2y]!= NULL_REFERENCE)
        soFreeCluster(second_cluster_buffer[i2y]);
    
    soAllocCluster(cnp);
    second_cluster_buffer[i2y] = *cnp;
    soWriteCluster(first_cluster_buffer[i2x],second_cluster_buffer);


    // Update i-node info
    ip->csize++;
}