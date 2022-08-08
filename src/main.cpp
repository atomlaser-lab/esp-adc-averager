#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

const int LED = 2;
/*
 * WiFi network credentials
 */
const char* ssid = "";
const char* password = "";
/*
 * Define some global variables for handling connections
 */
WiFiServer wifi_server(6666);
WiFiClient client;
String recv_string = "";
/*
 * Filtering settings
 */
unsigned long filter_delay = 10;  //10 ms
float filter_weight = 2*3.14*((float)filter_delay)/1000; //1 Hz filter
int avalue_int = 0;
float avalue_volts = 0;
float avalue_avg = 0;
unsigned long current_millis = 0;
unsigned long previous_millis = 0;
/*
 * Setup phase
 */
void setup() {
  pinMode(LED,OUTPUT);
  digitalWrite(LED,0);
  //Start the serial connection
  Serial.setTimeout(20);
  Serial.begin(115200);
  Serial.println("Booting...");
  //Connect to the WiFi
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("esp-power-monitor");
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());

  if (!MDNS.begin("esp-power-monitor")) {
    Serial.println("Error setting up MDNS responder!");
  } else {
    Serial.println("mDNS started");
  }
  /*
   * This stuff allows for "over-the-air" updates via WiFi.
   */
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  //Start the web server
  wifi_server.begin();
  //Get the current time
  current_millis = millis();
}

void loop() {
  //This is needed for OTA updates to work
  ArduinoOTA.handle();
  /*
   * This section reads data from the ADC and averages it without introducing a delay
   */
  current_millis = millis();
  if (current_millis - previous_millis > filter_delay) {
    previous_millis = current_millis;
    avalue_int = analogRead(A0);
    avalue_volts = ((float) avalue_int)/1024*6;
    avalue_avg = avalue_avg/(1 + filter_weight) + filter_weight/(1 + filter_weight)*avalue_volts;
  } else if (current_millis < previous_millis) {
    previous_millis = 0;
  }
  
  /*
   * This section connects to a client and returns averaged data
   */
  client = wifi_server.available();
  if (client) {
    //Only executes when a client is connected
    // Serial.println("Client connected");
    while (client) {
      //Exits this loop when the client disconnects
      digitalWrite(LED,1);
      while (client.available() > 0) {
        //Grab data from client
        char c = client.read();
        if (c == '\n') {
          if (recv_string == "get") {
            client.print(String(avalue_avg,6) + "\r\n");
            recv_string = "";
          } else {
            recv_string = "";
          }
        } else {
          recv_string += c;
        }
      }
      //Delay between reads (10 ms)
      delay(10);
    }
    client.stop();
    digitalWrite(LED,0);
    // Serial.println("Client disconnected");
  } else {
    recv_string = "";
    digitalWrite(LED,0);
  }
  
}