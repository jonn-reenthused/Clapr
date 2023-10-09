#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"
#include <stdlib.h>
#include "WiFi.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <LEDMatrixDriver.hpp>

// Set to WEMOS D1 MINI ESP32

const float claprVersion = 1.00;

// Pins
const int rollplus = 4;
const int rollminus = 0;
const int sceneplus = 17;
const int sceneminus = 16;
const int takeplus = 33;
const int takeminus = 32;
const int refresh = 35;
const int clapper = 34;
const int activityLed = 22;
const int refreshLed = 21;

int  rollPlusLastState = HIGH;
int  rollMinusLastState = HIGH;
int  scenePlusLastState = HIGH;
int  sceneMinusLastState = HIGH;
int  takePlusLastState = HIGH;
int  takeMinusLastState = HIGH;
int  refreshLastState = HIGH;
int  clapperLastState = HIGH;
bool activityLedActive = false;
bool refreshLedActive = false;
bool refreshNeeded = false;

unsigned long activityLedMillis = millis();
unsigned long refreshLedMillis = millis();

const uint8_t LEDMATRIX_CS_PIN = 5;

// LED SEGMENT
// Number of 8x8 segments you are connecting
const int LEDMATRIX_SEGMENTS = 8;
const int LEDMATRIX_WIDTH    = LEDMATRIX_SEGMENTS * 8;
const int ANIM_DELAY = 30;

unsigned long setupMillis = millis();
LEDMatrixDriver lmd(LEDMATRIX_SEGMENTS, LEDMATRIX_CS_PIN);

// WIFI
const char* ssid     = "clapr";
const char* password = "123456789";
String HTTP_req;

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// APP

int rollpartition = (EPD_3IN7_HEIGHT / 5);
int takepartition = (EPD_3IN7_HEIGHT - (EPD_3IN7_HEIGHT / 5));

// Clapperboard Data
String productionName = "LOST AND FOUND";
int rollNumber = 33;
int sceneNumber = 8;
int takeNumber = 7;
String directorName = "ERICA MILLER";
String cameraId = "1";
String dateText = "2022-07-03";
int timeOfDay = 0;
int location = 0;
bool mos = false;
bool sync = false;
bool filter = false;

