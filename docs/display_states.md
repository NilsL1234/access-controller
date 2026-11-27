# Display States

Texts shown on the SSD1306 OLED, as implemented in [oled.c](../firmware/Core/Src/oled.c).
The display is updated on every state change.

| State | Displayed text | Display time |
| --- | --- | --- |
| UNINITIALIZED | empty display (cleared during display initialization) | until the sensors are debounced (startup) |
| LOCKED_READY | "SHOW ID" | whole state |
| LOCKED_READY, after an unauthorized card | "ERROR:1/3" or "ERROR:2/3" | 300 ms, then "SHOW ID" again |
| LOCKED_BLOCKED | "BLOCKED" | whole block interval |
| UNLOCKING | no own text; the previous text stays (usually "SHOW ID") | while the bolt is moving (max. 500 ms) |
| UNLOCKED_DOOR_CLOSED | "UNLOCKED" | until the door is opened or 30 s have passed |
| UNLOCKED_DOOR_OPEN | "DOOR OPEN" | until the door is closed |
| LOCKING | no own text; the previous text stays ("UNLOCKED" or "DOOR OPEN") | while the bolt is moving (max. 500 ms) |
| FAULT | "ERROR" | until reset |

The third unauthorized attempt switches directly to LOCKED_BLOCKED, so "ERROR:3/3" is not shown.

## Not implemented

The following texts were planned but are not implemented:

- "AUTHORIZED" / "UNLOCKING" while unlocking and "LOCKING" while locking
- remaining block time ("BLOCKED FOR X")
- countdown before automatic locking ("LOCKS IN X")
- "CLOSE THE DOOR" when the door stays open too long

The display is optional. If it fails to initialize, the system continues without it (see [requirements](requirements.md), 4.6).
