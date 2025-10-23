/**
 * @file my_model.h
 * @author MSU
 * @brief This unit abstracts Tensorflow calculations.
 * @date 2024-12-17
 * 
 */

#pragma once

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tensorflow/lite/c/common.h"

/** Maximum model binary blob length in bytes, must be kept in sync with partition table configuration */
#define MY_MODEL_MAX_LEN (500 * 1024)

/// @brief TFMicro ML interpreter public API
namespace my_model {
    extern void (*computation_fininshed_callback)(float);

    esp_err_t init(const uint8_t* model_blob);
    void start_computation(float* buffer, size_t len);
}