// This is the font definition. You can use http://gurgleapps.com/tools/matrix to create your own font or sprites.
// If you like the font feel free to use it. I created it myself and donate it to the public domain.
byte font[95][8] = { {0,0,0,0,0,0,0,0}, // SPACE
                     {0x10,0x18,0x18,0x18,0x18,0x00,0x18,0x18}, // EXCL
                     {0x28,0x28,0x08,0x00,0x00,0x00,0x00,0x00}, // QUOT
                     {0x00,0x0a,0x7f,0x14,0x28,0xfe,0x50,0x00}, // #
                     {0x10,0x38,0x54,0x70,0x1c,0x54,0x38,0x10}, // $
                     {0x00,0x60,0x66,0x08,0x10,0x66,0x06,0x00}, // %
                     {0,0,0,0,0,0,0,0}, // &
                     {0x00,0x10,0x18,0x18,0x08,0x00,0x00,0x00}, // '
                     {0x02,0x04,0x08,0x08,0x08,0x08,0x08,0x04}, // (
                     {0x40,0x20,0x10,0x10,0x10,0x10,0x10,0x20}, // )
                     {0x00,0x10,0x54,0x38,0x10,0x38,0x54,0x10}, // *
                     {0x00,0x08,0x08,0x08,0x7f,0x08,0x08,0x08}, // +
                     {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x08}, // COMMA
                     {0x00,0x00,0x00,0x00,0x7e,0x00,0x00,0x00}, // -
                     {0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x06}, // DOT
                     {0x00,0x04,0x04,0x08,0x10,0x20,0x40,0x40}, // /
                     {0x3c,0x42,0x46,0x4a,0x52,0x62,0x42,0x3c}, // 0
                     {0x08,0x18,0x38,0x68,0x08,0x08,0x08,0x7e}, // 1
                     {0x3c,0x42,0x02,0x02,0x3e,0x40,0x40,0x7e}, // 2
                     {0x3c,0x42,0x02,0x1c,0x02,0x02,0x42,0x3c}, // 3
                     {0x04,0x0c,0x14,0x24,0x7e,0x04,0x04,0x04}, // 4
                     {0x7e,0x40,0x40,0x7c,0x02,0x02,0x02,0x7c}, // 5
                     {0x3c,0x42,0x40,0x7c,0x42,0x42,0x42,0x3c}, // 6
                     {0x7c,0x02,0x02,0x02,0x04,0x04,0x08,0x08}, // 7
                     {0x3c,0x42,0x42,0x3c,0x42,0x42,0x42,0x3c}, // 8
                     {0x3c,0x42,0x42,0x3e,0x02,0x02,0x42,0x3c}, // 9
                     {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00}, // :
                     {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x08}, // ;
                     {0x00,0x10,0x20,0x40,0x80,0x40,0x20,0x10}, // <
                     {0x00,0x00,0x7e,0x00,0x00,0xfc,0x00,0x00}, // =
                     {0x00,0x08,0x04,0x02,0x01,0x02,0x04,0x08}, // >
                     {0x00,0x38,0x44,0x04,0x08,0x10,0x00,0x10}, // ?
                     {0x00,0x30,0x48,0xba,0xba,0x84,0x78,0x00}, // @
                     {0x00,0x1c,0x22,0x42,0x42,0x7e,0x42,0x42}, // A
                     {0x00,0x78,0x44,0x44,0x78,0x44,0x44,0x7c}, // B
                     {0x00,0x3c,0x44,0x40,0x40,0x40,0x44,0x7c}, // C
                     {0x00,0x7c,0x42,0x42,0x42,0x42,0x44,0x78}, // D
                     {0x00,0x78,0x40,0x40,0x70,0x40,0x40,0x7c}, // E
                     {0x00,0x7c,0x40,0x40,0x78,0x40,0x40,0x40}, // F
                     {0x00,0x3c,0x40,0x40,0x5c,0x44,0x44,0x78}, // G
                     {0x00,0x42,0x42,0x42,0x7e,0x42,0x42,0x42}, // H
                     {0x00,0x7c,0x10,0x10,0x10,0x10,0x10,0x7e}, // I
                     {0x00,0x7e,0x02,0x02,0x02,0x02,0x04,0x38}, // J
                     {0x00,0x44,0x48,0x50,0x60,0x50,0x48,0x44}, // K
                     {0x00,0x40,0x40,0x40,0x40,0x40,0x40,0x7c}, // L
                     {0x00,0x82,0xc6,0xaa,0x92,0x82,0x82,0x82}, // M
                     {0x00,0x42,0x42,0x62,0x52,0x4a,0x46,0x42}, // N
                     {0x00,0x3c,0x42,0x42,0x42,0x42,0x44,0x38}, // O
                     {0x00,0x78,0x44,0x44,0x48,0x70,0x40,0x40}, // P
                     {0x00,0x3c,0x42,0x42,0x52,0x4a,0x44,0x3a}, // Q
                     {0x00,0x78,0x44,0x44,0x78,0x50,0x48,0x44}, // R
                     {0x00,0x38,0x40,0x40,0x38,0x04,0x04,0x78}, // S
                     {0x00,0x7e,0x90,0x10,0x10,0x10,0x10,0x10}, // T
                     {0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x3e}, // U
                     {0x00,0x42,0x42,0x42,0x42,0x44,0x28,0x10}, // V
                     {0x80,0x82,0x82,0x92,0x92,0x92,0x94,0x78}, // W
                     {0x00,0x42,0x42,0x24,0x18,0x24,0x42,0x42}, // X
                     {0x00,0x44,0x44,0x28,0x10,0x10,0x10,0x10}, // Y
                     {0x00,0x7c,0x04,0x08,0x7c,0x20,0x40,0xfe}, // Z
                      // (the font does not contain any lower case letters. you can add your own.)
                  };    // {}, //

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    DEV_Module_Init();

    // Setup Pins
    pinMode(rollplus, INPUT_PULLUP);
    pinMode(rollminus, INPUT_PULLUP);
    pinMode(sceneplus, INPUT_PULLUP);
    pinMode(sceneminus, INPUT_PULLUP);
    pinMode(takeplus, INPUT_PULLUP);
    pinMode(takeminus, INPUT_PULLUP);
    pinMode(refresh, INPUT_PULLUP);
    pinMode(clapper, INPUT_PULLUP);
    pinMode(activityLed, OUTPUT);
    
    printf("clapr ====\r\n");

    // Connect to Wi-Fi network with SSID and password
    Serial.print("Setting AP (Access Point)…");

    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    server.begin();
        
    setupstatic();

    lmd.setEnabled(true);
    lmd.setIntensity(2);   // 0 = low, 10 = high
}

