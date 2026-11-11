const int ldrPin = A0;      // Analog input pin for LDR module
const int mosfetPin = 9;    // PWM output pin to MOSFET Gate (must be a PWM capable pin)

// Thresholds for LDR readings. 
// For this module, a HIGHER value means DARKER ambient light.
const int lightThreshold = 400; // LDR reading ABOVE which the LED starts to glow
const int fullDarkValue = 800;  // LDR reading at which the MOSFET fully turns on (100% brightness)

void setup() {
  Serial.begin(9600);
  pinMode(ldrPin, INPUT);
  pinMode(mosfetPin, OUTPUT);
}

void loop() {
  int ldrValue = analogRead(ldrPin);
  
  // Print the LDR value to Serial Monitor for calibration
  Serial.print("LDR Value: ");
  Serial.print(ldrValue);

  int pwmValue = 0;

  if (ldrValue > lightThreshold) {
    if (ldrValue >= fullDarkValue) {
      // It's very dark, turn the MOSFET fully ON (Closed circuit)
      pwmValue = 255; 
    } else {
      // Map the LDR reading proportionally to the PWM output (0 to 255)
      // As LDR reading goes from lightThreshold up to fullDarkValue, PWM goes from 0 up to 255
      pwmValue = map(ldrValue, lightThreshold, fullDarkValue, 0, 255);
    }
  } else {
    // Ambient light is high enough (LDR is below threshold), turn LED off
    pwmValue = 0; 
  }

  Serial.print(" | PWM Output: ");
  Serial.println(pwmValue);

  // Write the PWM signal to the MOSFET
  analogWrite(mosfetPin, pwmValue);
  
  delay(100); // Small delay to stabilize readings
}
