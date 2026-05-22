#include <Arduino.h>
#include <SPI.h>

#include <driver/dac.h>

#include "config.h"
#include "nirs_logic.h"
#include "adc_sim.h"
#include "adc_hw.h"
#include "ble_service.h"

// ============================================================================
// GLOBALS
// ============================================================================

bool g_isSimulation = false;
bool g_isDebuging = true;

QueueHandle_t xRawQueue;
SemaphoreHandle_t xSmO2Mutex;
SemaphoreHandle_t xDrdySemaphore;

float g_sharedSmO2 = 0.0f;

// ============================================================================
// LED CONTROL
// ============================================================================

void leds_all_off()
{
    dac_output_voltage(DAC_CHANNEL_1, 0);
    dac_output_voltage(DAC_CHANNEL_2, 0);

    dac_output_disable(DAC_CHANNEL_1);
    dac_output_disable(DAC_CHANNEL_2);

    pinMode(PIN_LED_RED, INPUT);
    pinMode(PIN_LED_IR, INPUT);
}

void led_red_on(uint8_t intensity)
{
    dac_output_disable(DAC_CHANNEL_1);

    pinMode(PIN_LED_RED, OUTPUT);

    dac_output_enable(DAC_CHANNEL_2);
    dac_output_voltage(DAC_CHANNEL_2, intensity);
}

void led_ir_on(uint8_t intensity)
{
    dac_output_disable(DAC_CHANNEL_2);

    pinMode(PIN_LED_IR, OUTPUT);

    dac_output_enable(DAC_CHANNEL_1);
    dac_output_voltage(DAC_CHANNEL_1, intensity);
}

void optical_set_state(OpticalState state)
{
    switch (state)
    {
        case OpticalState::RED:
            led_red_on(DAC_VAL_ON);
            break;

        case OpticalState::IR:
            led_ir_on(DAC_VAL_ON);
            break;

        default:
            leds_all_off();
            break;
    }
}

void leds_init()
{
    dac_output_enable(DAC_CHANNEL_1);
    dac_output_enable(DAC_CHANNEL_2);

    leds_all_off();
}

// ============================================================================
// TASK ACQUISITION
// ============================================================================

void TaskAcquisition(void *pvParameters)
{
    // ========================================================================
    // INIT SENSOR
    // ========================================================================

    if (g_isSimulation)
    {
        adcSimInit();
    }
    else
    {
        adcHwInit();
    }

    OpticalRawFrame currentFrame = {};

    TickType_t xLastWakeTime =
        xTaskGetTickCount();

    // ========================================================================
    // MAIN LOOP
    // ========================================================================

    for (;;)
    {
        for (
            uint8_t i = 0;
            i < static_cast<uint8_t>(OpticalState::MAX_STATES);
            i++
        )
        {
            OpticalState currentState =
                static_cast<OpticalState>(i);

            // ================================================================
            // LED CONTROL
            // ================================================================

            if (!g_isSimulation)
            {
                optical_set_state(currentState);

                vTaskDelay(
                    pdMS_TO_TICKS(SETTLING_MS)
                );
            }

            // ================================================================
            // SAMPLE ACQUISITION
            // ================================================================

            uint32_t sample = 0;

            if (g_isSimulation)
            {
                sample =
                    adcSimReadSample(currentState);
            }
            else
            {
                sample =
                    adcHwReadSample();
            }

            // ================================================================
            // TIMEOUT PROTECTION
            // ================================================================

            if (sample == 0 && !g_isSimulation)
            {
                continue;
            }

            // ================================================================
            // STORE FRAME
            // ================================================================

            switch (currentState)
            {
                case OpticalState::RED:
                    currentFrame.red = sample;
                    break;

                case OpticalState::DARK_RED:
                    currentFrame.darkRed = sample;
                    break;

                case OpticalState::IR:
                    currentFrame.ir = sample;
                    break;

                case OpticalState::DARK_IR:
                    currentFrame.darkIr = sample;
                    break;

                default:
                    break;
            }
        }

        // ====================================================================
        // SEND FRAME
        // ====================================================================

        xQueueSend(
            xRawQueue,
            &currentFrame,
            0
        );

        vTaskDelayUntil(
            &xLastWakeTime,
            pdMS_TO_TICKS(CYCLE_PERIOD_MS)
        );

        if(g_isDebuging){
            Serial.printf(
                "RED:%lu DARK_RED:%lu IR:%lu DARK_IR:%lu\n",
                currentFrame.red,
                currentFrame.darkRed,
                currentFrame.ir,
                currentFrame.darkIr
            );
        }
    }
}

