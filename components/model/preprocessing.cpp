/**
 * @file preprocessing.cpp
 * @author MSU
 * @brief Data preprocessing for the Tensorflow model. Initially used polyfit to flatten the baseline.
 * Now it only scales the sensor resistance curve for it's values to fit into E[0,1] interval
 * @date 2024-11-28
 * 
 */

#include "preprocessing.h"

#include <math.h>
#include <eigen3/Eigen/Dense>
#include <iostream>
#include <cmath>
#include <vector>
#include <eigen3/Eigen/QR>
#include "esp_log.h"

/// @brief Debug log tag
static const char TAG[] = "PREPROC";

template<typename Type>
Type min(const Type* array, size_t array_size) {
    Type find_min = array[0];
    for (int i = 0; i <= array_size; i++) {
        if (array[i] < find_min) {
            find_min = array[i];
        }
    }
    return find_min;
}

template<typename Type>
Type min(const Type* array, int index, int window) {
    Type find_min = array[index];
    for (int i = index - window; i <= index + window; i++) {
        if (array[i] < find_min) {
            find_min = array[i];
        }
    }
    return find_min;
}

template<typename Type>
Type max(const Type* array, int index, int window, int* argmax) {
    Type find_max = array[index];
    int temp_argmax = 0;
    for (int i = index - window; i <= index + window; i++) {
        if (array[i] > find_max) {
            find_max = array[i];
            temp_argmax = i;
        }
    }
    *argmax = temp_argmax;
    return find_max;
}

template<typename Type>
Type max(const Type* array, int array_size, int* argmax) {
    Type find_max = array[0];
    int temp_argmax = 0;
    for (int i = 0; i < array_size; i++) {
        if (array[i] > find_max) {
            find_max = array[i];
            temp_argmax = i;
        }
    }
    *argmax = temp_argmax;
    return find_max;
}


template<typename Type>
Type max_of_3(Type val1, Type val2, Type val3) {
    if (val1 > val2) {
        if (val1 > val3) {
            return val1;
        }
        else
        {
            if (val3 > val2) {
                return val3;
            }
            else {
                return val2;
            }
        }
    }
    else 
    {
        if (val3 > val2) {
            return val3;
        }
        else {
            return val2;
        }
    }
}

namespace preprocessing {
    //float output_buffer[CYCLE_LENGTH];
    float keypoints[] = {18, 35, 52, 68, 85};
    float keypoints_divided_by_100[] = {0.18, 0.35, 0.52, 0.68, 0.85};
    float ascend_locations[] = {16, 33, 50, 66, 83};
    float descend_locations[] = {4, 20, 37, 54, 70, 87};
    std::vector<float> w_;
    std::vector<float> y_;
    std::vector<float> coeff;

    /*int resistance_log_and_aggregate(const float* initial, float* output_buffer) {
        for (int i = 0; i < CYCLE_LENGTH / 3; i++) {
            output_buffer[i] = max_of_3(log10(initial[i]), log10(initial[i+1]), log10(initial[i+2]));
        }
        return CYCLE_LENGTH / 3;
    }

    void polyfit(	const std::vector<float> &t,
            const std::vector<float> &v,
            std::vector<float> &coeff,
            int order

             )
    {
        // Create Matrix Placeholder of size n x k, n= number of datapoints, k = order of polynomial, for exame k = 3 for cubic polynomial
        Eigen::MatrixXf T(t.size(), order + 1);
        Eigen::VectorXf V = Eigen::VectorXf::Map(&v.front(), v.size());
        Eigen::VectorXf result;

        // check to make sure inputs are correct
        assert(t.size() == v.size());
        assert(t.size() >= order + 1);
        // Populate the matrix
        for(size_t i = 0 ; i < t.size(); ++i)
        {
            for(size_t j = 0; j < order + 1; ++j)
            {
                T(i, j) = pow(t.at(i), j);
            }
        }
        std::cout<<T<<std::endl;
        
        // Solve for linear least square fit
        result  = T.householderQr().solve(V);
        coeff.resize(order+1);
        for (int k = 0; k < order+1; k++)
        {
            coeff[k] = result[k];
        }

    }

    void poly_trend_remove(float* mutating_array) {
    }

    void peak_correction(float* initial, const int* ascend_locations, const int* descend_locations, int len_locations) {
        const int window = 3;
        // Здесь может быть баг, когда нет семи значений в окне, чтобы их обработать
        for (int i = 1; i < len_locations - 1; i++) {
            float descend_peak[window * 2 + 1];
            float min_descend_peak = min(initial, descend_locations[i], window);
            for (int j = 0; j < window * 2 + 1; j++) {
                descend_peak[j] = initial[descend_locations[i] + j - window] - min_descend_peak;
            }

            float ascend_peak[window * 2 + 1];
            float min_ascend_peak = min(initial, ascend_locations[i], window);
            for (int j = 0; j < window * 2 + 1; j++) {
                ascend_peak[j] = initial[ascend_locations[i] + j - window] - min_ascend_peak;
            }

            int argmax = 0;
            float max_descend_peak = max(descend_peak, 0, window, &argmax);
            if (max_descend_peak / descend_peak[argmax - 2] > 1.5) {
                
                int diff_argmax = 0;
                float diff_ascend_peak[window * 2];
                for (int k = 0; k < window * 2; k++) {
                    diff_ascend_peak[k] = ascend_peak[k + 1] - ascend_peak[k];
                }
                max(diff_ascend_peak, window * 2, &diff_argmax);


                int left_side = descend_locations[i] - window + argmax;
                int right_side = ascend_locations[i] - window + diff_argmax;

                for (int l = left_side; l <= right_side; l++) {
                    output_buffer[l] = initial[l] - (initial[left_side] - initial[left_side - 1]);

                }
            }
        }
    }*/

    /// @brief Perform preprocessing (scaling)
    /// @param buffer Buffer to work on
    void preprocess(float* buffer) {

        float min = 14;
        float max = 0;
        ESP_LOGI(TAG, "Invoking preprocessor...");
        for (size_t i = 0; i < 301; i++) {
            buffer[i] = log10(buffer[i]);
        }
        for (size_t i = 1; i < 300; i++) {
            if ((0 < buffer[i]) & (buffer[i] < min)) {
                min = buffer[i];
            }
        }
        for (size_t i = 0; i < 301; i++) {
            buffer[i] = buffer[i] - min;
        }
        for (size_t i = 1; i < 300; i++) {
            if ((max < buffer[i]) & (buffer[i] < 14)) {
                max = buffer[i];
            }
        }
        ESP_LOGI(TAG, "\tMin = %f; Max = %f", min, max);
        for (size_t i = 0; i < 301; i++) {
            buffer[i] = buffer[i] / max;
        }

        // w_.assign(output_buffer, output_buffer + sizeof(output_buffer) / 4);
        // y_.assign(output_buffer, output_buffer + sizeof(output_buffer) / 4);
        // polyfit(w_, y_, coeff, 3);
    }


}

