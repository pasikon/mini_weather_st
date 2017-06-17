#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Timers.h>

Timer sendTsTimer;

Adafruit_BMP280 bmp; // I2C

const char* host = "api.thingspeak.com";
const char* writeAPIKey = "1RUB0PWV56MZ3XRD";
const char* tsChannel = "270657";

ESP8266WebServer server(80);

void setup() {
  sendTsTimer.begin(MINS(15));
  
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //I2C stuff
  Wire.pins(0, 2);
  Wire.begin(0, 2);

  bmp.begin();

  if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");


  Serial.println("local ip");
  Serial.println(WiFi.localIP());

  server.on("/temp", []() {
    server.send(200, "text/html", String(bmp.readTemperature()));
  });

  server.on("/", []() {
    String webStr = (String("<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\r\n") +
    "<head><title>ESP8266</title></head>\r\n" +
    "<body style=\"margin-left:20px; margin-top:10px;\">\r\n" +
    "<div>\r\n" +
    "<br /> <br />\r\n" +
    "temperature: "+ bmp.readTemperature() +" C\r\n" +
    "<br /> <br />\r\n" +
    "pressure: "+ bmp.readPressure() +" P\r\n" +
    "</div>\r\n</body>\r\n</html>");
    
    server.send(200, "text/html", webStr);
  });


  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  if (sendTsTimer.available())
  {
    sendDataTS();
    sendTsTimer.restart();
  }

}

void sendDataTS() {
  Serial.println("Sending data TS");

  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    return;
  }

  String url = "/update?key=";
  url += writeAPIKey;
  url += "&field1=";
  url += String(bmp.readTemperature());
  url += "&field2=";
  url += String(bmp.readPressure());
  url += "\r\n";

  // Send request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
}

