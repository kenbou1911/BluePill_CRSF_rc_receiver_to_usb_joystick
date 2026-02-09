/*
 * BluePill_CRSF_rc_receiver_to_usb_joystick
 * * Version: 1.0.0
 * Copyright (c) 2026 kenbou1911
 * Licensed under the MIT License.
 * (See LICENSE file in the project root for details)
 */

#include <USBComposite.h>

// --- USB HID Configuration (6-axis, 32-button) ---
USBHID HID;
HIDJoystick Joystick(HID); 

// --- CRSF Parsing Variables ---
uint8_t crsf_packet[64];
uint16_t channels[16];
uint8_t ptr = 0;
uint32_t lastByteTime = 0;

void setup() {
    pinMode(PC13, OUTPUT);
    digitalWrite(PC13, HIGH);

    HID.begin(HID_JOYSTICK);
    Joystick.setManualReportMode(true);
    Joystick.begin();

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
            parseCrsf(); // Parse and store in the channel variable
            gotNewPacket = true; // Raise a flag to indicate that the latest data has arrived
            ptr = 0; 
            
        }
        
        if (ptr >= 64) ptr = 0;
        
    }

    
        if (millis() - lastByteTime > 10 && ptr != 0) {
            ptr = 0;
        
    }

    
        if (gotNewPacket) {
            updateJoystick(); 
        
        // LED inversion (signal of successful analysis)
        digitalWrite(PC13, !digitalRead(PC13)); 
    }
}

void parseCrsf() {
    // Decode 11-bit channel data from CRSF packet
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
    Joystick.X(map(channels[0], 172, 1811, 0, 1023));
    Joystick.Y(map(channels[1], 172, 1811, 0, 1023));
    Joystick.Xrotate(map(channels[2], 172, 1811, 0, 1023));
    Joystick.Yrotate(map(channels[3], 172, 1811, 0, 1023));
    Joystick.sliderLeft(map(channels[10], 172, 1811, 0, 1023));
    Joystick.sliderRight(map(channels[11], 172, 1811, 0, 1023));

    // --- Map each channel to 3 buttons (Comment out unused lines) ---

    // [CH5]
    uint16_t c5 = channels[4];
    Joystick.button(1,  c5 > 1600);                // High
    // Joystick.button(2,  c5 > 800 && c5 < 1200); // Mid - Not needed for 2-pos
    // Joystick.button(3,  c5 < 400);              // Low

    // [CH6]
    uint16_t c6 = channels[5];
    Joystick.button(4,  c6 > 1600);
    Joystick.button(5,  c6 > 800 && c6 < 1200);
    Joystick.button(6,  c6 < 400);

    // [CH7]
    uint16_t c7 = channels[6];
    Joystick.button(7,  c7 > 1600);
    Joystick.button(8,  c7 > 800 && c7 < 1200);
    Joystick.button(9,  c7 < 400);

    // [CH8]
    uint16_t c8 = channels[7];
    Joystick.button(10, c8 > 1600);
    // Joystick.button(11, c8 > 800 && c8 < 1200); // Not needed for 2-pos
    // Joystick.button(12, c8 < 400);

    // [CH9]
    uint16_t c9 = channels[8];
    Joystick.button(13, c9 > 1600);
    Joystick.button(14, c9 > 800 && c9 < 1200);
    Joystick.button(15, c9 < 400);

    // [CH10]
    uint16_t c10 = channels[9];
    Joystick.button(16, c10 > 1600);
    Joystick.button(17, c10 > 800 && c10 < 1200);
    Joystick.button(18, c10 < 400);

    // [CH11]
    uint16_t c11 = channels[10];
    Joystick.button(19, c11 > 1600);
    Joystick.button(20, c11 > 800 && c11 < 1200);
    Joystick.button(21, c11 < 400);

    // [CH12]
    uint16_t c12 = channels[11];
    Joystick.button(22, c12 > 1600);
    Joystick.button(23, c12 > 800 && c12 < 1200);
    Joystick.button(24, c12 < 400);

    // Send the HID report once per update
    Joystick.send(); 
}
