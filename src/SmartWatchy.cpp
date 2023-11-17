/*
SmartWatchy
https://github.com/theRealc2c2/SmartWatch

Based on Bahn-for-Watchy
https://github.com/BraininaBowl/Bahn-for-Watchy

Face for Watchy watch
https://watchy.sqfmi.com
*/

#include <Watchy.h> //include the Watchy library
#include "Teko_Regular12pt7b.h"
#include "DSEG7_Classic_Bold_53.h"
#include "icons.h"
#include "settings.h"

RTC_DATA_ATTR time_t timerEndTime;
RTC_DATA_ATTR tmElements_t timerDuration;
#define TIMER_OFF 0
#define TIMER_RUNNING 1
#define TIMER_FINISHED 2
RTC_DATA_ATTR byte timerState = TIMER_OFF;
#define TOGGLE_TIMER 2

RTC_DATA_ATTR bool HourScreenReset;

class SmartWatchy : public Watchy { //inherit and extend Watchy class
  public:
    /* Override */
    const char* getMenuName(int index) {
      if (index < 6) return Watchy::getMenuName(index);
      const char* testMenuName = "Set Timer";
      return testMenuName;
    }

    SmartWatchy(const watchySettings &s) : Watchy(s) {
      menuPages[6] = std::bind(&SmartWatchy::setTimerMenu, this);
    }

    void toggleTimer() {
      timerState = !timerState;
    }

    void setTimerMenu() {
      guiState = APP_STATE;

      int8_t minute = timerDuration.Minute;
      int8_t hour   = timerDuration.Hour;

      int8_t setIndex = SET_HOUR; // Index in timer menu

      int8_t blink = 0;

      pinMode(DOWN_BTN_PIN, INPUT);
      pinMode(UP_BTN_PIN, INPUT);
      pinMode(MENU_BTN_PIN, INPUT);
      pinMode(BACK_BTN_PIN, INPUT);

      display.setFullWindow();

      while (1) {

        if (digitalRead(MENU_BTN_PIN) == 1) {
          setIndex++;
          if (setIndex > TOGGLE_TIMER) {
            break;
          }
        }
        if (digitalRead(BACK_BTN_PIN) == 1) {
          if (setIndex != SET_HOUR) {
            setIndex--;
          }
        }

        blink = 1 - blink;

        if (digitalRead(DOWN_BTN_PIN) == 1) {
          blink = 1;
          switch (setIndex) {
          case SET_HOUR:
            hour == 23 ? (hour = 0) : hour++;
            break;
          case SET_MINUTE:
            minute == 59 ? (minute = 0) : minute++;
            break;
          case TOGGLE_TIMER:
            toggleTimer();
            break;
          default:
            break;
          }
        }

        if (digitalRead(UP_BTN_PIN) == 1) {
          blink = 1;
          switch (setIndex) {
          case SET_HOUR:
            hour == 0 ? (hour = 23) : hour--;
            break;
          case SET_MINUTE:
            minute == 0 ? (minute = 59) : minute--;
            break;
          case TOGGLE_TIMER:
            toggleTimer();
            break;
          default:
            break;
          }
        }

        display.fillScreen(GxEPD_BLACK);
        display.setTextColor(GxEPD_WHITE);
        display.setFont(&DSEG7_Classic_Bold_53);

        display.setCursor(5, 80);
        if (setIndex == SET_HOUR) { // blink hour digits
          display.setTextColor(blink ? GxEPD_WHITE : GxEPD_BLACK);
        }
        if (hour < 10) {
          display.print("0");
        }
        display.print(hour);

        display.setTextColor(GxEPD_WHITE);
        display.print(":");

        display.setCursor(108, 80);
        if (setIndex == SET_MINUTE) { // blink minute digits
          display.setTextColor(blink ? GxEPD_WHITE : GxEPD_BLACK);
        }
        if (minute < 10) {
          display.print("0");
        }
        display.print(minute);

        display.setTextColor(GxEPD_WHITE);

        display.setFont(&FreeMonoBold9pt7b);
        display.setCursor(45, 150);
        if (setIndex == TOGGLE_TIMER) { // blink timer text
          display.setTextColor(blink ?GxEPD_WHITE : GxEPD_BLACK);
        }
        display.print(timerState == TIMER_OFF ? "Disabled" : "Enabled");

        display.display(true); // partial refresh
      }

      timerDuration.Hour = hour;
      timerDuration.Minute = minute;

      if(timerState == TIMER_RUNNING) {
        RTC.read(currentTime);
        timerEndTime = makeTime(currentTime);
        timerEndTime += timerDuration.Hour * SECS_PER_HOUR;
        timerEndTime += timerDuration.Minute * SECS_PER_MIN;
        timerState = TIMER_RUNNING;
      } else {
        timerState = TIMER_OFF;
        timerEndTime = 0;
      }

      showWatchFace(false);
    }

    /* Override */
    void handleButtonPress() {
      // Only on the watch face
      if(guiState != WATCHFACE_STATE) { Watchy::handleButtonPress(); return; }
      uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();
      // Only the up and down buttons
      if(!(wakeupBit & UP_BTN_MASK || wakeupBit & DOWN_BTN_MASK)) { Watchy::handleButtonPress(); return; }
      // Dismiss notice
      if(timerState == TIMER_FINISHED) timerState = TIMER_OFF;
      RTC.read(currentTime);
      showWatchFace(true);
    }

