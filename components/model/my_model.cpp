/**
 * @file my_model.cpp
 * @author MSU
 * @brief This unit abstracts Tensorflow calculations.
 * @date 2024-11-28
 * 
 */

#include "my_model.h"
#include "preprocessing.h"

#include "esp_log.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

/// @brief ML-model input buffer pointer
TfLiteTensor *input;
/// @brief ML-model output buffer pointer
TfLiteTensor *output;
/// @brief TFMicro task handle
TaskHandle_t model_task_handle;
/// @brief TFmicro error reporter instance pointer
tflite::ErrorReporter* error_reporter = nullptr;
/// @brief TFMicro model instance pointer
const tflite::Model* model = nullptr;
/// @brief TFMicro interpreter instance pointer
tflite::MicroInterpreter* interpreter = nullptr;
/// @brief Pointer to preprocessed data buffer
float* preproc_buffer = NULL;
/// @brief Not used, the input data length is hardcoded into the TFMicro model
size_t buffer_len;

/// @brief create an area of memory to use for input, output, and intermediate arrays.
/// minimum arena size, at the time of writing. after allocating tensors
/// you can retrieve this value by invoking interpreter.arena_used_bytes().
const int kModelArenaSize = 2468 * 4;
/// @brief Extra headroom for model + alignment + future interpreter changes.
const int kExtraArenaSize = 560 + 16 + 100;
/// @brief Final tensor arena buffer size to be statically allocated
const int kTensorArenaSize = kModelArenaSize + kExtraArenaSize;
/// @brief Tensor arena buffer for TFMicro
uint8_t tensor_arena[kTensorArenaSize];

/// @brief Debug log tag
static const char TAG[] = "MODEL";

/// @brief Invokes preprocessing and Tensorflow on preproc_buffer. Returns with a debug console message if the buffer is NULL.
/// When completed, invokes computation_fininshed_callback.
static void prvInvokeInterpreter()
{
    if (!preproc_buffer) {
        ESP_LOGW(TAG, "Model input buffer is missing!");
        return; //Ignore missing buffer 
    }
    preprocessing::preprocess(preproc_buffer);
    /*ESP_LOGI(TAG, "Invoking TFMicro on following buffer:");
    for (size_t i = 0; i < buffer_len; i++)
    {
        printf("\t%i = %8.6f\n", i, preproc_buffer[i]);
    }*/
    input->data.f = preproc_buffer + 1;
    interpreter->Invoke();
    float res = output->data.f[0];
    ESP_LOGI(TAG, "y: %f", res);
    if (res < 0) res = 0;
    my_model::computation_fininshed_callback(res);
}

/// @brief Model task body function. Waits for a task notification, and if one is received, runs a calculation.
/// @param pvParameter Not used
static void model_task(void *pvParameter)
{
    static BaseType_t xResult;

    for (;;)
    {
        xResult = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100));
        if (xResult == pdTRUE)
        {
            prvInvokeInterpreter();
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

namespace my_model {
    /// @brief Is called from the model thread once a computation is finished.
    void (*computation_fininshed_callback)(float);

    /// @brief Initialize ML model
    /// @param model_blob Pointer to the model blob (usually a memory-mapped partition)
    /// @return ESP_OK if succeeded, ESP_FAIL if TFLite failed to allocate resources.
    esp_err_t init(const uint8_t* model_blob)
    {
        model = tflite::GetModel(model_blob);
        static tflite::MicroErrorReporter micro_error_reporter;
        error_reporter = &micro_error_reporter;
        static tflite::AllOpsResolver resolver;

        // Build an interpreter to run the model with.
        static tflite::MicroInterpreter static_interpreter(
            model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
        interpreter = &static_interpreter;

        // Allocate memory from the tensor_arena for the model's tensors.
        TfLiteStatus allocate_status = interpreter->AllocateTensors();
        if (allocate_status != kTfLiteOk) {
            TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
            return ESP_FAIL;
        }

        // Obtain pointers to the model's input and output tensors.
        input = interpreter->input(0);
        output = interpreter->output(0);

        // Keep track of how many inferences we have performed.
        xTaskCreate(model_task, "model_task", 4096, NULL, 1, &model_task_handle);
        assert(model_task_handle);
        return ESP_OK;
    }

    /// @brief Send a task notification to the model thread to start computation.
    /// @param buffer Buffer for the precprocessor and the TFLite model to work on
    /// @param len Buffer (data) length
    void start_computation(float* buffer, size_t len)
    {
        preproc_buffer = buffer;
        buffer_len = len;
        xTaskNotifyGive(model_task_handle);
    }
}
