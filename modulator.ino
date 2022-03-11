#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1331.h>
#include <SPI.h>

// The SSD1331 is connected like this (plus VCC plus GND)
const uint8_t   OLED_pin_scl_sck        = 7;
const uint8_t   OLED_pin_sda_mosi       = 11;
const uint8_t   OLED_pin_cs_ss          = 10;
const uint8_t   OLED_pin_res_rst        = 9;
const uint8_t   OLED_pin_dc_rs          = 8;

const uint8_t   OLED_max_fps            = 5;
const uint32_t  OLED_adjust_delay       = 1000 * 1000 / OLED_max_fps;


// Jack1 (in) pins
const uint8_t   JACK1_R                 = 4;
const uint8_t   JACK1_L                 = 22;
const uint8_t   JACK1_M                 = 23;

// Jack2 (out) pins
const uint8_t   JACK2_R                 = 6;
const uint8_t   JACK2_L                 = 3;
const uint8_t   JACK2_M                 = 5;

// Sensors pins
const uint8_t   SENSOR1_pin             = 19;
const uint8_t   SENSOR2_pin             = 20;
const uint8_t   SENSOR3_pin             = 21;

// Interval in microseconds to adjust frequency
const uint32_t  AdjustFrequencyIntervalMic = 1000;
const float     AdjustFrequencyDelay       = 5;

// Interval in microseconds to adjust sensors
const uint32_t  AdjustSensorsIntervalMic   = 1000;
const float     AdjustSensorsDelay         = 20;

// Loops per second measure
const uint32_t  AdjustLoopSpeedMic         = 1000*1000; // 1 sec

#define         M_PI                       3.14159265358979323846


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
    oled.begin();
    oled.setFont();
    oled.fillScreen(OLED_Backround_Color);
    oled.setTextSize(1);
    pinMode(13, OUTPUT);

    pinMode(JACK1_R, OUTPUT);
    pinMode(JACK1_L, OUTPUT);
    pinMode(JACK1_M, OUTPUT);
    pinMode(JACK2_R, OUTPUT);
    pinMode(JACK2_L, OUTPUT);
    pinMode(JACK2_M, OUTPUT);
    
    analogWriteResolution(10); // 0 - 1024

    digitalWrite(13, HIGH);
}

uint32_t _freq = -1;
uint32_t _freq_carrier = -1;
uint32_t _amp = -1;

float     frequency = -1;
float    _frequency = -1;

uint32_t _adjust = micros();

uint32_t _adjustSensorsIntervalMic = micros();

float _sensor1 = 1;
float _sensor2 = 1;
float _sensor3 = 1;

uint16_t sensor1 = 1;
uint16_t sensor2 = 1;
uint16_t sensor3 = 1;

uint32_t lpsCounter = 0;
uint32_t lps = 0;
uint32_t _lps = 0;

uint32_t _adjustLoopSpeedMic = micros();
uint32_t _OLED_adjust_delay = micros();




void loop() {

  uint32_t adjust = micros();

  if (adjust > _adjustLoopSpeedMic + AdjustLoopSpeedMic) {
    _adjustLoopSpeedMic = adjust;
    lps = lpsCounter;
    lpsCounter = 0;
  }
  lpsCounter++;
  

  int s1 = analogRead(SENSOR1_pin);
  int s2 = analogRead(SENSOR2_pin);
  int s3 = analogRead(SENSOR3_pin);
    
  _sensor1 = ((float)s1 + AdjustSensorsDelay * _sensor1) / (AdjustSensorsDelay + 1);
  _sensor2 = ((float)s2 + AdjustSensorsDelay * _sensor2) / (AdjustSensorsDelay + 1);
  _sensor3 = ((float)s3 + AdjustSensorsDelay * _sensor3) / (AdjustSensorsDelay + 1);

  if (adjust > _adjustSensorsIntervalMic + AdjustSensorsIntervalMic) {
    _adjustSensorsIntervalMic = adjust;
    sensor1 = (uint16_t)_sensor1;
    sensor2 = (uint16_t)_sensor2;
    sensor3 = (uint16_t)_sensor3;
  }
  
  
  uint32_t freq = (uint32_t)((sensor2-1)/8) * 256 + sensor1 / 4;
  uint32_t amp = sensor3 * 100 / 1023;

  // adjust output frequency to selected
  if (adjust > _adjust + AdjustFrequencyIntervalMic) {
    _adjust = adjust;

    frequency = ((float)freq + AdjustFrequencyDelay * frequency) / (AdjustFrequencyDelay + 1);

    // if frequency are equal to selected do not set it again to exclude additional noise
    if ((int)frequency != (int)_frequency) {
      _frequency = frequency;
      
      analogWriteFrequency(JACK1_R, frequency);
      analogWriteFrequency(JACK1_L, frequency);
      analogWriteFrequency(JACK1_M, frequency);

      analogWriteFrequency(JACK2_R, frequency);
      analogWriteFrequency(JACK2_L, frequency);
      analogWriteFrequency(JACK2_M, frequency);
    }
  }
  

  //analogWrite(JACK1_L, sensor3);
  //analogWrite(JACK1_R, sensor3);
  
  analogWrite(JACK2_L, sensor3);
  analogWrite(JACK2_R, sensor3);

  // do not update display more often than OLED_max_fps
  if (adjust > _OLED_adjust_delay + OLED_adjust_delay) {
    _OLED_adjust_delay = adjust;

    // show frequency info
    if ((int)_freq != (int)frequency) {
      _freq = (int)frequency;

      char full_buff[20];
      sprintf(full_buff, "\xDA\xDA\xDA\xDA\xDA\xDA\xDA\xDA\xDA\xDA\xDA\xDA\xDA\xDA\xDA\xDA");
  
      char freq_buff[20];
      sprintf(freq_buff, "freq:%9d%s", (int)_freq,"Hz");

      oled.setCursor(0,44);
      oled.setTextColor(OLED_Backround_Color);
      oled.print(full_buff);
    
      oled.setCursor(0,44);
      oled.setTextColor(OLED_Frequency_Text_Color);
      oled.print(freq_buff);
    }

    // show amplitude intro
    if (_amp != amp) {
      _amp = amp;

      char full_buff[10];
      sprintf(full_buff, "\xDA\xDA\xDA\xDA");
    
      char amp_buff[10];
      sprintf(amp_buff, "%03d%s", (int)amp,"%");
  
      oled.setCursor(25,30);
      oled.setTextColor(OLED_Backround_Color);
      oled.print(full_buff);

      oled.setCursor(25,30);
      oled.setTextColor(OLED_Duration_Text_Color);
      oled.print(amp_buff);
    }

    // show lps (loops per second) info 
    if (_lps != lps) {
      _lps = lps;
    
      char full_buff[20];
      sprintf(full_buff, "\xDA\xDA\xDA\xDA\xDA\xDA\xDA\xDA\xDA\xDA\xDA\xDA\xDA\xDA\xDA\xDA");
  
      char freq_buff[20];
      sprintf(freq_buff, "lps:%12lu", _lps);

      oled.setCursor(0,54);
      oled.setTextColor(OLED_Backround_Color);
      oled.print(full_buff);
    
      oled.setCursor(0,54);
      oled.setTextColor(OLED_Frequency_Text_Color);
      oled.print(freq_buff);
    }
  }
}
