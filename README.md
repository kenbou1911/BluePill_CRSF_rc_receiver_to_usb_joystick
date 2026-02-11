# BluePill_CRSF_rc_receiver_to_usb_joystick

Tested with Windows 11, Bluepill, RadioMaster 2.4G ELRS EP1 Nano Receiver (ELRS v3.5.2) baudrate 420000, and Jumper Smart ELRS Transmitter (ELRS v3.5.2).

Using the Bluepill board (STM32F103C8T6), we will connect the CRSF rc protocol receiver as a USB PC joystick.

Set the Bluepill board to boot0 high and boot1 low and write generic_boot20_pc13.bin of rogerclarkmelbourne's STM32duino-bootloader-master using ST-Link V2 emulator and STM32 ST-LINK Utility.
Download and unzip rogerclarkmelbourne's Arduino_STM32 and place the unzipped Arduino_STM32-master file.
Create a folder called "hardware" in Arduino IDE 1.8.18 (IDE 1.x) and place the file inside.
Download and unzip gcc-arm-none-eabi-8-2018-q4-major-win32.zip, and place the four folders inside it in a folder named arduino-1.8.18\hardware\Arduino_STM32-master\tools\arm-none-eabi-gcc\8-2018-q4-major.
Create a folder in the following hierarchy and place the four folders inside 8-2018-q4-major.

Set the Bluepill board to boot0 low and boot1 high, and connect it to your PC via USB.
Open the sketch in Arduino IDE 1.8.18 and select the Generic F103C series board.
Variant: Select based on the flash memory capacity.
Upload method: "STM32duino bootloader".
CPU speed: Select 72MHz.
Serial port: COM1.
Write to the Bluepill board.
Connect the Bluepill's 5v to the receiver's 5v.
Connect the Bluepill's GND to the receiver's GND.
Connect the Bluepill's PA_3 (Serial2 RX) to the receiver's TX.
Bind the transmitter and receiver.

The joystick has been upgraded from 6 axes and 32 buttons in v1.0.0 to 8 axes and 32 buttons in v1.1.0 .
You cannot increase the number of axes or decrease the number of buttons.
Customize it to suit your transmitter by uncommenting or adding code, increasing or decreasing Pos switches, and swapping channel numbers.
