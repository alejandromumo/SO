#!/bin/bash

source tools.sh

# Clean and recompile
(cd .. && make clean && make)

# Create and format disk
$bin/createDisk $diskname 20
$bin/mksofs $diskname

# Allocate necessary inodes
alloc_inodes 2 2 # dirs
alloc_inodes 2 1 # files

# Add dir entries
add_dir_entry 0 foo 1
add_dir_entry 1 bar 2
add_dir_entry 2 baz 3
add_dir_entry 0 bak 4

traverse_path / # passes
traverse_path /foo # passes
traverse_path /foo/bar # passes
traverse_path /foo/bar/baz # passes
traverse_path /foo/baz  # fails because /foo/baz does not exist
traverse_path /bak/bar/foo # fails because bak is not dir

$bin/showblock -i 1 $diskname
