#include <Arduino.h>

#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>

/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 by Daniel Eichhorn
 * Copyright (c) 2016 by Fabrice Weinberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <TimeLib.h>

// Include the correct display library
// For a connection via I2C using Wire include
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

// Include the UI lib
#include "OLEDDisplayUi.h"

// Include custom images
#include "images.h"

#define TP_HOME "sensornet/env/home/#"
#define TP_HOME_ALARM "sensornet/status/#"
#define TP_COMMAND "sensornet/command/envconsole09/#"
#define TP_LIVTEMP  "/living/temperature"
#define TP_LIVHUMI  "/living/humidity"
#define TP_LIVAQI  "/living/aqi"
#define TP_LIVMOTION  "/living/motion"
#define TP_BALTEMP  "/balcony/temperature"
#define TP_BALHUMI  "/balcony/humidity"
#define TP_BALAQI  "/balcony/aqi"
#define TP_STATUS   "sensornet/status/envconsole09"
#define TP_TIMESTR "sensornet/time/shortdatetime"
#define TP_UNIXTIME "sensornet/time/timestamp"
#define ARMED    "ARMED"
#define DISARMED   "DISARMED"
#define CMD_DISP_OFF "display/off"
#define CMD_DISP_ON "display/on"

const char* ssid     = "You are being watched";
const char* password = "Akihabara";
const char* mqtt_server = "10.0.1.250";

bool debug = false;

// WiFi setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Initialize the OLED display using Wire library
SSD1306  display(0x3c, 5, 4);

OLEDDisplayUi ui ( &display );

// Various MQTT topics of interest
String tp_livtemp = TP_LIVTEMP;
String tp_livhumi = TP_LIVHUMI;
String tp_livaqi = TP_LIVAQI;
String tp_baltemp = TP_BALTEMP;
String tp_balhumi = TP_BALHUMI;
String tp_balaqi = TP_BALAQI;
String tp_status = TP_STATUS;
String livtemp="-",livhumi="-",livaqi="-",baltemp="-",balhumi="-",balaqi="-",sstatus="-",timeStr="Mon 00 00:00", saqi;

int timeOffset = 8*3600;  // TZ, in seconds
int screenW = 128;
int screenH = 64;
int clockRadius = 23;
int clockCenterX = clockRadius;
int clockCenterY = ((screenH-16)/2)+16;   // top yellow part is 16 px height
int digitalClockCenterY = clockCenterY - 12;
int digitalClockCenterX = screenW/2;
float liv_temp, liv_rh;
int bal_aqi;

// utility function for digital clock display: prints leading 0
String twoDigits(int digits){
  if(digits < 10) {
    String i = '0'+String(digits);
    return i;
  }
  else {
    return String(digits);
  }
}

void clockOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {

}

void analogClockFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
//  ui.disableIndicator();

  // Draw the clock face
//  display->drawCircle(clockCenterX + x, clockCenterY + y, clockRadius);
    display->drawCircle(clockCenterX + x, clockCenterY + y, 2);
  //
  //hour ticks
  for( int z=0; z < 360;z= z + 30 ){
  //Begin at 0° and stop at 360°
    float angle = z ;
    angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
    int x2 = ( clockCenterX + ( sin(angle) * clockRadius ) );
    int y2 = ( clockCenterY - ( cos(angle) * clockRadius ) );
    int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    display->drawLine( x2 + x , y2 + y , x3 + x , y3 + y);
  }

  // display second hand
  float angle = second() * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
  //
  // display minute hand
  angle = minute() * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
  //
  // display hour hand
  angle = hour() * 30 + int( ( minute() / 12 ) * 6 )   ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);

  //
  // display weather info
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24);
  display->drawString(clockCenterX + (clockRadius *2) + 10 + x , clockCenterY + y, baltemp );
  display->setFont(ArialMT_Plain_16);
  display->drawString(clockCenterX + (clockRadius *2) + 10 + x , clockCenterY - 24 + y, balhumi );

}

void digitalClockFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  String timenow = String(hour())+":"+twoDigits(minute())+":"+twoDigits(second());
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24);
  display->drawString(digitalClockCenterX + x , 16 + y, timenow );
  display->setFont(ArialMT_Plain_24);
  display->drawString(digitalClockCenterX + x , 40 + y, saqi );
}

// This array keeps function pointers to all frames
// frames are the single views that slide in
FrameCallback frames[] = { analogClockFrame, digitalClockFrame };

// how many frames are there?
int frameCount = 2;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = { clockOverlay };
int overlaysCount = 1;

