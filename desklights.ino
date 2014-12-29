#include <YunServer.h>
#include <YunClient.h>
#include <Process.h>
#include <Mailbox.h>
#include <HttpClient.h>
#include <FileIO.h>
#include <Console.h>
#include <Bridge.h>
// Use if you want to force the software SPI subsystem to be used for some reason (generally, you don't)
// #define FORCE_SOFTWARE_SPI
// Use if you want to force non-accelerated pin access (hint: you really don't, it breaks lots of things)
// #define FORCE_SOFTWARE_SPI
// #define FORCE_SOFTWARE_PINS
#include "FastLED.h"

// Listen on default port 5555, the webserver on the Yun
// will forward there all the HTTP requests for us.
YunServer server;


///////////////////////////////////////////////////////////////////////////////////////////
//
// Move a white dot along the strip of leds.  This program simply shows how to configure the leds,
// and then how to turn a single pixel white and then off, moving down the line of pixels.
//
#define NUM_DESKS 6
#define DESK_LEDS 25

#define BRIGHTNESS  255

typedef struct deskInfo
{
  uint8_t mode;
  CRGB color;
  CRGBPalette16 currentPalette;
  TBlendType    currentBlending = NOBLEND;
};


uint8_t globalMode = 1;
uint8_t globalTempMode = 3;
// set initially to indicate bootup
unsigned long globalTempTime = millis()+10000;

CRGBPalette16 globalPalette = RainbowColors_p;
TBlendType    globalBlending = BLEND;

CRGBPalette16 globalTempPalette = RainbowColors_p;
TBlendType    globalTempBlending = BLEND;


deskInfo DESK[NUM_DESKS];
// How many leds are in the strip?
#define NUM_LEDS (NUM_DESKS*DESK_LEDS)

// Data pin that led data will be written out over
#define DATA_PIN 6

// Clock pin only needed for SPI based chipsets when not using hardware SPI
//#define CLOCK_PIN 8

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];
uint8_t count = 0;

volatile boolean power = true;

// This function sets up the ledsand tells the controller about them
void setup() {

  
  
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Bridge.begin();
  digitalWrite(13, HIGH);
  // sanity check delay - allows reprogramming if accidently blowing power w/leds

  server.listenOnLocalhost();
  server.begin();

  //while (!Console){
  //	; // wait for Console port to connect.
  //	}

  int time = getTimeMins();
  //Console.println("You're connected to the Console!!!!");
  //Console.print("Time=");
  //Console.println(time);
  delay(1000);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);


  FastLED.setBrightness(  BRIGHTNESS );
  
  fill_solid( globalPalette, 16, 1);
}

ISR(TIMER3_COMPA_vect)
{

}

void ledrun() {


  if (count++ == 256) count = 0;


  if (!power)
  {
    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds[i] = 0;
    }
    FastLED.show();
    return;
  }
  boolean useTemp = millis() < globalTempTime;
  uint8_t *currentGlobalMode_ptr = (useTemp) ? &globalTempMode : &globalMode;
  CRGBPalette16 *currentGlobalPalette_ptr = (useTemp) ? &globalTempPalette : &globalPalette;


  boolean processDesk = true;

  if (*currentGlobalMode_ptr == 1)
  {
    for (int i = 0; i < NUM_LEDS; i++)
    {
      //fix blending mode
      leds[i] =  ColorFromPalette(*currentGlobalPalette_ptr, map(i, 0, NUM_LEDS - 1, 0, 255), 255, BLEND);
    }
    processDesk = false;
  } else if (*currentGlobalMode_ptr == 2) {
    leds[map(cubicwave8(count), 0, 255, 0, NUM_LEDS - 1)] =  ColorFromPalette( *currentGlobalPalette_ptr, cubicwave8(count) , 255, BLEND);
    DimmAllGlobal(245);
    processDesk = false;
  }
  else if (*currentGlobalMode_ptr == 3) {
    for (int i = 0; i < NUM_LEDS; i++)
    {
      //fix blending mode
      leds[i] =  ColorFromPalette(*currentGlobalPalette_ptr, map(i, 0, NUM_LEDS - 1, 0, 255), 255, BLEND);

      leds[i].maximizeBrightness(cubicwave8(count * 2));
    }

    processDesk = false;
  }

  if (processDesk)
  {


    for (int i = 0; i < NUM_DESKS; i++)
    {

      if (DESK[i].mode == 0)
      {
        for (int j = 0; j < DESK_LEDS; j++)
        {
          leds[(i * DESK_LEDS) + j] =  ColorFromPalette( DESK[i].currentPalette, map(j, 0, DESK_LEDS - 1, 0, 255), 255, DESK[i].currentBlending);
        }
      }
      else if (DESK[i].mode == 1)
      {
        leds[(i * DESK_LEDS) + cubicwave8(count) / 10] =  ColorFromPalette( DESK[i].currentPalette, map(cubicwave8(count) / 10, 0, DESK_LEDS - 1, 0, 255), 255, DESK[i].currentBlending);
        DimmAll(245, i);
      }
      else if (DESK[i].mode == 2)
      {
        for (int j = 0; j < DESK_LEDS; j++)
        {

          leds[(i * DESK_LEDS) + j] =  ColorFromPalette( DESK[i].currentPalette, map(j, 0, DESK_LEDS - 1, 0, 255), 255, DESK[i].currentBlending);;
          leds[(i * DESK_LEDS) + j].maximizeBrightness(cubicwave8(count * 2));
        }
      }
      else if (DESK[i].mode == 3)
      {
        for (int j = 0; j < DESK_LEDS; j++)
        {
          uint8_t index = map(j, 0, DESK_LEDS - 1, 0, 255) + count;
          leds[(i * DESK_LEDS) + j] =  ColorFromPalette( DESK[i].currentPalette, index, 255, DESK[i].currentBlending);

        }
      }

    }
  }
  FastLED.show();
}
// This function runs over and over, and is where you do the magic to light
// your leds.
void loop() {

  YunClient client = server.accept();

  if (client) {
    process(client);
    client.stop();

  }
  ledrun();

}


