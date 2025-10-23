#!/bin/sh

#expect model file name as $1
#put model file in the working directory of this script
#this script must be ran from the root directory of the project (so that the model data file is at main/model_data.cpp)

rm main/model_data.cpp
echo -e '#include "model_data.h"\n'
xxd -i $1 >> main/model_data.cpp
vname = $(echo "$1" | tr '.' '_')
echo -e "\nconst uint8_t* model_data = ${vname};"
