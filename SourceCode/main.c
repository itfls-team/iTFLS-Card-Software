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
    extern unsigned char* keyboardToString(unsigned char scanCode);
    extern void lcd1602Clear(void);
    extern void serialPortInit(void);
    extern void serialPortTransmit(unsigned char* dataToTransmit, bit isString, unsigned char dataLenth);
    extern unsigned char* serialPortReceive(void);
    extern void MFRC500ChipStart(void);
    extern void MFRC500GetCardSerialNumber(unsigned char* cardSN);
    extern void buzzerShortBeep(void);
    extern void buzzerLongBeep(void);
    extern void MFRC500ChipHalt(void);

    unsigned char keyCode = 0xFF,
                  priceString[7],
                  digitOfDecimalPart = 0,
                  digitOfIntegerPart = 0,
                  cardSerialNumber[5];
    unsigned char xdata dataToSend[14];
    union {
        float f;
        unsigned char c[4];
    } price;
    bit hasDot = 0;

    lcd1602Init();
    lcd1602Backlight(LCD1602_STATUS_BACKLIGHT_ON);
    serialPortInit();
    EA = 1;
    priceString[0] = '\0';
    dataToSend[0] = '\0';
    buzzerShortBeep();
    lcd1602Display(1, 1, "Ready!");

    while (1) {
        while (1) {
            keyCode = keyboardGetKey();
            if (keyCode != 0xFF) {
                if (keyCode == 0x0B) {
                    lcd1602Clear();
                    hasDot = 0;
                    digitOfDecimalPart = 0;
                    digitOfIntegerPart = 0;
                    break;
                }

                if (strlen(priceString) < 6 && digitOfDecimalPart < 2) {
                    if (keyCode == 0x0A) {
                        if (!hasDot){
                            strcat(priceString, ".");
                            hasDot = 1;
                        }
                    } else {
                        if (digitOfIntegerPart >= 3 && !hasDot) {
                            continue;
                        }
                        strcat(priceString, keyboardToString(keyCode));
                        if (hasDot) {
                            digitOfDecimalPart++;
                        } else {
                            digitOfIntegerPart++;
                        }
                    }

                    lcd1602Clear();
                    lcd1602Display(1, 1, priceString);
                }
            }
        }

        lcd1602Display(1, 1, priceString);
        lcd1602Display(2, 1, "Please Read Card");

        MFRC500ChipStart();
        MFRC500GetCardSerialNumber(cardSerialNumber);
        buzzerShortBeep();
        MFRC500ChipHalt();
        
        price.f = atof(priceString);
        priceString[0] = '\0';
        dataToSend[0] = 0x69; //i
        dataToSend[1] = 0x54; //T
        dataToSend[2] = price.c[3];
        dataToSend[3] = price.c[2];
        dataToSend[4] = price.c[1];
        dataToSend[5] = price.c[0];
        dataToSend[6] = 0x46; //F
        memcpy(&dataToSend[7], &cardSerialNumber[1], 4);
        dataToSend[11] = 0x4C; //L
        dataToSend[12] = 0x53; //S
        serialPortTransmit(dataToSend, 0, 13);
        dataToSend[0] = '\0';
        lcd1602Clear();
        lcd1602Display(1, 5, "Success!");
    }
}