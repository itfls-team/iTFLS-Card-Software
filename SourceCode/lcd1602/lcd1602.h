#include "../common/common.h"

#define LCD_DATA_REGISTER       1
#define LCD_COMMAND_REGISTER    0
#define LCD_READ                1
#define LCD_WRITE               0
#define LCD_HIGH                1
#define LCD_LOW                 0

void display1602 (unsigned char line, unsigned char row, unsigned char *dataParameter, unsigned char lenth);
void lcdInit     (void);