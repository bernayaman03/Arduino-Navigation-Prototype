#include <Wire.h>
#include <MPU6050_tockn.h>
#include <OneWire.h>
#include <DallasTemperature.h>


MPU6050 mpu(Wire);
#define ONE_WIRE_BUS 13
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensorler(&oneWire);
DeviceAddress birTermometre, ikiTermometre;


const int sensorPin1 = A0; 
const int sensorPin2 = A1; 
const int sensorPin3 = A2; 
 
const int otonomPin = 12; 
const int CANLILIK_SINYALI_PIN = 11;
const int relayForward = 10; 
const int relayBackward = 9;  
const int relayStop = 8;  
const int ileriSignal = 6;  
const int frenSignal = 5;  
const int geriSignal = 4;         
const int hallPin = 3;        


const float tekerlekCevresi = 0.1728; 
const unsigned long debounceSuresi = 50000; 
const float FREN_KONUM_TOLERANSI = 0.001;  
const float ACI_FILTRE_ESIGI = 2.0; 
#define VERI_GONDERME_ARALIGI 100


volatile signed long turSayisi = 0;
long sonMevcutTur = 0; 
volatile unsigned long oncekiZaman = 0; 
volatile float anlikHizV = 0;
volatile bool motorYonuIleri = true; 
volatile unsigned long enSonTetikle_ms = 0; 
unsigned long zamanSayaci = 0; 
bool yarisBasladi = false;     

float AY = 0, AZ = 0, VY = 0, VZ = 0, LY = 0, LZ = 0;
float offsetLY = 0, offsetLZ = 0; 
float MT1 = 0, MT2 = 0, MT3 = 0; 

int R = 0;
int dogruveri = 0;
int ayniveri = 0;
bool engelAlgilandi = false; 
unsigned long sonReflektorZamani = 0;

float g = 9.81;
float oncekiGonderilenHizX = 0;
unsigned long sonVeriGondermeZamani = 0;
unsigned long motorCalismaBaslangicZamani = 0;

enum OtonomDurum { 
    OTON_GIDIS,       
    OTON_GIDIS_FREN,  
    OTON_DONUS,       
    OTON_DONUS_FREN,  
    OTON_GOREV_TAMAM  
};
OtonomDurum otonomDurum = OTON_GIDIS;

float otonomBaslangicLX = 0;
int otonomBeklemeSayaci = 0;
float otonomOncekiKonum = 0;

bool emniyetFreniBitti = false;

// --- Fonksiyonlar ---

void reflektorOku() {
    int v1 = !digitalRead(sensorPin1);
    int v2 = !digitalRead(sensorPin2);
    int v3 = !digitalRead(sensorPin3);
    
    ayniveri = 0; 

    if (v1 == 1 && v2 == 1) ayniveri++; 
    if (v1 == 1 && v3 == 1) ayniveri++;
    if (v2 == 1 && v3 == 1) ayniveri++;

    if (ayniveri >= 1) {
       
        dogruveri = 1;
    } else {
        dogruveri = 0; 
        engelAlgilandi = false; 
    }
}

void ileriMesafeHesapla() {
    float eskiLX = LX;
    R++; 
   
    if (R == 1) LX = 6.0;
    else if (R > 1  && R <= 20) LX += 4.0;
    else if (R > 20 && R <= 39) LX += 0.10;
    else if (R == 40)           LX += 2.10;
    else if (R > 40 && R <= 52) LX += 4.0;
    else if (R > 52 && R <= 61) LX += 0.10;
    else if (R == 62)           LX += 3.10;
    else if (R > 62 && R <= 73) LX += 4.0;

    // Reflektör bazlı hız düzeltmesi (Gerçek zamanlı)
    unsigned long suankiZaman = millis();
    float dt = (suankiZaman - sonReflektorZamani) / 1000.0;
    if (dt > 0 && sonReflektorZamani > 0) {
        VX = (LX - eskiLX) / dt; // Reflektörler arası hızı doğrula
    }
    sonReflektorZamani = suankiZaman;
}

void geriMesafeHesapla() {
    float eskiLX = LX;
 
    if (R > 62 && R <= 73)      LX -= 4.0; 
    else if (R == 62)           LX -= 3.10; 
    else if (R > 52 && R <= 61) LX -= 0.10;
    else if (R > 40 && R <= 52) LX -= 4.0;
    else if (R == 40)           LX -= 2.10;
    else if (R > 20 && R <= 39) LX -= 0.10;
    else if (R > 1  && R <= 20) LX -= 4.0;
    else if (R == 1)            LX = 6.0;

    if (R > 0) R--; 

   
    unsigned long suankiZaman = millis();
    float dt = (suankiZaman - sonReflektorZamani) / 1000.0;
    if (dt > 0 && sonReflektorZamani > 0) {
        VX = (LX - eskiLX) / dt;
    }
    sonReflektorZamani = suankiZaman;
}

