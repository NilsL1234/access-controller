# include "rfid.h"
# include "uart.h"
# include "main.h"

static const uint8_t SPISizeTransferData = 2U;
static const uint32_t SPITimeoutTransferDurationMS = 10U;
static const uint8_t AddressPos = 1U;
static const uint8_t DummyByte = 0x00;
static const uint8_t FIFOBufferSize = 64U;


// commands

static const uint8_t CommandMsk = 0x0fU;
static const uint8_t TransiveCommand = 0x03U;
static const uint8_t TransiveCommandPos = 2U;
static const uint8_t TransiveCommandMsk = TransiveCommand << TransiveCommandPos;
static const uint8_t IdleCommand = 0U;
static const uint8_t IdleCommandMsk = 0U;
static const uint8_t SoftResetCommand = 0x0fU;
static const uint8_t SoftResetCommandPos = 0U;
static const uint8_t SoftResetCommandMsk = SoftResetCommand << SoftResetCommandPos;



// run Transive Command 1100 idle =0000



// RFID

static UIDSearchState CurrentUIDSearchState = RFID_OFF;
static const uint8_t REQA [] = {0x26U};
static const uint8_t REQALength = 1U;
static const uint32_t MaxRespTimeRFID = 20U;
static const uint8_t AnticollisionCL1Req [] = {0x93U, 0x20U};
static const uint8_t AnticollisionCL1ReqLength = 2U;


static const UID WhiteListUID [] = {
{.UID = {0xe7, 0x20, 0x75, 0x33}, .UIDLength = 4U}, // card
{.UID = {0xe1, 0x65, 0x6f, 0x66}, .UIDLength = 4U}, // transponder
{.UID = {0U, 0U, 0U, 0U, 0U, 0U, 0U}, .UIDLength = 7U},
{.UID = {0U, 0U, 0U, 0U, 0U, 0U, 1U}, .UIDLength = 7U},
{.UID = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U}, .UIDLength = 10U},
{.UID = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 1U}, .UIDLength = 10U}
};

static const size_t WhiteListUIDLenth = sizeof(WhiteListUID) / sizeof(WhiteListUID[0]);

UIDSearchState MFRC522_GetCurrentUIDSearchState(void)
{
    return CurrentUIDSearchState;
}

void MFRC522_SetCurrentUIDSearchState(UIDSearchState State)
{
    CurrentUIDSearchState = State;
}

MFRC522Status MFRC522_ReadReg(uint8_t RegAddress, uint8_t *RegData)
{
    if(RegAddress > LastValidRegAddress)
    {
        return MFRC522_INVALID_REG_ADDRESS;
    }

    SPI_HandleTypeDef *hspi = GetSPIHandleTypeDef();
    uint8_t ReadBit = 0x80;
    uint8_t ReadRegByte = (RegAddress << AddressPos) | ReadBit;


    // prepare Tx/Rx
    const uint8_t TxDataSPI[] = {ReadRegByte, DummyByte};
    uint8_t RxDataSPI[] = {0U, 0U};

    // transmit data via SPI
    RFID_NSS_GPIO_Port->BSRR = (uint32_t)RFID_NSS_Pin << 16U;
    HAL_StatusTypeDef Status =  HAL_SPI_TransmitReceive(hspi, TxDataSPI, RxDataSPI, SPISizeTransferData, SPITimeoutTransferDurationMS);
    RFID_NSS_GPIO_Port->BSRR = (uint32_t)RFID_NSS_Pin;

    // return reg data and HAL status
    switch(Status)
    {
        case HAL_OK:
            *RegData = RxDataSPI[1];
            return MFRC522_OK;
        case HAL_ERROR:
            return MFRC522_ERROR;
        case HAL_BUSY:
            return MFRC522_BUSY;
        case HAL_TIMEOUT:
            return MFRC522_TIMEOUT;
        default: return MFRC522_ERROR;
    }
}

