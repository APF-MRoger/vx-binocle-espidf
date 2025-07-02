#include <stdio.h>
// #include "esp_panel_board_custom_conf.h"
#include "esp_display_panel.hpp"
// #include "esp_lib_utils.h"
#include <lvgl.h>
#include "lvgl_v9_port.h"
#include <ui.h>
#include "esp_timer.h"
#include <math.h>

#ifndef DISP_VALUES_REFRESH_INTERVAL
#define DISP_VALUES_REFRESH_INTERVAL 100
#endif

using namespace esp_panel::drivers;
using namespace esp_panel::board;

static const char *TAG = "main";

unsigned long lastLVGLTicked = 0;
unsigned long lastDispValuesRefreshed = 0;

// Vehicle variables
bool indicatorsOn, p_indicatorsOn = true;
bool highBeamOn, p_highBeamOn = true;
bool lowFuelOn, p_lowFuelOn = true;
bool overTemperatureOn, p_overTemperatureOn = true;
bool brakesOn, p_brakesOn = true;
bool absOn, p_absOn = true;
bool lowCoolantOn, p_lowCoolantOn = true;
bool batteryOn, p_batteryOn = true;
bool lowOilOn, p_lowOilOn = true;
bool milOn, p_milOn = true;
bool airbagOn, p_airbagOn = true;
uint32_t speed, p_speed = 0;
uint32_t rpm, p_rpm = 0;
uint8_t fuelLevel, p_fuelLevel = 50;
uint8_t coolant, p_coolant = 88;

#define BRIGHTNESS 215

// First read flags

bool screenON = false;

void generateValues()
{
    speed = 120 + 120 * sin((float)(esp_timer_get_time() / 1000) / 5000.0);
    rpm = 100 * (uint8_t)((3500 + 3500 * sin((float)(esp_timer_get_time() / 1000) / 10000.0)) / 100);
    fuelLevel = 50 + 50 * sin((float)(esp_timer_get_time() / 1000) / 15000.0);
    coolant = 88 + 12 * sin((float)(esp_timer_get_time() / 1000) / 20000.0);
    indicatorsOn = ((esp_timer_get_time() / 1000) / 500) % 2 == 0;
    highBeamOn = ((esp_timer_get_time() / 1000) / 5000) % 2 == 0;
    lowFuelOn = fuelLevel < 20;
    overTemperatureOn = coolant > 95;
    brakesOn = (((esp_timer_get_time()+2000) / 1000) / 5000) % 2 == 0;
    absOn = (((esp_timer_get_time()+4000) / 1000) / 5000) % 2 == 0;
    lowCoolantOn = (((esp_timer_get_time()+6000) / 1000) / 5000) % 2 == 0;
    batteryOn = (((esp_timer_get_time()+8000) / 1000) / 5000) % 2 == 0;
    lowOilOn = (((esp_timer_get_time()+10000) / 1000) / 5000) % 2 == 0;
    milOn = (((esp_timer_get_time()+12000) / 1000) / 5000) % 2 == 0;
    airbagOn = (((esp_timer_get_time()+14000) / 1000) / 5000) % 2 == 0;
}

