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

  TCCR1A = 0;
  TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);    // Set up timer with and CTC mode and /64 prescaler
  TCNT1 = 0;                                            // Initialize counter
  OCR1A = 2499;                                         // This should be set to (16,000,000 / (prescaler * desired interrupt frequency)) - 1 -> 100 Hz
  TIMSK1 |= (1 << OCIE1A);                              // Disable compare interrupt

  sei();                                                // Enable global interrupts

  Serial.begin(9600);
}

bool sound_enabled = false;
bool shower_timeout_enabled = false;

int humidity_measurement_timer = 0;
bool humidity_measurement_timer_ding = false;
int sound_measurement_timer = 0;
bool sound_measurement_timer_ding = false;
int tone_timer = 0;
bool tone_timer_ding = false;
int shower_timer = 0;
bool shower_timer_ding = false;

// Following timeout are in 10ms units
const int HUMIDITY_MEASUREMENT_TIMEOUT = 400;
const int SOUND_MEASUREMENT_TIMEOUT = 1;
const int TONE_TIMEOUT = 50;
const int SHOWER_TIMEOUT = 10*100; //10*60*100;
const int LAST_SOUND_THRESHOLD_IN_TICKS = 10*100;

unsigned long current_tick = 0;

ISR(TIMER1_COMPA_vect) 
{
  current_tick++;
  
  humidity_measurement_timer++;
  if (humidity_measurement_timer >= HUMIDITY_MEASUREMENT_TIMEOUT)
  {
    humidity_measurement_timer = 0;
    humidity_measurement_timer_ding = true;
  }

  sound_measurement_timer++;
  if (sound_measurement_timer >= SOUND_MEASUREMENT_TIMEOUT)
  {
    sound_measurement_timer = 0;
    sound_measurement_timer_ding = true;
  }

  if (shower_timeout_enabled)
  {
    shower_timer++;
  }
  else
  {
    shower_timer = 0;
    shower_timer_ding = false;
  }
  if (shower_timer >= SHOWER_TIMEOUT)
  {
    shower_timer_ding = true;
    sound_enabled = true;
  }
  else
  {
    sound_enabled = false;
  }

  if (sound_enabled)
  {
    tone_timer++;
  }
  else
  {
    tone_timer = 0;
    tone_timer_ding = false;
  }
  if (tone_timer >= TONE_TIMEOUT)
  {
    tone_timer = 0;
    tone_timer_ding = true;
  }
}

float humidity_hyst_high = 80.0;
float humidity_hyst_low = 60.0;
float temperature;
float humidity;
unsigned long last_sound_detected_tick = 0;
bool sound_detected_recently = false;

int no_of_notes = 16;
int melody[] = {NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_G5, NOTE_A5, NOTE_B5, NOTE_C6, NOTE_C6, NOTE_B5, NOTE_A5, NOTE_G5, NOTE_F5, NOTE_E5, NOTE_D5, NOTE_C5};
int tone_interval = 500;
int current_note = 0;

void printData(String label, float value) {
  Serial.print( "Time : " );
  Serial.print( current_tick, 1 );
  Serial.print( label + " = " );
  Serial.print( value, 1 );
  Serial.println( "%" );
}

void loop()
{
  if (humidity_measurement_timer_ding)
  {
    if(dht_sensor.measure(&temperature, &humidity ))
    {
      humidity_measurement_timer_ding = false;
      printData("Hum", humidity);
    }
  }

  if (sound_measurement_timer_ding)
  {
    soundAnalogValue = analogRead(soundSensorAnalogPin);
    soundDigitalValue=digitalRead(soundSensorDigitalPin);
    sound_measurement_timer_ding = false;
    if(soundDigitalValue==HIGH)
    {
      printData("Snd", soundAnalogValue);
      last_sound_detected_tick = current_tick;
      digitalWrite(soundLed,HIGH);
    }
    else
    {
      digitalWrite(soundLed,LOW);
    }
  }

  if ((current_tick > LAST_SOUND_THRESHOLD_IN_TICKS) && (current_tick - last_sound_detected_tick <= LAST_SOUND_THRESHOLD_IN_TICKS))
  {
    sound_detected_recently = true;
  } else 
  {
    sound_detected_recently = false;
  }


  if ((sound_detected_recently) || (humidity > humidity_hyst_high))
  {
    printData("Recent", 1);
    shower_timeout_enabled = true;
  }

  if (!sound_detected_recently && (humidity < humidity_hyst_low))
  {
    shower_timeout_enabled = false;
  }

  if (!sound_enabled)
  {
    current_note = 0;
    digitalWrite(buzzer, LOW);
  }
  else
  {
    if (tone_timer_ding)
    {
      tone(buzzer, melody[current_note], tone_interval);
      current_note++;
      tone_timer_ding = false;
    }
    if (current_note >= no_of_notes) {
      current_note = 0;
    }
  }
}
