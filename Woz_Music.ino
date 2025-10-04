// Date : Oct 2025
// Author : Suyash Joshi
// Hardware Setup : Circuit Playground Classic - Fruit Drum with diagnostics

#include <Adafruit_CircuitPlayground.h>
#include <MIDIUSB.h>

// TUNING PARAMETERS - Start with higher threshold
int CAP_THRESHOLDS[8] = {150, 150, 150, 150, 150, 150, 150, 700};  // Pad 9 (index 6) gets 700
#define DEBOUNCE_TIME 400        // INCREASED for more stability
#define CALIBRATION_SAMPLES 30   // More samples for better baseline

// MIDI Configuration
#define MIDI_CHANNEL 0
#define VELOCITY 100
#define NOTE_DURATION 150

uint8_t pads[] = {3, 2, 0, 1, 12, 6, 9, 10};
uint8_t numberOfPads = sizeof(pads)/sizeof(uint8_t);

// MIDI note numbers
uint8_t midiNotes[] = {60, 62, 64, 65, 67, 69, 71, 72};

// Frequencies for speaker
uint16_t frequencies[] = {262, 294, 330, 349, 392, 440, 494, 523};

// State tracking
unsigned long lastTriggerTime[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned long noteOffTime[8] = {0, 0, 0, 0, 0, 0, 0, 0};
boolean noteOn[8] = {false, false, false, false, false, false, false, false};
int baselineValues[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// Mode toggle
boolean MIDI_MODE = false;
boolean DIAGNOSTIC_MODE = true;  // Shows real-time values

// Buttons
#define LEFT_BUTTON 4
#define RIGHT_BUTTON 19
unsigned long lastButtonPress = 0;
#define BUTTON_DEBOUNCE 500

// For periodic diagnostics
unsigned long lastDiagnostic = 0;
#define DIAGNOSTIC_INTERVAL 2000  // Print diagnostics every 2 seconds

void setup() {
  Serial.begin(115200);
  CircuitPlayground.begin();
  
  delay(2000);
  
  Serial.println("\n\n========================================");
  Serial.println("FRUIT DRUM - DIAGNOSTIC MODE");
  Serial.println("========================================");
  Serial.println("CONTROLS:");
  Serial.println("  LEFT button  = Calibrate");
  Serial.println("  RIGHT button = Toggle MIDI/Speaker");
  Serial.println("========================================");
  Serial.println("\nConnect fruits, then press LEFT to calibrate");
  Serial.println("========================================\n");
  
  waitForCalibrationButton();
}

void loop() {
  unsigned long currentTime = millis();
  
  // LEFT BUTTON - Calibration
  if (CircuitPlayground.leftButton() && (currentTime - lastButtonPress > BUTTON_DEBOUNCE)) {
    lastButtonPress = currentTime;
    recalibrate();
  }
  
  // RIGHT BUTTON - Toggle mode
  if (CircuitPlayground.rightButton() && (currentTime - lastButtonPress > BUTTON_DEBOUNCE)) {
    lastButtonPress = currentTime;
    toggleMode();
  }
  
  // Periodic diagnostic output
  if (DIAGNOSTIC_MODE && (currentTime - lastDiagnostic > DIAGNOSTIC_INTERVAL)) {
    lastDiagnostic = currentTime;
    printDiagnostics();
  }
  
  // Check each pad
  for (int i = 0; i < numberOfPads; i++) {
    if (capButton(pads[i], i)) {
      playNote(i);
      lastTriggerTime[i] = millis();
    }
    
    // Turn off MIDI notes
    if (MIDI_MODE && noteOn[i] && (millis() - noteOffTime[i] >= NOTE_DURATION)) {
      sendNoteOff(midiNotes[i], MIDI_CHANNEL);
      noteOn[i] = false;
    }
  }
  
  delay(10);
}

// Print real-time sensor diagnostics
void printDiagnostics() { 
  Serial.println("\n--- REAL-TIME DIAGNOSTICS ---");
  Serial.print("Mode: ");
  Serial.println(MIDI_MODE ? "MIDI" : "SPEAKER");
  Serial.println("Thresholds: 150, 150, 150, 150, 150, 150, 150, 700");
  Serial.println();
  
  for (int i = 0; i < numberOfPads; i++) {
    int currentValue = CircuitPlayground.readCap(pads[i]);
    int delta = currentValue - baselineValues[i];
    
    Serial.print("Pad ");
    Serial.print(pads[i]);
    Serial.print(": Current=");
    Serial.print(currentValue);
    Serial.print(" | Baseline=");
    Serial.print(baselineValues[i]);
    Serial.print(" | Delta=");
    Serial.print(delta);
    Serial.print(" | Status: ");
    
    if (delta > CAP_THRESHOLDS[i]) {
      Serial.println(">>> TRIGGERING! <<<");
    } else if (delta > CAP_THRESHOLDS[i] * 0.5) {
      Serial.println("Close to threshold");
    } else {
      Serial.println("OK");
    }
  }
  Serial.println("-----------------------------\n");
}

void toggleMode() {
  MIDI_MODE = !MIDI_MODE;
  
  if (MIDI_MODE) {
    Serial.println("\n****************************************");
    Serial.println("*** MIDI MODE ENABLED ***");
    Serial.println("*** Sending MIDI to GarageBand ***");
    Serial.println("****************************************\n");
    
    // Send a test MIDI note
    Serial.println("Sending test MIDI note (C4)...");
    sendNoteOn(60, MIDI_CHANNEL);
    delay(200);
    sendNoteOff(60, MIDI_CHANNEL);
    Serial.println("Test note sent!\n");
    
    // Blue LEDs
    for (int i = 0; i < 10; i++) {
      CircuitPlayground.setPixelColor(i, 0, 0, 255);
    }
  } else {
    Serial.println("\n****************************************");
    Serial.println("*** SPEAKER MODE ENABLED ***");
    Serial.println("*** Playing from built-in speaker ***");
    Serial.println("****************************************\n");
    
    // Red LEDs
    for (int i = 0; i < 10; i++) {
      CircuitPlayground.setPixelColor(i, 255, 0, 0);
    }
  }
  
  delay(1500);
  
  // Turn off LEDs
  for (int i = 0; i < 10; i++) {
    CircuitPlayground.setPixelColor(i, 0, 0, 0);
  }
}

void recalibrate() {
  Serial.println("\n========================================");
  Serial.println("CALIBRATING - Don't touch fruits!");
  Serial.println("========================================");
  
  // Yellow LEDs
  for (int i = 0; i < 10; i++) {
    CircuitPlayground.setPixelColor(i, 255, 255, 0);
  }
  
  delay(1000);
  calibrateBaseline();
  printBaselines();
  
  // Green LEDs
  for (int i = 0; i < 10; i++) {
    CircuitPlayground.setPixelColor(i, 0, 255, 0);
  }
  delay(1500);
  
  // Turn off LEDs
  for (int i = 0; i < 10; i++) {
    CircuitPlayground.setPixelColor(i, 0, 0, 0);
  }
  
  Serial.println("Calibration complete! Watch diagnostics above.\n");
  lastDiagnostic = 0;  // Force immediate diagnostic print
}

void playNote(uint8_t index) {
  Serial.print(">>> TRIGGERED! PAD "); 
  Serial.print(pads[index]); 
  
  if (MIDI_MODE) {
    Serial.print(" - Sending MIDI Note: "); 
    Serial.println(midiNotes[index]);
    
    sendNoteOn(midiNotes[index], MIDI_CHANNEL);
    noteOn[index] = true;
    noteOffTime[index] = millis();
    
  } else {
    Serial.print(" - Playing tone: "); 
    Serial.print(frequencies[index]);
    Serial.println(" Hz");
    
    CircuitPlayground.playTone(frequencies[index], 100, false);
  }
}

void sendNoteOn(byte note, byte channel) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, note, VELOCITY};
  MidiUSB.sendMIDI(noteOn);
  MidiUSB.flush();
  Serial.println("  → MIDI Note ON sent");
}

