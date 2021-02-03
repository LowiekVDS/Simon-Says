#include <EEPROMex.h>
#include <EEPROMVar.h>
#include <Servo.h>

const int G = 3; //groen
const int Y = 5; //geel
const int B = 6;
const int R = 11;

const int start_btn = 4;
const int G_btn = 7; //groen
const int Y_btn = 12; //geel
const int B_btn = 8;
const int R_btn = 13;

Servo slot;
const int slot_pin = 2;
const int slot_btn = 9;
const int closed_deg = 20;
const int neutral_deg = 110;
const int open_deg = 180;

const int led_order[4] = {R, Y, B, G};
const int anim_dly = 125;
const int game_dly = 125;
const int breath_dly = 2;

const int adr_turnedon = 0;
const int adr_played = 100;
const int adr_won = 200;
const int adr_bypassed = 300;

const int plays = 7;
int history[plays][4];
int succes = 1;
const int decay_dly = 150;

int check_btn_press() {
  if (digitalRead(G_btn) == 0) {
    digitalWrite(G, 1);
    while (digitalRead(G_btn) == 0) {
      delay(10);
    }
    delay(decay_dly);
    digitalWrite(G, 0);
    return G;
  } else if (digitalRead(Y_btn) == 0) {
    digitalWrite(Y, 1);
    while (digitalRead(Y_btn) == 0) {
      delay(10);
    }
    delay(decay_dly);
    digitalWrite(Y, 0);
    return Y;
  } else if (digitalRead(B_btn) == 0) {
    digitalWrite(B, 1);
    while (digitalRead(B_btn) == 0) {
      delay(10);
    }
    delay(decay_dly);
    digitalWrite(B, 0);
    return B;
  } else if (digitalRead(R_btn) == 0) {
    digitalWrite(R, 1);
    while (digitalRead(R_btn) == 0) {
      delay(10);
    }
    delay(decay_dly);
    digitalWrite(R, 0);
    return R;
  }
  return -1;
}

int iteration = 0;
unsigned long startMillis = 0;
void startAnim_update() {
  if (millis() - startMillis >= anim_dly) {
    startMillis = millis();
    disable_leds();
    digitalWrite(led_order[iteration], 1);
    iteration++;
    if (iteration == 4) {
      iteration = 0;
    }
  }
}

void disable_leds() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(led_order[i], 0);
  }
}

int step = 0;
int dir = 1;
unsigned long startMillis2 = 0;
void breathAnim_update() {
  if (millis() - startMillis2 >= breath_dly) {
    step += dir;
    if (step > 255 || step < 0) {
      dir *= -1;
      step += dir;
    }
    for (int i = 0; i < 4; i++) {
      analogWrite(led_order[i], step);
    }
    startMillis2 = millis();
  }
}

int time = 0;
void checkSecret() {
  if (digitalRead(R_btn) + digitalRead(B_btn) + digitalRead(Y_btn) + digitalRead(G_btn) == 0) {
    time++;
    if (time > 1000) {
      disable_leds();
      digitalWrite(G, 1);
      slot.write(open_deg);
      delay(1000);
      slot.write(neutral_deg);
      EEPROM.updateInt(adr_bypassed, EEPROM.readInt(adr_bypassed) + 1);
      while (digitalRead(G_btn) == 1) {}
      while (digitalRead(G_btn) == 0) {}
      slot.write(closed_deg);
    }
  }
}

