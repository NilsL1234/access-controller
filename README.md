# Project: Access Controller

This project is a demonstrator for RFID-based access control using an STM32 microcontroller. 
The demonstrator consists of a case that is locked with a bolt. The case can be unlocked using an authorized RFID card.
If the card's UID matches the an entry in the whitelist, the bolt will be unlocked and the door can be opened.
The bolt is moved by the MG90S servo. There are three sensors in use to determine the current position of the bolt and the door.
An OLED display is used to give the user information about the current state.


## Project goals:

The main goal of this project is to grow my own understanding of embedded software development.

I had no prior expirience in software projects on the microcontroller level.
I did one raspberry Pi project before, but with its own operating system, there is a limited overplap.
All my other project expirience is in application software developement. 

For that reason I tryed to combine multiple conecepts and communication standards that are widly used in microcontroller development.
The following aspects are used in this project and helped me to gain a better understanding working with embedded systems:    
    - C
    - STM32-Mikrocontrollerentwicklung
    - GPIO
    - Timer
    - Interrupts 
    - UART (debugging via USB)
    - SPI (RFID reader <-> stm32 communication)
    - I²C (OLED <-> stm32 communication)
    - PWM (servo control)
    - Debugging
    - State maschine
    - Error handling 
    - Tests
    - documentation
    - datasheet reading
    - basic driver development (RFID reader/OLED)

XXX project idea/ what to use..


It definitly helped, that I had my introduction to programming in C/C++ at university.
I repeated the main concepts and syntax again and lerned how bitoperations are used in C to have a proper base understanding. 

Afterwards I did the setup of the board and the firmware project.
For this project I'm using GCC as a Compiler and GDB as Debugger. CMake for the build configuration and Ninja the execution. 
For the initial project setup and configuration I used the tool STM32Cube.
Then I made myself familiar with the structure of the STM32-F446RE and basics of embedded sytems. This included aspects like GPIOs, clocks and busses.
I used the official documents from STMicroelectronics to also lern how to use those kind of documents.
For lerning background knowlage I used an AI chatbot in project mode. I gave the model stict guidelines to ensure that the goal was my own understanding and not to finish the project as fast as possible with the AI doing all the work. Throughout the project I did all the coding myself outside of the displayed characters on the OLED at the end, which was a repetetive task after doing one character myself anyway. 
For the debugging process I also tryed to stay away from using the AI to gain more experience in that skill.



















## Komponenten:




## Funktionsweise:





