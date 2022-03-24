#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1331.h>
#include <SPI.h>

const float           FREQ_MAX_HZ                        = 32000;

const unsigned int    PWM_IN_AVG_SAMPLES                 = 1000;

const unsigned long   PWM_OUT_WIDTH_SAMPLES              = 1000;
const unsigned long   PWM_OUT_FREQ_SAMPLES               = 1000;

const unsigned long   PWM_OUT_FREQ_ADJUST_TIMES_PER_SEC  = 30;
const unsigned long   PWM_OUT_FREQ_ADJUST_INTERVAL_MIC   = (1000 * 1000) / PWM_OUT_FREQ_ADJUST_TIMES_PER_SEC;


const unsigned int    OLED_MAX_FPS                       = 10;
const unsigned int    OLED_WIDTH                         = 16;
const unsigned int    OLED_HEIGHT                        = 8;
const unsigned long   OLED_NEXT_CHAR_DELAY               = (1000 * 1000 / OLED_MAX_FPS) / (OLED_WIDTH * OLED_HEIGHT);

const unsigned int    OLED_TEXT_COLOR                    = 0xFFFF;
const unsigned int    OLED_BACKGROUND_COLOR              = 0x0307;


const byte            PWM1_IN_PIN                        = A0;
const byte            PWM2_IN_PIN                        = A1;
const byte            PWM1_OUT_PIN                       = 6;
const byte            PWM2_OUT_PIN                       = 3;

const byte            PWM_OUT_WIDTH_SELECT_PIN           = 21; 
const byte            FREQ_HIGH_SELECT_PIN               = 19; 
const byte            FREQ_LOW_SELECT_PIN                = 20;

const byte            OLED_PIN_scl_sck                   = 7;
const byte            OLED_PIN_sda_mosi                  = 11;
const byte            OLED_PIN_cs_ss                     = 10;
const byte            OLED_PIN_res_rst                   = 9;
const byte            OLED_PIN_dc_rs                     = 8;

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

unsigned int  pwm1             = 1;
unsigned int  pwm2             = 2;
unsigned int  pwm_out_width    = 0;
unsigned int  mode             = 0;
unsigned long pwm1_out_freq    = 1;
unsigned long pwm2_out_freq    = 1;
unsigned long loopsPS          = 0;
unsigned long scrMic           = 0;

unsigned long scr_counter = 0;

unsigned int pwm_out_width_percentages = 1;

char oled_current_buffer[OLED_WIDTH * OLED_HEIGHT];
unsigned int  oled_char_counter = 0;

void oled_update(){
  
  if (current_time > oled_update_deadline) {
    oled_update_deadline = current_time + OLED_NEXT_CHAR_DELAY;

    unsigned int current = micros();

    char oled_update_buffer[OLED_WIDTH * OLED_HEIGHT];
    
    sprintf(oled_update_buffer, "in pwm1:%8uin pwm2:%8uout duty c.:%3u%sout pwm1:%5luHzout pwm2:%5luHzloops/s:%8luscrMic:%9lu%16s", pwm1, pwm2, pwm_out_width_percentages,"%" ,pwm1_out_freq, pwm2_out_freq, 1000*(loopsPS/1000), 10000*(scrMic/10000), "");

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



float _pwm2_out_freq = -1;
unsigned long current_freq2_deadline = micros();

void calc_pwm2_out_freq(){
  unsigned int high = analogRead(FREQ_HIGH_SELECT_PIN) - 1;
  unsigned int low = analogRead(FREQ_LOW_SELECT_PIN) - 1;
  float __freq = (((float)(high * 1023 + (float)low)) / 1023) * (FREQ_MAX_HZ  / 1023);
  _pwm2_out_freq = (__freq + PWM_OUT_WIDTH_SAMPLES * _pwm2_out_freq) / (PWM_OUT_WIDTH_SAMPLES + 1);
  if (current_time > current_freq2_deadline) {
    current_freq2_deadline = current_time + PWM_OUT_FREQ_ADJUST_INTERVAL_MIC;
    if (pwm2_out_freq != (unsigned long)_pwm2_out_freq){
      pwm2_out_freq = (unsigned long)_pwm2_out_freq;
      analogWriteFrequency(PWM2_OUT_PIN, _pwm2_out_freq);
      analogWrite(PWM2_OUT_PIN, pwm_out_width);
    }
  }
}

float _pwm1 = 0;
void get_pwm1_value(){
  _pwm1 = (analogRead(PWM1_IN_PIN) + PWM_IN_AVG_SAMPLES * _pwm1) / (PWM_IN_AVG_SAMPLES + 1);
  pwm1 = _pwm1;
}

float _pwm2 = 0;
void get_pwm2_value(){
  _pwm2 = (analogRead(PWM2_IN_PIN) + PWM_IN_AVG_SAMPLES * _pwm2) / (PWM_IN_AVG_SAMPLES + 1);
  pwm2 = _pwm2;
}



float _pwm1_out_freq = 0;
void calc_pwm1_out_freq(){
  _pwm1_out_freq = FREQ_MAX_HZ * pwm1 / 1024;
  if (pwm1_out_freq != (unsigned long)_pwm1_out_freq){
    pwm1_out_freq = (unsigned long)_pwm1_out_freq;
    analogWriteFrequency(PWM1_OUT_PIN, pwm1_out_freq);
    analogWrite(PWM1_OUT_PIN, pwm_out_width);
  }
}



float _pwm_out_width = 0;
void calc_pwm_out_width(){
  unsigned int w = analogRead(PWM_OUT_WIDTH_SELECT_PIN);

  _pwm_out_width = (w + PWM_OUT_WIDTH_SAMPLES * _pwm_out_width) / (PWM_OUT_WIDTH_SAMPLES + 1);

  pwm_out_width_percentages = (_pwm_out_width * 100) / 1022;

  if (pwm_out_width != (unsigned int)_pwm_out_width) {
    pwm_out_width = _pwm_out_width;
    analogWrite(PWM1_OUT_PIN, pwm_out_width);
    analogWrite(PWM2_OUT_PIN, pwm_out_width);
  }
  
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

    pinMode(PWM_OUT_WIDTH_SELECT_PIN, INPUT);

    pinMode(FREQ_HIGH_SELECT_PIN, INPUT);
    pinMode(FREQ_LOW_SELECT_PIN, INPUT);

    analogWriteResolution(10); // 0 - 1024
    analogReadResolution(10); // 0 - 1024
}

void loop() {
  current_time = micros();

  get_pwm1_value();
  get_pwm2_value();
  
  calc_pwm_out_width();

  calc_pwm1_out_freq();
  calc_pwm2_out_freq();

  oled_update();

  calc_loopsPS();
}
