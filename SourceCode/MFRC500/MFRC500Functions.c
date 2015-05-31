#include "MFRC500.h"

sbit MFRC500RSTPD = MFRC500_PIN_RSTPD;

typedef struct {
    unsigned char  cmd;
    char           status;
    unsigned char  nBytesSent;
    unsigned char  nBytesToSend;
    unsigned char  nBytesReceived;
    unsigned short nBitsReceived;
    unsigned char  irqSource;
    unsigned char  collPos;

} MfCmdInfo;

#define ResetInfo(info)                     \
            info.cmd            = 0;        \
            info.status         = MI_OK;    \
            info.irqSource      = 0;        \
            info.nBytesSent     = 0;        \
            info.nBytesToSend   = 0;        \
            info.nBytesReceived = 0;        \
            info.nBitsReceived  = 0;        \
            info.collPos        = 0;

static volatile MfCmdInfo idata   MInfo;
static volatile MfCmdInfo         *MpIsrInfo;
static volatile unsigned char     *MpIsrOut;

unsigned char SerBuffer[20];

// 往一个地址写一个数据
static void MFRC500InternalFunctionWriteRawIO(unsigned char Address, unsigned char value)
{
    XBYTE[0x7F00 + Address] = value;
}

// 从一个地址读出一个数据
static unsigned char MFRC500InternalFunctionReadRawIO(unsigned char Address)
{
    return XBYTE[0x7F00 + Address];
}

// 往一个地址写一个数据(EEPROM)
static void MFRC500InternalFunctionWriteIO(unsigned char Address, unsigned char value)
{
    MFRC500InternalFunctionWriteRawIO(0x00, GetRegPage(Address));
    MFRC500InternalFunctionWriteRawIO(Address, value);
}

// 从一个地址读出一个数据(EEPROM)
static unsigned char MFRC500InternalFunctionReadIO(unsigned char Address)
{
    MFRC500InternalFunctionWriteRawIO(0x00, GetRegPage(Address));
    return MFRC500InternalFunctionReadRawIO(Address);
}

// 设置定时时间
static void MFRC500InternalFunctionPcdSetTmo(unsigned char tmoLength)
{
    switch(tmoLength) {
    case 1:
        MFRC500InternalFunctionWriteIO(RegTimerClock, 0x07);
        MFRC500InternalFunctionWriteIO(RegTimerReload, 0x6a);
        break;
    case 2:
        MFRC500InternalFunctionWriteIO(RegTimerClock, 0x07);
        MFRC500InternalFunctionWriteIO(RegTimerReload, 0xa0);
        break;
    case 3:
        MFRC500InternalFunctionWriteIO(RegTimerClock, 0x09);
        MFRC500InternalFunctionWriteIO(RegTimerReload, 0xa0);
        break;
    case 4:
        MFRC500InternalFunctionWriteIO(RegTimerClock, 0x09);
        MFRC500InternalFunctionWriteIO(RegTimerReload, 0xff);
        break;
    case 5:
        MFRC500InternalFunctionWriteIO(RegTimerClock, 0x0b);
        MFRC500InternalFunctionWriteIO(RegTimerReload, 0xff);
        break;
    case 6:
        MFRC500InternalFunctionWriteIO(RegTimerClock, 0x0d);
        MFRC500InternalFunctionWriteIO(RegTimerReload, 0xff);
        break;
    case 7:
        MFRC500InternalFunctionWriteIO(RegTimerClock, 0x0f);
        MFRC500InternalFunctionWriteIO(RegTimerReload, 0xff);
        break;
    default:
        MFRC500InternalFunctionWriteIO(RegTimerClock, 0x07);
        MFRC500InternalFunctionWriteIO(RegTimerReload, tmoLength);
        break;
    }
}

