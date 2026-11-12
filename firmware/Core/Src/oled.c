#include "oled.h"
#include "main.h"
#include "chars.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>



// 8 Bit address
static const uint8_t SSD1306_Address = 0x78;

static const uint32_t TxI2CCommandSizeData = 2U;
static const uint32_t TxI2CTimeoutDuration = 20000U;





// D/C#: command=0 | data=1
// Co: 0=no more Co | 1= more Co

// Co is set: more control bytes will follow and next byte is a command
static const uint8_t CoCommandContinuationByte = 0x80;
// Co is set: more control bytes will follow and next byte is data for GDDRAM
static const uint8_t CoDataContinuationByte = 0xC0;
// Co is unset: no more control bytes will follow and next byte is a command
static const uint8_t NoCoCommandContinuationByte = 0x00;
// Co is unset: no more control bytes will follow and next byte is data for GDDRAM
static const uint8_t NoCoDataContinuationByte = 0x40;





SSD1306_Status SSD1306_SendCommand(uint8_t Command, const uint8_t *Parameters, uint8_t ParameterLength)
{

    // with arguements

    // NoCoCommandContinuationByte
    //
    uint8_t TxMinByteAmout = 2U; 


    uint8_t TxCommand [MAX_TX_COMMAND_LENGTH];
    TxCommand[0] = NoCoCommandContinuationByte;
    TxCommand[1] = Command;

    if((Parameters != NULL) && (ParameterLength > 0U))
    {
    for(uint8_t i=0; i<ParameterLength;++i)
    {
        TxCommand[i+TxMinByteAmout] = Parameters[i];
    }
    }

    

    HAL_StatusTypeDef HAL_Status = HAL_I2C_Master_Transmit(GetI2CHandleTypeDef(), SSD1306_Address, TxCommand, ParameterLength+TxMinByteAmout, TxI2CTimeoutDuration);

    if(HAL_Status != HAL_OK)
    {
        return SSD1306_ERROR;
    }
    else
    {
        return SSD1306_OK;
    }
}

SSD1306_Status SSD1306_SendData(const uint8_t *Data, uint16_t Length)
{

// todo: check given length
uint8_t TxData [BYTES_TOTAL+1];
TxData[0] = NoCoDataContinuationByte;
memcpy(&TxData[1], Data, Length);
++Length;
HAL_StatusTypeDef HAL_Status = HAL_I2C_Master_Transmit(GetI2CHandleTypeDef(), SSD1306_Address, TxData, Length, TxI2CTimeoutDuration);

    if(HAL_Status != HAL_OK)
    {
        return SSD1306_ERROR;
    }
    else
    {
        return SSD1306_OK;
    }
}


/**/
static void SSD1306_ResetRAM(void)
{
    SSD1306_SendCommand(AddressingModeCommand, AddressingModePageArguement, 1U);
    SSD1306_SendCommand(SetLowerStartColumnPageModeCommand, NULL, 0U);
    SSD1306_SendCommand(SetHigherStartColumnPageMode, NULL, 0U);

    // clear all pixels:
    uint8_t PageStartCommandBase = 0xB0U;

    for(uint8_t i=0; i<PAGES_TOTAL; ++i)
    {
        uint8_t PageStartCommand = PageStartCommandBase|i;
        SSD1306_SendCommand(PageStartCommand, NULL, 0U);

        // prepare Tx byte for page clear
        uint8_t ClearByte = 0xFF;
        uint8_t TxClearPage[128];
        memset(TxClearPage, ClearByte, sizeof(TxClearPage));

        // clear current page
        SSD1306_SendData(TxClearPage, sizeof(TxClearPage));
    }  

}



