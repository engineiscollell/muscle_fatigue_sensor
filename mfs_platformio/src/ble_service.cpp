#include "ble_service.h"

NimBLEServer* pServer = nullptr;
NimBLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected = false;

class MyServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        deviceConnected = true;
        Serial.println("Client connectat!");
    };
    void onDisconnect(NimBLEServer* pServer) {
        deviceConnected = false;
        Serial.println("Client desconnectat. Reiniciant advertising...");
        NimBLEDevice::startAdvertising(); 
    }
};

void bleInit() {
    NimBLEDevice::init("NIRS_ESP32_Sensor");

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    NimBLEService* pService = pServer->createService(SERVICE_UUID);

    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    
    // Línia eliminada perquè NimBLE 2.5.0 ho fa automàticament.
    
    pAdvertising->start();
    Serial.println("BLE actiu i emetent senyal...");
    Serial.print("ATENCIÓ! La meva adreça MAC és: ");
    Serial.println(NimBLEDevice::getAddress().toString().c_str());
}

void bleSendData(uint16_t smo2_value) {
    if (deviceConnected && pCharacteristic != nullptr) {
        pCharacteristic->setValue((uint8_t*)&smo2_value, 2);
        pCharacteristic->notify();
    }
}