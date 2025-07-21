#include <stdio.h>
#include "gpio_defs.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include "driver/mcpwm_prelude.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define COOLANT_PWM_BASE_FREQ_HZ 100
#define COOLANT_PWM_BASE_DUTY_PCT 33
#define RPM_PWM_BASE_FREQ_HZ 4
#define RPM_PWM_BASE_DUTY_PCT 25
#define SPEED_PWM_BASE_FREQ_HZ 1000
#define SPEED_PWM_BASE_DUTY_PCT 10

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
    float duty_cycle;
    float frequency;
} pwm_info_t;

esp_err_t compute_freq_dut(volatile pwm_info_t *pwm_info) {
    if (pwm_info->period_ticks == 0) {
        pwm_info->frequency = 0;
        pwm_info->duty_cycle = 0;
        return ESP_FAIL;
    }
    pwm_info->frequency = 10*1000*1000.0 / ((float)pwm_info->period_ticks / 80.0); // Assuming 80MHz clock
    pwm_info->duty_cycle = (float)pwm_info->deltaT / (float)pwm_info->period_ticks;
    return ESP_OK;  
}

// static portMUX_TYPE counter_mux = portMUX_INITIALIZER_UNLOCKED;

static volatile pwm_info_t pwm_cap_coolant, pwm_cap_rpm, pwm_cap_speed = {.pos_edge_ts = 0, .prev_pos_edge_ts = 0, .period_ticks = 0, .neg_edge_ts = 0, .deltaT = 0};

// Generator channels
ledc_channel_t pwm_gen_coolant = LEDC_CHANNEL_0;
ledc_channel_t pwm_gen_rpm = LEDC_CHANNEL_1;
ledc_channel_t pwm_gen_speed = LEDC_CHANNEL_2;

// Capture channels
mcpwm_cap_channel_handle_t cap_chan_coolant = NULL;
mcpwm_cap_channel_handle_t cap_chan_rpm = NULL;
mcpwm_cap_channel_handle_t cap_chan_speed = NULL;

// ISR callback for the MCPWM capture channel
static bool IRAM_ATTR mcpwm_capture_cb_generic(mcpwm_cap_channel_handle_t cap_chan, const mcpwm_capture_event_data_t *edata, void *user_data)
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

esp_err_t set_pwm_generator(ledc_timer_t timer_num, uint32_t base_freq_hz, gpio_num_t output_gpio, ledc_channel_t channel, uint8_t duty_pc, ledc_clk_cfg_t clk_cfg = LEDC_USE_XTAL_CLK)
{
    // --- LEDC PWM Setup ---
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_14_BIT,
        .timer_num = timer_num,
        .freq_hz = base_freq_hz,
        .clk_cfg = clk_cfg};
    if (ledc_timer_config(&ledc_timer) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to configure LEDC timer");
        return ESP_FAIL;
    }
    // --- LEDC Channel Setup ---
    ledc_channel_config_t ledc_channel = {
        .gpio_num = output_gpio,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = channel,
        .timer_sel = timer_num,
        .duty = (duty_pc * ((uint32_t)1 << (uint32_t)(ledc_timer.duty_resolution))) / 100,
        .hpoint = 0};
    if (ledc_channel_config(&ledc_channel) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to configure LEDC channel");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Ledc channel %d set up on GPIO %d", channel, (int)output_gpio);
    return ESP_OK;
}

esp_err_t set_capture_channel(mcpwm_cap_channel_handle_t target_cap_chan, gpio_num_t cap_gpio, volatile pwm_info_t *pwm_info_buffer)
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
        .flags = {.pos_edge = true, .neg_edge = true, .pull_up = false, .pull_down = true, .io_loop_back = false}};
    mcpwm_new_capture_channel(cap_timer, &cap_chan_config, &target_cap_chan);

    mcpwm_capture_event_callbacks_t cap_cbs = {
        .on_cap = mcpwm_capture_cb_generic};
    mcpwm_capture_channel_register_event_callbacks(target_cap_chan, &cap_cbs, (void *)pwm_info_buffer);

    mcpwm_capture_channel_enable(target_cap_chan);
    
   

    return ESP_OK;
}

esp_err_t change_duty_cycle(ledc_channel_t channel, uint8_t duty_pc)
{
    // // Get the timer used by this channel
    // ledc_channel_config_t ledc_channel;
    // ledc_channel.channel = channel;
    // ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
    // // Query timer selection
    // ledc_timer_t timer_sel = LEDC_TIMER_0;
    // switch (channel) {
    //     case LEDC_CHANNEL_0: timer_sel = LEDC_TIMER_0; break;
    //     case LEDC_CHANNEL_1: timer_sel = LEDC_TIMER_1; break;
    //     case LEDC_CHANNEL_2: timer_sel = LEDC_TIMER_2; break;
    //     default: break;
    // }
    // Assume 14-bit resolution as used in set_pwm_generator
    //(duty_pc * ((uint32_t)1 << (uint32_t)(ledc_timer.duty_resolution))) / 100
    uint32_t duty = (duty_pc * ((uint32_t)1 << (uint32_t)(14))) / 100;
    esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, duty);
    if (err != ESP_OK) return err;
    return ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
}

