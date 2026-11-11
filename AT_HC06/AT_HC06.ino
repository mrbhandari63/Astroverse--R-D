#include <SoftwareSerial.h>

// RX on D10, TX on D11
SoftwareSerial btSerial(10, 11); 

void setup() {
  Serial.begin(9600);
  
  // HC-06 defaults to 9600 baud for AT commands
  btSerial.begin(9600); 
  
  Serial.println("HC-06 Passthrough Ready.");
  Serial.println("IMPORTANT: Set Serial Monitor to 'No line ending'.");
}

void loop() {
  // Read from HC-06 and print to Serial Monitor
  if (btSerial.available()) {
    Serial.write(btSerial.read());
  }
  
  // Read from Serial Monitor and send to HC-06
  if (Serial.available()) {
    btSerial.write(Serial.read());
  }
}