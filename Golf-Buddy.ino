#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH        128
#define SCREEN_HEIGHT        64
#define OLED_RESET          -1
#define SCREEN_ADDRESS      0x3C  // Updated address from 0x3D to 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const byte buttonPins[] = {5, 4, 3, 2}; // PREV, ADD, CLEAR, NEXT

typedef enum {
  START,
  RUNNING,
  IDLE,
  SCORE
} State;

typedef enum {
  PREV,
  ADD,
  CLEAR,
  NEXT
} Input;

typedef enum {
  HOLE,
  PAR,
  STROKE
} Option;

struct Holes {
  uint8_t par;
  uint8_t strokes;
};

#define GAME_HOLE_FULL      18
#define GAME_HOLE_HALF       9
#define MAX_PAR              5
#define MAX_STROKE          13
#define STARTING_HOLE        1
#define STARTING_PAR         3
#define STARTING_STROKE      0

State gameState;
Input inp;
Input newInp;
Option opt;

uint8_t totalHoles;
uint8_t currentHole;
Holes holes[GAME_HOLE_FULL];
bool selectingHoles;

unsigned long lastButtonTime   = 0;
#define DEBOUNCE_MS             200

unsigned long lastActivityTime = 0;
#define IDLE_TIMEOUT_MS         30000

// Call splash before game setup in setup()
void setup() {
  Serial.begin(9600);
  Serial.println("\nHello World!");

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  for (byte i = 0; i < 4; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  drawSplashScreen(); // Show splash for 2 seconds on boot
  delay(3000);        // Hold splash for 2 seconds
  setupGame();
}

void initHoles() {
  for (uint8_t i = 0; i < GAME_HOLE_FULL; i++) {
    holes[i].par     = STARTING_PAR;
    holes[i].strokes = STARTING_STROKE;
  }
}

void setupGame() {
  gameState      = START;
  selectingHoles = true;
  totalHoles     = 0;
  currentHole    = 0;  // 0 = 9 holes highlighted, 1 = 18 holes highlighted
  opt            = HOLE;
  initHoles();
  drawStartScreen();
}

void loop() {
  switch (gameState) {

    case START:
      if (buttonPressDebounced()) {
        handleStartInput();
      }
      break;

    // Only redraw hole screen if we didn't just transition to SCORE
    case RUNNING:
      if (buttonPressDebounced()) {
        lastActivityTime = millis();
        performOperation();
        if (gameState == RUNNING) {  // Guard added - skip redraw if score screen was just triggered
          drawHoleScreen();
        }
      }
      if (millis() - lastActivityTime >= IDLE_TIMEOUT_MS) {
        gameState = IDLE;
        drawIdleScreen();
      }
    break;

    case IDLE:
      if (buttonPressDebounced()) {
        lastActivityTime = millis();
        gameState        = RUNNING;
        drawHoleScreen();
      }
      break;

    case SCORE:
      if (buttonPressDebounced()) {
        delay(300);
        setupGame();
      }
      break;
  }
}

bool buttonPressDebounced() {
  if (millis() - lastButtonTime < DEBOUNCE_MS) return false;
  for (byte i = 0; i < 4; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      lastButtonTime = millis();
      newInp = Input(i);
      inp    = newInp;
      return true;
    }
  }
  return false;
}

bool buttonPress() {
  for (byte i = 0; i < 4; i++) {
    if (digitalRead(buttonPins[i]) == LOW) return true;
  }
  return false;
}

void readInput() {
  for (byte i = 0; i < 4; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      newInp = Input(i);
      return;
    }
  }
}

void handleStartInput() {
  switch (inp) {

    // PREV and NEXT now both cycle the selection, ADD confirms it
    case PREV:
    case NEXT:
      // Toggle between 0 (9 holes) and 1 (18 holes)
      currentHole = (currentHole == 0) ? 1 : 0;
      drawStartScreen();
      break;

    case ADD:
      // ADD is now the confirm button on the start screen
      totalHoles       = (currentHole == 0) ? GAME_HOLE_HALF : GAME_HOLE_FULL;
      currentHole      = STARTING_HOLE;
      opt              = PAR;
      gameState        = RUNNING;
      lastActivityTime = millis();
      drawHoleScreen();
      break;

    case CLEAR:
      // No-op on start screen
      break;
  }
}

