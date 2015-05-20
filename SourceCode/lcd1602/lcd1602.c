/**
 * 路径及文件名：  /SourceCode/lcd1602/lcd1602.c
 * 说明：          定义有关1602液晶操作的函数。
 * 函数列表：      extern void lcd1602Clear(void)
 *                 extern void lcd1602Init(void)
 *                 extern void lcd1602Display(
 *                     unsigned char line,
 *                     unsigned char row,
 *                     unsigned char* content,
 *                 )
 *                 extern void lcd1602Backlight(bit status)
 *                 static void lcd1602WaitBusy(void)
 *                 static void lcd1602WriteCommand(unsigned char lcd1602Command, bit waitBusy)
 *                 static void lcd1602WriteData(unsigned char lcd1602Data)
 */
 
#include "lcd1602.h"

#if LCD1602_FUNCTION_BACKLIGHT
sbit lcd1602BacklightSwitch =   LCD1602_PIN_BACKLIGHT;
#endif
sbit lcd1602Enable          =   LCD1602_PIN_ENABLE;
sbit lcd1602ReadWrite       =   LCD1602_PIN_READWRITE;
sbit lcd1602Register        =   LCD1602_PIN_REGISTER;

static void lcd1602WaitBusy(void);
static void lcd1602WriteCommand(unsigned char lcd1602Command, bit waitBusy);
static void lcd1602WriteData(unsigned char lcd1602Data);

#if LCD1602_FUNCTION_BACKLIGHT
extern void lcd1602Backlight(bit status)
{
    lcd1602BacklightSwitch = status;
}
#endif

extern void lcd1602Clear(void)
{
    lcd1602WriteCommand(0x01, 1);
}

extern void lcd1602Display(
    unsigned char line,
    unsigned char row,
    unsigned char* content
) {
    unsigned char* contentCopy = content;
    unsigned char address = (line == 1 ? 0x80 : 0xC0) + 0x01 * (row - 1);
    unsigned char maxContentLenth = 16;

	lcd1602WriteCommand(address, 1);
	while (maxContentLenth--) {
        if (*contentCopy == '\0') {
            return;
        }
        lcd1602WriteData(*contentCopy);
        contentCopy++;
    }
}

extern void lcd1602Init(void)
{
    delayMs(15);
    lcd1602WriteCommand(0x38, 0);
    delayMs(5);
    lcd1602WriteCommand(0x38, 0);
    delayMs(5);
    lcd1602WriteCommand(0x38, 0);
    delayMs(5);
    lcd1602WriteCommand(0x38, 1);
    delayMs(5);
    lcd1602WriteCommand(0x0c, 1);
    delayMs(5);
    lcd1602WriteCommand(0x06, 1);
    delayMs(5);
    lcd1602WriteCommand(0x01, 1);
    delayMs(5);
}

static void lcd1602WaitBusy(void)
{
    bit isLcdBusy;
    isLcdBusy = 1;
    while (isLcdBusy) {
        lcd1602Register = LCD1602_COMMAND_REGISTER;
        lcd1602ReadWrite = LCD1602_READ;
        lcd1602Enable = HIGH;
        isLcdBusy = (bit)(LCD1602_PORT_BUS & 0x80);
        delayNop();
    }
    lcd1602Enable = LOW;
}

static void lcd1602WriteCommand(unsigned char lcd1602Command, bit waitBusy)
{
    if (waitBusy) {
        lcd1602WaitBusy();
    }

    lcd1602Register = LCD1602_COMMAND_REGISTER;
    lcd1602ReadWrite = LCD1602_WRITE;
    lcd1602Enable = HIGH;
    LCD1602_PORT_BUS = lcd1602Command;
    delayNop();
    lcd1602Enable = LOW;
}

static void lcd1602WriteData(unsigned char lcd1602Data)
{
    lcd1602WaitBusy();
    lcd1602Register = LCD1602_DATA_REGISTER;
    lcd1602ReadWrite = LCD1602_WRITE;
    lcd1602Enable = HIGH;
    LCD1602_PORT_BUS = lcd1602Data;
    delayNop();
    lcd1602Enable = LOW;
}