esp_err_t change_frequency(ledc_channel_t channel, uint32_t freq_hz)
{
    // Find the timer associated with the channel
    ledc_timer_t timer_sel = LEDC_TIMER_0;
    switch (channel) {
        case LEDC_CHANNEL_0: timer_sel = LEDC_TIMER_0; break;
        case LEDC_CHANNEL_1: timer_sel = LEDC_TIMER_1; break;
        case LEDC_CHANNEL_2: timer_sel = LEDC_TIMER_2; break;
        default: break;
    }
    return ledc_set_freq(LEDC_LOW_SPEED_MODE,timer_sel,freq_hz);

}

extern "C" void app_main(void)
{

    set_pwm_generator(LEDC_TIMER_0, COOLANT_PWM_BASE_FREQ_HZ, (gpio_num_t)COOLANT_PWM_GEN_GPIO, pwm_gen_coolant, COOLANT_PWM_BASE_DUTY_PCT);
    set_pwm_generator(LEDC_TIMER_1, RPM_PWM_BASE_FREQ_HZ, (gpio_num_t)RPM_PWM_GEN_GPIO, pwm_gen_rpm, RPM_PWM_BASE_DUTY_PCT);
    set_pwm_generator(LEDC_TIMER_2, SPEED_PWM_BASE_FREQ_HZ, (gpio_num_t)SPEED_PWM_GEN_GPIO, pwm_gen_speed, SPEED_PWM_BASE_DUTY_PCT);

    set_capture_channel(cap_chan_coolant, (gpio_num_t)COOLANT_PWM_CAP_GPIO, &pwm_cap_coolant);
    set_capture_channel(cap_chan_rpm, (gpio_num_t)RPM_PWM_CAP_GPIO, &pwm_cap_rpm);
    set_capture_channel(cap_chan_speed, (gpio_num_t)SPEED_PWM_CAP_GPIO, &pwm_cap_speed);

    // --- Logging Loop ---
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(LOG_INTERVAL_MS));
        // uint32_t delta, period;
        // portENTER_CRITICAL(&counter_mux);
        // ESP_LOGI(TAG, "Coolant\t| delta: %.2f ns, period: %.2f ns, Duty: %.2f", (float)(pwm_cap_coolant.deltaT / 80.0), (float)(pwm_cap_coolant.period_ticks / 80.0), (float)pwm_cap_coolant.deltaT / (float)pwm_cap_coolant.period_ticks);
        // ESP_LOGI(TAG, "RPM\t| delta: %.2f ns, period: %.2f ns, Duty: %.2f", (float)(pwm_cap_rpm.deltaT / 80.0), (float)(pwm_cap_rpm.period_ticks / 80.0), (float)pwm_cap_rpm.deltaT / (float)pwm_cap_rpm.period_ticks);
        // ESP_LOGI(TAG, "Speed\t| delta: %.2f ns, period: %.2f ns, Duty: %.2f", (float)(pwm_cap_speed.deltaT / 80.0), (float)(pwm_cap_speed.period_ticks / 80.0), (float)pwm_cap_speed.deltaT / (float)pwm_cap_speed.period_ticks);
        compute_freq_dut(&pwm_cap_coolant);
        compute_freq_dut(&pwm_cap_rpm);
        compute_freq_dut(&pwm_cap_speed);
        ESP_LOGI(TAG, "Coolant:\t %.2f Hz, %.1f%% \t|\tRPM:\t %.2f Hz, %.1f%% \t|\tSpeed:\t %.2f Hz, %.1f%%",
            pwm_cap_coolant.frequency,  pwm_cap_coolant.duty_cycle * 100.0,
            pwm_cap_rpm.frequency,      pwm_cap_rpm.duty_cycle * 100.0,
            pwm_cap_speed.frequency,    pwm_cap_speed.duty_cycle * 100.0);
        
        pwm_cap_coolant.deltaT = 0;
        pwm_cap_coolant.period_ticks = 0;
        pwm_cap_rpm.deltaT = 0;
        pwm_cap_rpm.period_ticks = 0;
        pwm_cap_speed.deltaT = 0;
        pwm_cap_speed.period_ticks = 0;

        if(change_duty_cycle(pwm_gen_coolant, ((uint8_t)(pwm_cap_coolant.duty_cycle * 100)+1) % 100) != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to change coolant duty cycle");
        }
        change_frequency(pwm_gen_speed,(uint32_t)(pwm_cap_speed.frequency+1) % 996 + 4);
        // portEXIT_CRITICAL(&counter_mux);
    }
}