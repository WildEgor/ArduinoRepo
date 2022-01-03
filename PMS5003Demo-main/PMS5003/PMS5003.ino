#include "Adafruit_PM25AQI.h" // Проверенная библиотека для всех PMS* датчиков
#include "ReactESP.h" // Опционально, но упрощает работу с прерываниями, частотой опроса и пр
#include <MedianFilter.h> // Готовый фильтр 
#include "define.h" // Конфигурационный файл (обязательно посмотреть перед запуском)

uint8_t counterMeasure = 0; // Счетчик для измерений
bool pmsSleep = false; // Флаг для сна датчика

struct AQI { // Структура для AQI значений
   uint16_t pm25AQI;
   uint16_t pm10AQI;
   String mesAQI;
   String mesAQIColor;
};

Adafruit_PM25AQI aqi = Adafruit_PM25AQI(); // Объект PMS датчика
AQI myAQI; // Инициализация структура 
PM25_AQI_Data pmsData; // Структура данных

// Фильтрация
MedianFilter pm25envMF(COUNT_MEASURE, 0);
MedianFilter pm10envMF(COUNT_MEASURE, 0);
MedianFilter pm100envMF(COUNT_MEASURE, 0);
MedianFilter pm25stdMF(COUNT_MEASURE, 0);
MedianFilter pm10stdMF(COUNT_MEASURE, 0);
MedianFilter pm100stdMF(COUNT_MEASURE, 0);

// Функция используется для расчета показателя AQI, возможно это избыточно даже
void AQICalc(float pm25value, float pm10value) {
      const uint16_t pm25C_low[] = {0, 13, 36, 56, 151, 251, 351};
      const uint16_t pm25C_high[] = {12, 35, 55, 150, 250, 350, 500};
      const uint16_t pm10C_low[] = {0, 55, 155, 255, 355, 425, 505};
      const uint16_t pm10C_high[] = {54, 154, 254, 354, 424, 504, 604};
      const uint16_t I_low[] = {0, 51, 101, 151, 201, 301, 401};
      const uint16_t I_high[] = {50, 100, 150, 200, 300, 400, 500};
      const String AQIMESENG[] = {AQI_GOOD, AQI_MOD, AQI_UNHG, AQI_UNH, AQI_VUNH, AQI_HAZ};
      const String AQIMESCLR[] = {AQI_PLEASANT_GREEN, AQI_YELLOW, AQI_ORANGE, AQI_REALLY_RED, AQI_PURPLE, AQI_MAROON};
      const uint8_t AQIMESarraySize = *(&AQIMESENG + 1) - AQIMESENG;
      const uint8_t pm25C_lowarraySize = *(&pm25C_low + 1) - pm25C_low;
      uint8_t indexPM10 = 255;
      uint8_t indexPM25 = 255;
      // Не изящно, но работает
      for (int i = 0; i < pm25C_lowarraySize; i++) {
        if ((pm25value <= pm25C_high[i]) && indexPM25 == 255){
          indexPM25 = i;
          myAQI.mesAQI = AQIMESENG[i];
          myAQI.mesAQIColor = AQIMESCLR[i];
          myAQI.pm25AQI = (I_high[i] - I_low[i]) * (pm25value - pm25C_low[i]) / (pm25C_high[i] - pm25C_low[i]) + I_low[i];
        }
        if ((pm10value <= pm10C_high[i]) && indexPM10 == 255){
          indexPM10 = i;
          myAQI.pm10AQI = (I_high[i] - I_low[i]) * (pm10value - pm10C_low[i]) / (pm10C_high[i] - pm10C_low[i]) + I_low[i];
        }
      }
}