// Request Command defined in ISO14443(Mifare)
static char MFRC500InternalFunctionPcdCmd(
    unsigned char cmd,
    volatile unsigned char data *rcv,
    MfCmdInfo idata *info
) {
    static void MFRC500InternalFunctionFlushFIFO(void);
    static char MFRC500InternalFunctionSetBitMask(unsigned char reg, unsigned char mask);
    
    char          idata status    = MI_OK;
    char          idata tmpStatus ;
    unsigned char idata lastBits;
    unsigned int  idata timecnt = 0;
    unsigned char idata irqEn = 0x00;
    unsigned char idata waitFor = 0x00;
    unsigned char idata timerCtl  = 0x00;

    MFRC500InternalFunctionWriteIO(RegInterruptEn, 0x7F);
    MFRC500InternalFunctionWriteIO(RegInterruptRq, 0x7F);
    MFRC500InternalFunctionWriteIO(RegCommand, PCD_IDLE);

    MFRC500InternalFunctionFlushFIFO();
    MpIsrInfo = info;
    MpIsrOut = rcv;
    info->irqSource = 0x00;
    switch(cmd) {
    case PCD_IDLE:
        irqEn = 0x00;
        waitFor = 0x00;
        break;
    case PCD_WRITEE2:
        irqEn = 0x11;
        waitFor = 0x10;
        break;
    case PCD_READE2:
        irqEn = 0x07;
        waitFor = 0x04;
        break;
    case PCD_LOADCONFIG:
    case PCD_LOADKEYE2:
    case PCD_AUTHENT1:
        irqEn = 0x05;
        waitFor = 0x04;
        break;
    case PCD_CALCCRC:
        irqEn = 0x11;
        waitFor = 0x10;
        break;
    case PCD_AUTHENT2:
        irqEn = 0x04;
        waitFor = 0x04;
        break;
    case PCD_RECEIVE:
        info->nBitsReceived = -(MFRC500InternalFunctionReadIO(RegBitFraming) >> 4);
        irqEn = 0x06;
        waitFor = 0x04;
        break;
    case PCD_LOADKEY:
        irqEn = 0x05;
        waitFor = 0x04;
        break;
    case PCD_TRANSMIT:
        irqEn = 0x05;
        waitFor = 0x04;
        break;
    case PCD_TRANSCEIVE:
        info->nBitsReceived = -(MFRC500InternalFunctionReadIO(RegBitFraming) >> 4);
        irqEn = 0x3D;
        waitFor = 0x04;
        break;
    default:
        status = MI_UNKNOWN_COMMAND;
    }
    if (status == MI_OK) {
        irqEn |= 0x20;
        waitFor |= 0x20;
        timecnt = 1000;
        MFRC500InternalFunctionWriteIO(RegInterruptEn, irqEn | 0x80);
        MFRC500InternalFunctionWriteIO(RegCommand, cmd);
        while (!(MpIsrInfo->irqSource & waitFor || !(timecnt--)));
        MFRC500InternalFunctionWriteIO(RegInterruptEn, 0x7F);
        MFRC500InternalFunctionWriteIO(RegInterruptRq, 0x7F);
        MFRC500InternalFunctionSetBitMask(RegControl, 0x04);
        MFRC500InternalFunctionWriteIO(RegCommand, PCD_IDLE);
        if (!(MpIsrInfo->irqSource & waitFor)) {
            status = MI_ACCESSTIMEOUT;
        } else {
            status = MpIsrInfo->status;
        }
        if (status == MI_OK) {
            if (tmpStatus = (MFRC500InternalFunctionReadIO(RegErrorFlag) & 0x17)) {
                if (tmpStatus & 0x01) {
                    info->collPos = MFRC500InternalFunctionReadIO(RegCollPos);
                    status = MI_COLLERR;
                } else {
                    info->collPos = 0;
                    if (tmpStatus & 0x02) {
                        status = MI_PARITYERR;
                    }
                }
                if (tmpStatus & 0x04) {
                    status = MI_FRAMINGERR;
                }
                if (tmpStatus & 0x10) {
                    MFRC500InternalFunctionFlushFIFO();
                    status = MI_OVFLERR;
                }
                if (tmpStatus & 0x08) {
                    status = MI_CRCERR;
                }
                if (status == MI_OK)
                    status = MI_NY_IMPLEMENTED;
            }
            if (cmd == PCD_TRANSCEIVE) {
                lastBits = MFRC500InternalFunctionReadIO(RegSecondaryStatus) & 0x07;
                if (lastBits)
                    info->nBitsReceived += (info->nBytesReceived - 1) * 8 + lastBits;
                else
                    info->nBitsReceived += info->nBytesReceived * 8;
            }
        } else {
            info->collPos = 0x00;
        }
    }
    MpIsrInfo = 0;
    MpIsrOut  = 0;
    return status;
}

