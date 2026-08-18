# Display States

| State | display time | displayed text |
| --- | --- | --- |
| LOCKED_READY | whole time | "SHOW ID" |
| LOCKED_READY | 2 seconds | "X/3" |
| LOCKED_BLOCKED | blockinterval| "BLOCKED FOR X" |
| UNLOCKING | first 2 seconds | "AUTHORIZED" |
| UNLOCKING | rest of unlocking | "UNLOCKING" |
| UNLOCKED_DOOR_CLOSED | 25 seconds | "UNLOCKED" |
| UNLOCKED_DOOR_CLOSED | last 5 seconds of State | "LOCKS IN X" |
| UNLOCKED_DOOR_OPEN | 30 seconds | "OPEN" |
| UNLOCKED_DOOR_OPEN | 30 seconds | "CLOSE THE DOOR" |
| LOCKING | whole state | "LOCKING" |
| LOCKED_BLOCKED | blockinterval| "BLOCKED FOR X" |