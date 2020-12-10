# Product Requirements
## TODO
- Select the right battery. Probably 10S.
- Take voltage measurements on motor
- Find back wheel motor than can run on 36 volts at 2KW+
- Characterize downhill software behavior when pedaling
- Characterize the "load" of our battery + motor setup
- Purchase 32u4 can board and test with VESC.

## General Architecture

- Motor in back wheel
- Double chain: One to the back wheel. One to the generator. Single speed configuration.
- Removable Li-ion battery
- Double VESC
- Custom board: Controls the two VESCs. Allows to change setting w/ buttons and potentiometers. Performance/debug monitoring on LCD screen.
- Enclosed generator. Designed with normalized components. Adaptable to multiple architectures.
- Main mechanical aspects: generator, LCD board, main electronics, batteries. Generator and LCD have independant mounting solutions. Main electronics and batteries are fixed on the same mechanical contraption.

## Specific Requirements
### Motor
- 2 kw. Should try sensored and unsensored.

### Chain 
- Protected from the top generator to avoid touching leg.
- Generator should allow to set chain tension. Ideally integrated with the mounting solution.
- Might require chain tensioner if back derailleur is removed.

### Battery
- 36 volts. Even though it might be better to use 48 volts for the motor.

### VESCS
- Using VESC 6

### Custom Boards
###### VESC board
- Does it serve as a PDB for the two other VESCs + main ON/OFF Switch (could be good for safety cutoff switch)?
- Based on atmega32U4 microcontroller (To be tested)
- 1* CAN for two VESCs
- 1* UART (1x LCD screen)
###### LCD + Interface Board
- 3 switches input (1 push button and 2 toggle switches ?)
- 3 potentiometer input
- LCD integrated. Two lines of text.
- Should be mounted on the handlebar.
- The potentiometer part of the PCB has holes so that it can be broken off if not required by the user.

### Generator
- Connected to crankshaft through chain with sprocket. Must find sprocket that can interface with chain and shaft.
- use standardized components
- Should be able to change chain tension
- Should be able to mount on any conventional bike frame
- Fully enclosed

### Mechanical Design
###### Electronics and battery
- Mount battery easily
- support the two VESCs + the custom board

###### LCD Screen
- Support the LCD screen and buttons
- If the PCB has a part that can be broken off, there should be two version of the mechanical design