// Display Library example for SPI e-paper panels from Dalian Good Display and boards from Waveshare.
// Requires HW SPI and Adafruit_GFX. Caution: the e-paper panels require 3.3V supply AND data lines!
//
// Display Library based on Demo Example from Good Display: http://www.e-paper-display.com/download_list/downloadcategoryid=34&isMode=false.html
//
// Author: Jean-Marc Zingg
//
// Version: see library.properties
//
// Library: https://github.com/ZinggJM/GxEPD2
//
// Purpose: show uses of GxEPD2_GFX base class for references to a display instance

// Supporting Arduino Forum Topics:
// Waveshare e-paper displays with SPI: http://forum.arduino.cc/index.php?topic=487007.0
// Good Display ePaper for Arduino: https://forum.arduino.cc/index.php?topic=436411.0

// see GxEPD2_wiring_examples.h for wiring suggestions and examples

// base class GxEPD2_GFX can be used to pass references or pointers to the display instance as parameter, uses ~1.2k more code
// enable GxEPD2_GFX base class
#define ENABLE_GxEPD2_GFX 1
// uncomment next line to use class GFX of library GFX_Root instead of Adafruit_GFX
//#include <GFX.h>
// Note: if you use this with ENABLE_GxEPD2_GFX 1:
//       uncomment it in GxEPD2_GFX.h too, or add #include <GFX.h> before any #include <GxEPD2_GFX.h>
// !!!!  ============================================================================================ !!!!


#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <GxEPD2_7C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBoldOblique12pt7b.h>

#include "WiFiCredentials.h"
#include "BitmapDisplay.h"
#include "TextDisplay.h"

// select the display constructor line in one of the following files (old style):
#include "GxEPD2_display_selection.h"
#include "GxEPD2_display_selection_added.h"

//#include "GxEPD2_display_selection_more.h" // private

// or select the display class and display driver class in the following file (new style):
#include "GxEPD2_display_selection_new_style.h"

#include <WiFi.h>

BitmapDisplay bitmaps(display);
int count = 12;

uint16_t port = 6600;
char *host = "192.168.178.49"; // ip or dns

WiFiClient client;

long lastMillis = 0;
int interval = 0;

struct VersionedChar
{
  char value[40];
  bool updated = false;
};

struct MpdStatus
{
  VersionedChar state;
  VersionedChar duration;
  VersionedChar elapsed;

};

struct CurrentSong
{
  VersionedChar title;
  VersionedChar artist;
  VersionedChar name;
  VersionedChar album;
  VersionedChar time;
};

int mpc_connect(char *host, int port)
{
  char smsg[40];
  char rmsg[40];

  Serial.println("hmm?");
  if (!client.connect(host, port, 1000))
  {
    Serial.println("connection failed");
    return 0;
  }

  Serial.println("connect?");

  String line;
  client.setTimeout(1000);
  line = client.readStringUntil('\n');
  Serial.print("[");
  Serial.print(line);
  Serial.println("]");

  return 1;

  Serial.println("length()=" + String(line.length()));
  line.toCharArray(rmsg, line.length() + 1);
  Serial.println("strlen()=" + String(strlen(rmsg)));
  rmsg[line.length() - 1] = 0;
  Serial.println("rmsg=[" + String(rmsg) + "]");
  if (strncmp(rmsg, "O", 1) == 0)
    return 1;
  return 0;
}

int mpc_command(char *buf)
{
  char smsg[40];
  char rmsg[40];
  sprintf(smsg, "%s\n", buf);
  client.print(smsg);
  Serial.println("smsg=[" + String(buf) + "]");

  String line;
  client.setTimeout(1000);
  line = client.readStringUntil('OK');

  line.toCharArray(rmsg, line.length() + 1);
  //Serial.println("strlen()=" + String(strlen(rmsg)));
  rmsg[line.length() - 1] = 0;
  Serial.println("rmsg=[" + String(rmsg) + "]");
  if (strcmp(rmsg, "OK") == 0)
    return 1;
  return 0;
}

void mpc_error(char *buf)
{
  Serial.print("mpc command error:");
  Serial.println(buf);
  while (1)
  {
  }
}

