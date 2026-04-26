#ifndef TASKS_H
#define TASKS_H

#include <Arduino.h>

// Declaració de les tasques
void TaskAcquisition(void *pvParameters);
void TaskProcessing(void *pvParameters);
void TaskBLE(void *pvParameters);

// Handler per a la interrupció (necessari per a l'ADC real)
void IRAM_ATTR drdy_ISR();

#endif