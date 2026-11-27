# Hardwarelist

## Controller

- 1x NUCLEO-F446RE

## RFID

- 1x MFRC522 RFID reader
- 2x RFID tags/cards

## Display

- 1x SSD1306 OLED
  - 0.96"
  - 128x64px
  - I2C

## Servo

- 1x MG90S servo
- servo horn

## Sensors

- 2x microswitches for endposition of bolt (locked/unlocked)
- 1x reed sensor for door position (open/closed)

## Power supply

- USB cable (Nucleo power supply and ST-LINK connection)
- 5 V DC adapter (servo)

| Component | Supply |
| --- | --- |
| Nucleo | USB through the ST-LINK connector CN1 |
| MFRC522 | 3.3 V from the Nucleo |
| OLED | 3.3 V from the Nucleo through the breadboard supply rail |
| MG90S | 5 V from the external adapter |

All components share GND. The negative output of the servo adapter is also connected to Nucleo GND.
Available supply pins are CN6 pin 4 for 3.3 V and CN6 pin 6 or 7 for GND.
The servo is powered by an external 5 V adapter, not by the Nucleo, because its current peaks are too high for the USB supply.

## Connections

- breadboard
- jumper wires
- copper cable

### Pin connections

These pins match the firmware. The Nucleo header positions are taken from the board manual.

| Component | Connection | STM32 pin | Nucleo header |
| --- | --- | --- | --- |
| MFRC522 | SCK | PB13 | CN10 pin 30 |
| MFRC522 | MISO | PB14 | CN10 pin 28 |
| MFRC522 | MOSI | PB15 | CN10 pin 26 |
| MFRC522 | NSS / SS | PB12 | CN10 pin 16 |
| MFRC522 | IRQ | PC2 | CN7 pin 35 |
| MFRC522 | RST | PC6 | CN10 pin 4 |
| OLED | SCL | PB6 | CN10 pin 17 |
| OLED | SDA | PB7 | CN7 pin 21 |
| Servo | PWM signal | PB10 | CN10 pin 25 |
| Bolt sensor locked | Signal | PC0 | CN7 pin 38* |
| Bolt sensor unlocked | Signal | PC1 | CN7 pin 36* |
| Door reed contact | Signal | PC3 | CN7 pin 37 |

*PC0 and PC1 need the board routing through SB56/SB51 ON and SB46/SB52 OFF. They are also available as A5 and A4 on CN8. The board is used with its default solder-bridge configuration.

If the reader's chip-select pin is labelled SDA, it is used as NSS/SS in SPI mode.

### Sensors

The sensor contacts connect the GPIO pins to GND. All three inputs use internal pull-ups.

- PC0 LOW: locked end-position sensor active.
- PC1 LOW: unlocked end-position sensor active.
- PC3 LOW: door closed; HIGH: door open.

The micro switches close when pressed (COM and NO). The reed contact is normally open and closes when the door is closed.

### Interface settings

- RFID: SPI2, mode 0, 8-bit, MSB first, 5.25 MHz. NSS and reset are active LOW.
- OLED: I2C1, 100 kHz, 7-bit address 0x3C. Internal pull-ups are disabled; the I²C pull-ups are on the OLED module (the bus does not work without the display connected).
- Servo: TIM2 channel 3, 50 Hz PWM. Position values are defined in `servo.c`. Servo wires: black = GND, red = 5 V from the external adapter, white = PWM signal (PB10).
- UART: USART2, PA2/TX and PA3/RX, 115200 baud, 8N1, through the onboard ST-LINK virtual COM port.
- Debugging: SWD through the onboard ST-LINK, using PA13/SWDIO and PA14/SWCLK.

Pins and peripherals are configured in [firmware.ioc](../firmware/firmware.ioc).
The RFID interrupt on PC2 is added in [main.c](../firmware/Core/Src/main.c), and UART is initialized in [uart.c](../firmware/Core/Src/uart.c).

Other configured pins are PA5 (onboard LED), PC13 (onboard button), PB3 (SWO) and PB0 (earlier button/EXTI0 exercise). They are still configured by CubeMX but not used by the firmware.

## Mechanical

- case with door
- sliding bolt
- link between servo and bolt

## Board documentation

[UM1724 – STM32 Nucleo-64 boards (MB1136)](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf) (ST user manual): the sections "Solder bridges", "ARDUINO connectors" and "ST morpho connector" show the connectors and solder-bridge routing.
Board page: [NUCLEO-F446RE](https://www.st.com/en/evaluation-tools/nucleo-f446re.html)
