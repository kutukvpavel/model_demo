#pragma once

#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>

#define TEST_DATA_LEN 301

/// @brief ML model preamble structure (contains metadata)
struct __packed model_preamble_t
{
    uint32_t version; ///< Model version
    uint32_t length; ///< Model binary blob length
    uint32_t crc; ///< CRC32, see crc32_le from ESP-IDF for implementation details
};

extern const uint8_t* model_data;
extern const float* test_data;
