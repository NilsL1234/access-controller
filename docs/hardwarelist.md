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
| OLED | TODO: add the voltage and supply connection used |
| MG90S | 5 V from the external adapter |

All components share GND. The negative output of the servo adapter is also connected to Nucleo GND.
Available supply pins are CN6 pin 4 for 3.3 V and CN6 pin 6 or 7 for GND.

TODO: add the supply/GND pins actually used, the adapter's output current rating and the DC connector polarity.

## Connections

- breadboard
- jumper wires
- copper cable

### Pin connections

These pins match the firmware. The Nucleo header positions are taken from the board manual.
TODO: confirm which header positions are used in the actual setup.

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

*PC0 and PC1 need the board routing through SB56/SB51 ON and SB46/SB52 OFF. They are also available as A5 and A4 on CN8. TODO: confirm the board routing used.

If the reader's chip-select pin is labelled SDA, it is used as NSS/SS in SPI mode.

### Sensors

The sensor contacts connect the GPIO pins to GND. All three inputs use internal pull-ups.

- PC0 LOW: locked end-position sensor active.
- PC1 LOW: unlocked end-position sensor active.
- PC3 LOW: door closed; HIGH: door open.

TODO: add the COM/NO/NC contacts used on the microswitches and confirm the reed contact closes when the door is closed.

### Interface settings

- RFID: SPI2, mode 0, 8-bit, MSB first, 5.25 MHz. NSS and reset are active LOW.
- OLED: I2C1, 100 kHz, 7-bit address 0x3C. TODO: confirm the I2C pull-ups and their supply voltage; internal pull-ups are disabled.
- Servo: TIM2 channel 3, 50 Hz PWM. Position values are defined in `servo.c`. TODO: add the servo wire colours or connector pin order.
- UART: USART2, PA2/TX and PA3/RX, 115200 baud, 8N1, through the onboard ST-LINK virtual COM port.
- Debugging: SWD through the onboard ST-LINK, using PA13/SWDIO and PA14/SWCLK.

Pins and peripherals are configured in [firmware.ioc](../firmware/firmware.ioc).
The RFID interrupt on PC2 is added in [main.c](../firmware/Core/Src/main.c), and UART is initialized in [uart.c](../firmware/Core/Src/uart.c).

Other configured pins are PA5 (onboard LED), PC13 (onboard button), PB3 (SWO) and PB0 (earlier button/EXTI0 exercise).

## Mechanical

- case with door
- sliding bolt
- link between servo and bolt

## Board documentation

[Nucleo board manual](stm32/user-manual-1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf): Figure 24 and Tables 10, 19 and 29 show the connectors and solder-bridge routing.
