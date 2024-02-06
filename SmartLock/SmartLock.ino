#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>

SoftwareSerial sim(2, 3);

const int RELAY_PIN = 4;
const int BUZZER = 5;
const int SensorGetar = 6;
const int LED_GREEN_PIN = 7;
const int LED_RED_PIN = 8;
const int RST_PIN = 9;
const int SS_PIN = 10;

String message;

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

bool accessGranted = false; // Variabel flag untuk mengontrol akses
bool isVibrationDetected = false;

void setup()
{
  Serial.begin(9600);
  sim.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  pinMode(SensorGetar, INPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(LED_RED_PIN, HIGH);
  digitalWrite(RELAY_PIN, HIGH);
  delay(1000);
  sim.println("AT");
  //  sim.println("ATEO");
  sim.println("AT+CSQ");
  sim.println("AT+CMGF=1");
  sim.println("AT+CNMI=1,2,0,0,0");
  kirim("Sistem Standby");

  Serial.println("I am waiting for card...");
}

void loop()
{
  //============================Function Untuk Membaca Pesan Masuk===========================//
  if (Serial.available() > 0)
  {
    sim.write(Serial.read());
  }
  if (sim.available() > 0)
  {
    message = sim.readStringUntil('\n');
    Serial.println(message);
  }

  //=========================================Sensor Getar========================================//
  int statusSensorGetar = digitalRead(SensorGetar);
  if (statusSensorGetar == HIGH) // Apabila Sensor Getar mendeteksi adanya getaran
  {
    isVibrationDetected = true;
    Serial.println("Getaran Terdeteksi");
    kirim ("Getaran terdeteksi");
    alarm();
    return;
  }
  //===========================Konfigurasi RFID Untuk Permit UID Card==========================//
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));
  // Check is the PICC of Classic MIFARE typez
  // if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K)
  // {
  //   Serial.println(F("Your tag is not of type MIFARE Classic."));
  //   return;
  // }
  String strID = "";
  for (byte i = 0; i < 4; i++)
  {
    strID +=
      (rfid.uid.uidByte[i] < 0x10 ? "0" : "") + String(rfid.uid.uidByte[i], HEX) + (i != 3 ? ":" : "");
  }
  strID.toUpperCase();
  Serial.print("Tap card key: ");
  Serial.println(strID);

  //===================================UID Card yang dipermit==================================//
  if (strID.indexOf("UID_Card") >= 0) // RFID Master Card
  {
    delay(2000);
    accessGranted = true;
    Serial.println("Pintu diakses menggunakan Master Card");
    unlock();
    kirim("Pintu diakses menggunakan kartu RFID");
    // Serial.print("dengan ");
    // Serial.println(kartu2);
    return;
  }
  /*else if (strID.indexOf("43:49:B3:1B") >= 0) // RFID Master Card
    {
    delay(2000);
    accessGranted = true;
    Serial.println("Pintu diakses menggunakan Master Card");
    unlock();
    kirim("Pintu diakses menggunakan kartu RFID");
    // Serial.print("dengan ");
    // Serial.println(kartu2);
    return;
    }*/
  else
  {
    // Kartu tidak diotorisasi
    delay(2000);
    accessGranted = false;
    Serial.println("Access denied");
    kirim ("Kartu tidak dikenal terdeteksi");
    accessDenied();
    return;
  }

  /*
    //======================Pesan yang Diotoritaskan Untuk Mendapatkan Akses=====================//
    if (message == "Unlock\r")
    {
      delay(2000);
      accessGranted = true;
      unlock();
      kirim("Pintu dibuka menggunakan SMS Gateway");
      Serial.println("Pintu dibuka menggunakan SMS Gateway");
      return;
    }
    else
    {
      kirim("Pesan tidak dikenali");
      return;
    }*/
}

void unlock()
{
  // Serial.print("Pintu diakses dengan ");
  // Serial.print("pada ");
  tone(BUZZER, 2000);
  digitalWrite(LED_GREEN_PIN, HIGH);
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(RELAY_PIN, LOW);
  delay(5000);
  noTone(BUZZER);
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_RED_PIN, HIGH);
  digitalWrite(RELAY_PIN, HIGH);
}

void alarm()
{
  // Serial.println("Getaran terdeteksi");
  for (int i = 0; i <= 4; i++)
  {
    digitalWrite(LED_RED_PIN, HIGH);
    tone(BUZZER, 2000);
    delay(1000);
    noTone(BUZZER);
    digitalWrite(LED_RED_PIN, LOW);
    delay(1000);
  }
}
void kirim(String p)
{
  sim.print("AT+CMGF=1\r\n");
  delay(500);
  // sim.print("AT+CMGS=\"+6287795010939\"\r");
  sim.print("AT+CMGS=\"Nomor_HP_Tujuan\"\r");
  // sim.print("08127670651");
  // sim.print("\"r\n");
  delay(1000);
  sim.print(p);
  sim.write(0x1A);
  delay(1000);
  sim.print("AT+CMGD=3");
}

void accessDenied()
{
  digitalWrite(BUZZER, HIGH);
  digitalWrite(LED_RED_PIN, HIGH);
  delay(1000);
  digitalWrite(BUZZER, LOW);
  digitalWrite(LED_RED_PIN, LOW);
}