void sendNoteOff(byte note, byte channel) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, note, 0};
  MidiUSB.sendMIDI(noteOff);
  MidiUSB.flush();
}

void waitForCalibrationButton() {
  boolean buttonPressed = false;
  int brightness = 0;
  int direction = 5;
  
  while (!buttonPressed) {
    if (CircuitPlayground.leftButton()) {
      buttonPressed = true;
      
      for (int i = 0; i < 10; i++) {
        CircuitPlayground.setPixelColor(i, 255, 255, 0);
      }
      
      delay(1000);
      Serial.println("\nCalibrating baseline...");
      calibrateBaseline();
      printBaselines();
      
      for (int i = 0; i < 10; i++) {
        CircuitPlayground.setPixelColor(i, 0, 255, 0);
      }
      delay(1500);
      
      for (int i = 0; i < 10; i++) {
        CircuitPlayground.setPixelColor(i, 0, 0, 0);
      }
      
      Serial.println("\n>>> Ready! Starting in SPEAKER mode");
      Serial.println(">>> Diagnostics will print every 2 seconds\n");
    }
    
    // Pulsing white
    for (int i = 0; i < 10; i++) {
      CircuitPlayground.setPixelColor(i, brightness, brightness, brightness);
    }
    brightness += direction;
    if (brightness >= 50 || brightness <= 0) direction = -direction;
    
    delay(20);
  }
}

void calibrateBaseline() {
  Serial.println("Taking samples...");
  
  for (int i = 0; i < numberOfPads; i++) {
    long sum = 0;
    
    for (int sample = 0; sample < CALIBRATION_SAMPLES; sample++) {
      int reading = CircuitPlayground.readCap(pads[i]);
      sum += reading;
      delay(50);
    }
    
    baselineValues[i] = sum / CALIBRATION_SAMPLES;
  }
}

void printBaselines() {
  Serial.println("\n========================================");
  Serial.println("BASELINE VALUES (fruits connected):");
  Serial.println("========================================");
  for (int i = 0; i < numberOfPads; i++) {
    Serial.print("  Pad ");
    Serial.print(pads[i]); 
    Serial.print(": "); 
    Serial.println(baselineValues[i]);
  }
  Serial.println("========================================\n");
}

boolean capButton(uint8_t pad, uint8_t index) {
  int currentValue = CircuitPlayground.readCap(pad);
  int delta = currentValue - baselineValues[index];
  unsigned long currentTime = millis();
  boolean debounced = (currentTime - lastTriggerTime[index]) > DEBOUNCE_TIME;
  
  return (delta > CAP_THRESHOLDS[index]) && debounced;
}
