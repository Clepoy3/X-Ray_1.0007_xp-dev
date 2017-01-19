#!/bin/bash

cmake CMakeLists.txt -G 'Unix Makefiles'
make -j8
cp src/libluabind.a ./
