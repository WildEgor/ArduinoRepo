#include "PubSubClient.h" // Библиотека для работы с MQTT
#include <BlynkSimpleEsp32.h> // Библиотека для работы с Blynk сервером
#include <ArduinoJson.h> // Библиотека для работы с JSON

#define BLYNK_PRINT Serial // Для отладки
#define BLYNK_MAX_READBYTES 1024 // Буфер для отладки
//#define MSG_BUFFER_SIZE  1024
//char msg[MSG_BUFFER_SIZE];

BlynkTimer timer, timer1, timer2, timer3; // Таймеры для вызова ПП

int checkflag = 0; // Для запроса HTTP
int indexGPS = 0; // Для модуля GPS
bool ConBlynk = true;
unsigned long TimeBetween = 0;
unsigned long TimeStart, TimeCur, TimerAll = 0;

float lat = 43.224671; // Координаты частного дома
float lon = 76.871088;

String webhookdata; // JSON ответ

struct DEMS // Структура данных
{
  float windspeed;
  float windpower;
  int solarrad;
  float solarpower;
  float curload;
  float batterysoc;
  int RequestChoice = 1;
  float mygps[1];
  float voltage;
  float cur;
};
DEMS DEMS;

char auth[] = "API_BLYNK"; // API для Blynk
//char auth[] = "API_BLYNK";
//char serveraddress[] = "trsh.su"; // Адреса облачного на выбор сервера
//char serveraddress[] = "oasiskit.com";
char serveraddress[] = "192.168.1.7"; // IP локального сервера

char ssid[] = "Home_Network"; // Моя Wi-Fi сеть (дом)
char pass[] = "123";
char ssidAndroid[] = "AndroidAPf97a"; // Моя Wi-Fi сеть (телефон)
char passAndroid[] = "123";

// char* topic = "channels/991879/publish/S0OEVBLG2U4AN3FF";

const char* server = "mqtt.thingspeak.com"; // Адрес сервера
char mqttUserName[] = "ESP8266Client"; // Имя клиента для MQTT
char mqttPass[] = "API_MQTT"; // Уникальный API ключ для MQTT
long readChannelID = 991879; // Можно задать разные номера для записи и чтения
long writeChannelID = 991879;
char readAPIKey[] = "API_MQTT"; // Уникальный API ключ для HTTP для чтения и записи
char writeAPIKey[]   = "API_MQTT";
const int mqttPort = 1883; // Порт для MQTT

long CheckWiFiConnectionTime = 15000L; // Проверить wi-fi соединение каждые 15 сек
long CheckMQTTConnection = 15000L; // Проверить связь с сервером MQTT соединение каждые 15 сек
long CheckThingSpeakRequesTime = 2000L; // Сделать запросы и обновить пины
long CheckBlynkConnectionTime = 1000L; // Проверяем связь с Blynk

WiFiClient wifiClient; // Объект Wi-Fi клиента
PubSubClient client(wifiClient); // Объект MQTT клиента
WidgetMap myMap(V8); // модуль GPS в Blynk

//********************* Метод проверки соединений приложения с сервером Блинк ***************************
void CheckConnection(){
  TimeBetween = 0;
  ConBlynk = Blynk.connected();
  if(!ConBlynk){
    Serial.println("Нет соединения");
    WiFiConnect();
  }
  else{
    Serial.println("Все ок! Работаем дальше..");
  }
}

//********************* Функция организует подключение к Wi-Fi сети частного дома ***************************
void WiFiConnect()
{
  TimeBetween = 0;
  int switcher = 0;
  String clientName="ESP-Thingspeak";
  if (WiFi.status() != WL_CONNECTED)
  {
    while (WiFi.status() != WL_CONNECTED)
    {
    if (switcher == 0)
    {
      WiFi.begin(ssidAndroid, passAndroid);
      Serial.print("Connection to Android in process...");
      switcher = 1;
      delay(1000);
    }
    else if (switcher == 1)
    {
      WiFi.begin(ssid, pass);
      Serial.print("Connection to PC in process...");
      switcher = 0;
      delay(1000);
    }
    }
    Serial.println("");
    if (switcher == 0)
    {
    Serial.println("WiFi PC connected");
    } else if (switcher == 1)
    {
      Serial.println("WiFi Android connected");
    }
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Connecting to ");
    Serial.print(server);
    Serial.print(" as ");
    Serial.println(clientName);
     if (switcher == 0)
    {
      switcher = 1;
    } else switcher = 0;
  }
}

//********************* Функция формирует рандомный ID клиента MQTT для возможности подключения ***************************
void getID(char clientID[], int idLength)
{
  static const char alphanum[] ="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  for (int i = 0; i < idLength; i++)
  {
    clientID[i] = alphanum[random(62)];
  }
  clientID[idLength] = '\0';
}

//********************* Функция вызывается каждый раз когда происходит обновления подписок MQTT ***************************
void callback(char* topicX, byte* payloadX, unsigned int length)
{
    char p[length + 1];
    memcpy(p,payloadX,length);
    p[length] += NULL;
    webhookdata = p;
    Serial.print( "Answer " + String(length) + " : ");
    Serial.println(String(p));
}

