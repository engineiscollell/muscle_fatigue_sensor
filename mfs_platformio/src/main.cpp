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
  