#include <stdio.h>
#include "esp_log.h"
#include "driver/ledc.h"
#include "driver/mcpwm_prelude.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define PWM_GPIO          6
#define PWM_FREQ_HZ       500
#define PWM_DUTY_PCT      25

#define CAPTURE_GPIO      7
#define LOG_INTERVAL_MS   2000

static const char *TAG = "PWM_CAPTURE";

// Edge counters and protection
static volatile uint32_t pos_edge_count = 0;
static volatile uint32_t neg_edge_count = 0;
static portMUX_TYPE counter_mux = portMUX_INITIALIZER_UNLOCKED;

// ISR callback: count edges
static bool IRAM_ATTR mcpwm_capture_cb(mcpwm_cap_channel_handle_t cap_chan, const mcpwm_capture_event_data_t *edata, void *user_data)
{
    portENTER_CRITICAL_ISR(&counter_mux);
    if (edata->cap_edge == MCPWM_CAP_EDGE_POS) {
        pos_edge_count++;
    } else if (edata->cap_edge == MCPWM_CAP_EDGE_NEG) {
        neg_edge_count++;
    }
    portEXIT_CRITICAL_ISR(&counter_mux);
    return false;
}

extern "C" void app_main(void)
{
    // --- LEDC PWM Setup ---
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_8_BIT,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = PWM_FREQ_HZ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

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
    mcpwm_cap_timer_handle_t cap_timer = NULL;
    mcpwm_capture_timer_config_t cap_timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_CAPTURE_CLK_SRC_DEFAULT,
    };
    mcpwm_new_capture_timer(&cap_timer_config, &cap_timer);

    mcpwm_cap_channel_handle_t cap_chan = NULL;
    mcpwm_capture_channel_config_t cap_chan_config = {
        .gpio_num = CAPTURE_GPIO,
        .intr_priority = 1,
        .prescale = 1,
        .flags = { .pos_edge = true, .neg_edge = true, .pull_up = false, .pull_down = true, .io_loop_back = false }
    };
    mcpwm_new_capture_channel(cap_timer, &cap_chan_config, &cap_chan);

    mcpwm_capture_event_callbacks_t cap_cbs = {
        .on_cap = mcpwm_capture_cb
    };
    mcpwm_capture_channel_register_event_callbacks(cap_chan, &cap_cbs, NULL);

    mcpwm_capture_channel_enable(cap_chan);
    mcpwm_capture_timer_enable(cap_timer);
    mcpwm_capture_timer_start(cap_timer);

    // --- Logging Loop ---
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(LOG_INTERVAL_MS));
        uint32_t pos, neg;
        portENTER_CRITICAL(&counter_mux);
        pos = pos_edge_count;
        neg = neg_edge_count;
        pos_edge_count = 0;
        neg_edge_count = 0;
        portEXIT_CRITICAL(&counter_mux);

        ESP_LOGI(TAG, "Edges in last %d ms: POS=%lu, NEG=%lu", LOG_INTERVAL_MS, pos, neg);
    }
}