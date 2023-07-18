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

class WatchFace : public Watchy { //inherit and extend Watchy class
  using Watchy::Watchy;
  public:
    void drawWatchFace() { //override this method to customize how the watch face looks
      
      int16_t  x1, y1;
      uint16_t w, h;
      String textstring;
      bool light = false; // left this here if someone wanted to tweak for dark

      //resets step counter at midnight everyday
      if(currentTime.Hour == 00 && currentTime.Minute == 00) {
        sensor.resetStepCounter();
      }

      // ** DRAW **

      //drawbg
      display.fillScreen(light ? GxEPD_WHITE : GxEPD_BLACK);  
      display.setFont(&DSEG7_Classic_Bold_53);
      display.setTextColor(light ? GxEPD_BLACK : GxEPD_WHITE);
      display.setTextWrap(false);

      //draw image
      display.drawBitmap(16,159, epd_bitmap_icons, 168, 41, light ? GxEPD_BLACK : GxEPD_WHITE);

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

      //draw steps
      textstring = sensor.getCounter();
      textstring += " steps";
      display.getTextBounds(textstring, 0, 0, &x1, &y1, &w, &h);
      display.setCursor(155-w/2, 145);
      display.setTextColor(light ? GxEPD_BLACK: GxEPD_WHITE);
      display.print(textstring);

      // draw date
      textstring = monthShortStr(currentTime.Month);
      textstring += " ";
      textstring += currentTime.Day;
      textstring += " ";
      textstring += currentTime.Year + 1970;
      display.getTextBounds(textstring, 0, 0, &x1, &y1, &w, &h);
      display.setCursor(50-w/2, 145);
      display.print(textstring);

      // Clear screen on the hour
      if (currentTime.Minute == 0) {
        display.display(false);
      }
    }

};


WatchFace m(settings); //instantiate your watchface

void setup() {
  m.init(); //call init in setup
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
