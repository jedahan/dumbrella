/*
 * Teared from the WiFlyHQ Example httpclient_progmem.ino
 *
 * This sketch is released to the public domain.
 *
 */
#define DEBUG
#ifdef DEBUG
  #define DEBUG_PRINT(x)  Serial.println (x)
#else
  #define DEBUG_PRINT(x)
#endif 


#include <WiFlyHQ.h>
#include <SoftwareSerial.h>
SoftwareSerial wifiSerial(8,9);

WiFly wifly;

const char mySSID[] = "roastbeefcurtains";
const char myPassword[] = "fuckshoes";
const char site[] = "api.goingtorain.com";
const int timeout = 60 * 1000;

char buf[32];

void terminal();
void connect_to_wifly();
void join_network();
void display_info();
void reconnect_to_website();
void read_one_character();

void setup()
{
    connect_to_wifly();
    join_network();
    display_info();
    reconnect_to_website();
}

void loop()
{
    save_the_time();
    read_characters_with_timeout();
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
        Serial.println(F("Joining network"));
        wifly.setSSID(mySSID);
        wifly.setPassphrase(myPassword);
        wifly.enableDHCP();

        if (wifly.join()) {
            Serial.println(F("Joined wifi network"));
        } else {
            Serial.println(F("Failed to join wifi network"));
            terminal();
        }
    } else {
        Serial.println(F("Already joined network"));
    }
}

void display_info()
{
    Serial.print(F("MAC: "));
    Serial.println(wifly.getMAC(buf, sizeof(buf)));
    Serial.print(F("IP: "));
    Serial.println(wifly.getIP(buf, sizeof(buf)));
    Serial.print(F("Netmask: "));
    Serial.println(wifly.getNetmask(buf, sizeof(buf)));
    Serial.print(F("Gateway: "));
    Serial.println(wifly.getGateway(buf, sizeof(buf)));
    Serial.print(F("SSID: "));
    Serial.println(wifly.getSSID(buf, sizeof(buf)));

    wifly.setDeviceID("Dumbrella");
    Serial.print(F("DeviceID: "));
    Serial.println(wifly.getDeviceID(buf, sizeof(buf)));
}

void reconnect_to_website()
{
    if (wifly.isConnected()) {
        Serial.println(F("Old connection active. Closing"));
        wifly.close();
    }


    if (wifly.open(site, 80)) {
        print_P(F("Connected to "));
        Serial.println(site);

        /* Send the request */
        wifly.println("GET /today.json?key=baad1e015e4f9f334e7ca9cb560ebb5d HTTP/1.0");
        wifly.println();
    } else {
        Serial.println(F("Failed to connect"));
    }
}

void connect_to_wifly()
{
    Serial.begin(115200);
    Serial.println(F("Starting"));
    Serial.println(F("Free memory: "))
    Serial.println(wifly.getFreeMemory(),DEC);

    wifiSerial.begin(9600);
    if (!wifly.begin(&wifiSerial, &Serial)) {
        Serial.println(F("Failed to start wifly"));
        terminal();
    }
}

void read_characters_with_timeout()
{
    uint32_t start = wifly.getUptime();
    int charnum = 0;

    while (wifly.getUptime() - start < timeout) {
        if (wifly.available() > 0) {
            char ch = wifly.read();
            
            DEBUG_PRINT(ch);

            if (ch == '\n') { Serial.write('\r'); }
           
            // the 12th character of the body will be y or n
            if (charnum++==12) {
                Serial.println(ch);
                if (ch == 'y') { digitalWrite(LED_PIN, HIGH); }
                else { digitalWrite(LED_PIN, LOW); }
            }

            if (Serial.available() > 0) {
               wifly.write(Serial.read());
            }
        }
    }
}