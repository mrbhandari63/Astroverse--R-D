const int analogFlamePin = A0;

void setup() {
  // 9600 baud rate is standard and stable for Web Serial
  Serial.begin(9600); 
}

void loop() {
  int irIntensity = analogRead(analogFlamePin);
  
  // Print only the number followed by a newline
  Serial.println(irIntensity);
  
  // Update 20 times a second for a smooth HTML animation
  delay(50); 
}