void checkSerial() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil(';');
    if (command == "open") {
      Serial.print("Opening...");
      slot.write(open_deg);
    } else if (command == "close") {
      Serial.print("Closing...");
      slot.write(closed_deg);
    } else if (command == "write") {
      Serial.print("Writing...");
      slot.write(Serial.parseInt());
      Serial.read();
    } else if (command == "neutral") {
      Serial.print("Neutralizing...");
      slot.write(neutral_deg);
    } else if (command == "printVars") {
      Serial.println("----------");
      String temp = "(" + String(G) + ", " + String(B) + ", " + String(Y) + ", " + String(R) + ")";
      Serial.println("Pins LEDs : (G, B, Y, R) = " + temp);
      temp = "(" + String(G_btn) + ", " + String(B_btn) + ", " + String(Y_btn) + ", " + String(R_btn) + ", " + String(start_btn) + ", " + String(slot_btn) + ")";
      Serial.println("Pins btns : (G, B, Y, R, start, slot) = " + temp);
      temp = "(" + String(slot_pin) + ", " + String(open_deg) + ", " + String(neutral_deg) + ", " + String(closed_deg) + ")";
      Serial.println("Servo : (pin, open_deg, neutral_deg, closed_deg) = " + temp);
      Serial.println("Game : (difficulty) = (" + String(plays) + ")");
      temp = String(adr_turnedon) + ", " + String(adr_played) + ", " + String(adr_won) + ", " + String(adr_bypassed) + ")";
      Serial.println("EEPROM : (adr_turnedon, adr_played, adr_won, adr_bypassed) = (" + temp);
      temp = "(" + String(EEPROM.readInt(adr_turnedon)) + ", " + String(EEPROM.readInt(adr_played)) + ", " + String(EEPROM.readInt(adr_won)) + ", " + String(EEPROM.readInt(adr_bypassed)) + ", " + String(float(EEPROM.readInt(adr_won)) / float(EEPROM.readInt(adr_played))) + ")";
      Serial.println("Stats : (# turned on, # played, # games won, # bypassed, winrate) = " + temp);
      Serial.println("----------");
    } else if (command == "testBtns") {
      disable_leds();
      digitalWrite(G, 1);
      Serial.println();
      Serial.println("Release G_btn");
      while (digitalRead(G_btn) == 0) {}
      Serial.print("Press G_btn...");
      while (digitalRead(G_btn) == 1) {}
      Serial.println("OK");
      Serial.print("Release G_btn...");
      while (digitalRead(G_btn) == 0) {}
      Serial.println("OK");

      digitalWrite(G, 0);
      digitalWrite(B, 1);

      Serial.println();
      Serial.println("Release B_btn");
      while (digitalRead(B_btn) == 0) {}
      Serial.print("Press B_btn...");
      while (digitalRead(B_btn) == 1) {}
      Serial.println("OK");
      Serial.print("Release B_btn...");
      while (digitalRead(B_btn) == 0) {}
      Serial.println("OK");

      digitalWrite(B, 0);
      digitalWrite(Y, 1);

      Serial.println();
      Serial.println("Release Y_btn");
      while (digitalRead(Y_btn) == 0) {}
      Serial.print("Press Y_btn...");
      while (digitalRead(Y_btn) == 1) {}
      Serial.println("OK");
      Serial.print("Release Y_btn...");
      while (digitalRead(Y_btn) == 0) {}
      Serial.println("OK");

      digitalWrite(Y, 0);
      digitalWrite(R, 1);

      Serial.println();
      Serial.println("Release R_btn");
      while (digitalRead(R_btn) == 0) {}
      Serial.print("Press R_btn...");
      while (digitalRead(R_btn) == 1) {}
      Serial.println("OK");
      Serial.print("Release R_btn...");
      while (digitalRead(R_btn) == 0) {}
      Serial.println("OK");

      digitalWrite(R, 0);

      Serial.println();
      Serial.println("Release start_btn");
      while (digitalRead(start_btn) == 0) {}
      Serial.print("Press start_btn...");
      while (digitalRead(start_btn) == 1) {}
      Serial.println("OK");
      Serial.print("Release start_btn...");
      while (digitalRead(start_btn) == 0) {}
      Serial.println("OK");

      Serial.println();
      Serial.println("Release slot_btn");
      while (digitalRead(slot_btn) == 0) {}
      Serial.print("Press slot_btn...");
      while (digitalRead(slot_btn) == 1) {}
      Serial.println("OK");
      Serial.print("Release slot_btn...");
      while (digitalRead(slot_btn) == 0) {}
      Serial.println("OK");
    } else if (command == "reset") {
      Serial.println("Resetting...");
      EEPROM.updateInt(adr_turnedon, 0);
      EEPROM.updateInt(adr_played, 0);
      EEPROM.updateInt(adr_won, 0);
      EEPROM.updateInt(adr_bypassed, 0);
    } else {
      Serial.println("Unknown command : " + command);
      Serial.println("Possible commands : open; neutral; close; write; printVars; testBtns; reset;");
    }
    Serial.println("done");
  }
}

void setup() {
  EEPROM.updateInt(adr_turnedon, EEPROM.readInt(adr_turnedon) + 1);
  Serial.begin(115200);
  pinMode(R, OUTPUT);
  pinMode(Y, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(G, OUTPUT);

  pinMode(G_btn, INPUT_PULLUP);
  pinMode(Y_btn, INPUT_PULLUP);
  pinMode(B_btn, INPUT_PULLUP);
  pinMode(R_btn, INPUT_PULLUP);
  pinMode(start_btn, INPUT_PULLUP);
  pinMode(slot_btn, INPUT_PULLUP);

  slot.attach(slot_pin);
  slot.write(closed_deg);
  randomSeed(analogRead(A0));
}

boolean close = true;
void loop() {
  if (digitalRead(start_btn) == 0) {
    while (digitalRead(start_btn) == 0) {}
    EEPROM.updateInt(adr_played, EEPROM.readInt(adr_played) + 1);
    //reset lichten en spel
    succes = 1;
    disable_leds();
    slot.write(closed_deg);
    delay(500);
    for (int i = 0; i < plays; i++) {
      for (int j = 0; j < 4; j++) {
        history[i][j] = 0;
      }
    }
    //spel zelf

    for (int i = 0; i < plays; i++) {
      history[i][random(4)] = 1;
      for (int j = 0; j <= i; j++) {
        for (int k = 0; k < 4; k++) {
          if (history[j][k] == 1) {
            digitalWrite(led_order[k], 1);
            delay(game_dly);
            digitalWrite(led_order[k], 0);
            delay(game_dly);
          }
        }
      }
      //inputs
      for (int j = 0; j <= i; j++) {
        int btn;
        do {
          btn = check_btn_press();
        } while (btn == -1);
        for (int k = 0; k < 4; k++) {
          if (led_order[k] == btn) {
            if (history[j][k] != 1) {
              succes = 0;
            }
          }
        }
        if (succes == 0) {
          break;
        }
      }
      if (succes == 0) {
        break;
      }
      delay(500);
    }
    if (succes == 1) {
      EEPROM.updateInt(adr_won, EEPROM.readInt(adr_won) + 1);
      slot.write(open_deg);
      for (int i = 0; i < 1000; i++) {
        delay(1);
        breathAnim_update();
      }
      slot.write(neutral_deg);
      unsigned long temp = millis();
      do {
        breathAnim_update();
      } while (digitalRead(slot_btn) == 1);
      disable_leds();
      slot.write(closed_deg);
    }

  } else {
    startAnim_update();
    checkSecret();
    checkSerial();
    while (digitalRead(slot_btn) == 1) {
      close = true; 
      slot.write(neutral_deg);
      disable_leds();
      digitalWrite(R, 1);
      checkSecret();
      checkSerial();
      delay(1);
    }
    if (close) {
      slot.write(closed_deg);
      close = false;
    }
    delay(1);
  }
}
