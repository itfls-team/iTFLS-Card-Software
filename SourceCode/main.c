/**
 * 路径及文件名：  /SourceCode/main.c
 * 说明：          主函数文件。
 * 函数列表：      void main(main)
 */

#include "common/common.h"

void main(void)
{
    extern void lcd1602Backlight(bit status);
    extern void lcd1602Display(unsigned char line, unsigned char row, unsigned char* content);
    extern void lcd1602Init(void);
    extern unsigned char keyboardGetKey(void);
    extern char* keyboardToString(unsigned char scanCode);
    extern void lcd1602RollRight(void);
    extern void lcd1602Clear(void);
    extern void lcd1602WriteCommand(unsigned char lcd1602Command, bit waitBusy);

    unsigned char keyCode = 0xFF,
                  price[7],
                  digitOfDecimalPart = 0,
                  digitOfIntegerPart = 0;
    bit hasDot = 0;

    lcd1602Init();
    price[0] = '\0';
    
    #if LCD1602_FUNCTION_BACKLIGHT
    lcd1602Backlight(LCD1602_STATUS_BACKLIGHT_ON);
    #endif

    while (1) {
        keyCode = keyboardGetKey();
        if (keyCode != 0xFF) {
            if (keyCode == 0x0B) {
                lcd1602Clear();
                hasDot = 0;
                digitOfDecimalPart = 0;
                digitOfIntegerPart = 0;
                price[0] = '\0';
                continue;
            }
            
            if (strlen(price) < 6 && digitOfDecimalPart < 2) {
                if (keyCode == 0x0A) {
                    if (!hasDot){
                        strcat(price, ".");
                        hasDot = 1;
                    }
                } else {
                    if (digitOfIntegerPart >= 3 && !hasDot) {
                        continue;
                    }
                    strcat(price, keyboardToString(keyCode));
                    if (hasDot) {
                        digitOfDecimalPart++;
                    } else {
                        digitOfIntegerPart++;
                    }
                }

                lcd1602Display(1, 1, price);
            }
        }
    }
}
