#include <stdio.h>
#include "gpio_defs.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include "driver/mcpwm_prelude.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// #define PWM_GPIO 6
#define PWM_FREQ_HZ 4
#define PWM_DUTY_PCT 33

// #define CAPTURE_GPIO 7
#define LOG_INTERVAL_MS 2000

static const char *TAG = "PWM_CAPTURE";

// Edge counters and protection

typedef struct
{
    uint32_t pos_edge_ts;      /*!< Timestamp of the last positive edge */
    uint32_t prev_pos_edge_ts; /*!< Timestamp of the previous positive edge */
    uint32_t period_ticks;     /*!< Period in ticks between two positive edges */
    uint32_t neg_edge_ts;      /*!< Timestamp of the last negative edge */
    uint32_t deltaT;           /*!< Time difference between the last negative and positive edge */
} pwm_info_t;

// static volatile uint32_t pos_edge_ts = 0;
// static volatile uint32_t prev_pos_edge_ts = 0;
// static volatile uint32_t period_ticks = 0;

// static volatile uint32_t neg_edge_ts = 0;
// static volatile uint32_t deltaT = 0;
// static portMUX_TYPE counter_mux = portMUX_INITIALIZER_UNLOCKED;

static volatile pwm_info_t pwm_cap_coolant, pwm_cap_rpm, pwm_cap_speed = {.pos_edge_ts = 0, .prev_pos_edge_ts = 0, .period_ticks = 0, .neg_edge_ts = 0, .deltaT = 0};

// ISR callback: count edges
static bool IRAM_ATTR mcpwm_capture_cb_coolant(mcpwm_cap_channel_handle_t cap_chan, const mcpwm_capture_event_data_t *edata, void *user_data)
{
    // portENTER_CRITICAL_ISR(&counter_mux);
    if (edata->cap_edge == MCPWM_CAP_EDGE_POS)
    {
        pwm_cap_coolant.prev_pos_edge_ts = pwm_cap_coolant.pos_edge_ts;
        pwm_cap_coolant.pos_edge_ts = edata->cap_value;
        pwm_cap_coolant.period_ticks = pwm_cap_coolant.pos_edge_ts - pwm_cap_coolant.prev_pos_edge_ts;
    }
    else if (edata->cap_edge == MCPWM_CAP_EDGE_NEG)
    {

        pwm_cap_coolant.neg_edge_ts = edata->cap_value;
        pwm_cap_coolant.deltaT = pwm_cap_coolant.neg_edge_ts - pwm_cap_coolant.pos_edge_ts;
    }
    // portEXIT_CRITICAL_ISR(&counter_mux);
    return false;
}

extern "C" void app_main(void)
{
    // --- LEDC PWM Setup ---
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_14_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK};
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .gpio_num = COOLANT_PWM_GEN_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = (PWM_DUTY_PCT * ((uint32_t)1 << (uint32_t)(ledc_timer.duty_resolution))) / 100,
        .hpoint = 0};
    ledc_channel_config(&ledc_channel);

    ESP_LOGI(TAG, "Actual duty is %lu", ledc_channel.duty);

    // --- MCPWM Capture Setup ---
    mcpwm_cap_timer_handle_t cap_timer = NULL;
    mcpwm_capture_timer_config_t cap_timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_CAPTURE_CLK_SRC_DEFAULT,
    };
    mcpwm_new_capture_timer(&cap_timer_config, &cap_timer);

    mcpwm_cap_channel_handle_t cap_chan = NULL;
    mcpwm_capture_channel_config_t cap_chan_config = {
        .gpio_num = COOLANT_PWM_CAP_GPIO,
        .intr_priority = 1,
        .prescale = 1,
        .flags = {.pos_edge = true, .neg_edge = true, .pull_up = false, .pull_down = true, .io_loop_back = false}};
    mcpwm_new_capture_channel(cap_timer, &cap_chan_config, &cap_chan);

    mcpwm_capture_event_callbacks_t cap_cbs = {
        .on_cap = mcpwm_capture_cb_coolant};
    mcpwm_capture_channel_register_event_callbacks(cap_chan, &cap_cbs, NULL);

    mcpwm_capture_channel_enable(cap_chan);
    mcpwm_capture_timer_enable(cap_timer);
    mcpwm_capture_timer_start(cap_timer);

    // --- Logging Loop ---
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(LOG_INTERVAL_MS));
        uint32_t delta, period;
        // portENTER_CRITICAL(&counter_mux);
        ESP_LOGI(TAG, "Last %d ms:  delta: %.2f ns, period: %.2f ns, Duty: %.2f", LOG_INTERVAL_MS, (float)(pwm_cap_coolant.deltaT / 80.0), (float)(pwm_cap_coolant.period_ticks / 80.0), (float)pwm_cap_coolant.deltaT / (float)pwm_cap_coolant.period_ticks);

        pwm_cap_coolant.deltaT = 0;
        pwm_cap_coolant.period_ticks = 0;
        // portEXIT_CRITICAL(&counter_mux);
    }
}