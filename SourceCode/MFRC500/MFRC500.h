/**
 * 路径及文件名：  /SourceCode/MFRC500/MFRC500.h
 * 说明：          引用公共头文件、存储地址定义头文件、寄存器定义头文件。
 */

#include "../common/common.h"

/*寄存器及命令定义开始*/
#define RegPage   0x00
#define RegCommand    0x01
#define RegFIFOData   0x02
#define RegPrimaryStatus  0x03
#define RegFIFOLength 0x04
#define RegSecondaryStatus    0x05
#define RegInterruptEn    0x06
#define RegInterruptRq    0x07
#define RegControl    0x09
#define RegErrorFlag  0x0A
#define RegCollPos    0x0B
#define RegTimerValue 0x0C
#define RegCRCResultLSB   0x0D
#define RegCRCResultMSB   0x0E
#define RegBitFraming 0x0F
#define RegTxControl  0x11
#define RegCwConductance  0x12
#define RFU13 0x13
#define RegCoderControl   0x14
#define RegModWidth   0x15
#define RFU16 0x16
#define RFU17 0x17
#define RegRxControl1 0x19
#define RegDecoderControl 0x1A
#define RegBitPhase   0x1B
#define RegRxThreshold    0x1C
#define RFU1D 0x1D
#define RegRxControl2 0x1E
#define RegClockQControl  0x1F
#define RegRxWait 0x21
#define RegChannelRedundancy  0x22
#define RegCRCPresetLSB   0x23
#define RegCRCPresetMSB   0x24
#define RFU25 0x25
#define RegMfOutSelect    0x26
#define RFU27 0x27
#define RegFIFOLevel  0x29
#define RegTimerClock 0x2A
#define RegTimerControl   0x2B
#define RegTimerReload    0x2C
#define RegIRqPinConfig   0x2D
#define RFU2E 0x2E
#define RFU2F 0x2F
#define RFU31 0x31
#define RFU32 0x32
#define RFU33 0x33
#define RFU34 0x34
#define RFU35 0x35
#define RFU36 0x36
#define RFU37 0x37
#define RFU39 0x39
#define RegTestAnaSelect  0x3A
#define RFU3B 0x3B
#define RFU3C 0x3C
#define RegTestDigiSelect 0x3D
#define RFU3E 0x3E
#define RegTestDigiAccess 0x3F
#define DEF_FIFO_LENGTH   64

#define PCD_IDLE  0x00
#define PCD_WRITEE2   0x01
#define PCD_READE2    0x03
#define PCD_LOADCONFIG    0x07
#define PCD_LOADKEYE2 0x0B
#define PCD_AUTHENT1  0x0C
#define PCD_CALCCRC   0x12
#define PCD_AUTHENT2  0x14
#define PCD_RECEIVE   0x16
#define PCD_LOADKEY   0x19
#define PCD_TRANSMIT  0x1A
#define PCD_TRANSCEIVE    0x1E
#define PCD_RESETPHASE    0x3F
#define PICC_REQIDL   0x26   //寻天线区内未进入休眠状态    
#define PICC_REQALL   0x52    //寻天线区内全部卡  
#define PICC_ANTICOLL1    0x93    //防冲撞  
#define PICC_ANTICOLL2    0x95  //防冲撞    
#define PICC_ANTICOLL3    0x97
#define PICC_AUTHENT1A    0x60   //验证A密钥  
#define PICC_AUTHENT1B    0x61    //验证B密钥 
#define PICC_READ 0x30 //读块
#define PICC_WRITE    0xA0 	   //写块
#define PICC_DECREMENT    0xC0 	  //减少
#define PICC_INCREMENT    0xC1   //增加
#define PICC_RESTORE  0xC2   //调块数据到缓冲区  
#define PICC_TRANSFER 0xB0   //保存缓冲区中数据 
#define PICC_HALT 0x50   	   //休眠 
/*寄存器及命令定义结束*/

/*错误代码定义开始*/
#define READER_ERR_BASE_START           0
#define MI_OK                           0
#define MI_CHK_OK                       0
#define MI_CRC_ZERO                     0

#define MI_CRC_NOTZERO                  1

#define MI_NOTAGERR                     1
#define MI_CHK_FAILED                   1
#define MI_CRCERR                       2
#define MI_CHK_COMPERR                  2
#define MI_EMPTY                        3
#define MI_AUTHERR                      4
#define MI_PARITYERR                    5
#define MI_CODEERR                      6

#define MI_SERNRERR                     8
#define MI_KEYERR                       9
#define MI_NOTAUTHERR                   10
#define MI_BITCOUNTERR                  11
#define MI_BYTECOUNTERR                 12
#define MI_IDLE                         13
#define MI_TRANSERR                     14
#define MI_WRITEERR                     15
#define MI_INCRERR                      16
#define MI_DECRERR                      17
#define MI_READERR                      18
#define MI_OVFLERR                      19
#define MI_POLLING                      20
#define MI_FRAMINGERR                   21
#define MI_ACCESSERR                    22
#define MI_UNKNOWN_COMMAND              23
#define MI_COLLERR                      24
#define MI_RESETERR                     25
#define MI_INITERR                      25
#define MI_INTERFACEERR                 26
#define MI_ACCESSTIMEOUT                27
#define MI_NOBITWISEANTICOLL            28
#define MI_QUIT                         30
#define MI_RECBUF_OVERFLOW              50 
#define MI_SENDBYTENR                   51
	
#define MI_SENDBUF_OVERFLOW             53
#define MI_BAUDRATE_NOT_SUPPORTED       54
#define MI_SAME_BAUDRATE_REQUIRED       55

#define MI_WRONG_PARAMETER_VALUE        60

#define MI_BREAK                        99
#define MI_NY_IMPLEMENTED               100
#define MI_NO_MFRC                      101
#define MI_MFRC_NOTAUTH                 102
#define MI_WRONG_DES_MODE               103
#define MI_HOST_AUTH_FAILED             104

#define MI_WRONG_LOAD_MODE              106
#define MI_WRONG_DESKEY                 107
#define MI_MKLOAD_FAILED                108
#define MI_FIFOERR                      109
#define MI_WRONG_ADDR                   110
#define MI_DESKEYLOAD_FAILED            111

#define MI_WRONG_SEL_CNT                114

#define MI_WRONG_TEST_MODE              117
#define MI_TEST_FAILED                  118
#define MI_TOC_ERROR                    119
#define MI_COMM_ABORT                   120
#define MI_INVALID_BASE                 121
#define MI_MFRC_RESET                   122
#define MI_WRONG_VALUE                  123
#define MI_VALERR                       124
#define MI_SAKERR                       0x46	//added by robbie
/*错误代码定义结束*/

/*取页宏函数定义开始*/
#define GetRegPage(addr)    (0x80 | (addr>>3))
/*取页宏函数定义结束*/
