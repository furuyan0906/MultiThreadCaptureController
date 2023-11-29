#!/usr/bin/bash

LIB_PATH=$(cd $(dirname 0); pwd)/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${LIB_PATH}
