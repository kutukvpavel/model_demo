#!/bin/sh

#expect model file name as $1 (e.g. ./xxd_model.sh model_motya.vtflite)
#put model file in the working directory of this script
#this script must be ran from the root directory of the project (so that the model data file is at main/model_data.cpp)

if [ ! -n "$1" ]; then
    echo "Please specify the model file name (e.g. model.vtflite)"
    exit 1
fi

rm main/model_data.cpp
echo '' > main/model_data.cpp
printf '#include "data.h"\n\nconst ' > main/model_data.cpp
xxd -i $1 >> main/model_data.cpp
vname=$(echo "$1" | tr '.' '_')
echo "\nconst uint8_t* model_data = ${vname};" >> main/model_data.cpp
