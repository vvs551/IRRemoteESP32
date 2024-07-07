#include <IRRemote.h>

IRRemote irRemote;

void setup() {
    Serial.begin(115200);
    irRemote = IRRemote();
}

void loop() {
    int result = irRemote.checkRemote();
    if (result != -1) {
        Serial.println(result);
    }
    delay(100);
}
