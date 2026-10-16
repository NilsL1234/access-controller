# How to read embedded documents

#### 1. Whats the document type?

- Datasheet = eigenschaften, pins (their functions/which pin can MISO),  elec/timing data
- Reference Manual = how do the MCU-peripherals exactly work, which registeries to use
- Application Note = typical use cases
- User Manual = Board, Plugs, Jumper 


#### 2. Start with a concrete question.

- not "How does the MFRC522 work?"
- better "How do I read a register via SPI?", "How do I activate the RF-Field?"

#### 3. Look at Overview/Blockdiagramm 

- Overview whats there and how are the parts connected

#### 4. Use Search

- strf: "SPI", "reset", ...

#### 5. Follow references

#### 6. Do not read registers isolated.

- who is setting the bit?
- when is it active?
- whats the reset value?
- r, w or rw?
- which status bit gives success/errorinfo?

#### 7. Small process description before code

- NSS LOW -> send adressbyte -> send dummybyte -> read answer -> NSS HIGH

#### 8. Notice conjectures.

- conjectures should not evolve into facts

#### 9. Verify after small steps.

- better for error detection

