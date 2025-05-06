#include <SPI.h>
#include <RF24.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

RF24 radio(4, 5);
const byte address[6] = "00001";
char receivedData[32] = { 0 };
bool isSynced = false;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("ESP32 Initialized!");

  if (!radio.begin()) {
    Serial.println("ESP32: Radio hardware is not responding!");
    while (1)
      ;
  }

  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_2MBPS);
  radio.setChannel(100);
  radio.openWritingPipe(address);
  radio.openReadingPipe(1, address);
  radio.setAutoAck(false);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    radio.read(&receivedData, sizeof(receivedData));
    Serial.print("Received: ");
    Serial.println(receivedData);
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