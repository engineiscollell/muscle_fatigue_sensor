#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#include <Arduino.h>

// Adreces de Registres AD7172-2
#define REG_ADCMODE   0x01
#define REG_IFMODE    0x02
#define REG_DATA      0x04
#define REG_ID        0x07
#define REG_FILTCON0  0x28

// Comandes de comunicació
#define CMD_READ      0x40  // Bit 6 a 1 per llegir
#define CMD_WRITE     0x00  // Bit 6 a 0 per escriure

// Funcions del mòdul
void adcInit();
void adcStartSingleConversion();
uint32_t adcReadData24();

// Funcions de suport (tal com surten al PDF 9)
void adcWriteRegister(uint8_t reg, uint32_t value, uint8_t nBytes);
uint32_t adcReadRegister(uint8_t reg, uint8_t nBytes);

#endif