// Sensörün bağlı olduğu dijital pin
#define SENSOR_PIN 3 

int sayac = 0;            
int mevcutDurum = 0;      
int sonDurum = 1;         

void setup() {
  Serial.begin(9600);
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  Serial.println("Sayaç Sistemi Hazır. Sensörü tetikleyin...");
}

void loop() {

  mevcutDurum = digitalRead(SENSOR_PIN);

  if (mevcutDurum != sonDurum) {
    
//Sensörün NPN veya PNP olma durumuna göre mevcutDurum değişken değerini değiştir.
    if (mevcutDurum == LOW) {
      sayac++; 
      Serial.print("Cisim Algılandı! Toplam Sayı: ");
      Serial.println(sayac);
    }
    
  
    delay(50);
  }

  sonDurum = mevcutDurum;
}