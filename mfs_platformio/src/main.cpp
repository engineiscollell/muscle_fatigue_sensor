#include <Arduino.h>
#include "config.h"
#include "nirs_logic.h"
#include "ble_service.h"
#include "data_processor.h"

// Cua per passar dades del Core 1 (Acquisició) al Core 0 (Comunicació)
QueueHandle_t dataQueue;

// --- TASCA DE L'ADQUISICIÓ (CORE 1) ---
// Aquesta tasca té la màxima prioritat per no perdre el sincronisme dels 20ms
void taskAcquisition(void *pvParameters) {
    Serial.println("Task Acquisition started on Core 1");
    
    for (;;) {
        // Executa el cicle de 4 fases (RED, DARK, IR, DARK)
        // Aquesta funció ja gestiona els DACs i la interrupció del pin 19
        nirsRunCycle(); 
        
        // Enviem el frame capturat a la cua perquè el Core 0 el processi
        // No esperem (0), si la cua està plena, perdem la mostra per no bloquejar el timing
        extern OpticalRawFrame currentFrame; // Ve de nirs_logic.cpp
        xQueueSend(dataQueue, &currentFrame, 0);
    }
}

// --- TASCA DE COMUNICACIÓ I CÀLCUL (CORE 0) ---
void taskCommunication(void *pvParameters) {
    Serial.println("Task Communication started on Core 0");
    OpticalRawFrame frame;
    
    // Inicialitzem el servei de Bluetooth NimBLE
    bleInit();

    for (;;) {
        // Esperem que arribin dades de la cua
        if (xQueueReceive(dataQueue, &frame, portMAX_DELAY)) {
            
            // 1. Aquí aniria el càlcul de la SmO2 o processament de dades
            // Per ara, simulem un valor per enviar per BLE (exemple: 75.50%)
            // Recorda que hem quedat que enviem uint16_t escalat x100
            uint16_t smo2_to_send = 7550; 

            // 2. Enviem la notificació per Bluetooth
            bleSendData(smo2_to_send);
            
            // Opcional: Debug pels Serial
            // Serial.printf("R: %d | IR: %d\n", frame.red, frame.ir);
        }
    }
}

void setup() {
    Serial.begin(115200);
    
    // 1. Inicialitzem el hardware (DACs, SPI, ADC)
    nirsInit();
    
    // 2. Creem la cua (capacitat per 10 frames de reserva)
    dataQueue = xQueueCreate(10, sizeof(OpticalRawFrame));
    
    if (dataQueue != NULL) {
        // 3. Creem les tasques i les assignem a nuclis diferents
        
        // Tasca d'adquisició al Core 1 (Prioritat alta: 10)
        xTaskCreatePinnedToCore(
            taskAcquisition,   // Funció
            "Acquisition",     // Nom
            4096,              // Stack size
            NULL,              // Paràmetres
            10,                // Prioritat
            NULL,              // Handle
            1                  // Nucli 1
        );

        // Tasca de BLE al Core 0 (Prioritat mitjana: 5)
        xTaskCreatePinnedToCore(
            taskCommunication, 
            "Communication",   
            4096,              
            NULL,              
            5,                 
            NULL,              
            0                  // Nucli 0
        );
    }
}

void loop() {
    // El loop principal es queda buit ja que fem servir FreeRTOS (tasques)
    vTaskDelete(NULL); 
}