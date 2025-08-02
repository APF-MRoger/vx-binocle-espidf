#pragma once
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <ads111x.h>

#ifdef TAG
#undef TAG
#endif
#define TAG "ADC_HELPER"

static i2c_dev_t adc_slave;

/// @brief Hardware initialisation function
/// @return ESP_OK if all went right, ESP_FAIL otherwise
esp_err_t initialize_ADC()
{

    if (ads111x_init_desc(&adc_slave, 0x48, (i2c_port_t)0, (gpio_num_t)CONFIG_SDA_PIN, (gpio_num_t)CONFIG_SCL_PIN) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize ADC");
    }
    ESP_LOGI(TAG, "Initialized ADC OK");

    if (ads111x_set_mode(&adc_slave, ADS111X_MODE_SINGLE_SHOT) != ESP_OK)
    {
        ESP_LOGE(TAG, "Could not set ADC mode.");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Set mode OK.");

    if (ads111x_set_data_rate(&adc_slave, ADS111X_DATA_RATE_128) != ESP_OK)
    {
        ESP_LOGE(TAG, "Could not set ADC data rate");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Set data rate OK.");

    if (ads111x_set_input_mux(&adc_slave, ADS111X_MUX_0_GND) != ESP_OK)
    {
        ESP_LOGE(TAG, "Could not set input mux configuration.");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Set mux to 0 OK.");

    if (ads111x_set_gain(&adc_slave, ADS111X_GAIN_4V096) != ESP_OK)
    {
        ESP_LOGE(TAG, "Could not set ADC gain");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "ADC ready");

    return ESP_OK;
}

/// @brief ADC returns raw int16_t measurement, mutex-protected under the wrap
/// @param channel_num Channel ID (0 to 3) to read from.
/// @return Raw unscaled measurement
int16_t adc_measure_channel_raw(uint8_t channel_num)
{
    int16_t raw_measurement = 0;
    switch (channel_num)
    {
    case 0:
        if (ads111x_set_input_mux(&adc_slave, ADS111X_MUX_0_GND) != ESP_OK)
        {
            ESP_LOGE(TAG, "Could not swith ADC Mux");
            return 0;
        }

        break;
    case 1:
        if (ads111x_set_input_mux(&adc_slave, ADS111X_MUX_1_GND) != ESP_OK)
        {
            ESP_LOGE(TAG, "Could not swith ADC Mux");
            return 0;
        }

        break;
    case 2:
        if (ads111x_set_input_mux(&adc_slave, ADS111X_MUX_2_GND) != ESP_OK)
        {
            ESP_LOGE(TAG, "Could not swith ADC Mux");
            return 0;
        }

        break;
    case 3:
        if (ads111x_set_input_mux(&adc_slave, ADS111X_MUX_3_GND) != ESP_OK)
        {
            ESP_LOGE(TAG, "Could not swith ADC Mux");
            return 0;
        }

        break;
    default:
        ESP_LOGE(TAG, "Invalid channel identifier.");
        return 0;
        break;
    }
    if(ads111x_start_conversion(&adc_slave)!=ESP_OK)
    {
        ESP_LOGE(TAG,"Could not start conversion");
        return 0;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
    if (ads111x_get_value(&adc_slave, &raw_measurement) != ESP_OK)
    {
        ESP_LOGE(TAG, "Could not get measurement");
        return 0;
    }

    ESP_LOGD(TAG, "Measured channel %u : raw %d", channel_num, raw_measurement);
    return raw_measurement;
}
