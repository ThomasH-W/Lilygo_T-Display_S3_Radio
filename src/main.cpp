/*
  File : main.cpp
  Date : 26.03.2021

  ESP32 Internet Radio with Display based on TTGO
  https://github.com/ThomasH-W/ESP32-Radio-TTGO

  ToDo:
    ESP deepsleep, wakeup
    Battery monitoring
*/

#include <Arduino.h>

#define DEBUG true          // enable additional serial printf
#include "_SerialPrintf.h"  // Wrapper for Serial.print
#include "_GPIO.h"
  
#if !(defined(ESP32))
#error This code is intended to run on the ESP8266 or ESP32 platform! Please check your Tools->Board setting.
#endif

#define LED_ON HIGH
#define LED_OFF LOW

#include "wifiMgr.h"
void loop_audio();
void setup_rotary();
void loop_rotary();

#include "FS_tools.h"

#include "myDisplay.h"
myDisplay myDisplay1;
char statusChar[50];
wifi_data_struct wifiData;

#include "myNTP.h"
myNTP myNtp;
unsigned long lastNtp = 0;

#include <AceButton.h>
using namespace ace_button;
const int BUTTON1_PIN = PIN_BTN_1;
const int BUTTON2_PIN = PIN_BTN_2;
const int BUTTON3_PIN = PIN_BTN_3; // 37;
const int BUTTON4_PIN = PIN_BTN_4; // 37;

AceButton button1(BUTTON1_PIN);
AceButton button2(BUTTON2_PIN);
AceButton button3(BUTTON3_PIN);
AceButton button4(BUTTON4_PIN);
void handleEvent(AceButton *, uint8_t, uint8_t);
const int LED_PIN = 2; // for ESP32

audio_data_struct *audio_data_ptr;
wifi_data_struct *wifi_data_ptr;

bool displayUpdate = false;
int mode = 0, oldMode = 0; // based on state
unsigned long currentMillisLoop = 0, previousMillisDisplay = 0, intervalDisplayLoop = 3000;

// --------------------------------------------------------------------------
// https://github.com/bxparks/AceButton
// https://github.com/bxparks/AceButton/blob/develop/examples/TwoButtonsUsingOneButtonConfig/TwoButtonsUsingOneButtonConfig.ino
void setup_button()
{
  // Buttons use the built-in pull up register.
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
  pinMode(BUTTON4_PIN, INPUT_PULLUP);

  // Configure the ButtonConfig with the event handler, and enable all higher
  // level events.
  ButtonConfig *buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);

  // Check if the button was pressed while booting
  if (button1.isPressedRaw())
  {
    Serial.println(F("setup(): button 1 was pressed while booting"));
  }
  if (button2.isPressedRaw())
  {
    Serial.println(F("setup(): button 2 was pressed while booting"));
  }
  if (button3.isPressedRaw())
  {
    Serial.println(F("setup(): button 3 was pressed while booting"));
  }

} // end of function

// --------------------------------------------------------------------------
// The event handler for the button.
void handleEvent(AceButton *button, uint8_t eventType, uint8_t buttonState)
{
  int butPressed = button->getPin();

  // Print out a message for all events, for both buttons.
  Serial.print(F("handleEvent(): pin: "));
  Serial.print(butPressed);
  Serial.print(F("; eventType: "));
  Serial.print(eventType);
  Serial.print(F("; buttonState: "));
  Serial.println(buttonState);

  // Control the LED only for the Pressed and Released events of Button 1.
  // Notice that if the MCU is rebooted while the button is pressed down, no
  // event is triggered and the LED remains off.
  /*
#define PIN_BTN_4 27 // preset+
#define PIN_BTN_1 0  // preset+ - onboard button
#define PIN_BTN_2 35 // mode - onboard button
#define PIN_BTN_3 36 // mode
#define PIN_BTN_5 37 // reset

#define PIN_BTN_6 32 // volume +
#define PIN_BTN_7 33 // volume -

#define PIN_BTN_8 38 // preset -
*/

  switch (eventType)
  {
  case AceButton::kEventReleased:                               // kEventPressed
    if (butPressed == BUTTON1_PIN || butPressed == BUTTON4_PIN) // preset+
    {
      serial_printf("handleEvent> preset_up for pin %d\n", butPressed);
      audio_mode(AUDIO_PRESET_UP);
    }
    else if (butPressed == BUTTON2_PIN || butPressed == BUTTON3_PIN) // mode
    {
      serial_printf("handleEvent> GUI for pin %d\n", butPressed);
      // audio_mode(AUDIO_MUTE);
      if (mode == ST_GUI_1)
        mode = ST_GUI_4;
      else
        mode = ST_GUI_1;
      displayUpdate = true;
    }
    break;
  default:
    serial_printf("handleEvent> unknown for pin %d & %d\n", butPressed, eventType);
  }
} // end of function

