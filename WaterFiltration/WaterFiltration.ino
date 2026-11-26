#include <AFMotor.h>

// ---------------- Motor Shield devices ----------------
AF_DCMotor motorFilter(4);   // Filter press motor
AF_DCMotor motorMix(1);      // Mixing motor
AF_DCMotor pumpClean(3);     // Clean water pump
AF_DCMotor pumpAlum(2);      // Alum pump

// ---------------- Relay for centrifugal pump ----------------
const int relayCF = A2;
// Most cheap relay modules are active-LOW (LOW = relay ON).
// If yours is, swap these two values. Everything else follows automatically.
const int RELAY_ON  = HIGH;
const int RELAY_OFF = LOW;

// ---------------- Buttons (wired to GND, internal pull-ups) ----------------
const int startButton     = A3;  // Start button
const int emergencyButton = A9;  // Emergency stop (A9 exists on the Arduino Mega only)

// ---------------- Turbidity sensors ----------------
const int turbBeforePin = A0;
const int turbAfterPin  = A1;

// ---------------- Mixer speeds ----------------
const uint8_t FAST_SPEED = 255;
const uint8_t SLOW_SPEED = 155;

// ======================================================
void setup() {
  Serial.begin(9600);

  // Buttons
  pinMode(startButton, INPUT_PULLUP);
  pinMode(emergencyButton, INPUT_PULLUP);

  // Relay: set the OFF level BEFORE making the pin an output,
  // so the pump never blips on at power-up
  digitalWrite(relayCF, RELAY_OFF);
  pinMode(relayCF, OUTPUT);

  // Motors
  motorFilter.setSpeed(255);
  motorMix.setSpeed(FAST_SPEED);
  pumpClean.setSpeed(255);
  pumpAlum.setSpeed(255);

  stopAll();

  Serial.println("System Ready");
}

// ======================================================
void loop() {
  checkEmergency();

  // Start button pressed?
  if (digitalRead(startButton) == LOW) {
    waitWithEStop(50);                       // debounce
    if (digitalRead(startButton) == LOW) {
      runSequence();

      // Don't restart just because the button is still being held
      while (digitalRead(startButton) == LOW) {
        checkEmergency();
      }
      waitWithEStop(50);
    }
  }
}

// ======================================================
// MAIN SEQUENCE
// Every wait uses waitWithEStop(), so the emergency button
// is checked about once per millisecond for the whole run.
// ======================================================
void runSequence() {
  Serial.println("Sequence Started");

  // TURBIDITY BEFORE
  Serial.println("Reading Turbidity BEFORE:");
  float before = readTurbidity(turbBeforePin);
  Serial.print("Before Voltage: ");
  Serial.println(before);

  // 1. CENTRIFUGAL PUMP
  Serial.println("Centrifugal Pump ON");
  digitalWrite(relayCF, RELAY_ON);
  waitWithEStop(11000);   // 11 seconds

  Serial.println("Centrifugal Pump OFF");
  digitalWrite(relayCF, RELAY_OFF);
  waitWithEStop(500);

  // 2. ALUM PUMP
  Serial.println("Alum Pump ON");
  pumpAlum.run(FORWARD);
  waitWithEStop(5000);

  Serial.println("Alum Pump OFF");
  pumpAlum.run(RELEASE);
  waitWithEStop(500);

  // 3. FAST MIX
  Serial.println("Fast Mix Motor ON");
  motorMix.setSpeed(FAST_SPEED);   // reset every run (slow mix lowers it)
  motorMix.run(FORWARD);
  waitWithEStop(10000);

  Serial.println("Fast Mix Motor OFF");
  motorMix.run(RELEASE);
  waitWithEStop(500);

  // 4. SLOW MIX
  Serial.println("Slow Mix Motor ON");
  motorMix.setSpeed(SLOW_SPEED);
  motorMix.run(BACKWARD);
  waitWithEStop(10000);

  Serial.println("Slow Mix Motor OFF");
  motorMix.run(RELEASE);
  motorMix.setSpeed(FAST_SPEED);
  waitWithEStop(500);

  // 5. FILTER PRESS
  Serial.println("Filter Press ON");
  motorFilter.run(FORWARD);
  waitWithEStop(5000);

  Serial.println("Filter Press OFF");
  motorFilter.run(RELEASE);
  waitWithEStop(500);

  // 6. CLEAN WATER PUMP
  Serial.println("Clean Water Pump ON");
  pumpClean.run(FORWARD);
  waitWithEStop(8000);

  Serial.println("Clean Water Pump OFF");
  pumpClean.run(RELEASE);
  waitWithEStop(1000);   // let the water settle around the sensor

  // 7. TURBIDITY AFTER
  Serial.println("Reading Turbidity AFTER:");
  float after = readTurbidity(turbAfterPin);
  Serial.print("After Voltage: ");
  Serial.println(after);

  // For the common analog turbidity sensors, higher voltage = clearer water
  Serial.print("Change (after - before): ");
  Serial.println(after - before);

  Serial.println("Sequence Complete");
}

// ======================================================
// TURBIDITY SENSOR: average 20 readings to smooth out noise
// ======================================================
float readTurbidity(int pin) {
  long total = 0;
  for (int i = 0; i < 20; i++) {
    total += analogRead(pin);
    waitWithEStop(5);
  }
  return (total / 20.0) * (5.0 / 1023.0);
}

// ======================================================
// TIMING + EMERGENCY HELPERS
// ======================================================
// Replacement for delay() that keeps watching the emergency button
void waitWithEStop(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    checkEmergency();
  }
}

void checkEmergency() {
  if (digitalRead(emergencyButton) == LOW) {
    delay(20);   // short debounce
    if (digitalRead(emergencyButton) == LOW) {
      emergencyStop();
    }
  }
}

// STOP EVERYTHING
void stopAll() {
  digitalWrite(relayCF, RELAY_OFF);
  motorFilter.run(RELEASE);
  motorMix.run(RELEASE);
  pumpClean.run(RELEASE);
  pumpAlum.run(RELEASE);
}

// EMERGENCY STOP
void emergencyStop() {
  stopAll();   // stop outputs first, then report
  Serial.println("EMERGENCY STOP ACTIVATED");
  while (true) {}   // freeze until the board is reset
}