MFRC522Status MFRC522_WriteReg(uint8_t RegAddress, uint8_t RegData)
{
    if(RegAddress > LastValidRegAddress)
    {
        return MFRC522_INVALID_REG_ADDRESS;
    }

    SPI_HandleTypeDef *hspi = GetSPIHandleTypeDef();
    uint8_t WriteRegByte = (RegAddress << AddressPos);

    // prepare Tx/Rx
    const uint8_t WriteRegData[] = {WriteRegByte, RegData};
    uint8_t RxDataSPI[] = {0U, 0U};

    // transmit data via SPI
    RFID_NSS_GPIO_Port->BSRR = (uint32_t)RFID_NSS_Pin << 16U;
    HAL_StatusTypeDef Status =  HAL_SPI_TransmitReceive(hspi, WriteRegData, RxDataSPI, SPISizeTransferData, SPITimeoutTransferDurationMS);
    RFID_NSS_GPIO_Port->BSRR = (uint32_t)RFID_NSS_Pin;

    // MFRC should return reg address after valid write 

    /*
    if(RxDataSPI[1] != RegAddress)
    {
        return MFRC522_ERROR;
    }
    */
    
    // return HAL status
    switch(Status)
    {
        case HAL_OK:
            return MFRC522_OK;
        case HAL_ERROR:
            return MFRC522_ERROR;
        case HAL_BUSY:
            return MFRC522_BUSY;
        case HAL_TIMEOUT:
            return MFRC522_TIMEOUT;
        default: return MFRC522_ERROR;
    }

}

/*
MFRC522_ReadRegBit:   TODO: TEST
- reads a specific Bit for a Register
- *BitSet is 1U when Bit is set (0U when reset)

*/
MFRC522Status MFRC522_ReadRegBit(uint8_t RegAddress, uint8_t BitPosition, bool *BitSet)
{

    if(BitPosition >= RegBitLength)
    {
        return MFRC522_INVALID_BIT_POSITION;
    }

    uint8_t BitMask = 1U << BitPosition;
    uint8_t RegData = 0U;
    

    MFRC522Status Status = MFRC522_ReadReg(RegAddress, &RegData);

    if((RegData & BitMask) != 0U)
    {
    *BitSet = true;
    }
    else {
    *BitSet = false;
    }

    switch(Status)
    {
        case MFRC522_OK:
            return MFRC522_OK;
        case MFRC522_ERROR:
            return MFRC522_ERROR;
        case MFRC522_BUSY:
            return MFRC522_BUSY;
        case MFRC522_TIMEOUT:
            return MFRC522_TIMEOUT;
        default: return MFRC522_ERROR;
    }
}

MFRC522Status MFRC522_WriteRegBit(uint8_t RegAddress, uint8_t BitPosition, bool SetBit)
{
    if(BitPosition >= RegBitLength)
    {
        return MFRC522_INVALID_BIT_POSITION;
    }
    
    uint8_t BitMask = 1U << BitPosition;
    uint8_t RegData = 0U;
    MFRC522Status Status = MFRC522_ReadReg(RegAddress, &RegData);

    if(Status != MFRC522_OK)
    {
        return MFRC522_ERROR;
    }
    
    if(SetBit)
    {
        RegData |= BitMask;
    }
    else
    {
        RegData &= ~(BitMask);
    }

    Status = MFRC522_WriteReg(RegAddress, RegData);

    switch(Status)
    {
        case MFRC522_OK:
            return MFRC522_OK;
        case MFRC522_ERROR:
            return MFRC522_ERROR;
        case MFRC522_BUSY:
            return MFRC522_BUSY;
        case MFRC522_TIMEOUT:
            return MFRC522_TIMEOUT;
        default: return MFRC522_ERROR;
    }

}

MFRC522Status MFRC522_ExecuteCommand(MFRC522Command Command)
{
    uint8_t CommandRegData = 0U;

    MFRC522Status Status = MFRC522_ReadReg(CommandRegAddress, &CommandRegData);

    if(Status != MFRC522_OK)
    {
        return Status;
    }

    CommandRegData &= ~(CommandMsk);

    switch(Command)
    {
        case IDLE_COMMAND:
            CommandRegData |= IdleCommandMsk;
            break;
        case TRANSCEIVE_COMMAND:
            CommandRegData |= TransiveCommandMsk;
            break;
        case SOFT_RESET_COMMAND:
            CommandRegData |= SoftResetCommandMsk;
            break;

        default:
            return MFRC522_ERROR;    
    }

    Status = MFRC522_WriteReg(CommandRegAddress, CommandRegData);

    return Status;

}

