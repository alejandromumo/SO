#!/bin/bash

source tools.sh

# Clean and recompile
(cd .. && make clean && make)

# Create and format disk
$bin/createDisk $diskname 1000
$bin/mksofs $diskname

n=40

alloc_inodes $n 1

for i in $(seq 1 $n)
do
    add_dir_entry 0 foo$i $i
done

add_dir_entry 0 foo8 8
add_dir_entry 0 foo41 41

for i in $(seq 1 $n)
do
    get_dir_entry 0 "foo$i"
done

rename_dir_entry 0 foo2 bar2 # passes
rename_dir_entry 0 bar123 foo123 # fails because bar123 does not exist

delete_dir_entry 0 foo2 # fails because foo2 no longer exists
delete_dir_entry 0 foo3 # passes

$bin/showblock $diskname -d 18-27
$bin/showblock $diskname -s 0
$bin/showblock $diskname -i 1
