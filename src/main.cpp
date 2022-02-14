#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
// #include <ESP8266mDNS.h>
#include <WiFiUdp.h>
/*
 * Define some global variables for handling connections
 */
WiFiServer wifi_server(6666);
WiFiClient client;

// const char* ssid = "Atomlaser";
// const char* password = "becbecbec";

const char* ssid = "AtomopticsLAB-2G";
const char* password = "BeCbecBeC";

const int LED = 2;


void setup() {
  pinMode(LED,OUTPUT);
  //Start the serial connection
  Serial.setTimeout(20);
  Serial.begin(115200);
  Serial.println("Booting...");
  //Connect to the WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());

  // if (!MDNS.begin("esp8266test")) {
  //   Serial.println("Error setting up MDNS responder!");
  // }
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

}

void loop() {
  //This is needed for OTA updates to work
  ArduinoOTA.handle();
  //Check if a client has connected
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
        Serial.write(c);
        String resp = Serial.readString();
        client.print(resp);
        digitalWrite(LED,0);
      }
      //Delay between reads (10 ms)
      delay(10);
    }
    client.stop();
    digitalWrite(LED,0);
    // Serial.println("Client disconnected");
  } else {
    digitalWrite(LED,1);
  }
}