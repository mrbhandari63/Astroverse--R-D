const int ledPins[] = {3, 5, 6, 9, 10, 11}; 
const int micPin = A0;

// Sensitivity Tuning Variables
const int quietThreshold = 10;  // Ignore tiny electronic noise
const int loudThreshold = 150;  // Lower this number to make it MORE sensitive
const int sampleWindow = 30;    // 30ms for fast response

// Smoothing Variables
float smoothedBrightness = 0;   // Keeps track of the falling peak
const float decayRate = 35.0;   // How fast the LEDs fall (higher = faster drop)

void setup() {
  for (int i = 0; i < 6; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
}

void loop() {
  unsigned int signalMax = 0;
  unsigned int signalMin = 1023;
  unsigned long startMillis = millis();

  // 1. Fast Sampling Window
  while (millis() - startMillis < sampleWindow) {
    int sample = analogRead(micPin);
    if (sample > signalMax) signalMax = sample;
    if (sample < signalMin) signalMin = sample;
  }

  unsigned int peakToPeak = signalMax - signalMin;

  // 2. High Resolution Mapping
  // Instead of mapping 0-6, we map to the total "pool" of PWM brightness
  // 6 LEDs * 255 max PWM value = 1530 total brightness points
  int targetBrightness = map(peakToPeak, quietThreshold, loudThreshold, 0, 1530);
  targetBrightness = constrain(targetBrightness, 0, 1530); 

  // 3. Smooth Attack and Decay (The "Bounce")
  if (targetBrightness > smoothedBrightness) {
    // Attack fast: if the new sound is louder, jump straight to it
    smoothedBrightness = targetBrightness; 
  } else {
    // Decay slowly: if the sound drops, let the LEDs fade down smoothly
    smoothedBrightness -= decayRate; 
    if (smoothedBrightness < 0) smoothedBrightness = 0;
  }

  // 4. Drive LEDs with actual hardware PWM
  for (int i = 0; i < 6; i++) {
    int ledStart = i * 255;       // The threshold where this LED starts turning on
    int ledEnd = ledStart + 255;  // The threshold where this LED is fully on

    if (smoothedBrightness >= ledEnd) {
      analogWrite(ledPins[i], 255); // Fully on
    } else if (smoothedBrightness > ledStart) {
      analogWrite(ledPins[i], smoothedBrightness - ledStart); // Partially on (fading)
    } else {
      analogWrite(ledPins[i], 0); // Off
    }
  }
}