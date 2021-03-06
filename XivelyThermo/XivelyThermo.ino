/*************************************************** 
  This is a sketch to use the CC3000 WiFi chip & Xively
  
  Written by Marco Schwartz for Open Home Automation
 ****************************************************/

// Libraries
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"
#include "Adafruit_MCP9808.h"
#include<stdlib.h>
#include <Wire.h>

// Define CC3000 chip pins
#define ADAFRUIT_CC3000_IRQ   3
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10



// Create CC3000 instances
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIV2); // you can change this clock speed
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();
                                         


// WLAN parameters
#define WLAN_SSID       "PlayThatFunkyMusicWifi"
#define WLAN_PASS       "whiteboy"
#define WLAN_SECURITY   WLAN_SEC_WPA2

// Xively parameters
#define WEBSITE  "https://api.xively.com"
#define API_key  "MjERFDCfY9FKWECyq8AVj1ChCOBCNPE6i1bxb9F24xEZbLZT"
#define feedID  "1825093998"

uint32_t ip;

void setup(void)
{
  // Initialize
  Serial.begin(115200);
  if (!tempsensor.begin()) {
    Serial.println("Couldn't find MCP9808!");
    while (1);
  }
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }
}

void loop(void)
{
  // Connect to WiFi network
  cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY);
  Serial.println(F("Connected!"));
  
  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100);
  }  

  // Set the website IP
  uint32_t ip = cc3000.IP2U32(216,52,233,120);
  cc3000.printIPdotsRev(ip);
  
  // Get data & transform to integers
  char temperature [6];
  float t = tempsensor.readTempC();
  // translate float to string
  dtostrf (t,5,2,temperature);
  

  
  // Prepare JSON for Xively & get length
  int length = 0;

  String data = "";
  data = data + "\n" + "{\"version\":\"1.0.0\",\"datastreams\" : [ {\"id\" : \"Temperature\",\"current_value\" : \"" + String(temperature) + "\"}]}";
  
  length = data.length();
  Serial.print("Data length");
  Serial.println(length);
  Serial.println();
  
  // Print request for debug purposes
  Serial.print("PUT /v2/feeds/");
  Serial.print(feedID);
  Serial.println(".json HTTP/1.1");
  Serial.println("Host: api.xively.com");
  Serial.print("X-ApiKey: ");
  Serial.println(API_key);
  Serial.print("Content-Length: ");
  Serial.println(length, DEC);
  Serial.print("Connection: close");
  Serial.println();
  Serial.print(data);
  Serial.println();
  
  // Send request
  Adafruit_CC3000_Client client = cc3000.connectTCP(ip, 80);
  if (client.connected()) {
    Serial.println("Connected!");
    client.println("PUT /v2/feeds/" + String(feedID) + ".json HTTP/1.0");
    client.println("Host: api.xively.com");
    client.println("X-ApiKey: " + String(API_key));
    client.println("Content-Length: " + String(length));
    client.print("Connection: close");
    client.println();
    client.print(data);
    client.println();
  } else {
    Serial.println(F("Connection failed"));    
    return;
  }
  
  Serial.println(F("-------------------------------------"));
  while (client.connected()) {
  while (client.available()) {
  char c = client.read();
  Serial.print(c);
    }
  }
  client.close();
  Serial.println(F("-------------------------------------"));
  
  Serial.println(F("\n\nDisconnecting"));
  cc3000.disconnect();
  
  // Wait 10 seconds until next update
  delay(10000);
  
}
