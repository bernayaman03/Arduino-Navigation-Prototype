#define SENSOR_SAYISI 3


const int sensorPinleri[SENSOR_SAYISI] = {2, 3, 4}; 


uint32_t sayaclar[SENSOR_SAYISI] = {0, 0, 0};
int sonDurumlar[SENSOR_SAYISI] = {HIGH, HIGH, HIGH};
unsigned long sonZamanlar[SENSOR_SAYISI] = {0, 0, 0};


const unsigned long debounceSuresi = 50; 

// Mesafe hesaplama fonksiyonu
static float MesafeHesapla(uint32_t R) {
    if (R > 73) R = 73;
    float m = 0.0f;
    for (uint32_t i = 1; i <= R; i++) {
        if (i == 1)           m += 6.0f;
        else if (i <= 20)     m += 4.0f;
        else if (i <= 39)     m += 0.1f;
        else if (i == 40)     m += 2.1f;
        else if (i <= 52)     m += 4.0f;
        else if (i <= 61)     m += 0.1f;
        else if (i == 62)     m += 3.1f;
        else if (i <= 73)     m += 4.0f;
    }
    return m;
}

// İki mesafe arasındaki +- %5 tolerans durumunu kontrol eden fonksiyon
bool toleransKontrol(float m1, float m2) {
    if (m1 == 0 && m2 == 0) return true; // Başlangıçta ikisi de sıfırken onaylasın
    
    float fark = abs(m1 - m2);
    // Güvenlik için iki değerden büyük olanı referans alarak %5'ini hesaplıyoruz
    float referans = max(m1, m2); 
    
    return (fark <= (referans * 0.05)); 
}

void setup() {
    Serial.begin(9600); 
    
    for (int i = 0; i < SENSOR_SAYISI; i++) {
        pinMode(sensorPinleri[i], INPUT_PULLUP);
    }
    
    Serial.println("Sistem Hazır: Üçlü Sensör Okuması Başlıyor...");
}

void loop() {
    bool yeniOkumaVar = false;

    for (int i = 0; i < SENSOR_SAYISI; i++) {
        int okuma = digitalRead(sensorPinleri[i]);

    
        if (okuma == LOW && sonDurumlar[i] == HIGH && (millis() - sonZamanlar[i]) > debounceSuresi) { 
            sayaclar[i]++;
            yeniOkumaVar = true; 
            sonZamanlar[i] = millis(); 
        }
        

        sonDurumlar[i] = okuma;
    }

    if (yeniOkumaVar) {
        float m1 = MesafeHesapla(sayaclar[0]);
        float m2 = MesafeHesapla(sayaclar[1]);
        float m3 = MesafeHesapla(sayaclar[2]);

        Serial.print("Hesaplanan -> S1: "); Serial.print(m1);
        Serial.print("m | S2: "); Serial.print(m2);
        Serial.print("m | S3: "); Serial.print(m3);
        Serial.print("m  ====>  ");

        // 1. Senaryo: Üç sensör de %5 tolerans içinde eşleşiyor
        if (toleransKontrol(m1, m2) && toleransKontrol(m1, m3) && toleransKontrol(m2, m3)) {
            float ortalama = (m1 + m2 + m3) / 3.0;
            Serial.print("SONUÇ: GÜVENLİ (3 Sensör) | Kapsül Mesafesi: ");
            Serial.println(ortalama);
        }
        // 2. Senaryo: Sadece Sensör 1 ve Sensör 2 eşleşiyor (3. sensör hata vermiş olabilir)
        else if (toleransKontrol(m1, m2)) {
            float ortalama = (m1 + m2) / 2.0;
            Serial.print("SONUÇ: ONAYLI (S1-S2) | Kapsül Mesafesi: ");
            Serial.println(ortalama);
        }
        // 3. Senaryo: Sadece Sensör 1 ve Sensör 3 eşleşiyor
        else if (toleransKontrol(m1, m3)) {
            float ortalama = (m1 + m3) / 2.0;
            Serial.print("SONUÇ: ONAYLI (S1-S3) | Kapsül Mesafesi: ");
            Serial.println(ortalama);
        }
        // 4. Senaryo: Sadece Sensör 2 ve Sensör 3 eşleşiyor
        else if (toleransKontrol(m2, m3)) {
            float ortalama = (m2 + m3) / 2.0;
            Serial.print("SONUÇ: ONAYLI (S2-S3) | Kapsül Mesafesi: ");
            Serial.println(ortalama);
        }
        // 5. Senaryo: Hiçbiri birbiriyle eşleşmiyor 
        else {
            Serial.println("SONUÇ: KRİTİK HATA - Sensör verileri tolerans dışı!");
        }
    }
}