#include <Servo.h>

// --- Hardware Connections ---
const int leftPanelPin = A0;   
const int rightPanelPin = A1;  
const int servoPin = 9;        

Servo trackerServo;

// --- Tracker Variables ---
int servoPos = 90;             

// --- Filter Variables ---
float filteredLeft = 0;
float filteredRight = 0;

// MODIFIED: Increased to 0.3. This is a "lighter" filter. 
// It reacts 3x faster to shadows/light changes than 0.1 did.
const float FILTER_WEIGHT = 0.3; 

// --- System Settings ---
const int TOLERANCE = 15;      
const int NIGHT_MODE = 50;     
// MODIFIED: Dropped from 25 to 5. This makes the 1-degree steps happen lightning fast.
const int SPEED_LIMIT = 5;    

void setup() {
  Serial.begin(9600); 
  
  trackerServo.attach(servoPin);
  trackerServo.write(servoPos); 
  
  filteredLeft = analogRead(leftPanelPin);
  filteredRight = analogRead(rightPanelPin);
  
  delay(1000); 
}

void loop() {
  // 1. Read the raw light levels
  int rawLeft = analogRead(leftPanelPin);
  int rawRight = analogRead(rightPanelPin);

  // 2. Apply the Exponential Moving Average Filter
  filteredLeft = (FILTER_WEIGHT * rawLeft) + ((1.0 - FILTER_WEIGHT) * filteredLeft);
  filteredRight = (FILTER_WEIGHT * rawRight) + ((1.0 - FILTER_WEIGHT) * filteredRight);

  int leftLight = (int)filteredLeft;
  int rightLight = (int)filteredRight;

  // 3. Print data to the Serial Monitor
  Serial.print("Raw L: "); Serial.print(rawLeft);
  Serial.print(" | Smooth L: "); Serial.print(leftLight);
  Serial.print(" || Raw R: "); Serial.print(rawRight);
  Serial.print(" | Smooth R: "); Serial.print(rightLight);

  // 4. Night Time Check
  if (leftLight < NIGHT_MODE && rightLight < NIGHT_MODE) {
    Serial.println(" | Status: SLEEPING");
    delay(1000);
    return; 
  }

  // 5. Calculate the difference
  int difference = leftLight - rightLight;

  // 6. Movement Logic
  if (abs(difference) > TOLERANCE) {
    
    if (leftLight > rightLight) {
      // MODIFIED: Back to 1-degree steps for smooth mechanical sweeping
      servoPos++; 
      Serial.println(" | Action: <- Left");
    } else {
      // MODIFIED: Back to 1-degree steps
      servoPos--; 
      Serial.println(" | Action: Right ->");
    }

    // Safety Limits 
    servoPos = constrain(servoPos, 10, 170);
    trackerServo.write(servoPos);
    
  } else {
    Serial.println(" | Action: LOCKED");
  }

  // 7. Speed Governor
  delay(SPEED_LIMIT); 
}