#!/bin/bash

source tools.sh

# Clean and recompile
(cd .. && make clean && make)

# Create and format disk
$bin/createDisk $diskname 10000
$bin/mksofs $diskname

# Allocate some clusters and check SB
for i in $(seq 1 98)
do
$bin/showblock -s 0 $diskname
alloc_clusters
alloc_clusters
alloc_clusters
alloc_clusters
alloc_clusters
$bin/showblock -s 0 $diskname
done
alloc_clusters
