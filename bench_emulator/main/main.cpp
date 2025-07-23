#include <stdio.h>
#include "gpio_defs.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include "driver/mcpwm_prelude.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pwm_gen_helpers.h"
#include "mcpwm_capture_helpers.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "mdns.h"
#include "esp_http_server.h"
#include "driver/uart.h"

#define LOG_INTERVAL_MS 2000
#define WIFI_CONNECT_TIMEOUT_MS 45000
#define UART_NUM UART_NUM_0
#define UART_BUF_SIZE 128

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

static char wifi_ssid[32] = {0};
static char wifi_pass[64] = {0};

static bool pwm_enabled[3] = {true, true, true}; // Coolant, RPM, Speed

// Helper: Read string from UART
void uart_read_line(char *buf, size_t max_len) {
    size_t idx = 0;
    while (idx < max_len - 1) {
        int len = uart_read_bytes(UART_NUM, (uint8_t*)&buf[idx], 1, 1000 / portTICK_PERIOD_MS);
        if (len > 0) {
            if (buf[idx] == '\n' || buf[idx] == '\r') break;
            idx++;
        }
    }
    buf[idx] = '\0';
}

// Helper: Save credentials to NVS
esp_err_t save_wifi_creds(const char *ssid, const char *pass) {
    nvs_handle_t nvs;
    esp_err_t err = nvs_open("wifi", NVS_READWRITE, &nvs);
    if (err == ESP_OK) {
        nvs_set_str(nvs, "ssid", ssid);
        nvs_set_str(nvs, "pass", pass);
        nvs_commit(nvs);
        nvs_close(nvs);
    }
    return err;
}

// Helper: Load credentials from NVS
esp_err_t load_wifi_creds(char *ssid, size_t ssid_len, char *pass, size_t pass_len) {
    nvs_handle_t nvs;
    esp_err_t err = nvs_open("wifi", NVS_READONLY, &nvs);
    if (err == ESP_OK) {
        err = nvs_get_str(nvs, "ssid", ssid, &ssid_len);
        if (err == ESP_OK)
            err = nvs_get_str(nvs, "pass", pass, &pass_len);
        nvs_close(nvs);
    }
    return err;
}
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static int s_retry_num = 0;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < 10)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}
// WiFi connect logic
void connect_wifi(const char *ssid, const char *pass) {
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);
    wifi_config_t wifi_config = {};

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));


    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));
    wifi_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    wifi_config.sta.failure_retry_cnt = 20;
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
    esp_wifi_scan_start(NULL,true);
    esp_wifi_connect();

/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 ssid, pass);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 ssid, pass);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
    vEventGroupDelete(s_wifi_event_group);
    
}

// HTTP handler for PWM control
esp_err_t pwm_control_handler(httpd_req_t *req) {
    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf));
    if (ret <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "No data");
        return ESP_FAIL;
    }
    buf[ret] = 0;

    int channel = 0, freq = 0, duty = 0, enable = 1;
    sscanf(buf, "channel=%d&freq=%d&duty=%d&enable=%d", &channel, &freq, &duty, &enable);

    if (channel < 0 || channel > 2) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid channel");
        return ESP_FAIL;
    }

    pwm_enabled[channel] = (enable != 0);
    if (pwm_enabled[channel]) {
        change_frequency((ledc_channel_t)channel, freq);
        change_duty_cycle((ledc_channel_t)channel, duty);
    } else {
        change_duty_cycle((ledc_channel_t)channel, 0); // Disable by setting duty to 0
    }

    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

// HTTP handler for main page
esp_err_t webpage_handler(httpd_req_t *req) {
    const char *html =
        "<!DOCTYPE html><html><head><title>PWM Control</title></head><body>"
        "<h2>PWM Channel Control</h2>"
        "<form id='pwmForm'>"
        "Channel: <select name='channel'>"
        "<option value='0'>Coolant</option>"
        "<option value='1'>RPM</option>"
        "<option value='2'>Speed</option>"
        "</select><br>"
        "Frequency: <input type='number' name='freq' min='1' max='10000'><br>"
        "Duty Cycle: <input type='number' name='duty' min='0' max='100'><br>"
        "Enable: <input type='checkbox' name='enable' checked><br>"
        "<button type='button' onclick='sendPWM()'>Set</button>"
        "</form>"
        "<script>"
        "function sendPWM() {"
        "  var f = document.getElementById('pwmForm');"
        "  var data = 'channel=' + f.channel.value + '&freq=' + f.freq.value + '&duty=' + f.duty.value + '&enable=' + (f.enable.checked ? 1 : 0);"
        "  var xhr = new XMLHttpRequest();"
        "  xhr.open('POST', '/control', true);"
        "  xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');"
        "  xhr.onreadystatechange = function() {"
        "    if (xhr.readyState == 4) alert('Response: ' + xhr.responseText);"
        "  };"
        "  xhr.send(data);"
        "}"
        "</script>"
        "</body></html>";
    httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

    httpd_uri_t uri_get = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = webpage_handler,
        .user_ctx = NULL
    };
    httpd_uri_t uri_post = {
        .uri = "/control",
        .method = HTTP_POST,
        .handler = pwm_control_handler,
        .user_ctx = NULL
    };


