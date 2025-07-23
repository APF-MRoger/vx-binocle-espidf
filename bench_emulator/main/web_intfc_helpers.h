#pragma once
#include <stdio.h>
#include "gpio_defs.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pwm_gen_helpers.h"
#include "mdns.h"
#include "esp_http_server.h"

#ifdef TAG
#undef TAG
#endif
#define TAG "Web Interface"

static bool pwm_enabled[3] = {true, true, true}; // Coolant, RPM, Speed

// HTTP handler for PWM control
esp_err_t pwm_control_handler(httpd_req_t *req)
{
    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf));
    if (ret <= 0)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "No data");
        return ESP_FAIL;
    }
    buf[ret] = 0;

    uint8_t channel = 0,  duty = 0, enable = 1;
    uint32_t freq = 0;
    sscanf(buf, "channel=%hhu&freq=%lu&duty=%hhu&enable=%hhu", &channel, &freq, &duty, &enable);

    if (channel < 0 || channel > 2)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid channel");
        return ESP_FAIL;
    }

    if (freq < 1 || freq > 10000)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid frequency");
        return ESP_FAIL;
    }

    if( duty <1 || duty > 100)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid duty cycle");
            return ESP_FAIL;
    }

    pwm_enabled[channel] = (enable != 0);
    if (pwm_enabled[channel])
    {
        change_frequency((ledc_channel_t)channel, freq);
        change_duty_cycle((ledc_channel_t)channel, duty);
    }
    else
    {
        change_duty_cycle((ledc_channel_t)channel, 0); // Disable by setting duty to 0
    }

    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

// HTTP handler for main page
esp_err_t webpage_handler(httpd_req_t *req)
{
    const char *html =
        "<!DOCTYPE html><html><head><title>PWM Control</title></head><body>"
        "<h2>PWM Channel Control</h2>"
        "<form id='pwmForm'>"
        "Channel: <select name='channel'>"
        "<option value='0'>Coolant</option>"
        "<option value='1'>RPM</option>"
        "<option value='2'>Speed</option>"
        "</select><br>"
        "Frequency: <input type='number' name='freq' value='100' min='1' max='10000' step='1'><br>"
        "Duty Cycle: <input type='number' name='duty' value='33' min='1' max='100' step='1'><br>"
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
    .user_ctx = NULL};
httpd_uri_t uri_post = {
    .uri = "/control",
    .method = HTTP_POST,
    .handler = pwm_control_handler,
    .user_ctx = NULL};

// Setup HTTP server
httpd_handle_t start_webserver()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    httpd_start(&server, &config);

    httpd_register_uri_handler(server, &uri_get);
    httpd_register_uri_handler(server, &uri_post);
    return server;
}

// Setup mDNS
void start_mdns()
{
    mdns_init();
    mdns_hostname_set("emulator_board");
    mdns_instance_name_set("ESP32 PWM Emulator");
    // Advertise HTTP service
    mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
}