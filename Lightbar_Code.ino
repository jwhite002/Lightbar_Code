#include "Libraries/LightBar.h"

void setup() {
    Serial.begin(9600);

    strip.begin();
    strip.setBrightness(50);
    strip.show();
}

LedRotator test(0, 50, 700, 0);

void loop() {
    test.update();
    strip.show();
}
