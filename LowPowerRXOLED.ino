#include <Arduino.h>
#include "LowPower.h"
#include <U8g2lib.h>
#include <Wire.h>
#include <SPI.h>
#include <RH_RF95.h>
const int RFM_RST_PIN = 4;
const int RFM_INT_PIN = 3;
const int RFM_CS_PIN = 10;
const int wakeUpPin = 3;

RH_RF95 rf95(RFM_CS_PIN,RFM_INT_PIN);
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

void wakeUp() {
}

void setup()  {
  Serial.begin(9600);
  pinMode(RFM_RST_PIN, OUTPUT); // Refresh LoRa
  pinMode(wakeUpPin, INPUT);   
  u8x8.begin(); // Initialize the 8x8 screen
  u8x8.setFont(u8x8_font_chroma48medium8_r);//Set a font
  
  digitalWrite(RFM_RST_PIN, HIGH);
  delay(100);
  digitalWrite(RFM_RST_PIN, LOW);
  delay(25);
  digitalWrite(RFM_RST_PIN, HIGH);
  delay(25);
  
  while (!Serial) ; // Wait for serial port to be available
  if ( !rf95.init() ) {
    Serial.println("Could not initialize RFM95");
    while(1);
  }
  Serial.println("RFM95 initialized");
  u8x8.setCursor(0,1);
  u8x8.print("RFM95 init OK");
  delay(2000);
  u8x8.clear(); 
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
}
void loop() {
  
  attachInterrupt(digitalPinToInterrupt(wakeUpPin),wakeUp,HIGH); // Give an interrupt to pin 3 on ATMega328p
  rf95.setModeRx(); // It says that .available() sets the mode to idle after receiving so resetting it to Rx can't hurt I guess
  delay(50);
  u8x8.setCursor(0,0);
  u8x8.print("Off in 2s...");
  delay(2000);
  u8x8.setPowerSave(1); // Shut down the screen to save power
  delay(200); // Wait a bit before sleeping the chip
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
  // After waking up (ONLY WAKES UP ONCE AND DOESN'T READ THE MESSAGE)
  detachInterrupt(digitalPinToInterrupt(wakeUpPin)); // After waking up, detach the interrupt on pin 3
  u8x8.setPowerSave(0); // All of this writing to the screen works until if(rf95.available()) -> it reads nothing from the message
  delay(10);
  u8x8.clear();
  u8x8.setCursor(1,0);
  u8x8.print("Just woke up!");
  u8x8.setCursor(1,1);
  u8x8.print("What's new?");
  delay(2000);
  u8x8.clear();
  if (rf95.available()) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    //uint8_t buf[251];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len)) {
    u8x8.setInverseFont(0);
    u8x8.setCursor(1,0);
    u8x8.print((char*)buf);
    delay(2000);
    u8x8.setCursor(1,2);
    u8x8.print("Job done.");
    delay(1000);
    u8x8.setCursor(0,4);
    u8x8.setInverseFont(1);
    u8x8.print("Back to bed...");
    delay(1000);
    //Serial.print("RSSI: ");
    //Serial.println(rf95.lastRssi(), DEC);
    }
    else  {
      Serial.println("recv failed");
      u8x8.print("recv failed");
      delay(2000);
    }
  }
}
