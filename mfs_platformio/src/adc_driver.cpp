#include "adc_driver.h"
#include "config.h"
#include <SPI.h>

// Configuració SPI: 1MHz, el xip AD7172 funciona en SPI MODE 3 (CPOL=1, CPHA=1)
SPISettings adcSettings(1000000, MSBFIRST, SPI_MODE3);

void adcInit() {
    pinMode(PIN_ADC_CS, OUTPUT);
    digitalWrite(PIN_ADC_CS, HIGH);

    // Inicialitzem el bus SPI amb els teus pins de config.h
    SPI.begin(PIN_ADC_SCK, PIN_ADC_MISO_DRDY, PIN_ADC_MOSI, PIN_ADC_CS);

    // 1. Configurem la velocitat a 1000 SPS (Valor buscat: 0x000B)
    adcWriteRegister(REG_FILTCON0, ADC_SPS_1000, 2);
    
    // 2. Configurem el mode d'interfície (Bàsic, sense dades extres)
    adcWriteRegister(REG_IFMODE, 0x0000, 2);
}

void adcStartSingleConversion() {
    // Escrivim al registre ADCMODE el valor 0x8010:
    // - Activa referència interna
    // - Posa el xip en mode "Single Conversion"
    // En fer això, el xip fa UNA mesura i es queda esperant.
    adcWriteRegister(REG_ADCMODE, 0x8010, 2); 
    
    // DECISIÓ CRÍTICA: Després de demanar la conversió, BAIXEM el CS.
    // Com que el pin MISO és compartit, el xip només posarà el pin a LOW (DRDY)
    // si el CS està activat.
    digitalWrite(PIN_ADC_CS, LOW);
}

uint32_t adcReadData24() {
    // Llegim els 24 bits del registre de dades (0x04)
    uint32_t value = 0;
    
    SPI.beginTransaction(adcSettings);
    // El CS ja està LOW des de adcStartSingleConversion, però ho assegurem
    digitalWrite(PIN_ADC_CS, LOW); 
    
    SPI.transfer(CMD_READ | REG_DATA);
    for (uint8_t i = 0; i < 3; i++) {
        value <<= 8;
        value |= SPI.transfer(0x00);
    }
    
    // Un cop llegit, pugem el CS per alliberar el pin i finalitzar el cicle
    digitalWrite(PIN_ADC_CS, HIGH);
    SPI.endTransaction();
    
    return value;
}

// Implementació genèrica de l'activitat 1 del PDF 9
void adcWriteRegister(uint8_t reg, uint32_t value, uint8_t nBytes) {
    SPI.beginTransaction(adcSettings);
    digitalWrite(PIN_ADC_CS, LOW);
    
    SPI.transfer(CMD_WRITE | reg);
    for (int i = nBytes - 1; i >= 0; i--) {
        SPI.transfer((value >> (i * 8)) & 0xFF);
    }
    
    digitalWrite(PIN_ADC_CS, HIGH);
    SPI.endTransaction();
}

uint32_t adcReadRegister(uint8_t reg, uint8_t nBytes) {
    uint32_t value = 0;
    SPI.beginTransaction(adcSettings);
    digitalWrite(PIN_ADC_CS, LOW);
    
    SPI.transfer(CMD_READ | reg);
    for (uint8_t i = 0; i < nBytes; i++) {
        value <<= 8;
        value |= SPI.transfer(0x00);
    }
    
    digitalWrite(PIN_ADC_CS, HIGH);
    SPI.endTransaction();
    return value;
}