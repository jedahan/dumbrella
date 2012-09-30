/* teared from the wiflyhq httpclient_progmem example
 * this is all public domain, do what you want
 * it would be nice to credit jonathan@jedahan.com
 */
#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(x)  Serial.print(x)
#define DEBUG_PRINTLN(x)  Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif 

#include <LowPower.h>

#include <WiFlyHQ.h>

#include <SoftwareSerial.h>
SoftwareSerial wifiSerial(8,9);

WiFly wifly;

const char device_id[] = "dumbrella";

const char mySSID[] = "dd-wrt";
const char myPassword[] = "";

const char site[] = "talon";
const int port = 3000;

const int buffer_read_timeout = 15 * 1000;
char buf[256];

const int ledPin = 13;

const int wakePin = 2;
// wake every 29 minutes
const int wake_time = 60 * 29;
// sleep 1 minute after tcp connection closes
const int sleep_time = 60 * 1;

void terminal();
void connect_to_wifly();
void join_network();
void display_info();
void reconnect_to_website();
void read_one_character();
void set_alternate_pin_functions();

void setup()
{
  pinMode(wakePin, INPUT);
  pinMode(ledPin, OUTPUT);
  connect_to_wifly();
  set_alternate_pin_functions();
}

void loop()
{
  join_network();
//  display_info();
  reconnect_to_website();
  check_if_its_raining();
  delay(30*1000);
//  go_to_sleep();
}

void check_if_its_raining()
{
  for(int i=1; i<9; i++) {
    wifly.gets(buf,3,buffer_read_timeout);
    DEBUG_PRINT(i);
    DEBUG_PRINT(":");
    DEBUG_PRINTLN(buf);
  }
  Serial.println(buf);
  digitalWrite(ledPin, (buf[0]=='1'));
}

void go_to_sleep() {
  DEBUG_PRINTLN("wifly: zzzZZZ");
  wifly.print(F("set sys wake "));
  wifly.println(wake_time);
  wifly.println(F("set sys sleep "));
  wifly.println(sleep_time);

  // NOTE: can only use LOW with SLEEP_MODE_PWR_DOWN
  // TODO: pullup + transistor to invert logic
  DEBUG_PRINTLN(F("arduino: zzzZZZ"));
  //attachInterrupt(0,wakeup,LOW);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  //detachInterrupt(0);
}

// do as little as possible here
void wakeup()
{
  DEBUG_PRINTLN(F("what a wonderful nap!"));
}

// Connect the WiFly serial to the serial monitor
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
    DEBUG_PRINT(F("Joining network "));
    DEBUG_PRINTLN(mySSID);
    wifly.setSSID(mySSID);

    if (myPassword!="") {
      wifly.setPassphrase(myPassword);
    }
    wifly.enableDHCP();

    if (wifly.join()) {
      DEBUG_PRINTLN(F("Joined wifi network"));
    }
    else {
      DEBUG_PRINTLN(F("Failed to join wifi network"));
      terminal();
    }
  }
  else {
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

  if (wifly.open(site, port)) {
    DEBUG_PRINT(F("Connected to "));
    DEBUG_PRINTLN(site);

    wifly.println(F("GET /rain HTTP/1.1\r\nUser-Agent: dumbrella"));
    wifly.println();
  } else {
    DEBUG_PRINTLN(F("Failed to connect"));
  }
}

void connect_to_wifly()
{
  Serial.begin(115200);
  DEBUG_PRINTLN(F("Starting"));
  DEBUG_PRINT(F("Free memory: "));
  Serial.println(wifly.getFreeMemory(),DEC);

  wifiSerial.begin(9600);
  if (!wifly.begin(&wifiSerial,&Serial)) {
    DEBUG_PRINTLN(F("Failed to start wifly"));
    terminal();
  }
}

void set_alternate_pin_functions() {
  wifly.println("set sys iofunc 0x70");
  wifly.println("set sys printlvl 0");
  wifly.save();
  wifly.reboot();
}