void getItem(String line, char *item, struct VersionedChar &target)
{
  int len = sizeof(target.value);
  int posOfItem, posOfSeparator, posOfEnd;
  String line2;
  String value;

  posOfItem = line.indexOf(item);
  line2 = line.substring(posOfItem);
  posOfSeparator = line2.indexOf(":");
  posOfEnd = line2.indexOf(0x0a);

  value = line2.substring(posOfSeparator + 2, posOfEnd);
  if (value.length() > len)
  {
    value = value.substring(0, len - 1);
  }

  if(!value.equals(target.value)){
    value.toCharArray(target.value, value.length() + 1);
    target.updated = true;
  }
}

void string2char(String line, char *cstr4, int len)
{
  char cstr3[40];
  line.toCharArray(cstr3, line.length() + 1);
  //Serial.println("cstr3=[" + String(cstr3) + "]");
  int pos4 = 0;
  for (int i = 0; i < strlen(cstr3); i++)
  {
    //if (cstr3[i] == ' ') continue;
    if (cstr3[i] == ' ' && pos4 == 0)
      continue;
    cstr4[pos4++] = cstr3[i];
    cstr4[pos4] = 0;
    if (pos4 == (len - 1))
      break;
  }
  //Serial.println("cstr4=[" + String(cstr4) + "]");
}

void fillBuffer(char *line, int len)
{
  int sz = strlen(line);
  for (int i = sz; i < len; i++)
  {
    line[i] = 0x20;
    line[i + 1] = 0;
  }
}

void lcdDisplay(char *lcdbuf, int rows)
{
  char line[17];
  memset(line, 0, sizeof(line));
  strncpy(line, lcdbuf, 16);
  fillBuffer(line, 16);
  Serial.println("line1=[" + String(line) + "]");

  Serial.println("line2=[" + String(line) + "]");
  Serial.println("line3=[" + String(line) + "]");

  Serial.println("line4=[" + String(line) + "]");
}

void showPartialUpdate(int count)
{
  // some useful background
  //helloWorld(display, count);
  // use asymmetric values for test

  uint16_t box_x = 0;
  uint16_t box_y = 55;
  uint16_t box_w = 140;
  uint16_t box_h = 20;
  uint16_t cursor_y = box_y + box_h - 6;
  float value = 13.95;
  uint16_t incr = display.epd2.hasFastPartialUpdate ? 1 : 3;
  display.setFont(&FreeMonoBoldOblique12pt7b);
  display.setTextColor(GxEPD_BLACK);

  display.setRotation(1);
  display.setPartialWindow(box_x, box_y, box_w, box_h);
  for (uint16_t i = 1; i <= 5; i += incr)
  {
    display.firstPage();
    do
    {
      display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
      display.setCursor(box_x, cursor_y);

      display.print(count * i);
      display.print(" / 3:49");
    } while (display.nextPage());
    delay(500);
  }
  delay(1000);
  display.firstPage();
  /*
  do
  {
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
  }
  while (display.nextPage());
  */
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  delay(100);

  Serial.print("Wait for WiFi...");
  WiFi.begin(SSID, PASSWORD);

  int cnt = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    cnt++;
    if ((cnt % 60) == 0)
      Serial.println();
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  while (1)
  {
    Serial.print("connecting to ");
    Serial.println(host);
    //if (mpc_connect(host, port) == 0) mpc_error("connect");
    if (mpc_connect(host, port) == 1)
      break;
    delay(10 * 1000);
  }

  display.init(115200);

  delay(1000);

  //helloFullScreenPartialMode(display);

  if (display.epd2.hasPartialUpdate)
  {
    //showPartialUpdate();
    delay(1000);
  } // else // on GDEW0154Z04 only full update available, doesn't look nice

  Serial.println("setup done");
}

MpdStatus mpdStatus; 
CurrentSong currentSong;

