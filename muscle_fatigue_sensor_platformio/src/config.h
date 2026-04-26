#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// CONFIGURACIÓ DE MODE
extern bool g_isSimulation; 

enum class OpticalState : uint8_t {
    RED = 0,
    DARK_RED,
    IR,
    DARK_IR,
    MAX_STATES // Saber quants estats hi ha (4)
};

// PINS HARDWARE
constexpr uint8_t PIN_DRDY = 19;
constexpr uint8_t PIN_SPI_CS = 5;
constexpr uint8_t PIN_LED_RED = 26;
constexpr uint8_t PIN_LED_IR = 25;

// PARÀMETRES DAC
constexpr uint8_t DAC_VAL_ON = 39; // Aprox. 0.5V -> 3.3V * 39/255
constexpr uint8_t DAC_VAL_OFF = 0; 

// TIMING (ms)
constexpr uint32_t CYCLE_PERIOD_MS = 20;
constexpr uint32_t SETTLING_MS = 1;

// BLE UUIDs
constexpr const char* SERVICE_UUID = "d4c2a937-2e1a-4d7c-8f9b-1e5b6c3a8d2f";
constexpr const char* CHARACTERISTIC_UUID = "8a3e2b1f-4c5d-4a9b-8c7d-3e2f1a5b6c4d";

// PROCESSAMENT
constexpr uint8_t AVG_WINDOW_SIZE = 25; // 0.5s (cada cicle 20ms).

// LLINDAR MÍNIM INTENSITAT ÒPTICA NETA
constexpr float MIN_SIGNAL_INTENSITY = 100.0f;

// CONSTANTS BEER-LAMBERT (660nm i 940nm)
constexpr float EPS_O2_RED = 319.6f;
constexpr float EPS_HB_RED = 3226.56f;
constexpr float EPS_O2_IR = 1214.0f;
constexpr float EPS_HB_IR = 693.44f;

// Referència d'intensitat: Valor màxim ADC 24 bits
constexpr float ADC_REFERENCE_I0 = 16777215.0f; //POSSIBLE MILLORA*

#endif