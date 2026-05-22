// ============================================================================
// adc_sim.cpp
// ============================================================================

#include "adc_sim.h"

#include "config.h"

#include <Arduino.h>
#include <math.h>

// ============================================================================
// CONFIG
// ============================================================================

static const uint32_t ADC_CONVERSION_TIME_MS = 1;

// ============================================================================
// INTERNAL STATE
// ============================================================================

static OpticalState g_requestedState =
    OpticalState::RED;

static uint8_t g_simByteIndex = 0;

static bool g_simWaitingCommand = true;

// ============================================================================
// ADC CONTEXT
// ============================================================================

struct SimAdcContext
{
    bool conversionRunning;

    bool dataReady;

    uint32_t conversionStartMs;

    uint32_t conversionTimeMs;

    uint32_t currentCode;
};

static SimAdcContext g_adc =
{
    false,
    false,
    0,
    ADC_CONVERSION_TIME_MS,
    0
};

// ============================================================================
// SIGNAL MODEL
// ============================================================================

float effortTrend()
{
    float t =
        millis() / 1000.0f;

    if (t < 10.0f)
    {
        return 0.0f;
    }
    else if (t < 40.0f)
    {
        return
            (t - 10.0f) / 30.0f;
    }
    else if (t < 70.0f)
    {
        return
            1.0f -
            (t - 40.0f) / 30.0f;
    }

    return 0.0f;
}

float simulateRedLevel()
{
    float t =
        millis() / 1000.0f;

    float trend =
        effortTrend();

    float base =
        0.42f +
        0.03f *
        sinf(2.0f * PI * 0.10f * t);

    return
        base +
        0.05f * trend;
}

float simulateIRLevel()
{
    float t =
        millis() / 1000.0f;

    float trend =
        effortTrend();

    float base =
        0.56f +
        0.035f *
        sinf(
            2.0f *
            PI *
            0.10f *
            t +
            0.7f
        );

    return
        base -
        0.05f * trend;
}

float simulateAmbientLevel()
{
    float t =
        millis() / 1000.0f;

    return
        0.015f +
        0.003f *
        sinf(
            2.0f *
            PI *
            0.03f *
            t +
            1.2f
        );
}

float simulateNoise()
{
    float t =
        millis() / 1000.0f;

    return
        0.0015f *
        sinf(
            2.0f *
            PI *
            1.7f *
            t
        );
}

// ============================================================================
// GENERATE ADC VALUE
// ============================================================================

uint32_t generateSimADCValueForState(
    OpticalState state
)
{
    float value = 0.0f;

    switch (state)
    {
        case OpticalState::RED:

            value =
                simulateRedLevel() +
                simulateAmbientLevel() +
                simulateNoise();

            break;

        case OpticalState::DARK_RED:

            value =
                simulateAmbientLevel() +
                simulateNoise();

            break;

        case OpticalState::IR:

            value =
                simulateIRLevel() +
                simulateAmbientLevel() +
                simulateNoise();

            break;

        case OpticalState::DARK_IR:

            value =
                simulateAmbientLevel() +
                simulateNoise();

            break;

        default:
            break;
    }

    if (value < 0.0f)
    {
        value = 0.0f;
    }

    if (value > 1.0f)
    {
        value = 1.0f;
    }

    return
        (uint32_t)(
            value * 16777215.0f
        );
}

// ============================================================================
// INIT
// ============================================================================

void adcSimInit()
{
    g_adc.conversionRunning = false;

    g_adc.dataReady = false;

    g_adc.currentCode = 0;

    Serial.println(
        "[SIM] ADC Init OK"
    );
}

// ============================================================================
// START CONVERSION
// ============================================================================

static void adcStartConversion(
    OpticalState state
)
{
    g_requestedState = state;

    g_adc.conversionRunning = true;

    g_adc.dataReady = false;

    g_adc.conversionStartMs =
        millis();
}

// ============================================================================
// UPDATE ADC
// ============================================================================

static void adcUpdate()
{
    if (!g_adc.conversionRunning)
    {
        return;
    }

    if (
        (
            millis() -
            g_adc.conversionStartMs
        ) >= g_adc.conversionTimeMs
    )
    {
        g_adc.conversionRunning = false;

        g_adc.dataReady = true;

        g_adc.currentCode =
            generateSimADCValueForState(
                g_requestedState
            );
    }
}

// ============================================================================
// SIM SPI FRAME
// ============================================================================

static void simSPIStartFrame()
{
    g_simByteIndex = 0;

    g_simWaitingCommand = true;
}

// ============================================================================
// SIM SPI TRANSFER
// ============================================================================

static uint8_t simSPItransfer(
    uint8_t txByte
)
{
    (void)txByte;

    if (g_simWaitingCommand)
    {
        g_simWaitingCommand = false;

        return 0x00;
    }

    uint8_t outByte = 0;

    if (g_simByteIndex == 0)
    {
        outByte =
            (g_adc.currentCode >> 16) &
            0xFF;
    }
    else if (g_simByteIndex == 1)
    {
        outByte =
            (g_adc.currentCode >> 8) &
            0xFF;
    }
    else
    {
        outByte =
            g_adc.currentCode &
            0xFF;
    }

    g_simByteIndex++;

    if (g_simByteIndex >= 3)
    {
        g_simByteIndex = 0;
    }

    return outByte;
}

// ============================================================================
// READ SAMPLE
// ============================================================================

uint32_t adcSimReadSample(
    OpticalState state
)
{
    // ========================================================================
    // START CONVERSION
    // ========================================================================

    adcStartConversion(state);

    // ========================================================================
    // WAIT SIMULATED ADC
    // ========================================================================

    while (!g_adc.dataReady)
    {
        adcUpdate();

        delay(1);
    }

    // ========================================================================
    // READ SIMULATED SPI DATA
    // ========================================================================

    uint32_t value = 0;

    simSPIStartFrame();

    // Simulem READ DATA REGISTER

    simSPItransfer(
        0x40 | 0x04
    );

    for (uint8_t i = 0; i < 3; ++i)
    {
        value =
            (value << 8) |
            simSPItransfer(0x00);
    }

    g_adc.dataReady = false;

    return value;
}