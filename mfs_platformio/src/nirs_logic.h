#ifndef NIRS_LOGIC_H
#define NIRS_LOGIC_H

#include <Arduino.h>
#include "config.h"

// Inicialitza els pins DAC i el semàfor
void nirsInit();

// Executa la màquina d'estats (Es cridarà des de la Tasca del Core 1)
void nirsRunCycle();

// Funció que cridarà la ISR (Interrupció) del pin 19
void IRAM_ATTR nirsNotifyDataReady();

#endif