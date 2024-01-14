// Shower timer

int motion_sensor_input_pin = 7;
int motion_sensor_state = LOW;
int motion_sensor_val = 0;
 
void setup() {
  pinMode(motion_sensor_input_pin, INPUT);
  Serial.begin(9600);
}
 
void loop(){
  motion_sensor_val = digitalRead(motion_sensor_input_pin);
  if (motion_sensor_val == HIGH) {
    if (motion_sensor_state == LOW) {
      Serial.println("Motion detected!");
      motion_sensor_state = HIGH;
    }
  } else {
    if (motion_sensor_state == HIGH){
      Serial.println("Motion ended!");
      motion_sensor_state = LOW;
    }
  }
}
