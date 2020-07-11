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

#define Version "1.1"
#include "Config.h"
#include "Setup.h"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#ifdef MODE_AP
IPAddress ip(192, 168, 88, 1);          // SynScan default
IPAddress netmask(255, 255, 255, 0);
const char *ssid = AP_SSID;
const char *pass = AP_PASS;
#endif

#ifdef MODE_STA
// For STATION mode:
const char *ssid = STA_SSID;            // Your ROUTER SSID
const char *pass = STA_PASS;            // and WiFi PASSWORD
#endif

#ifdef PROTOCOL_UDP
const int port = UDP_PORT;
#include <WiFiUdp.h>
WiFiUDP udp;
IPAddress remoteIP;
uint16_t packetSize = 0;
#endif

#ifdef PROTOCOL_TCP
#include <WiFiClient.h>
const int port = TCP_PORT;
WiFiServer server(port);
WiFiClient client;
#endif

/**************************************************************
 *  Data Buffers
*/
typedef struct{
  uint8_t text[PKT_BUFFERSIZE];
  uint16_t len;
} DataStruct;

DataStruct txWiFi;
DataStruct rxWiFi;

DataStruct rxUART;
DataStruct txUART;

uint8_t eresp[8] = {'=','0','5','0','2','0','9',0x0d};
uint8_t qresp[8] = {'=','0','0','3','0','0','8',0x0d};

void setup() {
  delay(500);
  Serial.begin(UART_BAUD);

  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.persistent(false);

  #ifdef MODE_AP 
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, pass);              // configure ssid and password for softAP
  delay(2000);                          // avoid Guru Panic for later UDP request
  WiFi.softAPConfig(ip, ip, netmask);   // configure ip address for softAP 
  #endif
  
  #ifdef MODE_STA   // STATION mode (ESP connects to router and gets an IP)
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
  #ifdef DEBUG_ON
    Serial.println(".");
  #endif
    delay(100);
    }
  #endif

  #ifdef PROTOCOL_TCP
    #ifdef DEBUG_ON
    Serial.println("Starting TCP Server");
    #endif
  server.begin(); // start TCP server 
  #endif

  #ifdef PROTOCOL_UDP
    #ifdef DEBUG_ON
    Serial.println("Starting UDP Server");
    #endif
  udp.begin(port); // start UDP server 
  #endif

  #ifdef DEBUG_ON
  Serial.print("Starting MmNS .");
  #endif
  while (!MDNS.begin("SynScan")) {        // Can use SynScan.local
    #ifdef DEBUG_ON
    Serial.print(".");
    #endif
    delay(1000);
  }
  #ifdef DEBUG_ON
  Serial.println("");  Serial.println("mDNS Responder started");
  #endif
  MDNS.addService("_osc", "_udp", port);
  
  #ifdef DEBUG_ON
    Serial.println("");
    Serial.print("WiFi connected ");
    Serial.print("IP address: ");
    #ifdef MODE_AP 
    Serial.println(WiFi.softAPIP());
    #endif
    #ifdef MODE_STA 
    Serial.println(WiFi.localIP());
    #endif
  #endif
  Serial.println("Kx");  // SynScan ECHO
  txUART.len = 0;
}

void loop() {
// wait for WiFi data
  #ifdef PROTOCOL_TCP
  if(!client.connected()) {       // if client not connected
    client = server.available();  //  wait for it to connect
    return;
  }
  if(client.available()) {
    while(client.available()) {
      txUART.text[txUART.len] = (uint8_t)client.read();
      if(txUART.len<(PKT_BUFFERSIZE-1)) txUART.len+=1;
    }
  }
  rxWiFi.len = txUART.len;
  #endif

  #ifdef PROTOCOL_UDP
  // if there’s data available, read a packet
  rxWiFi.len = udp.parsePacket();
  if(rxWiFi.len>0) {
    remoteIP = udp.remoteIP(); // store the ip of the remote device
    txUART.len = udp.read(txUART.text, PKT_BUFFERSIZE);
  }
  #endif

// send to UART
  if(rxWiFi.len>0) {
    Serial.write((char*)txUART.text, txUART.len);
    txUART.len = 0;

// read UART response
    #ifndef DEBUG_ON
    if(Serial.available()) {
      while(1) {
        if(Serial.available()) {
          rxUART.text[rxUART.len] = (char)Serial.read(); // read char from UART
          if(rxUART.len<(PKT_BUFFERSIZE-1)) rxUART.len+=1;
        } else {
          delay(UART_TIMEOUT);
          if(!Serial.available()) {
            break;
          }
        }
      }
    }
    #endif
// mock UART response
    #ifdef DEBUG_ON
    if (txUART.text[0] == ':'){
      switch (txUART.text[1]) {
        case 'e':
          rxUART.len = 8;
          memcpy(rxUART.text, eresp, rxUART.len);
          break;
        case 'q':
          rxUART.len = 8;
          memcpy(rxUART.text, qresp, rxUART.len);
          break;
        default:
          rxUART.len = 0;
          break;     
      }
    }
    #endif
    Serial.write((char*)rxUART.text, rxUART.len);
// send to WiFi:
    #ifdef PROTOCOL_TCP    
      client.write((char*)rxUART.text, rxUART.len);
      rxUART.len = 0;
    #endif

    #ifdef PROTOCOL_UDP
      udp.beginPacket(remoteIP, port); // remote IP and port
      udp.write(rxUART.text, rxUART.len);
      udp.endPacket();
      rxUART.len = 0;
    #endif
  }
  rxWiFi.len = 0;
}
