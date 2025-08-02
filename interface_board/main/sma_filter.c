#include "sma_filter.h"
#include <stdlib.h> // For malloc and free

#ifdef TAG
#undef TAG
#endif
#define TAG "SMA"

sma_handle_t *sma_init_full(uint8_t size, int16_t startValue)
{
    if (size == 0)
        return NULL;

    sma_handle_t *sma = (sma_handle_t *)malloc(sizeof(sma_handle_t));
    if (sma == NULL)
        return NULL;

    sma->buffer = (int16_t *)calloc(size, sizeof(int16_t));
    if (sma->buffer == NULL)
    {
        free(sma);
        return NULL;
    }

    for (int i = 0; i < size; i++)
    {
        sma->buffer[i] = startValue;
    }
    

    sma->size = size;
    sma->head = 0;
    sma->sum = startValue*size;
    sma->count = size;
    sma->mutex = xSemaphoreCreateMutex();
    if (sma->mutex == NULL)
    {
        free(sma->buffer);
        free(sma);
        return NULL;
    }

    return sma;
}

sma_handle_t *sma_init(uint8_t size)
{
        if (size == 0)
        return NULL;

    sma_handle_t *sma = (sma_handle_t *)malloc(sizeof(sma_handle_t));
    if (sma == NULL)
        return NULL;

    sma->buffer = (int16_t *)calloc(size, sizeof(int16_t));
    if (sma->buffer == NULL)
    {
        free(sma);
        return NULL;
    }
    

    sma->size = size;
    sma->head = 0;
    sma->sum = 0;
    sma->count = 0;
    sma->mutex = xSemaphoreCreateMutex();
    if (sma->mutex == NULL)
    {
        free(sma->buffer);
        free(sma);
        return NULL;
    }

    return sma;
}

void sma_add(sma_handle_t *sma, int16_t value)
{
    if (sma == NULL || xSemaphoreTake(sma->mutex, portMAX_DELAY) != pdTRUE)
    {
        ESP_LOGE(TAG, "Could not add to SMA");
        return;
    }

    // Subtract the old value that is being replaced
    sma->sum -= sma->buffer[sma->head];

    // Add the new value to the buffer and the sum
    sma->buffer[sma->head] = value;
    sma->sum += value;

    // Move head to the next position
    sma->head = (sma->head + 1) % sma->size;

    // Keep track of the number of elements until the buffer is full
    if (sma->count < sma->size)
    {
        ESP_LOGI(TAG, "SMA buffer not full, count %u vs size %u", sma->count, sma->size);
        sma->count++;
    }

    xSemaphoreGive(sma->mutex);
}

float sma_get_avg(sma_handle_t *sma)
{
    if (sma == NULL || sma->count == 0)
    {
        return 0.0f;
    }

    float avg = 0.0f;
    if (xSemaphoreTake(sma->mutex, portMAX_DELAY) == pdTRUE)
    {
        avg = (float)sma->sum / sma->count;
        xSemaphoreGive(sma->mutex);
    }
    return avg;
}

void sma_deinit(sma_handle_t *sma)
{
    if (sma == NULL)
        return;

    vSemaphoreDelete(sma->mutex);
    free(sma->buffer);
    free(sma);
}