void loadData() {
  
}

void checkButtons() {
    /*pinMode(rollplus, INPUT);
    pinMode(rollminus, INPUT);
    pinMode(sceneplus, INPUT);
    pinMode(sceneminus, INPUT);
    pinMode(takeplus, INPUT);
    pinMode(takeminus, INPUT);
    pinMode(refresh, INPUT);
    pinMode(clapper, INPUT);*/

    bool activateLED = false;

    int refreshState = digitalRead(refresh);
    int clapperState = digitalRead(clapper);
    int takePlusState = digitalRead(takeplus);
    int takeMinusState = digitalRead(takeminus);
    int scenePlusState = digitalRead(sceneplus);
    int sceneMinusState = digitalRead(sceneminus);
    int rollPlusState = digitalRead(rollplus);
    int rollMinusState = digitalRead(rollminus);

    Serial.print("Clapper State: " + String((clapperState == LOW ? "LOW" : "HIGH")) + "\n");
    Serial.print("Refresh State: " + String((refreshState == LOW ? "LOW" : "HIGH")) + "\n");

    if (refreshLastState == HIGH && refreshState == LOW) {
      activateLED=true;
      refreshNeeded=false;
      changeRefreshLedState(LOW);
      setupstatic();
    }

    if (clapperLastState == HIGH && clapperState == LOW) {
      activateLED=true;
      refreshNeeded=true;
      activateTimer();
    }

    if (takePlusLastState == HIGH && takePlusState == LOW) {
      activateLED=true;
      refreshNeeded=true;
      takeNumber+=1;
    }

    if (takeMinusLastState == HIGH && takeMinusState == LOW) {
      activateLED=true;
      refreshNeeded=true;
      if (takeNumber > 0) takeNumber-=1;
    }

    if (scenePlusLastState == HIGH && scenePlusState == LOW) {
      activateLED=true;
      refreshNeeded=true;
      sceneNumber+=1;
    }

    if (sceneMinusLastState == HIGH && sceneMinusState == LOW) {
      activateLED=true;
      refreshNeeded=true;
      if (sceneNumber > 1) sceneNumber-=1;
    }

    if (rollPlusLastState == HIGH && rollPlusState == LOW) {
      activateLED=true;
      refreshNeeded=true;
      rollNumber+=1;
    }

    if (rollMinusLastState == HIGH && rollMinusState == LOW) {
      activateLED=true;
      refreshNeeded=true;
      if (rollNumber > 0) rollNumber-=1;
    }

    clapperLastState = clapperState;
    refreshLastState = refreshState;
    takePlusLastState = takePlusState;
    takeMinusLastState = takeMinusState;
    scenePlusLastState = scenePlusState;
    sceneMinusLastState = sceneMinusState;
    rollPlusLastState = rollPlusState;
    rollMinusLastState = rollMinusState;

    if (activateLED) {
      activityLedMillis = millis();
      digitalWrite(activityLed,HIGH);
      activityLedActive=true;
    }

    if (refreshNeeded) {
      refreshLedMillis = millis();
      changeRefreshLedState(HIGH);
    }
}

