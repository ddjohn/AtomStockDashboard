#include <Arduino.h>
#include <M5Atom.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "config.h"

#define MATRIX_SIZE 5
#define YAHOO_URL "https://query1.finance.yahoo.com/v8/finance/chart/"
#define YAHOO_USERAGENT "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/105.0.0.0 Safari/537.36"

String stocks[] = {
  "ABB.ST"   , "ALFA.ST"  , "ASSA-B.ST"  , "ATCO-A.ST", "AZN.ST"   ,
  "BOL.ST"   , "ERIC-B.ST", "ESSITY-B.ST", "EVO.ST"   , "HEXA-B.ST",
  "HM-B.ST"  , "INVE-B.ST", "NDA-SE.ST"  , "SAND.ST"  , "SCA-B.ST" ,
  "SEB-A.ST" , "SHB-A.ST" , "SINCH.ST"   , "SKA-B.ST" , "SKF-B.ST" ,
  "SWED-A.ST", "SWMA.ST"  , "TEL2-B.ST"  , "TELIA.ST" , "VOLV-B.ST",
};  

uint8_t buffer[2 + MATRIX_SIZE * MATRIX_SIZE * 3];
HTTPClient http;

void setup() {
  M5.begin(true, false, true);
  Serial.println("setup()");

  buffer[0] = MATRIX_SIZE;
  buffer[1] = MATRIX_SIZE;

  WiFi.begin(SSID, PASSWORD);  
}

void setLed(int led, int red, int green, int blue) {
  buffer[2 + led * 3 + 0] = red;
  buffer[2 + led * 3 + 1] = green;
  buffer[2 + led * 3 + 2] = blue;
  M5.dis.displaybuff(buffer);    
}

void showOK() {
  for(int i = 0; i < MATRIX_SIZE*MATRIX_SIZE; i++) {
    setLed(i, 0, 0, 0);
  }  
}

void showNOK() {
   for(int i = 0; i < MATRIX_SIZE*MATRIX_SIZE; i++) {
    setLed(i, random(1, 128), random(1, 128), random(1, 128));
  }
}

float pct(String stock) {
    http.begin(YAHOO_URL + stock + "?interval=1d&range=1d");
    http.setUserAgent(YAHOO_USERAGENT);
    http.setReuse(true);
        
    Serial.println(http.GET());

    String payload = http.getString();
    Serial.println(payload);

    StaticJsonDocument<2000> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if(error) {
      Serial.println(error.f_str());
    } else {
      Serial.println("ok");
    }

    float price = doc["chart"]["result"][0]["meta"]["regularMarketPrice"]; //.as<char*>();
    float closed = doc["chart"]["result"][0]["meta"]["chartPreviousClose"]; //.as<char*>();
    float pct = (price-closed)/closed;
        
    http.end();

    return pct*100;
}

void loop() {
  Serial.println("loop()");

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  if(WiFi.status() == WL_CONNECTED) {
    //showOK();          

    for (int i = 0; i < sizeof stocks/sizeof stocks[0]; i++) {
      float percentage = pct(stocks[i]);
      
     if(percentage > 0) {
        // Green
        float value = 100.0 * percentage > 192 ? 192 : 100 * percentage;
        setLed(i, 0, value, 0);        
      } 
      else if(percentage < 0) {
        // Red              
        float value = -100.0 * percentage > 192 ? 192 : - 100 * percentage;
        setLed(i, value, 0, 0);
      }
      else {
        // Blue
        setLed(i, 0, 0, 128);        
      }
      Serial.println(percentage);
       delay(200);      
    }
    delay(60000);
  } 
  else {
    showNOK();
    delay(100);
  }
}