bool performOperation() {
  uint8_t holeIdx = currentHole - 1;

  switch (inp) {

    case PREV:
      if (opt == STROKE) {
        opt = PAR;
      } else if (opt == PAR) {
        if (currentHole > STARTING_HOLE) {
          currentHole--;
          opt = STROKE;
        }
      }
      break;

    case NEXT:
      if (opt == PAR) {
        opt = STROKE;
      } else if (opt == STROKE) {
        if (currentHole < totalHoles) {
          currentHole++;
          opt = PAR;
        } else {
          gameState = SCORE;
          drawScoreScreen();
          return true;
        }
      }
      break;

    case ADD:
      if (opt == PAR) {
        if (holes[holeIdx].par < MAX_PAR) {
          holes[holeIdx].par++;
        } else {
          holes[holeIdx].par = STARTING_PAR;
        }
      } else if (opt == STROKE) {
        if (holes[holeIdx].strokes < MAX_STROKE) {
          holes[holeIdx].strokes++;
        } else {
          holes[holeIdx].strokes = STARTING_STROKE;
        }
      }
      break;

    case CLEAR:
      if (opt == PAR) {
        holes[holeIdx].par = STARTING_PAR;
      } else if (opt == STROKE) {
        holes[holeIdx].strokes = STARTING_STROKE;
      }
      break;
  }
  return true;
}

void drawStartScreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(20, 2);
  display.print(F("GOLF SCORECARD"));

  display.drawLine(0, 13, SCREEN_WIDTH, 13, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(30, 20);
  display.print(F("9 Holes"));

  display.setCursor(22, 40);
  display.print(F("18 Holes"));

  // Arrow tracks currentHole (0 = 9 holes, 1 = 18 holes)
  uint8_t arrowY = (currentHole == 0) ? 20 : 40;
  display.setCursor(2, arrowY);
  display.print(F(">"));

  display.setTextSize(1);
  display.setCursor(8, 57);
  display.print(F("Press ADD to select"));

  display.display();
}

void drawHoleScreen() {
  uint8_t holeIdx = currentHole - 1;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("Hole: "));
  display.print(currentHole);
  display.print(F("/"));
  display.print(totalHoles);

  uint8_t barWidth = 50;
  uint8_t filled   = (uint8_t)((float)currentHole / totalHoles * barWidth);
  display.drawRect(78, 1, barWidth, 6, SSD1306_WHITE);
  display.fillRect(78, 1, filled, 6, SSD1306_WHITE);

  display.drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(10, 16);
  display.print(F("PAR:   "));
  display.print(holes[holeIdx].par);

  if (opt == PAR) {
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print(F(">"));
  }

  display.setTextSize(2);
  display.setCursor(10, 38);
  display.print(F("STROKE:"));
  display.print(holes[holeIdx].strokes);

  if (opt == STROKE) {
    display.setTextSize(1);
    display.setCursor(0, 42);
    display.print(F(">"));
  }

  display.setTextSize(1);
  display.setCursor(0, 57);
  display.print(F("PREV  ADD+  CLR  NEXT"));

  display.display();
}

void drawIdleScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 28);
  display.print(F("Press any key..."));
  display.display();
}

void drawScoreScreen() {
  uint16_t totalPar    = 0;
  uint16_t totalStroke = 0;

  for (uint8_t i = 0; i < totalHoles; i++) {
    totalPar    += holes[i].par;
    totalStroke += holes[i].strokes;
  }
  int16_t diff = (int16_t)totalStroke - (int16_t)totalPar;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(30, 0);
  display.print(F("FINAL SCORE"));
  display.drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 16);
  display.print(F("PAR     "));
  display.print(totalPar);

  display.setCursor(0, 34);
  display.print(F("STROKES "));
  display.print(totalStroke);

  display.setTextSize(1);
  display.setCursor(0, 51);
  display.print(F("HANDICAP:"));
  if (diff > 0) display.print(F("+"));
  display.print(diff);

  //if (diff == 0)      display.print(F(" (Even!)"));
  //else if (diff < 0)  display.print(F(" (Under par!)"));
  //else if (diff <= 5) display.print(F(" (Over par)"));
  //else                display.print(F(" (Keep at it!)"));

  display.setCursor(100, 57);
  display.print(F("NEXT"));

  display.display();
}

void drawSplashScreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Large title
  display.setTextSize(2);
  display.setCursor(5, 10);
  display.print(F("GOLF BUDDY"));

  // Golf scene -----------------------------------------------------

  // Ground line
  display.drawLine(0, 48, 127, 48, SSD1306_WHITE);

  // Hill / fairway mound
  display.fillTriangle(58, 48, 70, 42, 82, 48, SSD1306_WHITE);

  // Flag pole on top of hill
  display.drawLine(70, 42, 70, 30, SSD1306_WHITE);

  // Flag
  display.fillTriangle(70, 30, 76, 33, 70, 36, SSD1306_WHITE);

  // Golf ball
  display.fillCircle(34, 45, 3, SSD1306_WHITE);

  // -----------------------------------------------------------------

  // Small subtitle
  display.setTextSize(1);
  display.setCursor(30, 56);
  display.print(F("EPG tech. "));
  // Draw a small (c) circle manually after the text
  int x = 90, y = 56;
  display.drawCircle(x, y+3, 4, SSD1306_WHITE); // circle
  display.setCursor(x-2, y);
  display.print(F("P"));  

  display.display();
}