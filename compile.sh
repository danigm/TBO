#!/bin/bash

if [ ! -d build ]
then
    mkdir build
fi

DATA="$PWD/data"

cd build
cmake -DDATA_DIR=$DATA ../
make
cd ..
cp build/src/tbo .

