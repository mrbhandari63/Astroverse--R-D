#include <SoftwareSerial.h>
SoftwareSerial btSerial(10, 11); // RX, TX

const int btnF = 2; // Forward
const int btnB = 3; // Backward
const int btnL = 4; // Left
const int btnR = 5; // Right

void setup() {
  btSerial.begin(9600);
  
  // Enable internal pull-ups (Pins are HIGH until pressed to GND)
  pinMode(btnF, INPUT_PULLUP);
  pinMode(btnB, INPUT_PULLUP);
  pinMode(btnL, INPUT_PULLUP);
  pinMode(btnR, INPUT_PULLUP);
}

void loop() {
  // digitalRead is LOW when button is pressed
  bool f = !digitalRead(btnF);
  bool b = !digitalRead(btnB);
  bool l = !digitalRead(btnL);
  bool r = !digitalRead(btnR);

  char command = 'S'; // Default state is Stop

  // Determine the command character based on button combinations
  if (f && r) command = 'I';      // Forward-Right
  else if (f && l) command = 'G'; // Forward-Left
  else if (b && r) command = 'J'; // Backward-Right
  else if (b && l) command = 'H'; // Backward-Left
  else if (f) command = 'F';      // Forward
  else if (b) command = 'B';      // Backward
  else if (l) command = 'L';      // Spin Left
  else if (r) command = 'R';      // Spin Right

  // Send the command
  btSerial.println(command);
  
  // Wait 50ms before sending again (20 times per second)
  delay(50); 
}