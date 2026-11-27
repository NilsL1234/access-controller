# How to read embedded documents

#### 1. What's the document type?

- Datasheet = properties, pins (their functions/which pin can MISO),  elec/timing data
- Reference Manual = how do the MCU-peripherals exactly work, which registers to use
- Application Note = typical use cases
- User Manual = Board, Plugs, Jumper


#### 2. Start with a concrete question.

- not "How does the MFRC522 work?"
- better "How do I read a register via SPI?", "How do I activate the RF-Field?"

#### 3. Look at Overview/Block diagram

- Overview what's there and how are the parts connected

#### 4. Use Search

- Ctrl+F: "SPI", "reset", ...

#### 5. Follow references

#### 6. Do not read registers isolated.

- who is setting the bit?
- when is it active?
- what's the reset value?
- r, w or rw?
- which status bit gives success/error info?

#### 7. Small process description before code

- NSS LOW -> send address byte -> send dummy byte -> read answer -> NSS HIGH

#### 8. Notice conjectures.

- conjectures should not evolve into facts

#### 9. Verify after small steps.

- better for error detection
