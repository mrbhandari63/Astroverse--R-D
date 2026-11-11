const int ledPins[] = {3, 5, 6, 9, 10, 11}; 
const int micPin = A0;
const int toggleLedPin = 12; // D12 for the two-clap toggle

// Sensitivity Tuning Variables
const int quietThreshold = 10;  // Ignore tiny electronic noise
const int loudThreshold = 150;  // Lower this number to make it MORE sensitive
const int sampleWindow = 30;    // 30ms for fast response

// Smoothing Variables
float smoothedBrightness = 0;   // Keeps track of the falling peak
const float decayRate = 35.0;   // How fast the LEDs fall (higher = faster drop)

// Clap Detection Variables
const int clapThreshold = 200;  // Minimum peakToPeak to be considered a clap. Adjust if needed.
int clapCount = 0;
unsigned long lastClapTime = 0;
const unsigned long clapDebounceTime = 200;   // Min time (ms) between valid claps
const unsigned long doubleClapMaxTime = 800;  // Max time (ms) to wait for a second clap
bool toggleLedState = false;

void setup() {
  for (int i = 0; i < 6; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
  pinMode(toggleLedPin, OUTPUT);
  digitalWrite(toggleLedPin, LOW); // Start with the toggle LED off
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

  // -- NEW FEATURE: Two Claps Toggle D12 --
  if (peakToPeak >= clapThreshold) {
    unsigned long currentMillis = millis();
    
    if (clapCount == 0) {
      // First clap detected
      clapCount = 1;
      lastClapTime = currentMillis;
    } else if (clapCount == 1) {
      unsigned long timeSinceLastClap = currentMillis - lastClapTime;
      
      if (timeSinceLastClap > clapDebounceTime && timeSinceLastClap < doubleClapMaxTime) {
        // Valid second clap
        toggleLedState = !toggleLedState;
        digitalWrite(toggleLedPin, toggleLedState ? HIGH : LOW);
        clapCount = 0; // Reset for next time
      } else if (timeSinceLastClap >= doubleClapMaxTime) {
        // Too much time passed since first clap, treat this as a new first clap
        clapCount = 1;
        lastClapTime = currentMillis;
      }
      // If timeSinceLastClap <= clapDebounceTime, ignore (debounce the first clap)
    }
  }

  // Reset clap count if too much time has passed without a second clap
  if (clapCount == 1 && (millis() - lastClapTime >= doubleClapMaxTime)) {
    clapCount = 0;
  }

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