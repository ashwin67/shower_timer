// Shower timer
#include "dht_nonblocking.h"

int motion_sensor_input_pin = 7;
int motion_sensor_state = LOW;
int motion_sensor_val = 0;

#define DHT_SENSOR_TYPE DHT_TYPE_11
static const int DHT_SENSOR_PIN = 8;
DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );
 
int buzzer = 13;

void setup() {
  pinMode(motion_sensor_input_pin, INPUT);
  pinMode(buzzer,OUTPUT);
  Serial.begin(9600);
}

static bool measure_environment( float *temperature, float *humidity )
{
  static unsigned long measurement_timestamp = millis( );

  /* Measure once every four seconds. */
  if( millis( ) - measurement_timestamp > 3000ul )
  {
    if( dht_sensor.measure( temperature, humidity ) == true )
    {
      measurement_timestamp = millis( );
      return( true );
    }
  }
  return( false );
}

unsigned long previousMillis = 0; 
unsigned long interval = 1; 
int i = 0;
bool isHigh = false;

// TODO: This doesn't work yet as the beep_buzzer is not getting called regularly due to the delay in measure_environment
static void beep_buzzer()
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    i++;
    if (i < 500) {
      Serial.println("Speaker on");
      Serial.println(currentMillis);
      if (isHigh) {
        digitalWrite(buzzer, LOW);
        isHigh = false;
      } else {
        digitalWrite(buzzer, HIGH);
        isHigh = true;
      }
    }
    else if (i < 1000) {
      Serial.println("Speaker off");
      Serial.println(currentMillis);
      digitalWrite(buzzer, LOW);
      isHigh = false;
    }
    else {
      i = 0;
    }
  }
}

void loop(){
  motion_sensor_val = digitalRead(motion_sensor_input_pin);
  float temperature;
  float humidity;

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

  if( measure_environment( &temperature, &humidity ) == true )
  {
    Serial.print( "T = " );
    Serial.print( temperature, 1 );
    Serial.print( " deg. C, H = " );
    Serial.print( humidity, 1 );
    Serial.println( "%" );
  }

  beep_buzzer();
}
