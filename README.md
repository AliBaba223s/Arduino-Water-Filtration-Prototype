# Arduino Water Filtration Prototype

C++ firmware for an Arduino Mega that runs a small-scale water treatment prototype from start to finish with one button press: pumping, chemical dosing, mixing, filtering and clean water transfer, with turbidity checks before and after. Built for Problem Solving and Modelling at Queen's University (Fall 2025).

## Results

- Turned dirty water into clean water in testing, confirmed by comparing turbidity sensor readings before and after each run.
- Used the test results to model a full-scale system treating 555,000 L of water per day, with cost and logistics analysis.

## How it works

One press of the start button runs a 7-stage sequence, then prints the change in turbidity to the Serial Monitor.

| Stage | Device | Run time |
| --- | --- | --- |
| Start: measure turbidity before | Turbidity sensor (A0) | 20 samples |
| 1. Centrifugal pump | Relay (A2) | 11 s |
| 2. Alum dosing | Alum pump (motor port M2) | 5 s |
| 3. Fast mix | Mixing motor (M1), full speed | 10 s |
| 4. Slow mix | Mixing motor (M1), reversed at lower speed | 10 s |
| 5. Filter press | Filter press motor (M4) | 5 s |
| 6. Clean water transfer | Clean water pump (M3) | 8 s |
| 7. Measure turbidity after | Turbidity sensor (A1), after 1 s to settle | 20 samples |

Short pauses between stages keep each motor or pump from overlapping with the next.

## Safety

- **Emergency stop:** every wait in the sequence uses `waitWithEStop()`, a replacement for `delay()` that checks the emergency button about once per millisecond. Pressing it stops every motor and pump right away and freezes the system until the board is reset.
- **Safe power-up:** the relay is set to OFF before its pin becomes an output, so the centrifugal pump never switches on when the board starts.
- **Button handling:** both buttons are debounced, and the sequence will not restart while the start button is still held down.

## Turbidity readings

Each reading averages 20 samples to smooth out noise, then converts the result to a voltage (0 to 5 V). On common analog turbidity sensors, a higher voltage means clearer water, so a positive change from before to after shows the water got cleaner.

## Hardware

| Part | Details |
| --- | --- |
| Microcontroller | Arduino Mega |
| Motor driver | Adafruit Motor Shield (v1) |
| Motors and pumps | Mixing motor (M1), alum pump (M2), clean water pump (M3), filter press motor (M4) |
| Centrifugal pump | Switched through a relay module on A2 |
| Turbidity sensors | 2x analog turbidity sensors on A0 (before) and A1 (after) |
| Buttons | Start on A3, emergency stop on A9, both wired to GND with internal pull-ups |

## Setup

1. Install the **Adafruit Motor Shield library** (AFMotor, for the v1 shield).
2. Open `WaterFiltration/WaterFiltration.ino`, select the Arduino Mega, and upload.
3. Open the Serial Monitor at 9600 baud and press the start button.

If your relay module is active-low, swap the values of `RELAY_ON` and `RELAY_OFF` at the top of the sketch.
