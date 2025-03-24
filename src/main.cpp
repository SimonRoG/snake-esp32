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

#define BTNS 15

int BTNSRead = 0;

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

byte X = 6;
byte Y = 6;
byte LEN = 6;

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
  byte direction = 1; // 1 -> ++x ; 2 -> ++y ; 3 -> --x ; 4 -> --y
  byte speed = 10;
  std::deque<std::pair<byte, byte>> body;
  std::pair<byte, byte> mouse;

  Snake(byte x, byte y, byte lenght) {
    this->x = x;
    this->y = y;
    this->lenght = lenght;
    lenghtPrev = lenght;

    for (byte i = 0; i < lenght; i++) {
      body.push_back({ x - i, y });
    }

    do {
      mouse = std::make_pair(std::rand() % 128, std::rand() % 64);
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
    if (x == 128) this->x = 0;
    if (x == 255) this->x = 127;
    if (y == 64) this->y = 0;
    if (y == 255) this->y = 63;

    if (oled.getPixel(x, y) == SSD1306_WHITE) {
      if (contains(body, std::make_pair(x, y)))
        direction = 69;
      if (mouse == std::make_pair(x, y)) {
        lenght++;
        do {
          mouse = std::make_pair(std::rand() % 128, std::rand() % 64);
        } while (contains(body, mouse));
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
    delay(1000 / speed);
    switch (direction) {
    case 1:
      Snake::moveTo(++x, y);
      break;
    case 2:
      Snake::moveTo(x, ++y);
      break;
    case 3:
      Snake::moveTo(--x, y);
      break;
    case 4:
      Snake::moveTo(x, --y);
      break;
    case 69:
      break;
    }
  }
};

Snake snake(X, Y, LEN);

void setup() {
  Serial.begin(115200);
  std::srand(std::time(0));

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!oled.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  snake.start();
}

void loop() {
  snake.move();
  BTNSRead = analogRead(BTNS);

  if (BTNSRead == 4095 && snake.direction != 1 && snake.direction != 69) snake.direction = 3;
  if (BTNSRead < 3800 && BTNSRead > 3400 && snake.direction != 2 && snake.direction != 69) snake.direction = 4;
  if (BTNSRead < 3000 && BTNSRead > 2500 && snake.direction != 4 && snake.direction != 69) snake.direction = 2;
  if (BTNSRead < 2200 && BTNSRead > 1900 && snake.direction != 3 && snake.direction != 69) snake.direction = 1;
  if (BTNSRead < 1000 && BTNSRead > 600 && snake.direction != 69) snake.lenght++;
}

