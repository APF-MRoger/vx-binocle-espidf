#include <stdio.h>
#include "gpio_defs.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include "driver/mcpwm_prelude.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mcpwm_capture_helpers.h"
// #include "gpio_exp_helper.h"

// #include "adc_helpers.h"
#include "adc_processor.h"
// #include "pcf8574_helpers.h"
#include "active_hi_low_processor.h"

#include "sma_filter.h"

#if CONFIG_DEBUG_GENERATE_PWM
#include "pwm_gen_helpers.h"
#endif

#define LOG_INTERVAL_MS 2000

#ifdef TAG
#undef TAG
#endif
#define TAG "MAIN"

// static portMUX_TYPE counter_mux = portMUX_INITIALIZER_UNLOCKED;

// PWM stats structures for coolant, rpm and speed captures
static volatile pwm_info_t pwm_cap_coolant, pwm_cap_rpm, pwm_cap_speed = {.pos_edge_ts = 0, .prev_pos_edge_ts = 0, .period_ticks = 0, .neg_edge_ts = 0, .deltaT = 0};

// Generator channels
#if CONFIG_DEBUG_GENERATE_PWM
ledc_channel_t pwm_gen_coolant = LEDC_CHANNEL_0;
ledc_channel_t pwm_gen_rpm = LEDC_CHANNEL_1;
ledc_channel_t pwm_gen_speed = LEDC_CHANNEL_2;
#endif
// Capture channels
mcpwm_cap_channel_handle_t cap_chan_coolant = NULL;
mcpwm_cap_channel_handle_t cap_chan_rpm = NULL;
mcpwm_cap_channel_handle_t cap_chan_speed = NULL;

TaskHandle_t base_slow_metrics_PKG_hdl;
TaskHandle_t base_fast_metrics_PKG_hdl;
TaskHandle_t base_active_hi_lo_PKG_hdl;

/// @brief Packaging task for base_slow_metrics
/// @param pvParameters 
void base_slow_metrics_PKG(void *pvParameters)
{
    // SMA-ed slower metrics : 
    // coolant_temp
    // fuel_level_pc
    // lv_voltage_v
    
    while(true)
    {
        // SMAs should already be protected, and some of the MCPWM logic can be brought in here.      
        ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(5000));
        if (compute_freq_dut(&pwm_cap_coolant)!=ESP_OK)
        {
            ESP_LOGW(TAG,"Could not compute frequency and duty for coolant");   
        }
        else
        {
            ESP_LOGI(TAG,"Coolant: %.2f - %.1f",pwm_cap_coolant.frequency,pwm_cap_coolant.duty_cycle);
        }
        // Already protected by SMA mutex
        ESP_LOGI(TAG,"Fuel : %.2f 12V: %.2f",sma_get_avg(adc_channels[0].sma),sma_get_avg(adc_channels[1].sma));
    }
}

/// @brief Packaging task for base_fast_metrics
/// @param pvParameters 
void base_fast_metrics_PKG(void *pvParameters)
{
    // Only faster metrics
    // speed_kph
    // rpm
    while(true)
    {
        // Transport some of the MCPWM logic in there
        ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(5000));
        if (compute_freq_dut(&pwm_cap_rpm) != ESP_OK )
        {
            ESP_LOGW(TAG,"Could not compute frequency and duty for RPM");
            continue;
        }
        if (compute_freq_dut(&pwm_cap_speed) != ESP_OK )
        {
            ESP_LOGW(TAG,"Could not compute frequency and duty for speed");
            continue;
        }
        ESP_LOGI(TAG,"RPM : %.2f - %.1f Speed: %.2f - %.1f",pwm_cap_rpm.frequency,pwm_cap_rpm.duty_cycle,pwm_cap_speed.frequency,pwm_cap_speed.duty_cycle);
        
    }
}

