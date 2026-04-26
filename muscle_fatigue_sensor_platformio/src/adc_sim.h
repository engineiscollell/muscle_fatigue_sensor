#ifndef ADC_SIM_H
#define ADC_SIM_H

#include <Arduino.h>
#include "config.h" 

void adcInit();
void adcStartConversion(OpticalState state); // <-- El canvi està aquí
void adcUpdate();
bool adcIsDataReady();
uint32_t adcReadSample();

#endif