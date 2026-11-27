# Requirements

## 1. Purpose and Scope

The access controller with the STM32 is primarily a learning project to gain knowledge about microcontrollers, how to program them in C and how to communicate with peripherals. Strong authentication is not intended.

The access controller locks a small door with a bolt that a servo motor can lock and unlock. A reed contact checks if the door is open or closed.
The STM32 is the controlling unit that controls the servo motor and handles the RFID reader.
RFID is used to unlock the lock when the UID of the card is on the list of allowed UIDs.
When the door is closed again, it will be locked as well.
A display gives the user information about the current state, for example access denied.
After a number of unsuccessful tries, the state switches to blocked and no access attempts are possible for a limited amount of time.

Deviations between these requirements and the implementation are marked with **Deviation**.

## 2. Definitions

- **access controller**: the whole system
- **locked**: the bolt blocks the door
- **unlocked**: the bolt does not block the door
- **bolt position sensors**: two micro switches that detect the end positions of the bolt (locked / unlocked)
- **door position sensor**: one reed contact that detects if the door is open or closed
- **unlock process**: starts with the door being unlocked and ends when the door is successfully locked again
- **blocked**: access attempts are not possible for a limited amount of time
- **door open / door closed**: the door is physically open / closed
- **RFID reader**: the STM32 starts the reading process; the reader communicates wirelessly with the card and the STM32 reads the result
- **UID**: Unique Identifier of an RFID card
- **UID reading area**: space where the system can read cards
- **access attempt**: a card is placed in the reading area, its UID is read successfully and the system decides authorized / unauthorized ([flow chart](./access_control_flow_chart.svg))
- **authorized access attempt**: the UID matches a UID in the whitelist
- **unauthorized access attempt**: the UID matches no UID in the whitelist
- **block counter**:
  - counts unauthorized access attempts in a row, up to a maximum of 3
  - when 3 is reached, the block interval starts
  - resets when the maximum is reached, after an authorized access attempt or when the system is initialized
- **block interval**:
  - starts when the block counter reaches 3
  - no access attempts are accepted during this interval
  - starts at 10 s and doubles after each block interval, up to a maximum of 5 min (10 s → 20 s → 40 s → 80 s → 160 s → 300 s)
  - resets to 10 s after an authorized access attempt or when the system is initialized

## 3. Assumptions

- nothing else blocks the door from opening
- the power supply is stable

## 4. Functional Requirements

### 4.1 Initialization

- clocks, GPIO, timer, SPI and I²C are initialized by the CubeMX code (required)
- servo control (PWM) must be started successfully (required); in case of failure see 4.6
- the RFID reader must be initialized successfully (required); in case of failure see 4.6
- the display is initialized (not required); in case of failure see 4.6
- door and bolt are read after debouncing, and the initial state is chosen from them:

| door | bolt | action | initial state |
| --- | --- | --- | --- |
| closed | locked | none | LOCKED_READY |
| closed | unlocked | lock the door | LOCKING |
| closed | between (no sensor active) | lock the door | LOCKING |
| open | locked | unlock the door | UNLOCKING |
| open | unlocked | none | UNLOCKED_DOOR_OPEN |
| open | between (no sensor active) | unlock the door | UNLOCKING |
| any | invalid (both sensors active) | stop | FAULT |

- software state initialization: controller state, door and bolt state, block counter and block interval are set to their initial values; the whitelist is stored as a constant

### 4.2 RFID

- a card is processed while the system is locked and not blocked (LOCKED_READY)
- cards are not processed while the door is unlocked or moving, while the system is blocked, during initialization and in FAULT
- each read UID is classified as authorized or unauthorized
- the authorized UIDs are stored in a whitelist in flash
- only 4-byte UIDs are supported
- an authorized UID starts the unlock process
- **Deviation**: the definition of an access attempt requires the card to be removed between two attempts. This is not implemented. A card that stays in the reading area is read again after 100 ms and counts as a new access attempt.

### 4.3 Door and Bolt

- after an authorized UID, the servo moves the bolt back and the door is unlocked
- if the door stays closed after unlocking, the door is locked again after 30 s (the timer starts when the bolt has reached the unlocked position)
- if the door is opened after unlocking, it is locked again as soon as it is closed
- if the door is opened while locking, the bolt is moved back (unlocking)
- the bolt is never moved to the locked position while the door is open

### 4.4 Blocking behavior

- after 3 unauthorized access attempts in a row, new access attempts are blocked for the block interval
- the number of unauthorized access attempts in a row is stored (max. 3)
- the block counter resets when the maximum is reached, after an authorized access attempt and when the system is initialized
- the block interval increases with each block up to a maximum (see 2.)

### 4.5 User Feedback

- the user gets feedback about the current state on the display ([display states](./display_states.md))
- **Out of scope**: an alarm sound when the door stays open too long is not implemented

### 4.6 Error handling

Errors are detected where they occur and handled in one place (`HandleFault()` in the access controller).

