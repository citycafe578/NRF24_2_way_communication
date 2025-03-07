#include <SPI.h>
#include <RF24.h>

RF24 radio(4, 5);  // CE: GPIO 4, CSN: GPIO 5
const byte address[6] = "00001";  // RF24 地址
bool isConnected = false;
int retryLimit = 3;
int modes = 1;  // 1 = 發送模式, 0 = 接收模式
TaskHandle_t clock_modeHandle;

void clock_mode(void *pvParam) {
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        vTaskDelay(200 / portTICK_PERIOD_MS);  // 每 200ms 切換模式

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
    radio.openWritingPipe(address);
    radio.openReadingPipe(1, address);
    radio.setPALevel(RF24_PA_MAX);  // 最大功率
    radio.setDataRate(RF24_250KBPS);  // 低數據速率提高穩定性
    radio.setChannel(76);
    radio.setRetries(3, 15);  // 重試 3 次，每次間隔 4ms
    radio.stopListening();  // 初始為發送模式

    // 創建模式切換任務
    xTaskCreatePinnedToCore(clock_mode, "clock_mode", 8192, NULL, 1, &clock_modeHandle, 1);
}

void loop() {
    static unsigned long lastSwitch = 0;
    if (millis() - lastSwitch >= 200) {  // 每 200ms 切換模式
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
        Serial.println("Sent: knock knock");
        sendMessageWithAck("knock knock");

        char buffer[32] = {0};
        if (receiveMessage(buffer) && strcmp(buffer, "who's there") == 0) {
            Serial.println("Received: who's there");
            Serial.println("Sent: it's ground");
            sendMessageWithAck("it's ground");

            isConnected = true;
            Serial.println("Connection established!");
            xTaskNotifyGive(clock_modeHandle);
        }
    } else {
        if (modes == 0) {  // 接收模式
            char buffer[32] = {0};
            if (receiveMessage(buffer)) {
                Serial.print("Received: ");
                Serial.println(buffer);
            }
        } else {  // 發送模式
            sendMessageWithAck("Hello, drone");
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }
}

bool sendMessageWithAck(const char* message) {
    radio.stopListening();
    delay(100);

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