/// @brief Packaging task for base_active_hi_lo
/// @param pvParameters 
void base_active_hi_lo_PKG(void *pvParameters)
{
    // Will need to be using notifications and delay
    // Essentially direct from expander + a couple virtual telltales
    while(true)
    {
        ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(5000));
        if(xSemaphoreTake(exp_act_high_low_sem,pdMS_TO_TICKS(1)) == pdTRUE)
        {
            ESP_LOGI(TAG,"Ignition: %s",active_hi_lo_grp.AH_ignition ? "ON" : "OFF");
            ESP_LOGI(TAG,"Hi beams: %s",active_hi_lo_grp.AH_hi_beams ? "ON" : "OFF");
            ESP_LOGI(TAG,"Alternator: %s",active_hi_lo_grp.AL_alternator ? "ON" : "OFF");
            ESP_LOGI(TAG,"Brake low level: %s",active_hi_lo_grp.AL_brake_low ? "ON" : "OFF");
            ESP_LOGI(TAG,"Parking brake: %s",active_hi_lo_grp.AL_parking_brake ? "ON" : "OFF");
            ESP_LOGI(TAG,"Oil alarm: %s",active_hi_lo_grp.AL_oil_pressure ? "ON" : "OFF");
            ESP_LOGI(TAG,"Airbag: %s",active_hi_lo_grp.AL_airbag ? "ON" : "OFF");
            ESP_LOGI(TAG,"CEL: %s",active_hi_lo_grp.AL_CEL ? "ON" : "OFF");
            ESP_LOGI(TAG,"Right turn: %s",active_hi_lo_grp.AH_right_turn ? "ON" : "OFF");
            ESP_LOGI(TAG,"Left turn: %s",active_hi_lo_grp.AH_left_turn ? "ON" : "OFF");
            ESP_LOGI(TAG,"ABS: %s",active_hi_lo_grp.AL_ABS ? "ON" : "OFF");
            ESP_LOGI(TAG,"Door: %s",active_hi_lo_grp.AL_door ? "ON" : "OFF");
            ESP_LOGI(TAG,"Low coolant: %s",active_hi_lo_grp.AL_coolant_low ? "ON" : "OFF");
            ESP_LOGI(TAG,"Button: %s",active_hi_lo_grp.AL_button ? "ON" : "OFF");
            ESP_LOGI(TAG,"B07: %s",active_hi_lo_grp.AH_B07 ? "ON" : "OFF");
            ESP_LOGI(TAG,"Backlight: %s",active_hi_lo_grp.AH_backlight ? "ON" : "OFF");

            xSemaphoreGive(exp_act_high_low_sem);
        }
        
    }
}



//===========================================================================

// static sma_handle_t *chan0_sma;
// static sma_handle_t *chan1_sma;

// /**
//  * @brief Task to continuously sample the ADC and add values to the filter.
//  */
// void adc_sampling_task(void *pvParameters)
// {
//     while (1)
//     {
//         int16_t adc_value_0 = adc_measure_channel_raw(0);
//         int16_t adc_value_1 = adc_measure_channel_raw(1);

//         sma_add(chan0_sma, adc_value_0);
//         sma_add(chan1_sma, adc_value_1);

//         // Sample at a regular interval, e.g., every 100 ms
//         vTaskDelay(pdMS_TO_TICKS(100));
//     }
// }

// /**
//  * @brief Task to periodically calculate and use the moving average.
//  */
// void sma_processing_task(void *pvParameters)
// {
//     while (1)
//     {
//         //Realistically needs to be moved to another context
//         float average_0 = sma_get_avg(chan0_sma);
//         float average_1 = sma_get_avg(chan1_sma);
//         ads111x_gain_t chan_gain;
//         ads111x_get_gain(&adc_slave,&chan_gain);
//         float average_0_converted = ads111x_gain_values[chan_gain] / ADS111X_MAX_VALUE * average_0;
//         float average_1_converted = ads111x_gain_values[chan_gain] / ADS111X_MAX_VALUE * average_1;
//         ESP_LOGI(TAG, "Current moving average 0 : %.2f converted: %.2f V", average_0, average_0_converted);
//         ESP_LOGI(TAG, "Current moving average 1 : %.2f converted: %.2f V", average_1, average_1_converted);

//         // Process the average less frequently, e.g., every 1 second
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }

// ===========================================================================