void loop()
{
  String mpd_response = "";
  char request[40];
  static char oldbuf[80] = {0};

  if (!client.connected())
  {
    Serial.println("server disconencted");
    delay(10 * 1000);
    ESP.restart();
  }
  
  getMpdStatus(client, mpdStatus);

  Serial.println("thestateis=[" + String(mpdStatus.state.value) + "]");

  if (String(mpdStatus.state.value) == "play")
  {

    getCurrentSong(client, currentSong);

    Serial.println("Artist=" + String(currentSong.artist.value));
    Serial.println("Title=" + String(currentSong.title.value));

    int elapsedNumber = String(mpdStatus.elapsed.value).toInt();
    int durationNumber = String(mpdStatus.duration.value).toInt();

    int elapsedS = 0;
    int elapsedM = 0;
    int elapsedH = 0;

    int durationS = 0;
    int durationM = 0;
    int durationH = 0;

    secondsToHMS(elapsedNumber, elapsedH, elapsedM, elapsedS);
    secondsToHMS(durationNumber, durationH, durationM, durationS);

    String timeString = appendZero(elapsedH) + ":" + appendZero(elapsedM) + ":" + appendZero(elapsedS) + "/" + appendZero(durationH) + ":" + appendZero(durationM) + ":" + appendZero(durationS);

    char timeChar[19];
    timeString.toCharArray(timeChar, 19);

    if (strlen(currentSong.artist.value) > 0)
    {
      displayInfo(display, currentSong.artist.value, currentSong.title.value, currentSong.album.value, timeChar);
    }
    else
    {
      displayInfo(display, currentSong.name.value, currentSong.title.value, currentSong.album.value, timeChar);
    }

    currentSong.name.updated = false;
    currentSong.title.updated = false;
    currentSong.album.updated = false;
    currentSong.artist.updated = false;

    mpdStatus.elapsed.updated = false;
    mpdStatus.duration.updated = false;
    mpdStatus.state.updated = false;
  }
  else
  { // state = stop
    Serial.println("status != play");
  }

  //showPartialUpdate(count);
  delay(2000);
}

String appendZero(int value)
{
  char response[2];

  sprintf(response, "%02d", value);
  return String(response);
}

void getCurrentSong(WiFiClient client, struct CurrentSong &currentSong)
{
  char request[40];

  sprintf(request, "currentsong\n");
  client.print(request);
  client.setTimeout(1000);

  String mpd_response = "";

  while (mpd_response != "OK")
  {
    mpd_response = client.readStringUntil('\n');
    Serial.println("mpd_response=[" + mpd_response + "]");

    if (strlen(currentSong.artist.value) == 0 || !currentSong.artist.updated)
    {
      getItem(mpd_response, "Artist:", currentSong.artist);
    }

    if (strlen(currentSong.title.value) == 0 || !currentSong.title.updated)
    {
      getItem(mpd_response, "Title:", currentSong.title);
    }

    if (strlen(currentSong.name.value) == 0 || !currentSong.name.updated)
    {
      getItem(mpd_response, "Name:", currentSong.name);
    }

    if (strlen(currentSong.album.value) == 0 || !currentSong.album.updated)
    {
      getItem(mpd_response, "Album:", currentSong.album);
    }
  }
}


void getMpdStatus(WiFiClient client, struct MpdStatus &mpdStatus)
{
  char request[40];

  sprintf(request, "status\n");
  client.print(request);
  client.setTimeout(1000);

  String mpd_response = "";

  while (mpd_response != "OK")
  {
    mpd_response = client.readStringUntil('\n');
    Serial.println("status_response=[" + mpd_response + "]");
    if (strlen(mpdStatus.state.value) == 0 || !mpdStatus.state.updated)
    {
      getItem(mpd_response, "state:", mpdStatus.state);
    }
    
    if (strlen(mpdStatus.duration.value) == 0 || !mpdStatus.duration.updated)
    {
      getItem(mpd_response, "duration:", mpdStatus.duration);
    }

    if (strlen(mpdStatus.elapsed.value) == 0 || !mpdStatus.elapsed.updated)
    {
      getItem(mpd_response, "elapsed:", mpdStatus.elapsed);
    }
  }
}

void secondsToHMS(const int seconds, int &h, int &m, int &s)
{
  uint32_t t = seconds;
  s = t % 60;
  t = (t - s) / 60;
  m = t % 60;
  t = (t - m) / 60;
  h = t;
}
