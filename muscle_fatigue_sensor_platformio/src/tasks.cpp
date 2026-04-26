#include "tasks.h"
#include "config.h"
#include "nirs_logic.h"
#include "sensor_hal.h" // Cridem només al HAL
#include "ble_service.h"

extern QueueHandle_t xRawQueue;
extern SemaphoreHandle_t xSmO2Mutex;
extern float g_sharedSmO2;

void TaskAcquisition(void *pvParameters) {    
    // 1. Inicialitzem el sensor sense importar quin és (Simulat o Real)
    sensorInit();

    OpticalRawFrame currentFrame = {};
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        for (uint8_t i = 0; i < static_cast<uint8_t>(OpticalState::MAX_STATES); i++) {
            OpticalState currentState = static_cast<OpticalState>(i);

            // 2. Control dels LEDs
            if (currentState == OpticalState::RED) { 
                dacWrite(PIN_LED_RED, DAC_VAL_ON); 
                dacWrite(PIN_LED_IR, DAC_VAL_OFF); 
            }
            else if (currentState == OpticalState::IR) { 
                dacWrite(PIN_LED_RED, DAC_VAL_OFF); 
                dacWrite(PIN_LED_IR, DAC_VAL_ON); 
            }
            else { 
                dacWrite(PIN_LED_RED, DAC_VAL_OFF); 
                dacWrite(PIN_LED_IR, DAC_VAL_OFF); 
            }

            vTaskDelay(pdMS_TO_TICKS(SETTLING_MS));

            // 3. Lectura de la mostra a través del HAL (tant per sim com per real)
            uint32_t sample = sensorReadSample(currentState);

            if (sample == 0 && !g_isSimulation) continue; // Protecció contra timeouts

            // 4. Assignació de la dada al frame
            if      (currentState == OpticalState::RED)      currentFrame.red = sample;
            else if (currentState == OpticalState::DARK_RED) currentFrame.darkRed = sample;
            else if (currentState == OpticalState::IR)       currentFrame.ir = sample;
            else if (currentState == OpticalState::DARK_IR)  currentFrame.darkIr = sample;
        }

        if (xQueueSend(xRawQueue, &currentFrame, 0) != pdPASS) {
            // Error cua plena. De moment no fem res.
        }
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(CYCLE_PERIOD_MS));
    }
}

void TaskProcessing(void *pvParameters) {
    Serial.println("[TASK] Proc: Iniciant...");
    OpticalRawFrame raw;
    float sumMetric = 0.0f;
    int count = 0;

    for (;;) {
        if (xQueueReceive(xRawQueue, &raw, portMAX_DELAY)) {
            float metric = calculateNIRS(raw);
            
            sumMetric += metric;
            count++;

            // Quan arribem a 25 mostres (cada 500ms aprox)
            if (count >= AVG_WINDOW_SIZE) {
                float average = sumMetric / (float)AVG_WINDOW_SIZE;
                
                if (xSemaphoreTake(xSmO2Mutex, pdMS_TO_TICKS(10))) {
                    g_sharedSmO2 = average;
                    xSemaphoreGive(xSmO2Mutex);
                }
                
                Serial.printf("SmO2 (mostres %d): %.4f\n", count, average);
                
                sumMetric = 0.0f;
                count = 0;
            }
        }
    }
}

void TaskBLE(void *pvParameters) {
    Serial.println("[TASK] BLE: Iniciant...");
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(500));
        float val = 0.0f;
        if (xSemaphoreTake(xSmO2Mutex, pdMS_TO_TICKS(10))) {
            val = g_sharedSmO2;
            xSemaphoreGive(xSmO2Mutex);
        }
        bleNotifySmO2((uint16_t)(val * 100.0f));
        Serial.printf("[BLE] Notificat: %.2f\n", val);
    }
}