void hallHesapla(float hallHiz, long suankiTur) {
    
    float deltaMesafe = (suankiTur - sonMevcutTur) * tekerlekCevresi;
    LX += deltaMesafe;
    sonMevcutTur = suankiTur;

    
    AX = (hallHiz - VX) / (VERI_GONDERME_ARALIGI / 1000.0);
    VX = hallHiz;
    

}

void mesafeVeHizYonetimi() {
    noInterrupts();
    float suankiHallHizi = anlikHizV;
    long suankiTur = turSayisi;
    interrupts();

    // DURUM 1: REFLEKTÖR ALGILANDI (En az iki sensör onay veriyor)
    if (dogruveri == 1) { 
        if (!engelAlgilandi) {
         
            if (motorYonuIleri) {
                ileriMesafeHesapla();
            } else {
                geriMesafeHesapla();
            }
            
            
            sonMevcutTur = suankiTur; 
            engelAlgilandi = true; 
        }
    } 
   
    else {
       
        float deltaMesafe = (suankiTur - sonMevcutTur) * tekerlekCevresi;
        LX += deltaMesafe;
        sonMevcutTur = suankiTur;
        
      
        VX = suankiHallHizi;
        engelAlgilandi = false; 
    }


    static float oncekiVX = 0;
    AX = (VX - oncekiVX) / (VERI_GONDERME_ARALIGI / 1000.0);
    oncekiVX = VX;
}

void otonomKontrol() {
    switch (otonomDurum) {
        
        case OTON_GIDIS:
            motorYonuIleri = true;
            digitalWrite(relayStop, HIGH);     // Freni bırak
            digitalWrite(relayForward, LOW);   // İleri git
            digitalWrite(relayBackward, HIGH); 

            // 150. metreye ulaştığımız an (veya geçtiğimiz an)
            if (LX >= 150.0) { 
                otonomDurum = OTON_GIDIS_FREN; 
                otonomBeklemeSayaci = 0; 
                otonomOncekiKonum = LX;
            }
            break;

        case OTON_GIDIS_FREN:
           
            digitalWrite(relayForward, HIGH); 
            digitalWrite(relayBackward, HIGH);
            digitalWrite(relayStop, LOW);      

         
            // Konum değişimi toleransın altına düşene kadar bekle
            if (abs(LX - otonomOncekiKonum) < FREN_KONUM_TOLERANSI) {
                otonomBeklemeSayaci++;
            } else {
                otonomBeklemeSayaci = 0; 
                otonomOncekiKonum = LX;
            }

            // Araç fiziksel olarak durduysa geri dönmeye başla
            if (otonomBeklemeSayaci >= 100) { 
                otonomDurum = OTON_DONUS; 
            }
            break;

        case OTON_DONUS:
            motorYonuIleri = false;
            digitalWrite(relayStop, HIGH);     // Freni bırak
            digitalWrite(relayForward, HIGH); 
            digitalWrite(relayBackward, LOW);  // Geri git

            // Geri giderken 30. metreye ulaştığımız an
            if (LX <= 30.0) { 
                otonomDurum = OTON_DONUS_FREN; 
                otonomBeklemeSayaci = 0; 
                otonomOncekiKonum = LX;
            }
            break;

        case OTON_DONUS_FREN:
            digitalWrite(relayForward, HIGH); 
            digitalWrite(relayBackward, HIGH);
            digitalWrite(relayStop, LOW);      // Fren yap

            // Tamamen durana kadar bekle
            if (abs(LX - otonomOncekiKonum) < FREN_KONUM_TOLERANSI) {
                otonomBeklemeSayaci++;
            } else {
                otonomBeklemeSayaci = 0; 
                otonomOncekiKonum = LX;
            }

            if (otonomBeklemeSayaci >= 100) {
                otonomDurum = OTON_GOREV_TAMAM;
            }
            break;

        case OTON_GOREV_TAMAM:
            // Tüm sistemleri kapat
            digitalWrite(relayForward, HIGH);
            digitalWrite(relayBackward, HIGH);
            digitalWrite(relayStop, HIGH); 
            break;
    }
}