    void drawWatchFace() { //override this method to customize how the watch face looks
      int16_t  x1, y1;
      uint16_t w, h;
      String textstring;
      bool light = false; // left this here if someone wanted to tweak for dark

      //resets step counter at midnight everyday
      if(currentTime.Hour == 00 && currentTime.Minute == 00) {
        sensor.resetStepCounter();
      }

      // Set time every night
      if (currentTime.Hour == 4 && currentTime.Minute == 00) {
        updateTime();
      }

      // ** DRAW **

      //drawbg
      display.fillScreen(light ? GxEPD_WHITE : GxEPD_BLACK);  
      display.setFont(&DSEG7_Classic_Bold_53);
      display.setTextColor(light ? GxEPD_BLACK : GxEPD_WHITE);
      display.setTextWrap(false);

      //draw image
      // display.drawBitmap(16,159, epd_bitmap_icons, 168, 41, light ? GxEPD_BLACK : GxEPD_WHITE);

      //draw time
      textstring = currentTime.Hour;
      textstring += ":";
      if (currentTime.Minute < 10) {
        textstring += "0";
      } else {
        textstring += "";
      }
      textstring += currentTime.Minute;
      display.getTextBounds(textstring, 0, 0, &x1, &y1, &w, &h);
      display.setCursor(10, 110);
      display.print(textstring);

      display.setFont(&Teko_Regular12pt7b);

      // draw battery
      display.fillRoundRect(16,16,34,12,4,light ? GxEPD_BLACK : GxEPD_WHITE);
      display.fillRoundRect(49,20,3,4,2,light ? GxEPD_BLACK : GxEPD_WHITE);
      display.fillRoundRect(18,18,30,8,3,light ? GxEPD_WHITE : GxEPD_BLACK);
      float batt = (getBatteryVoltage()-3.3)/0.9;
      if (batt > 0) {
        display.fillRoundRect(20,20,26*batt,4,2,light ? GxEPD_BLACK : GxEPD_WHITE);
      }
      
      // batt percentage
      textstring = String(min(100, int(batt*100)));
      textstring += "%";
      display.getTextBounds(textstring, 0, 0, &x1, &y1, &w, &h);
//      textstring += " " + String(h);			// Debugging purpose							
      display.setCursor(55, 22+h/2);
      display.print(textstring);

      auto now = makeTime(currentTime);

      //draw steps
      switch (timerState) {
      case TIMER_OFF:
        textstring = sensor.getCounter();
        textstring += " steps";
        display.getTextBounds(textstring, 0, 0, &x1, &y1, &w, &h);
        display.setCursor(155-w/2, 145);
        display.setTextColor(light ? GxEPD_BLACK: GxEPD_WHITE);
        display.print(textstring);

        // draw date
        textstring = currentTime.Day;
        textstring += "-";
        textstring += currentTime.Month;
        textstring += "-";
        textstring += currentTime.Year + 1970;
        display.getTextBounds(textstring, 0, 0, &x1, &y1, &w, &h);
        display.setCursor(50-w/2, 145);
        display.println(textstring);
        break;
      case TIMER_RUNNING:
        if (timerEndTime < now) { // ended
          RTC.clearTimer();
          timerState = TIMER_FINISHED;
          timerEndTime = 0;
          textstring = "Timer Finished";
          display.getTextBounds(textstring, 0, 0, &x1, &y1, &w, &h);
          display.setCursor(100-w/2, 145);
          display.setTextColor(light ? GxEPD_BLACK: GxEPD_WHITE);
          display.println(textstring);
          vibMotor();
        } else { // still running
          tmElements_t t;
          breakTime(timerEndTime - now, t);
          textstring = "Timer: ";
          if(t.Hour < 10) textstring += "0";
          textstring += t.Hour + ":";
          if(t.Minute < 10) textstring += "0";
          textstring += t.Minute;
          if (t.Minute < 2) { // Set rtc interrupt to 1 sec and display secs
            textstring += ":";
            if (t.Second < 10) textstring += "0";
            textstring += t.Second;
          }
          display.setFont(&FreeMonoBold9pt7b);
          display.getTextBounds(textstring, 0, 0, &x1, &y1, &w, &h);
          display.setCursor(100-w/2, 145);
          display.setTextColor(light ? GxEPD_BLACK: GxEPD_WHITE);
          display.println(textstring);
        }
        break;
      case TIMER_FINISHED:
        timerEndTime = 0;
        textstring = "Timer Finished";
        display.getTextBounds(textstring, 0, 0, &x1, &y1, &w, &h);
        display.setCursor(100-w/2, 145);
        display.setTextColor(light ? GxEPD_BLACK: GxEPD_WHITE);
        display.println(textstring);
        break;
      }

      // Clear screen on the hour
      if (currentTime.Minute == 0) {
        display.display(false);
      }
    }

    /* Override */
    void deepSleep() {
      // Only override if timer is running to make timer interrupt work
      if (timerState != TIMER_RUNNING) { Watchy::deepSleep(); return; }
      auto now = makeTime(currentTime);
      // Only if 2 minutes left
      if ((timerEndTime - now) > (120)) { Watchy::deepSleep(); return; }
      display.hibernate();
      RTC.stopAlarm();
      RTC.setTimer();

      // Set GPIOs 0-39 to input to avoid power leaking out
      const uint64_t ignore = 0b11110001000000110000100111000010; // Ignore some GPIOs due to resets
      for (int i = 0; i < GPIO_NUM_MAX; i++) {
        if ((ignore >> i) & 0b1)
          continue;
        pinMode(i, INPUT);
      }
      esp_sleep_enable_ext0_wakeup((gpio_num_t)RTC_INT_PIN,
                                  0); // enable deep sleep wake on RTC interrupt
      esp_sleep_enable_ext1_wakeup(
          BTN_PIN_MASK,
          ESP_EXT1_WAKEUP_ANY_HIGH); // enable deep sleep wake on button press
      esp_deep_sleep_start();
    }

};


SmartWatchy m(settings); //instantiate your watchface

void setup() {
  m.init(); //call init in setup
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
