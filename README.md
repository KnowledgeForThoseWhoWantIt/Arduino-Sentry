# Arduino-Sentry
Arduino Mega motion-alert turret, servo sweep, PIR detection, buzzer/LED/LCD alerts, built entirely from a starter kit.
## What it does

Sentry sweeps a servo back and forth like it's scanning an area. The moment its PIR sensor detects motion, it stops mid-sweep, sounds a buzzer, lights an LED, and switches its LCD display to an alert message. After a few seconds with no further motion, it resets and resumes scanning.

## Components

All parts came from an ELEGOO Mega 2560 Ultimate Starter Kit — no additional purchases.

- Arduino Mega 2560
- Micro servo motor
- HC-SR501 PIR motion sensor
- Active buzzer
- LED + resistor
- LCD1602 display

## Wiring

| Component | Connects to |
|---|---|
| Servo — signal | Pin 9 |
| PIR sensor — OUT | Pin 7 |
| Buzzer — + | Pin 8 |
| LED — anode (via resistor) | Pin 6 |
| LCD1602 — RS / E | Pin 12 / Pin 11 |
| LCD1602 — D4–D7 | Pins 5, 4, 3, 2 |
| LCD1602 — V0 (contrast) | Wired directly to GND |
| Shared power | Breadboard 5V / GND rails |

## Behavior

**Scanning** — Servo sweeps 0°–180° continuously. LCD reads "Scanning...". No alert outputs active.

**Alert** — PIR triggers HIGH. Servo holds its position. Buzzer and LED activate, LCD switches to "MOTION DETECTED".

**Cooldown** — After ~4 seconds with no motion, alert clears and the sweep resumes from wherever it left off.

## Code

```cpp
#include <Servo.h>
#include <LiquidCrystal.h>

Servo sweepServo;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int pirPin = 7;
const int buzzerPin = 8;
const int ledPin = 6;
const int servoPin = 9;

int angle = 0;
int direction = 1; // 1 = increasing, -1 = decreasing

unsigned long lastMotionTime = 0;
const unsigned long cooldown = 4000; // ms to stay in alert mode after motion stops

bool alertMode = false;

void setup() {
  pinMode(pirPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  sweepServo.attach(servoPin);
  lcd.begin(16, 2);
  lcd.print("Scanning...");

  Serial.begin(9600);
}

void loop() {
  int motionState = digitalRead(pirPin);

  if (motionState == HIGH) {
    lastMotionTime = millis();
    if (!alertMode) {
      enterAlertMode();
    }
  }

  if (alertMode && (millis() - lastMotionTime > cooldown)) {
    exitAlertMode();
  }

  if (alertMode) {
    delay(50);
  } else {
    sweepStep();
    delay(15);
  }
}

void sweepStep() {
  angle += direction * 1;
  if (angle >= 180) {
    angle = 180;
    direction = -1;
  } else if (angle <= 0) {
    angle = 0;
    direction = 1;
  }
  sweepServo.write(angle);
}

void enterAlertMode() {
  alertMode = true;
  digitalWrite(buzzerPin, HIGH);
  digitalWrite(ledPin, HIGH);
  lcd.clear();
  lcd.print("MOTION DETECTED");
  Serial.println("Motion detected - alert mode");
}

void exitAlertMode() {
  alertMode = false;
  digitalWrite(buzzerPin, LOW);
  digitalWrite(ledPin, LOW);
  lcd.clear();
  lcd.print("Scanning...");
  Serial.println("Clear - resuming scan");
}
```

## Problems encountered

- **Wrong sensor assumed.** Planned the whole project around an HC-SR04 ultrasonic sensor before realizing the kit only had an HC-SR501 PIR sensor — no distance data, just motion. Required redesigning the concept from a radar scanner into a motion-alert sentry.
- **Direct-plug assumption.** Didn't realize components needed jumper wires to bridge into the Arduino's header holes rather than plugging in directly.
- **Active vs. passive buzzer.** The kit included both — each needs different code (simple `digitalWrite` vs. `tone()`), so identifying the right one mattered.
- **Breadboard column mix-up.** Placed a potentiometer's two outer legs in the same column, accidentally shorting them, before understanding that only same-column placement creates a connection.
- **Odd-shaped potentiometer legs.** The trimmer pot's legs were splayed at angles instead of a straight line, making it awkward to seat correctly on the breadboard.
- **Simplified LCD contrast.** Skipped the potentiometer entirely by wiring V0 directly to GND — a simpler fixed-contrast workaround that worked fine.
- **Ran out of GND pins.** Solved by consolidating all grounds onto the breadboard's shared GND rail instead of wiring each one back to the Arduino individually.
- **Merging independent test sketches.** The servo sweep had to change from a blocking loop to a step-by-step non-blocking approach so it could be interrupted by PIR motion detection mid-sweep.

## What's next

- **Add distance sensing back in** — pair the PIR trigger with an ultrasonic sensor so it can report how far away the motion is, not just that it happened.
- **Real enclosure** — move off tape-and-cardboard to a printed or laser-cut housing.
- **Adjustable sensitivity via button** — replace the PIR's physical trimmer pots with a button-cycled setting shown on the LCD.
- **RFID override** — the kit's RC522 reader was never used; a scanned badge could pause alerts, turning this into a lightweight access-aware sentry.