extern "C" void app_main()
{
    Board *board = new Board();
    assert(board);
    ESP_LOGI(TAG, "Initializing board");
    ESP_UTILS_CHECK_FALSE_EXIT(board->init(), "Board init failed");
    auto lcd = board->getLCD();
    lcd->configFrameBufferNumber(LVGL_PORT_BUFFER_NUM);
#if ESP_PANEL_DRIVERS_BUS_ENABLE_RGB && CONFIG_IDF_TARGET_ESP32S3
    auto lcd_bus = lcd->getBus();
    /**
     * As the anti-tearing feature typically consumes more PSRAM bandwidth, for the ESP32-S3, we need to utilize the
     * "bounce buffer" functionality to enhance the RGB data bandwidth.
     * This feature will consume `bounce_buffer_size * bytes_per_pixel * 2` of SRAM memory.
     */
    if (lcd_bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB)
    {
        static_cast<BusRGB *>(lcd_bus)->configRGB_BounceBufferSize(lcd->getFrameWidth() * 10);
    }
#endif
    auto expander = board->getIO_Expander()->getBase();
    ESP_UTILS_CHECK_FALSE_EXIT(board->begin(), "Board begin failed");
    expander->printStatus();
    // vTaskDelay(pdMS_TO_TICKS(10000));
    auto backLight = board->getBacklight();
    ESP_LOGI("Backlight OFF", " %d", backLight->off());
    // vTaskDelay(pdMS_TO_TICKS(10000));
    // ESP_LOGI("Backlight"," %d",backLight->on());
    // lcd->colorBarTest();
    lvgl_port_init(board->getLCD(), board->getTouch());

    lvgl_port_lock(-1);
    ui_init();
    lvgl_port_unlock();
    ESP_LOGI("Backlight ON", " %d", backLight->on());
    generateValues();
    // vTaskDelay(pdMS_TO_TICKS(10000));
    ESP_LOGI(TAG, "Setup done");

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(DISP_VALUES_REFRESH_INTERVAL));
        generateValues();
        // Refresh the items in the UI

        if (lvgl_port_lock(1))
        {

            if (p_speed != speed)
            {
                // lv_arc_set_value(objects.speed_arc, speed);
                lv_scale_set_line_needle_value(objects.speed_scale, objects.speed_needle, 230, speed);
                lv_label_set_text_fmt(objects.speed, "%03ld", speed);
                p_speed = speed;
            }
            if (p_rpm != rpm)
                // {
                //     // lv_arc_set_value(objects.rpm_arc, rpm);
                //     lv_scale_set_line_needle_value(objects.rpm_scale,objects.rpm_needle,180,rpm/100);
                //     lv_label_set_text_fmt(objects.rpm, "%04ld", rpm);
                //     p_rpm = rpm;
                // }
                if (p_fuelLevel != fuelLevel)
                {
                    lv_bar_set_value(objects.fuel_bar, fuelLevel, LV_ANIM_OFF);
                    lv_label_set_text_fmt(objects.fuel_level, "%03d", fuelLevel);
                    p_fuelLevel = fuelLevel;
                }
            if (p_coolant != coolant)
            {
                lv_bar_set_value(objects.coolant_bar, coolant, LV_ANIM_OFF);
                lv_label_set_text_fmt(objects.coolant, "%03d", coolant);
                p_coolant = coolant;
            }
            if (p_absOn != absOn)
            {
                lv_obj_set_style_image_opa(objects.abs_tt, absOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
                p_absOn = absOn;
            }
            if (p_lowFuelOn != lowFuelOn)
            {
                lv_obj_set_style_image_opa(objects.low_fuel_tt, lowFuelOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
                p_lowFuelOn = lowFuelOn;
            }
            if (p_overTemperatureOn != overTemperatureOn)
            {
                lv_obj_set_style_image_opa(objects.over_temperature_tt, overTemperatureOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
                p_overTemperatureOn = overTemperatureOn;
            }
            if (p_brakesOn != brakesOn)
            {
                lv_obj_set_style_image_opa(objects.brakes_tt, brakesOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
                p_brakesOn = brakesOn;
            }
            if (p_lowCoolantOn != lowCoolantOn)
            {
                lv_obj_set_style_image_opa(objects.low_coolant_tt, lowCoolantOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
                p_lowCoolantOn = lowCoolantOn;
            }
            if (p_batteryOn != batteryOn)
            {
                lv_obj_set_style_image_opa(objects.battery_tt, batteryOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
                p_batteryOn = batteryOn;
            }
            if (p_lowOilOn != lowOilOn)
            {
                lv_obj_set_style_image_opa(objects.low_oil_tt, lowOilOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
                p_lowOilOn = lowOilOn;
            }
            if (p_milOn != milOn)
            {
                lv_obj_set_style_image_opa(objects.mil_tt, milOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
                p_milOn = milOn;
            }
            if (p_highBeamOn != highBeamOn)
            {
                lv_obj_set_style_image_opa(objects.hi_beam_tt, highBeamOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
                p_highBeamOn = highBeamOn;
            }
            if (p_indicatorsOn != indicatorsOn)
            {
                lv_obj_set_style_image_opa(objects.indicators_tt, indicatorsOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
                p_indicatorsOn = indicatorsOn;
            }
            if (p_airbagOn != airbagOn)
            {
                lv_obj_set_style_image_opa(objects.airbag_tt, airbagOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
                p_airbagOn = airbagOn;
            }
            lvgl_port_unlock();
        }

        // lv_obj_set_style_image_opa(objects.over_temperature_tt, overTemperatureOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
        // lv_obj_set_style_image_opa(objects.brakes_tt, brakesOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
        // lv_obj_set_style_image_opa(objects.low_coolant_tt, lowCoolantOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
        // lv_obj_set_style_image_opa(objects.battery_tt, batteryOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
        // lv_obj_set_style_image_opa(objects.low_oil_tt, lowOilOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
        // lv_obj_set_style_image_opa(objects.mil_tt, milOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
        // lv_obj_set_style_image_opa(objects.hi_beam_tt, highBeamOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
        // lv_obj_set_style_image_opa(objects.indicators_tt, indicatorsOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
        // lv_obj_set_style_image_opa(objects.airbag_tt, airbagOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);

        // Update shit here
    }
}