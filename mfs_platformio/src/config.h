#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =================================================================
// 1. PINS DAC PER ALS LEDS (Control directe de potència)
// =================================================================
static constexpr int PIN_DAC_RED = 25; 
static constexpr int PIN_DAC_IR  = 26; 

static constexpr uint8_t DAC_VAL_05V = 39; 
static constexpr uint8_t DAC_VAL_OFF = 0;

// =================================================================
// 2. COMUNICACIÓ SPI I PIN COMPARTIT (AD7172-2)
// =================================================================
static constexpr int PIN_ADC_CS   = 5;
static constexpr int PIN_ADC_MISO_DRDY = 19; // Pin compartit DOUT/DRDY
static constexpr int PIN_ADC_SCK  = 18;
static constexpr int PIN_ADC_MOSI = 23;

// =================================================================
// 3. CONFIGURACIÓ DE MOSTRATGE (SPS)
// =================================================================
// Definim 1000 SPS per tenir 1ms de conversió.
// El valor 0x000B correspon a la configuració del registre FILTCON 
// de l'AD7172-2 per obtenir aquesta velocitat.
static constexpr uint16_t ADC_SPS_1000 = 0x000B; 

// =================================================================
// 4. TEMPORITZACIÓ DEL CICLE ÒPTIC
// =================================================================
static constexpr uint32_t PHASE_MS = 2;        // Finestra de cada fase
static constexpr uint32_t TOTAL_CYCLE_MS = 20;  // Temps total del cicle (4 fases + repòs)

// =================================================================
// 5. ESTRUCTURES DE DADES
// =================================================================
enum class OpticalState : uint8_t {
    RED = 0,
    DARK_RED = 1,
    IR = 2,
    DARK_IR = 3
};

struct OpticalRawFrame {
    uint32_t timestampMs;
    uint32_t red;
    uint32_t darkRed;
    uint32_t ir;
    uint32_t darkIr;
};

#endif