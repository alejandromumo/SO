#!/bin/bash

bin="./../bin"
diskname="/tmp/sofs16disk"
mountpoint="/tmp/sofs16mnt"
testtool_flags="-l 1,1000"

# Allocate $1 inodes of type $2
# alloc_inodes 10 1
alloc_inodes()
{
    for i in $(seq 1 $1)
    do
        echo -e "2\n$2\n0\n" | $bin/testtool $diskname $testtool_flags
    done
}

# Frees given inodes
# free_inodes 4 8 13 2
free_inodes()
{
    for i in $*
    do
        echo -e "3\n$i\n0\n" | $bin/testtool $diskname $testtool_flags
    done
}

# Allocates $1 clusters
# alloc_clusters 5
alloc_clusters()
{
    for i in $(seq 1 $1)
    do
        echo -e "4\n0\n" | $bin/testtool $diskname $testtool_flags
    done
}

# Frees given clusters
# free_clusters 4 8 13 2
free_clusters()
{
    for i in $*
    do
        echo -e "5\n$i\n0\n" | $bin/testtool $diskname $testtool_flags
    done
}

# Replenish head cache
# replenish
replenish()
{
    echo -e "6\n0\n" | $bin/testtool $diskname $testtool_flags
}

# Deplete tail cache
# deplete
deplete()
{
    echo -e "7\n0\n" | $bin/testtool $diskname $testtool_flags
}

# File clusters

get_file_cluster()
{
    echo -e "8\n$1\n$2\n0\n" | $bin/testtool $diskname $testtool_flags
}

alloc_file_cluster()
{
    echo -e "9\n$1\n$2\n0\n" | $bin/testtool $diskname $testtool_flags
}

free_file_cluster()
{
    echo -e "10\n$1\n$2\n0\n" | $bin/testtool $diskname $testtool_flags
}

read_file_cluster()
{
    echo -e "12\n$1\n$2\n0\n" | $bin/testtool $diskname $testtool_flags
}

write_file_cluster()
{
    echo -e "13\n$1\n$2\n$3\n0\n" | $bin/testtool $diskname $testtool_flags
}

# Dir entries

get_dir_entry()
{
    echo -e "14\n$1\n$2\n0\n" | $bin/testtool $diskname $testtool_flags
}

add_dir_entry()
{
    echo -e "15\n$1\n$2\n$3\n0\n" | $bin/testtool $diskname $testtool_flags
}

rename_dir_entry()
{
    echo -e "16\n$1\n$2\n$3\n0\n" | $bin/testtool $diskname $testtool_flags
}

delete_dir_entry()
{
    echo -e "17\n$1\n$2\n0\n" | $bin/testtool $diskname $testtool_flags
}

traverse_path()
{
    echo -e "18\n$1\n0\n" | $bin/testtool $diskname $testtool_flags
}
