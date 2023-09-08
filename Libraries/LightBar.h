#ifndef LIGHTBAR.H
#define LIGHTBAR.H

#include <Adafruit_NeoPixel.h>
#include <millisDelay.h>

// define and declare all strips to be used
Adafruit_NeoPixel strips[4] = {
    Adafruit_NeoPixel(4, 5, NEO_GRB + NEO_KHZ800), // driver side headlight
    Adafruit_NeoPixel(4, 9, NEO_GRB + NEO_KHZ800), // passenger side headlight
    Adafruit_NeoPixel(4, 10, NEO_GRB + NEO_KHZ800), // driver side taillight
    Adafruit_NeoPixel(4, 11, NEO_GRB + NEO_KHZ800), // passenger side taillight
};

#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif


enum LED_STATE {
  RISE, 
  ON_MAX, 
  FALL, 
  ON_MIN
};

/*!
    @brief   Class that stores state and functions for controlling the rise/fall
             time and brightness of a NeoPixel.
*/
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
    Adafruit_NeoPixel stripNum;
    double hueVal;
    double saturationVal;
    bool running;
  
public:
    /*!
    @brief   Class that stores state and functions for controlling the rise/fall
             time and brightness of a NeoPixel.
    @param   strip              The name of the strip to be affected
    @param   n                  The index of the LED to be modified (starts at 0)
    @param   brightness_max     The maximum brightness the LED can achieve (0 to 255)
    @param   brightness_min     The minimum brightness the LED can achieve (0 to 255)
    @param   delay_on           Time the LED stays at its maximum brightness (in milliseconds)
    @param   delay_off          Time the LED stays at its minimum brightness (in milliseconds)
    @param   delay_pulse        Time it takes the LED to bright/dim completely (in milliseconds)
    @param   start_brightness   The initial brightness that the LED starts with (0 to 255)
    @param   hue                In reference to the Hue, Saturation, and Value color model (0 to 360 degrees)
    @param   saturation         In reference to the Hue, Saturation, and Value color model (0 to 100 percent)
    */
    LedControl(Adafruit_NeoPixel strip, int n, int brightness_max, int brightness_min, int delay_on, int delay_off, int delay_pulse, int start_brightness, double hue, double saturation)
      : stripNum(strip), ledNum(n), maxBrightness(brightness_max), minBrightness(brightness_min), ledState(RISE), currentBrightness(start_brightness), hueVal(hue), saturationVal(saturation), running(true) {
        
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

        uint32_t color = stripNum.gamma32(stripNum.ColorHSV((hueVal/360.0)*65536.0, (saturationVal/100.0)*255.0, currentBrightness));
        stripNum.setPixelColor(ledNum, color);
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

/*!
    @brief   Class that contains functions to simulate the light emitted from a rotator
             using a series of desired LEDs
*/
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
    Adafruit_NeoPixel stripNum;

public:
    /*!
    @brief   Class that contains functions to simulate the light emitted from a rotator
             using a series of desired LEDs.
    @param   strip              The name of the strip to be affected
    @param   n                  The starting LED index of the strip (starts at 0)
    @param   brightness_max     The maximum brightness the LED can achieve (0 to 255)
    @param   delay_revolution   Time to complete one revolution (in milliseconds)
    @param   hue                In reference to the Hue, Saturation, and Value color model (0 to 360 degrees)
    @note    There is no ability to change the number of LED's to simulate the rotator. As it stands now, 
             only four LED's are being used.
    */
    LedRotator(Adafruit_NeoPixel strip, int n, int brightness_max, int delay_revolution, int hue)
        : stripNum(strip), ledNum(n), maxBrightness(brightness_max), hueVal(hue), rotatorPosDeg(0.0) {
        
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
                ledColor[i] = stripNum.gamma32(stripNum.ColorHSV((hueVal/360.0)*65536.0, 255, ledBrightness[i]));
                stripNum.setPixelColor(ledNum+i, ledColor[i]);
            }

            rotatorPosDeg = (rotatorPosDeg > 359.0) ? 0.0 : rotatorPosDeg;
            rotatorPosDeg = rotatorPosDeg + 1.0;

            revolutionDelay.start(stepDelay); // Restart with stepDelay

        }
    }
    
};


#endif