//********************* Функция обновления пинов Blynk. Это передача значений на сервер blynk и в приложение ***************************
void UpdateVirtualPins()
{
  Blynk.virtualWrite(V0, DEMS.windspeed);
  Blynk.virtualWrite(V1, DEMS.windpower);
  Blynk.virtualWrite(V2, DEMS.solarrad);
  Blynk.virtualWrite(V3, DEMS.solarpower);
  Blynk.virtualWrite(V4, DEMS.curload);
  Blynk.virtualWrite(V5, DEMS.batterysoc);
  Blynk.virtualWrite(V10, DEMS.RequestChoice);
  TimeCur = millis();
  TimerAll = TimeCur - TimeStart;
  TimeBetween = TimerAll;
  if (DEMS.RequestChoice == 1)
  {
    Blynk.virtualWrite(V11, TimeBetween);
  }
  else if (DEMS.RequestChoice == 2)
  {
    Blynk.virtualWrite(V12, TimeBetween);
  }
    Serial.println(TimeBetween);
    TimerAll = 0;
    TimeCur = 0;
    TimeStart = 0;
    checkflag = 0;
  TimeBetween = 0;
}

//********************* Функция формирования запроса HTTP/MQTT и обработки JSON запросов ***************************
void CheckThingSpeak()
{
  if (DEMS.RequestChoice == 1)
  {
  checkflag = 1;
  TimeStart = millis();
  Blynk.virtualWrite(V6, checkflag);
  }
  else if (DEMS.RequestChoice == 2)
  {
    TimeStart = millis();
    if (MQTTSubscribe(readChannelID, 0, readAPIKey, 0))
    {
      //TimeStart = millis();checkflag = 1;
      client.loop();
      Serial.println("Subscribed!");
    }
  JsonParsingMQTT();
  }
  UpdateVirtualPins();
}

//********************* Вызывается получения JSON ответа от HTTP запроса ***************************
BLYNK_WRITE(V6)
{
  if (DEMS.RequestChoice == 1)
  {
  webhookdata = param.asStr();
  Serial.println(webhookdata);
  JsonParsingHTTP();
  }
}

//********************* Выбор HTTP или MQTT из приложения Blynk ***************************
BLYNK_WRITE(V10)
{
  DEMS.RequestChoice = param.asInt();
}

//********************* Функция проверяет подключение к MQTT серверу ***************************
void MQTTConnect()
{
   char clientID[9];
   client.setServer(server, mqttPort);
   client.setCallback(callback);
   if (!client.connected())
   {
    while (!client.connected())
    {
      getID(clientID, 8);
      Serial.print("Attempting MQTT connection...");
      if (client.connect(clientID, mqttUserName, mqttPass))
        {
          Serial.println("Connected with Client ID: " + String(clientID) + " User "+ String(mqttUserName) + " Pwd "+String(mqttPass));
        }
        else
          {
            TimeBetween = 0;
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
          }
    }
   }
   client.loop();
}

//********************* Подписка на канал связи Thingspeak ***************************
int MQTTSubscribe(long subChannelID, int field, char* readKey, int unsubSub)
{
String myTopic;
  if (field == 0)
  {
        myTopic = "channels/" + String(subChannelID) + "/subscribe/json/" + String(readKey);
    }
    else
  {
        myTopic = "channels/" + String(subChannelID) + "/subscribe/fields/field" + String(field) + "/" + String(readKey);
    }
    Serial.println("Subscribing to " + myTopic);
    Serial.println("State= " + String(client.state()));
  if (unsubSub == 1)
  {
    return client.unsubscribe(myTopic.c_str());
  }
return client.subscribe(myTopic.c_str(),0);
}

//********************* Функция чтения JSON запроса  ***************************
void JsonParsingHTTP()
{
  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(10) + JSON_OBJECT_SIZE(16) + 570;
  DynamicJsonDocument doc(capacity);
  DeserializationError error = deserializeJson(doc, webhookdata);
  JsonObject feeds_0 = doc["feeds"][0];
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
    Serial.println("HTTP REQUEST IN WORK");
    DEMS.windspeed = feeds_0["field1"];
    DEMS.windpower = feeds_0["field2"];
    DEMS.solarrad = feeds_0["field3"];
    DEMS.solarpower = feeds_0["field4"];
    DEMS.curload = feeds_0["field5"];
    DEMS.batterysoc = feeds_0["field6"];
    DEMS.voltage = feeds_0["field7"];
    DEMS.cur = feeds_0["field8"];
}

void JsonParsingMQTT()
{
  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(10) + JSON_OBJECT_SIZE(16) + 570;
  DynamicJsonDocument doc(capacity);
  DeserializationError error = deserializeJson(doc, webhookdata);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
    Serial.println("MQTT REQUEST IN WORK");
    DEMS.windspeed = doc["field1"];
    DEMS.windpower = doc["field2"];
    DEMS.solarrad = doc["field3"];
    DEMS.solarpower = doc["field4"];
    DEMS.curload = doc["field5"];
    DEMS.batterysoc = doc["field6"];
    DEMS.voltage = doc["field7"];
    DEMS.cur = doc["field8"];
}

//********************* Функция инициализации  ***************************
void setup()
{
  int status = WL_IDLE_STATUS;
  Serial.begin(115200);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,8), 8080);
  WiFiConnect();
  MQTTConnect();
  Blynk.begin(auth, ssid, pass, serveraddress, 8080);
  myMap.location(indexGPS, lat, lon, "MyGPS");
  timer.setInterval(CheckWiFiConnectionTime, WiFiConnect);
  timer1.setInterval(CheckMQTTConnection, MQTTConnect);
  timer2.setInterval(CheckThingSpeakRequesTime, CheckThingSpeak);
  timer3.setInterval(CheckBlynkConnectionTime, CheckConnection);
}

//********************* Основной цикл программы  ***************************
void loop()
{
  if(ConBlynk){
    Blynk.run();  // Вызываем функцию если соединение есть
  } else {
      Blynk.connect();
      Blynk.run();
  }
  timer.run();
  timer1.run();
  timer2.run();
  timer3.run();
}
