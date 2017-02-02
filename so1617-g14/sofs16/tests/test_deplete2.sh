#!/bin/bash

source tools.sh

# Clean and recompile
(cd .. && make clean && make)

# Create and format disk
$bin/createDisk $diskname 1000
$bin/mksofs $diskname

# Allocate all clusters (487+3 because of clusters used
# by the FCT that are freed in the process)
alloc_clusters 490

# Free 50 clusters that go to the tail
free_clusters $(seq 51 100)

# Move the previous 50 clusters to the head
replenish

# Free 50 more clusters that go to the tail again
free_clusters $(seq 151 200)

# Free 1 cluster that allocates a cluster for a new FCT and moves the clusters
# from the tail to the new FCT
free_clusters 237

$bin/showblock -r 120-121 $diskname
$bin/showblock -s 0 $diskname
