#pragma once
#include <stdio.h>
#include "esp_log.h"
#include "driver/mcpwm_prelude.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef TAG
#undef TAG
#endif
#define TAG "MCPWM_CAP"

/// @brief Holder structure for the raw metrics of a pwm (single sample) and computed metrics
typedef struct
{
    uint32_t pos_edge_ts;      /*!< Timestamp of the last positive edge */
    uint32_t prev_pos_edge_ts; /*!< Timestamp of the previous positive edge */
    uint32_t period_ticks;     /*!< Period in ticks between two positive edges */
    uint32_t neg_edge_ts;      /*!< Timestamp of the last negative edge */
    uint32_t deltaT;           /*!< Time difference between the last negative and positive edge */
    float duty_cycle = 0.0;
    float frequency = 0.0;
} pwm_info_t;

esp_err_t compute_freq_dut(volatile pwm_info_t *pwm_info, uint32_t clock_Hz=80000000) {
    if (pwm_info->period_ticks == 0) {
        pwm_info->frequency = 0;
        pwm_info->duty_cycle = 0;
        return ESP_FAIL;
    }
    pwm_info->frequency = 1.0 / ((float)pwm_info->period_ticks / (float)(clock_Hz)); 
    pwm_info->duty_cycle = (float)pwm_info->deltaT / (float)pwm_info->period_ticks;
    return ESP_OK;  
}

/// @brief MCPWM generic capture ISR that updates a PWM data structure
/// @param cap_chan Capture channel
/// @param edata Event data
/// @param user_data Target PWM data structure to update
/// @return Always false because currently not triggering a context switch
static bool IRAM_ATTR mcpwm_capture_cb_generic(mcpwm_cap_channel_handle_t cap_chan, 
                                                const mcpwm_capture_event_data_t *edata, 
                                                void *user_data)
{
    pwm_info_t *target_pwm_signal = static_cast<pwm_info_t *>(user_data);

    // portENTER_CRITICAL_ISR(&counter_mux);
    if (edata->cap_edge == MCPWM_CAP_EDGE_POS)
    {
        target_pwm_signal->prev_pos_edge_ts = target_pwm_signal->pos_edge_ts;
        target_pwm_signal->pos_edge_ts = edata->cap_value;
        target_pwm_signal->period_ticks = target_pwm_signal->pos_edge_ts - target_pwm_signal->prev_pos_edge_ts;
    }
    else if (edata->cap_edge == MCPWM_CAP_EDGE_NEG)
    {

        target_pwm_signal->neg_edge_ts = edata->cap_value;
        target_pwm_signal->deltaT = target_pwm_signal->neg_edge_ts - target_pwm_signal->pos_edge_ts;
    }
    // portEXIT_CRITICAL_ISR(&counter_mux);
    return false;
}

/// @brief Creator and initialisator for a single MCPWM capture channel, using APB 80MHz clock
/// @param target_cap_chan Handle of the capture channel to initialise and start
/// @param cap_gpio Physical GPIO associated for capture
/// @param pwm_info_buffer Target PWM info buffer that will be updated by the capture callback
/// @return ESP_OK if all started correctly, ESP_FAIL otherwise (not implemented yet)
esp_err_t set_capture_channel(mcpwm_cap_channel_handle_t target_cap_chan, 
                                gpio_num_t cap_gpio, 
                                volatile pwm_info_t *pwm_info_buffer)
{
    // --- MCPWM Capture Setup ---
    static mcpwm_cap_timer_handle_t cap_timer = NULL;
    if (cap_timer == NULL)
    {
        ESP_LOGI(TAG, "Creating new capture timer");
        mcpwm_capture_timer_config_t cap_timer_config = {
            .group_id = 0,
            .clk_src = MCPWM_CAPTURE_CLK_SRC_DEFAULT,
        };
        mcpwm_new_capture_timer(&cap_timer_config, &cap_timer);
        mcpwm_capture_timer_enable(cap_timer);
        mcpwm_capture_timer_start(cap_timer);
        
    }
    else
    {
        ESP_LOGI(TAG, "Reusing existing capture timer");
    }

    // mcpwm_cap_channel_handle_t cap_chan = NULL;
    mcpwm_capture_channel_config_t cap_chan_config = {
        .gpio_num = cap_gpio,
        .intr_priority = 1,
        .prescale = 1,
        .flags = {
            .pos_edge = true, 
            .neg_edge = true, 
            .pull_up = false, 
            .pull_down = true, 
            .io_loop_back = false
        }
    };
    mcpwm_new_capture_channel(cap_timer, &cap_chan_config, &target_cap_chan);

    mcpwm_capture_event_callbacks_t cap_cbs = {
        .on_cap = mcpwm_capture_cb_generic};
    mcpwm_capture_channel_register_event_callbacks(target_cap_chan, &cap_cbs, (void *)pwm_info_buffer);

    mcpwm_capture_channel_enable(target_cap_chan);
    
   

    return ESP_OK;
}