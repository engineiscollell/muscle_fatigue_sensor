#ifndef NIRS_LOGIC_H
#define NIRS_LOGIC_H

#include <stdint.h>

// Estructura de dades per a un frame complet d'adquisició
struct OpticalRawFrame {
    uint32_t red;
    uint32_t darkRed;
    uint32_t ir;
    uint32_t darkIr;
};

// Funció per calcular la mètrica SmO2 a partir del frame
float calculateNIRS(const OpticalRawFrame& frame);

#endif