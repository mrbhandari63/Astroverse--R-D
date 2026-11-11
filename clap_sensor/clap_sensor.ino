// Pins
const int micPin = 2;    // Digital Output of mic module
const int ledPin = 13;   // Onboard LED or external LED

// Variables for clap detection
int clapCount = 2;
long lastClapTime = 0;
int sensorState = HIGH;
int lastSensorState = HIGH;

// Settings
const int clapGap = 500;       // Max time (ms) between two claps
const int debounceTime = 100;  // Min time (ms) to ignore sensor noise

bool ledStatus = false;

void setup() {
  pinMode(micPin, INPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  sensorState = digitalRead(micPin);

  // Detect transition from HIGH to LOW (typical for these modules)
  if (sensorState == LOW && lastSensorState == HIGH) {
    long currentTime = millis();

    // Debounce: ignore if the signal is too fast (likely one long sound)
    if (currentTime - lastClapTime > debounceTime) {
      clapCount++;
      lastClapTime = currentTime;
      Serial.println("Clap detected!");
    }
  }

  lastSensorState = sensorState;

  // Check if the timing window for the second clap has expired
  if (clapCount > 0 && (millis() - lastClapTime > clapGap)) {
    if (clapCount == 2) {
      ledStatus = !ledStatus;      // Toggle state
      digitalWrite(ledPin, ledStatus ? HIGH : LOW);
      Serial.println("Double Clap: TOGGLE");
    }
    
    clapCount = 0; // Reset count after the window closes
  }
}