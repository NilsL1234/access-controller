#ifndef OLED_H
#define OLED_H

#include "access_controler.h"

#include <stdbool.h>
#include <stdint.h>

#define BYTES_TOTAL 1024U
#define COLUMNS_TOTAL 128U
#define ROWS_TOTAL 64U
#define PAGES_TOTAL 8U


#define LETTER_HIGHT_PX 14U
#define LETTER_WIDTH_PX 10U
#define LETTER_THICKNESS_PX 2U

// Co-Byte + CommandByte + max. 6 Parameters
#define MAX_TX_COMMAND_LENGTH 8U


// COMMANDS:

static const uint8_t ChargePumpSettingCommand = 0x8D;
static const uint8_t ChargePumpEnableCommand = 0x14;
static const uint8_t ChargePumpDisableCommand = 0x10;

static const uint8_t EntireDisplayOnEnableCommand = 0xA5;
static const uint8_t EntireDisplayOnDisableCommand = 0xA4;

static const uint8_t SegmentRemapEnableCommand = 0xA1;
static const uint8_t SegmentRemapDisableCommand = 0xA0;

static const uint8_t COMRemapEnableCommand = 0xC8;
static const uint8_t COMRemapDisableCommand = 0xC0;

static const uint8_t DisplayOnCommand = 0xAF;
static const uint8_t DisplayOffCommand = 0xAE;


// addressing mode
static const uint8_t AddressingModeCommand = 0x20;
static const uint8_t AddressingModeHorizontalArguement [] = {0x00};
static const uint8_t AddressingModeVerticalArguement [] = {0x01};
static const uint8_t AddressingModePageArguement [] = {0x02};

// addressing start/end postions
static const uint8_t SetColumnStartEndCommand = 0x21;
static const uint8_t ResetColumnStartEndArguement [] = {0x00, 0x7F};
static const uint8_t SetPageStartEndCommand = 0x22;
static const uint8_t ResetPageStartEndArguement [] = {0x00, 0x07};

static const uint8_t SetLowerStartColumnPageModeCommand = 0x00; 
static const uint8_t SetHigherStartColumnPageMode = 0x1F;


typedef enum
{
SSD1306_OK = 0x00,
SSD1306_ERROR = 0x01
} SSD1306_Status;


// FUNCTIONS:

SSD1306_Status SSD1306_InitOLED(void);

SSD1306_Status SSD1306_SendCommand(uint8_t Command, const uint8_t *Parameters, uint8_t ParameterLength);

SSD1306_Status SSD1306_SendData(const uint8_t *Data, uint16_t Length);


SSD1306_Status SSD1306_DisplayCurrentState(void);

SSD1306_Status SSD1306_DisplayTryBlocked(void);

void ACCESS_CONTROLER_SetBlockCounterMessageDisplayed(bool Displayed);

bool ACCESS_CONTROLER_GetBlockCounterMessageDisplayed(void);


// How do I write letters to display?



//write 3 prixels in a row.




//void SSD1306_
//void SSD1306_






//void SSD1306_
//void SSD1306_
//void SSD1306_


#endif