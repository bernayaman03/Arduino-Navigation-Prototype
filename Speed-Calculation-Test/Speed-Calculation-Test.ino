//=========================================
// E18-D80NK Reflektör Hız Telemetri Sistemi
//=========================================

const int sensorPin = 2;


// Reflektörler arası mesafeler (cm)
float mesafeler[] = {
  11.0,
  11.0,
  10.0,
  16.0,
  10.0,
  11.0,
  11.0
};


const int toplamReflektor = 7;


int sayac = 0;

int sonSensorDurumu = HIGH;


// Zaman değişkenleri
unsigned long baslangicZamani = 0;
unsigned long oncekiZaman = 0;
unsigned long sonOkumaZamani = 0;


// Debounce
const unsigned long debounceSuresi = 150000; // microsecond


// Hız değişkenleri
float anlikHiz = 0;
float oncekiHiz = 0;

float ortalamaHiz = 0;
float ivme = 0;


float toplamMesafe = 0;


void setup()
{
  Serial.begin(115200);

  pinMode(sensorPin, INPUT_PULLUP);


  Serial.println("Reflektor Telemetri Baslatildi");

  Serial.println("---------------------------------------------");

  Serial.println(
    "No | Mesafe | Sure | Hiz | Ort Hiz | Ivme"
  );


}


void loop()
{

  if(sayac >= toplamReflektor)
  {
    return;
  }


  int durum = digitalRead(sensorPin);

  unsigned long simdi = micros();



  // Yeni reflektor algilandi
  if(durum == LOW &&
     sonSensorDurumu == HIGH &&
     (simdi - sonOkumaZamani > debounceSuresi))
  {


    // ilk reflektor
    if(sayac == 0)
    {

      baslangicZamani = simdi;
      oncekiZaman = simdi;

      sayac++;

      sonOkumaZamani = simdi;

      sonSensorDurumu = durum;

      return;
    }



    //----------------------------
    // Süre hesaplama
    //----------------------------

    float dt = (simdi - oncekiZaman) / 1000000.0;

    float toplamSure =
    (simdi - baslangicZamani) / 1000000.0;



    //----------------------------
    // Mesafe
    //----------------------------

    float mesafe_m =
    mesafeler[sayac-1] / 100.0;


    toplamMesafe += mesafe_m;



    //----------------------------
    // Anlık hız
    //----------------------------

    oncekiHiz = anlikHiz;

    anlikHiz = mesafe_m / dt;



    //----------------------------
    // Ortalama hız
    //----------------------------

    ortalamaHiz =
    toplamMesafe / toplamSure;



    //----------------------------
    // İvme
    //----------------------------

    if(sayac == 1)
    {
      ivme = 0;
    }
    else
    {
      ivme =
      (anlikHiz - oncekiHiz) / dt;
    }



    //----------------------------
    // Seri çıktı
    //----------------------------

    Serial.print("Ref: ");
    Serial.print(sayac);

    Serial.print(" | Mesafe: ");
    Serial.print(mesafe_m,3);
    Serial.print(" m");


    Serial.print(" | Sure: ");
    Serial.print(dt,6);
    Serial.print(" s");


    Serial.print(" | Hiz: ");
    Serial.print(anlikHiz,3);
    Serial.print(" m/s");


    Serial.print(" | Ort Hiz: ");
    Serial.print(ortalamaHiz,3);
    Serial.print(" m/s");


    Serial.print(" | Ivme: ");
    Serial.print(ivme,3);
    Serial.println(" m/s2");



    //----------------------------
    // Bir sonraki ölçüm
    //----------------------------

    oncekiZaman = simdi;

    sonOkumaZamani = simdi;

    sayac++;

  }


  sonSensorDurumu = durum;

}