// 置一个bit
static char MFRC500InternalFunctionSetBitMask(unsigned char reg, unsigned char mask)
{
    char idata tmp = 0x00;

    tmp = MFRC500InternalFunctionReadIO(reg);
    MFRC500InternalFunctionWriteIO(reg, tmp | mask); // set bit mask
    return 0x00;
}

// 清一个bit
static char MFRC500InternalFunctionClearBitMask(unsigned char reg, unsigned char mask)
{
    char idata tmp = 0x00;

    tmp = MFRC500InternalFunctionReadIO(reg);
    MFRC500InternalFunctionWriteIO(reg, tmp & ~mask); // clear bit mask
    return 0x00;
}

//清除FIFO
static void MFRC500InternalFunctionFlushFIFO(void)
{
    MFRC500InternalFunctionSetBitMask(RegControl, 0x01);
}

/*// Value format operations for Mifare Standard card ICs
extern char MFRC500InternalFunctionPiccValue(
    unsigned char dd_mode,
    unsigned char addr,
    unsigned char *value,
    unsigned char trans_addr
) {
    char status = MI_OK;

    MFRC500InternalFunctionPcdSetTmo(1);
    ResetInfo(MInfo);
    SerBuffer[0] = dd_mode;
    SerBuffer[1] = addr;
    MInfo.nBytesToSend = 2;
    status = MFRC500InternalFunctionPcdCmd(PCD_TRANSCEIVE,
                        SerBuffer,
                        &MInfo);

    if (status != MI_NOTAGERR) {
        if (MInfo.nBitsReceived != 4) {
            status = MI_BITCOUNTERR;
        } else {
            SerBuffer[0] &= 0x0f;
            switch(SerBuffer[0]) {
            case 0x00:
                status = MI_NOTAUTHERR;
                break;
            case 0x0a:
                status = MI_OK;
                break;
            case 0x01:
                status = MI_VALERR;
                break;
            default:
                status = MI_CODEERR;
                break;
            }
        }
    }

    if ( status == MI_OK) {
        MFRC500InternalFunctionPcdSetTmo(3);
        ResetInfo(MInfo);
        memcpy(SerBuffer, value, 4);
        MInfo.nBytesToSend   = 4;
        status = MFRC500InternalFunctionPcdCmd(PCD_TRANSCEIVE,
                            SerBuffer,
                            &MInfo);

        if (status == MI_OK) {
            if (MInfo.nBitsReceived != 4) {
                status = MI_BITCOUNTERR;
            } else {
                SerBuffer[0] &= 0x0f;
                switch(SerBuffer[0]) {
                case 0x00:
                    status = MI_NOTAUTHERR;
                    break;
                case 0x01:
                    status = MI_VALERR;
                    break;
                default:
                    status = MI_CODEERR;
                    break;
                }
            }
        } else {
            if (status == MI_NOTAGERR )
                status = MI_OK;
        }
    }
    if (status == MI_OK) {
        ResetInfo(MInfo);
        SerBuffer[0] = PICC_TRANSFER;
        SerBuffer[1] = trans_addr;
        MInfo.nBytesToSend   = 2;
        status = MFRC500InternalFunctionPcdCmd(PCD_TRANSCEIVE,
                            SerBuffer,
                            &MInfo);
        if (status != MI_NOTAGERR) {
            if (MInfo.nBitsReceived != 4) {
                status = MI_BITCOUNTERR;
            } else {
                SerBuffer[0] &= 0x0f;
                switch(SerBuffer[0]) {
                case 0x00:
                    status = MI_NOTAUTHERR;
                    break;
                case 0x0a:
                    status = MI_OK;
                    break;
                case 0x01:
                    status = MI_VALERR;
                    break;
                default:
                    status = MI_CODEERR;
                    break;
                }
            }
        }
    }
    return status;
}*/

// 终止卡的操作
extern char MFRC500InternalFunctionPiccHalt(void)
{
    char idata status = MI_CODEERR;

    // ************* Cmd Sequence **********************************
    ResetInfo(MInfo);
    SerBuffer[0] = PICC_HALT ;      // Halt command code
    SerBuffer[1] = 0x00;            // dummy address
    MInfo.nBytesToSend = 2;
    status = MFRC500InternalFunctionPcdCmd(PCD_TRANSCEIVE,
                        SerBuffer,
                        &MInfo);
    if (status) {
        // timeout error ==> no NAK received ==> OK
        if (status == MI_NOTAGERR || status == MI_ACCESSTIMEOUT) {
            status = MI_OK;
        }
    }
    //reset command register - no response from tag
    MFRC500InternalFunctionWriteIO(RegCommand, PCD_IDLE);	//启动和停止命令的执行
    return status;
}

