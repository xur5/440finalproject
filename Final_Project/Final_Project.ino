#define WIFI_SSID "University of Washington" // wifi network name
#define WIFI_PASS "" // wifi password

#define mqtt_server "mediatedspaces.net"  //this is its address, unique to the server
#define mqtt_user "hcdeiot"               //this is its server login, unique to the server
#define mqtt_password "esp8266"           //this is it server password, unique to the server

#define wifi_ssid "University of Washington"   // Wifi Stuff
#define wifi_password "" //


// pin used to control the servo
#define NP_PIN 2

// Required libraries for code to work
#include <Wire.h> // library allows you to communicate with I2C / TWI devices
#include <SPI.h> // Needed to communicate with MQTT
#include <PubSubClient.h>   // Needed to communicate with MQTT
#include <ArduinoJson.h>    // Needed to parse json files
#include <ESP8266WiFi.h>  // library provides ESP8266 specific WiFi methods we are calling to connect to network
#include <ESP8266HTTPClient.h>  // Needed to communicate with websites
#include <Servo.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

WiFiClient espClient;
PubSubClient mqtt(espClient);

Servo myservo;  // create servo object to control a servo
Adafruit_NeoPixel strip = Adafruit_NeoPixel(30, NP_PIN, NEO_GRBW + NEO_KHZ800);

char mac[6]; //A MAC address is a 'truly' unique ID for each device, lets use that as our 'truly' unique user ID!!!
char message[201]; //201, as last character in the array is the NULL character, denoting the end of the array
String mood;
int pos;

void setup() {
  myservo.attach(13);
  strip.begin();
  strip.setBrightness(100);
  strip.show(); // Initialize all pixels to 'off'

  Serial.begin(115200);
  setup_wifi();
  // System status
  while (! Serial);
  Serial.print("This board is running: ");
  Serial.println(F(__FILE__));
  Serial.print("Compiled: ");
  Serial.println(F(__DATE__ " " __TIME__));

  mqtt.setServer(mqtt_server, 1883); // Start the mqtt
  mqtt.setCallback(callback); //Register the callback function
  //pinMode(led, OUTPUT);        // for when joke is told
  //pinMode(LED_BUILTIN, OUTPUT); //for incoming message
  mood = "";
  count = 0;
  pos = 1;
  //  up = false;
  //  down = false;
  //  mid = true;
  colorWipe(strip.Color(236, 125, 243), 100); // pink
}

void loop() {
  if (!mqtt.connected()) {  // Try connecting again
    reconnect();
  }
  mqtt.loop(); //this keeps the mqtt connection 'active'
  delay(2500);

}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");  //get the unique MAC address to use as MQTT client ID, a 'truly' unique ID.
  Serial.println(WiFi.macAddress());  //.macAddress returns a byte array 6 bytes representing the MAC address
  WiFi.macAddress().toCharArray(mac, 4);  // creating unique identifier for mqtt
}

void reconnect() {
  // Loop until we're reconnected
  while (!espClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    // Attempt to connect
    if (mqtt.connect(mac, mqtt_user, mqtt_password)) { //the connction
      Serial.println("connected");
      mqtt.subscribe("Sentiment");
      mqtt.subscribe("TreeWeather");
      Serial.println("subscribing");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println();
  Serial.print("Message arrived [");
  Serial.print(topic); 
  Serial.println("] ");

  DynamicJsonBuffer  jsonBuffer; //blah blah blah a DJB
  JsonObject& root = jsonBuffer.parseObject(payload); //parse it!
  // if sentiment is received 
  if (String(topic) == "Sentiment") {
    Serial.println("printing message");
    Serial.print("Message arrived in topic: ");
    mood = root["mood"].as<String>();
    Serial.println("mood = " + mood);
    if (mood == "Negative") {
      if (pos == 0) {
        colorWipe(strip.Color(236, 125, 243), 100); // light pink
        myservo.write(130);
        delay(400);
      }
      pos = 0;
      myservo.write(50);
      colorWipe(strip.Color(0, 0, 204), 100); // Dark Blue


    } else if (mood == "Neutral") {
      if (pos == 1) {
        colorWipe(strip.Color(236, 125, 243), 100); // pink
        myservo.write(130);
        delay(200);
        myservo.write(50);
        delay(400);
      }
      pos = 1;
      myservo.write(90);
      colorWipe(strip.Color(0, 255, 0), 100); // Green
    } else {
      if (pos == 2) {
        colorWipe(strip.Color(236, 125, 243), 100); // pink
        myservo.write(50);
        delay(400);
      }
      pos = 2;
      myservo.write(130);
      rainbow(20);
    }

    Serial.println("it worked");
  }
  if (String(topic) == "TreeWeather") {
    Serial.println("printing message");
    Serial.print("Message arrived in topic: ");
    float humd = root["humd"];
    float temp = root["temp"];
    Serial.println("humd = " + String(humd));
    Serial.println("temp = " + String(temp));
    if (temp < 15 || humd < 20 || temp > 21 || humd > 25) {
      if (pos == 0) {
        colorWipe(strip.Color(236, 125, 243), 100); // pink
        myservo.write(130);
        delay(400);
      }
      pos = 0;
      myservo.write(50);
      colorWipe(strip.Color(0, 0, 204), 100); // Dark Blue
    }
  }
  if (!root.success()) { //well?
    Serial.println("parseObject() failed, are you sure this message is JSON formatted.");
    return;
  }
}

void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j = 0; j < 10; j++) { //do 10 cycles of chasing
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, c);  //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j = 0; j < 256; j++) {   // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}



