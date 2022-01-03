#ifndef defines_h
#define defines_h
  #define SKETCH_NAME "PMS5003 DEMO"
  #define SW_VERSION "v0.2"
  #define SKETCH_AUTHOR "Egor Kartashov @wildegor"
  #define SERIAL_PRINTING true // Макрос, поставьте false чтобы убрать все выводу в консоль
  #define COUNT_MEASURE 5 // Количество измерений в цикле для фильтрации
  #define MEASURE_INTERVAL 1000 // Интервал между измерениями
  #define MEASURE_DELAY 5000 // Интервал записи
  #define PMS_DELAY 30000 // Согласно документации, после ухода в режим сна и обратно датчику нужно время "проснуться"
  #define PMS5003_SERIAL_BAUD 9600 // Скорость software serial между ESP и датчиком
  #define ESP_SERIAL_BAUD 230400 // Скорость порта между ПК и ESP
  // Разные цвета для AQI уровня
  #define AQI_YELLOW            "#ED9D00"
  #define AQI_RED               "#D3435C"
  #define AQI_REALLY_RED        "#F90000"
  #define AQI_PLEASANT_GREEN    "#009140"
  #define AQI_ORANGE            "#ffa500"
  #define AQI_PURPLE            "#800080"
  #define AQI_MAROON            "#800000"
  // Сообщения AQI уровня
  #define AQI_GOOD                "Good"
  #define AQI_MOD                 "Moderate"
  #define AQI_UNHG                "Unhealthy for Sensitive Groups"
  #define AQI_UNH                 "Unhealthy"
  #define AQI_VUNH                "Very Unhealthy"
  #define AQI_HAZ                 "Hazardous"
  // Определение подключения датчика, можно убрать лишнее
  #if defined(ARDUINO_AVR_MEGA2560) || defined(ARDUINO_AVR_LEONARDO) || defined(ARDUINO_SAM_DUE)
      #define pmsSerial           Serial1 // Используем второй порт 
  #elif defined(ARDUINO_ARCH_AVR)
      #define PMS5003_TX_PIN        4
      #define PMS5003_RX_PIN        5
      #include <SoftwareSerial.h>          // Используем программный порт
      SoftwareSerial pmsSerial(PMS5003_RX_PIN, PMS5003_TX_PIN);
  #elif defined(ARDUINO_ARCH_ESP8266) // <---------- Вроде бы так, согласно схеме
      #define PMS5003_TX_PIN        D6
      #define PMS5003_RX_PIN        D7
      #include <SoftwareSerial.h>          // Используем программный порт
      SoftwareSerial pmsSerial(PMS5003_RX_PIN, PMS5003_TX_PIN);
      #define LED_BUILTIN_HIGH      LOW
      #define LED_BUILTIN_LOW       HIGH
  #elif defined(ARDUINO_ARCH_ESP32)
      #define PMS5003_TX_PIN        18
      #define PMS5003_RX_PIN        19
      #include <SoftwareSerial.h>          // Используем программный порт
      SoftwareSerial pmsSerial(PMS5003_RX_PIN, PMS5003_TX_PIN);
      #ifdef LED_BUILTIN
        #undef LED_BUILTIN      
      #endif 
      #define LED_BUILTIN           13  // NOT DEFINED IN ESP32 BOARD FILES - HMMM.
  #else
      #error "May work, but not tested on this target"
      #define LED_BUILTIN_HIGH      HIGH
      #define LED_BUILTIN_LOW       LOW
  #endif
  #if SERIAL_PRINTING
    #define SERIAL_DEBUG  (Serial.begin(ESP_SERIAL_BAUD))
    #define Sprintln(a) (Serial.println(a))
    #define Sprint(a) (Serial.print(a))
    #define Sprintf(...) (Serial.printf(__VA_ARGS__))
    #define BLYNK_PRINT Serial
  #else
    #define SERIAL_DEBUG
    #define Sprintln(a) 
    #define Sprint(a) 
    #define Sprintf(...) 
  #endif
#endif 
