/*
 * BluePill_SBUS_rc_receiver_to_usb_joystick
 * * Version: 1.4.0
 * Copyright (c) 2026 kenbou1911
 * Licensed under the MIT License.
 * (See LICENSE file in the project root for details)
 */
 
#include <USBComposite.h>

// --- Custom HID Report Structure (8 axes 16-bit range + 32 buttons) ---
struct MyJoystickReport_t {
    uint8_t reportID;      
    uint16_t x;
    uint16_t y;
    uint16_t z;
    uint16_t rx;
    uint16_t ry;
    uint16_t rz;
    uint16_t slider;
    uint16_t dial;
    uint32_t buttons;      
} __attribute__((packed));

MyJoystickReport_t joyReport;

// --- HID Descriptor ---
const uint8_t reportDescription[] = {
    0x05, 0x01,                   // USAGE_PAGE (Generic Desktop)
    0x09, 0x04,                   // USAGE (Joystick)
    0xA1, 0x01,                   // COLLECTION (Application)
    0x85, 0x01,                   //   REPORT_ID (1)

    // --- 8 Axes (16-bit range inside 16-bit fields) ---
    0x05, 0x01,
    0xA1, 0x00,                   // COLLECTION (Physical)
    0x15, 0x00,                   // LOGICAL_MINIMUM (0)
    0x27, 0xFF, 0xFF, 0x00, 0x00, // LOGICAL_MAXIMUM (65535) 16-bit (0xFFFF)
    0x75, 0x10,                   // REPORT_SIZE (16-bit)
    0x95, 0x08,                   // REPORT_COUNT (8)
    0x09, 0x30, 0x09, 0x31, 0x09, 0x32, 0x09, 0x33, 
    0x09, 0x34, 0x09, 0x35, 0x09, 0x36, 0x09, 0x37,
    0x81, 0x02,                   // INPUT (Data,Var,Abs)
    0xC0,                         // END_COLLECTION

    // --- 32 Buttons ---
    0x05, 0x09,                   // USAGE_PAGE (Button)
    0x19, 0x01, 0x29, 0x20,       // USAGE_MIN(1), MAX(32)
    0x15, 0x00, 0x25, 0x01,       // LOGICAL_MIN(0), MAX(1)
    0x75, 0x01, 0x95, 0x20,       // SIZE(1), COUNT(32)
    0x81, 0x02,                   // INPUT (Data,Var,Abs)
    
    0xC0                          // END_COLLECTION
};

USBHID HID;
HIDReporter reporter(HID, (uint8_t*)&joyReport, sizeof(joyReport));

// --- Helper function for button mapping ---
void setButton(uint8_t buttonNum, bool pressed) {
    if (pressed) {
        joyReport.buttons |= (1UL << (buttonNum - 1));
    } else {
        joyReport.buttons &= ~(1UL << (buttonNum - 1));
    }
}

// --- SBUS Parsing Variables ---
uint8_t sbus_packet[25];  // SBUS is fixed at 25 bytes.
uint16_t channels[16];
uint8_t ptr = 0;
uint32_t lastByteTime = 0;

void setup() {
    pinMode(PC13, OUTPUT);
    digitalWrite(PC13, HIGH);

USBComposite.setProductId(0x0025);              // Shift from the original 0x0024 to "0x0025"
USBComposite.setProductString("RCRX_usbJoy_A"); // Display name on PC (alphanumeric characters)

    joyReport.reportID = 1;
    HID.begin(reportDescription, sizeof(reportDescription));

    // Serial2 (PA3: RX2)
    Serial2.begin(100000, SERIAL_8E2);  // change for CRSF to SBUS
}

void loop() {
    bool gotNewPacket = false;
    
// --- change for CRSF to SBUS ---
    while (Serial2.available() > 0) {
        lastByteTime = millis();
        uint8_t b = Serial2.read();
        
        // Determining the start of a packet (Start Byte)
        if (ptr == 0) {
            if (b == 0x0F) {
                sbus_packet[ptr++] = b;
            }
            continue;
        }

        // Data Storage
        sbus_packet[ptr++] = b;

        // Packet completion determination (SBUS is always fixed at 25 bytes)
        if (ptr == 25) {

            // Verify that the last End Byte is 0x00 (SBUS standard)
            if (sbus_packet[24] == 0x00) {
                parseSbus(); // Call the SBUS parsing function described later.
                gotNewPacket = true; 
            }
            ptr = 0; //Reset when finished
        }
        
        if (ptr >= 25) ptr = 0; // Buffer overflow prevention
    }

    if (millis() - lastByteTime > 10 && ptr != 0) {
        ptr = 0;
    }

    // Processing upon successful packet reception
    if (gotNewPacket) {
        updateJoystick(); 
        digitalWrite(PC13, !digitalRead(PC13)); 
    }

}

