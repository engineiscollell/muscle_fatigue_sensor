#pragma once

#include <stdint.h>
#include <stdbool.h>

// Inicialitza el servei BLE
void bleInit();

// Indica si hi ha un client connectat
bool bleIsConnected();

// Envia SmO2 en format uint16_t (valor *10)
// Exemple: 73.6% → 736
void bleNotifySmO2(uint16_t smo2_x10);