| Error | Detected by | Reaction |
| --- | --- | --- |
| both bolt sensors active | sensors | FAULT |
| bolt does not reach the unlocked position within 500 ms | access controller | FAULT |
| bolt does not reach the locked position within 500 ms | access controller | FAULT |
| locking requested although the sensor state does not allow it | servo | FAULT |
| RFID reader initialization fails | access controller | FAULT |
| RFID communication error (SPI, FIFO) during operation | RFID driver | FAULT |
| invalid parameters for UID validation | RFID driver | FAULT |
| invalid controller state | access controller | FAULT |
| PWM for the servo cannot be started | main | error log, system halts (`Error_Handler()`) |
| display initialization fails | main | warning log, system continues without display |
| disturbed card response (wrong length, checksum) | RFID driver | new read attempt, no error |

FAULT behavior:

- the servo PWM is stopped; the bolt stays where it is
- the cause is logged over UART and "ERROR" is shown on the display
- FAULT is latched: the system only leaves FAULT after a reset

### 4.7 Physical requirements

- when locked, the bolt prevents the door from being opened
- when unlocked, the bolt does not prevent the door from being opened

## 5. Non-Functional Requirements

### 5.1 Timing

Values as implemented. They are design values and were not measured.

| Item | Value |
| --- | --- |
| pause between two RFID read attempts | 100 ms |
| timeout for a card response | 20 ms |
| SPI transfer timeout | 10 ms |
| I²C transfer timeout | 50 ms |
| UART timeout per byte | 5 ms |
| maximum time for unlocking / locking | 500 ms each |
| automatic locking after unlocking without opening the door | 30 s |
| debounce time bolt sensors | 20 ms |
| debounce time door sensor | 50 ms |
| display time of "ERROR:x/3" | 300 ms |
| display update with text (estimated) | about 0.1 s |

- all communication with peripherals uses timeouts
- time-based actions are checked in every loop; the main loop only blocks during display updates and log output

### 5.2 Robustness

- sensor inputs are debounced
- contradictory sensor states lead to FAULT
- disturbed card responses are retried without counting as an access attempt
- the bolt is not moved out while the door is open

### 5.3 Diagnosability

- state changes, sensor changes, access attempts, blocking and errors are logged over UART with a timestamp
- errors include their cause (RFID, servo/bolt, sensors, display)
- log format: `[<time> ms] <LEVEL> <SOURCE>: <message>`

### 5.4 Maintainability

- the project is documented: [README](../README.md), requirements, [hardware](hardwarelist.md), [architecture](architecture.md), [code style](codestyle.md)
- the code follows the [code style](codestyle.md)
- own code is compiled with `-Wall -Wextra` without warnings

### 5.5 Testability

- each functional requirement is verified by at least one test (see 8.)

## 6. Technical Constraints

- the code is written in C (C11)
- NUCLEO-F446RE (STM32F446RE, Cortex-M4)
- bare metal, no operating system

## 7. Security Limitations and Out of Scope

- the system is not protected against manipulation
- the UID is not a secure authentication: it is transmitted unencrypted and can be copied
- UIDs are logged in plain text for debugging
- a broken door sensor wire reads as "door open"; the door is then not locked
- no alarm sound
- error flags reported by the MFRC522 itself (CRC, collision, parity) are not evaluated

## 8. Verification

Manual tests on the hardware. UART log and display are used to check the result.

| # | Test | Expected result | Requirement | Result |
| --- | --- | --- | --- | --- |
| 1 | start with door closed and bolt locked | LOCKED_READY, "SHOW ID" | 4.1 | pass |
| 2 | start with door closed and bolt unlocked | LOCKING, then LOCKED_READY | 4.1 | pass |
| 3 | start with door open | UNLOCKING or UNLOCKED_DOOR_OPEN | 4.1 | pass |
| 4 | authorized card | bolt unlocks, "UNLOCKED", UID logged | 4.2, 4.3 | pass |
| 5 | unlock, leave door closed | locks again after 30 s | 4.3 | pass |
| 6 | unlock, open and close the door | locks again after closing | 4.3 | pass |
| 7 | open the door right after locking started | bolt moves back | 4.3 | pass |
| 8 | unauthorized card | "ERROR:1/3", attempt logged | 4.2, 4.4 | pass |
| 9 | 3 unauthorized cards | "BLOCKED" for 10 s, then "SHOW ID" | 4.4 | pass |
| 10 | block twice in a row | second block lasts 20 s | 4.4 | pass |
| 11 | authorized card after unauthorized attempts | counter and block interval reset | 4.4 | pass |
| 12 | card moved quickly past the reader | no FAULT | 4.6 | pass |
| 13 | both bolt sensors active | FAULT, "ERROR", cause logged | 4.6 | pass |
| 14 | bolt blocked mechanically while moving | FAULT after 500 ms | 4.6 | pass |
| 15 | RFID reader disconnected during operation | FAULT, cause logged | 4.6 | pass |
| 16 | display disconnected at startup | warning logged, system works without display | 4.6 | pass |

## 9. Open Decisions

- **Position sensor for the servo**: not used. The two end-position switches and the 500 ms timeout detect if the bolt reaches its position.
- **Door opened for a short time only**: door changes shorter than the debounce time (50 ms) are not detected.