extern "C" void app_main(void)
{

    // processor_task_hdl = xTaskGetCurrentTaskHandle();
    // To be removed later
#if CONFIG_DEBUG_GENERATE_PWM
    set_pwm_generator(LEDC_TIMER_0, CONFIG_COOLANT_PWM_BASE_FREQ_HZ, (gpio_num_t)CONFIG_COOLANT_PWM_GEN_GPIO, pwm_gen_coolant, CONFIG_COOLANT_PWM_BASE_DUTY_PCT);
    set_pwm_generator(LEDC_TIMER_1, CONFIG_RPM_PWM_BASE_FREQ_HZ, (gpio_num_t)CONFIG_RPM_PWM_GEN_GPIO, pwm_gen_rpm, CONFIG_RPM_PWM_BASE_DUTY_PCT);
    set_pwm_generator(LEDC_TIMER_2, CONFIG_SPEED_PWM_BASE_FREQ_HZ, (gpio_num_t)CONFIG_SPEED_PWM_GEN_GPIO, pwm_gen_speed, CONFIG_SPEED_PWM_BASE_DUTY_PCT);
#endif
    // Set up the capture channels
    set_capture_channel(cap_chan_coolant, (gpio_num_t)CONFIG_COOLANT_PWM_CAP_GPIO, &pwm_cap_coolant);
    set_capture_channel(cap_chan_rpm, (gpio_num_t)CONFIG_RPM_PWM_CAP_GPIO, &pwm_cap_rpm);
    set_capture_channel(cap_chan_speed, (gpio_num_t)CONFIG_SPEED_PWM_CAP_GPIO, &pwm_cap_speed);

    // Set up the IO Expander
    // initialize_expanders();
    i2cdev_init();
    // initialize_ADC();
    initialize_adc_processor();
    // initialize_io_expanders();
    initialize_exp_active_hi_lo_proc(&base_active_hi_lo_PKG_hdl);

    // gpio_dump_io_configuration(stdout,SOC_GPIO_VALID_GPIO_MASK);

    xTaskCreate(base_active_hi_lo_PKG,"Base AHL packager",2048,NULL,3,&base_active_hi_lo_PKG_hdl);
    xTaskCreate(base_slow_metrics_PKG,"Base Slow packager",2048,NULL,3,&base_slow_metrics_PKG_hdl);
    xTaskCreate(base_fast_metrics_PKG,"Base Fast packager",2048,NULL,3,&base_fast_metrics_PKG_hdl);
    //===================================================
    // // SMA init and tasks
    // chan0_sma = sma_init(20);
    // chan1_sma = sma_init(15);
    // if (chan0_sma == NULL || chan1_sma == NULL)
    // {
    //     ESP_LOGE(TAG, "Failed to initialize SMA objects.");
    //     return;
    // }

    // ESP_LOGI(TAG, "SMA filter initialized successfully.");

    // // Create the tasks
    // xTaskCreate(adc_sampling_task, "ADC Sampling Task", 4096, NULL, 5, NULL);
    // xTaskCreate(sma_processing_task, "SMA Processing Task", 4096, NULL, 5, NULL);
    // =================================================

    float channels_raw[NUM_ADC_CHANNELS] = {0.0};
    // --- Logging Loop ---
    while (1)
    {

        // for (int i = 0; i < 4; i++)
        // {
        //     ESP_LOGI(TAG,"SMA %u unscaled %.2f scaled %.2f V",i,smaOutputArray[i].unscaled,smaOutputArray[i].scaled);
        // }
        for (int i = 0; i < 4; i++)
        {
            channels_raw[i] = sma_get_avg(adc_channels[i].sma);
        }
        ESP_LOGI(TAG,"Channels raw : %.2f %.2f %.2f %.2f",channels_raw[0],channels_raw[1],channels_raw[2],channels_raw[3]);
        


        vTaskDelay(pdMS_TO_TICKS(5000));

        // adc_measure_channel_raw(0);

        // Only used to log MCPWM output
#ifdef CONFIG_LOOP_LOG_MCPWM
        vTaskDelay(pdMS_TO_TICKS(LOG_INTERVAL_MS));
        compute_freq_dut(&pwm_cap_coolant);
        compute_freq_dut(&pwm_cap_rpm);
        compute_freq_dut(&pwm_cap_speed);
        ESP_LOGI(TAG, "Coolant:\t %.2f Hz, %.1f%% \t|\tRPM:\t %.2f Hz, %.1f%% \t|\tSpeed:\t %.2f Hz, %.1f%%",
                 pwm_cap_coolant.frequency, pwm_cap_coolant.duty_cycle * 100.0,
                 pwm_cap_rpm.frequency, pwm_cap_rpm.duty_cycle * 100.0,
                 pwm_cap_speed.frequency, pwm_cap_speed.duty_cycle * 100.0);

        pwm_cap_coolant.deltaT = 0;
        pwm_cap_coolant.period_ticks = 0;
        pwm_cap_rpm.deltaT = 0;
        pwm_cap_rpm.period_ticks = 0;
        pwm_cap_speed.deltaT = 0;
        pwm_cap_speed.period_ticks = 0;
#endif
#if CONFIG_DEBUG_GENERATE_PWM
        start_duty_cycle = (start_duty_cycle + 1) % 99;
        if (change_duty_cycle(pwm_gen_coolant, start_duty_cycle + 1) != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to change coolant duty cycle");
        }
        start_frequency = (start_frequency + 1) % 996;
        change_frequency(pwm_gen_speed, 4 + start_frequency);
#endif

        // portEXIT_CRITICAL(&counter_mux);
    }
}