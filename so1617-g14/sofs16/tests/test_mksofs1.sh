#!/bin/bash

source tools.sh

# Clean and recompile
(cd .. && make clean && make)

for i in $(seq 37 100)
do
    echo $i

    # Both disks start in the same state
    $bin/createDisk $diskname $i &>/dev/null
    copy1=$diskname"_copy1"
    copy2=$diskname"_copy2"
    cp $diskname $copy1
    cp $diskname $copy2

    quiet="-q"

    #Teste 1
    $bin/mksofs $quiet $copy1
    $bin/mksofs_bin $quiet $copy2
    diff <(xxd $copy1) <(xxd $copy2)

    #Teste 2
    $bin/mksofs $quiet -n sofs $copy1
    $bin/mksofs_bin $quiet -n sofs $copy2
    diff <(xxd $copy1) <(xxd $copy2)

    #Teste 3
    $bin/mksofs $quiet -n sofs -i 28 $copy1
    $bin/mksofs_bin $quiet -n sofs -i 28 $copy2
    diff <(xxd $copy1) <(xxd $copy2)

    #Teste 4
    $bin/mksofs $quiet -n sofs -i 235 -c 3 $copy1
    $bin/mksofs_bin $quiet -n sofs -i 235 -c 3 $copy2
    diff <(xxd $copy1) <(xxd $copy2)

    #Teste 5
    $bin/mksofs $quiet -n sofs -i 37 -c 4 -z $copy1
    $bin/mksofs_bin $quiet -n sofs -i 37 -c 4 -z $copy2
    diff <(xxd $copy1) <(xxd $copy2)
done
