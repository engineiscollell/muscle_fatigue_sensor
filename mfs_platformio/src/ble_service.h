#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include <Arduino.h>
#include <NimBLEDevice.h>

// UUIDs per al servei i la característica (Fes-los servir al Python)
#define SERVICE_UUID           "abcd-1234-5678-90ab-cdef12345678"
#define CHARACTERISTIC_UUID    "1234abcd-5678-90ab-cdef-1234567890ab"

// Inicialitza el Bluetooth
void bleInit();

// Envia la dada de SmO2
void bleSendData(uint16_t smo2_value);

#endif