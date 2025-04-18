#include <Arduino.h>
#include <FastLED.h>

FASTLED_USING_NAMESPACE

#define LED_TYPE WS2812B
#define COLOR_ORDER GBR
#define NUM_LEDS 168

int changePatternDelay = 30;

char strftime_buf[64];
struct tm timeinfo;


// #include <WiFi.h>

// const char* ssid1 = "";
// const char* password1 = "";

// const char* ssid2 = "";
// const char* password2 = ";

// const int connectionAttempts = 100;
// const int connectionDelay = 1000;

// void connectToWiFi() {
//     Serial.begin(9600);
//     delay(connectionDelay);

//     Serial.print("Connecting to WiFi: ");
//     Serial.println(ssid1);
//     WiFi.begin(ssid1, password1);

//     int attempt = 0;
//     while (WiFi.status() != WL_CONNECTED && attempt < connectionAttempts) {
//         delay(connectionDelay);
//         Serial.print(".");
//         attempt++;
//     }

//     if (WiFi.status() != WL_CONNECTED) {
//         Serial.println("\nFailed to connect to Eeyore, trying FreeCandy");
//         WiFi.begin(ssid2, password2);
//         attempt = 0;
//         while (WiFi.status() != WL_CONNECTED && attempt < connectionAttempts) {
//             delay(connectionDelay);
//             Serial.print(".");
//             attempt++;
//         }
//     }

//     if (WiFi.status() == WL_CONNECTED) {
//         Serial.println("\nConnected to FreeCandy");
//         Serial.print("IP Address: ");
//         Serial.println(WiFi.localIP());
//     } else {
//         Serial.println("\nFailed to connect to any WiFi network");
//     }
// }

// void setupWiFi() {
//     connectToWiFi();
// }


CRGB leds[NUM_LEDS];

#define BRIGHTNESS 45
#define FRAMES_PER_SECOND 120

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0;                  // rotating "base color" used by many of the patterns
typedef void (*SimplePatternList[])();

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void setup()
{
    delay(300); // 3 second delay for recovery
    // FastLED.addLeds<LED_TYPE, GPIO_NUM_18, GPIO_NUM_5, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.addLeds<LED_TYPE, GPIO_NUM_18, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);

    // xTaskCreatePinnedToCore(
    //     [](void* pvParameters) {
    //         setupWiFi();
    //         vTaskDelete(NULL);
    //     },
    //     "WiFiSetupTask",
    //     4096,
    //     NULL,
    //     1,
    //     NULL,
    //     1
    // );
    setenv("TZ", "UTC-8", 1);
    tzset();
}

void rainbow()
{
    // FastLED's built-in rainbow generator
    fill_rainbow(leds, NUM_LEDS, gHue, 25);
}

void addGlitter(fract8 chanceOfGlitter)
{
    if (random8() < chanceOfGlitter)
    {
        leds[random16(NUM_LEDS)] += CRGB::White;
    }
}

void confetti()
{
    // random colored speckles that blink in and fade smoothly
    fadeToBlackBy(leds, NUM_LEDS, 10);
    int pos = random16(NUM_LEDS);
    leds[pos] += CHSV(gHue + random8(64), 200, 255);
}

void sinelon()
{
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy(leds, NUM_LEDS, 1);
    int pos = beatsin16(2, 0, NUM_LEDS - 1);
    // int pos = beatsin16(2, 0, NUM_LEDS - 1, -1, gHue);
    leds[pos] += CHSV(gHue, 255, 192);
}

void bpm()
{
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = PartyColors_p;
    uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
    for (int i = 0; i < NUM_LEDS; i++)
    { // 9948
        leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
    }
}

void juggle()
{
    // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy(leds, NUM_LEDS, 20);
    uint8_t dothue = 0;
    for (int i = 0; i < 8; i++)
    {
        leds[beatsin16(i + 7, 0, NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
        dothue += 32;
    }
}

void rainbowWithGlitter()
{
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow();
    addGlitter(80);
}

// // List of patterns to cycle through.  Each is defined as a separate function below.
SimplePatternList gPatterns = {
    rainbow,
    rainbowWithGlitter, 
    sinelon,
    juggle,
    bpm
    };

void nextPattern()
{
    // add one to the current pattern number, and wrap around at the end
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);
}

void printTime() {
    time_t now;
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    Serial.println(strftime_buf);
}

void loop()
{
    // Call the current pattern function once, updating the 'leds' array
    gPatterns[gCurrentPatternNumber]();

    // send the 'leds' array out to the actual LED strip
    FastLED.show();
    // insert a delay to keep the framerate modest
    FastLED.delay(1000 / FRAMES_PER_SECOND);

    // do some periodic updates
    // 20 for later
    // Serial.println(gHue);
    EVERY_N_MILLISECONDS(3) { gHue++; }  // slowly cycle the "base color" through the rainbow
    EVERY_N_SECONDS(changePatternDelay) { nextPattern(); } // change patterns periodically
    EVERY_N_SECONDS(1) { printTime(); }




}
