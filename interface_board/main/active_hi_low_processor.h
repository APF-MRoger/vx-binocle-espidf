// Uses IO expander interrupt and other GPIO interrupts to trigger update of the internal signals
// Uses task notification from interrupts to read the IO expander
#pragma once
#include "tca9555_helpers.h"

#ifdef TAG
#undef TAG
#endif

#define TAG "ActHiLo Processor"

#define EXP_IO_0_BITMASK (1 << 0)
#define EXP_IO_1_BITMASK (1 << 1)
#define EXP_IO_2_BITMASK (1 << 2)
#define EXP_IO_3_BITMASK (1 << 3)
#define EXP_IO_4_BITMASK (1 << 4)
#define EXP_IO_5_BITMASK (1 << 5)
#define EXP_IO_6_BITMASK (1 << 6)
#define EXP_IO_7_BITMASK (1 << 7)
#define EXP_IO_8_BITMASK (1 << 8)
#define EXP_IO_9_BITMASK (1 << 9)
#define EXP_IO_10_BITMASK (1 << 10)
#define EXP_IO_11_BITMASK (1 << 11)
#define EXP_IO_12_BITMASK (1 << 12)
#define EXP_IO_13_BITMASK (1 << 13)
#define EXP_IO_14_BITMASK (1 << 14)
#define EXP_IO_15_BITMASK (1 << 15)

struct active_hi_lo_grp_t {
bool AH_ignition = false;
bool AH_hi_beams = false;
bool AL_alternator = false;
bool AL_brake_low = false;
bool AL_parking_brake = false;
bool AL_oil_pressure = false;
bool AL_airbag = false;
bool AL_CEL = false;
bool AH_right_turn = false;
bool AH_left_turn = false;
bool AL_ABS = false;
bool AL_door = false;
bool AL_coolant_low = false;
bool AL_button = false;
bool AH_B07 = false;
bool AH_backlight = false;
} active_hi_lo_grp;

/// @brief Utility that associates a boolean return to a position being ON in the bitmask
/// @param bitfield 8-bit bitfield containing the various values of the IO expander
/// @param bitmask Bitmask to filter the exact boolean to parse
/// @return
bool read_bitmask(uint16_t bitfield, uint16_t bitmask)
{
    bool ret = false;
    ret = ((bitfield & bitmask) == bitmask);
    return ret;
}

/// @brief Notified task to read register of expander and map it to global variables
/// @param pvParameters
void exp_active_hi_lo_process(void *pvParameters)
{
    uint16_t raw;
    TaskHandle_t* subProcessTaskHandle = (TaskHandle_t*)(pvParameters);
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(CONFIG_EXPANDER_POLLING_RATE_MS));
        // Use taskENTER_CRITICAL(&spinlock) and taskEXIT_CRITICAL(&spinlock) to pause the interrupt ?
        if (tca95x5_port_read(&tca_slave, &raw) != ESP_OK)
        {
            ESP_LOGE(TAG, "Impossible to fetch register from expander");
        }
        else
        {
            ESP_LOGI(TAG, "Raw ISR: %04X", raw);
            if (xSemaphoreTake(exp_act_high_low_sem, pdMS_TO_TICKS(1)) == pdTRUE)
            {
                active_hi_lo_grp.AH_ignition = read_bitmask(raw, EXP_IO_0_BITMASK);
                active_hi_lo_grp.AH_hi_beams = read_bitmask(raw, EXP_IO_1_BITMASK);
                active_hi_lo_grp.AL_alternator = read_bitmask(raw, EXP_IO_2_BITMASK);
                active_hi_lo_grp.AL_brake_low = read_bitmask(raw, EXP_IO_3_BITMASK);
                active_hi_lo_grp.AL_parking_brake = read_bitmask(raw, EXP_IO_4_BITMASK);
                active_hi_lo_grp.AL_oil_pressure = read_bitmask(raw, EXP_IO_5_BITMASK);
                active_hi_lo_grp.AL_airbag = read_bitmask(raw, EXP_IO_6_BITMASK);
                active_hi_lo_grp.AL_CEL = read_bitmask(raw, EXP_IO_7_BITMASK);
                active_hi_lo_grp.AH_right_turn = read_bitmask(raw, EXP_IO_8_BITMASK);
                active_hi_lo_grp.AH_left_turn = read_bitmask(raw, EXP_IO_9_BITMASK);
                active_hi_lo_grp.AL_ABS = read_bitmask(raw, EXP_IO_10_BITMASK);
                active_hi_lo_grp.AL_door = read_bitmask(raw, EXP_IO_11_BITMASK);
                active_hi_lo_grp.AL_coolant_low = read_bitmask(raw, EXP_IO_12_BITMASK);
                active_hi_lo_grp.AL_button = read_bitmask(raw, EXP_IO_13_BITMASK);
                active_hi_lo_grp.AH_B07 = read_bitmask(raw, EXP_IO_14_BITMASK);
                active_hi_lo_grp.AH_backlight = read_bitmask(raw, EXP_IO_15_BITMASK);
                xSemaphoreGive(exp_act_high_low_sem);
                if(subProcessTaskHandle!=NULL)
                {
                    xTaskNotify(*subProcessTaskHandle,1,eSetValueWithoutOverwrite);
                }
                else
                {
                    ESP_LOGW(TAG,"subProcessTaskHandle is not defined");
                }
                
            }
            else
            {
                ESP_LOGW(TAG, "Could not take Semaphore for active high low");
            }

            // Mutex OUT
        }
    }
}

/// @brief Initialisation routine for expander IO active high low routines
/// @return ESP_OK when all initialised correctly, otherwise ESP_FAIL
esp_err_t initialize_exp_active_hi_lo_proc(TaskHandle_t* daughterTaskHandle)
{
    if (initialize_io_expanders() != ESP_OK)
    {
        return ESP_FAIL;
    }

    if (xTaskCreate(exp_active_hi_lo_process, "Expander ActHL processor", 2048, daughterTaskHandle, 6, &processor_task_hdl) != pdPASS)
    {
        ESP_LOGE(TAG, "Could not create exp IO processor task.");
        return ESP_FAIL;
    }

    return ESP_OK;
}