SSD1306_Status SSD1306_InitOLED(void)
{



    // enable charge pump
    if(SSD1306_SendCommand(ChargePumpSettingCommand, NULL, 0U) != SSD1306_OK)
    {
        return SSD1306_ERROR;
    }
    
    if(SSD1306_SendCommand(ChargePumpEnableCommand, NULL, 0U) != SSD1306_OK)
    {
        return SSD1306_ERROR;
    }
    
    if(SSD1306_SendCommand(EntireDisplayOnDisableCommand, NULL, 0U) != SSD1306_OK)
    {
        return SSD1306_ERROR;
    }

    if(SSD1306_SendCommand(DisplayOnCommand, NULL, 0U) != SSD1306_OK)
    {
        return SSD1306_ERROR;
    }

    // adjust horizontal/vertical display
    if(SSD1306_SendCommand(SegmentRemapEnableCommand, NULL, 0U) != SSD1306_OK)
    {
        return SSD1306_ERROR;
    }

    if(SSD1306_SendCommand(COMRemapEnableCommand, NULL, 0U) != SSD1306_OK)
    {
        return SSD1306_ERROR;
    }

    
    if(SSD1306_SendCommand(AddressingModeCommand, AddressingModePageArguement, 1U) != SSD1306_OK)
    {
        return SSD1306_ERROR;
    }

    
    if(SSD1306_SendCommand(SetLowerStartColumnPageModeCommand, NULL, 0U) != SSD1306_OK)
    {
        return SSD1306_ERROR;
    }

    if(SSD1306_SendCommand(SetHigherStartColumnPageMode, NULL, 0U) != SSD1306_OK)
    {
        return SSD1306_ERROR;
    }

    

    // clear all pixels:

    SSD1306_ResetRAM();


/*
    uint8_t PageStartCommandBase = 0xB0U;

    for(uint8_t i=0; i<NUMBER_OF_PAGES; ++i)
    {
        uint8_t PageStartCommand = PageStartCommandBase|i;
        SSD1306_SendCommand(PageStartCommand, NULL, 0U);

        // prepare Tx byte for page clear
        uint8_t ClearByte = 0xFF;
        uint8_t TxClearPage[128];
        memset(TxClearPage, ClearByte, sizeof(TxClearPage));

        // clear current page
        SSD1306_SendData(TxClearPage, sizeof(TxClearPage));
    }  
*/        
    return SSD1306_OK;
}

// gets called when block counter is 1 or 2
SSD1306_Status SSD1306_DisplayTryBlocked(void)
{
    uint8_t BlockCounter = ACCESS_CONTROLER_GetCurrentBlockCounter();

    if((HAL_GetTick() - ACCESS_CONTROLER_GetBlockCounterDisplayStartTick()) > ACCESS_CONTROLER_GetBlockCounterDisplayTime() &&
        (ACCESS_CONTROLER_GetBlockCounterMessageDisplayed() == false))
    {
        // TODO: Error handling
        SSD1306_DisplayCurrentState();
        ACCESS_CONTROLER_SetBlockCounterMessageDisplayed(true);
        return SSD1306_OK;
    }

    if((BlockCounter == 1U) && (ACCESS_CONTROLER_GetBlockCounterMessageDisplayed() == false))
    {
        uint8_t BorderStartEndBlockCounter1[EMPTY_BYTES_START_END_9CHARS];
        memset(BorderStartEndBlockCounter1, 0xFF, sizeof(BorderStartEndBlockCounter1));
        SSD1306_SendData(BorderStartEndBlockCounter1, EMPTY_BYTES_START_END_9CHARS);
        SSD1306_SendData(CharE, CHAR_RASTER);
        SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
        SSD1306_SendData(CharR, CHAR_RASTER);
        SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
        SSD1306_SendData(CharR, CHAR_RASTER);
        SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
        SSD1306_SendData(CharO, CHAR_RASTER);
        SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
        SSD1306_SendData(CharR, CHAR_RASTER);
        SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
        SSD1306_SendData(CharColon, CHAR_RASTER);
        SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
        SSD1306_SendData(Char1, CHAR_RASTER);
        SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
        SSD1306_SendData(CharSlash, CHAR_RASTER);
        SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
        SSD1306_SendData(Char3, CHAR_RASTER);
        SSD1306_SendData(BorderStartEndBlockCounter1, EMPTY_BYTES_START_END_9CHARS);
        return SSD1306_OK;
    }
    else if((BlockCounter == 2U) && (ACCESS_CONTROLER_GetBlockCounterMessageDisplayed() == false))
    {
        uint8_t BorderStartEndBlockCounter1[EMPTY_BYTES_START_END_9CHARS];
        memset(BorderStartEndBlockCounter1, 0xFF, sizeof(BorderStartEndBlockCounter1));
        SSD1306_SendData(BorderStartEndBlockCounter1, EMPTY_BYTES_START_END_9CHARS);
        SSD1306_SendData(CharE, CHAR_RASTER);
        SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
        SSD1306_SendData(CharR, CHAR_RASTER);
        SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
        SSD1306_SendData(CharR, CHAR_RASTER);
        SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
        SSD1306_SendData(CharO, CHAR_RASTER);
        SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
        SSD1306_SendData(CharR, CHAR_RASTER);
        SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
        SSD1306_SendData(CharColon, CHAR_RASTER);
        SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
        SSD1306_SendData(Char2, CHAR_RASTER);
        SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
        SSD1306_SendData(CharSlash, CHAR_RASTER);
        SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
        SSD1306_SendData(Char3, CHAR_RASTER);
        SSD1306_SendData(BorderStartEndBlockCounter1, EMPTY_BYTES_START_END_9CHARS);
        return SSD1306_OK;
    }
    else {
    //todo: better error handling
    return SSD1306_ERROR;
    }
}

