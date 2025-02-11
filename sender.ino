#include <SPI.h>
#include <RF24.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

<<<<<<< HEAD
RF24 radio(4, 5);
const byte address[6] = "00001";
int modes = 1;
// TaskHandle_t clockModeHandle = NULL;

void clock_mode(void *pvParam) {
  for (;;) {
=======
RF24 radio(17, 5);
const byte address[][6] = {"00001", "00002"};
int data[4] = {};
bool mode = 0; // 0 == TX, 1 == RX
bool connected = false;
int lost_connected = 0;
int send_data = 1232;
int request_data = 0001;

void clock(void *pvParam){
  vTaskDelay(200 / portTICK_PERIOD_MS);
  mode++;
  if(mode > 2){
    mode = 0;
  }
}

void nrf_receive_Task(void *pvParam){
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  while (1) {
    if(mode == 1){

    }
    radio.startListening();
    if (radio.available()){
      read_data();
    }
    for(int i = 0; i < 4; i++){
      Serial.print(data[i]);
    }
    Serial.println();
>>>>>>> 5d68cb1f85fab6038cca8ad08e61beab028b80de
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

<<<<<<< HEAD
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
=======


void read_data(){
  char receivedData[64] = {0};
  radio.read(&receivedData, sizeof(receivedData));
  char *token = strtok(receivedData, " ");
  int index = 0;
  while (token != NULL && index < 4) {
    data[index] = atoi(token);
    index++;
    token = strtok(NULL, " ");
  }
}