/*
 * BluePill_CRSF_rc_receiver_to_usb_joystick
 * * Version: 1.1.0
 * Copyright (c) 2026 kenbou1911
 * Licensed under the MIT License.
 * (See LICENSE file in the project root for details)
 */

#include <USBComposite.h>

// --- Custom HID Report Structure (8 axes x 10-bit + 32 buttons) ---
struct MyJoystickReport_t {
    uint8_t reportID;      
    unsigned x      : 10;
    unsigned y      : 10;
    unsigned z      : 10;
    unsigned rx     : 10;
    unsigned ry     : 10;
    unsigned rz     : 10;
    unsigned slider : 10;
    unsigned dial   : 10;
    uint32_t buttons;      
} __attribute__((packed));

MyJoystickReport_t joyReport;

// --- HID Descriptor ---
const uint8_t reportDescription[] = {
    0x05, 0x01,             // USAGE_PAGE (Generic Desktop)
    0x09, 0x04,             // USAGE (Joystick)
    0xA1, 0x01,             // COLLECTION (Application)
    0x85, 0x01,             //   REPORT_ID (1)

    // --- 8 Axes (10-bit each) ---
    0x05, 0x01,
    0xA1, 0x00,             // COLLECTION (Physical)
    0x15, 0x00,             // LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x03,       // LOGICAL_MAXIMUM (1023)
    0x75, 0x0A,             // REPORT_SIZE (10-bit)
    0x95, 0x08,             // REPORT_COUNT (8)
    0x09, 0x30, 0x09, 0x31, 0x09, 0x32, 0x09, 0x33, 
    0x09, 0x34, 0x09, 0x35, 0x09, 0x36, 0x09, 0x37,
    0x81, 0x02,             // INPUT (Data,Var,Abs)
    0xC0,                   // END_COLLECTION

    // --- 32 Buttons ---
    0x05, 0x09,             // USAGE_PAGE (Button)
    0x19, 0x01, 0x29, 0x20, // USAGE_MIN(1), MAX(32)
    0x15, 0x00, 0x25, 0x01, // LOGICAL_MIN(0), MAX(1)
    0x75, 0x01, 0x95, 0x20, // SIZE(1), COUNT(32)
    0x81, 0x02,             // INPUT (Data,Var,Abs)
    
    0xC0                    // END_COLLECTION
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

// --- CRSF Parsing Variables ---
uint8_t crsf_packet[64];
uint16_t channels[16];
uint8_t ptr = 0;
uint32_t lastByteTime = 0;

void setup() {
    pinMode(PC13, OUTPUT);
    digitalWrite(PC13, HIGH);

    joyReport.reportID = 1;
    HID.begin(reportDescription, sizeof(reportDescription));

    // Serial2 (PA3: RX2)
    Serial2.begin(420000);
}

void loop() {
    bool gotNewPacket = false; 

    while (Serial2.available() > 0) {
        lastByteTime = millis();
        uint8_t b = Serial2.read();
        
        if (ptr == 0) {
            if (b == 0xEE || b == 0xC8) {
                crsf_packet[ptr++] = b;
            }
            continue;
        }

        crsf_packet[ptr++] = b;

        if (ptr == 3) {
            if (crsf_packet[2] != 0x16) {
                ptr = 0;
                continue;
            }
        }

        if (ptr >= 2 && ptr == crsf_packet[1] + 2) {
            parseCrsf(); 
            gotNewPacket = true; 
            ptr = 0; 
        }
        
        if (ptr >= 64) ptr = 0;
    }

    if (millis() - lastByteTime > 10 && ptr != 0) {
        ptr = 0;
    }

    if (gotNewPacket) {
        updateJoystick(); 
        digitalWrite(PC13, !digitalRead(PC13)); 
    }
}

void parseCrsf() {
    channels[0]  = ((crsf_packet[3]       | crsf_packet[4]  << 8)                         & 0x07FF);
    channels[1]  = ((crsf_packet[4]  >> 3 | crsf_packet[5]  << 5)                         & 0x07FF);
    channels[2]  = ((crsf_packet[5]  >> 6 | crsf_packet[6]  << 2 | crsf_packet[7] << 10)  & 0x07FF);
    channels[3]  = ((crsf_packet[7]  >> 1 | crsf_packet[8]  << 7)                         & 0x07FF);
    channels[4]  = ((crsf_packet[8]  >> 4 | crsf_packet[9]  << 4)                         & 0x07FF);
    channels[5]  = ((crsf_packet[9]  >> 7 | crsf_packet[10] << 1 | crsf_packet[11] << 9)  & 0x07FF);
    channels[6]  = ((crsf_packet[11] >> 2 | crsf_packet[12] << 6)                         & 0x07FF);
    channels[7]  = ((crsf_packet[12] >> 5 | crsf_packet[13] << 3)                         & 0x07FF);
    channels[8]  = ((crsf_packet[14]      | crsf_packet[15] << 8)                         & 0x07FF);
    channels[9]  = ((crsf_packet[15] >> 3 | crsf_packet[16] << 5)                         & 0x07FF);
    channels[10] = ((crsf_packet[16] >> 6 | crsf_packet[17] << 2 | crsf_packet[18] << 10) & 0x07FF);
    channels[11] = ((crsf_packet[18] >> 1 | crsf_packet[19] << 7)                         & 0x07FF);
}

void updateJoystick() {
    // --- Update Axes ---
    // Mapping: x=ch1, y=ch2, z=ch3, rx=ch4, ry=ch11, rz=ch12, slider=ch6, dial=ch7
    joyReport.x      = map(channels[0],  172, 1811, 0, 1023);
    joyReport.y      = map(channels[1],  172, 1811, 0, 1023);
    joyReport.z      = map(channels[2],  172, 1811, 0, 1023);
    joyReport.rx     = map(channels[3],  172, 1811, 0, 1023);
    joyReport.ry     = map(channels[10], 172, 1811, 0, 1023);
    joyReport.rz     = map(channels[11], 172, 1811, 0, 1023);
    joyReport.slider = map(channels[5],  172, 1811, 0, 1023);
    joyReport.dial   = map(channels[6],  172, 1811, 0, 1023);

    // --- Map each channel to 3 buttons (Comment out unused lines) ---
    // Use setButton(buttonNumber, condition) to update joyReport.buttons bits.

    // [CH5]
    uint16_t c5 = channels[4];
    setButton(1,  c5 > 1600);                // High
    // setButton(2,  c5 > 800 && c5 < 1200); // Mid
    // setButton(3,  c5 < 400);              // Low

    // [CH6]
    uint16_t c6 = channels[5];
    setButton(4,  c6 > 1600);
    setButton(5,  c6 > 800 && c6 < 1200);
    setButton(6,  c6 < 400);

    // [CH7]
    uint16_t c7 = channels[6];
    setButton(7,  c7 > 1600);
    setButton(8,  c7 > 800 && c7 < 1200);
    setButton(9,  c7 < 400);

    // [CH8]
    uint16_t c8 = channels[7];
    setButton(10, c8 > 1600);
    // setButton(11, c8 > 800 && c8 < 1200);
    // setButton(12, c8 < 400);

    // [CH9]
    uint16_t c9 = channels[8];
    setButton(13, c9 > 1600);
    setButton(14, c9 > 800 && c9 < 1200);
    setButton(15, c9 < 400);

    // [CH10]
    uint16_t c10 = channels[9];
    setButton(16, c10 > 1600);
    setButton(17, c10 > 800 && c10 < 1200);
    setButton(18, c10 < 400);

    // [CH11]
    uint16_t c11 = channels[10];
    setButton(19, c11 > 1600);
    setButton(20, c11 > 800 && c11 < 1200);
    setButton(21, c11 < 400);

    // [CH12]
    uint16_t c12 = channels[11];
    setButton(22, c12 > 1600);
    setButton(23, c12 > 800 && c12 < 1200);
    setButton(24, c12 < 400);

    // Send the custom HID report
    reporter.sendReport();
}
