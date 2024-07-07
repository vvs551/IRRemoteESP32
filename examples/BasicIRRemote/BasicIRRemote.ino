#include <IRRemoteESP32.h>

IRRemoteESP32 irRemote;

void setup() {
    Serial.begin(115200);
    irRemote = IRRemoteESP32();
}

void loop() {
    int result = irRemote.checkRemote();
    if (result != -1) {
        Serial.println(result);
    }
    delay(100);
}