SSD1306_Status SSD1306_DisplayCurrentState(void)
{
    SSD1306_ResetRAM();

    // config vertical mode to vertical mode
    SSD1306_SendCommand(AddressingModeCommand, AddressingModeVerticalArguement, 1);
    SSD1306_SendCommand(SetColumnStartEndCommand, ResetColumnStartEndArguement, 2);
    SSD1306_SendCommand(SetPageStartEndCommand, ResetPageStartEndArguement, 2);



    switch (ACCESS_CONTROLER_GetCurrentAccessControlerState())
    {
        case LOCKED_READY:  // displays: "SHOW ID"

            uint8_t BorderStartEndReady[EMPTY_BYTES_START_END_7CHARS];
            memset(BorderStartEndReady, 0xFF, sizeof(BorderStartEndReady));
            SSD1306_SendData(BorderStartEndReady, EMPTY_BYTES_START_END_7CHARS);
            SSD1306_SendData(CharS, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharH, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharO, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharW, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharEmpty, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharI, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharD, CHAR_RASTER);
            SSD1306_SendData(BorderStartEndReady, EMPTY_BYTES_START_END_7CHARS);
            return SSD1306_OK;

        /*    
        case UNLOCKING:  // "UNLOCKING"
            uint8_t BorderStartEndUnlocking[EMPTY_BYTES_START_END_9CHARS];
            memset(BorderStartEndUnlocking, 0xFF, sizeof(BorderStartEndUnlocking));
            SSD1306_SendData(BorderStartEndUnlocking, EMPTY_BYTES_START_END_9CHARS);
            SSD1306_SendData(CharU, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharN, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharL, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharO, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharC, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharK, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharI, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharN, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharG, CHAR_RASTER);
            SSD1306_SendData(BorderStartEndUnlocking, EMPTY_BYTES_START_END_9CHARS);
            return SSD1306_OK;

        case LOCKING:   // "LOCKING"
            uint8_t BorderStartEndLocking[EMPTY_BYTES_START_END_7CHARS];
            memset(BorderStartEndLocking, 0xFF, sizeof(BorderStartEndLocking));
            SSD1306_SendData(BorderStartEndLocking, EMPTY_BYTES_START_END_7CHARS);
            SSD1306_SendData(CharL, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharO, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharC, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharK, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharI, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharN, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharG, CHAR_RASTER);
            SSD1306_SendData(BorderStartEndLocking, EMPTY_BYTES_START_END_7CHARS);
            return SSD1306_OK;
        */

        case LOCKED_BLOCKED:    // displays: "BLOCKED"
            uint8_t BorderStartEndBlocked[EMPTY_BYTES_START_END_7CHARS];
            memset(BorderStartEndBlocked, 0xFF, sizeof(BorderStartEndBlocked));
            SSD1306_SendData(BorderStartEndBlocked, EMPTY_BYTES_START_END_7CHARS);
            SSD1306_SendData(CharB, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharL, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharO, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharC, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharK, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharE, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharD, CHAR_RASTER);
            SSD1306_SendData(BorderStartEndBlocked, EMPTY_BYTES_START_END_7CHARS);
            return SSD1306_OK;

        case UNLOCKED_DOOR_CLOSED:  // displays: "UNLOCKED"
            uint8_t BorderStartEndDoorClosed[EMPTY_BYTES_START_END_8CHARS];
            memset(BorderStartEndDoorClosed, 0xFF, sizeof(BorderStartEndDoorClosed));
            SSD1306_SendData(BorderStartEndDoorClosed, EMPTY_BYTES_START_END_8CHARS);
            SSD1306_SendData(CharU, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharN, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharL, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharO, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharC, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharK, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharE, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharD, CHAR_RASTER);
            SSD1306_SendData(BorderStartEndDoorClosed, EMPTY_BYTES_START_END_8CHARS);
            return SSD1306_OK;

        case UNLOCKED_DOOR_OPEN:    // displays: "DOOR OPEN"
            uint8_t BorderStartEndOpen[EMPTY_BYTES_START_END_9CHARS];
            memset(BorderStartEndOpen, 0xFF, sizeof(BorderStartEndOpen));
            SSD1306_SendData(BorderStartEndOpen, EMPTY_BYTES_START_END_9CHARS);
            SSD1306_SendData(CharD, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharO, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharO, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharR, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharEmpty, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharO, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharP, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharE, CHAR_RASTER);
            SSD1306_SendData(BorderBetweenChars, BORDER_BETWEEN_CHARS);
            SSD1306_SendData(CharN, CHAR_RASTER);
            SSD1306_SendData(BorderStartEndOpen, EMPTY_BYTES_START_END_9CHARS);
            return SSD1306_OK;

        case UNINITIALIZED: // TODO
             return SSD1306_OK;
        default:
            return SSD1306_ERROR;
    }

}


