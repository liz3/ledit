#!/bin/sh
mkdir Debug
cd Debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j2
