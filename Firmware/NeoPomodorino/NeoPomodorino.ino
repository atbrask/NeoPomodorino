#include <Adafruit_NeoPixel.h>

#define BUTTON_PIN     3
#define PIXEL_PIN      6
#define PIXEL_COUNT    24

#define POMODORO_LIMIT 4
#define POMODORO_TIME  25

#define SHORT_PAUSE    5
#define LONG_PAUSE     25

#define LUMINANCE      16

enum State
{
  Off = 0,
  Pomodoro = 1,
  ShortPause = 2,
  LongPause = 3
};

enum ButtonClick
{
  None,
  ShortClick,
  LongClick
};

Adafruit_NeoPixel ring = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

State currentState;
int pomodoro;
unsigned long starttime;
unsigned long duration;
bool buttonState = HIGH;

void setup() 
{
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  ring.begin();
  ring.show();
  
  turnOff();
}

void loop()
{
  unsigned long now = millis();
  ButtonClick button = getButtonClick();

  // Time can be extended by pressing the button. 1 short click => +1 minute.
  if (currentState != Off && button == ShortClick)
    starttime = min(starttime + 60000, now);

  // Long clicks can be used to turn the device on and off
  else if (button == LongClick)
  {
    if (currentState == Off)
      startPomodoro();
    else
      turnOff();
  }

  // If time runs out, go to next phase.
  else
  {
    unsigned long elapsedtime = now - starttime;
    if (elapsedtime > duration)
    {
      if (currentState == Pomodoro)
        startPause();
      else if (currentState == ShortPause || currentState == LongPause)
        startPomodoro();
    }
  }
  
  // Update ring
  updateDisplay();
  delay(20);

}

void turnOff()
{
  pomodoro = 0;
  currentState = Off;
}

void startPomodoro()
{
  pomodoro = (pomodoro % 4) + 1;
  blink((PIXEL_COUNT * pomodoro) / 4);
  starttime = millis();
  duration = POMODORO_TIME * 60000;
  currentState = Pomodoro;
  initLeds();
}

void startPause()
{
  starttime = millis();
  if (pomodoro < 4)
  {
    duration = SHORT_PAUSE * 60000;
    currentState = ShortPause;
  }
  else
  {
    duration = LONG_PAUSE * 60000;
    currentState = LongPause;
  }
  initLeds();
}

ButtonClick getButtonClick()
{
  ButtonClick result = None;
  bool newState = digitalRead(BUTTON_PIN);
  if (newState == LOW && buttonState == HIGH)
  {
    delay(20);
    buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == LOW)
    {
      for(int i = 0; i < 25; i++)
      {
        delay(20);
        buttonState = digitalRead(BUTTON_PIN);
        if (buttonState == HIGH)
          return ShortClick;
      }
      return LongClick;
    }
    else
      return None;
  }
  else
  {
    buttonState = newState;
    return None;
  }
}

void updateDisplay()
{
  float progress = ((float)(millis() - starttime) / duration) * PIXEL_COUNT;
  int done = floor(progress);
  float fraction = progress - done;
  for(int i = 0; i < PIXEL_COUNT; i++)
  {
    uint8_t intensity = 0;
    if (i == done)
      intensity = LUMINANCE * (1 - fraction);
    else if (i > done)
      intensity = LUMINANCE;

    if (currentState == Pomodoro)
      ring.setPixelColor(i, intensity, 0, 0);
    else if (currentState == ShortPause)
      ring.setPixelColor(i, intensity, intensity, 0);
    else if (currentState == LongPause)
      ring.setPixelColor(i, 0, intensity, 0);
    else
      ring.setPixelColor(i, 0, 0, 0);
  }
  ring.show();
}

void blink(int ledcount)
{
  for(int i = 0; i < 3; i++)
  {
    for(int j = 0; j < PIXEL_COUNT; j++)
      ring.setPixelColor(j, 0, 0, j < ledcount ? LUMINANCE : 0);
    ring.show();
    delay(200);
    for(int j = 0; j < PIXEL_COUNT; j++)
      ring.setPixelColor(j, 0, 0, 0);
    ring.show();
    delay(200);
  }
}

void initLeds()
{
  uint32_t color;
  if (currentState == Pomodoro)
    color = ring.Color(LUMINANCE, 0, 0);
  else if (currentState == ShortPause)
    color = ring.Color(LUMINANCE, LUMINANCE, 0);
  else if (currentState == LongPause)
    color = ring.Color(0, LUMINANCE, 0);

  for(int i = PIXEL_COUNT; i > 0; i--)
  {
    ring.setPixelColor(i - 1, color);
    ring.show();
    delay(20);
  }
}


