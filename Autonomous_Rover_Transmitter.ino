#include <WiFiConfig.h>

#define CENTERX 209
#define CENTERY 217
#define DEADZONE 15
#define GREEN 10
#define RED 8
#define SW 3
#define VRY 0
#define VRX 1
RemoteData data;
uint8_t receiverAddress[] = {0xA8, 0x46, 0x74, 0x5C, 0x0F, 0x9C};
bool autoMode = false;
int lastSW = 1;

void setup() {
  Serial.begin(115200);
  wifi_setup();
  peer_setup(receiverAddress);

  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(VRX, INPUT);
  pinMode(VRY, INPUT);
  pinMode(SW, INPUT_PULLUP);

  //Start at Center rather than 0
  data.vx = 147;
  data.vy = 147;
  data.sw = 1;

  delay(1000);
}

void loop() {
  data.vx = map(analogRead(VRX), 0, 4095, 0, 255);
  data.vy = map(analogRead(VRY), 0, 4095, 0, 255);
  data.sw = digitalRead(SW);

  if (data.sw == 0 && lastSW == 1) {
    autoMode = !autoMode;
  }
  lastSW = data.sw;

  if (autoMode) {
    digitalWrite(GREEN, HIGH);
    digitalWrite(RED, LOW);
  } else {
    if ((abs(data.vx - CENTERX) < DEADZONE) && (abs(data.vy - CENTERY) < DEADZONE)) {
      digitalWrite(GREEN, LOW);
      digitalWrite(RED, HIGH);
    }else{
      digitalWrite(GREEN, HIGH);
      digitalWrite(RED, LOW);
    }
  }
  send_data(receiverAddress, data);
  delay(50);
}
