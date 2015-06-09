int left = 0;
int right = 0;

int lpParts = 10;
int lpResponse = 5;

int leftMax = 0;
int rightMax = 0;

int highOffset = -75;
int lowOffset = -100;

void setup() {
  Serial.begin(9600);
}

void loop() {
  left = (lpResponse * analogRead(0) + (lpParts - lpResponse) * left) / lpParts;
  right = (lpResponse * analogRead(2) + (lpParts - lpResponse) * right) / lpParts;

  if (left > leftMax) leftMax = left;
  if (right > rightMax) rightMax = right;

  Serial.print(left);
  Serial.print(",");
  Serial.print(leftMax + highOffset);
  Serial.print(",");
  Serial.print(leftMax + lowOffset);
  Serial.print(",");
  Serial.print(right);
  Serial.print(",");
  Serial.print(rightMax + highOffset);
  Serial.print(",");
  Serial.print(rightMax + lowOffset);
  Serial.print("\n");

  delay(1);
}

