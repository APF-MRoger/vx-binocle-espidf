#pragma once
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_io_expander.hpp>
#include "gpio_defs.h"

#ifdef TAG
#undef TAG
#endif
#define TAG "GPIO_EXP_PRIMARY"

static esp_expander::Base *primary_expander = nullptr;

esp_err_t initialize_primary_expander()
{

    primary_expander = new esp_expander::HT8574(I2C_SCL_GPIO,I2C_SDA_GPIO,CONFIG_PRIMARY_IO_EXPANDER_ADDRESS);
    if(primary_expander->init() == false)
    {
        ESP_LOGE(TAG,"Failed to initialize primary IO expander");
        return ESP_FAIL;
    }
    if(primary_expander->begin() == false)
    {
        ESP_LOGE(TAG,"Failed to begin the primary IO expander");
        return ESP_FAIL;
    }
    primary_expander->printStatus();
    return ESP_OK;
}
