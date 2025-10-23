#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <sys/fcntl.h>

#include "my_model.h"
#include "data.h"

static const char TAG[] = "model_demo";

void computed_callback(float result)
{
    ESP_LOGI(TAG, "Computation finished, result: %f", result);
}

_BEGIN_STD_C
void app_main()
{
    static float test_data_buffer[TEST_DATA_LEN];

    vTaskDelay(pdTICKS_TO_MS(1000));

    my_model::computation_fininshed_callback = computed_callback;
    ESP_ERROR_CHECK(my_model::init(model_data + sizeof(model_preamble_t::version)));
    ESP_LOGI(TAG, "Model initialized. Version = %" PRIu32, *reinterpret_cast<const uint32_t*>(model_data));

    while (1) {
        putchar('\n');
        ESP_LOGI(TAG, "Starting the test...");

        memcpy(test_data_buffer, test_data, TEST_DATA_LEN);
        my_model::start_computation(test_data_buffer, TEST_DATA_LEN);

        vTaskDelay(pdMS_TO_TICKS(500));
        ESP_LOGI(TAG, "Test completed. Press any key to repeat...");
        while (getchar() == EOF)
        {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}
_END_STD_C