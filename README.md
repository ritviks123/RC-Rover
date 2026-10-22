# RC Rover

A joystick-controlled rover with an obstacle-avoiding auto mode. Built solo as the capstone for IEEE at UC Irvine's Open Project Space.

Two ESP32-C3 boards talk over **ESP-NOW** (a direct board-to-board Wi-Fi link, no router needed). The remote runs on a **custom PCB I designed in KiCad** and had manufactured. The rover is wired on a breadboard.

- **Write-up:** https://ritviksingh.net/projects/autonomous-rc-rover

## Features

| Feature | How it works |
|---|---|
| **Manual mode** | The joystick drives forward, backward and turns. A deadzone around the joystick's resting point stops the rover from creeping. |
| **Auto mode** | Press the joystick button. The rover drives forward until something is closer than 25 cm, then backs up, scans 8 directions and heads toward the clearest one. |
| **Instant takeover** | Press the button again to stop auto mode, even mid-scan. Every wait in auto mode checks the button every 10 ms instead of freezing the board. |
| **Backup beeper** | A buzzer sounds when the rover stops or backs up. |
| **Mode LEDs** | On the remote: green when the rover is moving or in auto mode, red when it's idle in manual mode. |
| **Motor trim** | Motor A runs at PWM 200 and motor B at 160, because one motor is weaker. Trimming them makes the rover drive straight. |

## Auto mode, step by step

1. Measure the distance ahead with the ultrasonic sensor.
2. Clear for 25 cm or more: drive forward.
3. Closer than 25 cm: stop, beep, back up.
4. Turn in 8 short steps, measuring the distance at each one.
5. Every direction under 15 cm: back up further and try again.
6. Otherwise: turn to the clearest direction and drive forward.

## Hardware

**Remote (custom PCB)**
- ESP32-C3
- Analog joystick (with push button)
- Green and red LEDs

**Rover (breadboard)**
- ESP32-C3
- L293D H-bridge motor driver
- 2 gearbox motors with wheels, ball caster
- Ultrasonic distance sensor
- Buzzer

## Pin map

**Remote** (`Autonomous_Rover_Transmitter.ino`)

| Signal | GPIO |
|---|---|
| Joystick VRX | 1 |
| Joystick VRY | 0 |
| Joystick SW (internal pull-up) | 3 |
| Green LED | 10 |
| Red LED | 8 |

**Rover** (`Autonomous_Rover_Receiver.ino`)

| Signal | GPIO |
|---|---|
| Motor A speed (L293D enable, PWM) | 6 |
| Motor A direction | 7, 5 |
| Motor B speed (L293D enable, PWM) | 4 |
| Motor B direction | 3, 10 |
| Ultrasonic TRIG | 0 |
| Ultrasonic ECHO | 1 |
| Buzzer | 8 |

## Files

| File | What it is |
|---|---|
| `Autonomous_Rover_Transmitter.ino` | Remote: reads the joystick (12-bit ADC scaled to 0 to 255), sets the LEDs, sends a packet every 50 ms |
| `Autonomous_Rover_Receiver.ino` | Rover: manual driving, auto mode, buzzer |
| `capstone_Ritvik_Singh.zip` | KiCad project for the remote PCB |

## Setup

1. Install the Arduino IDE (2.x) and ESP32 board support, then select your ESP32-C3 board.
2. Install the `WiFiConfig` library. It handles the ESP-NOW link and defines `RemoteData`. It is not my code.
3. In `Autonomous_Rover_Transmitter.ino`, replace `receiverAddress` with your rover board's MAC address.
4. Calibrate `CENTERX` and `CENTERY` in both sketches to your joystick's resting values (mine rests at 209 and 217). Print `data.vx` and `data.vy` over Serial to find yours.
5. Upload the receiver sketch to the rover and the transmitter sketch to the remote.

## Credits

- Project spec: IEEE at UC Irvine, Open Project Space
- `WiFiConfig` library: not written by me
