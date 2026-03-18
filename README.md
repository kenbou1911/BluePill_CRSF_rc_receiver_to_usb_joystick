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

The joystick has been upgraded from 6 axes and 32 buttons in v1.0.0 to 8 axes and 32 buttons in v1.1.0. In v1.2.0, the analog axes have a 16-bit range. You can customize it to fit your transmitter by uncommenting or adding code, adding or removing Pos switches, swapping channel numbers, etc.

The USB polling rate is set to 10ms in the Arduino_STM32 library.
If you want to use a 1ms polling rate, use the following:
\hardware\Arduino_STM32-master\STM32F1\libraries\USBComposite\usb_hid.c

static uint8 txInterval = 0x0A;

Change this to:

static uint8 txInterval = 0x01;

Write the sketch.

Changes in v1.3.0.
Added ch13 to ch16 to parseCrsf.
I changed ch16 to a 6pos switch.
ch13 to ch16 only function at ELRS Full Res16ch Rate/2.
If the transmitter only has channels 1-12 and you assign buttons to channels 13-16, some of those buttons may remain lit (in the case of Jumper Smart).
If you are concerned, please comment out the code for the button specified in that channel.
Tested with Jumper Smart and RM TX15.

After v1.3.0, I wrote new, separate code for the SBUS protocol.
I confirmed operation in the happymodel ep1's web UI by setting the serial protocol to Inverted SBUS and the UART baud in OPTIONS to 100000.
SBUS receivers without Inverted SBUS functionality require an SBUS inverter.
