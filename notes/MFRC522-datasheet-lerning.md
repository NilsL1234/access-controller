# MFRC522 Datasheet understanding


#### Question: How do I read a regester with SPI?

- page 10, 8.1.2.1 SPI read data
- send via MOSI the address (0 up to address n; 1address=1byte)
- answer via MISO (comes 1 byte later; like: Byte0/adress0 ->byte1/data0)
- send MSB first

#### How does a SPI address byte look like?

- page 11 8.1.2.3 SPI address byte
- MSB(7) defines the mode: 1=read 0=write
- bit 6-1 is the address of the register
- LSB(0) is set to 0

#### What register addresses are there?

- 6 bits long
- 9.1 Register overview

- Example: page 38: 9.3.1.3 ComIEnReg register
    - Control bits to enable and disable the passing of interrupt requests
    - address: 02h
    - reset value: 80h
    - readbyte: 1 000010 0 (1=read, 000010=address, 0)


#### How do I init SPI on the stm32?

- set CPOL (clock polarity/idle clock state HIGH1 or LOW0) and CPHA (clockphase, where data is transmitted, 0 is first edge and 1 is second edge )in SPIx_CR1 register
- ...set MISO MOSI and so on (stm32 ref manual 26.3.7 SPI configuration)

#### How to send the read request to the MFRC from stm?

- SPI is enabled (SPI_CR1 SPE/bit6 is set)
- write data in SPI_DR (stores data in Tx Buffer before transmission) /read data in SPI_DR (gets data from Rx Buffer from transmission)
- TXE flag is set when data is moved from Tx Buffer to shift register/is resetted by new writing in Tx Buffer
- The RXNE flag (Rx buffer not empty) is set on the last sampling clock edge, when the data are transferred from the shift register to the Rx buffer. It indicates that data are ready to be read from the SPI_DR register (resetted by reading Rx buffer)


#### Which registers do I need for SPI data transmission?

- SPI_CR1: SPE/bit6 is set (SPI enable)
- SPI_SR: TXE (transmission empty/byte is moved to shift reg when set, new byte can be written in DR)
SPI_SR: RXNE (Receive buffer not empty 0: Rx buffer empty
1: Rx buffer not empty)
- SPI_DR: 16 bit register for tx and rx data

#### What is necessary for reading for example the ComIEnReg register?

- init SPI on stm32 according to needs of the MFRC
- enable SPI with setting SPE in SPI_CR1
- check if TXE is set in SPI_SR if so writing is possible
- write the addressbyte in the DR (1 000010 0)
- send a dummy (0x00)
- check RXNE from SPI_SR
- when set -> read SPI_DR




### Questions:


