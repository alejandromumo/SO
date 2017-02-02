#include "mksofs.h"

#include "superblock.h"
#include "exception.h"
#include "inode.h" /* added */
#include "core.h" /* added */
#include "direntry.h" /* added */

#include <errno.h>
#include <unistd.h> /* added */
#include <sys/stat.h> /* added */
#include <time.h> /* added */


/*
 * filling in the inode table:
 *   only inode 0 is in use (it describes the root directory)
 */
void fillInInodeTable(SOSuperBlock * p_sb)
{
	/* Prepare an empty block of inodes to be written on each block of the inode table */
	SOInode blockOfInodes[IPB];
	uint32_t inodeCounter = 2;

	/* Initialize generic empty inode */
	SOInode emptyInode;
	memset(&emptyInode, 0x00, sizeof(SOInode));
	emptyInode.mode = INODE_FREE;
	memset(emptyInode.d, NULL_REFERENCE, N_DIRECT*sizeof(uint32_t));
	memset(emptyInode.i1, NULL_REFERENCE, N_INDIRECT*sizeof(uint32_t));
	emptyInode.i2 = NULL_REFERENCE;

	/* Initialize root dir inode */
	SOInode rootDirInode = emptyInode;
	rootDirInode.mode = S_IFDIR | S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
	rootDirInode.refcount = 2;
	rootDirInode.owner = getuid();
	rootDirInode.group = getgid();
	rootDirInode.size = sizeof(SODirEntry)*2;
	rootDirInode.csize = 1;
	rootDirInode.atime = rootDirInode.ctime = rootDirInode.mtime = time(NULL);
	rootDirInode.d[0] = 0;
	memset(rootDirInode.d+1, NULL_REFERENCE, (N_DIRECT-1)*sizeof(uint32_t));
	memset(rootDirInode.i1, NULL_REFERENCE, N_INDIRECT*sizeof(uint32_t));
	rootDirInode.i2 = NULL_REFERENCE;

	/* Write the first block of inodes */
	blockOfInodes[0] = rootDirInode;
	for (uint32_t inode = 1; inode < IPB; inode++){
		blockOfInodes[inode] = emptyInode;
		blockOfInodes[inode].next = (inodeCounter == p_sb->itotal) ? NULL_REFERENCE : inodeCounter++;
	}
	soWriteRawBlock(p_sb->itstart, blockOfInodes);

	/* Write the remaining blocks of inodes */
	blockOfInodes[0] = emptyInode;
	for (uint32_t block = 1; block < p_sb->itsize; block++){
		for (uint32_t inode = 0; inode < IPB; inode++){
			blockOfInodes[inode].next = (inodeCounter == p_sb->itotal) ? NULL_REFERENCE : inodeCounter++;
		}
		soWriteRawBlock(p_sb->itstart+block, blockOfInodes);
	}
}