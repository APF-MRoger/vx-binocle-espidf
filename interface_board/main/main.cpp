#include <stdio.h>
#include "stdlib.h"
#include <math.h>
#include "esp_log.h"
#include "driver/ledc.h"
#include "driver/mcpwm_prelude.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"



#define PWM_GPIO          10
#define PWM_FREQ_HZ       50000    // Change as needed
#define PWM_DUTY_PCT      50      // Change as needed (0-100)

#define CAPTURE_GPIO      7
#define LOG_INTERVAL_MS   2000

#define MCPWM_CAPTURE_CLK_HZ 80*1000*1000  // 80 MHz fixed on ESP32-S3

static const char *TAG = "PWM_CAPTURE";

static volatile uint32_t last_rising_edge = 0;
static volatile uint32_t last_falling_edge = 0;
static volatile uint32_t period = 0;
static volatile uint32_t high_time = 0;

static bool IRAM_ATTR mcpwm_capture_cb(mcpwm_cap_channel_handle_t cap_chan, const mcpwm_capture_event_data_t *edata, void *user_data)
{
    static uint32_t prev_rising = 0;
    // React to edge type
    if (edata->cap_edge == MCPWM_CAP_EDGE_POS) { //React to positive edge
        // Periods are calculated on positive edges
        period = edata->cap_value - prev_rising;
        prev_rising = edata->cap_value;
        last_rising_edge = edata->cap_value;
    } else if (edata->cap_edge == MCPWM_CAP_EDGE_NEG) { // React to negative edge
        high_time = edata->cap_value - last_rising_edge;
        last_falling_edge = edata->cap_value;
    }
    return false;
}

extern "C" void app_main(void)
{
    // --- LEDC PWM Setup ---
    // Configure the timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_8_BIT,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = PWM_FREQ_HZ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // Configure the channel
    ledc_channel_config_t ledc_channel = {
        .gpio_num       = PWM_GPIO,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER_0,
        .duty           = (PWM_DUTY_PCT * 255) / 100,
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);




    // --- MCPWM Capture Setup ---
    // Create the Capture timer, resolution is automatically arbitrated
    mcpwm_cap_timer_handle_t cap_timer = NULL;
    mcpwm_capture_timer_config_t cap_timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_CAPTURE_CLK_SRC_DEFAULT,
    };

    if (mcpwm_new_capture_timer(&cap_timer_config, &cap_timer) != ESP_OK)
    {
        ESP_LOGI(TAG,"Failed to create capture timer");
    }


    // Initialise a new channel
    mcpwm_cap_channel_handle_t cap_chan = NULL;
    mcpwm_capture_channel_config_t cap_chan_config = {
        .gpio_num = CAPTURE_GPIO,
        .intr_priority = 1,
        .prescale = 80,
        .flags = { .pos_edge=true, .neg_edge = true, .pull_up = false, .pull_down = false }
    };
    if(mcpwm_new_capture_channel(cap_timer, &cap_chan_config, &cap_chan)!=ESP_OK)
    {
        ESP_LOGW(TAG,"Failed to create capture channel");
    }


    // Register a capture event callback
    mcpwm_capture_event_callbacks_t cap_cbs = {
        .on_cap = mcpwm_capture_cb
    };
    if(mcpwm_capture_channel_register_event_callbacks(cap_chan, &cap_cbs, NULL)!=ESP_OK)
    {
        ESP_LOGW(TAG,"Failed to register capture channel callback");
    }

    if(mcpwm_capture_channel_enable(cap_chan)!=ESP_OK)
    {
        ESP_LOGE(TAG,"Failed to enable capture channel");
    }
    if(mcpwm_capture_timer_enable(cap_timer)!=ESP_OK)
    {
        ESP_LOGE(TAG,"Failed to enable capture timer");
    }
    if(mcpwm_capture_timer_start(cap_timer)!=ESP_OK)
    {
        ESP_LOGE(TAG,"Failed to start capture timer");
    }

    // Test the capture channel
    // if (mcpwm_capture_channel_trigger_soft_catch(cap_chan)!=ESP_OK)
    // {
    //     ESP_LOGW(TAG,"Failed to soft trigger capture CB");
    // }
    
    ESP_LOGI(TAG,"LEDC clock: %d",ledc_timer.clk_cfg);
    // --- Logging Loop ---
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(LOG_INTERVAL_MS));
        uint32_t p, h;
        p = period;
        h = high_time;
        float freq = (p > 0) ? ((float)(MCPWM_CAPTURE_CLK_HZ) / p) : 0.0f;
        float duty = (p > 0) ? (100.0f * h / p) : 0.0f;
        ESP_LOGI(TAG, "Measured freq: %lu ticks %lu high time %.2f Hz, duty: %.2f%%", period,high_time, freq, duty);
        ESP_LOGI(TAG, "Last rising edge %lu , last falling edge %lu, ", last_rising_edge, last_falling_edge);
        period = 0;
        high_time = 0;
    }
}