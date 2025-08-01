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

#define ADC_SMA_0_SAMPLE_TIME_MS 100
#define ADC_SMA_1_SAMPLE_TIME_MS 100
#define ADC_SMA_2_SAMPLE_TIME_MS 100
#define ADC_SMA_3_SAMPLE_TIME_MS 100

TaskHandle_t adc_to_sma_0_task = nullptr;
TaskHandle_t adc_to_sma_1_task = nullptr;
TaskHandle_t adc_to_sma_2_task = nullptr;
TaskHandle_t adc_to_sma_3_task = nullptr;

#define SMA_0_PROC_TIME_MS 1000
#define SMA_1_PROC_TIME_MS 1000
#define SMA_2_PROC_TIME_MS 1000
#define SMA_3_PROC_TIME_MS 1000

TaskHandle_t adc_sma_0_proc_task = nullptr;
TaskHandle_t adc_sma_1_proc_task = nullptr;
TaskHandle_t adc_sma_2_proc_task = nullptr;
TaskHandle_t adc_sma_3_proc_task = nullptr;

struct sma_output
{
    float unscaled;
    float scaled;
};

static struct sma_output smaOutputArray[4] = {{0,0}};

// ADC to SMA core function and wrapper tasks
void adc_to_sma_X(sma_handle_t* targetSMA, uint8_t targetChannel, uint32_t samplingTimeMS)
{
    while(true)
    {
        int16_t adc_measurement = adc_measure_channel_raw(targetChannel);
        sma_add(targetSMA,adc_measurement);
        vTaskDelay(pdMS_TO_TICKS(samplingTimeMS));
    }
}

void adc_to_sma_0(void* pvParameters)
{
    adc_to_sma_X(adc_sma_0,0,ADC_SMA_0_SAMPLE_TIME_MS);
}

void adc_to_sma_1(void* pvParameters)
{
    adc_to_sma_X(adc_sma_1,1,ADC_SMA_1_SAMPLE_TIME_MS);
}

void adc_to_sma_2(void* pvParameters)
{
    adc_to_sma_X(adc_sma_2,2,ADC_SMA_2_SAMPLE_TIME_MS);
}

void adc_to_sma_3(void* pvParameters)
{
    adc_to_sma_X(adc_sma_3,3,ADC_SMA_3_SAMPLE_TIME_MS);
}

void proc_sma_0(void* pvParameters)
{
    ads111x_gain_t chan_gain;
    ads111x_get_gain(&adc_slave,&chan_gain);
    while(1)
    {
        smaOutputArray[0].unscaled = sma_get_avg(adc_sma_0);
        smaOutputArray[0].unscaled = ads111x_gain_values[chan_gain] / ADS111X_MAX_VALUE * smaOutputArray[0].unscaled;
        vTaskDelay(pdMS_TO_TICKS(SMA_0_PROC_TIME_MS));
    }    
}

void proc_sma_1(void* pvParameters)
{
    ads111x_gain_t chan_gain;
    ads111x_get_gain(&adc_slave,&chan_gain);
    while(1)
    {
        smaOutputArray[1].unscaled = sma_get_avg(adc_sma_1);
        smaOutputArray[1].unscaled = ads111x_gain_values[chan_gain] / ADS111X_MAX_VALUE * smaOutputArray[1].unscaled;
        vTaskDelay(pdMS_TO_TICKS(SMA_1_PROC_TIME_MS));
    }    
}

void proc_sma_2(void* pvParameters)
{
    ads111x_gain_t chan_gain;
    ads111x_get_gain(&adc_slave,&chan_gain);
    while(1)
    {
        smaOutputArray[2].unscaled = sma_get_avg(adc_sma_2);
        smaOutputArray[2].unscaled = ads111x_gain_values[chan_gain] / ADS111X_MAX_VALUE * smaOutputArray[2].unscaled;
        vTaskDelay(pdMS_TO_TICKS(SMA_2_PROC_TIME_MS));
    }    
}

void proc_sma_3(void* pvParameters)
{
    ads111x_gain_t chan_gain;
    ads111x_get_gain(&adc_slave,&chan_gain);
    while(1)
    {
        smaOutputArray[3].unscaled = sma_get_avg(adc_sma_3);
        smaOutputArray[3].unscaled = ads111x_gain_values[chan_gain] / ADS111X_MAX_VALUE * smaOutputArray[3].unscaled;
        vTaskDelay(pdMS_TO_TICKS(SMA_3_PROC_TIME_MS));
    }    
}

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

    xTaskCreate(adc_to_sma_0,"ADC to SMA",3000,NULL,5,&adc_to_sma_0_task);
    xTaskCreate(adc_to_sma_1,"ADC to SMA",3000,NULL,5,&adc_to_sma_1_task);
    xTaskCreate(adc_to_sma_2,"ADC to SMA",3000,NULL,5,&adc_to_sma_2_task);
    xTaskCreate(adc_to_sma_3,"ADC to SMA",3000,NULL,5,&adc_to_sma_3_task);

    xTaskCreate(proc_sma_0,"Process SMA 0",2048,NULL,4,&adc_sma_0_proc_task);
    xTaskCreate(proc_sma_1,"Process SMA 1",2048,NULL,4,&adc_sma_1_proc_task);
    xTaskCreate(proc_sma_2,"Process SMA 2",2048,NULL,4,&adc_sma_2_proc_task);
    xTaskCreate(proc_sma_3,"Process SMA 3",2048,NULL,4,&adc_sma_3_proc_task);

    return ESP_OK;
}