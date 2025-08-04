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
#define CAN_RX 3
#endif

#ifndef CAN_TX
#define CAN_TX 2
#endif

#ifndef CAN_RX_POLL_MS
#ifdef CONFIG_CAN_RX_POLLING_RATE_MS
#define CAN_RX_POLL_MS CONFIG_CAN_RX_POLLING_RATE_MS
#else
#define CAN_RX_POLL_MS 50
#endif
#endif

#ifndef CAN_TX_POLL_MS
#ifdef CONFIG_CAN_RX_POLLING_RATE_MS
#define CAN_TX_POLL_MS CONFIG_CAN_RX_POLLING_RATE_MS
#else
#define CAN_TX_POLL_MS 50
#endif
#endif

// Only active if the TWAI_WATCHDOG is used




typedef esp_err_t frameDispatcher_t(twai_message_t *messageToDispatch);

// Pointer to dispatcher function, attached on init and defined externally
static frameDispatcher_t *dispatchCANFrame = nullptr;

// Pointer to rx and dispatch task handle
static TaskHandle_t CAN_RX_tsk_hdl = nullptr;
static TaskHandle_t CAN_TX_tsk_hdl = nullptr;

/// @brief Queue for messages to be sent out
static QueueHandle_t CAN_TX_queue_hdl = nullptr;

/// @brief Initialises the TWAI driver and attaches the frame dispatcher function
/// @param frameDispatcher 
/// @return OK when all is well, otherwise an error code
esp_err_t initCAN(frameDispatcher_t *frameDispatcher);

/// @brief FreeRTOS task that receives and send frames to the dispatcher function
/// @param arg 
void CAN_RX_Task(void *pvParameters);

/// @brief FreeRTOS task that periodically sends messages stored in a queue.
/// @param pvParameters 
void CAN_TX_Task(void *pvParameters);