#include <WiFiConfig.h>

#define CENTERX 209
#define CENTERY 217
#define DEADZONE 15
#define APWM 6
#define BPWM 4
#define AX 7
#define AY 5
#define BX 3
#define BY 10
#define TRIG 0
#define ECHO 1
#define BUZZ 8
#define SCAN_STEPS 8
#define STEP_TIME 400

RemoteData data;
bool autonomous = false;
int lastSW = 1;
unsigned long buzzStart = 0;
bool buzzing = false;

void stop(){
  digitalWrite(AX, LOW); digitalWrite(AY, LOW);
  digitalWrite(BX, LOW); digitalWrite(BY, LOW);
  analogWrite(APWM, 0); analogWrite(BPWM, 0);
}

void moveBackward(){
  digitalWrite(AX, LOW); digitalWrite(AY, HIGH);
  analogWrite(APWM, 200);
  digitalWrite(BX, LOW); digitalWrite(BY, HIGH);
  analogWrite(BPWM, 160);
}

void moveForward(){
  digitalWrite(AX, HIGH); digitalWrite(AY, LOW);
  analogWrite(APWM, 200);
  digitalWrite(BX, HIGH); digitalWrite(BY, LOW);
  analogWrite(BPWM, 160);
}

void moveBackwardRight(){
  // Left motor stops, right motor forward
  digitalWrite(AX, LOW); digitalWrite(AY, LOW);
  analogWrite(APWM, 0);
  digitalWrite(BX, LOW); digitalWrite(BY, HIGH);
  analogWrite(BPWM, 160);
}

void moveBackwardLeft(){
  // Left motor forward, right motor stops
  digitalWrite(AX, LOW); digitalWrite(AY, HIGH);
  analogWrite(APWM, 200);
  digitalWrite(BX, LOW); digitalWrite(BY, LOW);
  analogWrite(BPWM, 0);
}

void moveForwardRight(){
  // Left motor stops, right motor backward
  digitalWrite(AX, LOW); digitalWrite(AY, LOW);
  analogWrite(APWM, 0);
  digitalWrite(BX, HIGH); digitalWrite(BY, LOW);
  analogWrite(BPWM, 160);
}

void moveForwardLeft(){
  // Left motor backward, right motor stops
  digitalWrite(AX, HIGH); digitalWrite(AY, LOW);
  analogWrite(APWM, 200);
  digitalWrite(BX, LOW); digitalWrite(BY, LOW);
  analogWrite(BPWM, 0);
}

long pulseSensor(){
  digitalWrite(TRIG, LOW);
  delayMicroseconds(100);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(100);
  digitalWrite(TRIG, LOW);
  long duration = pulseIn(ECHO, HIGH);
  return duration / 58.0;
}

void buzz(){
  digitalWrite(BUZZ, HIGH);
  buzzStart = millis();
  buzzing = true;
}

void endBuzz(){
  if (buzzing && millis() - buzzStart > 200){
    digitalWrite(BUZZ, LOW);
    buzzing = false;
  }
}

bool smartDelay(int ms){
  unsigned long start = millis();
  while (millis() - start < ms){
    if (data.sw == 0 && lastSW == 1){
      autonomous = false;
      lastSW = data.sw;
      stop();
      return true;
    }
    lastSW = data.sw;
    delay(10);
  }
  return false;
}

long scanDirection(bool left){
  if (left) moveForwardLeft(); else moveForwardRight();
  if (smartDelay(800)) return -1;
  stop();
  if (smartDelay(200)) return -1;
  long dist = pulseSensor();
  if (left) moveBackwardLeft(); else moveBackwardRight();
  if (smartDelay(800)) return -1;
  stop();
  buzz();
  if (smartDelay(200)) return -1;
  return dist;
}

void setup(){
  Serial.begin(115200);
  wifi_setup();
  set_data_receiver(&data);
  pinMode(APWM, OUTPUT); pinMode(BPWM, OUTPUT);
  pinMode(AX, OUTPUT); pinMode(AY, OUTPUT);
  pinMode(BX, OUTPUT); pinMode(BY, OUTPUT);
  pinMode(TRIG, OUTPUT); pinMode(ECHO, INPUT);
  pinMode(BUZZ, OUTPUT);
  data.vx = 209;
  data.vy = 217;
  data.sw = 1;
  delay(1000);
}

void loop(){
  if (data.sw == 0 && lastSW == 1) autonomous = !autonomous;
  lastSW = data.sw;
  endBuzz();

  int vx = (int)data.vx;
  int vy = (int)data.vy;

  if (!autonomous){
    if (abs(vx - CENTERX) < DEADZONE && abs(vy - CENTERY) < DEADZONE){
      stop();
      buzz();
    } else if (vy > CENTERY){
      if (abs(vx - CENTERX) < DEADZONE) moveBackward();
      else if (vx > CENTERX) moveBackwardLeft();
      else moveBackwardRight();
    } else {
      if (abs(vx - CENTERX) < DEADZONE) moveForward();
      else if (vx > CENTERX) moveForwardLeft();
      else moveForwardRight();
    }
    delay(20);
  } else {
    long distance = pulseSensor();
    if (distance >= 25){
      moveForward();
      return;
    }

    // Too close to obstacle, back up first
    stop();
    buzz();
    moveBackward();
    if (smartDelay(600)) return;
    stop();
    if (smartDelay(200)) return;

    // Now scan with some breathing room
    long distances[SCAN_STEPS];
    for (int i = 0; i < SCAN_STEPS; i++){
      moveForwardRight();
      if (smartDelay(STEP_TIME)) return;
      stop();
      if (smartDelay(150)) return;
      distances[i] = pulseSensor();
    }

    int bestStep = 0;
    long bestDist = 0;
    for (int i = 0; i < SCAN_STEPS; i++){
      if (distances[i] > bestDist){
        bestDist = distances[i];
        bestStep = i;
      }
    }

    // Everything blocked even after backing up, back up more
    if (bestDist < 15){
      buzz();
      moveBackward();
      if (smartDelay(800)) return;
      stop();
      return; // try again next loop
    }

    // Spin to best direction
    buzz();
    for (int i = 0; i < bestStep; i++){
      moveForwardRight();
      if (smartDelay(STEP_TIME)) return;
    }
    stop();
    if (smartDelay(150)) return;

    moveForward();
    if (smartDelay(500)) return;
  }
}