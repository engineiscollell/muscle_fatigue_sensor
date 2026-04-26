#ifndef SENSOR_HAL_H
#define SENSOR_HAL_H

#include <Arduino.h>
#include "config.h"

// Inicialitza el maquinari o el simulador segons g_isSimulation
void sensorInit();

// Llegeix una mostra de forma bloquejant (espera a que estigui llesta)
uint32_t sensorReadSample(OpticalState state);

#endif