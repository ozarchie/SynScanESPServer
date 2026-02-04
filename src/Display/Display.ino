/***************************************************************************
  This is a library for the BMP280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BMP280 Breakout
  ----> http://www.adafruit.com/products/2651

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  MIT license, see LICENSE.txt for more information
 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include "U8glib.h"

#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)

Adafruit_BMP280 bmp; // I2C
//Adafruit_BMP280 bmp(BMP_CS); // hardware SPI
//Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

// OLED
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);

char sT[20];
char sP[9];
char sA[9];
char sA_MIN[9];
char sA_MAX[9];
double A_MIN = 0;
double A_MAX = 0;

void draw(double T, double P, double A) {
  u8g.setFont(u8g_font_unifont);

  dtostrf(T, 4, 2, sT);
  dtostrf(P, 4, 2, sP);
  dtostrf(A, 4, 2, sA);

  u8g.drawStr( 5, 10, "Temp: ");
  u8g.drawStr( 5, 30, "Bar : ");
  u8g.drawStr( 5, 50, "Alt : ");
  u8g.drawStr( 50, 10, sT);
  u8g.drawStr( 50, 30, sP);
  u8g.drawStr( 50, 50, sA);
}

void draw2(double A_MIN, double A_MAX) {
  u8g.setFont(u8g_font_unifont);

  dtostrf(A_MIN, 4, 2, sA_MIN);
  dtostrf(A_MAX, 4, 2, sA_MAX);
  u8g.drawStr( 5, 20, "A Min: ");
  u8g.drawStr( 60, 20, sA_MIN);
  u8g.drawStr( 5, 45, "A Max: ");
  u8g.drawStr( 60, 45, sA_MAX);
}

void setup() {
  Serial.begin(9600);
  while ( !Serial ) delay(100);   // wait for native usb
  Serial.println(F("BMP280 test"));
  unsigned status;
  //status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);
  status = bmp.begin();
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  u8g.setColorIndex(1);
  u8g.setFont(u8g_font_unifont);

}

void loop() {

    float T, P, A;

    T = bmp.readTemperature();
    P = bmp.readPressure();
    A = bmp.readAltitude(1013.25); // Adjusted to local sealevel

    Serial.print(F("T: ")); Serial.print(T); Serial.print(" *C, ");
    Serial.print(F("P: ")); Serial.print(P); Serial.print(" Pa, ");
    Serial.print(F("A: ")); Serial.print(A); Serial.println(" m");

      if ( A > A_MAX) {
        A_MAX = A;
      }

      if ( A < A_MIN || A_MIN == 0) {
        A_MIN = A;
      }

      u8g.firstPage();
      do {
        draw(T, P, A);
      } while ( u8g.nextPage() );
      u8g.firstPage();
      delay(1000);

      do {
        draw2(A_MIN, A_MAX);
      } while ( u8g.nextPage() );
      u8g.firstPage();

      delay(1000);

    Serial.println();
    delay(2000);
}

