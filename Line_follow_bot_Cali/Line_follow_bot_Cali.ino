// ================= MOTOR PINS =================
int mr1 = 8;  int mr2 = 9;   
int ml1 = 10; int ml2 = 11;  

// ================= SENSOR PINS =================
int sl = 6;  // right sensor
int sr = 7;  // left sensor

// ================= ENABLE (PWM) =================
int enr = 3; 
int enl = 5; 

// ================= SPEED CALIBRATION =================
int baseSpeed = 100;         // Normal forward cruising speed
int maxSpeed = 255;          // Absolute maximum speed limit for any motor (0-255)

// Differential Turning Speeds
int turnSpeedOuter = 170;    // Speed of the outer wheel pushing the turn (Forward)
int turnSpeedInner = 170;    // Speed of the inner wheel pulling the turn (Reverse)

// =====================================================

void setup() {
  pinMode(mr1, OUTPUT); pinMode(mr2, OUTPUT);
  pinMode(ml1, OUTPUT); pinMode(ml2, OUTPUT);
  pinMode(enr, OUTPUT); pinMode(enl, OUTPUT);

  pinMode(sr, INPUT);
  pinMode(sl, INPUT);

  delay(2000); // Startup delay to place robot on track
}

void loop() {
  int leftSeen  = digitalRead(sl);
  int rightSeen = digitalRead(sr);

  if (leftSeen == LOW && rightSeen == LOW) {
    // Both on line -> Go straight
    goForward(baseSpeed);
  } 
  else if (leftSeen == HIGH && rightSeen == LOW) {
    // Left off, Right on -> Turn RIGHT
    // Left wheel goes forward (Outer), Right wheel goes reverse (Inner)
    pivotRight(turnSpeedOuter, turnSpeedInner);
  } 
  else if (leftSeen == LOW && rightSeen == HIGH) {
    // Right off, Left on -> Turn LEFT
    // Right wheel goes forward (Outer), Left wheel goes reverse (Inner)
    pivotLeft(turnSpeedOuter, turnSpeedInner);
  } 
  else {
    // Both off line -> Hard Stop
    stopRobot();
  }
}

// ================= DIFFERENTIAL MOTION HELPERS =================

void goForward(int speed) {
  int safeSpeed = min(speed, maxSpeed); // Caps speed at maxSpeed
  
  digitalWrite(mr1, HIGH); digitalWrite(mr2, LOW);
  digitalWrite(ml1, HIGH); digitalWrite(ml2, LOW);
  
  analogWrite(enr, safeSpeed);
  analogWrite(enl, safeSpeed);
}

void pivotRight(int outerSpeed, int innerSpeed) {
  int safeOuter = min(outerSpeed, maxSpeed);
  int safeInner = min(innerSpeed, maxSpeed);
  
  // Left motor FORWARD (Outer pushing wheel)
  digitalWrite(ml1, HIGH); digitalWrite(ml2, LOW);
  analogWrite(enl, safeOuter);
  
  // Right motor REVERSE (Inner pulling wheel)
  digitalWrite(mr1, LOW); digitalWrite(mr2, HIGH);;         // Normal forward cruising speed
int maxSpeed = 255;          // Absolute maximum speed limit for any motor (0-255)

// Differential Turning Speeds
int turnSpeedOuter = 255; 
  analogWrite(enr, safeInner);
}

void pivotLeft(int outerSpeed, int innerSpeed) {
  int safeOuter = min(outerSpeed, maxSpeed);
  int safeInner = min(innerSpeed, maxSpeed);
  
  // Right motor FORWARD (Outer pushing wheel)
  digitalWrite(mr1, HIGH); digitalWrite(mr2, LOW);
  analogWrite(enr, safeOuter);
  
  // Left motor REVERSE (Inner pulling wheel)
  digitalWrite(ml1, LOW); digitalWrite(ml2, HIGH);
  analogWrite(enl, safeInner);
}

void stopRobot() {
  // Electronic braking
  digitalWrite(mr1, LOW); digitalWrite(mr2, LOW);
  digitalWrite(ml1, LOW); digitalWrite(ml2, LOW);
  
  analogWrite(enr, 0);
  analogWrite(enl, 0);
}
