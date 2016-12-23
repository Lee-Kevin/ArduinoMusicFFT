/*
 * File: backdoor.ino
 * 
 * Author: Jiankai Li
 * Date: 2016-11-22
 */
#include <avr/wdt.h>
#include <TimerOne.h>
#include <Adafruit_NeoPixel.h>

#include <math.h>

#define PIXIEL_PIN0 2
#define PIXIEL_PIN1 3
#define PIXIEL_PIN2 4
#define PIXIEL_PIN3 5

//#define PIXIEL_NUM 24

#define FASTADC 1

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define LOG_OUT 1 // use the log output function
#define FHT_N 256 // set to 256 point fht

#define OCTAVE 1
#define OCT_NORM 0

#include <FHT.h> // include the library

uint8_t THRESHOLD[] = {10,7,90,90,90,60,60,45};
uint8_t PIXIEL_NUM[] = {0,24,16,8};

// Attempt to 'zero out' noise when line in is 'quiet'.  You can change this to make some segments more sensitive.
// int  oct_bias[] = { 231, 215, 80, 55, 54, 82, 65};

int  oct_bias[] = { 217, 201, 65, 46, 50,55, 55, 50};

// Divide Threshold by 2 for top octave? 1 - yes 2 - no.  Makes highest frequency blink more.
#define TOP_OCTAVE_DIVIDE true 

Adafruit_NeoPixel strip0 = Adafruit_NeoPixel(PIXIEL_NUM[0], PIXIEL_PIN0, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(PIXIEL_NUM[1], PIXIEL_PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(PIXIEL_NUM[2], PIXIEL_PIN2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip3 = Adafruit_NeoPixel(PIXIEL_NUM[3], PIXIEL_PIN3, NEO_GRB + NEO_KHZ800);
uint32_t RGB[8] = {0x150000,0x001500,0x000015,0x151500,0x150015,0x001515,0x0A2A3A,0x3A2A0A}; 
//uint32_t RGB[8] = {0x150000,0x001500,0x000015,0x150000,0x150015,0x001515,0x0A2A3A,0x3A2A0A}; 
Adafruit_NeoPixel strip[4] = {strip0,strip1,strip2,strip3};
void setup() {
    int start ;
    int i ;
    
    Serial.begin(115200);
    Serial.println("power on!");

    wdt_enable(WDTO_2S);
     
    Timer1.initialize(1000000); // set a timer of length 1000000 microseconds 1 second
    Timer1.attachInterrupt( timerIsrFeedFog ); // attach the service routine here
    wdt_reset();
    
    // strip[0] = Adafruit_NeoPixel(PIXIEL_NUM, PIXIEL_PIN0, NEO_GRB + NEO_KHZ800);
    // strip[1] = Adafruit_NeoPixel(PIXIEL_NUM, PIXIEL_PIN1, NEO_GRB + NEO_KHZ800);
    // strip[2] = Adafruit_NeoPixel(PIXIEL_NUM, PIXIEL_PIN2, NEO_GRB + NEO_KHZ800);
    // strip[3] = Adafruit_NeoPixel(PIXIEL_NUM, PIXIEL_PIN3, NEO_GRB + NEO_KHZ800);
      
      for (uint8_t j=1; j<4; j++){
          strip[j].begin();
          for (int i = 0; i<PIXIEL_NUM[j]; i++) {
                strip[j].setPixelColor(i,RGB[j]);
                //strip[j].setPixelColor(i,0xA0,0xA0,00);
                
          }
          strip[j].show();          
      }
    delay(1000);
     
    #if FASTADC
     // set prescale to 16
    sbi(ADCSRA,ADPS2) ;
    cbi(ADCSRA,ADPS1) ;
    cbi(ADCSRA,ADPS0) ;
    #endif
    Serial.print("ADCTEST: ") ;
    start = millis() ;
    for (i = 0 ; i < 1000 ; i++)
       analogRead(0) ;
    Serial.print(millis() - start) ;
    Serial.println(" msec (1000 calls)") ;
}

void loop() {
  while(1) { // reduces jitter

    for (int i = 0 ; i < FHT_N ; i++) { // save 256 samples
      
      fht_input[i] = (analogRead(0)-512)*64; // put real data into bins
    }
    fht_window(); // window the data for better frequency response
    fht_reorder(); // reorder the data before doing the fht
    fht_run(); // process the data in the fht
    // fht_mag_log(); // take the output of the fht
    fht_mag_octave(); 

    // Serial.println("start");
    for (uint8_t i = 0 ; i < 8 ; i++) {
      // Serial.print(fht_oct_out[i]);
      //Serial.print(fht_oct_out[i] - oct_bias[i]);
      // Serial.print(" "); // send out the data
    }
    // Serial.println(" ");
    frequencyGraph();
  }
}
void timerIsrFeedFog()
{
    wdt_reset();
}

void frequencyGraph() {
    for(uint8_t i=2; i<8; i++) {
        if (abs((fht_oct_out[i] - oct_bias[i])) > THRESHOLD[i]) {
                for (uint8_t j=(i%2)*(PIXIEL_NUM[i/2]/2); j<(i%2+1)*(PIXIEL_NUM[i/2]/2); j++){
                    // strip[i/2].setPixelColor(i,fht_oct_out[i],0xA0,00);
                    strip[i/2].setPixelColor(j,RGB[i]);
                    strip[i/2].show();
                }    
        } else{
                for (uint8_t j=(i%2)*(PIXIEL_NUM[i/2]/2); j<(i%2+1)*(PIXIEL_NUM[i/2]/2); j++){
                    // strip[i/2].setPixelColor(i,fht_oct_out[i],0xA0,00);
                    strip[i/2].setPixelColor(j,0x000000);
                    strip[i/2].show();
                }
        }
        
    }
    
}