// --------------------------------------------------------------------------
// set flag for updating display, will be recognized next time in displayLoop
void main_displayUpdate(bool clearScreen)
{
  if (audio_data_ptr)                                                       // pointer is initialized after display setup
    serial_printf("main::main_displayUpdate %d\n", audio_data_ptr->update); // audio_data_ptr->update
  else
    serial_printf("main::main_displayUpdate\n"); // audio_data_ptr->update
  displayUpdate = true;
  if (clearScreen)
    displayReset();
} // end of function

// ----------------------------------------------------------------------------------------
// if not in boot mode, print message on TFT display
void displayDebugPrint(const char *message)
{
  if (1 > mode)
    myDisplay1.print(message);
  Serial.print(message);
} // end of function

// ----------------------------------------------------------------------------------------
// if not in boot mode, print message on TFT display
void displayDebugPrintln(const char *message)
{
  if (1 > mode)
    myDisplay1.println(message);
  Serial.println(message);
} // end of function

// ---------------------------------------------------------------------------------------
// clear display, request update of current UI
void displayReset(void)
{
  serial_println("main::displayReset");
  myDisplay1.clear();
  displayUpdate = true;
} // end of function

// ---------------------------------------------------------------------------------------
// after interval, fallback to GUI1 showing mani screen
// if audio_data changed, flag update will be set
void displayLoop(void)
{
  currentMillisLoop = millis();
  if (currentMillisLoop - previousMillisDisplay > intervalDisplayLoop)
  {
    audio_data_ptr->update = UP_INFO;
    mode = ST_GUI_1;
    previousMillisDisplay = millis();
  }

  switch (audio_data_ptr->update)
  {
  case UP_INFO: // 3
    // mode = ST_GUI_1;
    break;
  case UP_VOLUME: // 3
    mode = ST_GUI_2;
    break;
  case UP_PRESET: // 4
    mode = ST_GUI_3;
    break;
  }

  if (mode != oldMode)
  {
    myDisplay1.clear();
    oldMode = mode;
    displayUpdate = true;
  }

  if (displayUpdate == true)
  {
    switch (mode)
    {
    case ST_GUI_1:     // 3
      pub_wifi_info(); // get latest wifi signal strength
      myNtp.value(wifi_data_ptr->timeOfDayChar, wifi_data_ptr->dateChar);
      myDisplay1.Gui1(audio_data_ptr, wifi_data_ptr); // text
      break;
    case ST_GUI_2:                     // 4
      myDisplay1.Gui2(audio_data_ptr); // gauge left/right
      break;
    case ST_GUI_3:                     // 4
      myDisplay1.Gui3(audio_data_ptr); // gauge left/right
      break;
    case ST_GUI_4:                    // 4
      myDisplay1.Gui4(wifi_data_ptr); // gauge left/right
      break;
    }
    previousMillisDisplay = millis();
    displayUpdate = false;
  }
    yield();
    taskYIELD();
} // end of function

// --------------------------------------------------------------------------
void setup()
{
  Serial.begin(MONITOR_SPEED);
  while (!Serial);// auf serielle Verbindung warten
  delay(500);
  serial_println("-------------------------------------------------------- setup");

  //#define TFT_BL 38
  pinMode( TFT_BL, OUTPUT); // fix runtime: __digitalWrite(): IO 38 is not set as GPIO. Execute digitalMode(38, OUTPUT) first

  myDisplay1.begin(); // start display
  myDisplay1.println("> Boot TTGO-Radio ...");

  setup_gpio_pins(); // get default gpio pins
  setup_button();

  delay(1000);
  myDisplay1.print("> Firmware ");
  myDisplay1.println(FIRMWARE_VERSION);

  bool func_success = setup_read_file(); // read config file setup.ini from littleFS and pass every line to parser
  if (func_success == false)
  {
    myDisplay1.println("> PANIC: setup.ini not found in SIPFFS!");
    myDisplay1.println("> Build and upload filesystem in platformIO");
    myDisplay1.println("> System halted");
    while (true)
      ; // loop forever
  }

  // myDisplay1.Gui0(); test gui using differrent fonts, files to be loaded into SPIFFS

  myDisplay1.println("> setup WiFi ...");
  setup_wifi(); // call wifimanager and establish mqtt connection
  wifi_data_ptr = setup_wifi_info();

  myDisplay1.println("> setup NTP ...");
  myNtp.begin();

  myDisplay1.println("> setup audio ...");
  audio_data_ptr = setup_audio(); // start audio

  mqtt_pub_tele("INFO", "setup complete");
  list_FS(); // debug: show files in littlefs

  setup_rotary(); // initialize rotary encoder
  myDisplay1.println("> setup complete");

  mode = ST_GUI_1; // switch to default GUI 7 main screen
} // end of function


// --------------------------------------------------------------------------
void loop()
{
  loop_wifi(); // update mqtt
  loop_rotary();
  loop_audio();
  myNtp.loop();

  button1.check();
  button2.check();
  button3.check();
  button4.check();

  displayLoop();
  
  yield();
  taskYIELD();
} // end of function
