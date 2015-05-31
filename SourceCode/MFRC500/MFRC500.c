#include "MFRC500.h"

sbit led = P1^1;

extern void MFRC500ChipStart(void)
{
    extern char MFRC500InternalFunctionStart(void);
    
    while (MFRC500InternalFunctionStart() != MI_OK) {
        delayMs(100);
    }
}

extern void MFRC500ChipHalt(void)
{
    extern char MFRC500InternalFunctionPiccHalt(void);
    extern void MFRC500InternalFunctionHalt(void);
    
    while (MFRC500InternalFunctionPiccHalt() != MI_OK) {
        delayMs(100);
    }
    MFRC500InternalFunctionHalt();
}

extern void MFRC500GetCardSerialNumber(unsigned char* cardSN)
{
    extern char MFRC500InternalFunctionPiccRequest(unsigned char req_code, unsigned char *atq);
    extern char MFRC500InternalFunctionPiccCascAnticoll(unsigned char bcnt, unsigned char *snr);
    extern void serialPortTransmit(unsigned char* dataToTransmit);
    
    char tmp[2] = "\0\0";
    
	while (MFRC500InternalFunctionPiccRequest(PICC_REQIDL, &cardSN[0]) != MI_OK) {
        delayMs(100);
	}
    //此时 cardSN[0] 为卡类型，2: Mifare Pro 卡、4: Mifare One 卡、16: Mifare Light 卡
    while ((tmp[0] = MFRC500InternalFunctionPiccCascAnticoll(0, &cardSN[1])) != MI_OK) {
        delayMs(100);
	}
    //此时 cardSN[1] 到 cardSN[4] 为四个字节的十六进制卡号
}