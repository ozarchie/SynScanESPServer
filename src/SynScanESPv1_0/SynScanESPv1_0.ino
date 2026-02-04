/* 
   SynScanESPx1_0.ino

   SynScan ESP Server

   Copyright (c) 2018 Roman Hujer   http://hujer.net

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,ss
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

  Description:
   SynScanESPServer 
   
   Wiring and PCB on  https://easyeda.com/hujer.roman/wifi-for-synscan

*/

#define Version "1.0"
#include "Config.h"
#include "Setup.h"


const int port = tcp_port; 


#include <ESP8266WiFi.h>
#include <WiFiClient.h>




WiFiServer server(port);
WiFiClient client;

uint8_t buf1[bufferSize];
uint8_t i1=0;

uint8_t buf2[bufferSize];
uint8_t i2=0;


#ifdef MODE_AP
// For AP mode:

const char *ssid =  MY_SSID;
const char *pw =   MY_PASS; 

IPAddress ip(10, 0, 0, 1); 
IPAddress netmask(255, 255, 255, 0);

#endif


#ifdef MODE_STA
// For STATION mode:
const char *ssid_c = MY_C_SSID;  
const char *pw_c = MY_C_PASS; 

IPAddress wifi_sta_ip = IPAddress(10,0,0,10);
IPAddress wifi_sta_gw = IPAddress(10,0,0,1);
IPAddress wifi_sta_sn = IPAddress(255,255,255,0);


#endif




void setup() {

  delay(500);
  
  Serial.begin(UART_BAUD);

  #ifdef MODE_AP 

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ip, ip, netmask); // configure ip address for softAP 
  WiFi.softAP(ssid, pw); // configure ssid and password for softAP
  #endif

  
  #ifdef MODE_STA
  // STATION mode (ESP connects to router and gets an IP)


  WiFi.mode(WIFI_STA);
  WiFi.config(wifi_sta_ip, wifi_sta_gw, wifi_sta_sn);
  WiFi.begin(ssid_c, pw_c);
  while (WiFi.status() != WL_CONNECTED) {
  #ifdef DEBUG_ON
    Serial.println(".");
  #endif
    delay(100);
    }
  #endif
  #ifdef DEBUG_ON
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  #endif

  Serial.println("Kx");  // SynScan ECHO
  server.begin(); // start TCP server 
  
}

void loop() {

  if(!client.connected()) { // if client not connected
    client = server.available(); // wait for it to connect
    return;
  }

  // here we have a connected client

  if(client.available()) {
    while(client.available()) {
      buf1[i1] = (uint8_t)client.read(); 
      if(i1<bufferSize-1) i1++;
    }
    // now send to UART:
    Serial.write(buf1, i1);
    i1 = 0;
  }

  if(Serial.available()) {

    // read the data until pause:
    
    while(1) {
      if(Serial.available()) {
        buf2[i2] = (char)Serial.read(); // read char from UART
        if(i2<bufferSize-1) i2++;
      } else {
        //delayMicroseconds(packTimeoutMicros);
        delay(packTimeout);
        if(!Serial.available()) {
          break;
        }
      }
    }
    
    // now send to WiFi:
    client.write((char*)buf2, i2);
    i2 = 0;
  }
  
}
