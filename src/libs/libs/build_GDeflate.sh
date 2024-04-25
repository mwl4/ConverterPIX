#!/bin/sh
mkdir build_GDeflate
cd build_GDeflate
git clone https://github.com/microsoft/DirectStorage.git ./
git submodule init
git submodule update
cd GDeflate/GDeflate
cmake -B build -S ./
cmake --build build
cp ./build/libdeflate.a ../../../
cp ./build/libGDeflate.a ../../../