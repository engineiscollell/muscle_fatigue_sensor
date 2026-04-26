#include "nirs_logic.h"
#include "config.h"
#include <math.h>

// nirs_logic.cpp (Inici)
// Passem de 'det' a 'inv_det' precalculat
static const float inv_det = 1.0f / ((EPS_O2_RED * EPS_HB_IR) - (EPS_HB_RED * EPS_O2_IR));

float calculateNIRS(const OpticalRawFrame& frame) {
    // 1. Intensitat neta (Cancel·lació de llum ambient)
    float iRed = (float)frame.red - (float)frame.darkRed;
    float iIR  = (float)frame.ir - (float)frame.darkIr;

    // Seguretat: evitem logaritmes de números negatius o zero
    if (iRed < MIN_SIGNAL_INTENSITY || iIR < MIN_SIGNAL_INTENSITY) return 0.0f;

    // 2. Càlcul de l'Absorbància (A = -log10(I / I0))
    // Estem usant ADC_REFERENCE_I0 com a estimació de la potència del LED
    float absRed = -log10f(iRed / ADC_REFERENCE_I0);
    float absIR  = -log10f(iIR / ADC_REFERENCE_I0);

    // 3. Resolució del sistema MBLL per trobar concentracions relatives
    // El denominador és el determinant de la matriu de coeficients d'extinció
    //float det = (EPS_O2_RED * EPS_HB_IR) - (EPS_HB_RED * EPS_O2_IR); PRECALCULAT (IGUAL PER TOTS ELS CICLES) + multiplicació 3 a 5 vegades més ràpid que divisió.
    
    // Concentracions relatives d'Hemoglobina Oxigenada i Desoxigenada
    float relHbO2 = (absRed * EPS_HB_IR - absIR * EPS_HB_RED) * inv_det;
    float relHb   = (absIR * EPS_O2_RED - absRed * EPS_O2_IR) * inv_det;

    // Si la lectura produeix concentracions negatives, el frame és invàlid per soroll
    if (relHbO2 < 0.0f || relHb < 0.0f) {
        return 0.0f; // Podries retornar -1.0f per detectar "error", però 0 ho deixem com estàs fent
    }

    // 4. Càlcul del percentatge SmO2
    float totalHb = relHbO2 + relHb;

    // Ara totalHb mai pot ser < 0
    if (totalHb > 0.00001f) {
        float smo2 = (relHbO2 / totalHb) * 100.0f;
        
        // Com que hem evitat negatius a dalt, smo2 SEMPRE estarà entre 0.0 i 100.0 de manera natural.
        // Tot i així, mantenim el clamping de seguretat per errors d'arrodoniment de coma flotant.
        if (smo2 > 100.0f) return 100.0f;
        
        return smo2;
    }

    return 0.0f;
}