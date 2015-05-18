/**
 * 路径及文件名：  /SourceCode/main.c
 * 说明：          主函数文件。
 * 函数列表：      void main(main)
 */

#include "common/common.h"

void main(void)
{
    extern void lcd1602Backlight(bit status);
    extern void lcd1602Display(unsigned char line, unsigned char row, unsigned char *content, unsigned char contentLenth);
    extern void lcd1602Init(void);
    extern unsigned char* keyboardGetKey(void);

    unsigned char* keyNumber;

    lcd1602Init();
    lcd1602Backlight(LCD1602_STATUS_BACKLIGHT_ON);

    while (1) {
        keyNumber = keyboardGetKey();
        if (keyNumber != NULL) {
            lcd1602Display(1, 1, keyNumber, 1);
        }
    }
}
