#include <SPI.h>
#include <RF24.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

RF24 radio(4, 5);
const byte address[6] = "00001";
char receivedData[32] = {0};
bool isSynced = false;

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("ESP32 Initialized!");

    if (!radio.begin()) {
        Serial.println("ESP32: Radio hardware is not responding!");
        while (1);
    }

    radio.setPALevel(RF24_PA_HIGH);
    radio.setDataRate(RF24_2MBPS);
    radio.setChannel(100);
    radio.openWritingPipe(address);
    radio.openReadingPipe(1, address);
    radio.startListening();

    // **同步機制**
    Serial.println("ESP32: Waiting for SYNC...");
    unsigned long startTime = millis();
    while (millis() - startTime < 100) {
        if (radio.available()) {
            radio.read(&receivedData, sizeof(receivedData));
            if (strcmp(receivedData, "SYNC") == 0) {
                Serial.println("ESP32: SYNC received, sending READY...");
                radio.stopListening();
                const char readyMsg[] = "READY";
                radio.write(&readyMsg, sizeof(readyMsg));
                radio.startListening();
                isSynced = true;
                break;
            }
        }
    }
}

void loop() {
    if (radio.available()) {
        radio.read(&receivedData, sizeof(receivedData));
        Serial.print("ESP32: Received - ");
        Serial.println(receivedData);

        if (strncmp(receivedData, "CMD:", 4) == 0) {
            Serial.println("ESP32: Valid command received, sending ACK...");
            radio.stopListening();
            const char ackMsg[] = "ACK";
            bool success = radio.write(&ackMsg, sizeof(ackMsg));
            if (success) {
                Serial.println("ESP32: ACK sent successfully!");
            } else {
                Serial.println("ESP32: ACK send failed!");
            }
            radio.startListening();
        }
    }
}
