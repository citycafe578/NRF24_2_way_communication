#include <SPI.h>
#include <RF24.h>

RF24 radio(7, 8);
const byte address[6] = "00001";
char receivedData[32] = { 0 };
bool isSynced = false;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Arduino Initialized!");

  if (!radio.begin()) {
    Serial.println("Arduino: Radio hardware is not responding!");
    while (1)
      ;
  }

  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_2MBPS);
  radio.setChannel(100);
  radio.openWritingPipe(address);
  radio.openReadingPipe(1, address);
  radio.setAutoAck(false);
  radio.stopListening();
}

void loop() {
  const char cmd[] = "CMD:DATA";
  bool success = radio.write(&cmd, sizeof(cmd));

  if (success) {
    Serial.println("send successfully");
    radio.startListening();
    unsigned long startTime = millis();
    while (millis() - startTime < 50) {
      if (radio.available()) {
        radio.read(&receivedData, sizeof(receivedData));
        Serial.print("Receive: ");
        Serial.println(receivedData);
      }
    }
    radio.stopListening();
  } else {
    Serial.println("send failed!");
  }
  delay(250);
}