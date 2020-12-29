#include <Arduino.h>
#include <ESP8266WiFi.h>

// is not using ESP RAM! -- best practice
#define OUTPUT_CHANNEL 4
#define PWM_MAX_VAL 1024

// wifi setting's
#define WIFI_NAME "Virus.exe"
#define WIFI_PASSWORD "24zov83xgrzwg7"

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Variable to store LEDs state
bool isOn = false;

// Variable to store brightness
float brightness = 1.0;

// put your setup code here, to run once:
void setup() {
  Serial.begin(115200);

  // Set the pinmode of the pins to which the LEDs are connected
  pinMode(OUTPUT_CHANNEL, OUTPUT);
  analogWriteFreq(1000);

  // Setting up WiFi
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
  Serial.print("Connection to ");
  Serial.println(WIFI_NAME);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // print IP adress and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());   // this will display the Ip address of the Pi which should be entered into your browser
  server.begin();
}

// put your main code here, to run repeatedly:
void loop() {
  WiFiClient client = server.available();     // Listen for incoming clients

  if (client) {                               // If a new client connect's
    String currentLine = "";                  // make a String to hold incoming data from the client

    while(client.connected()) {               // loop while the client's connected
      if (client.available()) {               // if there are bytes to read
        char c = client.read();               // read a byte then
        Serial.write(c);                      // print it out the serial monitor
        header += c;

        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:application/json");
            client.println("Connection: close");
            client.println();

            // get content of header
            if (header.indexOf("GET /toggle") >= 0) {                 // toggle lights
              isOn = !isOn;

              Serial.print("Toggeling light to: ");

              analogWrite(OUTPUT_CHANNEL, ((int)(brightness * PWM_MAX_VAL) % (PWM_MAX_VAL + 1)) * isOn);

              if (isOn) {
                Serial.println("on");
                client.println("{ \"state\": \"on\" }");
              } else {
                Serial.println("off");
                client.println("{ \"state\": \"off\" }");
              }

              client.stop();
            } else if (header.indexOf("GET /brightness/") >= 0) {      // Adjust brightness
              String sBrightness = header.substring(header.indexOf("GET /brightness"));
              sBrightness = sBrightness.substring(0, sBrightness.indexOf("HTTP") - 1);
              sBrightness = sBrightness.substring(sBrightness.lastIndexOf('/') + 1);

              brightness = atof(sBrightness.c_str());

              Serial.print("Setting brightness to: ");
              Serial.println(brightness);

              client.println(brightness);

              if (isOn) {
                analogWrite(OUTPUT_CHANNEL, (int)(brightness * PWM_MAX_VAL) % (PWM_MAX_VAL + 1));
              }

              client.stop();
            } else {
              Serial.println("unrecognized header");
              client.println("unrecognized header");

              Serial.println(header);

              client.stop();
            }
          } else {                // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }

    // Clear the header variable
    header = "";

    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
