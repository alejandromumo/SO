#!/bin/bash

source tools.sh

# Clean and recompile
(cd .. && make clean && make)

# Create and format disk
$bin/createDisk $diskname 10000
$bin/mksofs $diskname

# Alloc an inode
alloc_inodes 1 1

# Alloc fileclusters on previous inode
for i in $(seq 0 5000)
do
alloc_file_cluster 2 $i
done

$bin/showblock -i 1 $diskname
#$bin/showblock -r 54-55 $diskname # 10
#$bin/showblock -r 568-569 $diskname # 10
#$bin/showblock -r 1082-1083 $diskname # cluster 524
#$bin/showblock -r 1084-1085 $diskname # cluster 525
#$bin/showblock -r 3132-3133 $diskname
$bin/showblock -r 1238-1239 $diskname # cluster x
#$bin/showblock -r 3136-3137 $diskname
$bin/showblock -s 0 $diskname
