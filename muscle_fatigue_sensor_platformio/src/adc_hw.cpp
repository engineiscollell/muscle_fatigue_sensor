#include "adc_hw.h"
#include <Arduino.h>
#include <SPI.h>

static uint8_t _csPin;
static SPISettings _spiSettings(2000000, MSBFIRST, SPI_MODE3); 

void ad7172_init(uint8_t csPin) {
    _csPin = csPin;
    pinMode(_csPin, OUTPUT);
    digitalWrite(_csPin, HIGH);

    SPI.beginTransaction(_spiSettings);
    digitalWrite(_csPin, LOW);

    SPI.transfer(AD7172_REG_IFMODE); 
    SPI.transfer16(0x0000);

    SPI.transfer(AD7172_REG_FILTCON0);
    SPI.transfer16(0x000B);

    digitalWrite(_csPin, HIGH);
    SPI.endTransaction();
}

void ad7172_start_single_conversion() {
    SPI.beginTransaction(_spiSettings);
    digitalWrite(_csPin, LOW); 

    SPI.transfer(AD7172_REG_ADCMODE);
    SPI.transfer16(0x8010); // Ref interna ON + Single Conversion

    digitalWrite(_csPin, HIGH); 
    SPI.endTransaction(); 
}

uint32_t ad7172_read_data24() {
    uint32_t data = 0;
    
    SPI.beginTransaction(_spiSettings);
    digitalWrite(_csPin, LOW); // <-- NOU: Tornem a agafar el xip per llegir
    
    // Assumim que ja hi ha una comanda de lectura a REG_DATA (0x44)
    SPI.transfer(0x40 | AD7172_REG_DATA); // Comanda de lectura al registre de dades
    
    data |= (SPI.transfer(0x00) << 16);
    data |= (SPI.transfer(0x00) << 8);
    data |= SPI.transfer(0x00);
    
    digitalWrite(_csPin, HIGH); // Tornem a pujar el CS
    SPI.endTransaction();
    
    return data;
}