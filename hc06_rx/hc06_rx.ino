#include <SoftwareSerial.h>
SoftwareSerial btSerial(10, 11); // RX, TX

// Motor A (Assuming Left Motor)
const int pwma = 5;
const int ain1 = 4;
const int ain2 = 2;

// Motor B (Assuming Right Motor)
const int pwmb = 6;
const int bin1 = 7;
const int bin2 = 8;

// Standby
const int stdby = 9;

// Failsafe timer
unsigned long lastReceiveTime = 0;

void setup() {
  btSerial.begin(9600);
  
  pinMode(pwma, OUTPUT);
  pinMode(ain1, OUTPUT);
  pinMode(ain2, OUTPUT);
  pinMode(pwmb, OUTPUT);
  pinMode(bin1, OUTPUT);
  pinMode(bin2, OUTPUT);
  pinMode(stdby, OUTPUT);
  
  // Enable the motor driver
  digitalWrite(stdby, HIGH); 
  stopMotors();
}

void loop() {
  // Check for incoming Bluetooth commands
  if (btSerial.available()) {
    String msg = btSerial.readStringUntil('\n');
    msg.trim();
    
    if (msg.length() > 0) {
      char cmd = msg[0];
      executeCommand(cmd);
      lastReceiveTime = millis(); // Reset the failsafe timer
    }
  }

  // Failsafe: If no command received for 500ms, stop the car
  if (millis() - lastReceiveTime > 500) {
    stopMotors();
  }
}

void executeCommand(char cmd) {
  int speedFull = 255; // Max speed for straight lines
  int speedTurn = 100; // Reduced speed for the inner wheel during moving turns
  int speedSpin = 180; // Speed for 0-radius spins (pure left/right)

  switch(cmd) {
    case 'F': moveMotors(speedFull, speedFull, HIGH, HIGH); break;
    case 'B': moveMotors(speedFull, speedFull, LOW, LOW); break;
    
    // Spin in place (wheels turn in opposite directions)
    case 'L': moveMotors(speedSpin, speedSpin, LOW, HIGH); break; 
    case 'R': moveMotors(speedSpin, speedSpin, HIGH, LOW); break; 
    
    // Moving turns (both wheels move same direction, but inner wheel is slower)
    case 'I': moveMotors(speedFull, speedTurn, HIGH, HIGH); break; // Fwd-Right
    case 'G': moveMotors(speedTurn, speedFull, HIGH, HIGH); break; // Fwd-Left
    case 'J': moveMotors(speedFull, speedTurn, LOW, LOW); break;   // Bwd-Right
    case 'H': moveMotors(speedTurn, speedFull, LOW, LOW); break;   // Bwd-Left
    
    case 'S': stopMotors(); break;
  }
}

// Function to control both motors cleanly
// dirA/dirB: HIGH = Forward, LOW = Backward
void moveMotors(int speedA, int speedB, bool dirA, bool dirB) {
  // Motor A (Left)
  digitalWrite(ain1, dirA);
  digitalWrite(ain2, !dirA);
  analogWrite(pwma, speedA);
  
  // Motor B (Right)
  digitalWrite(bin1, dirB);
  digitalWrite(bin2, !dirB);
  analogWrite(pwmb, speedB);
}

void stopMotors() {
  analogWrite(pwma, 0);
  analogWrite(pwmb, 0);
  digitalWrite(ain1, LOW);
  digitalWrite(ain2, LOW);
  digitalWrite(bin1, LOW);
  digitalWrite(bin2, LOW);
}