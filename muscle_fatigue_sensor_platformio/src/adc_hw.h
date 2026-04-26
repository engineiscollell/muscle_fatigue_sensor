#ifndef AD7172_HW_H
#define AD7172_HW_H

#include <stdint.h>

// Registres AD7172
constexpr uint8_t AD7172_REG_ADCMODE  = 0x01;
constexpr uint8_t AD7172_REG_IFMODE   = 0x02;
constexpr uint8_t AD7172_REG_DATA     = 0x04;
constexpr uint8_t AD7172_REG_FILTCON0 = 0x28;

void ad7172_init(uint8_t csPin);
void ad7172_start_single_conversion(); // Inicia 1ms de mesura
uint32_t ad7172_read_data24();         // Llegeix i puja CS a HIGH

#endif