bool timesync = false;

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
      if (debug) Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      if (mqttClient.connect("ESP8266Client")) {
          if (debug) Serial.println("connected");
          // Once connected, publish an announcement...
          mqttClient.publish(TP_STATUS, "ready");
          // ... and resubscribe
          mqttClient.subscribe(TP_HOME);
          mqttClient.subscribe(TP_UNIXTIME);
      } else {
          if (debug) {
              Serial.print("failed, rc=");
              Serial.print(mqttClient.state());
              Serial.println(" try again in 5 seconds");
          }
          // Wait 5 seconds before retrying
          delay(5000);
      }
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    if (debug) {
        Serial.print("Message arrived [");
        Serial.print(topic);
        Serial.print("] ");
        for (int i = 0; i < length; i++) {
          Serial.print((char)payload[i]);
        }
        Serial.println();
    }

    char p[length + 1];
    String m;
    String tp = String(topic);

    m.reserve(10);
    memcpy(p, payload, length);
    p[length] = '\0';
    m = String(p);
    if (tp.endsWith(TP_LIVTEMP)) {
        // Living room temperature
        m = String(round(m.toFloat()+0.5));
        m.concat("°C");
        livtemp = m;
    } else if (tp.endsWith(TP_LIVHUMI)) {
        // Living room temperature
        m = String(round(m.toFloat()+0.5));
        m.concat("%");
        livhumi = m;
    } else if (tp.endsWith(TP_LIVAQI)) {
        livaqi = m;
    } else if (tp.endsWith(TP_BALHUMI)) {
        // Balcony temperature
        m = String(round(m.toFloat()+0.5));
        m.concat("%rH");
        balhumi = m;
    } else if (tp.endsWith(TP_BALTEMP)) {
        // Baconly temperature
        m = String(round(m.toFloat()+0.5));
        m.concat("°C");
        baltemp = m;
    } else if (tp.endsWith(TP_BALAQI)) {
        balaqi = m;
        int i = m.toInt();
        if (i < 50) {
          saqi = "Excellent";
        } else if (i <= 100) {
          saqi = "Good";
        } else if (i <= 150) {
          saqi = "Light";
        } else if (i <= 200) {
          saqi = "Moderate";
        } else if (i <= 300) {
          saqi = "Heavy";
        } else if (i > 300) {
          saqi = "Severe";
        }
    } else if (tp.equals(TP_UNIXTIME)) {
        char cTime[10];

        String n = m.substring(0,10);
        uint32_t t = n.toInt();
        setTime(t+timeOffset);
    }
}

void WiFiEvent(WiFiEvent_t event)
{
    if (debug) Serial.printf("[WiFi-event] event: %d\n", event);

    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        if (debug) {
            Serial.println("WiFi connected");
            Serial.println("IP address: ");
            Serial.println(WiFi.localIP());
        }
        // Start ntp client
        timeClient.begin();
        timeClient.setTimeOffset(timeOffset);
        timeClient.update();
        delay(1000);
        if (debug) Serial.println(timeClient.getFormattedTime());
        // setTime(timeClient.getEpochTime());
        if (!mqttClient.connected()) {
          reconnect();
        }
        if (!mqttClient.connected()) {
            // turn off WiFi after time set and no MQTT connection
            WiFi.mode(WIFI_OFF);
        } else {
            saqi = "Connected";
        }
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        if (debug) Serial.println("WiFi lost connection");
        timesync = true;
        break;
    }
}


void setup() {
    if (debug) {
      Serial.begin(115200);
      Serial.println();
    }
    WiFi.disconnect(true);
    saqi = "No WiFi";
    delay(1000);
    WiFi.enableSTA(true);
    WiFi.enableAP(false);
    delay(2000);
    
    WiFi.onEvent(WiFiEvent);
    WiFi.begin(ssid, password);
    if (debug) {
      Serial.println();
      Serial.print("Wait for WiFi... ");
    }
    mqttClient.setServer(mqtt_server, 1883);
    mqttClient.setCallback(mqtt_callback);

  	// The ESP is capable of rendering 60fps in 80Mhz mode
  	// but that won't give you much time for anything else
  	// run it in 160Mhz mode or just set it to 30 fps
    ui.setTargetFPS(60);

  	// Customize the active and inactive symbol
    ui.setActiveSymbol(activeSymbol);
    ui.setInactiveSymbol(inactiveSymbol);

    // You can change this to
    // TOP, LEFT, BOTTOM, RIGHT
    ui.setIndicatorPosition(TOP);

    // Defines where the first frame is located in the bar.
    ui.setIndicatorDirection(LEFT_RIGHT);

    // You can change the transition that is used
    // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
    ui.setFrameAnimation(SLIDE_LEFT);

    // Add frames
    ui.setFrames(frames, frameCount);

    // Add overlays
    ui.setOverlays(overlays, overlaysCount);

    // Initialising the UI will init the display too.
    ui.init();

    display.flipScreenVertically();
}


void loop() {
  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.

    if ((minute() == 0) && (second() == 5)) {
        // Update time per hour
        if (timesync) {
            timesync = false;
            WiFi.begin(ssid, password);
        }
    }
    mqttClient.loop();
    delay(remainingTimeBudget);

  }


}
