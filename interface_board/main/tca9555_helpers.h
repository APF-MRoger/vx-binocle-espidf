#pragma once
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <tca95x5.h>

#ifdef TAG
#undef TAG
#endif
#define TAG "TCA9555_HELPER"

static i2c_dev_t tca_slave;

// Declare Mutex here to protect raw_isr with Semaphore
SemaphoreHandle_t exp_act_high_low_sem;
static TaskHandle_t processor_task_hdl = NULL;

static void IRAM_ATTR tca_int_handler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (xSemaphoreTakeFromISR(exp_act_high_low_sem, &xHigherPriorityTaskWoken) == pdTRUE)
    {
        vTaskNotifyGiveFromISR(processor_task_hdl, &xHigherPriorityTaskWoken);
    }
    xSemaphoreGiveFromISR(exp_act_high_low_sem, &xHigherPriorityTaskWoken);
}

esp_err_t initialize_io_expanders()
{
    // Semaphore to protect the raw reading from the
    exp_act_high_low_sem = xSemaphoreCreateBinary();
    if (exp_act_high_low_sem == NULL)
    {
        ESP_LOGE(TAG, "Could not create active high/low semaphore.");
    }
    xSemaphoreGive(exp_act_high_low_sem);

    // Setting the interrupt on Pin (gpio_num_t)CONFIG_EXP_INT_PIN
    gpio_set_direction((gpio_num_t)CONFIG_EXP_INT_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode((gpio_num_t)CONFIG_EXP_INT_PIN, GPIO_PULLUP_ONLY);
    gpio_pullup_en((gpio_num_t)CONFIG_EXP_INT_PIN);
    gpio_set_intr_type((gpio_num_t)CONFIG_EXP_INT_PIN, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(ESP_INTR_FLAG_LOWMED);
    gpio_isr_handler_add((gpio_num_t)CONFIG_EXP_INT_PIN, tca_int_handler, NULL);

    if (tca95x5_init_desc(&tca_slave, CONFIG_PRIMARY_IO_EXPANDER_ADDRESS, (i2c_port_t)0, (gpio_num_t)CONFIG_SDA_PIN, (gpio_num_t)CONFIG_SCL_PIN) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize IO Expander");
    }
    ESP_LOGI(TAG, "IO Expander initialized OK");
    tca95x5_port_set_mode(&tca_slave, 0xFFFF);
    uint16_t raw;
    tca95x5_port_read(&tca_slave, &raw);
    ESP_LOGI(TAG, "Expander value : %02X", raw);

    return ESP_OK;
}