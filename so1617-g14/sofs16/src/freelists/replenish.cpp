/**
 *  \author Manuel Costa
 *  \tester Manuel Costa
 */

#include "freelists.h"
#include "probing.h"
#include "exception.h"
#include "sbdealer.h" // getSuperblock
#include "czdealer.h" // soWriteCluster / read
#include "cluster.h" // RPB
#include "core.h" // NULL_REFERENCE
#include "superblock.h" // FCT_CACHE_SIZE

#include <errno.h>

/*
 * Even if some of them are not functionally necessary,
 * the following dictates must be obeyed by the implementation:
 * - do nothing if head cache is not empty; (V)
 * - the head cache should be filled from the beginning, 
 *      so, at the end, index out should be put at zero; (V)
 * - if crefs is equal to zero, transfer references from the tail cache; (V)
 * - otherwise, transfer references only from the head cluster; (V)
 * - if after the transfer the head cluster get empty, it should be freed; (V)
 * - after every reference is transferred, 
 *      the previous location should be filled with NULL_REFERENCE (V)
 */
void soReplenish(void)
{   
    soProbe(733, "soReplenish()\n");
    SOSuperBlock* spb =  sbGetPointer();
    uint32_t ref_per_cluster = soGetRPC();
    /*Chead cache is not empty, nothing to do*/
    if(spb->chead.cache.ref[spb->chead.cache.out] != NULL_REFERENCE){
        return;
    }
    else{
           spb->chead.cache.out = 0;
           spb->chead.cache.in  = 0;
        /*Transfer references from ctail or clusters*/
            /*Replenish from ctail if there aren't clusters to replenish from*/
        if (spb->crefs==0){
            /*If there are no clusters to fill from, both ctail and chead cluster_number are set to NULL and idx to 0*/
            if(spb->ctail.cache.ref[spb->ctail.cache.out] == NULL_REFERENCE) return; 
            soDeplete();
        } 
            /*Replenish from clusters if there are some free clusters to replenish from FCT*/
        else{
            uint32_t refs[ref_per_cluster];           
            soReadCluster(spb->chead.cluster_number,refs);
                /* Replenish cache from buffer (refs) read by soReadCluster()*/
            while(     refs[spb->chead.cluster_idx] != NULL_REFERENCE               /*enquanto houver indices a copiar*/
                    && spb->chead.cache.ref[spb->chead.cache.in] == NULL_REFERENCE  /*enquanto a cache tiver espaço*/
                    && spb->chead.cluster_idx < ref_per_cluster-1){                 /*enquanto não estivermos a copiar o último*/

                spb->chead.cache.ref[spb->chead.cache.in] = refs[spb->chead.cluster_idx];
                spb->chead.cache.in = (spb->chead.cache.in + 1) % FCT_CACHE_SIZE;
                refs[spb->chead.cluster_idx] = NULL_REFERENCE;
                spb->chead.cluster_idx++;               
            }
 
            /*  If cluster_number at cluster_idx has a NULL_REFERENCE means cluster is empty*/
            /*      The only reference left is pointing to next cluster from the FCT*/
            if(spb->chead.cluster_idx == ref_per_cluster-1 || refs[spb->chead.cluster_idx] == NULL_REFERENCE){
                /*Next cluster in the FCT containing references*/
                uint32_t nextCluster = refs[ref_per_cluster-1]; 
                soFreeCluster(spb->chead.cluster_number);
                spb->crefs--;
                /*If FCT is empty, chead and ctail cluster_number is NULL_REFERENCE. Otherwise, is an int*/
                if(spb->crefs==0){
                    spb->chead.cluster_number = NULL_REFERENCE;
                    spb->ctail.cluster_number = NULL_REFERENCE;    
                    spb->ctail.cluster_idx = 0;
                }
                spb->chead.cluster_number = nextCluster;
                spb->chead.cluster_idx = 0;
            }
            /*  If it's not empty, we shall re-write the cluster*/
            /*      Update cluster we've read from*/
            else
                soWriteCluster(spb->chead.cluster_number,refs); 
        }

        /*After we replenish chead, chead cache out is set to 0*/
        sbSave();
       
    }
}