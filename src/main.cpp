#include <Arduino.h>

#include <deque>
#include <ctime>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define I2C_ADDRESS   0x3C

#define SDA_PIN 22
#define SCL_PIN 23

// #define BTNS 15
// int BTNSRead = 0;

#define BTN1 27
#define BTN2 14
#define BTN3 12
#define BTN4 13


Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

byte X = 21;
byte Y = 20;
byte LEN = 20;

enum Direction { GAME_OVER, RIGHT, DOWN, LEFT, UP };

bool contains(std::deque<std::pair<byte, byte>> deque, std::pair<byte, byte> pair) {
  for (std::pair<byte, byte> px : deque)
    if (px == pair)
      return true;
  return false;
}

class Snake {
public:
  byte x, y;
  byte lenghtPrev;
  byte lenght;
  Direction direction = RIGHT; // 1 -> ++x ; 2 -> ++y ; 3 -> --x ; 4 -> --y ; 0 -> dead
  byte speed = 10;
  std::deque<std::pair<byte, byte>> body;
  std::pair<byte, byte> mouse;
  unsigned long lastMoveTime = 0;

  Snake(byte x, byte y, byte lenght) {
    this->x = x;
    this->y = y;
    this->lenght = lenght;
    lenghtPrev = lenght;

    for (byte i = 0; i < lenght; i++) {
      body.push_back({ x - i, y });
    }

    generateMouse(body, mouse);
  }

  void generateMouse(std::deque<std::pair<byte, byte>> body, std::pair<byte, byte>& mouse) {
    do {
      mouse = std::make_pair(std::rand() % SCREEN_WIDTH, std::rand() % SCREEN_HEIGHT);
    } while (contains(body, mouse));
  }

  void start() {
    oled.clearDisplay();
    for (std::pair<byte, byte> px : body) {
      oled.drawPixel(px.first, px.second, SSD1306_WHITE);
    }
    oled.drawPixel(mouse.first, mouse.second, SSD1306_WHITE);
    oled.display();
  }

  void moveTo(byte x, byte y) {
    this->x = (x + SCREEN_WIDTH) % SCREEN_WIDTH;
    this->y = (y + SCREEN_HEIGHT) % SCREEN_HEIGHT;

    if (oled.getPixel(x, y) == SSD1306_WHITE) {
      if (contains(body, std::make_pair(x, y)))
        direction = GAME_OVER;
      if (mouse == std::make_pair(x, y)) {
        lenght++;
        generateMouse(body, mouse);
        oled.drawPixel(mouse.first, mouse.second, SSD1306_WHITE);
      }
    }

    if (lenghtPrev == lenght) {
      oled.drawPixel(body.back().first, body.back().second, SSD1306_BLACK);
      body.pop_back();
    }

    oled.drawPixel(x, y, SSD1306_WHITE);
    body.push_front({ x, y });
    oled.display();

    lenghtPrev = lenght;
  }

  void move() {
    if (millis() - lastMoveTime >= 1000 / speed) {
      lastMoveTime = millis();
      switch (direction) {
      case RIGHT:
        Snake::moveTo(++x, y);
        break;
      case DOWN:
        Snake::moveTo(x, ++y);
        break;
      case LEFT:
        Snake::moveTo(--x, y);
        break;
      case UP:
        Snake::moveTo(x, --y);
        break;
      case GAME_OVER:
        break;
      }
    }
  }
};

Snake snake(X, Y, LEN);

void setup() {
  Serial.begin(115200);
  std::srand(std::time(0));

  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(BTN3, INPUT_PULLUP);
  pinMode(BTN4, INPUT_PULLUP);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!oled.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  snake.start();
}

void readButtons() {
  // BTNSRead = analogRead(BTNS);

  // if (BTNSRead == 4095 && snake.direction != RIGHT && snake.direction != GAME_OVER) snake.direction = LEFT;
  // if (BTNSRead < 3800 && BTNSRead > 3400 && snake.direction != DOWN && snake.direction != GAME_OVER) snake.direction = UP;
  // if (BTNSRead < 3000 && BTNSRead > 2500 && snake.direction != UP && snake.direction != GAME_OVER) snake.direction = DOWN;
  // if (BTNSRead < 2200 && BTNSRead > 1900 && snake.direction != LEFT && snake.direction != GAME_OVER) snake.direction = RIGHT;

  if (digitalRead(BTN1) == LOW && snake.direction != RIGHT && snake.direction != GAME_OVER) snake.direction = LEFT;
  if (digitalRead(BTN2) == LOW && snake.direction != DOWN && snake.direction != GAME_OVER) snake.direction = UP;
  if (digitalRead(BTN3) == LOW && snake.direction != UP && snake.direction != GAME_OVER) snake.direction = DOWN;
  if (digitalRead(BTN4) == LOW && snake.direction != LEFT && snake.direction != GAME_OVER) snake.direction = RIGHT;
}

void loop() {
  snake.move();
  readButtons();
}

