// Shower timer
#include "dht_nonblocking.h"
#include "pitches.h"

int  soundSensorAnalogPin = A0;    // Select the Arduino input pin to accept the Sound Sensor's analog output 
int  soundSensorDigitalPin = 9;    // Select the Arduino input pin to accept the Sound Sensor's digital output
int  soundAnalogValue = 0;         // Define variable to store the analog value coming from the Sound Sensor
int  soundDigitalValue;            // Define variable to store the digital value coming from the Sound Sensor
int  soundLed = 13;                // Define LED port; this is the LED built in to the Arduino (labled L)

#define DHT_SENSOR_TYPE DHT_TYPE_11
static const int DHT_SENSOR_PIN = 8;
DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );
 
int buzzer = 12;

void setup() {
  pinMode(buzzer,OUTPUT);

  pinMode(soundSensorDigitalPin,INPUT);
  pinMode(soundLed,OUTPUT);

  TCCR0B |= (1 << WGM02) | (1 << CS01) | (1 << CS00);  // Set up timer with and CTC mode and CLK/64 prescaler
  TCNT0 = 0;                                           // Initialize counter
  OCR0A = 24999;                                       // This should be set to (16,000,000 / (prescaler * desired interrupt frequency)) - 1 -> 10 Hz
  TIMSK0 |= (1 << OCIE0A);                             // Disable compare interrupt

  TCCR1B |= (1 << WGM12) | (1 << CS10);  // Set up timer with and CTC mode and no prescaler
  TCNT1 = 0;                             // Initialize counter
  OCR1A = 27256;                         // This should be set to (16,000,000 / (prescaler * desired interrupt frequency)) - 1 -> 587 Hz
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

ISR(TIMER0_COMPA_vect) {
  soundAnalogValue = analogRead(soundSensorAnalogPin);
  soundDigitalValue=digitalRead(soundSensorDigitalPin);
  Serial.println(soundAnalogValue);

  if(soundDigitalValue==HIGH) 
  {
    digitalWrite(soundLed,HIGH);
  }
  else
  {
    digitalWrite(soundLed,LOW);
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
  float temperature;
  float humidity;

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
