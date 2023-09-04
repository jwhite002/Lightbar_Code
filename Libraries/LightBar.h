#ifndef LIGHTBAR.H
#define LIGHTBAR.H

#include <Adafruit_NeoPixel.h>
#include <millisDelay.h>

#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define LED_PIN    9
#define LED_COUNT 7

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

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
    int currentBrightness;
    int stepDelay; // Delay for each individual brightness step
    millisDelay pulseDelay;
    millisDelay onMaxDelay;
    millisDelay onMinDelay;
    LED_STATE ledState;
    double hueVal;
    double saturationVal;
    bool running;
  
public:
    // allows control for an individual led
    LedControl(int n, int brightness_max, int brightness_min, int delay_on, int delay_off, int delay_pulse, int start_brightness, double hue, double saturation)
      : ledNum(n), maxBrightness(brightness_max), minBrightness(brightness_min), ledState(RISE), currentBrightness(start_brightness), hueVal(hue), saturationVal(saturation), running(true) {
        
        int totalSteps = maxBrightness - minBrightness; // Calculate total number of brightness steps
        stepDelay = delay_pulse / totalSteps;           // Calculate delay for each step
        
        pulseDelay.start(stepDelay);                    // Initialize pulseDelay with stepDelay
        onMaxDelay.start(delay_on);
        onMinDelay.start(delay_off);
    }

    void update() {
        if (!running) {
            return;
        }
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

        uint32_t color = strip.gamma32(strip.ColorHSV((hueVal/360.0)*65536.0, (saturationVal/100.0)*255.0, currentBrightness));
        strip.setPixelColor(ledNum, color);
    }

    void start() {
        running = true;
    }

    void stop() {
        running = false;
    }

    bool isRunning() {
        return this->running;
    }

    int get_state() {
        return this->ledState;
    }

};

class LedRotator {
private:
int ledNum;
    int maxBrightness;
    int stepDelay;
    int hueVal;
    int ledBrightness[4];
    uint32_t ledColor[4];
    double rotatorPosDeg;
    double rotatorPosRad;
    millisDelay revolutionDelay;

public:
    LedRotator(int n, int brightness_max, int delay_revolution, int hue)
        : ledNum(n), maxBrightness(brightness_max), hueVal(hue), rotatorPosDeg(0.0) {
        
        stepDelay = delay_revolution/360;
        revolutionDelay.start(stepDelay);
        }

    void update() {
        if(revolutionDelay.justFinished()) {

            rotatorPosRad = rotatorPosDeg*(PI/180.0); // convert to radians
            
            ledBrightness[0] = (int)(cos(rotatorPosRad)*255.0);
            ledBrightness[1] = (int)(-cos(rotatorPosRad + ((PI*1.0)/2))*255.0);
            ledBrightness[2] = (int)(cos(rotatorPosRad + ((PI*2.0)/2))*255.0);
            ledBrightness[3] = (int)(-cos(rotatorPosRad + ((PI*3.0)/2))*255.0);

            for (int i = 0; i < 4; i++) {
                ledBrightness[i] = (ledBrightness[i] < 0) ? 0 : ledBrightness[i];
                ledColor[i] = strip.gamma32(strip.ColorHSV((hueVal/360.0)*65536.0, 255, ledBrightness[i]));
                strip.setPixelColor(ledNum+i, ledColor[i]);
            }

            rotatorPosDeg = (rotatorPosDeg > 359.0) ? 0.0 : rotatorPosDeg;
            rotatorPosDeg = rotatorPosDeg + 1.0;

            revolutionDelay.start(stepDelay); // Restart with stepDelay

        }
    }
    
};

#endif