/*
 * vim: syntax=cpp ts=4 sw=4 sts=4 sr et
 * Use :SyntasticToggleMode
 * https://wiki.seeedstudio.com/Wio_Link
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
//#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

//#include <WiFi.h>

//#define SECRET_SSID ""
//#define SECRET_PASS ""

#ifndef SECRET_SSID
#include "arduino_secrets.h"
#endif

//#define RELAY_PIN 14
#define LED_IN 4
// e.g. Red LED
//#define RED_SWITCH 5 // PD5

//#define RELAY_IN SDA
//#define relayOut SCL // PB0

//  Pin 16 can be INPUT, OUTPUT or INPUT_PULLDOWN_16. At startup, pins are configured as INPUT.
const uint8_t qSwitchOut = 13; //SCL;
const uint8_t motorOut = 14; //SCL;
const uint8_t flashIn = 12;

const char *ssid = SECRET_SSID;
const char *password = SECRET_PASS;

bool ledState = false;
bool wifiOK = false;

unsigned long qSwitchOn = 0;
unsigned long qSwitchOff = 0;
unsigned long ledOff = 0;
unsigned long nextPrint = 0;

struct Button {
    const uint8_t PIN;
    uint32_t numberKeyPresses;
    bool pressed;
};

Button button1 = {flashIn, 0, false};

void IRAM_ATTR isr() {
    button1.numberKeyPresses++;
    if(button1.pressed != true){
        qSwitchOn = micros() + 200;
        nextPrint = millis() + 100; // Don't pospone Q-Switch
    }
    button1.pressed = true;
}


#define WIFI_RETRY 20

bool reconnectWifi() {
    Serial.print("Re-connecting to ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    for(int i = 0; i < WIFI_RETRY; i++)
        if (WiFi.status() != WL_CONNECTED) {
            //while (WiFi.status() != WL_CONNECTED) {

            Serial.print(".");
            ledState = false;
            digitalWrite(LED_BUILTIN, ledState);
            delay(100);
            ledState = true;
            digitalWrite(LED_BUILTIN, ledState);
            delay(500);
            //continue;
        }
        else {
            Serial.print("Connected to ");
            Serial.println(ssid);
            Serial.print("IP address: ");
            Serial.print(WiFi.localIP());
            if (MDNS.begin("wio-link")) 
                Serial.println(" MDNS responder started");
            
            Serial.println("");
            return true;
        }
    return false;
}
void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
//    pinMode(RELAY_PIN, OUTPUT);
    pinMode(qSwitchOut, OUTPUT);
    pinMode(motorOut, OUTPUT);
    digitalWrite(motorOut, LOW);
    pinMode(flashIn,  INPUT_PULLUP);

    //pinMode(PIN_GROVE_POWER, OUTPUT);
    //digitalWrite(PIN_GROVE_POWER, 1);
    // ou must pull up GPIO 15 in your Arduino sketch to power on the Grove system:
    pinMode(PIN_GROVE_POWER,  INPUT_PULLUP);

    Serial.begin(115200);
    delay(500);
    Serial.println("Ola");
    wifiOK = reconnectWifi();
    attachInterrupt(button1.PIN, isr, RISING);

}

void loop() {
    unsigned long nowMs =  millis();
    unsigned long nowUs =  micros();
    if ( nowUs > qSwitchOn  ) {
        digitalWrite(qSwitchOut, HIGH);
        digitalWrite(motorOut, HIGH);
        digitalWrite(LED_BUILTIN, HIGH);
        qSwitchOn =  4294967295U;
        qSwitchOff = nowMs + 1000;
    }
    if ( nowMs > qSwitchOff  ) {
        digitalWrite(qSwitchOut, LOW);
        digitalWrite(motorOut, LOW);
        digitalWrite(LED_BUILTIN, LOW);
        qSwitchOff = 4294967295U;
    }

    if ( nowMs > nextPrint  ) {
        nextPrint = nowMs + 1000;
        //digitalWrite(RELAY_PIN, ledState);
        ledState = not ledState;

        //digitalWrite(qSwitchOut, ledState);
        if (button1.pressed) {
            Serial.printf("\nButton has been pressed %u times\n", button1.numberKeyPresses);
            button1.pressed = false;
        }

        Serial.print(" ");
        Serial.print(ledState);
    }
}
