#include <ArduinoJson.h>

#define float_low_pin 12   // low float
#define float_high_pin 10  // high float
#define pump_pin 7         // pump pin
#define heater_pin 5       // heater pin
#define thermo_pin 15      // thermo pin

#define error_pin 13  // error pin

// initialization thermo
#include <microDS18B20.h>
MicroDS18B20<thermo_pin> sensor;

// set some variables
int temp;
int low_temp = 37;
int high_temp = 40;

boolean is_pump_available = true;
boolean is_heater_available = true;
boolean is_temp_sensor_ok = false;

boolean is_force_fill_tank_ready = false;
boolean is_force_heat_water_ready = false;
boolean is_shower_ready = false;

boolean float_low;   // global float low state
boolean float_high;  // global float high state

unsigned long auto_fill_period = 60000ul * 60ul;         // 60 minutes
unsigned long auto_fill_timer = auto_fill_period * 2ul;  // for filling the tank on launch

unsigned long serial_interval = 1000;  // 1 sec
unsigned long serial_timer = 0;

const size_t send_capacity = JSON_OBJECT_SIZE(20);
DynamicJsonDocument data_for_send(send_capacity);
const size_t receive_capacity = JSON_OBJECT_SIZE(15);
DynamicJsonDocument data_for_reseive(receive_capacity);

void setup() {
  // initialization floats
  pinMode(float_low_pin, INPUT_PULLUP);
  pinMode(float_high_pin, INPUT_PULLUP);
  // initialization relay
  pinMode(pump_pin, OUTPUT);
  pinMode(heater_pin, OUTPUT);

  // init build-in LED
  pinMode(error_pin, OUTPUT);
  // relay force off
  digitalWrite(pump_pin, LOW);
  digitalWrite(heater_pin, LOW);

  Serial.begin(115200);

  digitalWrite(error_pin, HIGH);
  read_sensors();
  delay(1000);  // wait 1 sec
  digitalWrite(error_pin, LOW);
}

boolean is_time_to_fill() {
  // TRUE if timer worked OR low float is LOW
  return (((millis() - auto_fill_timer) >= auto_fill_period) || !float_low);
}

void fill_tank() {
  digitalWrite(heater_pin, LOW);  // turn heater off just in case
  while (!float_high && is_pump_available) {
    read_sensors();
    digitalWrite(pump_pin, HIGH);  // keep pump running
    send_data();
    receive_data();
  }
  digitalWrite(pump_pin, LOW);  // turn pump off
  auto_fill_timer = millis();   // update timer
}

void heat_water(int target_temp = high_temp) {
  while (temp < target_temp && is_heater_available && is_temp_sensor_ok) {
    digitalWrite(heater_pin, HIGH);  // turn heater on
    read_sensors();
    send_data();
    if (receive_data()) break;
    if (is_time_to_fill()) {
      digitalWrite(heater_pin, LOW);  // turn heater off if water level LOW
      return;                         // break heating ???
    }
  }
  digitalWrite(heater_pin, LOW);  // turn heater off
}

void shower(int target_temp = high_temp) {
  fill_tank();
  heat_water(target_temp);
  fill_tank();
  heat_water(target_temp);
}

void read_sensors() {
  float_low = digitalRead(float_low_pin);
  float_high = digitalRead(float_high_pin);

  sensor.requestTemp();

  if (sensor.readTemp()) {
    temp = sensor.getTemp();
    is_temp_sensor_ok = true;
    digitalWrite(error_pin, LOW);
  } else {
    is_temp_sensor_ok = false;
    digitalWrite(error_pin, HIGH);
  }
}

void send_data() {
  if (millis() - serial_timer >= serial_interval) {
    data_for_send["float_low"] = float_low;
    data_for_send["float_high"] = float_high;
    data_for_send["low_temp"] = low_temp;
    data_for_send["high_temp"] = high_temp;
    data_for_send["temp"] = temp;
    data_for_send["is_temp_sensor_ok"] = is_temp_sensor_ok;
    data_for_send["is_pump_available"] = is_pump_available;
    data_for_send["is_heater_available"] = is_heater_available;
    data_for_send["pump_state"] = digitalRead(pump_pin);
    data_for_send["heater_state"] = digitalRead(heater_pin);
    if (is_force_fill_tank_ready) {
      data_for_send["is_force_fill_tank_ready"] = is_force_fill_tank_ready;
      is_force_fill_tank_ready = false;
    }
    if (is_force_heat_water_ready) {
      data_for_send["is_force_heat_water_ready"] = is_force_heat_water_ready;
      is_force_heat_water_ready = false;
    }
    if (is_shower_ready) {
      data_for_send["is_shower_ready"] = is_shower_ready;
      is_shower_ready = false;
    }

    serializeJson(data_for_send, Serial);
    Serial.println();
    serial_timer = millis();
  }
}

boolean receive_data() {
  if (Serial.available()) {
    deserializeJson(data_for_reseive, Serial);
    low_temp = data_for_reseive.containsKey("low_temp") ? data_for_reseive["low_temp"] : low_temp;
    high_temp = data_for_reseive.containsKey("high_temp") ? data_for_reseive["high_temp"] : high_temp;
    is_pump_available = data_for_reseive.containsKey("is_pump_available") ? data_for_reseive["is_pump_available"] : is_pump_available;
    is_heater_available = data_for_reseive.containsKey("is_heater_available") ? data_for_reseive["is_heater_available"] : is_heater_available;

    if (data_for_reseive.containsKey("force_fill_tank")) {
      fill_tank();
      is_force_fill_tank_ready = true;
      return true;
    }
    if (data_for_reseive.containsKey("force_heat_water")) {
      heat_water(data_for_reseive["force_heat_water"].as<int>());
      is_force_heat_water_ready = true;
      return true;
    }
    if (data_for_reseive.containsKey("shower")) {
      shower(data_for_reseive["shower"].as<int>());
      is_shower_ready = true;
      return true;
    }
  }
  return false;
}

void loop() {
  // reading all sensors
  read_sensors();
  send_data();
  receive_data();

  // check water level
  if (is_time_to_fill()) {
    fill_tank();
  }

  // check temp
  if (temp < low_temp) {
    heat_water();
  }
}