//重启RC500
static char MFRC500InternalFunctionPcdReset(void)
{
    char idata status = MI_OK;
    unsigned int idata timecnt = 0;

    MFRC500RSTPD = HIGH;
    MFRC500RSTPD = LOW;
    delayMs(10);
    timecnt = 1000;
    while ((MFRC500InternalFunctionReadIO(RegCommand) & 0x3F) && timecnt--);
    if(!timecnt) {
        status = MI_RESETERR;
    }
    if (status == MI_OK) {
        if (MFRC500InternalFunctionReadIO(RegCommand) != 0x00) {
            status = MI_INTERFACEERR;
        }
    }
    return status;
}

/*//将一个密匙从E2PROM 复制到密匙缓冲区
extern char MFRC500InternalFunctionPcdLoadKeyE2(
    unsigned char key_type,
    unsigned char sector,
    unsigned char *uncoded_keys
) {
    extern char MFRC500InternalFunctionHostCodeKey(unsigned char *uncoded, unsigned char *coded);
    static char MFRC500InternalFunctionPcdWriteE2(unsigned int startaddr, unsigned char length, unsigned char *_data);
    
    signed char status = MI_OK;
    unsigned int e2addr = 0x80 + sector * 0x18;
    unsigned char coded_keys[12];

    if (key_type == PICC_AUTHENT1B) {
        e2addr += 12;           // key B offset
    }
    if ((status = MFRC500InternalFunctionHostCodeKey(uncoded_keys, coded_keys)) == MI_OK) {
        status = MFRC500InternalFunctionPcdWriteE2(e2addr, 12, coded_keys);
    }
    return status;
}*/

/*//从FIFO 缓冲区获得数据并写入内部 E2PROM
static char MFRC500InternalFunctionPcdWriteE2(
    unsigned int startaddr,
    unsigned char length,
    unsigned char *_data
) {
    char status = MI_OK;
    ResetInfo(MInfo);
    SerBuffer[0] = startaddr & 0xFF;
    SerBuffer[1] = (startaddr >> 8) & 0xFF;
    memcpy(SerBuffer + 2, _data, length);

    MInfo.nBytesToSend   = length + 2;

    status = MFRC500InternalFunctionPcdCmd(PCD_WRITEE2,
                        SerBuffer,
                        &MInfo);
    return status;
}*/

// 寻卡，防冲突，选择卡    返回卡类型（2 bytes）+ 卡系列号(4 bytes)
extern char MFRC500InternalFunctionPiccRequest(unsigned char req_code, unsigned char *atq)
{
    char idata status = MI_OK;

    MFRC500InternalFunctionPcdSetTmo(3);
    MFRC500InternalFunctionWriteIO(RegChannelRedundancy, 0x03);
    MFRC500InternalFunctionClearBitMask(RegControl, 0x08);
    MFRC500InternalFunctionWriteIO(RegBitFraming, 0x07);
    MFRC500InternalFunctionSetBitMask(RegTxControl, 0x03);
    ResetInfo(MInfo);
    SerBuffer[0] = req_code;
    MInfo.nBytesToSend = 1;

    status = MFRC500InternalFunctionPcdCmd(PCD_TRANSCEIVE, SerBuffer, &MInfo);
    if (status) {
        *atq = 0;
    } else {
        if (MInfo.nBitsReceived != 16) {
            *atq = 0;
            status = MI_BITCOUNTERR;
        } else {
            status = MI_OK;
            memcpy(atq, SerBuffer, 2);
        }
    }
    return status;
}

