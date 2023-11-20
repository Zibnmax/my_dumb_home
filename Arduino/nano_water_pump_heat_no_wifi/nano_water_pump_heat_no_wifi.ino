
#define float_low_pin 12    // low float
#define float_high_pin 10   // high float
#define pump_pin 7          // pump pin
#define heater_pin 5        // heater pin
#define thermo_pin 15       // thermo pin

#define error_pin 13       // error pin

// initialization thermo
#include <microDS18B20.h>
MicroDS18B20<thermo_pin> sensor;

// set some variables
int temp;
int low_temp = 36;
int high_temp = 41;

boolean is_pump_available = true;
boolean is_heater_available = true;
boolean is_temp_sensor_ok = false;

boolean float_low;        // global float low state
boolean float_high;       // global float high state

unsigned long auto_fill_period = 60000ul * 60ul;     // 60 minutes
unsigned long auto_fill_timer = auto_fill_period * 2ul;   


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

  // Serial.begin(9600);
  read_sensors();
  delay(1000);
  digitalWrite(error_pin, HIGH);
  delay(1000);          // wait 1 sec
  digitalWrite(error_pin, LOW);
  // fill_tank();

}

void fill_tank () {
  if (!is_pump_available) {
    return;                                   // don't do anything
  }
  digitalWrite(heater_pin, LOW);              // turn heater off just in case
  while (!float_high) {
    read_sensors();
    digitalWrite(pump_pin, HIGH);             // keep pump running
  }
  digitalWrite(pump_pin, LOW);                // turn pump off
  auto_fill_timer = millis();                 // update timer
}

void heat_water (int target_temp=high_temp) {
  if (!is_heater_available || !is_temp_sensor_ok) {
    return;                                   // don't do anything
  }
  while (temp < target_temp) {
    read_sensors();
    if (is_time_to_fill()) {
      digitalWrite(heater_pin, LOW);          // turn heater off if water level LOW
      return;                                 // break heating ???
      }
    digitalWrite(heater_pin, HIGH);           // turn heater on
  }
  digitalWrite(heater_pin, LOW);              // turn heater off
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

boolean is_time_to_fill() {
  // TRUE if timer worked OR low float is LOW
  return (((millis() - auto_fill_timer) >= auto_fill_period) || !float_low);
}

void loop() {

  // reading all sensors
  read_sensors();

  // check water level
  if (is_time_to_fill()) {
    fill_tank();
  }

  // check temp
  if (temp < low_temp) {
    heat_water();
  }    
}


void serprint() {
  // serial printing
  // read_sensors();
  Serial.print("Low flow sensor: ");
  Serial.println(float_low);
  Serial.print("High flow sensor: ");
  Serial.println(float_high);
  Serial.print("Temp sensor: ");
  Serial.println(temp);
  Serial.print("is heater available: ");
  Serial.println(is_heater_available);
  Serial.print("is_time_to_fill: ");
  Serial.println(is_time_to_fill());
  Serial.println((millis() - auto_fill_timer) >= auto_fill_period);
  Serial.println(millis());
  Serial.println(auto_fill_timer);
  Serial.println(auto_fill_period);
  Serial.println();
  digitalWrite(error_pin, HIGH);
  delay(1000);
}