// --- change for CRSF to SBUS ---
void parseSbus() {
    channels[0]  = ((sbus_packet[1]       | sbus_packet[2]  << 8) & 0x07FF);
    channels[1]  = ((sbus_packet[2]  >> 3 | sbus_packet[3]  << 5) & 0x07FF);
    channels[2]  = ((sbus_packet[3]  >> 6 | sbus_packet[4]  << 2  | sbus_packet[5]  << 10) & 0x07FF);
    channels[3]  = ((sbus_packet[5]  >> 1 | sbus_packet[6]  << 7) & 0x07FF);
    channels[4]  = ((sbus_packet[6]  >> 4 | sbus_packet[7]  << 4) & 0x07FF);
    channels[5]  = ((sbus_packet[7]  >> 7 | sbus_packet[8]  << 1  | sbus_packet[9]  <<  9) & 0x07FF);
    channels[6]  = ((sbus_packet[9]  >> 2 | sbus_packet[10] << 6) & 0x07FF);
    channels[7]  = ((sbus_packet[10] >> 5 | sbus_packet[11] << 3) & 0x07FF);
    
    channels[8]  = ((sbus_packet[12]      | sbus_packet[13] << 8) & 0x07FF);
    channels[9]  = ((sbus_packet[13] >> 3 | sbus_packet[14] << 5) & 0x07FF);
    channels[10] = ((sbus_packet[14] >> 6 | sbus_packet[15] << 2  | sbus_packet[16] << 10) & 0x07FF);
    channels[11] = ((sbus_packet[16] >> 1 | sbus_packet[17] << 7) & 0x07FF);
    channels[12] = ((sbus_packet[17] >> 4 | sbus_packet[18] << 4) & 0x07FF);
    channels[13] = ((sbus_packet[18] >> 7 | sbus_packet[19] << 1  | sbus_packet[20] <<  9) & 0x07FF);
    channels[14] = ((sbus_packet[20] >> 2 | sbus_packet[21] << 6) & 0x07FF);
    channels[15] = ((sbus_packet[21] >> 5 | sbus_packet[22] << 3) & 0x07FF);
}

void updateJoystick() {
    // --- Update Axes ---
    // Mapping: x=ch1, y=ch2, z=ch3, rx=ch4, ry=ch11, rz=ch12, slider=ch6, dial=ch7
    joyReport.x      = map(channels[0],  172, 1811, 0, 65535);
    joyReport.y      = map(channels[1],  172, 1811, 0, 65535);
    joyReport.z      = map(channels[2],  172, 1811, 0, 65535);
    joyReport.rx     = map(channels[3],  172, 1811, 0, 65535);
    joyReport.ry     = map(channels[10], 172, 1811, 0, 65535);
    joyReport.rz     = map(channels[11], 172, 1811, 0, 65535);
    joyReport.slider = map(channels[5],  172, 1811, 0, 65535);
    joyReport.dial   = map(channels[6],  172, 1811, 0, 65535);

    // --- Map each channel to buttons ---
    // Use setButton(buttonNumber, condition) to update joyReport.buttons bits.
    
    // [CH5]
    uint16_t c5 = channels[4];
    setButton(1,  c5 > 1600);                 // High
    // setButton(2,  c5 >  800 && c5 < 1200); // Mid
    // setButton(3,  c5 <  400);              // Low

    // [CH6]
    uint16_t c6 = channels[5];
    setButton(4,  c6 > 1600);
    setButton(5,  c6 >  800 && c6 < 1200);
    setButton(6,  c6 <  400);

    // [CH7]
    uint16_t c7 = channels[6];
    setButton(7,  c7 > 1600);
    setButton(8,  c7 >  800 && c7 < 1200);
    setButton(9,  c7 <  400);

    // [CH8]
    uint16_t c8 = channels[7];
    setButton(10, c8 > 1600);
    // setButton(11, c8 >  800 && c8 < 1200);
    // setButton(12, c8 <  400);

    // [CH9]
    uint16_t c9 = channels[8];
    setButton(13, c9 > 1600);
    setButton(14, c9 >  800 && c9 < 1200);
    setButton(15, c9 <  400);

    // [CH10]
    uint16_t c10 = channels[9];
    setButton(16, c10 > 1600);
    setButton(17, c10 >  800 && c10 < 1200);
    setButton(18, c10 <  400);

    // [CH11]
    uint16_t c11 = channels[10];
    setButton(19, c11 > 1600);
    setButton(20, c11 >  800 && c11 < 1200);
    setButton(21, c11 <  400);

    // [CH12]
    uint16_t c12 = channels[11];
    setButton(22, c12 > 1600);
    setButton(23, c12 >  800 && c12 < 1200);
    setButton(24, c12 <  400);

    // [CH13]
    // uint16_t c13 = channels[12];
    // setButton(25, c13 > 1600);
    // setButton(26, c13 >  800 && c13 < 1200);
    // setButton(27, c13 <  400);

    // [CH14]
    // uint16_t c14 = channels[13];
    // setButton( 2, c14 > 1600);
    // setButton(11, c14 >  800 && c14 < 1200);
    // setButton( 3, c14 <  400);

    // [CH15]
    // uint16_t c15 = channels[14];
    // setButton(28, c15 > 1600);
    // setButton(29, c15 >  800 && c15 < 1200);
    // setButton(30, c15 <  400);
    
    // [CH16]
    // --- Map channel to 6 buttons ---
    uint16_t c16 = channels[15];
    setButton(27, c16 <  300);                    // -100%  (172)
    setButton(28, c16 >  300 && c16 <  600);      //  -60%  (512)
    setButton(29, c16 >  600 && c16 <  900);      //  -20%  (854)
    // --- This is the blank area where the mixer is at 0% (1024) ---
    setButton(30, c16 > 1100 && c16 < 1350);      //  +20% (1194)
    setButton(31, c16 > 1350 && c16 < 1650);      //  +60% (1535)
    setButton(32, c16 > 1650);                    // +100% (1811)

    // Send the custom HID report
    reporter.sendReport();
}
