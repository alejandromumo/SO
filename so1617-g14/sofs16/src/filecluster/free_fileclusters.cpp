/**
 *  \author Luís Leira
 *  \tester Luís Leira
 */

#include "filecluster.h"

#include "probing.h"
#include "exception.h"
#include "inode.h"
#include "freelists.h" /* added */
#include "core.h" /* added */
#include "dealers.h" /* added */

#include <errno.h>
#include <stdint.h>

#if 1
static void soFreeIndirectFileClusters(SOInode * ip, uint32_t ffcn, uint32_t * n_clusters, uint32_t * finished);
static void soFreeDoubleIndirectFileClusters(SOInode * ip, uint32_t ffcn, uint32_t * n_clusters);
#endif

/* ********************************************************* */

void soFreeFileClusters(int ih, uint32_t ffcn)
{
    soProbe(600, "soFreeFileClusters(%d, %u)\n", ih, ffcn);

    SOInode *p_inode = iGetPointer(ih);
    uint32_t RPC = soGetRPC();
    uint32_t n_clusters = p_inode->csize; /* Variable that counts the number of clusters left */
    uint32_t finished; /* Variable that indicates if soFreeDoubleIndirectFileClusters has to be used */
    

    /* Check if ffcn is in range */
	if((ffcn < 0) || (ffcn >= (N_DIRECT + (N_INDIRECT * RPC) + (RPC * RPC))))
		throw SOException(EINVAL, __FUNCTION__);

	/* Direct */
	for(uint32_t i = 0; i < N_DIRECT; i++){
		/* Check if there are no more clusters */
		if(n_clusters == 0) return;

		if(p_inode->d[i] != NULL_REFERENCE){
			/* Check if index is equal or above ffcn */
			if(i >= ffcn){
				/* Free cluster */
				soFreeCluster(p_inode->d[i]);
				p_inode->d[i] = NULL_REFERENCE;
				p_inode->csize--;
			}

			n_clusters--; /* Decrement number of clusters left */
		}
	}

	/* Check if there are no more clusters */
	if(n_clusters == 0) return;

	/* Single Indirect */
	if(ffcn < N_INDIRECT * RPC + N_DIRECT)
		soFreeIndirectFileClusters(p_inode, ffcn, &n_clusters, &finished);

	/* Double Indirect */
	if(!finished)
		soFreeDoubleIndirectFileClusters(p_inode, ffcn, &n_clusters);

}

#if 1
/* ********************************************************* */

/* only a hint to decompose the solution */
static void soFreeIndirectFileClusters(SOInode * ip, uint32_t ffcn, uint32_t * n_clusters, uint32_t * finished)
{
    soProbe(600, "soFreeIndirectFileClusters(%p, %u, %u, %u)\n", ip, ffcn, n_clusters, finished);

    //throw SOException(ENOSYS, __FUNCTION__);

	uint32_t RPC = soGetRPC();
	uint32_t ref[RPC];
	uint32_t free; /* Variable that indicates if cluster has to be erased */

	for(uint32_t i = 0; i < N_INDIRECT; i++){
		if(ip->i1[i] != NULL_REFERENCE){
			free = 1;

			soReadCluster(ip->i1[i], ref);

			for(uint32_t j = 0; j < RPC; j++){
				/* Checks if is only itself */
				if((*n_clusters) <= 1){
					if(free){
						soFreeCluster(ip->i1[i]);
						ip->i1[i] = NULL_REFERENCE;
						ip->csize--;
						(*n_clusters)--; /* Decrement number of clusters left */
					}

					soWriteCluster(ip->i1[i], ref);
					(*finished) = 1;
					return;
				}

				if(ref[j] != NULL_REFERENCE){
					/* Check if cluster is to be freed */
					if(ffcn <= N_DIRECT || j >= (ffcn - N_DIRECT) - (RPC * i)){
						soFreeCluster(ref[j]);
						ref[j] = NULL_REFERENCE;
						ip->csize--;
					}else{
						free = 0;
					}

					(*n_clusters)--; /* Decrement number of clusters left */
				}
			}

			soWriteCluster(ip->i1[i], ref);
			
			if(free){
				soFreeCluster(ip->i1[i]);
				ip->i1[i] = NULL_REFERENCE;
				ip->csize--;
				(*n_clusters)--; /* Decrement number of clusters left */
				return;
			}
		}
	}
	
	(*finished) = 0;
}

/* ********************************************************* */

/* only a hint to decompose the solution */
static void soFreeDoubleIndirectFileClusters(SOInode * ip, uint32_t ffcn, uint32_t * n_clusters)
{
    soProbe(600, "soFreeDoubleIndirectFileClusters(%p, %u, %u)\n", ip, ffcn, n_clusters);

    //throw SOException(ENOSYS, __FUNCTION__);

    uint32_t RPC = soGetRPC();
	uint32_t ref[RPC], ref_in[RPC];
	uint32_t free, free_in;

	if(ip->i2 == NULL_REFERENCE)
		return;

	free = 1;
	soReadCluster(ip->i2, ref);

	for(uint32_t i = 0; i < RPC; i++){
		if(ref[i] != NULL_REFERENCE){
			free_in = 1;

			soReadCluster(ref[i], ref_in);

			for(uint32_t j = 0; j < RPC; j++){
				/* Checks if is only itself and his parent */
				if((*n_clusters) <= 2){
					if(free_in){
						soFreeCluster(ref[i]);
						ref[i] = NULL_REFERENCE;
						ip->csize--;
						(*n_clusters)--; /* Decrement number of clusters left */
						if(free){
							soFreeCluster(ip->i2);
							ip->i2 = NULL_REFERENCE;
							ip->csize--;
							(*n_clusters)--; /* Decrement number of clusters left */
						}
					}

					soWriteCluster(ref[i], ref_in);
					soWriteCluster(ip->i2, ref);
					return;
				}

				if(ref_in[j] != NULL_REFERENCE){
					/* Check if cluster is to be freed */
					if(ffcn <= N_DIRECT + N_DIRECT || j >= (ffcn - N_DIRECT - (N_INDIRECT * RPC)) - (RPC * i)){
						soFreeCluster(ref_in[j]);
						ref_in[j] = NULL_REFERENCE;
						ip->csize--;
					}else{
						free_in = 0;
					}

					(*n_clusters)--; /* Decrement number of clusters left */
				}
			}

			soWriteCluster(ref[i], ref_in);
			
			if(free_in){
				soFreeCluster(ref[i]);
				ref[i] = NULL_REFERENCE;
				ip->csize--;
				(*n_clusters)--; /* Decrement number of clusters left */
			}else{
				free = 0;
			}
		}
	}

	soWriteCluster(ip->i2, ref);
			
	if(free){
		soFreeCluster(ip->i2);
		ip->i2 = NULL_REFERENCE;
		ip->csize--;
		(*n_clusters)--; /* Decrement number of clusters left */
	}
}
#endif

/* ********************************************************* */
