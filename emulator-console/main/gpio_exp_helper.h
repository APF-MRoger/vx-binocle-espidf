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
#define TAG "GPIO_EXP_HELPER"

static esp_expander::Base *expanders[3] = {nullptr};

esp_err_t initialize_expanders()
{

    
    expanders[0] = new esp_expander::HT8574(I2C_SCL_GPIO,I2C_SDA_GPIO,CONFIG_PRIMARY_IO_EXPANDER_ADDRESS);
    if(expanders[0]->init() == false)
    {
        ESP_LOGE(TAG,"Failed to initialize primary IO expander");
        return ESP_FAIL;
    }
    if(expanders[0]->begin() == false)
    {
        ESP_LOGE(TAG,"Failed to begin the primary IO expander");
        return ESP_FAIL;
    }
    expanders[0]->printStatus();
    return ESP_OK;
}
