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

esp_err_t initialize_ADC()
{
    i2cdev_init();

    if (ads111x_init_desc(&adc_slave, 0x48, (i2c_port_t)0, (gpio_num_t)CONFIG_SDA_PIN, (gpio_num_t)CONFIG_SCL_PIN) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize ADC");
    }
    ESP_LOGI(TAG, "Initialized ADC OK");
    ads111x_set_mode(&adc_slave, ADS111X_MODE_CONTINUOUS);
    ESP_LOGI(TAG, "Set mode OK.");
    ads111x_set_data_rate(&adc_slave, ADS111X_DATA_RATE_128);
    ESP_LOGI(TAG, "Set data rate OK.");
    ads111x_set_input_mux(&adc_slave, ADS111X_MUX_0_GND);
    ESP_LOGI(TAG, "Set mux OK.");
    ads111x_set_gain(&adc_slave, ADS111X_GAIN_4V096);
    ESP_LOGI(TAG, "ADC ready");

    int16_t raw_measurement = 0;
    for (int i = 0; i < 100; i++)
    {
        while (ads111x_get_value(&adc_slave, &raw_measurement) != ESP_OK)
        {
            ESP_LOGI(TAG, "Waiting for measurement");
        }
        ESP_LOGI(TAG, "Measurement (raw) %d", raw_measurement);
    }

    return ESP_OK;
}