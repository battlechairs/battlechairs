int readA0, readA1, readA2, readA3, readA4, readA5;

void setup() {
  Serial.begin(9600);
}

void loop() {
  readA0 = analogRead(A0);
  readA1 = analogRead(A1);
  readA2 = analogRead(A2);
  readA3 = analogRead(A3);
  readA4 = analogRead(A4);
  readA5 = analogRead(A5);

  Serial.print("[");
  Serial.print(readA0);
  Serial.print(",");
  Serial.print(readA1);
  Serial.print(",");
  Serial.print(readA2);
  Serial.print(",");
  Serial.print(readA3);
  Serial.print(",");
  Serial.print(readA4);
  Serial.print(",");
  Serial.print(readA5);
  Serial.print("]");

  delay(2);
}
