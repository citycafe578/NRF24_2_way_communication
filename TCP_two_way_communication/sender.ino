#include <SPI.h>
#include <RF24.h>

RF24 radio(4, 5);  // CE: GPIO 4, CSN: GPIO 5
const byte address[6] = "00001";  // RF24 通訊地址

bool isConnected = false;  // 連線狀態
int retryLimit = 3;  // 最大重試次數
int modes = 0;  // 0: 接收模式, 1: 發送模式
TaskHandle_t clock_modeHandle;

void clock_mode(void *pvParam) {
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        vTaskDelay(200 / portTICK_PERIOD_MS);  // 每 200ms 切換一次模式

        modes = (modes + 1) % 2;
        if (modes == 0) {
            radio.startListening();
        } else {
            radio.stopListening();
        }

        Serial.print("Mode switched to: ");
        Serial.println(modes);
    }
}

void setup() {
    Serial.begin(115200);
    
    if (!radio.begin()) {
        Serial.println("Radio hardware is not responding!");
        while (1);
    }

    // RF24 設定
    radio.openReadingPipe(1, address);
    radio.setPALevel(RF24_PA_MAX);  // 最大功率
    radio.setDataRate(RF24_250KBPS);  // 低數據速率（增加穩定性）
    radio.setChannel(76);  // 設置頻道
    radio.setRetries(3, 15);  // 重試 3 次，每次間隔 4ms
    radio.startListening();

    // 創建任務
    xTaskCreatePinnedToCore(clock_mode, "clock_mode", 8192, NULL, 1, &clock_modeHandle, 1);
}

void loop() {
    static unsigned long lastSwitch = 0;
    if (millis() - lastSwitch >= 200) {
        lastSwitch = millis();
        modes = (modes + 1) % 2;

        if (modes == 0) {
            radio.startListening();
        } else {
            radio.stopListening();
        }

        Serial.print("Mode switched to: ");
        Serial.println(modes);
    }

    if (!isConnected) {
        Serial.println("Waiting for 'knock knock'...");
        char buffer[32] = {0};
        if (receiveMessage(buffer) && strcmp(buffer, "knock knock") == 0) {
            Serial.println("Received: knock knock");
            Serial.println("Sent: who's there");
            sendMessageWithAck("who's there");

            Serial.println("Waiting for 'it's ground'...");
            if (receiveMessage(buffer) && strcmp(buffer, "it's ground") == 0) {
                Serial.println("Received: it's ground");
                isConnected = true;
                Serial.println("Connection established!");
                xTaskNotifyGive(clock_modeHandle);
            }
        }
    } else {
        if (modes == 0) {  // 接收模式
            char buffer[32] = {0};
            if (receiveMessage(buffer)) {
                Serial.print("Received: ");
                Serial.println(buffer);
            }
        } else {  // 發送模式
            sendMessageWithAck("Hello, ground");
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }
}

bool sendMessageWithAck(const char* message) {
    radio.stopListening();
    delay(100);  // 確保模式切換完成

    int attempts = 0;
    while (attempts < retryLimit) {
        if (radio.write(message, strlen(message) + 1)) {
            Serial.println("Message sent successfully.");
            
            char response[32] = {0};
            if (receiveMessage(response)) {
                Serial.print("Received response: ");
                Serial.println(response);
                radio.startListening();
                return true;
            }
        }
        attempts++;
        Serial.print("Retry sending... Attempt ");
        Serial.println(attempts);
        delay(200);
    }
    Serial.println("Failed to send message.");
    radio.startListening();
    return false;
}

bool receiveMessage(char* buffer) {
    radio.startListening();
    unsigned long timeout = millis() + 3000;

    while (millis() < timeout) {
        if (radio.available()) {
            radio.read(buffer, 32);
            Serial.print("Received: ");
            Serial.println(buffer);
            return true;
        }
        delay(10);
    }

    Serial.println("Receive timeout.");
    return false;
}