FIFOBufferStatus MFRC522_ReadFIFOBuffer(uint8_t *Data, uint8_t Length)
{
    uint8_t FIFOLevelRegData = 0U;
    uint8_t FIFODataRegData = 0U;

    // check that FIFO is not empty
    MFRC522_ReadReg(FIFOLevelRegAddress, &FIFOLevelRegData);

    if(FIFOLevelRegData < Length)
    {
        return FIFO_EMPTY;
    }

    // read FIFO 
    for(uint8_t i=0; i<Length; ++i)
    {
        MFRC522Status Status = MFRC522_ReadReg(FIFODataRegAddress, &FIFODataRegData);
        if (Status != MFRC522_OK)
        {
            return FIFO_READ_ERROR;
        } 
        Data[i] = FIFODataRegData;
    }
    return FIFO_OK; 
}

FIFOBufferStatus MFRC522_WriteFIFOBuffer(const uint8_t *Data, uint8_t Length)
{
    uint8_t FIFOLevelRegData = 0U;

    // check that FIFO is not full
    MFRC522_ReadReg(FIFOLevelRegAddress, &FIFOLevelRegData);

    if((FIFOLevelRegData + Length) >= FIFOBufferSize)
    {
        return FIFO_FULL;
    }
    
    // write FIFO 
    for(uint8_t i=0; i<Length; ++i)
    {
        MFRC522Status Status = MFRC522_WriteReg(FIFODataRegAddress, Data[i]);
        if (Status != MFRC522_OK)
        {
            return FIFO_WRITE_ERROR;
        } 
    }
    return FIFO_OK; 
}

FIFOBufferStatus MFRC522_ResetFIFOBuffer(void)
{
    uint8_t FlushBufferBitPos = 7U;

    MFRC522Status status = MFRC522_WriteRegBit(FIFOLevelRegAddress, FlushBufferBitPos, true);

    if (status == MFRC522_OK)
    {
        return FIFO_OK;
    }
    else {
    return FIFO_WRITE_ERROR;
    }
}

MFRC522Status MFRC522_SetRxInterruptEnable(bool enable)
{

    uint8_t RxIEnPos = 5U;
    MFRC522Status Status = MFRC522_OK;

    if(enable)
   {
    Status = MFRC522_WriteRegBit(ComIEnRegAddress, RxIEnPos, 1U);
   }
   else {
    Status = MFRC522_WriteRegBit(ComIEnRegAddress, RxIEnPos, 0U);
   }
   return Status; 
}

MFRC522Status MFRC522_InitTransceive(void)
{

// enable Tx1 in TxControlRegister:
uint8_t Tx1RFEnPos = 0U;
MFRC522Status Status = MFRC522_WriteRegBit(TxControlRegAddress, Tx1RFEnPos, 1U);

if(Status != MFRC522_OK)
{
    return Status;
}

// enable Tx2 in TxControlRegister:
uint8_t Tx2RFEnPos = 1U;
Status = MFRC522_WriteRegBit(TxControlRegAddress, Tx2RFEnPos, 1U);

if(Status != MFRC522_OK)
{
    return Status;
}

// Enable RxIEn in ComIEnReg (Rx as Interrupt)

//uint8_t RxIEnPos = 5U;
//Status = MFRC522_WriteRegBit(ComIEnRegAddress, RxIEnPos, 1U);

Status = MFRC522_SetRxInterruptEnable(true);

if(Status != MFRC522_OK)
{
    return Status;
}

// force 100 ASK
uint8_t Force100ASKPos = 6U;
MFRC522_WriteRegBit(TxASKRegAddress, Force100ASKPos, 1U);

// Reset RcvOff

uint8_t RcvOffPos = 5U;
MFRC522_WriteRegBit(CommandRegAddress, RcvOffPos, 0U);


// ignore invalid received data stream (less than 4 bits)

uint8_t RxNoErrPos = 3U;
Status = MFRC522_WriteRegBit(RxModeRegAddress, RxNoErrPos, 1U);

if(Status != MFRC522_OK)
{
    return Status;
}  

MFRC522_ResetFIFOBuffer();

Status = MFRC522_ExecuteCommand(TRANSCEIVE_COMMAND);

CurrentUIDSearchState = RFID_INIT;

return Status;

}