void checkActivityLed() {
  if (activityLedActive) {
    if (millis() - activityLedMillis > 500) {
      digitalWrite(activityLed, LOW);
      activityLedActive = false;
    }
  }

  if (refreshNeeded) {
    if (millis() - refreshLedMillis > 300) {
      if (refreshLedActive) {
        changeRefreshLedState(LOW);
      } else {
        changeRefreshLedState(HIGH);
      }
    }
  }
}

void changeRefreshLedState(int newState) {
  if (newState == LOW) {
    refreshLedActive = false;
  } else {
    refreshLedActive = true;
  }

  digitalWrite(refreshLed, newState);
}

void reloadDisplay() {
    loadData();

    int width = Paint_StringWidth(String(rollNumber).c_str(), &Font24);
    int height = Paint_StringHeight(&Font24);

    Paint_DrawString_EN(90, 2, productionName.c_str(), &Font24, WHITE, BLACK);
    Paint_DrawString_EN((rollpartition - width) / 2, 75, String(rollNumber).c_str(), &Font24, WHITE, BLACK);

    width = Paint_StringWidth(String(sceneNumber).c_str(), &Font24);
    Paint_DrawString_EN((EPD_3IN7_HEIGHT - width) / 2, 75, String(sceneNumber).c_str(), &Font24, WHITE, BLACK);

    width = Paint_StringWidth(String(takeNumber).c_str(), &Font24);

    int takeposition = takepartition + (((EPD_3IN7_HEIGHT - takepartition) - width) / 2);
    Paint_DrawString_EN(takeposition, 75, String(takeNumber).c_str(), &Font24, WHITE, BLACK);

    Paint_DrawString_EN(160, 146, directorName.c_str(), &Font24, WHITE, BLACK);
    Paint_DrawString_EN(160, 186, cameraId.c_str(), &Font24, WHITE, BLACK);
    
    width = Paint_StringWidth(dateText.c_str(), &Font24);
    Paint_DrawString_EN(2, EPD_3IN7_WIDTH - height, dateText.c_str(), &Font24, WHITE, BLACK);

    Paint_DrawString_EN(118, 228, "DAY", &Font24, timeOfDay == 0 ? BLACK : WHITE,timeOfDay == 0 ? WHITE : BLACK);
    Paint_DrawString_EN(190, 228, "NIGHT", &Font24, timeOfDay == 1 ? BLACK : WHITE,timeOfDay == 1 ? WHITE : BLACK);
    Paint_DrawString_EN(285, 228, "INT", &Font24, location == 0 ? BLACK : WHITE,location == 0 ? WHITE : BLACK);
    Paint_DrawString_EN(355, 228, "EXT", &Font24, location == 1 ? BLACK : WHITE,location == 1 ? WHITE : BLACK);

    width = Paint_StringWidth("Mos", &Font24);
    Paint_DrawString_EN(EPD_3IN7_HEIGHT - width, 228, "Mos", &Font24, mos ? BLACK : WHITE, mos ? WHITE : BLACK);
    width = Paint_StringWidth("Filter", &Font24);
    Paint_DrawString_EN(180, EPD_3IN7_WIDTH - height, "Filter", &Font24, filter ? BLACK : WHITE, filter ? WHITE : BLACK);

    width = Paint_StringWidth("Sync", &Font24);
    Paint_DrawString_EN(EPD_3IN7_HEIGHT - width, EPD_3IN7_WIDTH - height, "Sync", &Font24, sync ? BLACK : WHITE, filter ? WHITE : BLACK);
}

