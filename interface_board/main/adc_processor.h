#pragma once
#include "adc_helpers.h"
#include "sma_filter.h"

#ifdef TAG
#undef TAG
#endif

#define TAG "ADC Processor"

#define ADC_SMA_0_SIZE 20
#define ADC_SMA_1_SIZE 20
#define ADC_SMA_2_SIZE 20
#define ADC_SMA_3_SIZE 20

static sma_handle_t* adc_sma_0 = nullptr;
static sma_handle_t* adc_sma_1 = nullptr;
static sma_handle_t* adc_sma_2 = nullptr;
static sma_handle_t* adc_sma_3 = nullptr;


esp_err_t initialize_adc_processor()
{
    if( initialize_ADC() != ESP_OK)
    {
        ESP_LOGE(TAG,"Could not initialize ADC unit.");
        return ESP_FAIL;
    }

    adc_sma_0 = sma_init(ADC_SMA_0_SIZE);
    adc_sma_1 = sma_init(ADC_SMA_1_SIZE);
    adc_sma_2 = sma_init(ADC_SMA_2_SIZE);
    adc_sma_3 = sma_init(ADC_SMA_3_SIZE);

    if (adc_sma_0 == NULL || adc_sma_1 == NULL || adc_sma_2 == NULL || adc_sma_3 == NULL)
    {
        ESP_LOGE(TAG,"Could not initialize the SMAs");
        return ESP_FAIL;
    }


    return ESP_OK;
}