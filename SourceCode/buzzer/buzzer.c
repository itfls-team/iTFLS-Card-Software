#include "buzzer.h"

sbit buzzer = BUZZER_PIN;

extern void buzzerShortBeep(void)
{
    buzzer = HIGH;
    delayMs(100);
    buzzer = LOW;
}

extern void buzzerLongBeep(void)
{
    buzzer = HIGH;
    delayMs(500);
    buzzer = LOW;
}
