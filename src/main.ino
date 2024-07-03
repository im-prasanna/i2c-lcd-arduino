#include "I2C_LCD.h"
void setup(){
  lcd_init(0x27, 0, 0);
  lcd_entryModeSet(1,0);
  lcd_displayControl(1,1,1);
  lcd_clear();
  lcd_backlight(1);
  lcd_print("Hello world");
}

void loop()
{
  
}
