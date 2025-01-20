#include <RF24.h>
#include <Wire.h>
#define SDA 21
#define SCL 22

RF24 radio(17, 5);
const byte address[][6] = {"00001", "00002"};
int data[4] = {};
bool mode = 0; // 0 == TX, 1 == RX

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
    for(int i = 0; i < 4; i++){
      Serial.print(data[i]);
    }
    Serial.println();
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(9600);
  if (!radio.begin()) {
    Serial.println("Radio hardware is not responding!");
    while (1);
  }
  radio.openWritingPipe(address[0]);
  radio.openReadingPipe(1, address[1]);
  radio.setPALevel(RF24_PA_MAX);
  Wire.begin(SDA, SCL);
  xTaskCreatePinnedToCore(nrf_receive_Task, "clock", 2048, NULL, 1, NULL, 1);
}

void loop(){
  if(mode == 0){

  }else{
    if (radio.available()){
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
    for(int i = 0; i < 4; i++){
      Serial.print(data[i]);
    }
    Serial.println();
  }
}
