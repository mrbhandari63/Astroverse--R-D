#include <Servo.h>

// Pin Definitions
const int trigPin = 10;
const int echoPin = 11;
const int servoPin = 12;

// Variables
long duration;
int distance;
Servo myServo;

void setup() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);  // Sets the echoPin as an Input
  Serial.begin(9600);       // Matches the baud rate in your Processing script
  myServo.attach(servoPin); // Defines on which pin the servo is attached
}

void loop() {
  // Rotates the servo from 0 to 180 degrees
  for (int i = 0; i <= 180; i++) {
    myServo.write(i);
    delay(30);
    distance = calculateDistance(); // Get distance for current angle
    
    // Format: Angle,Distance. (Matches your Processing substring/comma logic)
    Serial.print(i);
    Serial.print(",");
    Serial.print(distance);
    Serial.print(".");
    delay(10);
  }
  
  // Rotates the servo back from 180 to 0 degrees
  for (int i = 180; i > 0; i--) {
    myServo.write(i);
    delay(30);
    distance = calculateDistance();
    
    Serial.print(i);
    Serial.print(",");
    Serial.print(distance);
    Serial.print(".");
    delay(10);
  }
}

// Function to calculate the distance measured by the Ultrasonic sensor
int calculateDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH); // Reads the echoPin, returns the sound wave travel time
  distance = duration * 0.034 / 2;    // Calculating the distance in cm
  
  // Constrain distance to match your UI's 80cm limit
  if (distance > 80) distance = 81; 
  
  return distance;
}