// 防冲突 读卡的系列号 MLastSelectedSnr
extern char MFRC500InternalFunctionPiccCascAnticoll(unsigned char bcnt, unsigned char *snr)
{
    char idata status = MI_OK;
    char idata snr_in[4];
    char idata nbytes = 0;
    char idata nbits = 0;
    char idata complete = 0;
    char idata i        = 0;
    char idata byteOffset = 0;
    unsigned char dummyShift1;
    unsigned char dummyShift2;

    MFRC500InternalFunctionPcdSetTmo(106);
    memcpy(snr_in, snr, 4);

    MFRC500InternalFunctionWriteIO(RegDecoderControl, 0x28); //在一个位冲突之后的任何位都屏蔽为0
    MFRC500InternalFunctionClearBitMask(RegControl, 0x08);
    complete = 0;
    while (!complete && (status == MI_OK) ) {
        ResetInfo(MInfo);
        MFRC500InternalFunctionWriteIO(RegChannelRedundancy, 0x03); //选择RF 信道上数据完整性检测的类型和模式,单独产生或者出现奇数的奇偶校验
        nbits = bcnt % 8;
        if(nbits) {
            MFRC500InternalFunctionWriteIO(RegBitFraming, nbits << 4 | nbits); //位方式帧的调节,
            nbytes = bcnt / 8 + 1;
            if (nbits == 7) {
                MInfo.cmd = PICC_ANTICOLL1;
                MFRC500InternalFunctionWriteIO(RegBitFraming, nbits);
            }
        } else {
            nbytes = bcnt / 8;
        }
        SerBuffer[0] = 0x93;
        SerBuffer[1] = 0x20 + ((bcnt / 8) << 4) + nbits;

        for (i = 0; i < nbytes; i++) {
            SerBuffer[i + 2] = snr_in[i];
        }
        MInfo.nBytesToSend   = 2 + nbytes;

        status = MFRC500InternalFunctionPcdCmd(PCD_TRANSCEIVE, SerBuffer, &MInfo);
        if (nbits == 7) {
            dummyShift1 = 0x00;
            for (i = 0; i < MInfo.nBytesReceived; i++) {
                dummyShift2 = SerBuffer[i];
                SerBuffer[i] = (dummyShift1 >> (i + 1)) | (SerBuffer[i] << (7 - i));
                dummyShift1 = dummyShift2;
            }
            MInfo.nBitsReceived -= MInfo.nBytesReceived;
            if (MInfo.collPos ) MInfo.collPos += 7 - (MInfo.collPos + 6) / 9;
        }
        if (status == MI_OK || status == MI_COLLERR) {
            if (MInfo.nBitsReceived != (40 - bcnt)) {
                status = MI_BITCOUNTERR;
            } else {
                byteOffset = 0;
                if(nbits != 0 ) {
                    snr_in[nbytes - 1] = snr_in[nbytes - 1] | SerBuffer[0];
                    byteOffset = 1;
                }

                for (i = 0; i < (4 - nbytes); i++) {
                    snr_in[nbytes + i] = SerBuffer[i + byteOffset];
                }

                if (status != MI_COLLERR ) {
                    dummyShift2 = snr_in[0] ^ snr_in[1] ^ snr_in[2] ^ snr_in[3];
                    dummyShift1 = SerBuffer[MInfo.nBytesReceived - 1];
                    if (dummyShift2 != dummyShift1) {
                        status = MI_SERNRERR;
                    } else {
                        complete = 1;
                    }
                } else {
                    bcnt = bcnt + MInfo.collPos - nbits;
                    status = MI_OK;
                }
            }
        }
    }
    if (status == MI_OK) {
        memcpy(snr, snr_in, 4);
    } else {
        memcpy(snr, "0000", 4);
    }
    MFRC500InternalFunctionClearBitMask(RegDecoderControl, 0x20);

    return status;
}

/*// 选择卡 Select Card
extern char MFRC500InternalFunctionPiccCascSelect(unsigned char *snr, unsigned char *sak)
{
    char idata status = MI_OK;

    MFRC500InternalFunctionPcdSetTmo(106);

    MFRC500InternalFunctionWriteIO(RegChannelRedundancy, 0x0F);
    MFRC500InternalFunctionClearBitMask(RegControl, 0x08);
    ResetInfo(MInfo);
    SerBuffer[0] = 0x93;
    SerBuffer[1] = 0x70;

    memcpy(SerBuffer + 2, snr, 4);
    SerBuffer[6] = SerBuffer[2] ^ SerBuffer[3] ^ SerBuffer[4] ^ SerBuffer[5];
    MInfo.nBytesToSend = 7;
    status = MFRC500InternalFunctionPcdCmd(PCD_TRANSCEIVE, SerBuffer, &MInfo);

    *sak = 0;
    if (status == MI_OK) {
        if (MInfo.nBitsReceived != 8) {
            status = MI_BITCOUNTERR;
        } else {
            *sak = SerBuffer[0];
        }
    }
    return status;
}*/

