#include "nirs_logic.h"
#include "adc_driver.h"

// Semàfor per sincronitzar l'adquisició amb la interrupció
SemaphoreHandle_t adcSemaphore;

// Variable per guardar el frame actual
OpticalRawFrame currentFrame;

void nirsInit() {
    // 1. Configurem els pins DAC com a sortida (encara que dacWrite ja ho fa)
    pinMode(PIN_DAC_RED, OUTPUT);
    pinMode(PIN_DAC_IR, OUTPUT);
    
    // 2. Creem el semàfor
    adcSemaphore = xSemaphoreCreateBinary();
    
    // 3. Inicialitzem l'ADC
    adcInit();
}

// Funció interna per canviar els voltatges dels DACs
void applyOpticalState(OpticalState state) {
    switch (state) {
        case OpticalState::RED:
            dacWrite(PIN_DAC_RED, DAC_VAL_05V);
            dacWrite(PIN_DAC_IR,  DAC_VAL_OFF);
            break;
        case OpticalState::IR:
            dacWrite(PIN_DAC_RED, DAC_VAL_OFF);
            dacWrite(PIN_DAC_IR,  DAC_VAL_05V);
            break;
        default: // Estats DARK
            dacWrite(PIN_DAC_RED, DAC_VAL_OFF);
            dacWrite(PIN_DAC_IR,  DAC_VAL_OFF);
            break;
    }
}

// Aquesta és la ISR que es dispara pel pin 19 (MISO/DRDY)
void IRAM_ATTR nirsNotifyDataReady() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // Apaguem la interrupció per no rebre soroll durant la lectura SPI
    detachInterrupt(digitalPinToInterrupt(PIN_ADC_MISO_DRDY));
    // Avisem que la dada ja és al pin
    xSemaphoreGiveFromISR(adcSemaphore, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) portYIELD_FROM_ISR();
}

uint32_t captureSample(OpticalState state) {
    // 1. Encendre LED segons estat
    applyOpticalState(state);
    
    // 2. Temps d'estabilització (settling time) de 0.5ms
    delayMicroseconds(500);
    
    // 3. Activar interrupció i demanar conversió (trigarà 1ms)
    attachInterrupt(digitalPinToInterrupt(PIN_ADC_MISO_DRDY), nirsNotifyDataReady, FALLING);
    adcStartSingleConversion();
    
    // 4. Esperar que el semàfor ens avisi (màxim 5ms per seguretat)
    if (xSemaphoreTake(adcSemaphore, pdMS_TO_TICKS(5))) {
        uint32_t val = adcReadData24();
        applyOpticalState(OpticalState::DARK_RED); // Apaguem LED immediatament
        return val;
    }
    
    return 0; // Error de timeout
}

void nirsRunCycle() {
    uint32_t startTime = millis();

    // Executem les 4 fases (2ms cadascuna nominalment)
    currentFrame.red     = captureSample(OpticalState::RED);
    delay(1); // Petit marge per completar els 2ms de la fase
    
    currentFrame.darkRed = captureSample(OpticalState::DARK_RED);
    delay(1);

    currentFrame.ir      = captureSample(OpticalState::IR);
    delay(1);

    currentFrame.darkIr  = captureSample(OpticalState::DARK_IR);
    delay(1);

    currentFrame.timestampMs = millis();

    // El cicle total ha de durar 20ms. 
    // Com que hem gastat uns 8ms en les fases, esperem la resta.
    uint32_t elapsed = millis() - startTime;
    if (elapsed < TOTAL_CYCLE_MS) {
        delay(TOTAL_CYCLE_MS - elapsed);
    }
}