// Setup HTTP server
httpd_handle_t start_webserver() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    httpd_start(&server, &config);

    httpd_register_uri_handler(server, &uri_get);
    httpd_register_uri_handler(server, &uri_post);
    return server;
}

// Setup mDNS
void start_mdns() {
    mdns_init();
    mdns_hostname_set("emulator_board");
    mdns_instance_name_set("ESP32 PWM Emulator");
    // Advertise HTTP service
    mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
}

void init_uart()
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_driver_install(UART_NUM, UART_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}


extern "C" void app_main(void)
{
    // Init UART
    init_uart();
    // Init NVS
    nvs_flash_init();
    //save_wifi_creds("****","*****"); // Uncomment and edit this the first time around
    // if(esp_event_loop_create_default()!=ESP_OK)
    // {
    //     ESP_LOGW(TAG,"Events loop already created");
    // }
    // ESP_ERROR_CHECK(esp_netif_init());
    // Try to load WiFi credentials
    if (load_wifi_creds(wifi_ssid, sizeof(wifi_ssid), wifi_pass, sizeof(wifi_pass)) != ESP_OK) {
        // Prompt for SSID/password over UART
        printf("Enter WiFi SSID:\n");
        uart_read_line(wifi_ssid, sizeof(wifi_ssid));
        printf("Enter WiFi Password:\n");
        uart_read_line(wifi_pass, sizeof(wifi_pass));
        save_wifi_creds(wifi_ssid, wifi_pass);
    }
    else
    {
        ESP_LOGW(TAG,"Attempting to connect with %s and %s ", wifi_ssid,wifi_pass);
    }

connect_wifi(wifi_ssid, wifi_pass);

    ESP_LOGW(TAG,"Connected to AP, pause for mdns");
    vTaskDelay(pdMS_TO_TICKS(10000));
    // Start mDNS and webserver
     start_mdns();

     ESP_LOGW(TAG,"Pause for webserver");
    vTaskDelay(pdMS_TO_TICKS(10000));
    static httpd_handle_t server = NULL;
    server = start_webserver();
    ESP_LOGW(TAG,"Wait post webserver");
    vTaskDelay(pdMS_TO_TICKS(10000));

    set_pwm_generator(LEDC_TIMER_0, COOLANT_PWM_BASE_FREQ_HZ, (gpio_num_t)COOLANT_PWM_GEN_GPIO, pwm_gen_coolant, COOLANT_PWM_BASE_DUTY_PCT);
    set_pwm_generator(LEDC_TIMER_1, RPM_PWM_BASE_FREQ_HZ, (gpio_num_t)RPM_PWM_GEN_GPIO, pwm_gen_rpm, RPM_PWM_BASE_DUTY_PCT);
    set_pwm_generator(LEDC_TIMER_2, SPEED_PWM_BASE_FREQ_HZ, (gpio_num_t)SPEED_PWM_GEN_GPIO, pwm_gen_speed, SPEED_PWM_BASE_DUTY_PCT);

    set_capture_channel(cap_chan_coolant, (gpio_num_t)COOLANT_PWM_CAP_GPIO, &pwm_cap_coolant);
    set_capture_channel(cap_chan_rpm, (gpio_num_t)RPM_PWM_CAP_GPIO, &pwm_cap_rpm);
    set_capture_channel(cap_chan_speed, (gpio_num_t)SPEED_PWM_CAP_GPIO, &pwm_cap_speed);

    static uint32_t start_frequency = 0;
    static uint8_t start_duty_cycle = 0;
    // --- Logging Loop ---
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(LOG_INTERVAL_MS));
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

        // start_duty_cycle = (start_duty_cycle + 1)%99;
        // if(change_duty_cycle(pwm_gen_coolant, start_duty_cycle+1) != ESP_OK)
        // {
        //     ESP_LOGE(TAG, "Failed to change coolant duty cycle");
        // }
        // start_frequency = (start_frequency +1)%996;
        // change_frequency(pwm_gen_speed,4+start_frequency);
        // portEXIT_CRITICAL(&counter_mux);
    }
}