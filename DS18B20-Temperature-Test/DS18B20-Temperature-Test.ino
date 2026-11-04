#include <OneWire.h>
#include <DallasTemperature.h>


// Veri kablosunun bağlı olduğu Arduino Mega pini
#define ONE_WIRE_BUS 2 


OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

int deviceCount = 0;

void setup() {
  
  Serial.begin(9600);
  Serial.println("DS18B20 Testi Başlatılıyor...");


  sensors.begin();

  deviceCount = sensors.getDeviceCount();
  Serial.print("Bulunan Sensör Sayısı: ");
  Serial.println(deviceCount);
  Serial.println("--------------------------");
}

void loop() {

  sensors.requestTemperatures(); 
  
  for (int i = 0; i < deviceCount; i++) {

    float tempC = sensors.getTempCByIndex(i);
    
    Serial.print("Sensör ");
    Serial.print(i + 1); 
    Serial.print(" Sıcaklık: ");
    

    if (tempC == DEVICE_DISCONNECTED_C) {
      Serial.println("Hata! Sensör okunamıyor.");
    } else {
      Serial.print(tempC);
      Serial.println(" °C");
    }
  }
  
  Serial.println("--------------------------");
  

  delay(2000); 
}