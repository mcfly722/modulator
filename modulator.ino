#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1331.h>
#include <SPI.h>

const byte            PWM1_IN_PIN               = A0;
const byte            PWM2_IN_PIN               = A1;

const byte            PWM1_OUT_PIN              = 6;
const byte            PWM2_OUT_PIN              = 3;

const unsigned long   FREQ_ADJUST_TIMES_PER_SEC = 30;
const unsigned long   FREQ_ADJUST_INTERVAL_MIC  = (1000 * 1000) / FREQ_ADJUST_TIMES_PER_SEC;

const byte            FREQ_HIGH_SELECT_PIN      = 19; 
const byte            FREQ_LOW_SELECT_PIN       = 20;
const unsigned int    FREQ_AVG_SAMPLES          = 15000;
const float           FREQ_MAX_HZ               = 32000;
const byte            MODE_AND_PWM_WIDTH_PIN    = 21;

const unsigned int    PWM_IN_AVG_SAMPLES        = 1500;

const unsigned int    OLED_MAX_FPS              = 10;
const unsigned int    OLED_WIDTH                = 16;
const unsigned int    OLED_HEIGHT               = 8;
const unsigned long   OLED_NEXT_CHAR_DELAY      = (1000 * 1000 / OLED_MAX_FPS) / (OLED_WIDTH * OLED_HEIGHT);

const unsigned int    OLED_TEXT_COLOR           = 0xFFFF;
const unsigned int    OLED_BACKGROUND_COLOR     = 0x0307;

const byte            OLED_PIN_scl_sck          = 7;
const byte            OLED_PIN_sda_mosi         = 11;
const byte            OLED_PIN_cs_ss            = 10;
const byte            OLED_PIN_res_rst          = 9;
const byte            OLED_PIN_dc_rs            = 8;

// declare the display
Adafruit_SSD1331 oled = Adafruit_SSD1331(
  OLED_PIN_cs_ss,
  OLED_PIN_dc_rs,
  OLED_PIN_sda_mosi,
  OLED_PIN_scl_sck,
  OLED_PIN_res_rst
);

unsigned long current_time = micros();

unsigned long oled_update_deadline = micros();

unsigned int  pwm1    = 1;
unsigned int  pwm2    = 2;
unsigned int  mode    = 0;
unsigned int  pwmOut  = 300;
unsigned long freq    = 123456;
unsigned long loopsPS = 0;
unsigned long scrMic  = 0;

unsigned long scr_counter = 0;

char oled_current_buffer[OLED_WIDTH * OLED_HEIGHT];
unsigned int  oled_char_counter = 0;

void oled_update(){
  
  if (current_time > oled_update_deadline) {
    oled_update_deadline = current_time + OLED_NEXT_CHAR_DELAY;

    unsigned int current = micros();

    char oled_update_buffer[OLED_WIDTH * OLED_HEIGHT];
    
    const char *modes[3] = { "GENERATION", "PWM1", "PWM2" };
    sprintf(oled_update_buffer, "pwm1:%11upwm2:%11umode:%11spwmOut:%4u=%3lu%sfreq:%9luHzloops/s:%8luscrMic:%9lu%16s", pwm1, pwm2, modes[mode], pwmOut,((unsigned long)pwmOut)*100/1024 ,"%", freq, 1000*(loopsPS/1000), 10000*(scrMic/10000), "");

    unsigned int x = oled_char_counter / OLED_HEIGHT;
    unsigned int y = oled_char_counter % OLED_HEIGHT;
    int c = y * OLED_WIDTH + x;

    if (oled_current_buffer[c] != oled_update_buffer[c]) {
      oled_current_buffer[c] = oled_update_buffer[c];
      
      oled.setCursor(x * 6, y * 8);
      oled.setTextColor(OLED_BACKGROUND_COLOR);
      oled.print("\xDA");

      oled.setCursor(x * 6, y * 8);
      oled.setTextColor(OLED_TEXT_COLOR);
      oled.print(oled_update_buffer[c]);
    }

    scr_counter += micros() - current;

    oled_char_counter++;
    oled_char_counter = oled_char_counter % (OLED_WIDTH * OLED_HEIGHT);
  }
}

