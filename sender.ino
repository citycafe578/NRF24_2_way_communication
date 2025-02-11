#include <SPI.h>
#include <RF24.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

RF24 radio(4, 5);
const byte address[6] = "00001";
int modes = 1;
// TaskHandle_t clockModeHandle = NULL;

void clock_mode(void *pvParam) {
  for (;;) {
    vTaskDelay(200 / portTICK_PERIOD_MS);
    modes = (modes + 1) % 2;
  }
}

void setup() {
  Serial.begin(115200);
  if (!radio.begin()) {
    Serial.println("Radio hardware is not responding!");
    while (1);
  }

  radio.openReadingPipe(1, address);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setChannel(100);
  radio.startListening();
  
  xTaskCreatePinnedToCore(clock_mode, "clock_mode", 2048, NULL, 1, NULL, 1);
}

void loop() {
  if(modes == 0){
    radio.stopListening();
    delay(10);
    const char text[] = "Hello, ground";
    radio.write(&text, sizeof(text));
    Serial.println("message sent!");
  }else{
    radio.startListening();
    delay(10);
    char text[32] = "";
    if(radio.available()){
      radio.read(&text, sizeof(text));
      Serial.print("Received: ");
      Serial.println(text);
    }
  }
}
