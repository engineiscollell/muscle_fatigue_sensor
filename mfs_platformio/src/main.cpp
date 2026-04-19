  #include <Arduino.h>
  #include <SPI.h>

  #define LEDIR_PIN = 25;
  #define LEDIR_PIN = 26;
  #define DRDY_PIN = 19;
  hw_timer_t * timerLedIR = NULL;
  hw_timer_t * timerLedR = NULL;
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
   
//DEFINICIÓ TASQUES
/*
void adcInit();
Configurar AD7172-2
ODR
Filtre
Referència
Canal
Mode de lectura

void bleInit();
Servei BLE
Caracteristica
Advertising

void processingInit();
Inicialitzar filtres
Variables de baseline
Estat del processament

void createRTOSObjects();
crear cua
crear mutex

void createTasks();
crear tasques
Fixar tasques a un core concret
*/

  void setup() {
    Serial.begin(115200);

    pinMode(DRDY_PIN, INPUT);
    pinMode(LEDIR_PIN, OUTPUT);
    pinMode(LEDR_PIN, OUTPUT);

    // Configurar interrupció per flanc de baixada
    attachInterrupt(digitalPinToInterrupt(DRDY_PIN), isr_drdy, FALLING); //he posat rising ja que vull interpretar quan comença el puls
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
      detachInterrupt(DRDY_PIN);
      // Lectura 24 bits d'informació
      // Serial.println("Interrupcio detectada!");
      attachInterrupt(digitalPinToInterrupt(DRDY_PIN), isr_drdy, FALLING);
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
  