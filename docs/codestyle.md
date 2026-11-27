# Code Style

Conventions used in the own code of this project (`firmware/Core/Src`, `firmware/Core/Inc`).
Generated CubeMX code keeps its own style.

## Language and build

- C11
- own sources are compiled with `-Wall -Wextra` and must build without warnings
- own source files are added to `APP_SOURCES` in [firmware/CMakeLists.txt](../firmware/CMakeLists.txt)

## Modules

- one module per component: `<module>.c` for the implementation, `<module>.h` for the public interface
- headers use include guards (`#ifndef MODULE_H`)
- module-internal functions and variables are `static`
- public functions start with the module prefix, e.g. `SERVO_LockDoor()`, `SENSORS_GetDoorState()`, `MFRC522_DetectUID()`, `ACCESS_CONTROLLER_Run()`
- module state is kept in `static` variables and accessed through getter functions

## Naming

| Element | Style | Example |
| --- | --- | --- |
| public function | `PREFIX_PascalCase` | `SSD1306_DisplayCurrentState()` |
| static function | `PascalCase` | `ChangeState()`, `HandleFault()` |
| variable, parameter | `PascalCase` | `CurrentBlockCounter`, `DetectedUID` |
| constant | `static const`, `PascalCase` | `MaxBlockCounter` |
| macro | `UPPER_CASE` | `MAX_UID_LENGTH` |
| enum value | `UPPER_CASE` with module prefix | `MFRC522_UID_DETECTED`, `BOLT_LOCKED` |

## Types and values

- fixed-width types from `<stdint.h>` (`uint8_t`, `uint32_t`), `size_t` for sizes and indices, `bool` from `<stdbool.h>`
- unsigned literals with `U` suffix (`500U`)
- variables shared with an interrupt handler are `volatile`
- time checks use `(HAL_GetTick() - StartTick) >= Interval` (safe across the `uint32_t` overflow)

## Error handling

- each module has its own status enum; success is `<PREFIX>_OK = 0`
- callers check for `!= <PREFIX>_OK`, not for a specific error value
- data is returned through pointer parameters, the return value is the status
- pointer parameters are checked for `NULL` in new functions (e.g. `MFRC522_ValidateUID()`, `SSD1306_SendData()`); older driver functions do not check them yet
- ignored return values are marked with `(void)`
- faults of the access controller are handled only in `HandleFault()`

## Comments

- functions are described above their definition:

```c
/*
FunctionName:
- what the function does
- important behaviour
*/
```

- public functions with a non-obvious contract are documented in the header in Doxygen format (`@brief`, `@param`, `@retval`, `@note`), e.g. `MFRC522_DetectUID()`
- comments explain why, not what the code obviously does
- no commented-out code and no open TODOs in the final code

## Known deviations

- the UART module uses `Uart_` instead of `UART_` as function prefix
- type names are not fully consistent (`ACCESS_CONTROLLER_State`, `MFRC522Status`, `Servo_Status`)

## Generated code

- own code in CubeMX files is placed only inside `USER CODE BEGIN` / `USER CODE END` blocks, otherwise it is overwritten when the code is regenerated
