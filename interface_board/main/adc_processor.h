#pragma once
#include "adc_helpers.h"
#include "sma_filter.h"

#ifdef TAG
#undef TAG
#endif
#define TAG "ADC Processor"

#define NUM_ADC_CHANNELS 4

uint8_t sma_sizes[NUM_ADC_CHANNELS] = {20,10,10,10};
uint32_t adc_sample_times[NUM_ADC_CHANNELS] = {1000,250,250,250};

typedef struct
{
    sma_handle_t *sma;
    TaskHandle_t adc_task;
    TaskHandle_t proc_task;
    uint8_t channel;
    uint32_t adc_sample_time_ms;
} adc_channel_ctx_t;

static adc_channel_ctx_t adc_channels[NUM_ADC_CHANNELS] = {0};

static SemaphoreHandle_t adc_to_sma_Semaphore;

// ADC to SMA sampling task
void adc_to_sma_task(void *pvParameters)
{
    adc_channel_ctx_t *ctx = (adc_channel_ctx_t *)pvParameters;
    while (1)
    {
        if (xSemaphoreTake(adc_to_sma_Semaphore, pdMS_TO_TICKS(1)) == pdTRUE)
        {
            int16_t adc_measurement = adc_measure_channel_raw(ctx->channel);
            sma_add(ctx->sma, adc_measurement);
        }
        xSemaphoreGive(adc_to_sma_Semaphore);
        vTaskDelay(pdMS_TO_TICKS(ctx->adc_sample_time_ms));
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
        adc_channels[i].adc_sample_time_ms = adc_sample_times[i];
        if (adc_channels[i].sma == NULL)
        {
            ESP_LOGE(TAG, "Could not initialize SMA for channel %d", i);
            return ESP_FAIL;
        }
        if (xTaskCreate(adc_to_sma_task, "ADC to SMA", 2048+1024, &adc_channels[i], 5, &adc_channels[i].adc_task) != pdPASS)
        {
            ESP_LOGE(TAG, "Could not create ADC to SMA tasks, aborting.");
            return ESP_FAIL;
        }
    }

    return ESP_OK;
}