/*// 校验卡密码(E2)
extern char MFRC500InternalFunctionPiccAuthE2(
    unsigned char auth_mode,
    unsigned char *snr,
    unsigned char keynr,
    unsigned char block
) {
    static char MFRC500InternalFunctionPiccAuthState( unsigned char auth_mode, unsigned char *snr, unsigned char block);
    static void MFRC500InternalFunctionFlushFIFO(void);
    
    char idata status = MI_OK;
    unsigned int e2addr = 0x80 + keynr * 0x18;
    unsigned char *e2addrbuf;

    e2addrbuf = (unsigned char *)&e2addr;
    if (auth_mode == PICC_AUTHENT1B) {
        e2addr += 12;
    }
    MFRC500InternalFunctionFlushFIFO();
    ResetInfo(MInfo);

    memcpy(SerBuffer, e2addrbuf, 2);
    SerBuffer[2] = SerBuffer[0];
    SerBuffer[0] = SerBuffer[1];
    SerBuffer[1] = SerBuffer[2];
    MInfo.nBytesToSend   = 2;
    if ((status = MFRC500InternalFunctionPcdCmd(PCD_LOADKEYE2, SerBuffer, &MInfo)) == MI_OK) {
        status = MFRC500InternalFunctionPiccAuthState(auth_mode, snr, block); //11.9
    }
    return status;
}*/

/*// Authentication key coding
extern char MFRC500InternalFunctionHostCodeKey(unsigned char *uncoded, unsigned char *coded)
{
    char idata status = MI_OK;
    unsigned char idata cnt = 0;
    unsigned char idata ln  = 0;
    unsigned char idata hn  = 0;
    for (cnt = 0; cnt < 6; cnt++) {
        ln = uncoded[cnt] & 0x0F;
        hn = uncoded[cnt] >> 4;
        coded[cnt * 2 + 1] = (~ln << 4) | ln;
        coded[cnt * 2 ] = (~hn << 4) | hn;
    }
    return MI_OK;
}*/

/*// 直接校验密码
extern char MFRC500InternalFunctionPiccAuthKey(
    unsigned char auth_mode,
    unsigned char *snr,
    unsigned char *keys,
    unsigned char block
) {
    static void MFRC500InternalFunctionFlushFIFO(void);
    
    char idata status = MI_OK;
    
    MFRC500InternalFunctionFlushFIFO();
    ResetInfo(MInfo);
    memcpy(SerBuffer, keys, 12);
    MInfo.nBytesToSend = 12;
    if ((status = MFRC500InternalFunctionPcdCmd(PCD_LOADKEY, SerBuffer, &MInfo)) == MI_OK) {
        status = MFRC500InternalFunctionPiccAuthState(auth_mode, snr, block);
    }
    return status;
}*/

/*static char MFRC500InternalFunctionPiccAuthState( unsigned char auth_mode, unsigned char *snr, unsigned char block)
{
    char idata status = MI_OK;
    unsigned char idata i = 0;

    status = MFRC500InternalFunctionReadIO(RegErrorFlag);
    if (status != MI_OK) {
        if (status & 0x40) {
            status = MI_KEYERR;
        } else {
            status = MI_AUTHERR;
        }
    } else {
        SerBuffer[0] = auth_mode;

        SerBuffer[1] = block;
        memcpy(SerBuffer + 2, snr, 4);
        ResetInfo(MInfo);
        MInfo.nBytesToSend = 6;
        if ((status = MFRC500InternalFunctionPcdCmd(PCD_AUTHENT1, SerBuffer, &MInfo)) == MI_OK) {
            if (MFRC500InternalFunctionReadIO(RegSecondaryStatus) & 0x07) {
                status = MI_BITCOUNTERR;
            } else {
                ResetInfo(MInfo);
                MInfo.nBytesToSend = 0;
                if ((status = MFRC500InternalFunctionPcdCmd(PCD_AUTHENT2,
                                         SerBuffer,
                                         &MInfo)) == MI_OK) {
                    if ( MFRC500InternalFunctionReadIO(RegControl) & 0x08 ) {
                        status = MI_OK;
                    } else {
                        status = MI_AUTHERR;
                    }
                }
            }
        }
    }
    return status;
}*/