MFRC522Status MFRC522_StartTransceive(void)
{
    // set StartSend Bit in BitFramingReg
    uint8_t StartSendBitPos = 7U;
    MFRC522Status Status = MFRC522_WriteRegBit(BitFramingRegAddress, StartSendBitPos, 1U);
    return Status;
}

// NOT TESTED
/*
MFRC522_ChangeTxLastBits:
    - Changes the length of the last Byte that gets transmitted
    - range 0U-7U
    - 0U sets Tx Byte back to normal 8 Bit
*/
MFRC522Status MFRC522_ChangeTxLastBits(uint8_t LengthTxLastByte)
{
    uint8_t TxBitLengthMask = 7U;
    uint8_t BitFramingRegData = 0U;

    if(LengthTxLastByte > TxBitLengthMask)
    {
        return MFRC522_INVALID_REG_ACCESS;
    }

    MFRC522Status Status = MFRC522_ReadReg(BitFramingRegAddress, &BitFramingRegData);

    if(Status != MFRC522_OK)
    {
        return Status;
    }

    BitFramingRegData &= ~(TxBitLengthMask);
    BitFramingRegData |= LengthTxLastByte;

    Status = MFRC522_WriteReg(BitFramingRegAddress, BitFramingRegData);

    return Status;
}

// gets used in loop?
// requires interrupt for IRQ



