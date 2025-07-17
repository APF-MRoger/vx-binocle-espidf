#include <stdio.h>
#include "esp_display_panel.hpp"
#include <lvgl.h>
#include "lvgl_v9_port.h"
#include <ui.h>
#include "esp_timer.h"
#include <math.h>
// #include "driver/twai.h"
#include "twai_daemon.h"
#include "binocan.h"

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
bool ignitionST, p_ignitionST = false;

// Vehicle numerical parameters
float speed, p_speed = 0;
float lvVoltage, p_lvVoltage = 12.0;
uint32_t rpm, p_rpm = 0;
uint8_t fuelLevel, p_fuelLevel = 50;
uint8_t coolant, p_coolant = 88;

// Global UI objects
lv_obj_t *needleLine = nullptr;

#pragma endregion

#pragma region Helper functions

bool generatorOn = true;

/// @brief Random generator for testing
void generateValues()
{
    if (generatorOn)
    {
        speed = 120.0 + 120.0 * sin((float)(esp_timer_get_time() / 1000) / 10000.0);
        rpm = 100 * (uint8_t)((3500 + 3500 * sin((float)(esp_timer_get_time() / 1000) / 10000.0)) / 100);
        fuelLevel = 50 + 50 * sin((float)(esp_timer_get_time() / 1000) / 15000.0);
        lvVoltage = 12 + 2 * sin((float)(esp_timer_get_time() / 1000) / 20000.0);
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
}

int updateLVGLObjects()
{
    int updatedElements = 0;

    if ((long)(p_speed * 10) != (long)(speed * 10))
    {
        // lv_arc_set_value(objects.speed_arc, speed);
        // animateTargetArc(objects.speed_arc,speed*10);
        // lv_arc_align_obj_to_angle(objects.speed_arc, objects.speed_needle, 0);
        // lv_arc_rotate_obj_to_angle(objects.speed_arc, objects.speed_needle, 0);
        // lv_scale_set_line_needle_value(objects.speed_scale, objects.speed_needle, 230, speed);
        // lv_scale_set_line_needle_value(objects.speed_scale,needleLine,-8,speed);
        lv_scale_set_image_needle_value(objects.speed_scale, objects.simple_needle, (long)(speed * 10));
        lv_label_set_text_fmt(objects.speed, "%03ld", (long)speed);
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
        // lv_bar_set_value(objects.coolant_bar, coolant, LV_ANIM_OFF);
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

esp_err_t dispatchFrame(twai_message_t *rxMsg)
{
    static binocan_base_telltales_t baseTellTales_msg;
    static binocan_base_slow_metrics_t baseSlowMetrics_msg;
    static binocan_base_vehicle_metrics_t baseVehicleMetrics_msg;

    static binocan_extra_oil_metrics_t baseExtraOilMetrics_msg;
    static binocan_extra_chargecooling_metrics_t baseChargeCoolingMetrics_msg;
    generatorOn = false;

    switch (rxMsg->identifier)
    {
    case BINOCAN_BASE_TELLTALES_FRAME_ID:
        binocan_base_telltales_unpack(&baseTellTales_msg, rxMsg->data, rxMsg->data_length_code);

        if (binocan_base_telltales_abs_tt_is_in_range(baseTellTales_msg.abs_tt))
        {
            binocan_base_telltales_abs_tt_decode(baseTellTales_msg.abs_tt) == BINOCAN_BASE_TELLTALES_ABS_TT_ON_CHOICE ? absOn = true : absOn = false;
        }
        else
        {
            ESP_LOGW(TAG, "ABS telltale signal out of range: %d", baseTellTales_msg.abs_tt);
            absOn = true;
        }

        if (binocan_base_telltales_airbag_tt_is_in_range(baseTellTales_msg.airbag_tt))
        {
            binocan_base_telltales_airbag_tt_decode(baseTellTales_msg.airbag_tt) == BINOCAN_BASE_TELLTALES_AIRBAG_TT_ON_CHOICE ? airbagOn = true : airbagOn = false;
        }
        else
        {
            ESP_LOGW(TAG, "Airbag telltale signal out of range: %d", baseTellTales_msg.airbag_tt);
            airbagOn = true;
        }

        if (binocan_base_telltales_cel_tt_is_in_range(baseTellTales_msg.cel_tt))
        {
            binocan_base_telltales_cel_tt_decode(baseTellTales_msg.cel_tt) == BINOCAN_BASE_TELLTALES_CEL_TT_ON_CHOICE ? milOn = true : milOn = false;
        }
        else
        {
            ESP_LOGW(TAG, "Check Engine Light telltale signal out of range: %d", baseTellTales_msg.cel_tt);
            milOn = true;
        }

        if (binocan_base_telltales_high_beams_tt_is_in_range(baseTellTales_msg.high_beams_tt))
        {
            binocan_base_telltales_high_beams_tt_decode(baseTellTales_msg.high_beams_tt) == BINOCAN_BASE_TELLTALES_HIGH_BEAMS_TT_ON_CHOICE ? highBeamOn = true : highBeamOn = false;
        }
        else
        {
            ESP_LOGW(TAG, "High beams telltale signal out of range: %d", baseTellTales_msg.high_beams_tt);
            highBeamOn = true;
        }

        if (binocan_base_telltales_low_brake_fluid_tt_is_in_range(baseTellTales_msg.low_brake_fluid_tt))
        {
            binocan_base_telltales_low_brake_fluid_tt_decode(baseTellTales_msg.low_brake_fluid_tt) == BINOCAN_BASE_TELLTALES_LOW_BRAKE_FLUID_TT_ON_CHOICE ? brakesOn = true : brakesOn = false;
        }
        else
        {
            ESP_LOGW(TAG, "Low brake fluid telltale signal out of range: %d", baseTellTales_msg.low_brake_fluid_tt);
            brakesOn = true;
        }

        if (binocan_base_telltales_low_coolant_tt_is_in_range(baseTellTales_msg.low_coolant_tt))
        {
            binocan_base_telltales_low_coolant_tt_decode(baseTellTales_msg.low_coolant_tt) == BINOCAN_BASE_TELLTALES_LOW_COOLANT_TT_ON_CHOICE ? lowCoolantOn = true : lowCoolantOn = false;
        }
        else
        {
            ESP_LOGW(TAG, "Low coolant level telltale signal out of range: %d", baseTellTales_msg.low_coolant_tt);
            lowCoolantOn = true;
        }

        if (binocan_base_telltales_low_fuel_tt_is_in_range(baseTellTales_msg.low_fuel_tt))
        {
            binocan_base_telltales_low_fuel_tt_decode(baseTellTales_msg.low_fuel_tt) == BINOCAN_BASE_TELLTALES_LOW_FUEL_TT_ON_CHOICE ? lowFuelOn = true : lowFuelOn = false;
        }
        else
        {
            ESP_LOGW(TAG, "Low fuel level telltale signal out of range: %d", baseTellTales_msg.low_fuel_tt);
            lowFuelOn = true;
        }

        if (binocan_base_telltales_low_oil_pressure_tt_is_in_range(baseTellTales_msg.low_oil_pressure_tt))
        {
            binocan_base_telltales_low_oil_pressure_tt_decode(baseTellTales_msg.low_oil_pressure_tt) == BINOCAN_BASE_TELLTALES_LOW_OIL_PRESSURE_TT_ON_CHOICE ? lowOilOn = true : lowOilOn = false;
        }
        else
        {
            ESP_LOGW(TAG, "Low oil pressure telltale signal out of range: %d", baseTellTales_msg.low_oil_pressure_tt);
            lowOilOn = true;
        }

        if (binocan_base_telltales_lv_system_tt_is_in_range(baseTellTales_msg.lv_system_tt))
        {
            binocan_base_telltales_lv_system_tt_decode(baseTellTales_msg.lv_system_tt) == BINOCAN_BASE_TELLTALES_LV_SYSTEM_TT_ON_CHOICE ? batteryOn = true : batteryOn = false;
        }
        else
        {
            ESP_LOGW(TAG, "Battery or alternator telltale signal out of range: %d", baseTellTales_msg.lv_system_tt);
            batteryOn = true;
        }

        if (binocan_base_telltales_over_temperature_tt_is_in_range(baseTellTales_msg.over_temperature_tt))
        {
            binocan_base_telltales_over_temperature_tt_decode(baseTellTales_msg.over_temperature_tt) == BINOCAN_BASE_TELLTALES_OVER_TEMPERATURE_TT_ON_CHOICE ? overTemperatureOn = true : overTemperatureOn = false;
        }
        else
        {
            ESP_LOGW(TAG, "Coolant overtemperature telltale signal out of range: %d", baseTellTales_msg.over_temperature_tt);
            overTemperatureOn = true;
        }

        if (binocan_base_telltales_parking_brake_tt_is_in_range(baseTellTales_msg.parking_brake_tt))
        {
            binocan_base_telltales_parking_brake_tt_decode(baseTellTales_msg.parking_brake_tt) == BINOCAN_BASE_TELLTALES_PARKING_BRAKE_TT_ON_CHOICE ? parkingBrakeOn = true : parkingBrakeOn = false;
        }
        else
        {
            ESP_LOGW(TAG, "Parking brake telltale signal out of range: %d", baseTellTales_msg.parking_brake_tt);
            parkingBrakeOn = true;
        }

        if (binocan_base_telltales_turn_indicators_tt_is_in_range(baseTellTales_msg.turn_indicators_tt))
        {
            binocan_base_telltales_turn_indicators_tt_decode(baseTellTales_msg.turn_indicators_tt) == BINOCAN_BASE_TELLTALES_TURN_INDICATORS_TT_ON_CHOICE ? indicatorsOn = true : indicatorsOn = false;
        }
        else
        {
            ESP_LOGW(TAG, "Turn indicators telltale signal out of range: %d", baseTellTales_msg.turn_indicators_tt);
            indicatorsOn = true;
        }

        ESP_LOGD(TAG, "Telltales: ABS %d, Airbag %d, CEL %d, High Beams %d, Low Brake Fluid %d, Low Coolant %d, Low Fuel %d, Low Oil Pressure %d, Battery/Alternator %d, Over Temperature %d, Parking Brake %d, Indicators %d",
                 absOn, airbagOn, milOn, highBeamOn, brakesOn, lowCoolantOn, lowFuelOn, lowOilOn, batteryOn,
                 overTemperatureOn, parkingBrakeOn, indicatorsOn);

        break;
    case BINOCAN_BASE_SLOW_METRICS_FRAME_ID:
        binocan_base_slow_metrics_unpack(&baseSlowMetrics_msg, rxMsg->data, rxMsg->data_length_code);

        if (binocan_base_slow_metrics_coolant_temp_is_in_range(baseSlowMetrics_msg.coolant_temp))
        {
            coolant = (uint8_t)binocan_base_slow_metrics_coolant_temp_decode(baseSlowMetrics_msg.coolant_temp);
        }
        else
        {
            ESP_LOGW(TAG, "Coolant temperature signal out of range: %d", baseSlowMetrics_msg.coolant_temp);
            coolant = 255;            // Default value
            overTemperatureOn = true; // Set over temperature on if coolant is out of range
        }

        if (binocan_base_slow_metrics_fuel_level_is_in_range(baseSlowMetrics_msg.fuel_level))
        {
            fuelLevel = (uint8_t)binocan_base_slow_metrics_fuel_level_decode(baseSlowMetrics_msg.fuel_level);
        }
        else
        {
            ESP_LOGW(TAG, "Fuel level signal out of range: %d", baseSlowMetrics_msg.fuel_level);
            fuelLevel = 0;    // Default value
            lowFuelOn = true; // Set low fuel on if fuel level is out of range
        }

        if (binocan_base_slow_metrics_lv_voltage_is_in_range(baseSlowMetrics_msg.lv_voltage))
        {
            lvVoltage = (float)binocan_base_slow_metrics_lv_voltage_decode(baseSlowMetrics_msg.lv_voltage);
        }
        else
        {
            ESP_LOGW(TAG, "LV Voltage signal out of range: %d", baseSlowMetrics_msg.lv_voltage);
            batteryOn = true; // Set battery on if LV voltage is out of range
        }

        ESP_LOGD(TAG, "Slow Metrics: Coolant %d, Fuel Level %d, LV Voltage %.2f",
                 coolant, fuelLevel, lvVoltage);

        break;

    case BINOCAN_BASE_VEHICLE_METRICS_FRAME_ID:
        binocan_base_vehicle_metrics_unpack(&baseVehicleMetrics_msg, rxMsg->data, rxMsg->data_length_code);
        if (binocan_base_vehicle_metrics_rpm_is_in_range(baseVehicleMetrics_msg.rpm))
        {
            rpm = (uint32_t)binocan_base_vehicle_metrics_rpm_decode(baseVehicleMetrics_msg.rpm);
        }
        else
        {
            ESP_LOGW(TAG, "RPM signal out of range: %d", baseVehicleMetrics_msg.rpm);
            rpm = 0; // Default value
        }

        if (binocan_base_vehicle_metrics_speed_is_in_range(baseVehicleMetrics_msg.speed))
        {
            speed = (float)binocan_base_vehicle_metrics_speed_decode(baseVehicleMetrics_msg.speed);
        }
        else
        {
            ESP_LOGW(TAG, "Speed signal out of range: %d", baseVehicleMetrics_msg.speed);
            speed = 0; // Default value
        }

        if (binocan_base_vehicle_metrics_ignition_st_is_in_range(baseVehicleMetrics_msg.ignition_st))
        {
            binocan_base_vehicle_metrics_ignition_st_decode(baseVehicleMetrics_msg.ignition_st) == BINOCAN_BASE_VEHICLE_METRICS_IGNITION_ST_ON_CHOICE ? ignitionST = true : ignitionST = false;
        }
        else
        {
            ESP_LOGW(TAG, "Ignition Status signal out of range: %d", baseVehicleMetrics_msg.ignition_st);
            ignitionST = false; // Default value
        }

        ESP_LOGD(TAG, "Vehicle Metrics: RPM %lu, Speed %.2f, Ignition Status %d",
                 rpm, speed, ignitionST);

        break;

    case BINOCAN_EXTRA_OIL_METRICS_FRAME_ID:
        binocan_extra_oil_metrics_unpack(&baseExtraOilMetrics_msg, rxMsg->data, rxMsg->data_length_code);

        break;

    case BINOCAN_EXTRA_CHARGECOOLING_METRICS_FRAME_ID:
        binocan_extra_chargecooling_metrics_unpack(&baseChargeCoolingMetrics_msg, rxMsg->data, rxMsg->data_length_code);

        break;
    default:
        ESP_LOGW(TAG, "Unknown CAN frame received: ID = 0x%03X", (uint16_t)rxMsg->identifier);
        generatorOn = true;
        break;
    }

    return ESP_OK;
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
    ESP_UTILS_CHECK_FALSE_EXIT(lvgl_port_lock(-1), "Failed to perform initial LVGL Mutex lock");
    ui_init();                                                               // Load the UI library and draw it
    lv_obj_set_style_pad_radial(objects.speed_scale, 15, LV_PART_INDICATOR); // Pad the scale labels away from the tick marks
    // needleLine = lv_line_create(objects.speed_scale); // Create the needle line indicator
    // lv_obj_set_style_line_color(needleLine, lv_palette_main(LV_PALETTE_RED),LV_PART_MAIN); // Set the needle to red
    // lv_obj_set_style_line_width(needleLine,8,LV_PART_MAIN);
    // lv_obj_set_style_length(needleLine, 20, LV_PART_MAIN);
    // lv_obj_set_style_line_rounded(needleLine,false,LV_PART_MAIN);
    // lv_obj_set_style_pad_right(needleLine,50,LV_PART_MAIN);
    // Following only needed when decimation is used
    static const char *scale_labels[14] = {"0", "20", "40", "60", "80", "100", "120", "140", "160", "180", "200", "220", "240", NULL};
    lv_scale_set_text_src(objects.speed_scale, scale_labels);

    // Masking circle
    //  lv_obj_t *maskCircle = lv_obj_create(objects.speed_scale);
    //  lv_obj_set_size(maskCircle, 300, 300);
    //  lv_obj_center(maskCircle);
    //  lv_obj_set_style_radius(maskCircle, LV_RADIUS_CIRCLE,0);
    //  lv_obj_set_style_bg_color(maskCircle,lv_obj_get_style_bg_color(lv_scr_act(),LV_PART_MAIN),0);
    //  lv_obj_set_style_bg_opa(maskCircle, LV_OPA_COVER,0);
    //  lv_obj_set_style_border_width(maskCircle,0,LV_PART_MAIN);

    // lv_arc_align_obj_to_angle(objects.speed_arc, objects.speed_needle, 0);
    // lv_arc_rotate_obj_to_angle(objects.speed_arc, objects.speed_needle, 0);
    lvgl_port_unlock();

    ESP_LOGI(TAG, "Backlight : %d", backLight->on());

    ESP_LOGI(TAG, "Starting TWAI port and daemon");
    ESP_UTILS_CHECK_ERROR_EXIT(initCAN(&dispatchFrame), "Failed to initialize TWAI port and daemon");
    // if (initCAN(&dispatchFrame) != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "Failed to initialize TWAI port and daemon");
    // }
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