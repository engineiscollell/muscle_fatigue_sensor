#include "data_processor.h"
#include <math.h>

uint16_t processNirsData(const OpticalRawFrame& frame) {
    // 1. Resta de la llum ambiental (Foscos)
    // Assegurem que el resultat no sigui negatiu per soroll
    float redNet = (frame.red > frame.darkRed) ? (float)(frame.red - frame.darkRed) : 1.0f;
    float irNet  = (frame.ir > frame.darkIr) ? (float)(frame.ir - frame.darkIr) : 1.0f;

    // 2. Càlcul de la Ràtio de Ràtios (R)
    // Utilitzem logaritmes per aproximar l'absorbància (Llei de Beer-Lambert)
    // R = log(Red_Net) / log(IR_Net)
    float ratio = log10(redNet) / log10(irNet);

    // 3. Càlcul de la SmO2 (%)
    float smo2 = CALIB_A - (CALIB_B * ratio);

    // 4. Limitar el resultat entre 0% i 100%
    if (smo2 > 100.0f) smo2 = 100.0f;
    if (smo2 < 0.0f)   smo2 = 0.0f;

    // 5. Retornar com a uint16_t escalat per 100 (per enviar per BLE)
    // Exemple: 75.52% -> 7552
    return (uint16_t)(smo2 * 100);
}