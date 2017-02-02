#!/bin/bash

source tools.sh

# Clean and recompile
(cd .. && make clean && make)

# Create and format disk
$bin/createDisk $diskname 1000
$bin/mksofs $diskname

# Mount
mkdir $mountpoint
$bin/sofsmount -d -l 1,1000 $diskname $mountpoint

# Unmount
fusermount -u $mountpoint
rm -rf $mountpoint
