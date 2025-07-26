#pragma once
#include <stdio.h>
#include "esp_log.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef TAG
#undef TAG
#endif
#define TAG "PWM_GEN"

#define COOLANT_PWM_BASE_FREQ_HZ 100
#define COOLANT_PWM_BASE_DUTY_PCT 10
#define RPM_PWM_BASE_FREQ_HZ 3
#define RPM_PWM_BASE_DUTY_PCT 50
#define SPEED_PWM_BASE_FREQ_HZ 3
#define SPEED_PWM_BASE_DUTY_PCT 50

static bool active_timers[3] = {false};

static uint32_t duty_resolutions_bit[3] = {0};

/// @brief Creator and initialisator function for the PWM sources (mostly used on the emulator board)
/// @param timer_num Identifier of the timer used
/// @param base_freq_hz Starting (or base) frequency, in Herz. Note that these are entire Hz steps
/// @param output_gpio GPIO pin used to output the PWM signal
/// @param channel Ledc channel that will be associated with this generator
/// @param duty_pc Start duty cycle, in pc (entire pcts)
/// @param clk_cfg Clock source. XTAL clock by default because of the low frequency range.
/// @return ESP_OK if all set correctly, ESP_FAIL otherwise.
esp_err_t set_pwm_generator(ledc_timer_t timer_num,
                            uint32_t base_freq_hz,
                            gpio_num_t output_gpio,
                            ledc_channel_t channel,
                            uint8_t duty_pc,
                            ledc_clk_cfg_t clk_cfg = LEDC_USE_RC_FAST_CLK)
{
    // --- LEDC PWM Setup ---
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = (ledc_timer_bit_t)ledc_find_suitable_duty_resolution(SOC_CLK_RC_FAST_FREQ_APPROX, base_freq_hz),
        .timer_num = timer_num,
        .freq_hz = base_freq_hz,
        .clk_cfg = clk_cfg,
        .deconfigure = false};
    duty_resolutions_bit[(uint32_t)(channel)] = (uint32_t)(ledc_timer.duty_resolution);
    ESP_LOGI(TAG, "Channel %lu : Max duty resolution to use: %lu", (uint32_t)(channel), ledc_find_suitable_duty_resolution(SOC_CLK_RC_FAST_FREQ_APPROX, base_freq_hz));
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
        .duty = (duty_pc * (((uint32_t)1 << (uint32_t)(ledc_timer.duty_resolution)) - 1)) / 100,
        .hpoint = 0};
    if (ledc_channel_config(&ledc_channel) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to configure LEDC channel");
        return ESP_FAIL;
    }
    active_timers[(int)timer_num] = true;
    ESP_LOGI(TAG, "Ledc channel %d set up on GPIO %d", channel, (int)output_gpio);
    return ESP_OK;
}

/// @brief Wrapper function to update a channel duty cycle specified in pct, assumes 14bit precision
/// @param channel PWM generator channel
/// @param duty_pc Integral value for the target duty cycle, in percents
/// @return ESP_OK if set and updated without issue, various error messages otherwise
esp_err_t change_duty_cycle(ledc_channel_t channel, uint8_t duty_pc)
{
    if (duty_pc > 100)
    {
        ESP_LOGE(TAG, "Duty cycle must be between 0 and 100, got %hhu", duty_pc);
        return ESP_ERR_INVALID_ARG;
    }
    else if (duty_pc == 0)
    {
        ESP_LOGW(TAG, "Duty cycle is 0, pausing channel %d", (int)channel);
        ledc_timer_pause(LEDC_LOW_SPEED_MODE, (ledc_timer_t)channel);
        active_timers[(int)channel] = false;
        return ESP_OK;
    }

    if (channel < LEDC_CHANNEL_0 || channel >= LEDC_CHANNEL_MAX)
    {
        ESP_LOGE(TAG, "Invalid channel %d", (int)channel);
        return ESP_ERR_INVALID_ARG;
    }

    ledc_timer_resume(LEDC_LOW_SPEED_MODE, (ledc_timer_t)channel);
    active_timers[(int)channel] = true;
    uint32_t duty = (duty_pc * (((uint32_t)1 << duty_resolutions_bit[(uint32_t)channel]) - 1)) / 100;
    esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, duty);
    if (err != ESP_OK)
        return err;
    return ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
}

/// @brief Wrapper function to update the timer of a PWM generator to update the frequency
/// @param channel LedC channel to update. This will map each channel to each timer (with same index number)
/// @param freq_hz Target frequency. Will throw an error if impossible to update because out of feasible range.
/// @return ESP_OK if all good, various errors otherwise
esp_err_t change_frequency(ledc_channel_t channel, uint32_t freq_hz)
{
    // Find the timer associated with the channel
    ledc_timer_t timer_sel = LEDC_TIMER_0;
    switch (channel)
    {
    case LEDC_CHANNEL_0:
        timer_sel = LEDC_TIMER_0;
        break;
    case LEDC_CHANNEL_1:
        timer_sel = LEDC_TIMER_1;
        break;
    case LEDC_CHANNEL_2:
        timer_sel = LEDC_TIMER_2;
        break;
    default:
        return ESP_ERR_INVALID_ARG;
        break;
    }

    if (freq_hz < 3)
    {
        ESP_LOGW(TAG, "Frequency must be greater than 3Hz, got %lu Hz, pausing channel", freq_hz);
        ledc_timer_pause(LEDC_LOW_SPEED_MODE, (ledc_timer_t)channel);
        active_timers[(int)channel] = false;
        return ESP_OK;
    }
    else
    {
        ledc_timer_resume(LEDC_LOW_SPEED_MODE, timer_sel);
        active_timers[(int)channel] = true;
        return ledc_set_freq(LEDC_LOW_SPEED_MODE, timer_sel, freq_hz);
    }
}