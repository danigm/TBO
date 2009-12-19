#!/bin/bash

if [ ! -d build ]
then
    mkdir build
fi

DATA="$PWD/tbo-data"

cd build
cmake -DDATA_DIR=$DATA ../
make
cd ..
cp build/src/tbo .

