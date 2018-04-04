#include <ESP8266WiFi.h>

IPAddress local_IP(192, 168, 10, 1);
IPAddress gateway(192, 168, 10, 254);
IPAddress subnet(255, 255, 255, 0);

#ifndef AP_PASSWORD
#define AP_PASSWORD "12345678"
#endif//AP_PASSWORD

#ifndef AP_NAME
#define AP_NAME "esp8266-tcp-bridge"
#endif//AP_NAME

WiFiServer server(1000);

constexpr unsigned NUM_CLIENTS = 10;
WiFiClient clients[NUM_CLIENTS];

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  Serial.begin(230400);

  if (!WiFi.softAPConfig(local_IP, gateway, subnet))
    Serial.println("Error in AP config.");

  if (!WiFi.softAP(AP_NAME, AP_PASSWORD))
    Serial.println("Error in AP start.");

  // Bind to port
  server.begin();
}

inline void led_toggle() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void accept_clients() {
  WiFiClient incoming = server.available();
  if (!incoming)
    return;

  for (int i=0; i<NUM_CLIENTS; i++) {
    if (!clients[i].connected()) {
      clients[i] = incoming;
      return;
    }
  }

  // Close the connection if there was no client available
  incoming.stop();
}

void discard_received() {
  for (int i=0; i<NUM_CLIENTS; i++)
    if (clients[i].connected())
      clients[i].flush();
}

void loop() {
  accept_clients();
  discard_received();
  
  while (Serial.available()) {
    char recv = Serial.read();
    for (int i=0; i<NUM_CLIENTS; i++)
      if (clients[i].connected())
        clients[i].write(recv);
    led_toggle();
  }
}

