#include <Arduino.h>
#include <driver/dac.h>

#include "config.h"

// ============================================================================
// CONFIG TEST
// ============================================================================

constexpr uint8_t TEST_DAC_VALUE = 5;

constexpr uint32_t LED_ON_MS  = 20;
constexpr uint32_t LED_OFF_MS = 980;

// ============================================================================
// HELPERS LEDs
// ============================================================================

void ledsOff()
{
    // Desactivem DACs

    dac_output_disable(DAC_CHANNEL_1); // GPIO25 -> IR
    dac_output_disable(DAC_CHANNEL_2); // GPIO26 -> RED

    // Forcem LOW real

    pinMode(PIN_LED_RED, OUTPUT);
    digitalWrite(PIN_LED_RED, LOW);

    pinMode(PIN_LED_IR, OUTPUT);
    digitalWrite(PIN_LED_IR, LOW);
}

// ============================================================================
// RED DESACTIVAT
// ============================================================================

void redOn()
{
    // NO fem res

    pinMode(PIN_LED_RED, INPUT_PULLDOWN);

    Serial.println("[LED] RED DISABLED");
}

// ============================================================================
// IR ACTIU
// ============================================================================

void irOn()
{
    // OFF total abans

    ledsOff();

    delay(1);

    // Activem DAC IR

    dac_output_enable(DAC_CHANNEL_1); // GPIO25

    dac_output_voltage(DAC_CHANNEL_1, TEST_DAC_VALUE);

    Serial.printf("[LED] IR ON DAC=%u\n", TEST_DAC_VALUE);
}

// ============================================================================
// SETUP
// ============================================================================

void setup()
{
    Serial.begin(115200);

    Serial.println();
    Serial.println("=================================");
    Serial.println("TEST LEDs NIRS");
    Serial.println("=================================");

    ledsOff();

    // RED sempre drenat

    pinMode(PIN_LED_RED, INPUT_PULLDOWN);

    Serial.println("[SYSTEM] LEDs OFF");

    delay(1000);
}

// ============================================================================
// LOOP
// ============================================================================

void loop()
{
    // ========================================================================
    // TEST IR NOMÉS
    // ========================================================================

    irOn();

    delay(LED_ON_MS);

    ledsOff();

    Serial.println("[LED] IR OFF");

    delay(LED_OFF_MS);
}