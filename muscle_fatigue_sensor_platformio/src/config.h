#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// ============================================================================
// MODE
// ============================================================================

extern bool g_isSimulation;

// ============================================================================
// ESTATS ÒPTICS
// ============================================================================

enum class OpticalState : uint8_t
{
    RED = 0,
    DARK_RED,

    IR,
    DARK_IR,

    MAX_STATES
};

// ============================================================================
// GPIO
// ============================================================================

// ADC
constexpr uint8_t PIN_ADC_CS   = 5;
constexpr uint8_t PIN_ADC_DRDY = 19;

// LEDs
constexpr uint8_t PIN_LED_RED  = 26;
constexpr uint8_t PIN_LED_IR   = 25;

// ============================================================================
// DAC
// ============================================================================

// ~0.5V sobre DAC ESP32
constexpr uint8_t DAC_VAL_ON  = 39;

constexpr uint8_t DAC_VAL_OFF = 0;

// ============================================================================
// TIMING
// ============================================================================

// Període complet adquisició
constexpr uint32_t CYCLE_PERIOD_MS = 20;

// Temps estabilització LED/fotodíode
constexpr uint32_t SETTLING_MS = 1;

// ============================================================================
// BLE
// ============================================================================

constexpr const char* SERVICE_UUID =
    "d4c2a937-2e1a-4d7c-8f9b-1e5b6c3a8d2f";

constexpr const char* CHARACTERISTIC_UUID =
    "8a3e2b1f-4c5d-4a9b-8c7d-3e2f1a5b6c4d";

// ============================================================================
// PROCESSAMENT
// ============================================================================

// Moving average
constexpr uint8_t AVG_WINDOW_SIZE = 25;

// Intensitat mínima senyal òptica
constexpr float MIN_SIGNAL_INTENSITY = 100.0f;

// ============================================================================
// COEFICIENTS BEER-LAMBERT
// ============================================================================

// 660nm
constexpr float EPS_O2_RED = 319.6f;
constexpr float EPS_HB_RED = 3226.56f;

// 940nm
constexpr float EPS_O2_IR = 1214.0f;
constexpr float EPS_HB_IR = 693.44f;

// ============================================================================
// DEBUG
// ============================================================================

constexpr bool ENABLE_DEBUG = true;

#endif