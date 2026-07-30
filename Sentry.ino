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

  // check if cooldown has passed to exit alert mode
  if (alertMode && (millis() - lastMotionTime > cooldown)) {
    exitAlertMode();
  }

  if (alertMode) {
    // stay still, buzzer/LED/LCD already set
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