#pragma once
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "stdio.h"
#include "stdlib.h"
#include "driver/twai.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"


#ifndef CAN_RX
#define CAN_RX 43
#endif

#ifndef CAN_TX
#define CAN_TX 44
#endif

#ifndef CAN_POLL_MS
#define CAN_POLL_MS 50
#endif

// Only active if the TWAI_WATCHDOG is used




typedef esp_err_t frameDispatcher_t(twai_message_t *messageToDispatch);

// Pointer to dispatcher function, attached on init and defined externally
static frameDispatcher_t *dispatchCANFrame = nullptr;

// Pointer to rx and dispatch task handle
static TaskHandle_t CANTaskHandle = nullptr;

/// @brief Initialises the TWAI driver and attaches the frame dispatcher function
/// @param frameDispatcher 
/// @return OK when all is well, otherwise an error code
esp_err_t initCAN(frameDispatcher_t *frameDispatcher);

/// @brief FreeRTOS task that receives and send frames to the dispatcher function
/// @param arg 
void CANTask(void *arg);