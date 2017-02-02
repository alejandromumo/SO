#!/bin/bash

source tools.sh

# Clean and recompile
(cd .. && make clean && make)

# Create and format disk
$bin/createDisk $diskname 1000
$bin/mksofs $diskname

# Allocate some clusters
alloc_clusters 100

# Free other clusters to the tail
free_clusters $(seq 11 60)

# Deplete them back to the FCT
deplete

$bin/showblock -r 20-21 $diskname
$bin/showblock -r 22-23 $diskname
$bin/showblock -r 224-225 $diskname
$bin/showblock -s 0 $diskname