/*// 读卡
extern char MFRC500InternalFunctionPiccRead(unsigned char addr, unsigned char *_data)
{
    static void MFRC500InternalFunctionFlushFIFO(void);
    
    char idata status = MI_OK;
    char idata tmp    = 0;

    MFRC500InternalFunctionFlushFIFO();

    MFRC500InternalFunctionPcdSetTmo(3);
    MFRC500InternalFunctionWriteIO(RegChannelRedundancy, 0x0F);
    ResetInfo(MInfo);
    SerBuffer[0] = PICC_READ;
    SerBuffer[1] = addr;
    MInfo.nBytesToSend   = 2;
    status = MFRC500InternalFunctionPcdCmd(PCD_TRANSCEIVE, SerBuffer, &MInfo);

    if (status != MI_OK) {
        if (status != MI_NOTAGERR ) {
            if (MInfo.nBitsReceived == 4) {
                SerBuffer[0] &= 0x0f;
                if ((SerBuffer[0] & 0x0a) == 0) {
                    status = MI_NOTAUTHERR;
                } else {
                    status = MI_CODEERR;
                }
            }
        }
        memcpy(_data, "0000000000000000", 16);
    } else {            // Response Processing
        if (MInfo.nBytesReceived != 16) {
            status = MI_BYTECOUNTERR;
            memcpy(_data, "0000000000000000", 16);
        } else {
            memcpy(_data, SerBuffer, 16);
        }
    }
    MFRC500InternalFunctionPcdSetTmo(1);
    return status;
}*/

/*// 写卡  下载密码
extern char MFRC500InternalFunctionPiccWrite(unsigned char addr, unsigned char *_data)
{
    char idata status = MI_OK;

    ResetInfo(MInfo);
    SerBuffer[0] = PICC_WRITE;
    SerBuffer[1] = addr;
    MInfo.nBytesToSend   = 2;
    status = MFRC500InternalFunctionPcdCmd(PCD_TRANSCEIVE, SerBuffer, &MInfo);

    if (status != MI_NOTAGERR) {
        if (MInfo.nBitsReceived != 4) {
            status = MI_BITCOUNTERR;
        } else {
            SerBuffer[0] &= 0x0f;
            if ((SerBuffer[0] & 0x0a) == 0) {
                status = MI_NOTAUTHERR;
            } else {
                if (SerBuffer[0] == 0x0a) {
                    status = MI_OK;
                } else {
                    status = MI_CODEERR;
                }
            }
        }
    }

    if (status == MI_OK) {
        MFRC500InternalFunctionPcdSetTmo(3);

        ResetInfo(MInfo);
        memcpy(SerBuffer, _data, 16);
        MInfo.nBytesToSend   = 16;
        status = MFRC500InternalFunctionPcdCmd(PCD_TRANSCEIVE, SerBuffer, &MInfo);

        if (status & 0x80) {
            status = MI_NOTAGERR;
        } else {
            if (MInfo.nBitsReceived != 4) {
                status = MI_BITCOUNTERR;
            } else {
                SerBuffer[0] &= 0x0f;
                if ((SerBuffer[0] & 0x0a) == 0) {
                    status = MI_WRITEERR;
                } else {
                    if (SerBuffer[0] == 0x0a) {
                        status = MI_OK;
                    } else {
                        status = MI_CODEERR;
                    }
                }
            }
        }
        MFRC500InternalFunctionPcdSetTmo(1);
    }
    return status;
}*/

// Reset Rf Card
static char MFRC500InternalFunctionPcdRfReset(unsigned char ms)
{
    char idata status = MI_OK;

    if(ms) {
        MFRC500InternalFunctionClearBitMask(RegTxControl, 0x03);
        delayMs(2);
        MFRC500InternalFunctionSetBitMask(RegTxControl, 0x03);
    } else {
        MFRC500InternalFunctionClearBitMask(RegTxControl, 0x03);
    }
    return status;
}

// 启动MFRC500
extern char MFRC500InternalFunctionStart(void)
{
    char status = MI_OK;

    EX0 = 1;
    IT0 = 1;

    if ((status = MFRC500InternalFunctionPcdReset()) == MI_OK) {
        MFRC500InternalFunctionWriteIO(RegClockQControl, 0x00);
        MFRC500InternalFunctionWriteIO(RegClockQControl, 0x40);
        delayMs(1);
        MFRC500InternalFunctionClearBitMask(RegClockQControl, 0x40);
        MFRC500InternalFunctionWriteIO(RegRxControl1, 0x73);
        MFRC500InternalFunctionWriteIO(RegRxControl2, 0x41);
        MFRC500InternalFunctionWriteIO(RegBitPhase, 0xAD);
        MFRC500InternalFunctionWriteIO(RegRxThreshold, 0xFF);
        MFRC500InternalFunctionWriteIO(RegRxControl2, 0x01);
        MFRC500InternalFunctionWriteIO(RegFIFOLevel, 0x1A);
        MFRC500InternalFunctionWriteIO(RegTimerControl, 0x02);
        MFRC500InternalFunctionWriteIO(RegIRqPinConfig, 0x03);
        MFRC500InternalFunctionPcdRfReset(1);
    }
    return status;
}

