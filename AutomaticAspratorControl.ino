#include "arduino_secrets.h"
#include "thingProperties.h"  
#include <DHT.h>

// IO pin definitions
#define LAMP_PIN D0
#define SPEED1_PIN D1
#define SPEED2_PIN D2
#define SPEED3_PIN D3
#define WIFI_STATUS_PIN D4
#define DHTPIN D5
#define DHTTYPE DHT11
#define MQ135_PIN A0

DHT dht(DHTPIN, DHTTYPE);

const float Ro = 76.8;

unsigned long lastSensorReadTime = 0;
const unsigned long sensorReadInterval = 3000;
unsigned long lastFuzzyEvalTime = 0;
const unsigned long fuzzyInterval = 60000;

float temperatureData[20];
float humidityData[20];
float ppmData[20];
int dataIndex = 0;

float getResistance(int raw_adc) {
  return (1023.0 / raw_adc - 1.0) * 10.0;
}

float getRatio(float Rs) {
  return Rs / Ro;
}

void setup() {
  Serial.begin(9600);
  delay(1500);

  pinMode(LAMP_PIN, OUTPUT);
  pinMode(SPEED1_PIN, OUTPUT);
  pinMode(SPEED2_PIN, OUTPUT);
  pinMode(SPEED3_PIN, OUTPUT);
  pinMode(WIFI_STATUS_PIN, OUTPUT);

  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

   ESP.wdtEnable(15000); //WDT 15sn de bir

  dht.begin();
  Serial.println("System initialized.");
}

void setFanSpeed(int speed) {
  switch (speed) {
    case 1:
      digitalWrite(SPEED1_PIN, HIGH); digitalWrite(SPEED2_PIN, LOW); digitalWrite(SPEED3_PIN, LOW); break;
    case 2:
      digitalWrite(SPEED1_PIN, LOW); digitalWrite(SPEED2_PIN, HIGH); digitalWrite(SPEED3_PIN, LOW); break;
    case 3:
      digitalWrite(SPEED1_PIN, LOW); digitalWrite(SPEED2_PIN, LOW); digitalWrite(SPEED3_PIN, HIGH); break;
    default:
      digitalWrite(SPEED1_PIN, LOW); digitalWrite(SPEED2_PIN, LOW); digitalWrite(SPEED3_PIN, LOW); break;
  }
}


  int fuzzyFanSpeed(float temp, float hum, float voc_ppm) {
  


  // Kategorilere ayır
  String tempLevel = (temp < 29.0) ? "dusuk" : (temp < 31.0) ? "orta" : "yuksek";
  String humLevel  = (hum < 40.0) ? "dusuk" : (hum < 60.0) ? "orta" : "yuksek";
  String vocLevel  = (voc_ppm < 135.0) ? "temiz" : (voc_ppm < 300.0) ? "orta" : "kirli";

  // Kural tabanlı kararlar
  if (vocLevel == "temiz" && tempLevel == "dusuk" && humLevel == "dusuk") return 0;
  if (vocLevel == "temiz" && tempLevel == "orta") return 1;
  if (vocLevel == "temiz" && tempLevel == "yuksek") return 1;

  if (vocLevel == "orta") {
    if (tempLevel == "dusuk") return 1;
    else if (tempLevel == "orta") return 2;
    else return 2;
  }

  if (vocLevel == "kirli") {
    if (tempLevel == "dusuk") return 2;
    else return 3;
  }

  // Hiçbir kural eşleşmezse: skor sistemine geç
  int score = 0;

  if (tempLevel == "orta") score += 1;
  else if (tempLevel == "yuksek") score += 2;

  if (humLevel == "orta") score += 1;
  else if (humLevel == "yuksek") score += 2;

  if (vocLevel == "orta") score += 1;
  else if (vocLevel == "kirli") score += 2;

  if (score <= 1) return 0;
  else if (score <= 3) return 1;
  else if (score <= 6) return 2;
  else return 3;
}




void loop() {
   ESP.wdtFeed(); //WDT Reset
  
  
  digitalWrite(WIFI_STATUS_PIN, WiFi.status() == WL_CONNECTED ? HIGH : LOW);
  ArduinoCloud.update();
  digitalWrite(LAMP_PIN, asplamp ? HIGH : LOW);

  if (millis() - lastSensorReadTime >= sensorReadInterval) {
    lastSensorReadTime = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int adc = analogRead(MQ135_PIN);
    float Rs = getResistance(adc);
    float p = pow(getRatio(Rs), 2.5);

    Serial.print("T: "); Serial.print(t);
    Serial.print(" C | H: "); Serial.print(h);
    Serial.print(" % | PPM: "); Serial.println(p);
    //alarmlar
    if (p < 0.8|| isnan(p)) {
    Serial.println("Gaz Alarmı");
}
    if (t > 55) {
    Serial.println("Yangın Alarmı");
}

    if (dataIndex == 0 || (abs(t - temperatureData[dataIndex - 1]) <= 10 && abs(h - humidityData[dataIndex - 1]) <= 10)) {
      if (dataIndex < 20) {
        temperatureData[dataIndex] = t;
        humidityData[dataIndex] = h;
        ppmData[dataIndex] = p;
        dataIndex++;
      }
    }
  }

  if (otomatikMod && millis() - lastFuzzyEvalTime >= fuzzyInterval) {
    lastFuzzyEvalTime = millis();
    if (dataIndex > 0) {
      float sumT = 0, sumH = 0, sumP = 0;
      for (int i = 0; i < dataIndex; i++) {
        sumT += temperatureData[i];
        sumH += humidityData[i];
        sumP += ppmData[i];
      }
      float avgT = sumT / dataIndex;
      float avgH = sumH / dataIndex;
      float avgP = sumP / dataIndex;

      int speed = fuzzyFanSpeed(avgT, avgH, avgP);
      setFanSpeed(speed);

      Serial.print("Avg T: "); Serial.print(avgT);
      Serial.print(" | Avg H: "); Serial.print(avgH);
      Serial.print(" | Avg P: "); Serial.print(avgP);
      Serial.print(" → Fan Speed: "); Serial.println(speed);
    }
    dataIndex = 0;
  }

  if (!otomatikMod) {
    setFanSpeed(aspspeed);
  }
}

// IoT callback functions
void onAsplampChange() {
  Serial.println(asplamp ? "Lamba ON" : "Lamba OFF");
}

void onAspspeedChange() {
  Serial.print("Manuel fan hızı: "); Serial.println(aspspeed);
}

void onOtomatikModChange() {
  Serial.print("Otomatik mod: "); Serial.println(otomatikMod ? "Aktif" : "Kapalı");
}

void onHumidityChange() {
  Serial.print("Humidity changed from Cloud: ");
  Serial.println(humidity);
}

void onTemperatureChange() {
  Serial.print("Temperature changed from Cloud: ");
  Serial.println(temperature);
}