void process(YunClient client) {
  String command = client.readStringUntil('/');
  //Console.println("command=" + command);
  if (command == "power") {
    powerCommand(client);
  }
  if (command == "desk") {
    deskCommand(client);
  }
  if (command == "global") {
    globalCommand(client);
  }

}


void powerCommand(YunClient client) {

  String mode = client.readStringUntil('\r');

  if (mode == "on") {
    power = true;
    return;
  } else if (mode == "off") {
    power = false;
  } else {
    client.print(F("error: invalid power mode "));
    client.print(mode);
  }
}

void deskCommand(YunClient client) {

  int deskNum = client.parseInt();

  if (client.read() != '/') {
    client.println(F("error"));
    return;
  }
  String mode = client.readStringUntil('/');
  if (mode == "solid") {

    String color = client.readStringUntil('/');
    DESK[deskNum].mode = 0;
    //Console.println("Solid color " + color);
    long number = strtol( &(color[0]), NULL, 16);
    fill_solid( DESK[deskNum].currentPalette, 16, number);
  }

  if (mode == "rotatePalette") {
    int palette = client.parseInt();
    DESK[deskNum].mode = 3;
    setDeskPalette(deskNum, palette);
  }


  if (mode == "palette") {
    int palette = client.parseInt();
    DESK[deskNum].mode = 0;
    setDeskPalette(deskNum, palette);
  }

  if (mode == "trail") {
    DESK[deskNum].mode = 1;
  }

  if (mode == "phase") {
    DESK[deskNum].mode = 2;
  }


  if (mode == "blend") {
    int blend = client.parseInt();
    switch (blend)
    {
      case 0 :
        DESK[deskNum].currentBlending = NOBLEND;
        break;
      case 1 :
        DESK[deskNum].currentBlending = BLEND;
        break;


    }
  }
}

void globalCommand(YunClient client) {

  String mode = client.readStringUntil('/');

  CRGBPalette16 * palette_ptr = &globalPalette;
  uint8_t * mode_ptr = &globalMode;


  //Console.println(globalMode);
  //Console.println(globalTempMode);

  if (mode == "temp")
  {
    globalTempTime = millis() + (client.parseInt()*1000);
    if (client.read() != '/') {
      client.println(F("error"));
      return;
    }
    mode = client.readStringUntil('/');
    palette_ptr = &globalTempPalette;
    mode_ptr = &globalTempMode;
    //Console.println("new mode=" + mode);
  }
  if (mode == "solid")
  {

    String color = client.readStringUntil('/');
    *mode_ptr = 1;
    //Console.println("Solid color " + color);
    long number = strtol( &(color[0]), NULL, 16);
    fill_solid( *palette_ptr, 16, number);

    //Console.println(globalMode);
    //Console.println(globalTempMode);


  }
  if (mode == "palette")
  {
    *palette_ptr = getPalette(client.parseInt());
    *mode_ptr = 1;
  }
  if (mode == "trail") {
    *mode_ptr = 2;
  }
  if (mode == "phase") {
    *mode_ptr = 3;
  }
  if (mode == "desk") {
    *mode_ptr = 0;
  }
  if (mode == "resetTemp") {
    globalTempTime=millis();
  }

}
void setDeskPalette(uint8_t deskNum, uint8_t palette)
{
  DESK[deskNum].currentPalette = getPalette(palette);
}

// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
const CRGBPalette16 getPalette(uint8_t paletteId)
{

  CRGBPalette16 palette;
  switch (paletteId)
  {
    case 0 :
      palette =  RainbowColors_p;
      break;
    case 1 :
      palette = PartyColors_p;
      break;
    case 2 :
      palette = RainbowStripeColors_p;
      break;
    case 3 :
      palette = CloudColors_p;
      break;
    case 4 :
      palette = OceanColors_p;
      break;
    case 5 :
      palette = ForestColors_p;
      break;

  }
  return palette;
}

void DimmAll(byte value, int desk)
{
  for (int i = 0; i < DESK_LEDS; i++)
  {
    leds[(desk * DESK_LEDS) + i].nscale8(value);
  }
}

void DimmAllGlobal(byte value)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i].nscale8(value);
  }
}

int getTimeMins() {
  String result;
  Process time;
  time.begin("date");
  time.addParameter("+%H:%M");
  time.run();

  while (time.available() > 0) {
    char c = time.read();
    if (c != '\n')
      result += c;
  }
  //Console.println("time result " + result);

  char ** pntr;
  int hour = strtol( &(result[0]), pntr, 10);
  int min = strtol( &(result[0]), pntr, 10);
  return hour + (min * 60);
}
