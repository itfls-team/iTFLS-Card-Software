/**
 * 路径及文件名：  /SourceCode/keyboard/keyboard.c
 * 说明：          定义有关键盘的函数。
 * 函数列表：      extern unsigned char keyboardGetKey(void)
 *                 extern unsigned char* keyboardToString(unsigned char scanCode)
 */

#include "keyboard.h"


static unsigned char code keyboardScanCodeTable[4][3] = {
    {0x01,0x02,0x03},
    {0x04,0x05,0x06},
    {0x07,0x08,0x09},
    {0x0A,0x00,0x0B},
};

extern unsigned char keyboardGetKey(void)
{
    unsigned char code scanTableRow[4] = {
        0xFE,
        0xFD,
        0xFB,
        0xF7,
    };
    unsigned char code scanTableLine[3] = {
        0xE0,
        0xD0,
        0xB0,
    };

    unsigned char keyboardScanTemp = 0,
                  currentRow,
                  currentLine;

    for (currentRow = 0; currentRow <= 3; currentRow++) {
        KEYBOARD_PORT_SCAN = scanTableRow[currentRow];
        keyboardScanTemp = KEYBOARD_PORT_SCAN & 0xF0;

        if (keyboardScanTemp != 0xF0) {
            delayMs(5);
            keyboardScanTemp = KEYBOARD_PORT_SCAN & 0xF0;

            if (keyboardScanTemp != 0xF0) {
                for (currentLine = 0; currentLine <= 2; currentLine++) {
                    if (keyboardScanTemp == scanTableLine[currentLine]) {
                        while (1) {
                            keyboardScanTemp = KEYBOARD_PORT_SCAN & 0xF0;
                            if (keyboardScanTemp == 0xF0) {
                                break;
                            }
                        }
                        return keyboardScanCodeTable[currentRow][currentLine];
                    }
                }
            }
        }
    }
    return 0xFF;
}

extern unsigned char* keyboardToString(unsigned char scanCode)
{
    char code keyboardStringTable[4][3][2] = {
        {"1","2","3"},
        {"4","5","6"},
        {"7","8","9"},
        {".","0","#"},
    };
    unsigned char currentRow,
                  currentLine;
    
    for (currentRow = 0; currentRow <= 3; currentRow++) {
        for (currentLine = 0; currentLine <= 2; currentLine++) {
            if (scanCode == keyboardScanCodeTable[currentRow][currentLine]) {
                return keyboardStringTable[currentRow][currentLine];
            }
        }
    }
    
    return NULL;
}