/* Function: MFRC522_DetectUID
                - detects 4 Bytes long UIDs (expandable for 7 and 10)
                - 
    
*/
MFRC522Status MFRC522_DetectUID(UID *DetectedUID)
{
    static MFRC522Status Status = MFRC522_OK;
    static FIFOBufferStatus FIFOStatus = FIFO_OK;
    static uint8_t ATQA [2] = {0U};
    const uint8_t ATQALength = 2U;
    static uint8_t AnticollisionCL1Resp [5] = {0U};
    const uint8_t AnticollisionCL1RespLength = 5U;
    
    UIDSearchState SearchStateTEST = MFRC522_GetCurrentUIDSearchState();


    switch(CurrentUIDSearchState)
    {
        case RFID_OFF:
            return MFRC522_OK;

        case RFID_INIT:
            CurrentUIDSearchState = RFID_REQA_SEND;
            return MFRC522_OK;

        case RFID_REQA_SEND:
            uint8_t TxBitLengthREQA = 7U;

            FIFOStatus = MFRC522_ResetFIFOBuffer();
            if(FIFOStatus != FIFO_OK)
            {
                return MFRC522_FIFO_BUFFER_ERROR;
            }

            FIFOStatus = MFRC522_WriteFIFOBuffer(REQA, REQALength);
            if(FIFOStatus != FIFO_OK)
            {
                return MFRC522_FIFO_BUFFER_ERROR;
            }

            Status = MFRC522_ChangeTxLastBits(TxBitLengthREQA);
            if(Status != MFRC522_OK)
            {
                return Status;
            }

            CurrentUIDSearchState = RFID_ATQA_WAITING;

            static uint32_t StartRFIDRespTimer = 0U;
            StartRFIDRespTimer = HAL_GetTick();

            MFRC522_SetRxInterruptEnable(true);
            Status = MFRC522_StartTransceive();

            if(Status != MFRC522_OK)
            {
                return Status;
            }
            return Status;
            


        case RFID_ATQA_WAITING:

            uint8_t CommandRegData = 0U;
            uint8_t FIFOLevelRegData = 0U;
            uint8_t ErrorRegData = 0U;
            uint8_t ComIEnRegData = 0U;
            uint8_t ComIrqRegData = 0U;

            MFRC522Status Status = MFRC522_ReadReg(CommandRegAddress, &CommandRegData);
            MFRC522_ReadReg(FIFOLevelRegAddress, &FIFOLevelRegData);
            MFRC522_ReadReg(ErrorRegAddress, &ErrorRegData);

            MFRC522_ReadReg(ComIEnRegAddress, &ComIEnRegData);
            MFRC522_ReadReg(ComIrqRegAddress, &ComIrqRegData);

            
            
            if((HAL_GetTick() - StartRFIDRespTimer) > MaxRespTimeRFID)
            {
                MFRC522_SetRxInterruptEnable(false);
                MFRC522_ExecuteCommand(IDLE_COMMAND);
                MFRC522_ExecuteCommand(TRANSCEIVE_COMMAND);
        
                MFRC522_ResetRxInterrupt();
                CurrentUIDSearchState = RFID_INIT;
            }
                
            return MFRC522_OK;
            
        case RFID_ATQA_AVAILABLE:
            MFRC522_ResetRxInterrupt();

            // check ATQA/FIFO length
            FIFOLevelRegData = 0U;
            MFRC522_ReadReg(FIFOLevelRegAddress, &FIFOLevelRegData);
            if(FIFOLevelRegData != ATQALength)
            {
                CurrentUIDSearchState = RFID_INIT;
                //better statusenum
                return MFRC522_ERROR;
            }

            // TODO!!!
            // check Error Reg (todo: check exact error bits)
            //uint8_t ErrorRegData = 0U;
            //MFRC522_ReadReg(ErrorRegAddress, &ErrorRegData);
            //if(ErrorRegData != 0U)
            //{
            //    return MFRC522_ERROR;
            //}

            // read ATQA from FIFO buffer
            FIFOBufferStatus FIFOStatus = MFRC522_ReadFIFOBuffer(ATQA, ATQALength);
            if(FIFOStatus != FIFO_OK)
            {
                return MFRC522_FIFO_BUFFER_ERROR;
            }

            CurrentUIDSearchState = RFID_ANTICOLLISION_CL1_SEND;
            return MFRC522_OK;
        
        case RFID_ANTICOLLISION_CL1_SEND:
            MFRC522_ChangeTxLastBits(0U);

            FIFOStatus = MFRC522_WriteFIFOBuffer(AnticollisionCL1Req, AnticollisionCL1ReqLength);
            if(FIFOStatus != FIFO_OK)
            {
                return MFRC522_FIFO_BUFFER_ERROR;
            }

            CurrentUIDSearchState = RFID_ANTICOLLISION_CL1_WAITING;
            MFRC522_SetRxInterruptEnable(true);
            Status = MFRC522_StartTransceive();
            if(Status != MFRC522_OK)
            {
                return Status;
            }
            return Status;
        
        case RFID_ANTICOLLISION_CL1_WAITING:

            if((HAL_GetTick() - StartRFIDRespTimer) > MaxRespTimeRFID)
            {
                
                MFRC522_SetRxInterruptEnable(false);
                MFRC522_ExecuteCommand(IDLE_COMMAND);
                MFRC522_ExecuteCommand(TRANSCEIVE_COMMAND);
        
                MFRC522_ResetRxInterrupt();
                CurrentUIDSearchState = RFID_INIT;
            }

            return MFRC522_OK;
        
        case RFID_ANTICOLLISION_CL1_AVAILABLE:

            MFRC522_ResetRxInterrupt();

            FIFOLevelRegData = 0U;     
            MFRC522_ReadReg(FIFOLevelRegAddress, &FIFOLevelRegData);
            if(FIFOLevelRegData != AnticollisionCL1RespLength)
            {
                CurrentUIDSearchState = RFID_INIT;
                //better statusenum
                return MFRC522_ERROR;
            }

            // check Error Reg (todo: check exact error bits)
            //MFRC522_ReadReg(ErrorRegAddress, &ErrorRegData);
            //if(ErrorRegData != 0U)
            //{
            //    return MFRC522_ERROR;
            //}

            FIFOStatus = MFRC522_ReadFIFOBuffer(AnticollisionCL1Resp, AnticollisionCL1RespLength);
            if(FIFOStatus != FIFO_OK)
            {
                return MFRC522_FIFO_BUFFER_ERROR;
            }

            uint8_t BCC = AnticollisionCL1Resp[0]^
                          AnticollisionCL1Resp[1]^
                          AnticollisionCL1Resp[2]^
                          AnticollisionCL1Resp[3];
            
            if (BCC == AnticollisionCL1Resp[4])
            {
                DetectedUID->UIDLength = UID_LENGTH_4;

                for(size_t i=0; i<UID_LENGTH_4; ++i)
                {
                    DetectedUID->UID[i] = AnticollisionCL1Resp[i];
                }

                // TODO: Change to idle after test
                CurrentUIDSearchState = RFID_INIT;
                return MFRC522_UID_DETECTED;
            }
            else 
            {
                return MFRC522_RFID_BCC_ERROR;
            }      
        default: return MFRC522_RFID_INVALID_STATE_ERROR;
    }

    return MFRC522_RFID_INVALID_STATE_ERROR;
}

