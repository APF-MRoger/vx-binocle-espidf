#include <stdio.h>
#include "esp_display_panel.hpp"
#include <lvgl.h>
#include "lvgl_v9_port.h"
#include <ui.h>
#include "esp_timer.h"
#include <math.h>

// Refresh interval to the LVGL objects
#ifndef DISP_VALUES_REFRESH_INTERVAL
#define DISP_VALUES_REFRESH_INTERVAL 25
#endif

using namespace esp_panel::drivers;
using namespace esp_panel::board;

static const char *TAG = "GENERAL";

#pragma region Global variables

// Vehicle variables and previous values retainers
bool indicatorsOn, p_indicatorsOn = true;
bool highBeamOn, p_highBeamOn = true;
bool lowFuelOn, p_lowFuelOn = true;
bool overTemperatureOn, p_overTemperatureOn = true;
bool brakesOn, p_brakesOn = true;
bool absOn, p_absOn = true;
bool parkingBrakeOn, p_parkingBrakeOn = true;
bool lowCoolantOn, p_lowCoolantOn = true;
bool batteryOn, p_batteryOn = true;
bool lowOilOn, p_lowOilOn = true;
bool milOn, p_milOn = true;
bool airbagOn, p_airbagOn = true;

// Vehicle numerical parameters
uint32_t speed, p_speed = 0;
uint32_t rpm, p_rpm = 0;
uint8_t fuelLevel, p_fuelLevel = 50;
uint8_t coolant, p_coolant = 88;

#pragma endregion

#pragma region Helper functions

/// @brief Random generator for testing
void generateValues()
{
    speed = 120 + 120 * sin((float)(esp_timer_get_time() / 1000) / 10000.0);
    rpm = 100 * (uint8_t)((3500 + 3500 * sin((float)(esp_timer_get_time() / 1000) / 10000.0)) / 100);
    fuelLevel = 50 + 50 * sin((float)(esp_timer_get_time() / 1000) / 15000.0);
    coolant = 88 + 12 * sin((float)(esp_timer_get_time() / 1000) / 20000.0);
    indicatorsOn = ((esp_timer_get_time() / 1000) / 500) % 2 == 0;
    highBeamOn = (esp_timer_get_time() / 1000000) % 2 == 0;
    lowFuelOn = fuelLevel < 20;
    overTemperatureOn = coolant > 95;
    brakesOn = (esp_timer_get_time() / 1000000) % 3 == 0;
    absOn = (esp_timer_get_time() / 1000000) % 4 == 0;
    lowCoolantOn = (esp_timer_get_time() / 1000000) % 5 == 0;
    batteryOn = (esp_timer_get_time() / 1000000) % 6 == 0;
    lowOilOn = (esp_timer_get_time() / 1000000) % 7 == 0;
    milOn = (esp_timer_get_time() / 1000000) % 8 == 0;
    airbagOn = (esp_timer_get_time() / 1000000) % 9 == 0;
}

