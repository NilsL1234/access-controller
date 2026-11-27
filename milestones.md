# STM32 Access Controller - Development milestones

I built this project step by step to learn embedded software development. These milestones describe the main steps I followed to finish the first version.

Some steps overlapped. I organized the code into modules during development and started building the application state machine alongside the RFID and OLED work.

## Development steps

### M1 - Development environment and board

- Set up the NUCLEO-F446RE and learned the project structure.
- Used VS Code on macOS with CMake and Ninja for building the firmware.
- Used STM32CubeMX for clock, pin and peripheral configuration.
- Used GCC and GDB, with Cortex-Debug and the onboard ST-LINK for debugging over SWD.

### M2 - Building and flashing firmware

- Built a first firmware project and flashed it to the board.
- Learned the difference between building, flashing and starting the program.
- Looked at the startup code, initialization and `main()`.
- Checked how the program behaves after a reset.

### M3 - GPIO output and LED

- Configured a GPIO output and controlled the onboard LED.
- Used HAL functions and direct register access to compare the two approaches.
- Worked with GPIO clock enable, `MODER` and `BSRR`.

### M4 - Debugging

- Used breakpoints and stepped through the program.
- Observed variables and peripheral registers.
- Used the debugger to compare the expected behaviour with the actual program state.

### M5 - GPIO input and button

- Connected a button and configured a GPIO input.
- Read HIGH and LOW states and detected changes.
- Observed the input register in the debugger.

### M6 - Pull resistors and debounce

- Learned about floating inputs, pull-ups and pull-downs.
- Added debounce for the button.
- Implemented debounce without blocking delays.

### M7 - Time base and timeouts

- Used `HAL_GetTick()` to measure time intervals.
- Implemented timed behaviour without stopping the main loop.
- Learned how clock configuration and SysTick relate to the software time base.
- Worked with unsigned time differences and timer overflow.

### M8 - Interrupts

- Configured EXTI and NVIC and wrote interrupt handlers.
- Compared polling with interrupt-based input handling.
- Kept interrupt handlers short and processed further work in the main loop.
- Worked with state shared between an interrupt handler and the main program.

### M9 - UART and diagnostics

- Implemented UART communication using direct register access.
- Sent characters, strings and numbers to the computer through the ST-LINK virtual COM port.
- Added interrupt-based reception and a receive ring buffer.
- Used UART messages to observe program behaviour.

The final firmware only sends over UART. Reception and the ring buffer were removed during the cleanup and are preserved at the tag `learning-exercises`.

### M10 - Servo, timer and PWM

- Configured TIM2 channel 3 for the MG90S servo.
- Calculated the timer clock, prescaler, period and pulse width.
- Worked with `PSC`, `ARR`, `CNT` and `CCR`.
- Tested servo positions using an external 5 V supply and common GND.

### M11 - Door and bolt sensors

- Added the two bolt end-position switches and a sensor module.
- Used GPIO inputs with pull-ups and non-blocking debounce.
- Distinguished locked, unlocked, between-position and contradictory sensor states.
- Added the door reed contact later as part of the system integration.

### M12 - SPI basics

- Learned the roles of SCK, MOSI, MISO and chip select.
- Worked with clock polarity, clock phase, bit order and transfer speed.
- Explored SPI transfers through direct register access.
- Used HAL for SPI communication in the RFID driver and controlled chip select as a GPIO.

### M13 - MFRC522 driver

- Used the data sheet to implement register reads, writes and bit access.
- Implemented reader initialization and FIFO access.
- Implemented REQA, ATQA processing and anticollision for reading 4-byte UIDs.
- Added response timeouts, FIFO length checks and BCC validation.
- Compared UIDs with a whitelist defined in the firmware.

The application state machine was developed alongside this work, so the reader could be integrated into the access-control flow.

### M14 - I2C and SSD1306 driver

- Learned I2C addressing, START/STOP and ACK/NACK.
- Implemented display initialization and functions for sending commands and data, using HAL for I2C transfers.
- Worked with display RAM, pages and character patterns.
- Worked out a first character pattern myself and used AI-generated character data for the remaining text.
- Integrated the display into the application to show system states.

### M15 - Module structure

- Organized the code into modules for the button, UART, sensors, RFID, servo, OLED and application logic.
- Used `.c` and `.h` files to define module interfaces.
- Used `static`, `const`, enums and structs as they became useful in the project.

This structure grew during the earlier milestones. It was not a separate step after all drivers were finished.
The button module belonged to the GPIO and interrupt exercises and was removed during the cleanup.

### M16 - State machine and system integration

- Defined application states and transitions.
- Combined RFID results, the whitelist, sensor feedback, servo control and display output.
- Added the access-attempt counter, blocking intervals and movement timeouts.
- Built and tested the normal unlocking and locking flow while integrating the components.

This work overlapped with M13 and M14. Error handling was added along the way and completed in M17.

## Testing during development

Testing was mainly manual and done during development. I used the real hardware, the debugger and UART output to check behaviour. I changed the code and tested it again until the tested function behaved as intended.

I considered possible errors while writing the code and added checks for them. Towards the end, the focus was on getting the complete system working. The remaining error checks were completed in M17.

## Finishing the first version

### M17 - Cleanup and error handling

- Removed the learning exercises from `main.c` and kept them at the tag `learning-exercises`.
- Built the own code with `-Wall -Wextra` and removed all warnings.
- Handled all faults in one function: stop the servo, store and log the cause, show "ERROR" on the display.
- Made the fault state latched, so the system only leaves it after a reset.
- Defined clear return values for the RFID detection and stored the detailed reason of a failed read separately.
- Handled disturbed card responses inside the driver as a new read attempt instead of an error.
- Never move the bolt out while the door is open, and always allow moving it back.
- Checked the return values of the driver functions, including the start of the servo PWM.
- Skipped display updates if the display failed to initialize, so the system keeps running without it.
- Added logs for state changes, sensor changes, access attempts and blocking.
- Limited the block interval to a maximum of 5 minutes.

### M18 - Final manual tests

- Tested the main functions and the important error cases on the assembled hardware.
- Used the UART log and the display to compare the expected and the actual behaviour.
- Recorded the test cases and results in [docs/requirements.md](docs/requirements.md), section 8.

### M19 - Documentation and portfolio

- Finished the README with hardware, build steps, project structure and known limitations.
- Distinguished my own code from generated code and libraries.
- Checked the implementation against the requirements and documented the differences.
- Added the architecture, code style and display state documents.
- Added photos of the setup.
- Added the MIT license for my own code.

### M20 - Explaining the project

- Be able to explain the architecture and the main technical decisions.
- Explain the roles of CubeMX, HAL, CMSIS and the development tools.
- Explain the RFID communication, PWM, interrupts, sensors and state machine.
- Describe examples of debugging, the tests performed and the project's limitations.

## Possible later extensions

These are optional follow-up tasks after the first version:

- Count a new access attempt only after the card was removed from the reader.
- Evaluate the MFRC522 error register (CRC, collision, parity).
- Re-initialize the RFID reader automatically after a communication error.
- Add an alarm sound when the door stays open too long.
- Render display texts from a string instead of sending each character separately, and add the missing texts (remaining block time, countdown before locking).
- Speed up display updates (I²C fast mode with 400 kHz, or non-blocking transfers with DMA) and start the servo movement timer after the display update.
- Store and change the whitelist in Flash without rebuilding the firmware.
- Add a watchdog and test the restart behaviour.
- Add automated tests for the whitelist, sensor logic and state transitions.
- Extend RFID reading to 7-byte and 10-byte UIDs.
