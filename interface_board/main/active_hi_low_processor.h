// Uses IO expander interrupt and other GPIO interrupts to trigger update of the internal signals
// Uses task notification from interrupts to read the IO expander 
#pragma once
#include "pcf8574_helpers.h"

#ifdef TAG
#undef TAG
#endif

#define TAG "ActHiLo Processor"

// Should move to KConfig later
#define AUTO_READ_EXP_IO_MS 200

/// @brief Notified from ISR IO-registry read task.
/// @param pvParameters 
void exp_active_hi_lo_process(void *pvParameters)
{
    while(true)
    {
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(AUTO_READ_EXP_IO_MS));
            if(pcf8574_port_read(&pcf_slave, &raw_isr) !=ESP_OK)
            {
                ESP_LOGE(TAG,"Impossible to fetch register from expander");
            }
            else 
            {
                ESP_LOGD(TAG, "Raw ISR: %02X", raw_isr);
            }
            
    }
}


esp_err_t initialize_exp_active_hi_lo_proc()
{
    if(initialize_io_expanders() != ESP_OK)
    {
        return ESP_FAIL;
    }
    
    if (xTaskCreate(exp_active_hi_lo_process,"Expander ActHL processor", 2048,NULL,6,&processor_task_hdl) != pdPASS)
    {
        ESP_LOGE(TAG,"Could not create exp IO processor task.");
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

