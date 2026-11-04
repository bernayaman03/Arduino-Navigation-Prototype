//=============================
// Hall Sensör Telemetri Sistemi
//=============================

const int sensorPin = 2;

// Tekerlek çevresi (metre)
const float tekerlekCevresi = 0.173;


const unsigned long debounce = 20;

int tur = 0;
int sonDurum = HIGH;

unsigned long baslangicZamani = 0;
unsigned long oncekiZaman = 0;
unsigned long sonOkuma = 0;

float anlikHiz = 0.0;
float oncekiHiz = 0.0;
float ortalamaHiz = 0.0;
float ivme = 0.0;
float toplamMesafe = 0.0;

void setup()
{
    Serial.begin(115200);

    pinMode(sensorPin, INPUT_PULLUP);

    Serial.println("Telemetri Basladi");
    Serial.println("Tur,ToplamSure(s),TurSuresi(s),AnlikHiz(m/s),OrtalamaHiz(m/s),Ivme(m/s2),Mesafe(m)");
}

void loop()
{
    int durum = digitalRead(sensorPin);
    unsigned long simdi = micros();

    if (durum == LOW &&
        sonDurum == HIGH &&
        (simdi - sonOkuma > debounce * 1000UL))
    {

        
        if (tur == 0)
        {
            baslangicZamani = simdi;
            oncekiZaman = simdi;
        }
        else
        {
           

            float dt = (simdi - oncekiZaman) / 1000000.0;
            float toplamSure = (simdi - baslangicZamani) / 1000000.0;

            //--------------- Mesafe -----------------

            toplamMesafe = tur * tekerlekCevresi;

            //--------------- Hız --------------------

            oncekiHiz = anlikHiz;
            anlikHiz = tekerlekCevresi / dt;

            //--------------- Ortalama Hız ----------

            ortalamaHiz = toplamMesafe / toplamSure;

            //--------------- İvme -------------------

            if (tur == 1)
            {
                // İlk hesaplamada önceki hız olmadığı için
                ivme = 0;
            }
            else
            {
                ivme = (anlikHiz - oncekiHiz) / dt;
            }

           

           Serial.print("Tur: ");
Serial.print(tur);

Serial.print(" | Mesafe: ");
Serial.print(toplamMesafe,3);
Serial.print(" m");

Serial.print(" | Toplam Sure: ");
Serial.print(toplamSure,6);
Serial.print(" s");

Serial.print(" | Tur Suresi: ");
Serial.print(dt,6);
Serial.print(" s");

Serial.print(" | Anlik Hiz: ");
Serial.print(anlikHiz,2);
Serial.print(" m/s");

Serial.print(" | Ortalama Hiz: ");
Serial.print(ortalamaHiz,2);
Serial.print(" m/s");

Serial.print(" | Ivme: ");
Serial.print(ivme,2);
Serial.println(" m/s^2");

        }

        tur++;

        oncekiZaman = simdi;
        sonOkuma = simdi;
    }

    sonDurum = durum;
}
