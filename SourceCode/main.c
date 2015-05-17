/**
 * 支持的芯片：{STC89C52RC} + {LCD1602}
 * 晶振频率：  22.1184MHz
 */

#include "common/common.h"
#include "lcd1602/lcd1602.h"
#include "keyboard/keyboard.h"

void main(void)
{
    unsigned char* keyNumber;

    lcdInit();

    while (1) {
        keyNumber = getKeyNumber();
        if (keyNumber != NULL) {
//            append(numbers, keyNumber, 6);
            display1602(1, 9, keyNumber, 1);
        }
    }
}