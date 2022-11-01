#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MIDI.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels.
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


//BUTTON DECLARATION
#define ROTDT  A4
#define ROTCLK A3
#define TOGGLE_BUTON A5

unsigned long TimeOfLastDebounce = 0;
unsigned int DelayofDebounce = 0.1;

unsigned long lastDebounceTimeToggle = 0;  // the last time the output pin was toggled
unsigned long debounceDelayToggle = 50;    // the debounce time; increase if the output flickers


// Store previous Pins state
int PreviousCLK;   
int PreviousDATA;

int buttonStateToggle;             // the current reading from the input pin
int lastButtonStateToggle = LOW; 

#define PIEZO 7
unsigned long lastwait = 0;
unsigned long wait = 0;
int bpm = 120;
bool activate = true;


//MIDI
MIDI_CREATE_DEFAULT_INSTANCE();
void setup() {

  //Serial.begin(9600);
  MIDI.begin(MIDI_CHANNEL_OFF);

  pinMode(PIEZO,OUTPUT);    
  pinMode(LED_BUILTIN, OUTPUT);
  PreviousCLK=digitalRead(ROTCLK);
  PreviousDATA=digitalRead(ROTDT);
  pinMode(TOGGLE_BUTON, INPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    // Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Clear the buffer
  display.clearDisplay();

  testdrawstyles(bpm);    // Draw 'stylized' characters
  // delay(1000);
}

void loop() {

  if ((millis() - TimeOfLastDebounce) > DelayofDebounce) {
    check_rotary();
    PreviousCLK=digitalRead(ROTCLK);
    PreviousDATA=digitalRead(ROTDT);
    TimeOfLastDebounce=millis();
  }
  
  int readingToggle = digitalRead(TOGGLE_BUTON);
  if (readingToggle != lastButtonStateToggle) {
    lastDebounceTimeToggle = millis();
  }
    
  if ((millis() - lastDebounceTimeToggle) > debounceDelayToggle) {
    if (readingToggle != buttonStateToggle) {
      buttonStateToggle = readingToggle;
  
      // only toggle the LED if the new button state is HIGH
      if (buttonStateToggle == LOW) {
          activate = !activate;
      }
    }
  }
  lastButtonStateToggle = readingToggle;

  wait = 60000/bpm;
  wait = wait-50;
  if(activate){
    if(millis()-lastwait>wait){
      MIDI.sendClock();
      toneGenerate();
    }
  }
}

void toneGenerate(){
  lastwait = millis();
  digitalWrite(LED_BUILTIN,HIGH);
  piezoTone(2000,50);
  digitalWrite(LED_BUILTIN,LOW);
}
void testdrawstyles(int ctnr) {
  display.clearDisplay();
  display.setTextSize(2);             // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20,20);
  display.print(ctnr);
  display.println( " BPM");
  display.display();
  delay(1);
}

void check_rotary() {
  if ((PreviousCLK == 0) && (PreviousDATA == 1)) {
    if ((digitalRead(ROTCLK) == 1) && (digitalRead(ROTDT) == 0)) {
      bpm=bpm+5;
      testdrawstyles(bpm);
    }
    if ((digitalRead(ROTCLK) == 1) && (digitalRead(ROTDT) == 1)) {
      bpm=bpm-5;
      testdrawstyles(bpm);
    }
  }

  if ((PreviousCLK == 1) && (PreviousDATA == 1)) {
    if ((digitalRead(ROTCLK) == 0) && (digitalRead(ROTDT) == 1)) {
      bpm=bpm+5;
      testdrawstyles(bpm);
    }
    if ((digitalRead(ROTCLK) == 0) && (digitalRead(ROTDT) == 0)) {
      bpm=bpm-5;
      testdrawstyles(bpm);
    }
  }     
}
void piezoTone(long freq, long duration){
  long aSecond = 1000000;
  long period = aSecond/freq;
  duration = duration*1000;
  duration = duration/period;
  for(long k = 0; k < duration; k++){
    digitalWrite(PIEZO,HIGH);
    delayMicroseconds(period/2);
    digitalWrite(PIEZO,LOW);
    delayMicroseconds(period/2);
  }
} 
