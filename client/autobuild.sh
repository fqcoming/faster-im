#!/bin/bash

# If the command execution fails, 
# immediately stop executing the entire script.
set -e


if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

rm -rf `pwd`/build/*
cd `pwd`/build &&
	cmake .. &&
	make
cd ..