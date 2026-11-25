#ifndef RFID_H
#define RFID_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX_UID_LENGTH 10U
#define UID_LENGTH_4 4U
#define UID_LENGTH_7 7U
#define UID_LENGTH_10 10U

// register addresses

static const uint8_t RegBitLength = 8U;
static const uint8_t LastValidRegAddress = 0x3B;

static const uint8_t CommandRegAddress = 0x01U;
static const uint8_t ComIEnRegAddress = 0x02U;
static const uint8_t ComIrqRegAddress = 0x04U;
static const uint8_t ErrorRegAddress = 0x06U;
static const uint8_t Status1RegAddress = 0x07U;
static const uint8_t Status2RegAddress = 0x08U;
static const uint8_t FIFODataRegAddress = 0x09U;
static const uint8_t FIFOLevelRegAddress = 0x0AU;
static const uint8_t BitFramingRegAddress = 0x0DU;
static const uint8_t RxModeRegAddress = 0x13U;
static const uint8_t TxControlRegAddress = 0x14U;
static const uint8_t TxASKRegAddress = 0x15U;

typedef enum
{
    MFRC522_OK = 0x00,
    MFRC522_ERROR = 0x01,
    MFRC522_BUSY = 0x02,
    MFRC522_TIMEOUT = 0x03,
    MFRC522_INVALID_REG_ACCESS = 0x04,
    MFRC522_INVALID_REG_ADDRESS = 0x05,
    MFRC522_INVALID_BIT_POSITION = 0x06,
    MFRC522_INVALID_COMMAND = 0x07,
    MFRC522_FIFO_BUFFER_ERROR = 0x08,
    MFRC522_RFID_INVALID_STATE_ERROR = 0x09,
    MFRC522_RFID_BCC_ERROR = 0x0A,
    MFRC522_INVALID_UID_LENGTH = 0x0B,
    MFRC522_UID_DETECTED = 0x0C,
    MFRC522_NO_UID_DETECTED = 0x0D,
    MFRC522_INVALID_ATQA_RESP_LENGTH = 0x0E,
    MFRC522_INVALID_ANTI_CL1_RESP_LENGTH = 0x0F,
    MFRC522_FIFO_FULL = 0x10,
    MFRC522_FIFO_EMPTY = 0x11,
    MFRC522_FIFO_READ_ERROR = 0x12,
    MFRC522_FIFO_WRITE_ERROR = 0x13

} MFRC522Status;

typedef enum
{
    IDLE_COMMAND = 0x00,
    TRANSCEIVE_COMMAND = 0x01,
    SOFT_RESET_COMMAND = 0x02
    // Add required commands

} MFRC522Command;

// better naming
typedef enum
{
    RFID_OFF = 0x00, //  InitTransceive() not completed
    RFID_INIT = 0x01, // InitTransceive() completed
    RFID_REQA_SEND = 0x02, // REQA is transmitted
    RFID_ATQA_WAITING = 0x03, // waiting for ATQA interrupt
    RFID_ATQA_AVAILABLE = 0x04,
    RFID_ANTICOLLISION_CL1_SEND = 0x05,
    RFID_ANTICOLLISION_CL1_WAITING = 0x06,
    RFID_ANTICOLLISION_CL1_AVAILABLE = 0x07,
    RFID_DETECTED = 0x08,
    RFID_IDLE = 0x09
    // ERRORS
} UIDSearchState;

typedef struct {
    uint8_t UID [MAX_UID_LENGTH];
    size_t UIDLength;
} UID;

// Setter & Getter

UIDSearchState MFRC522_GetCurrentUIDSearchState(void);

void MFRC522_SetCurrentUIDSearchState(UIDSearchState State);

MFRC522Status MFRC522_GetLastError(void);

// Register reading/writing

MFRC522Status MFRC522_ReadReg(uint8_t RegAddress, uint8_t *RegData);

MFRC522Status MFRC522_WriteReg(uint8_t RegAddress, uint8_t RegData);

MFRC522Status MFRC522_ReadRegBit(uint8_t RegAddress, uint8_t BitPosition, bool *BitSet);

MFRC522Status MFRC522_WriteRegBit(uint8_t RegAddress, uint8_t BitPosition, bool SetBit);

MFRC522Status MFRC522_ExecuteCommand(MFRC522Command Command);

MFRC522Status MFRC522_SetRxInterruptEnable(bool Enable);



// find better name
// examples: 7U (only 7 bits of last TxByte) 
// 0U sets to normal 8 bits
MFRC522Status MFRC522_ChangeTxLastBits(uint8_t LengthTxLastByte);

MFRC522Status MFRC522_ResetRxInterrupt(void);



// FIFO Buffer

MFRC522Status MFRC522_ReadFIFOBuffer(uint8_t *Data, uint8_t Length);

MFRC522Status MFRC522_WriteFIFOBuffer(const uint8_t *Data, uint8_t Length);

MFRC522Status MFRC522_ResetFIFOBuffer(void);


// Transceive UID

MFRC522Status MFRC522_StartTransceive(void);

MFRC522Status MFRC522_InitTransceive(void);

/**
 * @brief  Advances the UID detection by one step (non-blocking).
 *
 * @param[out] DetectedUID  filled with the UID when MFRC522_UID_DETECTED is returned
 *
 * @retval MFRC522_OK               attempt in progress, call again
 * @retval MFRC522_NO_UID_DETECTED  attempt finished, no valid card
 *                                  (reason: MFRC522_GetLastError())
 * @retval MFRC522_UID_DETECTED     attempt finished, UID in DetectedUID
 * @return any other value: communication error with the reader
 *
 * @pre    Reader initialized with MFRC522_InitTransceive()
 * @note   Call repeatedly from the main loop
 * @note   Only 4-byte UIDs are supported
 * @warning After a communication error the reader must be re-initialized
 */
MFRC522Status MFRC522_DetectUID(UID *DetectedUID);

MFRC522Status MFRC522_ValidateUID(const UID *UID, size_t UIDLength, bool *IsValid);

# endif
