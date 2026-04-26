#include "adc_sim.h"
#include "config.h"
#include <Arduino.h>
#include <math.h>


static const uint32_t ADC_CONVERSION_TIME_MS = 1; 

static OpticalState g_requestedState = OpticalState::RED;
static uint8_t g_simByteIndex = 0; // simula el numero de byte del convertidor que llegim, si el 1er, 2on o 3er. 
static bool g_simWaitingCommand = true;

struct SimAdcContext {
    bool conversionRunning;
    bool dataReady;
    uint32_t conversionStartMs;
    uint32_t conversionTimeMs;
    uint32_t currentCode;
};

static SimAdcContext g_adc = {
    false, false, 0, ADC_CONVERSION_TIME_MS, 0
};

float effortTrend() {
    float t = millis() / 1000.0f;

    if (t < 10.0f) {
        return 0.0f;                  // repòs
    } else if (t < 40.0f) {
        return (t - 10.0f) / 30.0f;   // puja de 0 a 1
    } else if (t < 70.0f) {
        return 1.0f - (t - 40.0f) / 30.0f; // baixa de 1 a 0
    } else {
        return 0.0f;                  // recuperat
    }
}

float simulateRedLevel() {
    float t = millis() / 1000.0f;
    float trend = effortTrend();
    float base = 0.42f + 0.03f * sinf(2.0f * PI * 0.10f * t);
    return  base +0.05f *trend;
}

float simulateIRLevel() {
    float t = millis() / 1000.0f;
    float trend = effortTrend();
    float base = 0.56f + 0.035f * sinf(2.0f * PI * 0.10f * t + 0.7f);
    return base - 0.05f * trend;
}

float simulateAmbientLevel() {
    float t = millis() / 1000.0f;
    return 0.015f + 0.003f * sinf(2.0f * PI * 0.03f * t + 1.2f);
}

float simulateNoise() {
    float t = millis() / 1000.0f;
    return 0.0015f * sinf(2.0f * PI * 1.7f * t);
}

uint32_t generateSimADCValueForState(OpticalState state) {
    float value = 0.0f;

    switch (state) {
        case OpticalState::RED:
            value = simulateRedLevel() + simulateAmbientLevel() + simulateNoise();
            break;
        case OpticalState::DARK_RED:
            value = simulateAmbientLevel() + simulateNoise();
            break;
        case OpticalState::IR:
            value = simulateIRLevel() + simulateAmbientLevel() + simulateNoise();
            break;
        case OpticalState::DARK_IR:
            value = simulateAmbientLevel() + simulateNoise();
            break;
    }

    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;

    return (uint32_t)(value * 16777215.0f);
}

void adcInit() {
    g_adc.conversionRunning = false;
    g_adc.dataReady = false;
    g_adc.currentCode = 0;
}

void adcStartConversion(OpticalState state) {
    g_requestedState = state;   // Guardem l'estat
    g_adc.conversionRunning = true;
    g_adc.dataReady = false;
    g_adc.conversionStartMs = millis();
}

// Dins de adc_sim.cpp
extern SemaphoreHandle_t xDrdySemaphore; // Referència al semàfor definit a main.cpp

void adcUpdate() {
    if (!g_adc.conversionRunning) return;

    // Simulem el pas del temps (2ms)
    if ((millis() - g_adc.conversionStartMs) >= g_adc.conversionTimeMs) {
        g_adc.conversionRunning = false;
        g_adc.dataReady = true;
        g_adc.currentCode = generateSimADCValueForState(g_requestedState);
        
        // SIMULEM LA INTERRUPCIÓ: Donem el semàfor per software
        xSemaphoreGive(xDrdySemaphore);
    }
}

void simSPIStartFrame() {
    g_simByteIndex = 0;
    g_simWaitingCommand = true;
}

uint8_t simSPItransfer(uint8_t txByte) {
    (void)txByte;

    if (g_simWaitingCommand) {
        g_simWaitingCommand = false;
        return 0x00; // resposta dummy a la comanda READ
    }

    uint8_t outByte = 0;

    if (g_simByteIndex == 0) {
        outByte = (g_adc.currentCode >> 16) & 0xFF;
    } else if (g_simByteIndex == 1) {
        outByte = (g_adc.currentCode >> 8) & 0xFF;
    } else {
        outByte = g_adc.currentCode & 0xFF;
    }

    g_simByteIndex++;
    if (g_simByteIndex >= 3) {
        g_simByteIndex = 0;
    }

    return outByte;
}

bool adcIsReady() {
    return g_adc.dataReady;
}

uint32_t adcReadSample() {
    uint32_t value = 0;

    simSPIStartFrame();

    // equivalent a enviar la comanda de lectura del registre DATA
    simSPItransfer(0x40 | 0x04);

    for (uint8_t i = 0; i < 3; ++i) {
        value = (value << 8) | simSPItransfer(0x00);
    }

    g_adc.dataReady = false;

    return value;
}
// Afegeix això al final de adc_sim.cpp
bool adcIsDataReady() {
    return g_adc.dataReady;
}