int updateLVGLObjects()
{
    int updatedElements = 0;

    if (p_speed != speed)
    {
        lv_arc_set_value(objects.speed_arc, speed);
        // animateTargetArc(objects.speed_arc,speed*10);
        // lv_arc_align_obj_to_angle(objects.speed_arc, objects.speed_needle, 0);
        // lv_arc_rotate_obj_to_angle(objects.speed_arc, objects.speed_needle, 0);
        // lv_scale_set_line_needle_value(objects.speed_scale, objects.speed_needle, 230, speed);
        lv_label_set_text_fmt(objects.speed, "%03ld", speed);
        p_speed = speed;
        updatedElements++;
    }
    // if (p_rpm != rpm)
    // {
    //     // lv_arc_set_value(objects.rpm_arc, rpm);
    //     lv_scale_set_line_needle_value(objects.rpm_scale,objects.rpm_needle,180,rpm/100);
    //     lv_label_set_text_fmt(objects.rpm, "%04ld", rpm);
    //     p_rpm = rpm;
    // updatedElements++;
    // }
    if (p_fuelLevel != fuelLevel)
    {
        lv_bar_set_value(objects.fuel_bar, fuelLevel, LV_ANIM_OFF);
        lv_label_set_text_fmt(objects.fuel_level, "%03d", fuelLevel);
        p_fuelLevel = fuelLevel;
        updatedElements++;
    }
    if (p_coolant != coolant)
    {
        lv_bar_set_value(objects.coolant_bar, coolant, LV_ANIM_OFF);
        lv_label_set_text_fmt(objects.coolant, "%03d", coolant);
        p_coolant = coolant;
        updatedElements++;
    }

    if (p_lowFuelOn != lowFuelOn)
    {
        lv_obj_set_style_image_opa(objects.low_fuel_tt, lowFuelOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
        p_lowFuelOn = lowFuelOn;
        updatedElements++;
    }
    if (p_overTemperatureOn != overTemperatureOn)
    {
        lv_obj_set_style_image_opa(objects.over_temperature_tt, overTemperatureOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
        p_overTemperatureOn = overTemperatureOn;
        updatedElements++;
    }
#ifdef STRESS_TEST
    if (p_absOn != absOn)
    {
        lv_obj_set_style_image_opa(objects.abs_tt, absOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
        p_absOn = absOn;
        updatedElements++;
    }
    if (p_brakesOn != brakesOn)
    {
        lv_obj_set_style_image_opa(objects.brakes_tt, brakesOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
        p_brakesOn = brakesOn;
        updatedElements++;
    }
    if (p_lowCoolantOn != lowCoolantOn)
    {
        lv_obj_set_style_image_opa(objects.low_coolant_tt, lowCoolantOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
        p_lowCoolantOn = lowCoolantOn;
        updatedElements++;
    }
    if (p_batteryOn != batteryOn)
    {
        lv_obj_set_style_image_opa(objects.battery_tt, batteryOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
        p_batteryOn = batteryOn;
        updatedElements++;
    }
    if (p_lowOilOn != lowOilOn)
    {
        lv_obj_set_style_image_opa(objects.low_oil_tt, lowOilOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
        p_lowOilOn = lowOilOn;
        updatedElements++;
    }
    if (p_milOn != milOn)
    {
        lv_obj_set_style_image_opa(objects.mil_tt, milOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
        p_milOn = milOn;
        updatedElements++;
    }
    if (p_highBeamOn != highBeamOn)
    {
        lv_obj_set_style_image_opa(objects.hi_beam_tt, highBeamOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
        p_highBeamOn = highBeamOn;
        updatedElements++;
    }

    if (p_airbagOn != airbagOn)
    {
        lv_obj_set_style_image_opa(objects.airbag_tt, airbagOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
        p_airbagOn = airbagOn;
        updatedElements++;
    }
#endif

    if (p_indicatorsOn != indicatorsOn)
    {
        lv_obj_set_style_image_opa(objects.indicators_tt, indicatorsOn ? LV_OPA_COVER : LV_OPA_TRANSP, LV_STATE_DEFAULT);
        p_indicatorsOn = indicatorsOn;
        updatedElements++;
    }

    return updatedElements;
}

#pragma endregion

#pragma region Main app

/// @brief Main app
extern "C" void app_main()
{

#pragma region Setup
    // Board initialization
    ESP_LOGI(TAG, "Initializing board");

    Board *board = new Board();
    assert(board);
    ESP_UTILS_CHECK_FALSE_EXIT(board->init(), "Board init failed");

    auto lcd = board->getLCD();
    ESP_UTILS_CHECK_FALSE_EXIT(lcd->configFrameBufferNumber(LVGL_PORT_BUFFER_NUM), "Failed to configure frame buffer(s)");

    // Setting up the Bounce Buffer size (might not be necessary)
#if ESP_PANEL_DRIVERS_BUS_ENABLE_RGB && CONFIG_IDF_TARGET_ESP32S3
    auto lcd_bus = lcd->getBus();
    if (lcd_bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB)
    {
        ESP_UTILS_CHECK_FALSE_EXIT(static_cast<BusRGB *>(lcd_bus)->configRGB_BounceBufferSize(lcd->getFrameWidth() * 20), "Failed to set up bounce buffer");
    }
#endif

    // Board start
    ESP_UTILS_CHECK_FALSE_EXIT(board->begin(), "Board begin failed");
    auto backLight = board->getBacklight();
    ESP_LOGD("Backlight OFF", " %d", backLight->off());

// Screen test when in debug mode
#if CONFIG_LOG_DEFAULT_LEVEL >= ESP_LOG_DEBUG
    auto expander = board->getIO_Expander()->getBase();
    expander->printStatus();
    ESP_LOGI("Backlight", " %d", backLight->on());
    lcd->colorBarTest();
    vTaskDelay(pdMS_TO_TICKS(2000));
#endif

    // Start LVGL port
    ESP_UTILS_CHECK_FALSE_EXIT(lvgl_port_init(board->getLCD(), board->getTouch()), "Failed to start LVGL port");

    // UI loading and mofidifiers
    ESP_LOGI(TAG, "Loading UI");
    ESP_UTILS_CHECK_FALSE_EXIT(lvgl_port_lock(20), "Failed to perform initial LVGL Mutex lock");
    ui_init();                                                               // Load the UI library and draw it
    lv_obj_set_style_pad_radial(objects.speed_scale, 15, LV_PART_INDICATOR); // Pad the scale labels away from the tick marks
    // lv_arc_align_obj_to_angle(objects.speed_arc, objects.speed_needle, 0);
    // lv_arc_rotate_obj_to_angle(objects.speed_arc, objects.speed_needle, 0);
    lvgl_port_unlock();

    ESP_LOGI(TAG, "Backlight : %d", backLight->on());
    ESP_LOGI(TAG, "Setup done");

#pragma region Main Loop
    while (true)
    {
        
        vTaskDelay(pdMS_TO_TICKS(DISP_VALUES_REFRESH_INTERVAL));
        generateValues();

        // Attempt locking LVGL elements prior to updating them (issue with jumping frames ?)
        if (lvgl_port_lock(-1))
        {
            updateLVGLObjects();
            lvgl_port_unlock();
        }


    }
#pragma endregion
}