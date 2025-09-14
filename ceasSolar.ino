#include <Stepper.h>
#include <Adafruit_NeoPixel.h>

/* === PINOUT === */
#define IN1 8
#define IN2 9
#define IN3 10
#define IN4 11
#define LED_PIN 3
#define NUM_LEDS 57

/* === motor === */
Stepper stepper(200, IN1, IN2, IN3, IN4);   // 1-2-3-4 full-step
const int  MOTOR_RPM = 60;
bool motorEnabled    = false;               // pornim în STOP

/* === banda LED === */
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
const uint8_t hourLed[12] = {
  28, // 12
  33, //  1
  38, //  2
  43, //  3
  48, //  4
  52, //  5
   0, //  6
   8, //  7 (deja mutat)
   9, //  8
  14, //  9
  19, // 10
  31  // 11  <-- nou!
};


/* aprinde spotul unui LED */
void showSpot(uint8_t idx) {
  strip.clear();
  if (idx < NUM_LEDS) strip.setPixelColor(idx, 0xFFFFFF);
  strip.show();
}

/* stinge toate LED-urile */
void clearAllLeds() {
  strip.clear();
  strip.show();
}

/* === comenzi Serial ===
   H<n>  → afiseaza LED ora n (1-12)
   STOP  → opreste motorul (dezenergizeaza bobinele)
   RUN   → impuls 10 pasi CW
   OFF   → stinge complet banda LED
*/
void parseSerial() {
  if (!Serial.available()) return;
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  if (cmd.startsWith("H")) {               // ex. H5
    int h = cmd.substring(1).toInt();
    if (h >= 1 && h <= 12) {
      showSpot(hourLed[h % 12]);
      Serial.print("LED pentru ora "); Serial.println(h);
    }
  }
  else if (cmd.equalsIgnoreCase("OFF")) {  // stinge banda
    clearAllLeds();
    Serial.println("Toate LED-urile stinse");
  }
  else if (cmd.equalsIgnoreCase("STOP")) {
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
    motorEnabled = false;
    Serial.println("Motor OFF");
  }
  else if (cmd.equalsIgnoreCase("RUN")) {
    motorEnabled = true;
    stepper.setSpeed(MOTOR_RPM);
    stepper.step(10);                      // 10 pasi CW
    Serial.println("Motor impuls 10 pasi");
  }
}

void setup() {
  Serial.begin(9600);
  stepper.setSpeed(MOTOR_RPM);

  strip.begin();
  strip.setBrightness(255);                // luminanta maxima
  strip.show();

  showSpot(hourLed[6]);                    // pornim la ora 6
  Serial.println("READY  –  H<n>, OFF, RUN, STOP");
}

void loop() {
  parseSerial();                            // asculta comenzi

  if (motorEnabled) {
    /* simulare „motor stricat” – 200 pasi CW, apoi 200 pasi CCW */
    static int dir = 1;
    static int cnt = 0;

    stepper.step(dir);          // 1 pas
    if (++cnt >= 200) {
      dir = -dir;
      cnt = 0;
    }
  }
}
