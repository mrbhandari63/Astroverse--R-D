#include <Servo.h>
#include <NewPing.h>

// --- Config ---
#define TRIG_PIN 9
#define ECHO_PIN 10
#define MAX_DIST 400

// --- FIXED SERVO POSITIONS ---
int initPos_D3 = 90;  int deployPos_D3 = 180;
int initPos_D4 = 90;   int deployPos_D4 = 0;
int initPos_D5 = 0;   int deployPos_D5 = 90;
int initPos_D6 = 180;  int deployPos_D6 = 90;

// --- Objects ---
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DIST);
Servo servoD3, servoD4, servoD5, servoD6;

// Variables
bool deployed = false;

void setup() {
  // Attach Servos
  servoD3.attach(3); 

// Variables
bool deployed = false;

void setup() {
  // Attach Servos
  servoD4.attach(4); 
  servoD5.attach(5); 
  servoD6.attach(6);

  // 1. Move to Initial Positions (Flight Mode)
  servoD3.write(initPos_D3); 
  servoD4.write(initPos_D4);
  servoD5.write(initPos_D5); 
  servoD6.write(initPos_D6);
  
  delay(1000); // Give servos time to reach starting positions
}

void loop() {
  // If already deployed, do nothing else
  if (deployed) return;

  // 30ms delay is the hardware requirement so the ultrasonic sensor doesn't jam
  delay(30); 
  
  int currDist = sonar.ping_cm();

  // THE LOGIC: 
  // 1. currDist > 0 ensures it ignores "out of range" errors
  // 2. currDist < 30 triggers the deployment
  if (currDist > 0 && currDist < 50) {
    deploy();
  }
}

void deploy() {
  // Move to Deployed Positions (Landing Mode)
  servoD3.write(deployPos_D3); 
  servoD4.write(deployPos_D4);
  servoD5.write(deployPos_D5); 
  servoD6.write(deployPos_D6);
  
  // Lock the state
  deployed = true;
}