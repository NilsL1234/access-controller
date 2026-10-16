# Requirements v1

## 1. Purpose and Scope

The access controller with the STM32 is primarily a learning project to gain knowledge about microcontrollers, how to program them in C and how to communicate with peripherals. Strong authentication is not intended.
The access controller locks a small door with a bolt that a servo motor can lock and unlock. A reed contact checks if the door is open or closed. 
The STM32 is the controlling unit that controls the servo motor and handles the RFID reader.
RFID is used to unlock the lock when the UID of the card is on the list of allowed UIDs. 
When the door is closed again, it will be locked as well.
A display gives the user information about the current state, for example access denied. 
After a number of unsuccessful tries, the state switches to blocked and no access attempts are possible for a limited amount of time.


## 2. Definitions

- access controller: The whole system

- locked: the bolt blocks the door

- unlocked: the bolt does not block the door

- bolt position sensors: two sensors to detect if the bolt is locking the door or not 

- unlock process: starts with the door being unlocked and ends when the door is successfully locked again

- blocked: access attempts are not possible for a limited amount of time

- door open: The door is physically open

- door closed: The door is physically closed

- door position sensor: one sensor detect if door is open or closed 

- RFID reader: STM32 starts reading process. RFID reader sends RFID-Commands and communicates wirelessly with the RFID card. 
               The STM32 reads this information.

- UID: Unique Identifier

- UID reading area: space where the system can read UID cards

- access attempt: - card is playsed in the range of the reader
                  - UID card is read successfully
                  - system decided authorized/unauthorized
                  - new access attempt possible when card is removed inbetween attempts [open Diagram](./access_control_flow_chart.drawio.svg)

- authorized access attempt: system reads UID from card successfully and UID matches UID in Whitelist

- unauthorized access attempt: system reads UID from card successfully and UID matches no UID in Whitelist

- authorized UID: UID listed in the whitelist

- unauthorized UID: UID not listed in the whitelist

- block couter:     - counts the number of unauthorized access attempts up to a maximum of 3
                    - when 3 is reached the blockinterval starts
                    - resets when maximum is reached, an authorized access attempt or the system is initialized
                
- blockinterval:    - starts when block couter reaches 3 
                    - no new access attempts will be accepted in this interval
                    - length increases each time a blockinterval is entered:    - 30 sec.
                                                                                -  1 min.
                                                                                -  2 min. 
                                                                                -  5 min. (maximum)
                    - gets a reset to 30 sec. when a authorized access attempt happens or the system is initialized again


## 3. Assumptions:

- nothing else blocks the door from opening

- power supply is stable


## 4. Functional Requirements

### 4.1 Initialisation

- RFID reader must be successfully initialized (required), in case of failure see 4.6

- clocking must be initialized (required)

- servo control mus be successfully initialized (required)

- the door and bolt combined must be in a valid state (required)

- the following table lists the possible combinations of the possition of the door and lock; only when the required changes are confirmed by the sensors the initialisation can continue

| door | bolt | required changes | initial state | user feedback |
| --- | --- | --- | --- | --- |
| closed | locked | nothing | LOCKED_READY | X |
| open | locked | unlock door | UNLOCKED_DOOR_OPEN | TBD |
| closed | unlocked | lock door | LOCKED_READY | X |
| open | unlocked |  | UNLOCKED_DOOR_OPEN | X |
| closed | unclear (both sensors active) | lock door | LOCKED_READY | TBD check bolt sensors! |
| closed | unclear (no sensor active) | lock door | LOCKED_READY | TBD |
| open | unclear (both sensors active) | unlock door | TBD | TBD check bolt sensors! |
| open | unclear (no sensor active) | unlock door | UNLOCKED_DOOR_OPEN | TBD |

- initialize the display (not required); in case of failure see 4.6

- software state initialisation (required): - set current state of the access controller 
                                            - whitelist RFIDs
                                            - set current door position
                                            - set current bolt position
                                            - reset block_couter
                                            - reset blockinterval


### 4.2 RFID

- process a RFID card that is held close against the RFID reader while door is initialized, locked and not blocked

- RFID cards will not be processed when: - unlock process is ongoing 
                                         - door is blocked
                                         - initialization is not successfuly finished
                                         - a UID card is being read but not removed from the UID reading area after that

- decide for each access attempt if a card is authorized or unauthorized

- the system must store the authorized UIDs in a whitelist in the Flash

- hand over the authorization result to the unlocking process

- sytem is ready for a new access attempt after the card from the previous access attempt is removed from   the UID reading area or after the locking process is finished in case the door was unlocked


### 4.3 Door and Bolt

- the door gets unlocked by moving the bolt back through a servo motor, after an authorized UID is detected

- detect when the door stays closed after unlocking and lock the door again after 30 seconds (timer starts after estimated time for unlocking the door is over)

- detect that the door was opened after unlocking and locks again after closing

### 4.4 Blocking behavior

- block new access attempts for X seconds after 3 unauthorized access attempts in a row

- store number of unauthorized access attempts in a row (max. 3)

- block counter resets after:  - maximum is reached (3)
                               - authorized access attempt
                               - system gets initialized

- blockinterval inceases each time till a maximum block time is reached (afterwards maximum block time is used till authorized access attempt or new Initialisation)


### 4.5 User Feedback: Screen/Alarm

- the user is given feedback about the current state of the locking system (more details about the display states here [open List](./display_states.md))

- when the door is open for to long after being unlocked the user is informed with an alarm sound (30 seconds after the unlocking process is finished, stops after 1 min)


### 4.6 Error handling



    
## 3.2 Functional Requirements (Physical)

- when locked: bolt stops door from being opened

- when unlocked: bolt does not stop door from being opened

## 6. Non-Functional Requirements

### 6.1 Timing

- holding a UID card against the RFID reader and the start of the unlocking process is less than  TBD ms

- the unlocking process takes less than TBD seconds

- the locking process takes less than TBD seconds

- changes at the door sensor must be processed within TBD ms

- changes at the bolt sensors must be processed within TBD ms

- after the system enters a new state the correct display message should be presented within TBD ms

- timebased actions must be executed within TBD ms: 
    - enter UNLOCKED_DOOR_CLOSED -> timelimit reached -> LOCKING
    - unlocked door is opened -> timeinterval reached -> alarmsound

- communication between components require timeouts
    
### 6.2 Robustness

- 
    
-

### 6.3 Diagnosability

- in case of an error the system should inform about the origin (RFID, Servo/Bolt, Sensor, Display)

- error handling must include relevant diagnose information

### 6.4 Maintainability

- the following components are documented:     - project overview (README.md)
                                               - requirements
                                               - hardwarecomponents
                                               - softwaredesign and architecture
                                               - code documentation

- the code must follow the codestyle which is in the project folder
    

### 6.5 Testability 

- each functional requirement must be verfified by at least one verification method (see 9. Verification) 



## 7. Technical Constraints

- code is written in C

## 8. Security Limitations and Out of Scope

- the system is not prepared for manipulations

## 9. Verification





## 10. Open Decisions and TBDs

- is a sensor for the position of the servo motor required?

- can the system catch the case that the door is opened for just a short period of time like 1 second and lock the door afterwards





