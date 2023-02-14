#include <SPI.h>

#include <Betweener.h>

Betweener b;
int LED = 8;  // Pin 8 is attached to Betweener's LED
int count;
int trigLength = 15;

byte clock_tick;
byte clock_value;
byte play_flag;
byte play_tick;

// CV Outputs
const int CV1 = 1;
const int CV2 = 2;
const int CV3 = 3;
const int CV4 = 4;

int clockDivs[] = {1,3,6,9};

void setup() {
  b.begin();
  
  Serial.begin(9600);
  
  usbMIDI.setHandleRealTimeSystem(OnClock);
  
  pinMode(LED, OUTPUT);
}

void loop() {
  ///////////////////////////////////////////////////
  //this function reads triggers, CV, knobs and USBMIDI.
  b.readAllInputs();

  // Check each trigger for rising voltage
  if (b.triggerRose(1)) {
    OnClock();
  }
  // if (b.triggerRose(2)) {
  // }
  // if (b.triggerRose(3)) {
  // }
  // if (b.triggerRose(4)) {
  // }

  // Check each trigger for a falling voltage/Note OFF
  // if (b.triggerFell(1)) {

  // }
  // if (b.triggerFell(2)) {

  // }
  // if (b.triggerFell(3)) {

  // }
  // if (b.triggerFell(4)) {

  // }

  //////////////////////////////////////////////////////////////

  //CVInputs and KNOBS to USB MIDI Continous Controller (CC) messages/////////
  // only transmit MIDI messages if analog input changed
  // if (b.CVChanged(1)) {
  // }

  // if (b.CVChanged(2)) {
  // }

  // if (b.CVChanged(3)) {
  // }

  // if (b.CVChanged(4)) {
  // }

  if (b.knobChanged(1)) {
    // update divisions for first clock output
    setClockDiv(1, b.currentKnob1);
  }

  if (b.knobChanged(2)) {
    // update divisions for second clock output
    setClockDiv(2, b.currentKnob2);
  }

  if (b.knobChanged(3)) {
    // update divisions for third clock output
    setClockDiv(3, b.currentKnob3);
  }

  if (b.knobChanged(4)) {
    // update divisions for fourth clock output
    setClockDiv(4, b.currentKnob4);
  }

  //////////////////////////////////////////////////////////////////////
}

void trigOut(byte cvOut) {
  b.writeCVOut(cvOut, 4095); //write the CV HIGH (approx 5v)
  digitalWriteFast(8, HIGH); //turn LED On when a Note On message is received
  delay(trigLength); //write the CV LOW (0v)
  digitalWriteFast(8, LOW); //turn LED Off
  b.writeCVOut(cvOut, 0);
}

void setClockDiv(int channel, int val) {
  int index = channel - 1;
  if (val < 128) {
    clockDivs[index] = 1;
  } else if (val > 127 && val < 256) {
    clockDivs[index] = 2;
  } else if (val > 255 && val < 384) {
    clockDivs[index] = 3;
  } else if (val > 383 && val < 512) {
    clockDivs[index] = 4;
  } else if (val > 511 && val < 640) {
    clockDivs[index] = 6;
  } else if (val > 639 && val < 768) {
    clockDivs[index] = 8;
  } else if (val > 767 && val < 896) {
    clockDivs[index] = 9;
  } else if (val > 895 && val < 1024) {
    clockDivs[index] = 12;
  } else {
    clockDivs[index] = 1;
  }
}

void OnStart() {
  count = 0;
}

void OnStop() {
  digitalWrite(LED, LOW);
}

void OnContinue() {
  count = 0;
}

void OnClock(byte clockbyte) {
  if (clockbyte == 0xf8) {
    // % 12 = quarters
    // % 3 = sixteenths
    // % 6 = eights
    // for (int counter = 1; counter == 4; counter++) {
    //   int index = counter - 1;
    //   if (1 - bitRead(clock_tick / clockDivs[index], 0)) {
    //     trigOut(counter);
    //   }
    // }
    if (1 - bitRead(clock_tick / 12, 0)) {
      trigOut(1);
    }
    if (1 - bitRead(clock_tick / 3, 0)) {
      trigOut(2);
    }
    if (1 - bitRead(clock_tick / 6, 0)) {
      trigOut(3);
    }

    digitalWriteFast(LED, 1 - bitRead(clock_tick / 12, 0));
    digitalWriteFast(LED, 1 - bitRead(clock_tick / 12, 0));  // quarter note LED flash

    clock_tick++;

    if (clock_tick == 24) {
      clock_tick = 0;
    }

    if (clock_tick == 6 && play_tick == 1) {
      play_tick = 0;
      b.writeCVOut(CV1, 0);
    }
  }

  if (clockbyte == 0xfa || clockbyte == 0xfb) {  // start or continue bytes
    play_flag = 1;
    play_tick = 1;
    clock_value = 0;
    clock_tick = 0;
    // b.writeCVOut(CV1, HIGH);
  }

  if (clockbyte == 0xfc) {  // stop byte
    b.writeCVOut(CV1, 0);
    b.writeCVOut(CV2, 0);
    b.writeCVOut(CV3, 0);
    b.writeCVOut(CV4, 0);
    digitalWriteFast(LED, LOW);
    play_flag = 0;
  }
}