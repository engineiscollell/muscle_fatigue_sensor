#include "sensor_hal.h"
#include "adc_sim.h"
#include "adc_hw.h"

extern SemaphoreHandle_t xDrdySemaphore; // Referència al semàfor creat a main.cpp

// Hem mogut la interrupció aquí perquè és gestió de hardware
void IRAM_ATTR drdy_ISR() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xDrdySemaphore, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) portYIELD_FROM_ISR();
}

void sensorInit() {
    if (g_isSimulation) {
        adcInit(); 
    } else {
        ad7172_init(PIN_SPI_CS);
        pinMode(PIN_DRDY, INPUT);
        attachInterrupt(digitalPinToInterrupt(PIN_DRDY), drdy_ISR, FALLING);
    }
}

uint32_t sensorReadSample(OpticalState state) {
    if (g_isSimulation) {
        adcStartConversion(state);
        // Esperem de forma que FreeRTOS pugui fer altres coses
        while (!adcIsDataReady()) { 
            adcUpdate(); 
            vTaskDelay(1); 
        }
        return adcReadSample();
    } else {
        ad7172_start_single_conversion();
        // Esperem la interrupció (o timeout de 50ms per seguretat)
        if (xSemaphoreTake(xDrdySemaphore, pdMS_TO_TICKS(50)) == pdFALSE) {
            Serial.println("Error: Timeout esperant ADC real!");
            return 0; 
        }
        return ad7172_read_data24();
    }
}