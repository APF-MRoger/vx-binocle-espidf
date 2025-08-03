#pragma once
#include "adc_helpers.h"
#include "sma_filter.h"


#ifdef TAG
#undef TAG
#endif
#define TAG "ADC Processor"

#define NUM_ADC_CHANNELS 4

uint8_t sma_sizes[NUM_ADC_CHANNELS] = {CONFIG_SMA_0_SIZE,CONFIG_SMA_1_SIZE,CONFIG_SMA_2_SIZE,CONFIG_SMA_3_SIZE};


typedef struct
{
    sma_handle_t *sma;
    
    uint8_t channel;
} adc_channel_ctx_t;

static adc_channel_ctx_t adc_channels[NUM_ADC_CHANNELS] = {0};
TaskHandle_t adc_to_sma_handle;
static SemaphoreHandle_t adc_to_sma_Semaphore;

// ADC to SMA sampling task
void adc_to_sma_task(void *pvParameters)
{

    while (1)
    {
        if (xSemaphoreTake(adc_to_sma_Semaphore, pdMS_TO_TICKS(1)) == pdTRUE)
        {
            for (int i = 0; i < NUM_ADC_CHANNELS; i++)
            {
                int16_t adc_measurement = adc_measure_channel_raw(adc_channels[i].channel);
                sma_add(adc_channels[i].sma, adc_measurement);
            }
            xSemaphoreGive(adc_to_sma_Semaphore);
        }
        else
        {
            ESP_LOGW(TAG,"Already sampling from ADC to SMA !");
        }
        vTaskDelay(pdMS_TO_TICKS((NUM_ADC_CHANNELS*conversion_interval_ms>CONFIG_ADC_TO_SMA_POLLING_RATE_MS)?NUM_ADC_CHANNELS*conversion_interval_ms:CONFIG_ADC_TO_SMA_POLLING_RATE_MS));
    }
}

esp_err_t initialize_adc_processor()
{
    adc_to_sma_Semaphore = xSemaphoreCreateBinary();
    if (adc_to_sma_Semaphore == NULL)
    {
        ESP_LOGE(TAG, "Could not create ADC SMA Semaphore, aborting.");
        return ESP_FAIL;
    }
    xSemaphoreGive(adc_to_sma_Semaphore);

    if (initialize_ADC() != ESP_OK)
    {
        ESP_LOGE(TAG, "Could not initialize ADC unit.");
        return ESP_FAIL;
    }

    for (uint8_t i = 0; i < NUM_ADC_CHANNELS; ++i)
    {
        adc_channels[i].channel = i;
        uint16_t startValue = adc_measure_channel_raw(i);
        adc_channels[i].sma = sma_init_full(sma_sizes[i], startValue);

        if (adc_channels[i].sma == NULL)
        {
            ESP_LOGE(TAG, "Could not initialize SMA for channel %d", i);
            return ESP_FAIL;
        }
        
    }

    if(CONFIG_ADC_TO_SMA_POLLING_RATE_MS<NUM_ADC_CHANNELS*conversion_interval_ms)
    {
        ESP_LOGW(TAG,"ADC to SMA polling rate is faster than channel acquisition loop time. Actual polling rate will be %lu ms",NUM_ADC_CHANNELS*conversion_interval_ms);
    }


    if (xTaskCreate(adc_to_sma_task, "ADC to SMA", 2048+1024, NULL, 5, &adc_to_sma_handle) != pdPASS)
        {
            ESP_LOGE(TAG, "Could not create ADC to SMA task, aborting.");
            return ESP_FAIL;
        }

    return ESP_OK;
}