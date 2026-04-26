#include <Arduino.h>
#include "ble_service.h"
#include "config.h"
#include <NimBLEDevice.h>

static NimBLECharacteristic* g_pCharacteristic = nullptr;
static bool g_deviceConnected = false;

class ServerCallbacks : public NimBLEServerCallbacks {
public:
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
        (void)pServer;
        (void)connInfo;
        g_deviceConnected = true;
    }

    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
        (void)pServer;
        (void)connInfo;
        (void)reason;
        g_deviceConnected = false;

        // Reprendre advertising manualment
        NimBLEDevice::startAdvertising();
    }
};

void bleInit() {
    Serial.println("BLE init start");
    NimBLEDevice::init("ESP32_NIRS");

    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    // Fem servir SERVICE_UUID definit a config.h
    NimBLEService* pService = pServer->createService(SERVICE_UUID);

    // Fem servir CHARACTERISTIC_UUID definit a config.h
    g_pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );

    uint16_t initValue = 0;
    g_pCharacteristic->setValue((uint8_t*)&initValue, sizeof(initValue));

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setName("ESP32_NIRS");
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->enableScanResponse(true);

    bool ok = pAdvertising->start();
    Serial.print("Advertising start result: ");
    Serial.println(ok ? "OK" : "FAIL");
    Serial.println("BLE init end");
}

bool bleIsConnected() {
    return g_deviceConnected;
}

void bleNotifySmO2(uint16_t smo2_x100) {
    if (!g_deviceConnected || g_pCharacteristic == nullptr) {
        return;
    }

    g_pCharacteristic->setValue((uint8_t*)&smo2_x100, sizeof(smo2_x100));
    g_pCharacteristic->notify();
}