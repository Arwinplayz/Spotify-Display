#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <WiFi.h>
#include <SpotifyEsp32.h>
#include <HTTPClient.h>
#include <SPI.h>

#define TFT_CS 1
#define TFT_RST 2
#define TFT_DC 3
#define TFT_SCLK 4
#define TFT_MOSI 5

char* SSID = "WiFi-B793-5G";
char* PASSWORD = "25324327";

const char* CLIENT_ID = "deac17a8479c4ceaa0e230443f3eaafc";
const char* CLIENT_SECRET = "0dbf02b13aa340828a57a4dcf5a03412";

Spotify sp(CLIENT_ID, CLIENT_SECRET);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

String lastTrack = "";
String lyrics = "";
int scrollY = 0;

String getLyrics(String artist, String track) {
    HTTPClient http;

    // Format URL (spaces -> %20)
    artist.replace(" ", "%20");
    track.replace(" ", "%20");

    String url = "https://api.lyrics.ovh/v1/" + artist + "/" + track;
    http.begin(url);

    int httpCode = http.GET();
    if (httpCode == 200) {
        String payload = http.getString();

        DynamicJsonDocument doc(8192);
        deserializeJson(doc, payload);

        String lyr = doc["lyrics"].as<String>();
        http.end();
        return lyr;
    }

    http.end();
    return "Lyrics not found.";
}

void drawLyrics() {
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);

    int y = -scrollY;
    int lineHeight = 10;

    int start = 0;
    int index;

    while ((index = lyrics.indexOf('\n', start)) != -1) {
        String line = lyrics.substring(start, index);

        tft.setCursor(0, y);
        tft.print(line);

        y += lineHeight;
        start = index + 1;
    }
}

void setup() {
    Serial.begin(115200);

    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);
    tft.fillScreen(ST77XX_BLACK);

    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    sp.begin();
    while (!sp.is_auth()) {
        sp.handle_client();
    }
}

void loop() {
    String artist = sp.current_artist_names();
    String track = sp.current_track_name();

    if (track != lastTrack && track != "null" && track != "Something went wrong") {
        lastTrack = track;

        tft.fillScreen(ST77XX_BLACK);
        tft.setCursor(0, 0);
        tft.print("Loading lyrics...");

        lyrics = getLyrics(artist, track);

        scrollY = 0;

        Serial.println("Lyrics loaded!");
    }

    drawLyrics();
    scrollY += 2;

    if (scrollY > 1000) {
        scrollY = 0;
    }

    delay(100);
}