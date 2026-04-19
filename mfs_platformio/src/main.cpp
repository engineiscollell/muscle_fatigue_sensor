#include <Arduino.h>
#include <SPI.h>

#define LEDIR = 25;
#define LEDIR = 26;
#define DRDY = 19;
#define MISO = 19;
#define MOSI = 23;
#define SCLK = 18;
#define CS = 5;
  
//*
struct OpticalFrame {
  uint32_t timestamp_ms;

  int32_t red_raw;
  int32_t dark_red_raw;
  int32_t ir_raw;
  int32_t dark_ir_raw;

  bool valid;
};

struct SmO2Result {
  uint32_t timestamp_ms;
  float smO2;
  bool valid;
};

//*
QueueHandle_t g_frameQueue = nullptr;
SemaphoreHandle_t g_resultMutex = nullptr;

SmO2Result g_latestResult = {0, 0.0f, false};

//*
TaskHandle_t g_taskAcq = nullptr;
TaskHandle_t g_taskProc = nullptr;
TaskHandle_t g_taskBLE = nullptr;

//Function prototypes*
//Low level ADC*
//Result Mutex

  
hw_timer_t * timerLedIR = NULL;
hw_timer_t * timerLedR = NULL;
void taskADC(void *pv);
void taskBLE(void *pv);

volatile bool dataReadyFlag = false;
volatile bool ledir = false;
volatile bool led4 = false;

void IRAM_ATTR isr_drdy() {
  dataReadyFlag = true;
}
void IRAM_ATTR onLedIR() {
  led1 = true;
}
void IRAM_ATTR onLedR() {
  led2 = true;
}
  
//DEFINICIÓ TASQUES
/*
// Declarem el task handle
TaskHandle_t BlinkTaskHandle = NULL;
void BlinkTask(void *parameter) {
  for (;;) { // llaç infinit!!!
    digitalWrite(LED_PIN, HIGH);
    Serial.println("BlinkTask: LED ON");
    vTaskDelay(1000 / portTICK_PERIOD_MS); // 1000ms
    digitalWrite(LED_PIN, LOW);
    Serial.println("BlinkTask: LED OFF");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    Serial.print(“Encén LED executant-se al core ");
    Serial.println(xPortGetCoreID());
  }
}
  
//Task1code: blinks an LED every 1000 ms
void Task1code( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  for(;;){
    digitalWrite(led1, HIGH);
    delay(1000);
    digitalWrite(led1, LOW);
    delay(1000);
    }
}

//Task2code: blinks an LED every 700 ms
void Task2code( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());
  for(;;){
    digitalWrite(led2, HIGH);
    delay(700);
    digitalWrite(led2, LOW);
    delay(700);
  }
}
*/
   
//DEFINICIÓ INITS
void systemInit(){
  System.begin(115200); //Arrancada general
  delay(1000);
  //No faria falta també posar-hi Logs, Config bàsica? Arrancada general ja estaria complerta?
  Serial.printf("CPU: %d MHz\n", getCpuFrequencyMhz());
  Serial.printf("Free Heap: %u bytes\n", ESP.getFreeHeap());
}

void hardwareInit(){
  pinMode(LEDIR, OUTPUT);
  pinMode(LEDR, OUTPUT);

  digitalWrite(LEDIR, LOW);
  digitalWrite(LEDR, LOW);

  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH); //*

  pinMode(DRDY, INPUT);
  
  //Falta configurar MOSI, CS, SCLK *spi.begin();?
  spi.begin(SCLK, CS, MOSI, CS);

  //Pins de sincronisme si cal?
  //Timers!
}

void adcInit(){
  //ad7172_reset();?

  // CONFIGURACIÓ SIMULADA
  adcWriteRegister(0x01, 0x0000); // MODE
  adcWriteRegister(0x02, 0x0005); // FILTER
  adcWriteRegister(0x03, 0x1000); // SETUP

  ad7172_writeRegister(REG_ADC_MODE, 0x0000);
  ad7172_writeRegister(REG_IFMODE,   0x0000);
  ad7172_writeRegister(REG_SETUP0,   0x1000);
  ad7172_writeRegister(REG_FILTER0,  0x0005);
  ad7172_writeRegister(REG_CH0,      0x8001);
  // ODR, Filtre, Referència, Canal, Mode de lectura
}

void bleInit(){
  //Servei BLE; caracter´sitica, advertising
}

void processingInit(){
  // Inicialitzar filtres, Variables de baseline, Estat del processament
  g_latestResult.valid = false;

}

