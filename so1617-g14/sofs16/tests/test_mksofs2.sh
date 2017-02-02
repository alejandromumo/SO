#!/bin/bash

source tools.sh

# Clean and recompile
(cd .. && make clean && make)

for i in $(seq 8 8)
do
    echo $i

    # Both disks start in the same state
    $bin/createDisk $diskname $i &>/dev/null
    copy1=$diskname"_copy1"
    copy2=$diskname"_copy2"
    cp $diskname $copy1
    cp $diskname $copy2

    quiet="-q"

    # Chosen Test (replace for desired)
    $bin/mksofs $quiet -n sofs -i 28 $copy1
    $bin/mksofs_bin $quiet -n sofs -i 28 $copy2
    let j=i-1

    # Chosen disk parts to compare (replace for desired)
    $bin/showblock -s 0 $copy1 > a
    $bin/showblock -s 0 $copy2 > b
    git diff --no-index a b
done

rm a
rm b
