/*
  Read an 8x8 array of distances from the VL53L5CX
  By: Nathan Seidle
  SparkFun Electronics
  Date: October 26, 2021
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  This example shows how to setup the I2C bus to minimize the amount
  of time taken to init the sensor.

  At each power on reset, a staggering 86,000 bytes of firmware have to be sent to the sensor.
  At 100kHz, this can take ~9.4s. By increasing the clock speed, we can cut this time down to ~1.4s.

  Two parameters can be tweaked:

    Clock speed: The VL53L5CX has a max bus speed of 1MHz.

    Max transfer size: The majority of Arduino platforms default to 32 bytes. If you are using one
    with a larger buffer (ESP32 is 128 bytes for example), this can help decrease transfer times a bit.

  Measurements:
    Default 100kHz clock and 32 byte transfer: 9.4s
    400kHz, 32 byte transfer: 2.8s
    400kHz, 128 byte transfer: 2.5s
    1MHz, 32 byte transfer: 1.65s
    1MHz, 128 byte transfer: 1.4s

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/18642

*/
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include "secrets.h"

#include <SparkFun_VL53L5CX_Library.h> //http://librarymanager/All#SparkFun_VL53L5CX

SparkFun_VL53L5CX myImager;
VL53L5CX_ResultsData measurementData; // Result data class structure, 1356 byes of RAM



int imageResolution = 0; //Used to pretty print output
int imageWidth = 0; //Used to pretty print output

bool led_state = false;



const uint16_t port = 10000;
const char * host = "172.21.72.142";

// wifi client
WiFiClient client;

char buffer[10];

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(250000);
  Serial.println("VL53L5CX Example 1");
  yield();
  delay(1000);
  

  // vl53l5cx

  Serial.println("SparkFun VL53L5CX Imager Example");
  Wire.begin(); //This resets I2C bus to 100kHz
  Wire.setClock(1200000); //Sensor has max I2C freq of 1MHz

  // yield();
  // myImager.setWireMaxPacketSize(128); //Increase default from 32 bytes to 128 - not supported on all platforms
  // yield();

  Serial.println("Initializing sensor board. This can take up to 10s. Please wait.");

  //Time how long it takes to transfer firmware to sensor
  long startTime = millis();
  bool startup = myImager.begin();
  long stopTime = millis();

  if (startup == false)
  {
    Serial.println(F("Sensor not found - check your wiring. Freezing"));
    while (1) ;
  }

  Serial.print("Firmware transfer time: ");
  float timeTaken = (stopTime - startTime) / 1000.0;
  Serial.print(timeTaken, 3);
  Serial.println("s");

  myImager.setResolution(8*8); //Enable all 64 pads

  imageResolution = myImager.getResolution(); //Query sensor for current resolution - either 4x4 or 8x8
  imageWidth = sqrt(imageResolution); //Calculate printing width

  myImager.startRanging();

  // wifi
  Serial.println("Connecting");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("...");
    delay(1000);
  }
  Serial.print("WiFi connected with IP: ");
  Serial.println(WiFi.localIP());

  while (!client.connect(host, port)) {
 
    Serial.println("Connection to host failed");

    delay(1000);
  }
}

void loop()
{
  //Poll sensor for new data
  if (myImager.isDataReady() == true)
  {
    long startTime = millis();
    if (myImager.getRangingData(&measurementData)) //Read distance data into array
    {
      // check if clinet is connected
      if(!client.connected()){
        while (!client.connect(host, port)) {
          Serial.println("Connection to host failed");
          delay(1000);
        }
      }
      // char *pos = buffer;
      // for (int i=0;i<64;i++){
      //   pos += sprintf(pos,"%d ",measurementData.distance_mm[i]);
      // }
      // client.println(buffer);
      // client.println((char*)(measurementData.distance_mm));
      for( int i=0;i<imageResolution;i++){
        for(int j=0;j<10;j++){
          buffer[j]='0';
        }
        sprintf(buffer,"%d:%d",i,measurementData.distance_mm[i]);
        client.println(buffer);
      }

      //The ST library returns the data transposed from zone mapping shown in datasheet
      //Pretty-print data with increasing y, decreasing x to reflect reality
      for (int y = 0 ; y <= imageWidth * (imageWidth - 1) ; y += imageWidth)
      {
        for (int x = imageWidth - 1 ; x >= 0 ; x--)
        {
          Serial.print("\t");
          Serial.print(measurementData.distance_mm[x + y]);
        }
        Serial.println();
      }
      Serial.println();
    }
    long stopTime = millis();
    Serial.print("FPS: ");
    float timeTaken = (stopTime - startTime) / 1000.0;
    Serial.println(1/timeTaken, 3);
    // Serial.println("s");
  }
  // client.println("Hello World !");
  

  led_state = !led_state;
  digitalWrite(LED_BUILTIN, led_state);
  delay(200); //Small delay between polling
}