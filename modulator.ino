#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1331.h>
#include <SPI.h>

#define SerialDebugging true

// The SSD1331 is connected like this (plus VCC plus GND)
const uint8_t   OLED_pin_scl_sck        = 7;
const uint8_t   OLED_pin_sda_mosi       = 11;
const uint8_t   OLED_pin_cs_ss          = 10;
const uint8_t   OLED_pin_res_rst        = 9;
const uint8_t   OLED_pin_dc_rs          = 8;

// SSD1331 color definitions
const uint16_t  OLED_Color_Black        = 0x0000;
const uint16_t  OLED_Color_Blue         = 0x001F;
const uint16_t  OLED_Color_Red          = 0xF800;
const uint16_t  OLED_Color_Green        = 0x07E0;
const uint16_t  OLED_Color_Cyan         = 0x07FF;
const uint16_t  OLED_Color_Magenta      = 0xF81F;
const uint16_t  OLED_Color_Yellow       = 0xFFE0;
const uint16_t  OLED_Color_White        = 0xFFFF;

// The colors we actually want to use
uint16_t        OLED_Frequency_Text_Color = 0xFFFF;
uint16_t        OLED_Duration_Text_Color  = 0x07FF;
uint16_t        OLED_Backround_Color      = 0x0307;

// declare the display
Adafruit_SSD1331 oled =
    Adafruit_SSD1331(
        OLED_pin_cs_ss,
        OLED_pin_dc_rs,
        OLED_pin_sda_mosi,
        OLED_pin_scl_sck,
        OLED_pin_res_rst
     );


void setup() {
    // initialise the SSD1331
    oled.begin();
    oled.setFont();
    oled.fillScreen(OLED_Backround_Color);
    oled.setTextSize(2);
}


void displayFrequency(uint16_t freq, uint8_t power) {
  char freq_buff[7];
  char power_buff[8];
  
  sprintf(freq_buff, "%05d", freq);
  sprintf(power_buff, "%03d%s", power,"%");
  
  oled.fillScreen(OLED_Backround_Color);
  oled.setCursor(20,10);
  oled.setTextColor(OLED_Frequency_Text_Color);
  oled.print(freq_buff);

  oled.setCursor(25,30);
  
  oled.setTextColor(OLED_Duration_Text_Color);
  
  oled.print(power_buff);

}

uint16_t counter = 0;
uint16_t power = 0;

void loop() {
  
  counter++;
  power++;
  
  if (counter>32768) {
    counter=0;
  }

  if (power>999) {
    power=0;
  }
  
    displayFrequency(counter,power);

    // no need to be in too much of a hurry
    delay(10);
   
}
