/**
 *  \author Manuel Costa, Fábio Maia
 *  \tester ...
 */

#include "czdealer.h"
#include "cluster.h"
#include "direntry.h"
#include "superblock.h"
#include "inode.h"
#include "sbdealer.h"
#include "probing.h"
#include "exception.h"
#include "core.h"

#include <errno.h>

static SOSuperBlock *sbp = sbGetPointer();
static bool isOpen = false;

void soOpenClusterZoneDealer()
{
	if(!isOpen)
		isOpen = true;
}

void soCloseClusterZoneDealer()
{
	if(isOpen)
		isOpen = false;

}

void soReadCluster(uint32_t n, void *buf)
{
	if(isOpen)
    	soReadRawCluster(n, buf, sbp->csize);

}

void soWriteCluster(uint32_t n, void *buf)
{
	if(isOpen)
    	soWriteRawCluster(n, buf, sbp->csize);
}

uint32_t soGetBPC()
{
    return BLOCK_SIZE * sbp->csize;
}

uint32_t soGetRPC()
{
    return RPB * sbp->csize;
}

uint32_t soGetDPC()
{
    return DPB * sbp->csize;
}

uint32_t soGetMaxFileSize()
{
    uint32_t RPC = soGetRPC();
    return (N_DIRECT + (N_INDIRECT*RPC) + (RPC*RPC)) * sbp->csize * BLOCK_SIZE;
}