/**
 * 路径及文件名：  /SourceCode/serialPort/serialPort.c
 * 说明：          定义有关串口通讯的函数。
 * 函数列表：      extern void serialPortInit(void)
 *                 extern unsigned char* serialPortReceive(void)
 *                 extern void serialPortTransmit(unsigned char* dataToTransmit, bit isString, unsigned char dataLenth)
 *                 static void serialPortInterruptHandler(void) interrupt 4
 */

#include "serialPort.h"

unsigned char xdata serialPortReceiverBuffer[32];
unsigned char* serialPortReceiverBufferPointer = serialPortReceiverBuffer;
bit isSerialPortDataReceivedCompletely = 0;

extern void serialPortInit(void)
{
    TMOD = 0x20;
    SCON = 0x50;
    TH1 = SERIALPORT_TIMER1_HIGHBYTE;
    TL1 = TH1;
    PCON = 0x00;
    TR1 = 1;
}

extern unsigned char* serialPortReceive(void)
{
    serialPortReceiverBuffer[0] = '\0';
    isSerialPortDataReceivedCompletely = 0;
    ES = 1;
    while (!isSerialPortDataReceivedCompletely) {
        ;
    }
    ES = 0;
    return serialPortReceiverBuffer;
}

extern void serialPortTransmit(unsigned char* dataToTransmit, bit isString, unsigned char dataLenth)
{
    unsigned char* dataCopy = dataToTransmit;
    
    if (isString) {
        dataLenth = 64;
        while (dataLenth--) {
            if (*dataCopy == '\0') {
                return;
            }
            SBUF = *dataCopy;
            dataCopy++;
            while (!TI) {
                ;
            }
            TI = 0;
        }
    } else {
        while (dataLenth--) {
            SBUF = *dataCopy;
            dataCopy++;
            while (!TI) {
                ;
            }
            TI = 0;
        }
    }
    

}

static void serialPortInterruptHandler(void) interrupt 4
{
    if (RI) {
        RI = 0;
        *serialPortReceiverBufferPointer = SBUF;
        if (*serialPortReceiverBufferPointer == '\0') {
            isSerialPortDataReceivedCompletely = 1;
            serialPortReceiverBufferPointer = serialPortReceiverBuffer;
            return;
        }
        serialPortReceiverBufferPointer++;
    }
}
