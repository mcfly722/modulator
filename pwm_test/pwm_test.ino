const uint8_t   LED_pin        = 13;
const uint8_t   PWM_1_pin      = A6;
const uint8_t   PWM_2_pin      = A9;

void setup() {
  pinMode(LED_pin, OUTPUT);
  pinMode(PWM_1_pin, OUTPUT);
  pinMode(PWM_2_pin, OUTPUT);

  digitalWrite(LED_pin, HIGH);
  
  analogWriteResolution(10);
  
}

void loop() {

  digitalWrite(LED_pin, HIGH);

  analogWrite(PWM_1_pin, 1024);
  analogWrite(PWM_2_pin, 100);
  delay(5000);
  
  digitalWrite(LED_pin, LOW);

  analogWrite(PWM_1_pin, 0);
  analogWrite(PWM_2_pin, 900);
  delay(5000);
  
}
