#include <Arduino.h>
#include <SPI.h>
#include "config.h"
#include "nirs_logic.h"
#include "adc_sim.h"
#include "adc_hw.h"
#include "ble_service.h"
#include "tasks.h"

bool g_isSimulation = true; // Actualitzar també a 'config.h'.

QueueHandle_t xRawQueue;
SemaphoreHandle_t xSmO2Mutex;
SemaphoreHandle_t xDrdySemaphore; 
float g_sharedSmO2 = 0.0f;

void setup() {
    Serial.begin(115200);
    SPI.begin();
    bleInit();

    xRawQueue = xQueueCreate(10, sizeof(OpticalRawFrame));
    xSmO2Mutex = xSemaphoreCreateMutex();
    xDrdySemaphore = xSemaphoreCreateBinary();

    if (xRawQueue && xSmO2Mutex && xDrdySemaphore) {
        xTaskCreatePinnedToCore(TaskAcquisition, "Acq", 4096, NULL, 3, NULL, 1);
        xTaskCreatePinnedToCore(TaskProcessing, "Proc", 4096, NULL, 2, NULL, 0);
        xTaskCreatePinnedToCore(TaskBLE, "BLE", 4096, NULL, 1, NULL, 0);
    }
}

void loop() { vTaskDelete(NULL); }