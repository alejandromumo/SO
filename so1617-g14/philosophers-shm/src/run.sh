#! /usr/bin/env bash

cleanup()
{
    echo ">>> Cleaning semaphores"
    rm /dev/shm/sem*
}

# Clean and recompile
make cleanall
make

# Default
cleanup
./simulation -n 5 -l 10 -L 100 -f 3 -k 2 -p 10 -s 10 -t 20 -c 50 -e 10 -w 15

# Hardcore
# ./simulation -n 5 -l 10 -L 100 -f 2 -k 1 -p 5 -s 5 -t 20 -c 50 -e 10 -w 15
# cleanup
