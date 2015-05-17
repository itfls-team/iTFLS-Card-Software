#include "lcd1602.h"

//sbit lcdBacklight   = P2^3;
sbit lcdRegister = P1^0;//P2^0;
sbit lcdReadWrite = P1^1;
sbit lcdEnable = P2^5;

static void lcdWriteCommand (unsigned char lcdCommand, bit waitBusy);
static void lcdWriteData    (unsigned char lcdData);

static void lcdWaitBusy()
{
    bit isLcdBusy;
    isLcdBusy = 1;
    while (isLcdBusy) {
        lcdRegister = LCD_COMMAND_REGISTER;
        lcdReadWrite = LCD_READ;
        lcdEnable = LCD_HIGH;
        isLcdBusy = (bit)(P0 & 0x80);
        delayNop();
    }
    lcdEnable = LCD_LOW;
}

static void lcdWriteCommand(unsigned char lcdCommand, bit waitBusy)
{
    if (waitBusy) {
        lcdWaitBusy();
    }

    lcdRegister = LCD_COMMAND_REGISTER;
    lcdReadWrite = LCD_WRITE;
    lcdEnable = LCD_HIGH;
    P0 = lcdCommand;
    delayNop();
    lcdEnable = LCD_LOW;
}

static void lcdWriteData(unsigned char lcdData)
{
    lcdWaitBusy();
    lcdRegister = LCD_DATA_REGISTER;
    lcdReadWrite = LCD_WRITE;
    lcdEnable = LCD_HIGH;
    P0 = lcdData;
    delayNop();
    lcdEnable = LCD_LOW;
}

void lcdInit(void)
{
    delayMs(15);
    lcdWriteCommand(0x38, 0);
    delayMs(5);
    lcdWriteCommand(0x38, 0);
    delayMs(5);
    lcdWriteCommand(0x38, 0);
    delayMs(5);
    lcdWriteCommand(0x38, 1);
    delayMs(5);
    lcdWriteCommand(0x0c, 1);
    delayMs(5);
    lcdWriteCommand(0x06, 1);
    delayMs(5);
    lcdWriteCommand(0x01, 1);
    delayMs(5);
}

void display1602(
    unsigned char line,
    unsigned char row,
    unsigned char *dataParameter,
    unsigned char lenth
) {
    unsigned char *lcdData = dataParameter;
    unsigned char address = (line == 1 ? 0x80 : 0xC0) + 0x01 * (row - 1);
    
    if (lenth > 16) {
        lenth = 16;
    }

	lcdWriteCommand(address, 1);
	while (lenth--)	{
        lcdWriteData(*lcdData++);
    }
}