void turAlgilandi() {
    unsigned long curMicros = micros();
    if (curMicros - oncekiZaman < debounceSuresi) return;
    if (oncekiZaman > 0) {
        anlikHizV = tekerlekCevresi / ((curMicros - oncekiZaman) / 1.0e6);
        enSonTetikle_ms = millis();
    }
    if (motorYonuIleri) turSayisi++; else turSayisi--;
    oncekiZaman = curMicros;
}

void IMU_VeriIsle() {
    mpu.update();

   
    AY = mpu.getAccY() * 9.81;
    AZ = mpu.getAccZ() * 9.81;

    // Eğim ve Yatış Açılarını Hesapla (LY ve LZ)
    
    float anlikLY = atan2(mpu.getAccY(), mpu.getAccZ()) * RAD_TO_DEG;
    float anlikLZ = atan2(-mpu.getAccX(), sqrt(mpu.getAccY() * mpu.getAccY() + mpu.getAccZ() * mpu.getAccZ())) * RAD_TO_DEG;

 
    if (abs(anlikLY - offsetLY) > ACI_FILTRE_ESIGI) {
        LY = anlikLY - offsetLY;
    } else {
        LY = 0;
    }

    if (abs(anlikLZ - offsetLZ) > ACI_FILTRE_ESIGI) {
        LZ = anlikLZ - offsetLZ;
    } else {
        LZ = 0;
    }
}
void setup() {
    Serial.begin(9600);
    Wire.begin(); mpu.begin();
    
    Serial.println("Stabilite bekleniyor...");
    bool stabil = false;
    while (!stabil) {
        mpu.update();
        if (abs(mpu.getGyroX()) < 0.5 && abs(mpu.getGyroY()) < 0.5) {
            delay(1500);
            stabil = true;
        }
        delay(100); Serial.print(".");
    }

    mpu.calcGyroOffsets(true);
    float sumLY = 0, sumLZ = 0;
    int samples = 200;
    for(int i=0; i<samples; i++) {
        mpu.update();
        sumLY += atan2(mpu.getAccY(), mpu.getAccZ()) * RAD_TO_DEG;
        sumLZ += atan2(-mpu.getAccX(), sqrt(mpu.getAccY() * mpu.getAccY() + mpu.getAccZ() * mpu.getAccZ())) * RAD_TO_DEG;
        delay(5);
    }
    offsetLY = sumLY / samples;
    offsetLZ = sumLZ / samples;

    sensorler.begin();
    sensorler.getAddress(birTermometre, 0); sensorler.getAddress(ikiTermometre, 1);
    
    pinMode(CANLILIK_SINYALI_PIN, OUTPUT); digitalWrite(CANLILIK_SINYALI_PIN, HIGH);
    pinMode(relayForward, OUTPUT); pinMode(relayBackward, OUTPUT);
    pinMode(relayStop, OUTPUT); 
    pinMode(frenSignal, INPUT); pinMode(ileriSignal, INPUT);
    pinMode(geriSignal, INPUT); pinMode(otonomPin, INPUT);
    pinMode(hallPin, INPUT_PULLUP); 
    
    pinMode(sensorPin1, INPUT_PULLUP); pinMode(sensorPin2, INPUT_PULLUP); pinMode(sensorPin3, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(hallPin), turAlgilandi, FALLING);
    digitalWrite(relayForward, HIGH); digitalWrite(relayBackward, HIGH); digitalWrite(relayStop, HIGH); 
}

