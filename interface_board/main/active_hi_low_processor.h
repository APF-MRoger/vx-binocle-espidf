// Uses IO expander interrupt and other GPIO interrupts to trigger update of the internal signals
// Uses task notification from interrupts to read the IO expander
#pragma once
#include "pcf8574_helpers.h"

#ifdef TAG
#undef TAG
#endif

#define TAG "ActHiLo Processor"

// Should move to KConfig later
#define AUTO_READ_EXP_IO_MS 200

#define EXP_IO_0_BITMASK (1 << 0)
#define EXP_IO_1_BITMASK (1 << 1)
#define EXP_IO_2_BITMASK (1 << 2)
#define EXP_IO_3_BITMASK (1 << 3)
#define EXP_IO_4_BITMASK (1 << 4)
#define EXP_IO_5_BITMASK (1 << 5)
#define EXP_IO_6_BITMASK (1 << 6)
#define EXP_IO_7_BITMASK (1 << 7)

bool glob_act_hi_lo_var1 = false;
bool glob_act_hi_lo_var2 = false;
bool glob_act_hi_lo_var3 = false;
bool glob_act_hi_lo_var4 = false;
bool glob_act_hi_lo_var5 = false;
bool glob_act_hi_lo_var6 = false;
bool glob_act_hi_lo_var7 = false;
bool glob_act_hi_lo_var8 = false;

/// @brief Utility that associates a boolean return to a position being ON in the bitmask
/// @param bitfield 8-bit bitfield containing the various values of the IO expander
/// @param bitmask Bitmask to filter the exact boolean to parse
/// @return
bool read_bitmask(uint8_t bitfield, uint8_t bitmask)
{
    bool ret = false;
    ret = ((bitfield & bitmask) == bitmask);
    return ret;
}

/// @brief Notified task to read register of expander and map it to global variables
/// @param pvParameters
void exp_active_hi_lo_process(void *pvParameters)
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(AUTO_READ_EXP_IO_MS));
        if (pcf8574_port_read(&pcf_slave, &raw_isr) != ESP_OK)
        {
            ESP_LOGE(TAG, "Impossible to fetch register from expander");
        }
        else
        {
            ESP_LOGD(TAG, "Raw ISR: %02X", raw_isr);
            if (xSemaphoreTake(exp_act_high_low_sem, pdMS_TO_TICKS(2)) == pdTRUE)
            {
                glob_act_hi_lo_var1 = read_bitmask(raw_isr, EXP_IO_0_BITMASK);
                glob_act_hi_lo_var2 = read_bitmask(raw_isr, EXP_IO_1_BITMASK);
                glob_act_hi_lo_var3 = read_bitmask(raw_isr, EXP_IO_2_BITMASK);
                glob_act_hi_lo_var4 = read_bitmask(raw_isr, EXP_IO_3_BITMASK);
                glob_act_hi_lo_var5 = read_bitmask(raw_isr, EXP_IO_4_BITMASK);
                glob_act_hi_lo_var6 = read_bitmask(raw_isr, EXP_IO_5_BITMASK);
                glob_act_hi_lo_var7 = read_bitmask(raw_isr, EXP_IO_6_BITMASK);
                glob_act_hi_lo_var8 = read_bitmask(raw_isr, EXP_IO_7_BITMASK);
                xSemaphoreGive(exp_act_high_low_sem);
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
esp_err_t initialize_exp_active_hi_lo_proc()
{
    if (initialize_io_expanders() != ESP_OK)
    {
        return ESP_FAIL;
    }

    if (xTaskCreate(exp_active_hi_lo_process, "Expander ActHL processor", 2048, NULL, 6, &processor_task_hdl) != pdPASS)
    {
        ESP_LOGE(TAG, "Could not create exp IO processor task.");
        return ESP_FAIL;
    }

    return ESP_OK;
}