void setupstatic()
{  
    printf("e-Paper Init and Clear...\r\n");
    EPD_3IN7_4Gray_Init();
    EPD_3IN7_4Gray_Clear();
    DEV_Delay_ms(100);

    UBYTE *BlackImage;
    
    UWORD Imagesize = ((EPD_3IN7_WIDTH % 4 == 0)? (EPD_3IN7_WIDTH / 4 ): (EPD_3IN7_WIDTH / 4 + 1)) * EPD_3IN7_HEIGHT;
    if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        while(1);
    }

    printf("Paint_NewImage\r\n");
    Paint_NewImage(BlackImage, EPD_3IN7_WIDTH, EPD_3IN7_HEIGHT, 270, WHITE);
    Paint_SetScale(4);
    Paint_Clear(WHITE);
  
    Paint_DrawRectangle(0,40,EPD_3IN7_HEIGHT,42, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(rollpartition,42,rollpartition + 2, 142, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(takepartition,42,takepartition + 2, 142, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);

    Paint_DrawRectangle(0,142, EPD_3IN7_HEIGHT, 144, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(0,224, EPD_3IN7_HEIGHT, 226, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);

    //  Static Text
    Paint_DrawString_EN(0, 2, "PROD.", &Font24, WHITE, BLACK);
    //
    int width = Paint_StringWidth("ROLL", &Font24);
    int height = Paint_StringHeight(&Font24);
    
    Paint_DrawString_EN((rollpartition - width) / 2, 44, "ROLL", &Font24, WHITE, BLACK);

    width = Paint_StringWidth("SCENE", &Font24);    
    Paint_DrawString_EN((EPD_3IN7_HEIGHT - width) / 2, 44, "SCENE", &Font24, WHITE, BLACK);

    width = Paint_StringWidth("TAKE", &Font24);
    int takeposition = takepartition + (((EPD_3IN7_HEIGHT - takepartition) - width) / 2);
    Paint_DrawString_EN(takeposition, 44, "TAKE", &Font24, WHITE, BLACK);
    //
    Paint_DrawString_EN(2, 146, "DIRECTOR:", &Font24, WHITE, BLACK);
    Paint_DrawString_EN(2, 186, "CAMERA:", &Font24, WHITE, BLACK);
    //
    Paint_DrawString_EN(2, 228, "DATE:", &Font24, WHITE, BLACK);

    Paint_DrawString_EN(170, 228, "/", &Font24, WHITE, BLACK);
    Paint_DrawString_EN(335, 228, "/", &Font24, WHITE, BLACK);

    reloadDisplay();

    EPD_3IN7_4Gray_Display(BlackImage);
    DEV_Delay_ms(4000);
    free(BlackImage);
    BlackImage = NULL;
    
    // Sleep & close 5V
    printf("Goto Sleep...\r\n");
    EPD_3IN7_Sleep();

    printf("close 5V, Module enters 0 power consumption ...\r\n");  

}

// Functions

String GenerateWebData() {
  String webStr = "<!DOCTYPE html><html>";
  webStr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  webStr += "<link rel=\"icon\" href=\"data:,\">";
  // CSS to style the on/off buttons 
  // Feel free to change the background-color and font-size attributes to fit your preferences
  webStr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}";
  webStr += ".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;";
  webStr += "text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}";
  webStr += ".button2 {background-color: #555555;}</style></head>";
  
  // Web Page Heading
  webStr += "<body><h1>clapr</h1>";
  webStr += "<p></p>";
  //webStr += "<form action=\"/Change\" method=\"post\">";
  webStr += "<label for='pname'>Production Name:</label><br>";
  webStr += "<input type='text' id='pname' name='pname' value='" + productionName + "'>";
  webStr += "<button type='button' onclick='ArduinoTextData(\"pname\")'>Update</button><br>";
  webStr += "<label for='dname'>Director Name:</label><br>";
  webStr += "<input type='text' id='dname' name='dname' value='" + directorName + "'>";
  webStr += "<button type='button' onclick='ArduinoTextData(\"dname\")'>Update</button><br>";
  webStr += "<label for='date'>Date:</label><br>";
  webStr += "<input type='date' id='date' name='date' value='" + dateText + "'>";
  webStr += "<button type='button' onclick='ArduinoTextData(\"date\")'>Update</button><br>";
  webStr += "<label for='camera'>Camera:</label><br>";
  webStr += "<input type='text' id='camera' name='camera' value='" + cameraId + "'>";
  webStr += "<button type='button' onclick='ArduinoTextData(\"camera\")'>Update</button><br>";
  webStr += "<label for='roll'>Roll:</label><br>";
  webStr += "<input type='number' id='roll' name='roll' value='" + String(rollNumber) + "'>";
  webStr += "<button type='button' onclick='FieldDecrease(\"roll\")'>-</button>";
  webStr += "<button type='button' onclick='FieldIncrease(\"roll\")'>+</button>";
  webStr += "<button type='button' onclick='ArduinoTextData(\"roll\")'>Update</button><br>";
  webStr += "<label for='scene'>Scene:</label><br>";
  webStr += "<input type='number' id='scene' name='scene' value='" + String(sceneNumber) + "'>";
  webStr += "<button type='button' onclick='FieldDecrease(\"scene\")'>-</button>";
  webStr += "<button type='button' onclick='FieldIncrease(\"scene\")'>+</button>";
  webStr += "<button type='button' onclick='ArduinoTextData(\"scene\")'>Update</button><br>";
  webStr += "<label for='take'>Take:</label><br>";
  webStr += "<input type='number' id='take' name='take' value='" + String(takeNumber) + "'>";
  webStr += "<button type='button' onclick='FieldDecrease(\"take\")'>-</button>";
  webStr += "<button type='button' onclick='FieldIncrease(\"take\")'>+</button>";
  webStr += "<button type='button' onclick='ArduinoTextData(\"take\")'>Update</button><br>";
  
  webStr += "<label for='intloc'>Location</label><br>";
  webStr += "<label for='intloc'>INT: </label>";
  webStr += "<input type='radio' id='intloc' value='int' name='location'" + String((location == 0 ? " checked" : "")) + " onclick='ArduinoData(\"intloc\")'>";            
  webStr += "<label for='extloc'>  EXT: </label>";
  webStr += "<input type='radio' id='extloc' value='ext' name='location'" + String((location == 1 ? " checked" : "")) + " onclick='ArduinoData(\"extloc\")'><br>";            
  
  webStr += "<label for='day'>Time of Day</label><br>";
  webStr += "<label for='day'>Day</label>: ";
  webStr += "<input type='radio' id='day' value='day' name='timeOfDay'" + String((timeOfDay == 0 ? " checked" : "")) + " onclick='ArduinoData(\"day\")'>";            
  webStr += "<label for='day'>  Night</label>: ";
  webStr += "<input type='radio' id='night' value='night' name='timeOfDay'" + String((timeOfDay == 1 ? " checked" : "")) + " onclick='ArduinoData(\"night\")'><br>";            
  
  webStr += "<label for='mos'>Mos:</label>";
  webStr += "<input type='checkbox' id='mos' name='mos'" + String((mos ? " checked" : "")) + " onclick='ArduinoData(\"mos\")'><br>";  
              
  webStr += "<label for='sync'>Sync:</label>";
  webStr += "<input type='checkbox' id='sync' name='sync'" + String((sync ? " checked" : "")) + " onclick='ArduinoData(\"sync\")'><br>";
  
  webStr += "<label for='filter'>Filter:</label>";
  webStr += "<input type='checkbox' id='filter' name='filter'" + String((filter ? " checked" : "")) + " onclick='ArduinoData(\"filter\")'><br>";
  //webStr += "</form>";
  
  webStr += "</body>";
  webStr += "  <SCRIPT>";
  webStr += "   function ArduinoData(SendData) {";
  webStr += "     var CBval = 0;";
  webStr += "     if (document.getElementById(SendData).checked) {";
  webStr += "       CBval = 1;";
  webStr += "     } else {";
  webStr += "       CBval = 0;";
  webStr += "     };";
  webStr += "     var NuData = 'INPUTDATA:' + SendData + '=' + CBval;";
  webStr += "     var xhttp = new XMLHttpRequest();";
  webStr += "     xhttp.open('POST', 'Change', true);";
  webStr += "     xhttp.setRequestHeader(\"Content-type\", \"application/xml\");";
  webStr += "     xhttp.send(NuData);";
  webStr += "   };";
  webStr += "   function ArduinoTextData(SendData) {";
  webStr += "     var CBval = document.getElementById(SendData).value;";
  webStr += "     var NuData = 'INPUTDATA:' + SendData + '=' + CBval;";
  webStr += "     var xhttp = new XMLHttpRequest();";
  webStr += "     xhttp.open('POST', 'Change', true);";
  webStr += "     xhttp.setRequestHeader(\"Content-type\", \"application/xml\");";
  webStr += "     xhttp.send(NuData);";
  webStr += "   };";
  webStr += "   function FieldIncrease(SendData) {";
  webStr += "     var field = document.getElementById(SendData);";
  webStr += "     var CBval = Number(field.value) + 1;";
  webStr += "     field.value = CBval.toString();";
  webStr += "   };";
  webStr += "   function FieldDecrease(SendData) {";
  webStr += "     var field = document.getElementById(SendData);";
  webStr += "     var CBval = Number(field.value) - 1;";
  webStr += "     if (CBval > 0) field.value = CBval.toString();";
  webStr += "   };";
  webStr += " </SCRIPT>";
  webStr += "</html>";

  return webStr;
}


/* LED TIMER */

void activateTimer()
{
  unsigned long currentMillis = millis() - setupMillis;
  unsigned long seconds = currentMillis / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;
  currentMillis %= 1000;
  seconds %= 60;
  minutes %= 60;
  hours %= 24;

  String text;
  
  text = "";
  text = String(hours < 10 ? "0" : "") + String(hours) + "";
  text += String(minutes < 10 ? "0" : "") + String(minutes) + "";
  text += String(seconds < 10 ? "0" : "") + String(seconds) + "";
  text += String(currentMillis < 10 ? "0" : "") + String(currentMillis);
  
  // Draw the text to the current position
  int len = text.length();
  drawString((char *)text.c_str(), len, 1, 0);
  // In case you wonder why we don't have to call lmd.clear() in every loop: The font has a opaque (black) background...

  // Toggle display of the new framebuffer
  lmd.display();

}

/**
 * This function draws a string of the given length to the given position.
 */
void drawString(char* text, int len, int x, int y )
{
  int digitCount = 0;
  
  for( int idx = 0; idx < len; idx ++ )
  {
    int c = text[idx] - 32;

    int counterStop = 8;

    if (c == 26)
      counterStop = 8;

    // stop if char is outside visible area
    if( x + idx * counterStop  > LEDMATRIX_WIDTH )
      return;

    // only draw if char is visible
    if( counterStop + x + idx * counterStop > 0 )
      drawSprite( font[c], x + idx * counterStop, y, counterStop, 8 );

    digitCount++;
    if (digitCount == 2) {
      if (idx < (len - 1)) {
        lmd.setPixel((x + idx * counterStop) + 7, 2, true);
        lmd.setPixel((x + idx * counterStop) + 7, 5, true);
      }
      digitCount = 0;
      x += 1;
    } else if (digitCount == 1) {
      x -= 1;
    }
  }
}

/**
 * This draws a sprite to the given position using the width and height supplied (usually 8x8)
 */
void drawSprite( byte* sprite, int x, int y, int width, int height )
{
  // The mask is used to get the column bit from the sprite row
  byte mask = B1000000;

  for( int iy = 0; iy < height; iy++ )
  {
    for( int ix = 0; ix < width; ix++ )
    {
      lmd.setPixel(x + ix, y + iy, (bool)(sprite[iy] & mask ));

      // shift the mask by one pixel to the right
      mask = mask >> 1;
    }

    // reset column mask
    mask = B1000000;
  }
}

/* The main loop -------------------------------------------------------------*/

void loop() {
  // Listen for incoming clients
  WiFiClient client = server.available();
  int ind1;
  String CT;
  String Module_x;
  String Val_x;
  String Colour_x;

  checkActivityLed();
  checkButtons();
  
  // Got a client?
  if (client) {
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      
      // Client data available to read 
      if (client.available()) {

        // Read one byte (character) from client
        char c = client.read(); 

        // Save the HTTP request to a string
        HTTP_req += c;
        
        // When this becomes false, the client has sent their last byte
        if (!client.available()) {

          // When first loading the web page, will be a GET request
          if (HTTP_req.indexOf("GET /") > -1) {

            // Web Page request
            // Send a standard HTTP response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();

            client.println(GenerateWebData());
          } // End; if (HTTP_req.indexOf("GET / HTTP/1.1") > -1)

          // When a checkbox state is changed, or text entered, POST request
          if (HTTP_req.indexOf("POST /Change") > -1) {

            // Data sent by POST in xhttp looks like this: INPUTDATA:M10R=255  
            // Look in the POST body, find out what is being sent, starting at the identifier 'INPUTDATA:'
            ind1 = HTTP_req.lastIndexOf(':');

            String token=HTTP_req.substring(ind1+1);
            String key=token.substring(0,token.indexOf('='));
            String value=token.substring(token.indexOf('=')+1);

            Serial.print("Key: " + key + " : Value: " + value);

            bool hasChange = false;

            if (key.equals("pname")) {
              if (!value.equals(productionName)) {
                hasChange=true;
              }

              productionName=value;
            }

            if (key.equals("dname")) {
              if (!value.equals(directorName)) {
                hasChange=true;
              }

              directorName=value;
            }

            if (key.equals("date")) {
              if (!value.equals(dateText)) {
                hasChange=true;
              }

              dateText=value;
            }

            if (key.equals("camera")) {
              if (!value.equals(cameraId)) {
                hasChange=true;
              }

              cameraId=value;
            }

            if (key.equals("roll")) {
              if (value.toInt() != rollNumber) {
                hasChange=true;
              }

              rollNumber=value.toInt();
            }

            if (key.equals("scene")) {
              if (value.toInt() != sceneNumber) {
                hasChange=true;
              }

              sceneNumber=value.toInt();
            }

            if (key.equals("take")) {
              if (value.toInt() != takeNumber) {
                hasChange=true;
              }

              takeNumber=value.toInt();
            }

            if (key.equals("mos")) {
              if (value.equals("1")) {
                if (!mos) hasChange = true;
                mos = true;
              } else {
                if (mos) hasChange = true;
                mos = false;
              }
            }

            if (key.equals("sync")) {
              if (value.equals("1")) {
                if (!mos) hasChange = true;
                sync = true;
              } else {
                if (mos) hasChange = true;

                sync = false;
              }
            }

            if (key.equals("filter")) {
              if (value.equals("1")) {
                if (!mos) hasChange = true;
                filter = true;
              } else {
                if (mos) hasChange = true;
                filter = false;
              }
            }

            if (key.equals("extloc")) {
              if (value.equals("1")) {
                if (location==0) hasChange = true;
                location = 1;
              } else {
                if (location==1) hasChange = true;
                location = 0;
              }
            }

            if (key.equals("intloc")) {
              if (value.equals("1")) {
                if (location==1) hasChange = true;
                location = 0;
              } else {
                if (location==0) hasChange = true;
                location = 1;
              }
            }

            if (key.equals("night")) {
              if (value.equals("1")) {
                if (timeOfDay==0) hasChange = true;
                timeOfDay = 1;
              } else {
                if (timeOfDay==1) hasChange = true;
                timeOfDay = 0;
              }
            }

            if (key.equals("day")) {
              if (value.equals("1")) {
                if (timeOfDay==1) hasChange = true;
                timeOfDay = 0;
              } else {
                if (timeOfDay==0) hasChange = true;
                timeOfDay = 1;
              }
            }

            if (hasChange) {
              setupstatic();
            }
          } // End; if (HTTP_req.indexOf("POST /Mega HTTP/1.1") > -1)
          
          // Display received HTTP request on Serial Monitor
          Serial.print(HTTP_req);

          // Clear saved string
          HTTP_req = "";
          break;
          
        } // End; if (!client.available())
        
      } // End; if (client.available())
    
    } // End; while (client.connected())

    // Give browser time to received data
    delay(1);

    // Close connections
    client.stop();
    
  } // End; if (client)

} // End; Main Programm Loop
