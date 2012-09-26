/*
 * Teared from the WiFlyHQ Example httpclient_progmem.ino
 *
 * This sketch is released to the public domain.
 *
 */
#define DEBUG
#ifdef DEBUG
  #define DEBUG_PRINT(x)  Serial.print (x)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif 

#define LED_PIN 13

#include <avr/sleep.h>
#include <WiFlyHQ.h>

#include <SoftwareSerial.h>
SoftwareSerial wifiSerial(8,9);

WiFly wifly;

const char device_id[] = "dumbrella";

const char mySSID[] = "dd-wrt";
const char myPassword[] = "";

//const char site[] = "api.goingtorain.com";
const char site[] = "talon";
const int port = 3000;
const int buffer_read_timeout = 15 * 1000;
boolean body = false;
int newline = 0;

char buf[32];

// wake every 29 minutes
const int wake_time = 29 * 60;
// sleep 1 minute after tcp connection closes
const int sleep_time = 60 * 60;

void terminal();
void connect_to_wifly();
void join_network();
void display_info();
void reconnect_to_website();
void read_one_character();

void setup()
{
    // call wakeup() when digital pin 2 goes low
    attachInterrupt(0, wakeup, LOW);
    connect_to_wifly();
}

void loop()
{
    join_network();
    display_info();
    reconnect_to_website();
    check_if_its_raining();
    go_to_sleep();
}

void check_if_its_raining()
{
    boolean rain = false;
    // only read up to the 13th character, which should be a 'y' or 'n'
    /* 
    wifly.gets(buf,13,buffer_read_timeout);
    if (buf[12] == 'y') { rain = true; }
    */
    wifly.gets(buf,2,buffer_read_timeout);
    if (buf[1] == '1') { rain = true; }

    if (rain) { digitalWrite(LED_PIN, HIGH); }
    else { digitalWrite(LED_PIN, LOW); }
}

void go_to_sleep() {
    /* sleep wifly */
    // set sys wake <secs>
    // set sys sleep <secs>

    /* sleep arduino */
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    attachInterrupt(0,wakeup, LOW);
    sleep_mode();

    /* THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP */
    sleep_disable();
    detachInterrupt(0);
}

void wakeup()
{
    // we can do cleanup stuff here if we feel like it, but DONT SET TIMERS
}

/* Connect the WiFly serial to the serial monitor. */
void terminal()
{
    while (1) {
        if (wifly.available() > 0) {
            Serial.write(wifly.read());
        }

        if (Serial.available() > 0) {
            wifly.write(Serial.read());
        }
    }
}

/* Join wifi network if not already associated */
void join_network()
{
    if (!wifly.isAssociated()) {
        /* Setup the WiFly to connect to a wifi network */
        DEBUG_PRINTLN(F("Joining network"));
        wifly.setSSID(mySSID);
        if (myPassword) { wifly.setPassphrase(myPassword); }
        wifly.enableDHCP();

        if (wifly.join()) {
            DEBUG_PRINTLN(F("Joined wifi network"));
        } else {
            DEBUG_PRINTLN(F("Failed to join wifi network"));
            terminal();
        }
    } else {
        DEBUG_PRINTLN(F("Already joined network"));
    }
}

void display_info()
{
    DEBUG_PRINT(F("MAC: "));
    DEBUG_PRINTLN(wifly.getMAC(buf, sizeof(buf)));
    DEBUG_PRINT(F("IP: "));
    DEBUG_PRINTLN(wifly.getIP(buf, sizeof(buf)));
    DEBUG_PRINT(F("Netmask: "));
    DEBUG_PRINTLN(wifly.getNetmask(buf, sizeof(buf)));
    DEBUG_PRINT(F("Gateway: "));
    DEBUG_PRINTLN(wifly.getGateway(buf, sizeof(buf)));
    DEBUG_PRINT(F("SSID: "));
    DEBUG_PRINTLN(wifly.getSSID(buf, sizeof(buf)));

    wifly.setDeviceID(device_id);
    DEBUG_PRINT(F("DeviceID: "));
    DEBUG_PRINTLN(wifly.getDeviceID(buf, sizeof(buf)));
}

void reconnect_to_website()
{
    if (wifly.isConnected()) {
        DEBUG_PRINTLN(F("Old connection active. Closing"));
        wifly.close();
    }


    if (wifly.open(site, 80)) {
        DEBUG_PRINT(F("Connected to "));
        DEBUG_PRINTLN(site);

        /* Send the request */
        //wifly.println(F("GET /today.json?key=baad1e015e4f9f334e7ca9cb560ebb5d HTTP/1.0"));
        wifly.println(F("GET / HTTP/1.1"));
        wifly.println();
    } else {
        DEBUG_PRINTLN(F("Failed to connect"));
    }
}

void connect_to_wifly()
{
    Serial.begin(115200);
    DEBUG_PRINTLN(F("Starting"));
    DEBUG_PRINTLN(F("Free memory: "));
    DEBUG_PRINTLN(wifly.getFreeMemory(),DEC);

    wifiSerial.begin(9600);
    if (!wifly.begin(&wifiSerial,&Serial)) {
        DEBUG_PRINTLN(F("Failed to start wifly"));
        terminal();
    }
}
