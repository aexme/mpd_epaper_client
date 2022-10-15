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
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>

#include "WiFiCredentials.h"
#include "BitmapDisplay.h"
#include "TextDisplay.h"

// select the display constructor line in one of the following files (old style):
#include "GxEPD2_display_selection.h"
#include "GxEPD2_display_selection_added.h"

// or select the display class and display driver class in the following file (new style):
#include "GxEPD2_display_selection_new_style.h"

#include <WiFi.h>

struct VersionedChar
{
  char value[50];
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

struct TextBox {
  int x;
  int y;
  int w;
  int h;
  int rows;
  int font;
};

MpdStatus mpdStatus; 
CurrentSong currentSong;

BitmapDisplay bitmaps(display);

// PINS
const int trigPin = 12;
const int echoPin = 14;
const int button1 = 21;
const int button2 = 22;
const int button3 = 32;
const int button4 = 33;

uint16_t port = 6600;
char *host = "192.168.178.49"; // ip or dns

volatile uint16_t delayTime= 500;

WiFiClient client;

char command[20];

int counter = 1;
int fullscreenRefresh = 180000;
unsigned long previousMillis  = 1;

TextBox displayRows[7] = {
  {0, 0, 264, 60, 2, 2}, 
  {0, 62, 264, 40, 2, 1},
  {0, 102, 264, 40, 2, 1},
  {0, 152, 264, 20, 1, 1},
}; 

void setup()
{
  Serial.begin(115200);
  Serial.println("setup");
  
  pinMode(button1,INPUT_PULLUP);
  pinMode(button2,INPUT_PULLUP);
  pinMode(button3,INPUT_PULLUP);
  pinMode(button4,INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(button1), myisr21, CHANGE);  
  attachInterrupt(digitalPinToInterrupt(button2), myisr22, CHANGE);  
  attachInterrupt(digitalPinToInterrupt(button3), myisr32, CHANGE);  
  attachInterrupt(digitalPinToInterrupt(button4), myisr33, CHANGE);  

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

  if (display.epd2.hasPartialUpdate)
  {
    delay(1000);
  }
  
  Serial.println("setup done");
}

void loop()
{  
  if (!client.connected())
  {
    Serial.println("server disconencted");
    delay(10 * 1000);
    ESP.restart();
  }

  // do fullscreen update every 180s
  if(millis() - previousMillis > fullscreenRefresh ){
    //Serial.println("fullScreenUpdate");
    display.refresh();
    previousMillis = millis();
  }

  if(millis() - previousMillis > 500){
    //Serial.println("updateScreen");
    updateScreen();
    previousMillis = millis(); 
  }
  
  if(strlen(command) > 1){

    if(command == "pause" && String(mpdStatus.state.value) != "play"){
      String("play").toCharArray(command, 20);
    }
    mpc_command(command);
    String(" ").toCharArray(command, 20);
  }

  delay(10);
}

void myisr21(){
  static byte state21 = 1;
  static uint32_t lastButtonTime21=0;

  // Filter out too quick buttons = errors.
  if (millis()-lastButtonTime21 > 300) {
    state21 = !state21;
    if (state21) delayTime = 500; else delayTime = 50;
    String("pause").toCharArray(command, 20);

    lastButtonTime21 = millis();
  }
}
void myisr22(){
  static byte state22 = 1;
  static uint32_t lastButtonTime22=0;

  // Filter out too quick buttons = errors.
  if (millis()-lastButtonTime22 > 300) {
    state22 = !state22;
    if (state22) delayTime = 500; else delayTime = 50;
    String("next").toCharArray(command, 20);

    lastButtonTime22 = millis();
  }
}
void myisr32(){
  static byte state32 = 1;
  static uint32_t lastButtonTime32=0;

  // Filter out too quick buttons = errors.
  if (millis()-lastButtonTime32 > 300) {
    state32 = !state32;
    if (state32) delayTime = 500; else delayTime = 50;
    String("previous").toCharArray(command, 20);

    lastButtonTime32 = millis();
  }
}
void myisr33(){
  static byte state33 = 1;
  static uint32_t lastButtonTime33=0;

  // Filter out too quick buttons = errors.
  if (millis()-lastButtonTime33 > 300) {
    state33 = !state33;
    if (state33) delayTime = 500; else delayTime = 50;
    String("previous").toCharArray(command, 20);
    lastButtonTime33 = millis();
  }
}

void updateScreen(){
  getMpdStatus(client, mpdStatus);

  if (String(mpdStatus.state.value) == "play")
  {
    getCurrentSong(client, currentSong);
  }

  doPartialUpdate(currentSong, mpdStatus);
  setAllToUpdatedFalse(currentSong, mpdStatus);
}

int mpc_connect(char *host, int port)
{
  char smsg[40];
  char rmsg[40];

  if (!client.connect(host, port, 1000))
  {
    Serial.println("connection failed");
    return 0;
  }

  Serial.println("connect?");

  String line;
  client.setTimeout(1000);
  line = client.readStringUntil('\n');
  Serial.println(line);

  if(line.indexOf("OK") >=0 ){
    return 1;  
  }
  return 0;
}

int mpc_command(char *buf)
{
  char smsg[40];
  char rmsg[40];
  sprintf(smsg, "%s\n", buf);
  client.print(smsg);
  Serial.println("mpdCommand=[" + String(buf) + "]");

  String line;
  client.setTimeout(1000);
  line = client.readStringUntil('\n');

  if (line.indexOf("OK") != -1)
    return 1;
  return 0;
}

void mpc_error(char *buf)
{
  Serial.print("mpc command error:");
  Serial.println(buf);
}

void getItem(String line, char *item, struct VersionedChar &target)
{
  int len = sizeof(target.value);
  int posOfItem, posOfSeparator, posOfEnd;
  String line2;
  String value;

  posOfItem = line.indexOf(item);

  // item not found
  if(posOfItem <0){
    return;
  }

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

void showPartialUpdate(char *item, int row)
{
  uint16_t box_x = displayRows[row-1].x;
  uint16_t box_y = displayRows[row-1].y;
  uint16_t box_w = displayRows[row-1].w;
  uint16_t box_h = displayRows[row-1].h;
  uint16_t cursor_y = box_y + box_h - 6;
  uint16_t incr = display.epd2.hasFastPartialUpdate ? 1 : 3;
  
  if(displayRows[row-1].font == 1){
    display.setFont(&FreeMono9pt7b);
  }else{
    display.setFont(&FreeMonoBold12pt7b);
  }
  display.setTextColor(GxEPD_BLACK);

  display.setRotation(1);
  display.setPartialWindow(box_x, box_y, box_w, box_h);

  display.firstPage();
  do
  {
    if(displayRows[row-1].rows == 2){
      cursor_y = box_y + (box_h/2) - 6;
      display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
      display.setCursor(box_x, cursor_y);
    }else{
      display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
      display.setCursor(box_x, cursor_y);      
    }
    display.println(item);
    
  } while (display.nextPage());
  delay(200);
  
  display.firstPage();
}

void doPartialUpdate(struct CurrentSong &currentSong, struct MpdStatus &mpdStatus){

  if(currentSong.artist.updated){    
    showPartialUpdate(currentSong.artist.value, 1);
  }

  if(currentSong.name.updated && strlen(currentSong.artist.value)<=1){
    showPartialUpdate(currentSong.name.value, 1);
  }

  if(currentSong.title.updated){
    showPartialUpdate(currentSong.title.value, 2);
  }
  
  if(currentSong.album.updated){
    showPartialUpdate(currentSong.album.value, 3);
  }

  if(mpdStatus.state.updated){
    showPartialUpdate(mpdStatus.state.value, 4);
  }
    
}

void setAllToUpdatedFalse(struct CurrentSong &currentSong, struct MpdStatus &mpdStatus){
  currentSong.name.updated = false;
  currentSong.title.updated = false;
  currentSong.album.updated = false;
  currentSong.artist.updated = false;

  mpdStatus.elapsed.updated = false;
  mpdStatus.duration.updated = false;
  mpdStatus.state.updated = false;
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
  bool foundartist = false;
  bool foundtitle = false;  
  bool foundalbum = false;  
  bool foundname = false;
  
  while (mpd_response != "OK")
  {
    mpd_response = client.readStringUntil('\n');
    //Serial.println("mpd_response=[" + mpd_response + "]");

    if(mpd_response.indexOf("Artist:")>=0){
      foundartist = true;
    }
    if(mpd_response.indexOf("Title:")>=0){
      foundtitle = true;
    }
    if(mpd_response.indexOf("Album:")>=0){
      foundalbum = true;
    }
    if(mpd_response.indexOf("Name:")>=0){
      foundname = true;
    }

    if (!currentSong.artist.updated)
    {
      getItem(mpd_response, "Artist:", currentSong.artist);
    }

    if (!currentSong.title.updated)
    {
      getItem(mpd_response, "Title:", currentSong.title);
    }

    if (!currentSong.name.updated)
    {
      getItem(mpd_response, "Name:", currentSong.name);
    }

    if (!currentSong.album.updated)
    {
      getItem(mpd_response, "Album:", currentSong.album);
    }
  }

  if(!foundartist && strlen(currentSong.artist.value)>0){
    currentSong.artist.updated = true;
    String("").toCharArray(currentSong.artist.value, 46);
  }
  if(!foundname && strlen(currentSong.name.value)>0){
    currentSong.name.updated = true;
    String("").toCharArray(currentSong.name.value, 46);
  }
  if(!foundalbum && strlen(currentSong.album.value)>0){
    currentSong.album.updated = true;
    String("").toCharArray(currentSong.album.value, 46);
  }
  if(!foundtitle && strlen(currentSong.title.value)>0){
    currentSong.title.updated = true;
    String("").toCharArray(currentSong.title.value, 46);
  }
}

void getMpdStatus(WiFiClient client, struct MpdStatus &mpdStatus)
{
  char request[40];

  sprintf(request, "status\n");
  client.print(request);
  client.setTimeout(1000);
  bool foundduration = false;  

  String mpd_response = "";

  while (mpd_response != "OK")
  {
    mpd_response = client.readStringUntil('\n');
    //Serial.println("status_response=[" + mpd_response + "]");

    if(mpd_response.indexOf("duration:")>=0){
      foundduration = true;
    }

    getItem(mpd_response, "state:", mpdStatus.state);
    getItem(mpd_response, "duration:", mpdStatus.duration);
    getItem(mpd_response, "elapsed:", mpdStatus.elapsed);
  }

  if(!foundduration){
    String("0").toCharArray(mpdStatus.duration.value, 46);
  }
}
