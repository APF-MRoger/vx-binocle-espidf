#include "twai_daemon.h"

const char *TAG = "CAN Daemon";



esp_err_t initCAN(frameDispatcher_t *frameDispatcher)
{
    #ifdef TWAI_WATCHDOG
// Set up alerts filter
uint32_t alerts_to_enable = TWAI_ALERT_RX_DATA |
                            TWAI_ALERT_TX_FAILED |
                            TWAI_ALERT_ERR_PASS |
                            TWAI_ALERT_BUS_ERROR |
                            TWAI_ALERT_RX_QUEUE_FULL |
                            TWAI_ALERT_ARB_LOST;
uint32_t twai_alerts_triggered;
twai_status_info_t twai_status;
unsigned long twai_wdg_rx_dropped = 0;
unsigned long twai_wdg_rx_dropped_prev = 0;
unsigned long twai_wdg_rx_dropped_rate = 0;
#else
uint32_t alerts_to_enable = TWAI_ALERT_NONE;
#endif
    
    
    
    twai_general_config_t g_config = {
        .mode = TWAI_MODE_NORMAL,
        .tx_io = (gpio_num_t)CAN_TX,
        .rx_io = (gpio_num_t)CAN_RX,
        .clkout_io = TWAI_IO_UNUSED,
        .bus_off_io = TWAI_IO_UNUSED,
        .tx_queue_len = 10,
        .rx_queue_len = 256,
        .alerts_enabled = alerts_to_enable,
        .clkout_divider = 0,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        if (twai_start() != ESP_OK)
        {
            ESP_LOGE(TAG,"Failed to start TWAI driver");
            return ESP_FAIL;
        }
        else
        {
            ESP_LOGI(TAG,"TWAI driver started successfully");
        }
    }
    else
    {
        ESP_LOGE(TAG,"Failed to install TWAI driver");
        return ESP_FAIL;
    }
    BaseType_t twai_core_id = 0;
    BaseType_t ret =xTaskCreatePinnedToCore(CANTask, "twai_daemon", 4096,NULL,5,&CANTaskHandle,twai_core_id); 
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create TWAI task");
        return ESP_FAIL;
    } else {
        ESP_LOGI(TAG, "TWAI task created successfully");
    }
    
    dispatchCANFrame = frameDispatcher;
    if (dispatchCANFrame == nullptr) {
        ESP_LOGE(TAG, "Frame dispatcher function does not exist!");
        return ESP_ERR_INVALID_ARG;
    }
    // All checks passed
    return ESP_OK;
}

void CANTask(void *arg)
{
    ESP_LOGI(TAG, "CANTask has started");
    static twai_message_t rxMessage;
    // Add timeout variable ?

    while (true)
    {
        while(twai_receive(&rxMessage, pdMS_TO_TICKS(0)) == ESP_OK)
        {
            if(dispatchCANFrame(&rxMessage) != ESP_OK)
            {
                ESP_LOGW(TAG, "Frame dispatcher returned an error");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(CAN_POLL_MS));
    }

}