void createRTOSObjects(){
  g_frameQueue = xQueueCreate(10, sizeof(OpticalFrame));
  g_resultMutex = xSemaphoreCreateMutex();
}

void createTasks(){
  xTaskCreatePinnedToCore(
    taskAcquisition,
    "TaskAcquisition",
    4096,
    NULL,
    3,
    &g_taskAcq,
    1
  );

  xTaskCreatePinnedToCore(
    taskProcessing,
    "TaskProcessing",
    4096,
    NULL,
    2,
    &g_taskProc,
    0
  );

  xTaskCreatePinnedToCore(
    taskBLE,
    "TaskBLE",
    4096,
    NULL,
    1,
    &g_taskBLE,
    0
  );
}

void taskAcquisition(void *pv) {

  OpticalFrame frame;

  while (true) {

    // RED ON
    digitalWrite(LED_RED_PIN, HIGH);
    digitalWrite(LED_IR_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(2));
    frame.red_raw = adcRead24();

    // DARK after RED
    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(LED_IR_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(2));
    frame.dark_red_raw = adcRead24();

    // IR ON
    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(LED_IR_PIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(2));
    frame.ir_raw = adcRead24();

    // DARK after IR
    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(LED_IR_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(2));
    frame.dark_ir_raw = adcRead24();

    frame.timestamp_ms = millis();
    frame.valid = true;

    xQueueSend(g_frameQueue, &frame, 0);

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void taskProcessing(void *pv) {

  OpticalFrame frame;

  while (true) {

    if (xQueueReceive(g_frameQueue, &frame, portMAX_DELAY) == pdTRUE) {

      float red = frame.red_raw - frame.dark_red_raw;
      float ir  = frame.ir_raw  - frame.dark_ir_raw;

      if (red < 1) red = 1;
      if (ir < 1)  ir = 1;

      // MODEL SIMPLIFICAT
      float smO2 = (ir / (ir + red)) * 100.0f;

      updateLatestResult(smO2);

      Serial.printf(
        "[PROC Core %d] RED=%.1f IR=%.1f SmO2=%.2f %%\n",
        xPortGetCoreID(),
        red,
        ir,
        smO2
      );
    }
  }
}

void taskBLE(void *pv) {

  SmO2Result result;

  while (true) {

    if (getLatestResult(result)) {

      Serial.printf(
        "[BLE Core %d] Send SmO2 = %.2f %% at %lu ms\n",
        xPortGetCoreID(),
        result.smO2,
        result.timestamp_ms
      );
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

  void setup() {
    //Serial.begin(115200); HO HEM POSAT A SYSTEMINIT!
    /*
    pinMode(DRDY, INPUT);
    pinMode(LEDIR, OUTPUT);
    pinMode(LEDR, OUTPUT);
    */
    // Configurar interrupció per flanc de baixada
    attachInterrupt(digitalPinToInterrupt(DRDY), isr_drdy, FALLING); //he posat rising ja que vull interpretar quan comença el puls
    Serial.println("Sistema preparat...");

    // Timer 1
    timerLedIR = timerBegin(0, xx, true);
    timerAttachInterrupt(timerIR, &onLedIR, true);
    timerAlarmWrite(timerIR, yyyyy, true);
    timerAlarmEnable(timerIR);

    // Timer 2
    timerLedR = timerBegin(1, xx, true);
    timerAttachInterrupt(timerR, &onLedR, true);
    timerAlarmWrite(timerR, yyyyy, true);
    timerAlarmEnable(timerR); 

    //TASQUES 
    /*
    xTaskCreatePinnedToCore(
      BlinkTask, // Nom funció de la tasca
      “Encén LED", // Nom tasca (el que vulgueu)
      10000, // Stack size (x4 bytes)
      NULL, // Parametres
      1, // Prioritat
      &BlinkTaskHandle, // Task handle
      1 // Core al que s’executa
    );

    */

    //INICIALITZACIONS
    systemInit();
    hardwareInit();
    adcInit();
    bleInit();
    processingInit();
    createRTOSObjects();
    createTasks();
  }

  void loop() {
    if (dataReadyFlag) {
      dataReadyFlag = false;
      detachInterrupt(DRDY);
      // Lectura 24 bits d'informació
      // Serial.println("Interrupcio detectada!");
      attachInterrupt(digitalPinToInterrupt(DRDY), isr_drdy, FALLING);
    }

    if (ledIR) {
      ledIR = false;
      //Configurar timer per recomençar la conta enere.

    }
    
    if (ledR) {
      ledIR = false;
      //Configurar timer per recomençar la conta enere.
    }

    delay(10); // petita pausa
  }
  