void loop() {
    unsigned long currentMillis = millis();
    
    // Sistem canlılık sinyali
    digitalWrite(CANLILIK_SINYALI_PIN, HIGH);
    
    // Sensör Verilerini Oku
    reflektorOku();         
    mesafeVeHizYonetimi();  
    IMU_VeriIsle(); 

    // Giriş Pinlerini Oku
    int fren = digitalRead(frenSignal);
    int ileri = digitalRead(ileriSignal);
    int geri = digitalRead(geriSignal);
    int otonom_mod = digitalRead(otonomPin);

    if (LX >= 150.0 && !emniyetFreniBitti) {
        digitalWrite(relayForward, HIGH); 
        digitalWrite(relayBackward, HIGH);
        digitalWrite(relayStop, LOW); 
        
        // Duruş Kontrolü
        if (abs(LX - otonomOncekiKonum) < FREN_KONUM_TOLERANSI) {
            otonomBeklemeSayaci++;
            if (otonomBeklemeSayaci >= 150) {
                digitalWrite(relayStop, HIGH); 
                emniyetFreniBitti = true; 
                otonomBeklemeSayaci = 0; 
            }
        } else {
            otonomBeklemeSayaci = 0; 
            otonomOncekiKonum = LX;
            
        }
    } 
    // --- BOŞA ALMA DURUMU: 
    else if (fren == LOW && ileri == LOW && geri == LOW && otonom_mod == LOW) {
        digitalWrite(relayForward, HIGH);
        digitalWrite(relayBackward, HIGH);
        digitalWrite(relayStop, HIGH);
        
        // Duruş Kontrolü 
        if (abs(LX - otonomOncekiKonum) < FREN_KONUM_TOLERANSI) {
            otonomBeklemeSayaci++;
            if (otonomBeklemeSayaci >= 150) {
                
            }
        } else {
            otonomBeklemeSayaci = 0; 
            otonomOncekiKonum = LX;
            
        }
    }
    // --- NORMAL ÇALIŞMA MODLARI ---
    else {
        if (fren == HIGH || ileri == HIGH || geri == HIGH) {
            if (fren == HIGH) {
                digitalWrite(relayStop, LOW); 
                digitalWrite(relayForward, HIGH); 
                digitalWrite(relayBackward, HIGH); 
            } else if (ileri == HIGH) {
                motorYonuIleri = true;
                digitalWrite(relayStop, HIGH); 
                digitalWrite(relayForward, LOW);
                digitalWrite(relayBackward, HIGH); 
            } else if (geri == HIGH) {
                motorYonuIleri = false;
                digitalWrite(relayStop, HIGH); 
                digitalWrite(relayForward, HIGH);
                digitalWrite(relayBackward, LOW); 
            }
            otonomDurum = OTON_GIDIS; // Manuel müdahale otonomu sıfırlar
        }
        else if (otonom_mod == HIGH) {
            static bool otonomYeniBasladi = true;
            if (otonomYeniBasladi) { otonomBaslangicLX = LX; otonomYeniBasladi = false; }
            otonomKontrol();
        }
    }

    if (millis() - enSonTetikle_ms > 500) { 
    anlikHizV = 0;
}

    // --- VERİ HESAPLAMA VE GÖNDERİMİ (100ms ARALIKLA) ---
    if (currentMillis - sonVeriGondermeZamani >= VERI_GONDERME_ARALIGI) {
        
        // Yarışma/Zamanlayıcı Kontrolü: 
        static bool yarisBasladi = false; 
        if (!yarisBasladi && abs(VX) > 0.05) {
            motorCalismaBaslangicZamani = currentMillis;
            yarisBasladi = true;
        }

        if (yarisBasladi) {
            float toplamSaniye = (currentMillis - motorCalismaBaslangicZamani) / 1000.0;
            if (toplamSaniye > 0) {
                
                AO = abs(LX) / toplamSaniye;
            }
        } else {
            AO = 0; 
        }

        // Sıcaklık verilerini güncelle
        sensorler.requestTemperatures();
        MT1 = sensorler.getTempC(birTermometre); 
        MT2 = sensorler.getTempC(ikiTermometre);
        MT3 = sensorler.getTempC(ucTermometre);
        

        Serial.print("C0:"); Serial.print(dogruveri); Serial.print(",");
        Serial.print("MT1:"); Serial.print(MT1, 2); Serial.print(",");
        Serial.print("MT2:"); Serial.print(MT2, 2); Serial.print(",");
        Serial.print("MT3:"); Serial.print(MT3, 2); Serial.print(",");
        Serial.print("VX:"); Serial.print(VX, 2); Serial.print(",");
        Serial.print("VY:"); Serial.print(VY, 2); Serial.print(","); 
        Serial.print("VZ:"); Serial.print(VZ, 2); Serial.print(","); 
        Serial.print("LX:"); Serial.print(LX, 3); Serial.print(",");
        Serial.print("LY:"); Serial.print(LY, 3); Serial.print(","); 
        Serial.print("LZ:"); Serial.print(LZ, 3); Serial.print(","); 
        Serial.print("AX:"); Serial.print(AX, 2); Serial.print(",");
        Serial.print("AY:"); Serial.print(AY, 2); Serial.print(",");
        Serial.print("AZ:"); Serial.print(AZ, 2); Serial.print(",");
        Serial.print("AO:"); Serial.print(AO, 2);
        Serial.println();

        sonVeriGondermeZamani = currentMillis;

        Sıcaklık1 = termokupl1.readCelsius();
        Sıcaklık2 = termokupl2.readCelsius();
        Sıcaklık3 = termokupl3.readCelsius();
    }
}