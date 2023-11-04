#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>

#include "config.h"

// consts from config.h >>
// const char* server_address
// const char* ssid
// const char* password

int wifi_connections_count = 0;

boolean float_low;
boolean float_high;
boolean is_temp_sensor_ok;
boolean is_pump_available;
boolean is_heater_available;
boolean pump_state;
boolean heater_state;
int low_temp;
int high_temp;
int temp;

String local_ip;
const String self_name = "ESP8266 Water Tank";

const size_t send_capacity = JSON_OBJECT_SIZE(20);
DynamicJsonDocument data_for_send_serial(send_capacity);
const size_t receive_capacity = JSON_OBJECT_SIZE(20);
DynamicJsonDocument data_for_reseive_serial(receive_capacity);

DynamicJsonDocument data_for_send_http(send_capacity);
DynamicJsonDocument data_for_reseive_http(receive_capacity);

ESP8266WebServer web_server(80);
HTTPClient httpc;
WiFiClient wifi_client;

void connect_wifi() {
  WiFi.begin(ssid, password);
  wifi_connections_count++;
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, !(digitalRead(LED_BUILTIN)));
    delay(1000);
  }

  for (int i = 0; i < 6; i++) {
    delay(150);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
    digitalWrite(LED_BUILTIN, HIGH);
  }
  local_ip = WiFi.localIP().toString();
  Serial.flush();
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  web_server.on("/", HTTP_GET, handle_http_get);
  web_server.on("/", HTTP_POST, handle_http_post);
  web_server.begin();
}

void loop() {

  if (WiFi.status() != WL_CONNECTED) connect_wifi();

  web_server.handleClient();

  if (Serial.available()) handle_serial_receive();
}

void handle_http_get() {
  digitalWrite(LED_BUILTIN, LOW);
  String json_string;
  data_for_send_http["name"] = self_name;
  data_for_send_http["ip"] = local_ip;
  data_for_send_http["wifi_connections_count"] = wifi_connections_count;
  data_for_send_http["float_low"] = float_low;
  data_for_send_http["float_high"] = float_high;
  data_for_send_http["low_temp"] = low_temp;
  data_for_send_http["high_temp"] = high_temp;
  data_for_send_http["temp"] = temp;
  data_for_send_http["is_temp_sensor_ok"] = is_temp_sensor_ok;
  data_for_send_http["is_pump_available"] = is_pump_available;
  data_for_send_http["is_heater_available"] = is_heater_available;
  data_for_send_http["pump_state"] = pump_state;
  data_for_send_http["heater_state"] = heater_state;
  serializeJson(data_for_send_http, json_string);

  web_server.send(200, "application/json", json_string);
  digitalWrite(LED_BUILTIN, HIGH);
}

void handle_http_post() {
  Serial.println(httpc.getString());
}

void handle_serial_receive() {
  String json_string;
  deserializeJson(data_for_reseive_serial, Serial);
  data_for_reseive_serial["name"] = self_name;
  data_for_reseive_serial["ip"] = local_ip;
  data_for_send_http["wifi_connections_count"] = wifi_connections_count;
  serializeJson(data_for_reseive_serial, json_string);

  httpc.begin(wifi_client, server_address);
  httpc.addHeader("Content-Type", "application/json");
  while (httpc.POST(json_string) != 200) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(50);
  }
  httpc.end();
  digitalWrite(LED_BUILTIN, HIGH);
}