// ============================================================================
// TASK PROCESSING
// ============================================================================

void TaskProcessing(void *pvParameters)
{
    OpticalRawFrame frame;

    const int CAL_SAMPLES = 50;

    float sumRed = 0.0f;
    float sumIr  = 0.0f;

    Serial.println(
        "[PROC] Iniciant calibratge..."
    );

    // ========================================================================
    // CALIBRATION
    // ========================================================================

    for (int i = 0; i < CAL_SAMPLES; i++)
    {
        if (
            xQueueReceive(
                xRawQueue,
                &frame,
                portMAX_DELAY
            ) == pdPASS
        )
        {
            sumRed +=
                (float)(frame.red - frame.darkRed);

            sumIr +=
                (float)(frame.ir - frame.darkIr);

            if (i % 10 == 0)
            {
                Serial.printf(
                    "Calibrant... %d%%\n",
                    (i * 100 / CAL_SAMPLES)
                );
            }
        }
    }

    // ========================================================================
    // SAVE BASELINE
    // ========================================================================

    g_I0_red =
        sumRed / (float)CAL_SAMPLES;

    g_I0_ir =
        sumIr / (float)CAL_SAMPLES;

    Serial.printf(
        "[PROC] Calibratge OK! "
        "I0_RED=%.1f I0_IR=%.1f\n",
        g_I0_red,
        g_I0_ir
    );

    // ========================================================================
    // MAIN LOOP
    // ========================================================================

    while (1)
    {
        if (
            xQueueReceive(
                xRawQueue,
                &frame,
                portMAX_DELAY
            ) == pdPASS
        )
        {
            float smo2 =
                calculateNIRS(frame);

            xSemaphoreTake(
                xSmO2Mutex,
                portMAX_DELAY
            );

            g_sharedSmO2 = smo2;

            xSemaphoreGive(
                xSmO2Mutex
            );
        }
    }
}

// ============================================================================
// TASK BLE
// ============================================================================

void TaskBLE(void *pvParameters)
{
    Serial.println("[TASK] BLE iniciada");

    for (;;)
    {
        vTaskDelay(
            pdMS_TO_TICKS(500)
        );

        float val = 0.0f;

        if (
            xSemaphoreTake(
                xSmO2Mutex,
                pdMS_TO_TICKS(10)
            )
        )
        {
            val = g_sharedSmO2;

            xSemaphoreGive(
                xSmO2Mutex
            );
        }

        bleNotifySmO2(
            (uint16_t)(val * 100.0f)
        );

        Serial.printf(
            "[BLE] %.2f\n",
            val
        );
    }
}

// ============================================================================
// SETUP
// ============================================================================

void setup()
{
    Serial.begin(115200);

    SPI.begin();

    leds_init();

    bleInit();

    // ========================================================================
    // RTOS OBJECTS
    // ========================================================================

    xRawQueue =
        xQueueCreate(
            10,
            sizeof(OpticalRawFrame)
        );

    xSmO2Mutex =
        xSemaphoreCreateMutex();

    xDrdySemaphore =
        xSemaphoreCreateBinary();

    // ========================================================================
    // TASKS
    // ========================================================================

    if (
        xRawQueue &&
        xSmO2Mutex &&
        xDrdySemaphore
    )
    {
        xTaskCreatePinnedToCore(
            TaskAcquisition,
            "Acq",
            4096,
            NULL,
            3,
            NULL,
            1
        );

        xTaskCreatePinnedToCore(
            TaskProcessing,
            "Proc",
            4096,
            NULL,
            2,
            NULL,
            0
        );

        xTaskCreatePinnedToCore(
            TaskBLE,
            "BLE",
            4096,
            NULL,
            1,
            NULL,
            0
        );
    }
}

// ============================================================================
// LOOP
// ============================================================================

void loop()
{
    vTaskDelete(NULL);
}