unsigned long current_sec_deadline = micros();
unsigned long current_loops_counter = 0;

void calc_loopsPS(){
  if (current_time > current_sec_deadline) {
    loopsPS = current_loops_counter;
    scrMic = scr_counter;
    current_loops_counter = 0;
    scr_counter = 0;
    current_sec_deadline =  current_time + 1000 * 1000;
  } else {
    current_loops_counter++;  
  }
}


float _freq = -1;
unsigned long current_freq_deadline = micros();

void calc_freq(){
  
  unsigned int high = analogRead(FREQ_HIGH_SELECT_PIN) - 1;
  unsigned int low = analogRead(FREQ_LOW_SELECT_PIN) - 1;
  
  float __freq = (((float)(high * 1023 + (float)low)) / 1023) * (FREQ_MAX_HZ  / 1023);
  
  _freq = (__freq + FREQ_AVG_SAMPLES * _freq) / (FREQ_AVG_SAMPLES + 1);

  if (current_time > current_freq_deadline) {
    current_freq_deadline = current_time + FREQ_ADJUST_INTERVAL_MIC;

    if (freq != (unsigned long)_freq){
      freq = (unsigned long)_freq;
      
      analogWriteFrequency(PWM1_OUT_PIN, freq);
      analogWrite(PWM1_OUT_PIN, pwmOut);

      analogWriteFrequency(PWM2_OUT_PIN, freq);
      analogWrite(PWM2_OUT_PIN, pwmOut);
    }
    
  }
  
}

unsigned int _pwmOut = -1;
void calc_mode_and_pwm_width(){
  unsigned int s = analogRead(MODE_AND_PWM_WIDTH_PIN);
  
  // Mode
  if (s >= 0 && s < 341) {
     mode = 1;
     pwmOut = (s - 1) *1024 / 341;
  }
    
  if (s >= 342 && s < 683) {
     mode = 2;
     pwmOut = (684 - s) * 1024 / 341;
  }
    
  if (s >= 683 && s < 1024) {
     mode = 0;
     pwmOut = 4 + (s - 683) * 1024 / 340;
  }

  if (pwmOut > 1024) {
    pwmOut = 1024;
  }

  if (pwmOut != _pwmOut) {
    _pwmOut = pwmOut;
    analogWrite(PWM1_OUT_PIN, pwmOut);
    analogWrite(PWM2_OUT_PIN, pwmOut);
  }

}


float _pwm1 = 0;
float _pwm2 = 0;

void get_pwm_values(){

  _pwm1 = (analogRead(PWM1_IN_PIN) + PWM_IN_AVG_SAMPLES * _pwm1) / (PWM_IN_AVG_SAMPLES + 1);
  _pwm2 = (analogRead(PWM2_IN_PIN) + PWM_IN_AVG_SAMPLES * _pwm2) / (PWM_IN_AVG_SAMPLES + 1);

  pwm1 = _pwm1;
  pwm2 = _pwm2;

}

void setup() {
    oled.begin();
    oled.setFont();
    oled.fillScreen(OLED_BACKGROUND_COLOR);
    oled.setTextSize(1);

    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);

    pinMode(PWM1_IN_PIN, INPUT);
    pinMode(PWM2_IN_PIN, INPUT);
    
    pinMode(PWM1_OUT_PIN, OUTPUT);
    pinMode(PWM2_OUT_PIN, OUTPUT);

    pinMode(MODE_AND_PWM_WIDTH_PIN, INPUT);

    pinMode(FREQ_HIGH_SELECT_PIN, INPUT);
    pinMode(FREQ_LOW_SELECT_PIN, INPUT);

    analogWriteResolution(10); // 0 - 1024
    analogReadResolution(10); // 0 - 1024
}

void loop() {
  current_time = micros();

  calc_freq();

  get_pwm_values();
  
  calc_mode_and_pwm_width();

  oled_update();

  calc_loopsPS();
}
