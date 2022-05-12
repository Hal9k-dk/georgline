#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <SPI.h>
#include <SD.h>

#define BUFSIZE 256

File file;
byte buffer[BUFSIZE];
volatile unsigned char count = 0;

volatile unsigned int step = 0;

void setup() {
  DDRC = 0x3f;
  DDRD = 0xfc;
  
  Serial.begin(115200);
  
  if (!SD.begin()) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
  file = SD.open("output.raw", FILE_READ);
  if (!file) {
    Serial.println("file not found");
    while(1);
  }
  Serial.println("file opened");

  file.read(buffer, BUFSIZE);

  cli();
  TCCR1A = 0;
  TCCR1B = 0; 
  OCR1A = 2000;
  TCCR1B = (1<<WGM12) | (1<<CS20); 
  TIMSK1 = (1<<OCIE1A);
  sei();

  wdt_enable(WDTO_15MS);
}


void play() {
    unsigned int *sample = (unsigned int*) (buffer + count);
    
    int ol = *sample>>4 & 0x3f;
    int oh = (*sample>>4 & 0xfc0) >> 4;

    
    PORTC = ol;
    PORTD = oh;

    count += 2;
}

void playTriangle() {
    int ol = step>>4 & 0x3f;
    int oh = (step>>4 & 0xfc0) >> 4;
    PORTC = ol;
    PORTD = oh;
    step += 512; 
}

ISR(TIMER1_COMPA_vect){
   play();
   //playTriangle();
   wdt_reset();
}


void loop() {
  while(count < BUFSIZE / 2);
  byte r = file.read(buffer, BUFSIZE / 2);
  if (!file.available()) file.seek(0);
  while(count >= BUFSIZE / 2);
  file.read(buffer+(BUFSIZE/2), BUFSIZE / 2);
  if (!file.available()) file.seek(0);
}
