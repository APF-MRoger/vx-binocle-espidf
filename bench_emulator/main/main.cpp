#include <stdio.h>
#include "gpio_defs.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include "driver/mcpwm_prelude.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pwm_gen_helpers.h"
#include "mcpwm_capture_helpers.h"
#include "wifi_helpers.h"
#include "web_intfc_helpers.h"


#define LOG_INTERVAL_MS 2000

#ifdef TAG
#undef TAG
#endif
#define TAG "MAIN"

// static portMUX_TYPE counter_mux = portMUX_INITIALIZER_UNLOCKED;

// PWM stats structures for coolant, rpm and speed captures
static volatile pwm_info_t pwm_cap_coolant, pwm_cap_rpm, pwm_cap_speed = {.pos_edge_ts = 0, .prev_pos_edge_ts = 0, .period_ticks = 0, .neg_edge_ts = 0, .deltaT = 0};

// Generator channels
ledc_channel_t pwm_gen_coolant = LEDC_CHANNEL_0;
ledc_channel_t pwm_gen_rpm = LEDC_CHANNEL_1;
ledc_channel_t pwm_gen_speed = LEDC_CHANNEL_2;

// Capture channels
mcpwm_cap_channel_handle_t cap_chan_coolant = NULL;
mcpwm_cap_channel_handle_t cap_chan_rpm = NULL;
mcpwm_cap_channel_handle_t cap_chan_speed = NULL;

extern "C" void app_main(void)
{

    // Init NVS
    nvs_flash_init();

    // save_wifi_creds("****","*****"); // Uncomment and edit this the first time around

    if (load_wifi_creds(wifi_ssid, sizeof(wifi_ssid), wifi_pass, sizeof(wifi_pass)) != ESP_OK)
    {
        ESP_LOGE(TAG, "No SSID or password found in NVS, check main code");
    }
    else
    {
        ESP_LOGW(TAG, "Attempting to connect with %s and %s ", wifi_ssid, wifi_pass);
    }
    connect_wifi(wifi_ssid, wifi_pass);
    start_mdns();
    ESP_LOGI(TAG,"Start PWM generators");
    set_pwm_generator(LEDC_TIMER_0, COOLANT_PWM_BASE_FREQ_HZ, (gpio_num_t)COOLANT_PWM_GEN_GPIO, pwm_gen_coolant, COOLANT_PWM_BASE_DUTY_PCT);
    set_pwm_generator(LEDC_TIMER_1, RPM_PWM_BASE_FREQ_HZ, (gpio_num_t)RPM_PWM_GEN_GPIO, pwm_gen_rpm, RPM_PWM_BASE_DUTY_PCT);
    set_pwm_generator(LEDC_TIMER_2, SPEED_PWM_BASE_FREQ_HZ, (gpio_num_t)SPEED_PWM_GEN_GPIO, pwm_gen_speed, SPEED_PWM_BASE_DUTY_PCT);

    set_capture_channel(cap_chan_coolant, (gpio_num_t)COOLANT_PWM_CAP_GPIO, &pwm_cap_coolant);
    set_capture_channel(cap_chan_rpm, (gpio_num_t)RPM_PWM_CAP_GPIO, &pwm_cap_rpm);
    set_capture_channel(cap_chan_speed, (gpio_num_t)SPEED_PWM_CAP_GPIO, &pwm_cap_speed);

    ESP_LOGI(TAG, "Starting web server interface");
    static httpd_handle_t server = NULL;
    server = start_webserver();

    // --- Logging Loop ---
    while (1)
    {
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

        // portEXIT_CRITICAL(&counter_mux);
    }
}