#include <Adafruit_NeoPixel.h>
#include <millisDelay.h>

#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define LED_PIN    9
#define LED_COUNT 7

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
int hue_conv = (int)((42.0/360.0)*65536.0);

enum LED_STATE {
  RISE, 
  ON_MAX, 
  FALL, 
  ON_MIN
};

class LedControl {
private:
    int ledNum;
    int maxBrightness;
    int minBrightness;
    millisDelay pulseDelay;
    millisDelay onMaxDelay;
    millisDelay onMinDelay;
    LED_STATE ledState;
    int currentBrightness;
    int stepDelay; // Delay for each individual brightness step
    int hueVal;
  
public:
    LedControl(int n, int brightness_max, int brightness_min, int delay_on, int delay_off, int delay_pulse, int start_brightness, int hue)
      : ledNum(n), maxBrightness(brightness_max), minBrightness(brightness_min), ledState(RISE), currentBrightness(start_brightness), hueVal(hue) {
        
        int totalSteps = maxBrightness - minBrightness; // Calculate total number of brightness steps
        stepDelay = delay_pulse / totalSteps;           // Calculate delay for each step
        
        pulseDelay.start(stepDelay);                    // Initialize pulseDelay with stepDelay
        onMaxDelay.start(delay_on);
        onMinDelay.start(delay_off);
    }

    void update() {
        switch(ledState) {
            case RISE:
                if(pulseDelay.justFinished()) {
                    currentBrightness++;
                    if(currentBrightness >= maxBrightness) {
                        currentBrightness = maxBrightness;
                        ledState = ON_MAX;
                        onMaxDelay.restart();
                    }
                    pulseDelay.start(stepDelay); // Restart with stepDelay
                }
                break;
            case ON_MAX:
                if(onMaxDelay.justFinished()) {
                    ledState = FALL;
                }
                break;
            case FALL:
                if(pulseDelay.justFinished()) {
                    currentBrightness--;
                    if(currentBrightness <= minBrightness) {
                        currentBrightness = minBrightness;
                        ledState = ON_MIN;
                        onMinDelay.restart();
                    }
                    pulseDelay.start(stepDelay); // Restart with stepDelay
                }
                break;
            case ON_MIN:
                if(onMinDelay.justFinished()) {
                    ledState = RISE;
                }
                break;
        }

        uint32_t color = strip.gamma32(strip.ColorHSV((hueVal/360.0)*65536.0, 255, currentBrightness));
        strip.setPixelColor(ledNum, color);
    }
};

#define NUM_LEDS 7
LedControl leds[NUM_LEDS] = {
    LedControl(0, 255, 0, 133, 400, 133, 0, 0),
    LedControl(1, 255, 0, 133, 267, 100, 0, 0),
    LedControl(2, 255, 0, 100, 167, 67, 0, 42),
    LedControl(3, 255, 50, 67, 233, 233, 0, 0),
    LedControl(4, 255, 0, 100, 167, 67, 0, 42),
    LedControl(5, 255, 0, 133, 267, 100, 0, 0),
    LedControl(6, 255, 0, 133, 400, 133, 0, 0)
};

void setup() {
    strip.begin();
    strip.show();
}

void loop() {
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i].update();
    }
    strip.show();
}
