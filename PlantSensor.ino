// Adafruit IO Publish & Subscribe Example
//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// Written by Todd Treece for Adafruit Industries
// Copyright (c) 2016 Adafruit Industries
// Licensed under the MIT license.
//
// All text above must be included in any redistribution.

/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"
/************************** Temp/Humidity ***********************************/
#include "DHTesp.h"
DHTesp dht;
// zeroed by testing moisture sensor in and out of water
double maxmoisture = 850.0;
double minmoisture = 425.0;
/************************ Main Code Start Here *******************************/

// Track time of last published messages and limit feed->save events to once
// every IO_LOOP_DELAY milliseconds.
//
// Because this sketch is publishing AND subscribing, we can't use a long
// delay() function call in the main loop since that would prevent io.run()
// from being called often enough to receive all incoming messages.
//
// Instead, we can use the millis() function to get the current time in
// milliseconds and avoid publishing until IO_LOOP_DELAY milliseconds have
// passed.
#define IO_LOOP_DELAY 15000
unsigned long lastUpdate = 0;

// set up the Feeds
AdafruitIO_Feed *relay = io.feed("relay");
AdafruitIO_Feed *humidity = io.feed("humidity");
AdafruitIO_Feed *temp = io.feed("temp");
AdafruitIO_Feed *moisture = io.feed("moisture");

// Declare Needed Pins
const int relayPin = D1;

// this function is called whenever a 'relay' message
// is received from Adafruit IO
void relayMessage(AdafruitIO_Data *data) {
  String d = data->value();
  if(d == "OFF"){
    digitalWrite(relayPin, LOW);
  }
  if(d == "ON"){
    digitalWrite(relayPin, LOW);
  }
  else{
    digitalWrite(relayPin, HIGH);
    delay(d.toInt()*100);
    digitalWrite(relayPin, LOW);
  }
}

void setup() {
  // Setup Needed Pins
  pinMode(relayPin, OUTPUT);
  // Setup DHT
  dht.setup(D5, DHTesp::DHT11);
  // start the serial connection 
  Serial.begin(115200);
  
  // wait for serial monitor to open
  while(! Serial);

  Serial.print("Connecting to Adafruit IO");

  // connect to io.adafruit.com
  io.connect();

  // set up a message handler for the count feed.
  // the handleMessage function (defined below)
  // will be called whenever a message is
  // received from adafruit io.
  relay->onMessage(relayMessage);
  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  
  // force IO to update our MQTT subscription with the current values of each feed
  relay->get();
  humidity->get();
  temp->get();
  moisture->get();

}

void loop() {

  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop
  // function. it keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  io.run();
  if (millis() > (lastUpdate + IO_LOOP_DELAY)) {
    //Take measurements: humidity, temperature & moisture
    int h =  dht.getHumidity();
    int t = (dht.getTemperature()*1.8)+32;
    int m = analogRead(0);
    Serial.println(m);
    //skip for bogus values
    if(h >= 0|| h <= 100){
      humidity->save(h);
    }
    if(t > -50 || t < 150){
      temp->save(t);
    }
    //random math to try and normalize the moisture reading
    double m2 = (1-(m-minmoisture)/(maxmoisture - minmoisture))*100;
    moisture->save(m2);
    lastUpdate = millis();
  }

}
