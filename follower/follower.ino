#include <QTRSensors.h>
const int m11Pin = 7;
const int m12Pin = 6;
const int m21Pin = 5;
const int m22Pin = 4;
const int m1Enable = 11;
const int m2Enable = 10;

int m1Speed = 0;
int m2Speed = 0;

int error = 0;
int lastError = 0;

const int maxSpeed = 255;
const int minSpeed = -100;
const int baseSpeed = 220;
const int calibrateMotorSpeed = 150;
int motorSpeed = 0;

float kp = 35;
float ki = 0;
float kd = 17;
int p = 1;
int i = 0;
int d = 0;

int qtrPinAVal = 0;
int qtrPinFVal = 0;

const int qtrPinA = A0;
const int qtrPinF = A5;



const int left = 1;
const int right = 0;
int lastDirection = 0;

int direction = 0;

const int calibrationThreshold = 500;
const int calibrationDuration = 250;

const int maxSensorReading = 5000;
const int minSensorReading = 0;
const int maxSensorMaping = 50;
const int minSensorMaping = -50;

const int errorLowPassFilter = 11.1;

QTRSensors qtr;
const int sensorCount = 6;
int sensorValues[sensorCount];
int sensors[sensorCount] = { 0, 0, 0, 0, 0, 0 };

void setup() {
  pinMode(m11Pin, OUTPUT);
  pinMode(m12Pin, OUTPUT);
  pinMode(m21Pin, OUTPUT);
  pinMode(m22Pin, OUTPUT);
  pinMode(m1Enable, OUTPUT);
  pinMode(m2Enable, OUTPUT);
  qtr.setTypeAnalog();
  qtr.setSensorPins((const uint8_t[]){ A0, A1, A2, A3, A4, A5 }, sensorCount);
  delay(500);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  readInput();
  if (qtrPinAVal > calibrationThreshold) {
    direction = 0;
  } else {
    if (qtrPinFVal > calibrationThreshold) {
      direction = 1;
    }
  }

  for (uint16_t i = 0; i < calibrationDuration; i++) {

    qtr.calibrate();
    if (direction == 1) {
      setMotorSpeed(0, calibrateMotorSpeed);
    } else {
      setMotorSpeed(0, -calibrateMotorSpeed);
    }

    readInput();

    if (qtrPinAVal < calibrationThreshold && qtrPinFVal > calibrationThreshold) {
      direction = 1;
    }
    if (qtrPinAVal > calibrationThreshold && qtrPinFVal < calibrationThreshold) {
      direction = 0;
    }
  }
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(9600);
}

void loop() {

  pidControl();
  m1Speed = constrain(m1Speed, minSpeed, maxSpeed);
  m2Speed = constrain(m2Speed, minSpeed, maxSpeed);
  setMotorSpeed(m1Speed, m2Speed);

  readInput();
}

void pidControl() {

  int error = map(qtr.readLineBlack(sensorValues), minSensorReading, maxSensorReading, minSensorMaping, maxSensorMaping);
  error = -error;
  p = error;
  i = i + error;
  d = error - lastError;
  int motorSpeed = kp * p + ki * i + kd * d;
  m1Speed = baseSpeed;
  m2Speed = baseSpeed;
  if (error <= errorLowPassFilter) {
    m1Speed += motorSpeed;
    m2Speed -= motorSpeed/(kp+kd);
  } else {
    m2Speed -= motorSpeed;
    m1Speed += motorSpeed/(kp+kd);
  }

}

void setMotorSpeed(int motor1Speed, int motor2Speed) {
  if (motor1Speed == 0) {
    digitalWrite(m11Pin, LOW);
    digitalWrite(m12Pin, LOW);
    analogWrite(m1Enable, motor1Speed);
  } else {
    if (motor1Speed > 0) {
      digitalWrite(m11Pin, HIGH);
      digitalWrite(m12Pin, LOW);
      analogWrite(m1Enable, motor1Speed);
    }
    if (motor1Speed < 0) {
      digitalWrite(m11Pin, LOW);
      digitalWrite(m12Pin, HIGH);
      analogWrite(m1Enable, -motor1Speed);
    }
  }
  if (motor2Speed == 0) {
    digitalWrite(m21Pin, LOW);
    digitalWrite(m22Pin, LOW);
    analogWrite(m2Enable, motor2Speed);
  } else {
    if (motor2Speed > 0) {
      digitalWrite(m21Pin, HIGH);
      digitalWrite(m22Pin, LOW);
      analogWrite(m2Enable, motor2Speed);
    }
    if (motor2Speed < 0) {
      digitalWrite(m21Pin, LOW);
      digitalWrite(m22Pin, HIGH);
      analogWrite(m2Enable, -motor2Speed);
    }
  }
}

void readInput() {
  qtrPinAVal = analogRead(qtrPinA);
  qtrPinFVal = analogRead(qtrPinF);
}