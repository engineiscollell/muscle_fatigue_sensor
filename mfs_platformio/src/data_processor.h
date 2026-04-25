#ifndef DATA_PROCESSOR_H
#define DATA_PROCESSOR_H

#include <Arduino.h>
#include "config.h"

// Constants de calibració (Ajustables segons proves reals)
// SmO2 = A - B * (Ràtio)
static constexpr float CALIB_A = 110.0; 
static constexpr float CALIB_B = 25.0;

// Processa un frame complet i retorna el valor de SmO2 escalat (x100)
uint16_t processNirsData(const OpticalRawFrame& frame);

#endif