//
// Arrenged by Tomoaki Yamaguchi
//
unsigned long beforetime = 0L;
float bme_temp = 0;
float bme_humi = 0;
float bme_press = 0;


void setup() {
  Serial.begin(9600); 
  Serial.println("LoRa TEMP/HUMI/PRESS Send for KiwiTech");

  // ここでLoRaWANの初期設定
}

#define LoRa_SEND_INTERVAL  5000    // LoRa送信間隔 (ミリ秒)

void loop() {
  if (millis() - beforetime > LoRa_SEND_INTERVAL) {

    sendTemp();   // ここでLoRaWANでデータを送る

    beforetime = millis();
  }
}


void sendTemp()
{
  Serial.print("Temperature: ");
  Serial.print(bme_temp, 2);
  Serial.println(" degrees C");

  Serial.print("%RH: ");
  Serial.print(bme_humi, 2);
  Serial.println(" %");

  Serial.print("Pressure: ");
  Serial.print(bme_press, 2);
  Serial.println(" Pa");
  
  Serial.println();
  Serial.println("LoRaWAN　Data 送信 ...");
  Serial.println();
}