// Каждую Nсек опрашиваем датчик с задержкой Tсек и уходим в "сон"
void onRepeatHandlerPMS5003(ReactESP &app, SoftwareSerial &pmsSerial){
    app.onRepeat(MEASURE_INTERVAL, [&app, &pmsSerial]() {
      if (aqi.read(&pmsData) && (counterMeasure < COUNT_MEASURE) && !pmsSleep) // Если будут проблемы с датчиком (нет ответа) можно добавить таймер задержки и изменять counterMeasure чтобы не зависнуть...
          {
            //do {
              Sprintln("Try to read data... ");
              Sprint(counterMeasure);
              pm10stdMF.in(pmsData.pm10_standard);
              pm25stdMF.in(pmsData.pm25_standard);
              pm100stdMF.in(pmsData.pm100_standard);
              pm10envMF.in(pmsData.pm10_env);
              pm25envMF.in(pmsData.pm25_env);
              pm100envMF.in(pmsData.pm100_env);
              counterMeasure++;
            //} while(counterMeasure < COUNT_MEASURE);
          }
      app.onDelay(MEASURE_DELAY, [&app, &pmsSerial]() {
        if (!pmsSleep) {          
          if (counterMeasure >= COUNT_MEASURE){
            AQICalc(pm25stdMF.out(), pm10stdMF.out());
            Sprintln("\r\n======PMS5003-INFO======");
            Sprintln("Concentration Units (standard, RAW | FILTERED)");
            Sprintln("PM 1.0: "); Sprint(pmsData.pm10_standard);
            Sprint("\t\tPM 2.5: "); Sprint(pmsData.pm25_standard);
            Sprint("\t\tPM 10: "); Sprintln(pmsData.pm100_standard);
            Sprintln("---------------------------------------");
            Sprintln("PM 1.0: "); Sprint(pm10stdMF.out());
            Sprint("\t\tPM 2.5: "); Sprint(pm25stdMF.out());
            Sprint("\t\tPM 10: "); Sprintln(pm100stdMF.out());
            Sprintln("Concentration Units (environmental, RAW | FILTERED)");
            Sprint("PM 1.0: "); Sprint(pmsData.pm10_env);
            Sprint("\t\tPM 2.5: "); Sprint(pmsData.pm25_env);
            Sprint("\t\tPM 10: "); Sprintln(pmsData.pm100_env);
            Sprintln("---------------------------------------");
            Sprintln("PM 1.0: "); Sprint(pm10envMF.out());
            Sprint("\t\tPM 2.5: "); Sprint(pm25envMF.out());
            Sprint("\t\tPM 10: "); Sprintln(pm100envMF.out());
            Sprintln("---------------------------------------");
            Sprintln("Particles > X / 0.1L air");
            Sprint("Particles > 0.3um: "); Sprintln(pmsData.particles_03um);
            Sprint("Particles > 0.5um: "); Sprintln(pmsData.particles_05um);
            Sprint("Particles > 1.0um: "); Sprintln(pmsData.particles_10um);
            Sprint("Particles > 2.5um: "); Sprintln(pmsData.particles_25um);
            Sprint("Particles > 5.0um: "); Sprintln(pmsData.particles_50um);
            Sprint("Particles > 10 um: "); Sprintln(pmsData.particles_100um);
            Sprintln("---------------------------------------"); 
            Sprint("AQI PM25: "); Sprint(myAQI.pm25AQI);
            Sprint("AQI PM25: "); Sprintln(myAQI.pm10AQI);
            Sprintln("---------------------------------------"); 
            Sprintln("Going to sleep...");
              app.onDelay(PMS_DELAY, []() { // Возможно здесь стоит переводить датчик в режим сна. После пробуждения подождать не менее 30 сек
                Sprintln("Wake the f*ck up samurai!");  
                pmsSleep = false;
              });
            counterMeasure = 0;
            pmsSleep = true; 
            }
          }
        });
    });
}

// Это аналог точки входа в программе (как setup())
ReactESP app([] () {
  SERIAL_DEBUG; // Конфигурация связи ПК-ESP
  pmsSerial.begin(PMS5003_SERIAL_BAUD); // Конфигурация связи ESP-PMS
  if (! aqi.begin_UART(&pmsSerial)) { // Если не найдет датчик, то зависнет
    Sprintln("\r\n======PMS5003-STATUS======");
    Sprintln("Could not find PM 2.5 sensor!");
    while (1) delay(10); // Поэтому стоит добавить свою логику
  }
  Sprintln("PM25 found!!!");
  onTickHandler(app); // Можно использовать для переодического вызова на каждый тик (аналог loop())
  onRepeatHandlerPMS5003(app, pmsSerial); // Навешиваем обработчик для опроса датчика
  pinMode ( LED_BUILTIN, OUTPUT ); // Можно помигать светодиодом :)
  digitalWrite ( LED_BUILTIN, LED_BUILTIN_LOW );
  // Системная информация
  Sprintln("\r\n======SYSTEM-STATUS======");
  Sprintln("\r\nDevice name: "); Sprint(SKETCH_NAME);
  Sprintln("\r\nSoftware version:"); Sprint(SW_VERSION);
  Sprintln("\r\nChipId: "); Sprint(ESP.getFlashChipId());
  Sprintln("\r\nFlashChipSize: "); Sprint(ESP.getFlashChipSize());
  Sprintln("\r\nFlashChipSpeed: "); Sprint(ESP.getFlashChipSpeed());
  Sprintln("\r\nCycleCount: "); Sprint(ESP.getCycleCount());
  Sprintln("\r\n---------------------------------------");
  // Информация общая
  Sprintln("\r\n======COMMON-INFO======");
  Sprintln("\r\nSketch: "); Sprint ( SKETCH_NAME );
  Sprintln("\r\nVersion: "); Sprint ( SW_VERSION );
  Sprintln("\r\nAuthor: "); Sprint ( SKETCH_AUTHOR );
  Sprintln("\r\n---------------------------------------");
});

// Пример, каждый программный тик исполняется определенный код (аналог loop())
void onTickHandler(ReactESP &app){
  app.onTick([]() {
    // Blynk.run(); // Например, синхронизация с сервером Blynk
  });
}
