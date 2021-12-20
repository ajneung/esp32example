#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 OLED(-1);

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

char auth[] = "xxxxxxxxxxxxxxxxxx"; //  Token ที่ แอปส่งเข้า Email ให้น้ำมาใส่แทน
char ssid[] = "xxxxxxx"; // ชื่อไวไฟ ที่บอร์ดจะ connect
char pass[] = "xxxxxxxx"; // พาสเวิร์ดของไวไฟ

int ReCnctCount = 0; // Reconnection counter

#define DHTPIN 4 /// Pin ที่ต่ออยุ่กับ sensor วัดอุณหภูมิ
#define DHTTYPE DHT11 // ชนิดของเซนเซอร์

BLYNK_CONNECTED() {
  Blynk.syncAll(); // ถ้ามีการถอดปลั๊กหรือไฟดับ ให้ใช้สถานะเดิม
}

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

//////////////////////////////// ชื่อตัวแประที่เราจะใช้เรียกไว้เก็บค่าที่ set ใน app //////////////////
int Temp_Max = 0;
int Temp_Min = 0;
int Hum_Max = 0;
int Hum_Min = 0;
int Light_Set = 0;

int Temp = 0;
int Hum = 0;
int Light = 0;

int botton_Temp = 0;
int botton_Hum = 0;
int botton_Light = 0;
/////////////////////////////////////////////////////////////////////////////////////////

BLYNK_WRITE(V41)  // ดึงค่า V41 จากแอป มาเก็บไว้ใน Temp_Max
{
  Temp_Max = param.asInt();
}
BLYNK_WRITE(V42)  // ดึงค่า V42 จากแอป มาเก็บไว้ใน Temp_Min
{
  Temp_Min = param.asInt();
}
BLYNK_WRITE(V43)  // ดึงค่า V43 จากแอป มาเก็บไว้ใน Hum_Max
{
  Hum_Max = param.asInt();
}
BLYNK_WRITE(V44)  // ดึงค่า V44 จากแอป มาเก็บไว้ใน Hum_Min
{
  Hum_Min = param.asInt();
}
BLYNK_WRITE(V45)  // ดึงค่า V45 จากแอป มาเก็บไว้ใน Light_Set
{
  Light_Set = param.asInt();
}
BLYNK_WRITE(V35)  // ดึงค่า V35 จากแอป มาเก็บไว้ใน botton_Temp
{
  botton_Temp = param.asInt();
  if (botton_Temp == 0) {
    Blynk.virtualWrite(V38, 0);
    digitalWrite(12, LOW);
  }

}
BLYNK_WRITE(V36)  // ดึงค่า V36 จากแอป มาเก็บไว้ใน botton_Hum
{
  botton_Hum = param.asInt();
  if (botton_Hum == 0 ) {
    Blynk.virtualWrite(V39, 0);
    digitalWrite(14, LOW);
  }
}
BLYNK_WRITE(V37)  // ดึงค่า V37 จากแอป มาเก็บไว้ใน botton_Light
{
  botton_Light = param.asInt();
  if (botton_Light == 0) {
    Blynk.virtualWrite(V40, 0);
    digitalWrite(27, LOW);
  }
}

void setup()
{
  OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C); // initialize with the I2C addr 0x3C (for the 128x64)
  Serial.begin(115200);
  delay(1);
  Blynk.begin(auth, ssid, pass, "bstick-board.com", 8080); // ชื่อ server และ port ที่ใช้
  dht.begin();
  timer.setInterval(3000, Sensor); ///////////////// สั่งให้เช็คทุกๆ 3 วินาที
}


void loop()
{
  if (Blynk.connected()) {
    ReCnctCount = 0;
  } else  {
    Serial.println("Starting reconnection timer in 5 seconds…");
    ReCnctCount++;
    Serial.print("Attempting reconnection #");
    delay(10000);
    ESP.restart();
    delay(3000);
    Serial.println(ReCnctCount);
    Blynk.connect();
  }
  Blynk.run();
  timer.run();
}

long rssi;
void Sensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  rssi = WiFi.RSSI();  // eg. -63
  if (rssi < -99)
  {
    rssi = -99;
  } else {
    rssi = rssi + 100;
  }

  int LDR = analogRead(A0); // pin ที่ต่อ LDR

  int measurement = 0;
  measurement = hallRead();
  Blynk.virtualWrite(V30, measurement);

  int Temp = t;
  int Hum = h;
  int Light = LDR;

  int sens_read = 0;
  sens_read = (temprature_sens_read() - 32) / 1.8;
  Blynk.virtualWrite(V31, sens_read);
  Blynk.virtualWrite(V7, LDR);
  Blynk.virtualWrite(V50, rssi);

  if (botton_Temp == 1 && Temp > Temp_Max) {//////////////////////////////// Temp
    Blynk.virtualWrite(V38, 255);
    digitalWrite(12, HIGH);
  } else if (botton_Temp == 1 && Temp < Temp_Min) {
    Blynk.virtualWrite(V38, 0);
    digitalWrite(12, LOW);
  }

  if (botton_Hum == 1 && Hum > Hum_Max) {//////////////////////////////// Hum
    Blynk.virtualWrite(V39, 255);
    digitalWrite(14, HIGH);
  } else if (botton_Hum == 1 && Hum < Hum_Min) {
    Blynk.virtualWrite(V39, 0);
    digitalWrite(14, LOW);
  }

  if (botton_Light == 1 && Light > Light_Set) {//////////////////////////////// Light
    Blynk.virtualWrite(V40, 0);
    digitalWrite(27, LOW);
  } else if (botton_Light == 1)     {
    Blynk.virtualWrite(V40, 255);
    digitalWrite(27, HIGH);
  }

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    Blynk.virtualWrite(V5, 0);
    Blynk.virtualWrite(V6, 0);
    return;
  }

  Blynk.virtualWrite(V5, h);
  Blynk.virtualWrite(V6, t);

  OLED.clearDisplay();
  OLED.setTextColor(WHITE, BLACK);
  OLED.setCursor(0, 0);
  OLED.setTextSize(1);
  OLED.println("Temp :");
  OLED.setCursor(0, 18);
  OLED.setTextSize(1);
  OLED.println(" Hum :");
  OLED.setCursor(47, 0);
  OLED.setTextSize(2);
  OLED.print(t);
  OLED.setTextSize(1);
  OLED.print(" *C");
  OLED.setCursor(47, 18);
  OLED.setTextSize(2);
  OLED.print(h);
  OLED.setTextSize(1);
  OLED.print("  %");
  OLED.display();

}
