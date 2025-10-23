#pragma once

//#define CYCLE_LENGTH 301

/// @brief Public API of the ML-model data preprocessor
namespace preprocessing {
    //extern float output_buffer[CYCLE_LENGTH];

    void preprocess(float* initial);
}