// TODO: validation info gets out via status and bool IsValid -> decide for one?
MFRC522Status MFRC522_ValidateUID(const UID *UID, size_t UIDLength, bool *IsValid)
{

    *IsValid = false;

    if(UIDLength == UID_LENGTH_4)
    {
        // go over all whitelisted UIDs
        for(size_t index=0; index<WhiteListUIDLenth; ++index)
        {
            // compare where UID has same length 
            if(WhiteListUID[index].UIDLength == UID_LENGTH_4)
            {
                *IsValid = true;
                // compare each UID byte
                for(size_t i=0; i<UID_LENGTH_4; ++i)
                {
                    if ((WhiteListUID[index].UID[i]) != (UID->UID[i]))
                    {
                        *IsValid = false;
                    }
                }
                if(*IsValid)
                {
                    return MFRC522_UID_VALID;
                }
            }
        }
    }
    else if (UIDLength == UID_LENGTH_7) {
        // go over all whitelisted UIDs
        for(size_t index=0; index<WhiteListUIDLenth; ++index)
        {
            // compare where UID has same length 
            if(WhiteListUID[index].UIDLength == UID_LENGTH_7)
            {
                *IsValid = true;
                // compare each UID byte
                for(size_t i=0; i<UID_LENGTH_7; ++i)
                {
                    if ((WhiteListUID[index].UID[i]) != (UID->UID[i]))
                    {
                        *IsValid = false;
                    }
                }
                if(*IsValid)
                {
                    return MFRC522_UID_VALID;
                }
            }
        }
    }
    else if (UIDLength == UID_LENGTH_10) {
        // go over all whitelisted UIDs
        for(size_t index=0; index<WhiteListUIDLenth; ++index)
        {
            // compare where UID has same length 
            if(WhiteListUID[index].UIDLength == UID_LENGTH_10)
            {
                *IsValid = true;
                // compare each UID byte
                for(size_t i=0; i<UID_LENGTH_10; ++i)
                {
                    if ((WhiteListUID[index].UID[i]) != (UID->UID[i]))
                    {
                        *IsValid = false;
                    }
                }
                if(*IsValid)
                {
                    return MFRC522_UID_VALID;
                }
            }
        }
    }
    else {
        return MFRC522_INVALID_UID_LENGTH;
    }
return MFRC522_UID_INVALID;

}

MFRC522Status MFRC522_ResetRxInterrupt(void)
{
    uint8_t RxIRqBitPos = 5U;
    uint8_t RxIRqBitMask = 1U << RxIRqBitPos;

    MFRC522Status Status = MFRC522_WriteReg(ComIrqRegAddress, RxIRqBitMask);
    return Status; 
}