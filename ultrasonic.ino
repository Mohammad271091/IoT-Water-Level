#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include "webPage.h";

const int MIN = 5;  // between the sensor and the water when the tank is full => min 2 cm for this sensor
int MAX = 84; // between the sensor and the bottom of the tank => maximum 400 cm for this sensor
int percentage = 0;
const int trigPin = 16;
const int echoPin = 5;
long duration;
int distance;
String JSONtxt;

// variables to make a 1 second delay between readings without using delay()
const long interval = 1000;
unsigned long previousMillis = 0;

// WiFi Credentials
const char *ssid = "SSID";
const char *password = "password";


// Create an instance for web server and websocket server
ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);


void setup()
{
  Serial.begin(115200);
  // pinMode(LED, OUTPUT);

  //-----------------------------------------------
  // Connect to the WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  //-----------------------------------------------
  server.on("/", rootEndPointResponse);
  server.begin();
  webSocket.begin();
  // webSocket.onEvent(webSocketEvent);
  //-----------------------------------------------

  // Ultrasonic pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop()
{
    if(Serial.available()>0) MAX = Serial.readString().toInt();

  //-----------------------------------------------
  webSocket.loop(); 
  server.handleClient();
  //-----------------------------------------------
  unsigned long currentMillis = millis();
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // every one second, do the reading and show it to serial monitor+
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    read_sensor();
  }


  //Broadcast the sensor reading via websockets
  JSONtxt = "{\"percentage\":\""+String(percentage)+"\"}"; 
  webSocket.broadcastTXT(JSONtxt);

}

void read_sensor()
{
  // Calculating the distance
  distance = duration * 0.034 / 2;

  // limits of reading in case they exceed
  if (distance > MAX)
    distance = MAX;
  if (distance < MIN)
    distance = MIN;

  percentage = map(distance, MIN, MAX, 100, 0);

  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);
  Serial.print(percentage);
  Serial.println("%");
  Serial.println(MAX);
}

void rootEndPointResponse()
{
  server.send(200,"text/html", webpageCode);
}

// void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t welength)
// {
//   String payloadString = (const char *)payload;
//   Serial.print("payloadString= ");
//   Serial.println(payloadString);

//   if(type == WStype_TEXT) //receive text from client
//   {
//     byte separator=payloadString.indexOf('=');
//     String var = payloadString.substring(0,separator);
//     Serial.print("var= ");
//     Serial.println(var);
//     String val = payloadString.substring(separator+1);
//     Serial.print("val= ");
//     Serial.println(val);
//     Serial.println(" ");

//     if(var == "LEDonoff")
//     {
//       LEDonoff = false;
//       if(val == "ON") LEDonoff = true;
//     }
//   }
// }