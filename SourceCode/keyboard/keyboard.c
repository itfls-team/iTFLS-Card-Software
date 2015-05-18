/**
 * 路径及文件名：  /SourceCode/keyboard/keyboard.c
 * 说明：          定义有关键盘的函数。
 * 函数列表：      extern unsigned char* keyboardGetKey(void)
 */

#include "keyboard.h"

extern unsigned char* keyboardGetKey(void)
{
    unsigned char code rowScanTable[4] = {
        0xFE,
        0xFD,
        0xFB,
        0xF7,
    };
    unsigned char code lineScanTable[3] = {
        0xE0,
        0xD0,
        0xB0,
    };
    unsigned char code keyTable[4][3][1] = {
        {"1","2","3"},
        {"4","5","6"},
        {"7","8","9"},
        {"*","0","#"},
    };

    unsigned char   keyScanTemp = 0,
                    row,
                    line;

    for (row = 0; row <= 3; row++) {
        KEYBOARD_PORT_SCAN = rowScanTable[row];
        keyScanTemp = KEYBOARD_PORT_SCAN & 0xF0;

        if (keyScanTemp != 0xF0) {
            delayMs(5);
            keyScanTemp = KEYBOARD_PORT_SCAN & 0xF0;

            if (keyScanTemp != 0xF0) {
                for (line = 0; line <= 2; line++) {
                    if (keyScanTemp == lineScanTable[line]) {
                        while (1) {
                            keyScanTemp = KEYBOARD_PORT_SCAN & 0xF0;
                            if (keyScanTemp == 0xF0) {
                                break;
                            }
                        }
                        return keyTable[row][line];
                    }
                }
            }
        }
    }
    return NULL;
}
