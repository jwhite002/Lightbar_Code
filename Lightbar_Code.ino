#include "Libraries/LightBar.h"

void setup() {
    Serial.begin(9600);

    // initialize all strips
    for (int i = 0; i < 4; i++) {
        strips[i].begin();
        strips[i].setBrightness(50);
        strips[i].show();
    }
}

// headlight array (THIS IS NOT WORKING FOR SOME REASON. THE REST OF THE CODE WORKS FINE)
LedControl leds[4] = {
    // driver side headlight
    LedControl(strips[0], 0, 255, 0, 200, 200, 500, 0, 42, 100), // low beam
    LedControl(strips[0], 1, 255, 0, 200, 200, 500, 0, 42, 100), // high beam
    // passenger side headlight
    LedControl(strips[1], 2, 255, 0, 200, 200, 500, 0, 0, 100), // low beam
    LedControl(strips[1], 3, 255, 0, 200, 200, 500, 0, 0, 100) // high beam
};

void loop() {
    // update all led's
    for (int i = 0; i < 4; ++i) {
        leds[i].update();
    }
    for (int i = 0; i < 4; i++) {
        strips[i].show();
    }
}
