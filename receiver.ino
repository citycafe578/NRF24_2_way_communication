#include <SPI.h>
#include <RF24.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

RF24 radio(4, 5);
const byte address[6] = "00001";
volatile int mode = 1;

void clock_mode(void *pvParam){
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    mode = (mode + 1) % 2;

    if(mode == 0){
        radio.startListening();
    }else{
      radio.stopListening();
    }

    Serial.print("ESP32 Mode changed to: ");
    Serial.println(mode);
  }
}

void setup(){
  Serial.begin(115200);
  delay(2000);
  Serial.println("ESP32 Initialized!");

  if (!radio.begin()){
    Serial.println("ESP32: Radio hardware is not responding!");
    while (1);
  }

  radio.setPALevel(RF24_PA_HIGH);
  radio.setChannel(100);
  radio.openWritingPipe(address);
  radio.openReadingPipe(1, address);
  radio.stopListening();
  xTaskCreatePinnedToCore(clock_mode, "clock_mode", 1024, NULL, 1, NULL, 1);
}

void loop(){
  const char text[] = "Hello, ground";
    
  if(mode == 0){
    if(radio.available()){
      char receivedData[32] = {0};
      radio.read(&receivedData, sizeof(receivedData));
      Serial.print("ESP32 Received: ");
      Serial.println(receivedData);
    }else{
      Serial.println("ESP32: No data");
      }
  }else{
    bool success = radio.write(&text, strlen(text) + 1);
    if (success){
      Serial.println("ESP32: Send successful");
    }else{
      Serial.println("ESP32: Send failed");
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}
