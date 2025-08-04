#pragma once
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <pcf8574.h>

#ifdef TAG
#undef TAG
#endif
#define TAG "PCF8574_HELPER"

static i2c_dev_t pcf_slave;

// Declare Mutex here to protect raw_isr with Semaphore
SemaphoreHandle_t exp_act_hilo_semaphore;
static TaskHandle_t exp_act_hilo_proc_task_hdl = NULL;


static void IRAM_ATTR pcf_int_handler(void* arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;  
    vTaskNotifyGiveFromISR(exp_act_hilo_proc_task_hdl,&xHigherPriorityTaskWoken);
}

esp_err_t initialize_io_expanders()
{
    // Semaphore to protect the raw reading from the 
    exp_act_hilo_semaphore = xSemaphoreCreateBinary();
    if(exp_act_hilo_semaphore == NULL)
    {
        ESP_LOGE(TAG,"Could not create active high/low semaphore.");
    }
    xSemaphoreGive(exp_act_hilo_semaphore);
    
    //Setting the interrupt on Pin (gpio_num_t)CONFIG_EXP_INT_PIN
    gpio_set_direction((gpio_num_t)CONFIG_EXP_INT_PIN,GPIO_MODE_INPUT);
    gpio_set_pull_mode((gpio_num_t)CONFIG_EXP_INT_PIN,GPIO_PULLUP_ONLY);
    gpio_pullup_en((gpio_num_t)CONFIG_EXP_INT_PIN);
    gpio_set_intr_type((gpio_num_t)CONFIG_EXP_INT_PIN,GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(ESP_INTR_FLAG_LOWMED);
    gpio_isr_handler_add((gpio_num_t)CONFIG_EXP_INT_PIN,pcf_int_handler,NULL);



    if (pcf8574_init_desc(&pcf_slave,CONFIG_PRIMARY_IO_EXPANDER_ADDRESS,(i2c_port_t)0,(gpio_num_t)CONFIG_SDA_PIN, (gpio_num_t)CONFIG_SCL_PIN) != ESP_OK)
    {
        ESP_LOGE(TAG,"Failed to initialize IO Expander");
    }
    ESP_LOGI(TAG,"IO Expander initialized OK");
    uint8_t raw;
    pcf8574_port_read(&pcf_slave,&raw);
    ESP_LOGI(TAG,"Expander value : %02X",raw);

    return ESP_OK;
}