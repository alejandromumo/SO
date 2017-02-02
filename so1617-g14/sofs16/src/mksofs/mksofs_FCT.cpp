	#include "mksofs.h"
	#include "superblock.h"
	#include "exception.h"
	#include "rawdisk.h"    // Included rawdisk to have access to BLOCK_SIZE and soWriteRawCluster
	#include "core.h"
	#include <errno.h>

	/*
	 * create the table of references to free data clusters 
	 */
	void fillInFreeClusterList(SOSuperBlock * p_sb)
	{
		/*
		* Free Cluster Table is made of linked arrays of integers where:
		*	-Each array occupies one cluster
		*	-Each integer is the index of a free cluster except the last index of each array
		* 	-Each array has N indexes where:
		*		-N * sizeOf(uint32_t) = C (size in bytes of a cluster)
		*		(=)N = C/4
		*		-C = csize * BLOCK_SIZE
		*	-The first N-1 integers are indexes of free clusters
		*	-The last integer is the number of the next cluster containing another array
		*	-cref is both the number of clusters used by this table and the number of arrays in the table
		*/
		uint32_t RPC,C,I,J; 
		C = p_sb->csize * BLOCK_SIZE;
		RPC = C/sizeof(uint32_t);
		uint32_t physical_index = p_sb -> czstart + p_sb -> csize;
		uint32_t already_filled = 0; /* Variable that  keeps how many indexes have been saved */
 
		if(p_sb->cfree == 0 && p_sb -> crefs == 0)
			return;
		I = 1;
		if(p_sb->cfree == 0 && p_sb -> crefs > 0){
			for(I = 1;I<= p_sb-> crefs;I++){
				uint32_t arrayRefs_I[RPC];
				for(J = 0;J < RPC; J++){
					arrayRefs_I[J] = NULL_REFERENCE;
				}
				soWriteRawCluster(physical_index,arrayRefs_I,p_sb-> csize);
				physical_index += p_sb -> csize;
			}
		}
		else{
			for(I = 1;I<= p_sb-> crefs;I++){
				uint32_t arrayRefs_I[RPC];
				for(J = 0;J < RPC -1 ; J++){
					if(already_filled < p_sb -> cfree){
						arrayRefs_I[J] = p_sb -> crefs + 1 + already_filled; // already_filled pode ser I * (RPC-1)
						already_filled++;
					}
					else
						arrayRefs_I[J] = NULL_REFERENCE;
				}
				if(I == p_sb->crefs){
					arrayRefs_I[RPC-1] = NULL_REFERENCE;
				}
				else{
					arrayRefs_I[RPC-1] = I+1;
				}
				soWriteRawCluster(physical_index,arrayRefs_I,p_sb-> csize);
				physical_index += p_sb -> csize;
			}
		}
	}