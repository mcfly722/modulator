# modulator
Simple signal modulator based on input PWM signal to variate output frequency. Could be used without input signal (frequency variate by resistors 1,2).
Working from 9V battery.
![alt tag](https://github.com/mcfly722/modulator/blob/main/photo.png?raw=true)

### Schema:<br>
![alt tag](https://github.com/mcfly722/modulator/blob/main/schema.png?raw=true)

<b>Jack 1</b> - PWM1 & PWM2 Inputs<br>
<b>Jack 2</b> - PWM1 & PWM2 Outputs<br>

By default, module reads <b>PWM1 input</b> signal and generates <b>PWM1 output</b> signal with frequency which variates by <b>PWM1</b> width (input width values from 0 to 1023 mapped to output frequency values from 0 to <b>FREQ_MAX_HZ</b>=32000Hz)<br>
Second <b>PWM2 output</b> frequency generates based on variable resistors. First one resistor variate upper part of frequency, second one is for bottom part.<br>
Third resistor variate <b>PWM2 output</b> signal width from 0% to 100%.<br>
<br>
To check module work, there are special test in <b>\\pwm_test\pwm_test.ino</b>.This sketch is for second one Teensy board which could be used to generate variate PWM width signal. Module should correctly identify it and generates correct output frequency.
To check output signal you can use just ordinary headphones with jack 3.5 (left ear is for PWM1, right for PWM2).<br>
