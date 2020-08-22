// Date : Aug 21, 2020
// Author : Suyash Joshi
// Hardware Setup : Circuit Playground Classic - USB/Battery Powered connected with Aligator Wires from touch pads to Fruits
// Description : Program that plays music when someone touches the fruits

#include <Adafruit_CircuitPlayground.h>

#define CAP_THRESHOLD 50
#define DELAY_TIME 250

uint8_t pads[] = {3, 2, 0, 1, 12, 6, 9, 10};
uint8_t numberOfPads = sizeof(pads)/sizeof(uint8_t);

void setup() {
  // Initialize serial.
  Serial.begin(9600); 
  
  // Initialize Circuit Playground library.
  CircuitPlayground.begin();
}

void loop() {
  Serial.print("number of pads " + numberOfPads);
  // Loop over every capacitive touch pad.
  for (int i=0; i<numberOfPads; i++) {
    // Play music for each pad if and when touched
    if (capButton(pads[i])) {
      playMusic(pads[i]);
    }
  }
}

// helper function to play music note at 4th octve frequency for each pad for 100 ms
void playMusic(uint8_t pad) {
  Serial.print("PAD "); Serial.print(pad); Serial.print(" playing note: ");
  switch (pad) {
    case 3:
      Serial.println("C");
      // CircuitPlayground.playTone(131, 100, false);
      CircuitPlayground.playTone(262, 100, false);
      break;
    case 2:
      Serial.println("D");
      // CircuitPlayground.playTone(147, 100, false);
      CircuitPlayground.playTone(294, 100, false);
      break;
    case 0:
      Serial.println("E");
      //CircuitPlayground.playTone(165, 100, false);
      CircuitPlayground.playTone(330, 100, false);
      break;
    case 1:
      Serial.println("F");
      // CircuitPlayground.playTone(175, 100, false);
      CircuitPlayground.playTone(349, 100, false);
      break;
    case 12:
      Serial.println("G");
     // CircuitPlayground.playTone(196, 100, false);
      CircuitPlayground.playTone(392, 100, false);
      break;
    case 6:
      Serial.println("A");
    //  CircuitPlayground.playTone(220, 100, false);
      CircuitPlayground.playTone(440, 100, false);
      break;
    case 9:
      Serial.println("B");
     // CircuitPlayground.playTone(246, 100, false);
      CircuitPlayground.playTone(494, 100, false);
      break;
    case 10:
      Serial.println("C");
     // CircuitPlayground.playTone(262, 100, false);
      CircuitPlayground.playTone(523, 100, false);
      break;
    default:
      Serial.println("Exception occured!");
  }
}

// helper function to check for real touch based on threshold value
boolean capButton(uint8_t pad) {
  if (CircuitPlayground.readCap(pad) > CAP_THRESHOLD) {
    return true;  
  } else {
    return false;
  }
}