// 终止RC500
extern void MFRC500InternalFunctionHalt(void)
{
    MFRC500RSTPD = HIGH;
    EX0 = 0;
}

//RC500中断请求 外中断0
static void MFRC500InternalFunctionInterruptHandler(void) interrupt 0
{
    static unsigned char idata irqBits;
    static unsigned char idata irqMask;
    static unsigned char idata nbytes;
    static unsigned char idata cnt;

    IE0 = 0;
    MFRC500InternalFunctionWriteRawIO(0, 0x80);		//选择寄存器页0
    if (MpIsrInfo && MpIsrOut) {
        //PrimaryStatus 寄存器，读取接收器、发送器和FIFO 缓冲区状态标志
        while( MFRC500InternalFunctionReadRawIO(RegPrimaryStatus) & 0x08) {	//test IRQ
            irqMask = MFRC500InternalFunctionReadRawIO(RegInterruptEn);	//使能和禁止中断请求通过的控制位
            irqBits = MFRC500InternalFunctionReadRawIO(RegInterruptRq) & irqMask;	//中断请求标志
            MpIsrInfo->irqSource |= irqBits;
            if (irqBits & 0x01) {
                nbytes = 64 - MFRC500InternalFunctionReadRawIO(RegFIFOLength);		//FIFO 中的缓冲字节数
                if ((MpIsrInfo->nBytesToSend - MpIsrInfo->nBytesSent) <= nbytes) {
                    nbytes = MpIsrInfo->nBytesToSend - MpIsrInfo->nBytesSent;
                    MFRC500InternalFunctionWriteRawIO(RegInterruptEn, 0x01);	//允许将LoAlert中断请求传递给脚IRQ
                }
                for ( cnt = 0; cnt < nbytes; cnt++) {
                    MFRC500InternalFunctionWriteRawIO(RegFIFOData, MpIsrOut[MpIsrInfo->nBytesSent]);	//FIFO 缓冲区
                    MpIsrInfo->nBytesSent++;
                }
                MFRC500InternalFunctionWriteRawIO(RegInterruptRq, 0x01);  	//中断请求标志
            }
            if (irqBits & 0x10) {
                MFRC500InternalFunctionWriteRawIO(RegInterruptRq, 0x10);
                MFRC500InternalFunctionWriteRawIO(RegInterruptEn, 0x82);
                if (MpIsrInfo->cmd == PICC_ANTICOLL1) {
                    MFRC500InternalFunctionWriteIO(RegChannelRedundancy, 0x02);	//选择RF 信道上数据完整性检测的类型和模式
                    MFRC500InternalFunctionWriteRawIO(0, 0x80);
                }
            }
            if (irqBits & 0x0E) {
                nbytes = MFRC500InternalFunctionReadRawIO(RegFIFOLength);
                for ( cnt = 0; cnt < nbytes; cnt++) {
                    MpIsrOut[MpIsrInfo->nBytesReceived] = MFRC500InternalFunctionReadRawIO(RegFIFOData);
                    MpIsrInfo->nBytesReceived++;
                }
                MFRC500InternalFunctionWriteRawIO(RegInterruptRq, 0x0A & irqBits);
            }
            if (irqBits & 0x04) {
                MFRC500InternalFunctionWriteRawIO(RegInterruptEn, 0x20);
                MFRC500InternalFunctionWriteRawIO(RegInterruptRq, 0x20);
                irqBits &= ~0x20;
                MpIsrInfo->irqSource &= ~0x20;
                MFRC500InternalFunctionWriteRawIO(RegInterruptRq, 0x04);
            }
            if (irqBits & 0x20) {
                MFRC500InternalFunctionWriteRawIO(RegInterruptRq, 0x20);
                MpIsrInfo->status = MI_NOTAGERR;
            }
        }
    }
}