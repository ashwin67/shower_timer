// Shower timer
#include "dht_nonblocking.h"
#include "pitches.h"

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
  
  TCCR1B |= (1 << WGM12) | (1 << CS10);  // Set up timer with and CTC mode and no prescaler
  TCNT1 = 0;                             // Initialize counter
  OCR1A = 27256;                         // This should be set to (16,000,000 / (prescaler * desired interrupt frequency)) - 1
  TIMSK1 |= (1 << OCIE1A);               // Disable compare interrupt
  sei();                                 // Enable global interrupts

  Serial.begin(115200);
}

int no_of_notes = 16;
int melody[] = {NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_G5, NOTE_A5, NOTE_B5, NOTE_C6, NOTE_C6, NOTE_B5, NOTE_A5, NOTE_G5, NOTE_F5, NOTE_E5, NOTE_D5, NOTE_C5};
int tick = 0;
int interval = 500;
int current_note = 0;
bool sound_enabled = false;

ISR(TIMER1_COMPA_vect) {
  if (sound_enabled)
  {
    tick++;
  }
  else if (!sound_enabled)
  {
    tick = 1;
    current_note = 0;
    digitalWrite(buzzer, LOW);
  }
  if (tick % interval == 0)
  {
    tone(buzzer, melody[current_note], interval);
    tick = 0;
    current_note++;
  }
  if (current_note == no_of_notes) {
    current_note = 0;
  }
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

float humidity_hyst_high = 60.0;
float humidity_hyst_low = 50.0;

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

  if (humidity > humidity_hyst_high) 
  {
    sound_enabled = true;
  }
  else if (humidity < humidity_hyst_low)
  {
    sound_enabled = false;
  }
}
