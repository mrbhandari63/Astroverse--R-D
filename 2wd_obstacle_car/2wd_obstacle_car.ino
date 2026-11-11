#include <NewPing.h>

// --- PIN DEFINITIONS ---
const int enA = 6;  // Right Speed
const int enB = 11; // Left Speed
const int IN1 = 3;  // Right Motor
const int IN2 = 2;
const int IN3 = 5;  // Left Motor
const int IN4 = 4;

const int trigPin = 9;
const int echoPin = 10;

// --- SETTINGS ---
const int obstacleThreshold = 25; 
const int backUpDelay = 600;

// *** SPEEDS ***
const int speedLeft = 170;   
const int speedRight = 170;  

// *** BURST TURN SETTINGS ***
const int burstSpeed = 150;
const int burstTime = 150; 
// const int burstSpeed = 255; 
// const int burstTime = 150;   
// IMPORTANT: Ensure '4' steps is enough for 90 degrees. 
// If it only turns 45 degrees, change this to 8.
const int stepsPerUnit = 4; 

// *** SCAN SETTINGS ***
// CHANGED TO 5: This creates 4 intervals (Turns)
// Positions: 0 (Front), 1 (Right), 2 (Back), 3 (Left), 4 (Front again)
const int totalPositions = 5; 
const int scanDelay = 200;      

// Sensor Setup
const int MAX_DISTANCE = 200; 
NewPing sonar(trigPin, echoPin, MAX_DISTANCE);

void setup() {
  pinMode(enA, OUTPUT); pinMode(enB, OUTPUT);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  
  Serial.begin(9600);
}

void loop() {
  int distance = getDistance();
  
  if (distance < obstacleThreshold) {
    stopMotors();
    delay(200);
    moveBackward();
    delay(backUpDelay);
    stopMotors();
    delay(300);

    performSafeSweep();
    
  } else {
    moveForward();
  }
  delay(50);
}

void performSafeSweep() {
  int distances[totalPositions]; 
  int maxDist = 0;
  int bestIndex = 0;

  // --- PART 1: SCANNING (0 -> 90 -> 180 -> 270 -> 360) ---
  for (int i = 0; i < totalPositions; i++) {
    // 1. Measure
    distances[i] = getDistance();
    
    // 2. Remember Best
    // Note: We scan position 0 and 4 (both are Front). 
    // This logic automatically picks the better measurement of the two.
    if (distances[i] > maxDist) {
      maxDist = distances[i];
      bestIndex = i;
    }

    // 3. Turn Right (4 times total)
    // i=0(Turn), i=1(Turn), i=2(Turn), i=3(Turn), i=4(STOP)
    if (i < (totalPositions - 1)) {
      burstTurn(true, stepsPerUnit); 
      delay(scanDelay); 
    }
  }
  
  // --- PART 2: THE WAIT ---
  stopMotors();
  delay(1000); 

  // --- PART 3: SAFE RETURN (Reverse/Left Only) ---
  // Robot is now at Index 4 (Facing Front/360).
  // Example 1: Best was Index 0 (Front). 4 - 0 = 4 steps back (Full circle reverse).
  // Example 2: Best was Index 4 (Front). 4 - 4 = 0 steps back (Stay).
  
  int chunksToReturn = (totalPositions - 1) - bestIndex;

  Serial.print("Best Index: "); Serial.println(bestIndex);
  Serial.print("Reversing Steps: "); Serial.println(chunksToReturn);

  // Loop to return step-by-step
  for (int j = 0; j < chunksToReturn; j++) {
    burstTurn(false, stepsPerUnit); // Turn Left
    delay(scanDelay);               
  }

  // --- FINISH ---
  stopMotors();
  delay(500);
}

// --- BURST TURN FUNCTION ---
void burstTurn(bool isRight, int steps) {
  analogWrite(enA, burstSpeed);
  analogWrite(enB, burstSpeed);

  for (int i = 0; i < steps; i++) {
    if (isRight) {
      digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); 
      digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); 
    } else {
      digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); 
      digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH); 
    }
    
    delay(burstTime); // 50ms ON
    stopMotors();     
    delay(30);        // 30ms OFF
  }
}

// --- STANDARD MOVEMENT ---
void moveForward() {
  analogWrite(enA, speedRight); analogWrite(enB, speedLeft);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void moveBackward() {
  analogWrite(enA, speedRight); analogWrite(enB, speedLeft);
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

int getDistance() {
  int cm = sonar.ping_cm();
  if (cm == 0) return 999; 
  return cm;
}
