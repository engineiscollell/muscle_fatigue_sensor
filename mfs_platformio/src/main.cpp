  #include <Arduino.h>
  #include <SPI.h>

  #define LED1_PIN = 
  #define DRDY_PIN = 19;
  hw_timer_t * timerLed1 = NULL;
  hw_timer_t * timerLed2 = NULL;
  volatile bool dataReadyFlag = false;
  volatile bool led1 = false;
  volatile bool led2 = false;

  void IRAM_ATTR isr_drdy() {
    dataReadyFlag = true;
  }
  void IRAM_ATTR onLed1() {
    led1 = true;
  }
  void IRAM_ATTR onLed2() {
    led2 = true;
  }

  void setup() {
    Serial.begin(115200);

    pinMode(DRDY_PIN, INPUT);
    pinMode(LED1_PIN, OUTPUT);
    pinMode(LED2_PIN, OUTPUT);

    // Configurar interrupció per flanc de baixada
    attachInterrupt(digitalPinToInterrupt(DRDY_PIN), isr_drdy, FALLING); //he posat rising ja que vull interpretar quan comença el puls
    Serial.println("Sistema preparat...");

    // Timer 1
    timerLed1 = timerBegin(0, xx, true);
    timerAttachInterrupt(timer1, &onLed1, true);
    timerAlarmWrite(timer1, yyyyy, true);
    timerAlarmEnable(timer1);

    // Timer 2
    timerLed2 = timerBegin(1, xx, true);
    timerAttachInterrupt(timer2, &onLed2, true);
    timerAlarmWrite(timer2, yyyyy, true);
    timerAlarmEnable(timer2); 
  }

  void loop() {
    if (dataReadyFlag) {
      dataReadyFlag = false;
      detachInterrupt(DRDY_PIN);
      // Lectura 24 bits d'informació
      // Serial.println("Interrupcio detectada!");
      attachInterrupt(digitalPinToInterrupt(DRDY_PIN), isr_drdy, FALLING);
    }

    if (led1) {
      led1 = false;
      //Configurar timer per recomençar la conta enere.

    }
    
    if (led2) {
    }

    delay(10); // petita pausa
  }
  