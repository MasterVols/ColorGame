#include <math.h>

#define PI 3.14159265
#define BUZZER_PIN 11 // Define the pin for the piezo buzzer

// User changing pins:
#define RED_PIN 9   
#define GREEN_PIN 10
#define BLUE_PIN 11

// Target LED Pins:
#define RED_PIN_TARGET 6   
#define GREEN_PIN_TARGET 7
#define BLUE_PIN_TARGET 8

const int stylusPin = A4; // Analog input pin for stylus
const int xAxisPositive = A0;
const int xAxisNegative = A1;
const int yAxisPositive = A2;
const int yAxisNegative = A3;
const int numSamples = 64; // Number of samples to average

int targetR, targetG, targetB; // Target RGB values

void setup() {
  // Set up serial communication
  Serial.begin(9600);

  // Set 2D board pins as output
  pinMode(xAxisPositive, OUTPUT);
  pinMode(xAxisNegative, OUTPUT);
  pinMode(yAxisPositive, OUTPUT);
  pinMode(yAxisNegative, OUTPUT);

  // Initialize 2D board pins to LOW
  digitalWrite(xAxisPositive, LOW);
  digitalWrite(xAxisNegative, LOW);
  digitalWrite(yAxisPositive, LOW);
  digitalWrite(yAxisNegative, LOW);

  pinMode(BUZZER_PIN, OUTPUT);

  // Set up RGB LED pins as output
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(RED_PIN_TARGET, OUTPUT);
  pinMode(GREEN_PIN_TARGET, OUTPUT);
  pinMode(BLUE_PIN_TARGET, OUTPUT);

   // Generate the first target color
  generateTargetColor();
}

void loop() {
  // Read and average X-axis
  digitalWrite(xAxisPositive, HIGH);
  digitalWrite(xAxisNegative, LOW);
  delay(10); // Short delay for stabilization
  long sumX = 0;
  for (int i = 0; i < numSamples; i++) {
    sumX += analogRead(stylusPin);
    delay(1); // Short delay between samples
  }
  int avgXValue = sumX / numSamples;
  digitalWrite(xAxisPositive, LOW); // Reset X-axis pins

  // Read and average Y-axis
  digitalWrite(yAxisPositive, HIGH);
  digitalWrite(yAxisNegative, LOW);
  delay(10); // Short delay for stabilization
  long sumY = 0;
  for (int i = 0; i < numSamples; i++) {
    sumY += analogRead(stylusPin);
    delay(1); // Short delay between samples
  }
  int avgYValue = sumY / numSamples;
  digitalWrite(yAxisPositive, LOW); // Reset Y-axis pins

  // Print averaged X and Y coordinates
  Serial.print("X: ");
  Serial.print(avgXValue);
  Serial.print(", Y: ");
  Serial.println(avgYValue);

  avgXValue = map(avgXValue, 910, 950, 1023, 0);
  avgYValue = map(avgYValue, 910, 950, 1023, 0);

  // Print averaged X and Y coordinates
  Serial.print("Xm: ");
  Serial.print(avgXValue);
  Serial.print(", Ym: ");
  Serial.println(avgYValue);

  // Convert averaged X and Y values to RGB and set the user LED color
  float xf = avgXValue / 1023.0 - 0.5;
  float yf = avgYValue / 1023.0 - 0.5;
  int r, g, b;
  getRGBFromXY(xf, yf, &r, &g, &b);
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);

  Serial.print("User Color: ");
  Serial.print("R=");
  Serial.print(r);
  Serial.print(", G=");
  Serial.print(g);
  Serial.print(", B=");
  Serial.println(b);
  
  Serial.print("Target Color: ");
  Serial.print("R=");
  Serial.print(targetR);
  Serial.print(", G=");
  Serial.print(targetG);
  Serial.print(", B=");
  Serial.println(targetB);

  // Check if the user's color matches the target color
  if (isColorMatch(r, g, b, targetR, targetG, targetB)) {
    playScoreBeep();
    // Flash both LEDs three times
    for (int i = 0; i < 3; i++) {
      flashLEDs();
    }
    // Generate a new target color
    generateTargetColor();
  }

  // Set the target LED color
  analogWrite(RED_PIN_TARGET, targetR);
  analogWrite(GREEN_PIN_TARGET, targetG);
  analogWrite(BLUE_PIN_TARGET, targetB);

  // Delay before the next loop iteration
  delay(10);
}

void generateTargetColor() {
  // Generate random RGB values for the target color
  targetR = random(256);
  targetG = random(256);
  targetB = random(128); //blue is hard to see because physics
}

bool isColorMatch(int r, int g, int b, int targetR, int targetG, int targetB) {
  // Check if the user's color is within 10% of the target color
  return abs(r - targetR) <= 100 && abs(g - targetG) <= 100 && abs(b - targetB) <= 110;
}

void flashLEDs() {
  // Turn both LEDs on
  analogWrite(RED_PIN, targetR);
  analogWrite(GREEN_PIN, targetG);
  analogWrite(BLUE_PIN, targetB);
  analogWrite(RED_PIN_TARGET, targetR);
  analogWrite(GREEN_PIN_TARGET, targetG);
  analogWrite(BLUE_PIN_TARGET, targetB);
  delay(200);

  // Turn both LEDs off
  analogWrite(RED_PIN, 0);
  analogWrite(GREEN_PIN, 0);
  analogWrite(BLUE_PIN, 0);
  analogWrite(RED_PIN_TARGET, 0);
  analogWrite(GREEN_PIN_TARGET, 0);
  analogWrite(BLUE_PIN_TARGET, 0);
  delay(200);
}

void getRGBFromXY(float x, float y, int* r, int* g, int* b) {
  // Convert x,y to polar coordinates (r, theta)
  float theta = atan2(y, x);
  if (theta < 0) {
    theta += 2 * PI; // Ensure angle is positive
  }

  // Map theta to Hue (0-360)
  float hue = theta * (360 / (2 * PI));

  // Convert Hue to RGB (this is a simplified version, you might need a more accurate conversion)
  if (hue < 120) {
    *r = 255 - (hue * 255 / 120);
    *g = hue * 255 / 120;
    *b = 0;
  } else if (hue < 240) {
    hue -= 120;
    *r = 0;
    *g = 255 - (hue * 255 / 120);
    *b = hue * 255 / 120;
  } else {
    hue -= 240;
    *r = hue * 255 / 120;
    *g = 0;
    *b = 255 - (hue * 255 / 120);
  }
}

void playScoreBeep() {
  // Play a short melody to indicate a score
  tone(BUZZER_PIN, 523, 200); // Play note C5 for 200 ms
  delay(200);                 // Wait for 200 ms
  tone(BUZZER_PIN, 587, 200); // Play note D5 for 200 ms
  delay(200);                 // Wait for 200 ms
  tone(BUZZER_PIN, 659, 400); // Play note E5 for 400 ms
  delay(400);                 // Wait for 400 ms
  noTone(BUZZER_PIN);         // Stop playing any tone
}
