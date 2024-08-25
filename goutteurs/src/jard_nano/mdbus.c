/**
* @file mdbus.c
* @brief Implementation of a very tiny modbus slave/server for Arduino
*/
#include "mdbus.h"

#include <stdio.h>
#include <string.h>

#define ENDIAN_INVERSION

#define MDBUS_FUNCT_READ_COIL_STATUS            (1)
#define MDBUS_FUNCT_READ_INPUT_STATUS           (2)
#define MDBUS_FUNCT_READ_HOLDING_REGISTERS      (3)
#define MDBUS_FUNCT_READ_INPUT_REGISTERS        (4)
#define MDBUS_FUNCT_WRITE_MULTIPLE_COILS        (15)
#define MDBUS_FUNCT_WRITE_MULTIPLE_REGISTERS    (16)

#define MDBUS_EXCEPT_BAD_FUNCTION               (1)
#define MDBUS_EXCEPT_BAD_ADDRESS                (2)
#define MDBUS_EXCEPT_BAD_BAD_COUNT              (3)
#define MDBUS_EXCEPT_MEM_ERROR                  (4)


#pragma pack(1)
typedef struct
{
    unsigned char address;
    unsigned char function;
} T_MDBUS_FRAME;

typedef struct
{
    unsigned char address;
    unsigned char function;
    unsigned short start;
    unsigned short count;
    unsigned char data0;
} T_MDBUS_FRAME_REQ;

typedef struct
{
    unsigned char address;
    unsigned char function;
    unsigned short start;
    unsigned short count;
    unsigned char bytes;
    unsigned char data0;
} T_MDBUS_FRAME_REQ_DATA;

typedef struct
{
    unsigned char address;
    unsigned char function;
    unsigned char bytes;
    unsigned char data0;
} T_MDBUS_FRAME_DATA;
#pragma pack()


static void _swapShort(unsigned short* pusValue);
static unsigned short _CRC16(const unsigned char* nData, unsigned char wLength);
static void _frame_reset(T_MDBUS_CTX* pCtx);

static void _send_error(T_MDBUS_CTX* pCtx, unsigned char i_ucFunction, unsigned char i_ucExceptCode);
static void _makeStartFrame(T_MDBUS_CTX* pCtx, unsigned char i_ucFunction);
static void _makeAddShort(T_MDBUS_CTX* pCtx, unsigned short i_usValue);
static void _makeAddByte(T_MDBUS_CTX* pCtx, unsigned char i_ucValue);
static void _makeAddCRC16(T_MDBUS_CTX* pCtx);

static void _treatReadBits(T_MDBUS_CTX* pCtx, T_MDBUS_FRAME_REQ* pFrameReq);
static void _treatReadRegisters(T_MDBUS_CTX* pCtx, T_MDBUS_FRAME_REQ* pFrameReq);
static void _treatWriteMultipleCoils(T_MDBUS_CTX* pCtx, T_MDBUS_FRAME_REQ_DATA* pFrameDta);
static void _treatWriteMultipleRegisters(T_MDBUS_CTX* pCtx, T_MDBUS_FRAME_REQ_DATA* pFrameDta);

T_MDBUS_FRAME_REQ* _convert_to_req_frame(T_MDBUS_FRAME* pFrame);
T_MDBUS_FRAME_REQ_DATA* _convert_to_req_data_frame(T_MDBUS_FRAME* pFrame);
void _treatFrame(T_MDBUS_CTX* pCtx, T_MDBUS_FRAME* pFrame);


/**
* @brief Swap short value (if needed)
* @param pusValue pointer to value to be swaped (if needed)
*/
void _swapShort(unsigned short* pusValue)
{
#ifdef ENDIAN_INVERSION
    unsigned short tmp = *pusValue;
    *pusValue = (tmp >> 8) | (tmp << 8);
#endif
}

/**
* @brief Calculate CRC16
* @param nData Buffer containing datas
* @param wLength Length of buffer
* @return CRC16
*/
unsigned short _CRC16(const unsigned char* nData, unsigned char wLength)
{
    static const unsigned short wCRCTable[] = {
       0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
       0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
       0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
       0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
       0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
       0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
       0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
       0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
       0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
       0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
       0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
       0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
       0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
       0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
       0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
       0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
       0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
       0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
       0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
       0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
       0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
       0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
       0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
       0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
       0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
       0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
       0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
       0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
       0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
       0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
       0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
       0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040 };

    unsigned char nTemp;
    unsigned short wCRCWord = 0xFFFF;

#ifdef MDBUS_DEBUG
    printf("\n    CRC [");
#endif

    while (wLength--)
    {
#ifdef MDBUS_DEBUG
        printf("%02x ", *nData);
#endif

        nTemp = *nData++ ^ wCRCWord;
        wCRCWord >>= 8;
        wCRCWord ^= wCRCTable[nTemp];
    }

#ifdef MDBUS_DEBUG
    printf("] => %x\n", wCRCWord);
#endif

    return wCRCWord;
}

/**
* @brief Reset the MODBUS context (position to 0 and all buffer set to 0)
* @param pCtx Modbus context
*/
void _frame_reset(T_MDBUS_CTX* pCtx)
{
    if (pCtx == NULL)
        return;

    pCtx->ucPos = 0;
    memset(pCtx->buffer, 0, pCtx->ucSzBuff);
}

/**
* @brief Make a start frame header into the given context
* @param pCtx Modbus context
* @param i_ucFunction Function
*/
void _makeStartFrame(T_MDBUS_CTX* pCtx, unsigned char i_ucFunction)
{
    memset(pCtx->buffer, 0, pCtx->ucSzBuff);
    pCtx->buffer[0] = pCtx->address;
    pCtx->buffer[1] = i_ucFunction;
    pCtx->ucPos = 2;
}

/**
* @brief Add a short 16 bits value to the given context
* @param pCtx Modbus context
* @param i_usValue 16 bit value
*/
void _makeAddShort(T_MDBUS_CTX* pCtx, unsigned short i_usValue)
{
    _swapShort(&i_usValue);

    pCtx->buffer[pCtx->ucPos++] = (i_usValue & 0xFF);
    pCtx->buffer[pCtx->ucPos++] = ((i_usValue >> 8) & 0xFF);
}

/**
* @brief Add a 8 bits value to the given context
* @param pCtx Modbus context
* @param i_ucValue 8 bit value
*/
void _makeAddByte(T_MDBUS_CTX* pCtx, unsigned char i_ucValue)
{
    pCtx->buffer[pCtx->ucPos++] = i_ucValue;
}

/**
* @brief Calculate the CRC at the current position of the given context and add it
* @param pCtx Modbus context
*/
void _makeAddCRC16(T_MDBUS_CTX* pCtx)
{
    unsigned short usCRC = _CRC16(pCtx->buffer, pCtx->ucPos);
    _swapShort(&usCRC);
    _makeAddShort(pCtx, usCRC);
}

/**
* @brief Send a MODBUS exception error
* @param pCtx Modbus context
* @param i_ucFunction MODBUS Function
* @param i_ucExceptCode MODBUS error exception code
*/
void _send_error(T_MDBUS_CTX* pCtx, unsigned char i_ucFunction, unsigned char i_ucExceptCode)
{
    _makeStartFrame(pCtx, i_ucFunction | 0x80);
    _makeAddByte(pCtx, i_ucExceptCode);
    _makeAddCRC16(pCtx);

    user_mdbus_send(pCtx->back, pCtx->buffer, pCtx->ucPos);
}

int _test_coils_range(unsigned short start, unsigned short count)
{
    if ((count > 2000) || (count == 0))
        return MDBUS_EXCEPT_BAD_BAD_COUNT;

    if (((unsigned long)start + count) > 0xFFFFL)
        return MDBUS_EXCEPT_BAD_ADDRESS;

    return 0;
}

int _test_registers_range(unsigned short start, unsigned short count)
{
    if ((count > 125) || (count == 0))
        return MDBUS_EXCEPT_BAD_BAD_COUNT;

    if (((unsigned long)start + count) > 0xFFFFL)
        return MDBUS_EXCEPT_BAD_ADDRESS;

    return 0;
}

void _treatReadBits(T_MDBUS_CTX* pCtx, T_MDBUS_FRAME_REQ* pFrameReq)
{
    unsigned short start = pFrameReq->start;
    unsigned short count = pFrameReq->count;
    unsigned char ucFunction = pFrameReq->function;

    int res = _test_coils_range(start, count);
    if (res != 0)
    {
        _send_error(pCtx, ucFunction, res);
        return;
    }

    _makeStartFrame(pCtx, ucFunction);
    unsigned char ucSize = count / 8;
    if (count % 8 != 0)
        ucSize += 1;	
    _makeAddByte(pCtx, ucSize);

    T_MDBUS_FRAME_DATA* pAnswer = (T_MDBUS_FRAME_DATA*)pFrameReq;
    res = MDBUS_ERR;
    switch (ucFunction)
    {
        case MDBUS_FUNCT_READ_COIL_STATUS:
        {
            res = user_mdbus_read_coils(start, count, (unsigned char*)&(pAnswer->data0));
            break;
        }
        case MDBUS_FUNCT_READ_INPUT_STATUS:
        {
            res = user_mdbus_read_inputs(start, count, (unsigned char*)&(pAnswer->data0));
            break;
        }        
        default:
        {
            _send_error(pCtx, ucFunction, MDBUS_EXCEPT_BAD_FUNCTION);
            return;
        }
    }

    if (res!= MDBUS_OK)
    {
        _send_error(pCtx, ucFunction, MDBUS_EXCEPT_MEM_ERROR);
        return;
    }

    pCtx->ucPos += ucSize;
    _makeAddCRC16(pCtx);

    user_mdbus_send(pCtx->back,pCtx->buffer, pCtx->ucPos);
}

void _treatReadRegisters(T_MDBUS_CTX* pCtx, T_MDBUS_FRAME_REQ* pFrameReq)
{
    unsigned short start = pFrameReq->start;
    unsigned short count = pFrameReq->count;
    unsigned char ucFunction = pFrameReq->function;

    int res = _test_registers_range(start, count);
    if (res != 0)
    {
        _send_error(pCtx, ucFunction, res);
        return;
    }
        
    _makeStartFrame(pCtx, ucFunction);
    unsigned char ucSize = count *2;
    _makeAddByte(pCtx, ucSize);

    T_MDBUS_FRAME_DATA* pAnswer = (T_MDBUS_FRAME_DATA*)pFrameReq;
    res = MDBUS_ERR;
    switch (ucFunction)
    {
        case MDBUS_FUNCT_READ_HOLDING_REGISTERS:
        {
            res = user_mdbus_read_holding_registers(start, count, (unsigned char*)&(pAnswer->data0));
            break;
        }
        case MDBUS_FUNCT_READ_INPUT_REGISTERS:
        {
            res = user_mdbus_read_input_registers(start, count, (unsigned char*)&(pAnswer->data0));
            break;
        }
        default:
        {
            _send_error(pCtx, ucFunction, MDBUS_EXCEPT_BAD_FUNCTION);
            return;
        }
    }

    if (res != MDBUS_OK)
    {
        _send_error(pCtx, ucFunction, MDBUS_EXCEPT_MEM_ERROR);
        return;
    }

    pCtx->ucPos += ucSize;
    _makeAddCRC16(pCtx);

    user_mdbus_send(pCtx->back,pCtx->buffer, pCtx->ucPos);
}

void _treatWriteMultipleCoils(T_MDBUS_CTX* pCtx, T_MDBUS_FRAME_REQ_DATA* pFrameDta)
{
    unsigned short start = pFrameDta->start;
    unsigned short count = pFrameDta->count;    
    unsigned char ucFunction = pFrameDta->function;
    unsigned char ucBytes;
    
    int res = _test_coils_range(start, count);
    if (res != 0)
    {
        _send_error(pCtx, ucFunction, res);
        return;
    }

    ucBytes = pFrameDta->count / 8;
    if (pFrameDta->count % 8 != 0)
        ucBytes++;

    if (pFrameDta->bytes != ucBytes)
    {
        _send_error(pCtx, ucFunction, MDBUS_EXCEPT_BAD_BAD_COUNT);
        return;
    }

    res = MDBUS_ERR;
    switch (ucFunction)
    {
        case MDBUS_FUNCT_WRITE_MULTIPLE_COILS:
        {
            res=user_mdbus_write_coils(start, count, (unsigned char*)&(pFrameDta->data0));
            break;
        }
        default:
        {
            _send_error(pCtx, ucFunction, MDBUS_EXCEPT_BAD_FUNCTION);
            return;
        }
    }

    if ( res != MDBUS_OK)
    {
        _send_error(pCtx, ucFunction, MDBUS_EXCEPT_MEM_ERROR);
        return;
    }

    _makeStartFrame(pCtx, ucFunction);
    _makeAddShort(pCtx, start);
    _makeAddShort(pCtx, count);
    _makeAddCRC16(pCtx);

    user_mdbus_send(pCtx->back,pCtx->buffer, pCtx->ucPos);
}


void _treatWriteMultipleRegisters(T_MDBUS_CTX* pCtx, T_MDBUS_FRAME_REQ_DATA* pFrameDta)
{
    unsigned short start = pFrameDta->start;
    unsigned short count = pFrameDta->count;
    unsigned char ucBytes = pFrameDta->count * 2;
    unsigned char ucFunction = pFrameDta->function;

    int res = _test_registers_range(start, count);
    if (res != 0)
    {
        _send_error(pCtx, ucFunction, res);
        return;
    }

    if (pFrameDta->bytes != ucBytes)
    {
        _send_error(pCtx, ucFunction, MDBUS_EXCEPT_BAD_BAD_COUNT);
        return;
    }

    res = MDBUS_ERR;
    switch (ucFunction)
    {
        case MDBUS_FUNCT_WRITE_MULTIPLE_REGISTERS:
        {
            res=user_mdbus_write_holding_registers(start, count, (unsigned char*)&(pFrameDta->data0));
            break;
        }
        default:
        {
            _send_error(pCtx, ucFunction, MDBUS_EXCEPT_BAD_FUNCTION);
            break;
        }
    }

    if ( res != MDBUS_OK)
    {
        _send_error(pCtx, ucFunction, MDBUS_EXCEPT_MEM_ERROR);
        return;
    }

    _makeStartFrame(pCtx, ucFunction);
    _makeAddShort(pCtx, start);
    _makeAddShort(pCtx, count);
    _makeAddCRC16(pCtx);

    user_mdbus_send(pCtx->back,pCtx->buffer, pCtx->ucPos);
}


T_MDBUS_FRAME_REQ* _convert_to_req_frame(T_MDBUS_FRAME* pFrame)
{
    T_MDBUS_FRAME_REQ* pFrameReq = (T_MDBUS_FRAME_REQ*)((unsigned char*)pFrame);

    _swapShort(&(pFrameReq->start));
    _swapShort(&(pFrameReq->count));

    return pFrameReq;
}

T_MDBUS_FRAME_REQ_DATA* _convert_to_req_data_frame(T_MDBUS_FRAME* pFrame)
{
    T_MDBUS_FRAME_REQ* pFrameDta = _convert_to_req_frame(pFrame);

    return (T_MDBUS_FRAME_REQ_DATA*)pFrameDta;
}

void _treatFrame(T_MDBUS_CTX* pCtx, T_MDBUS_FRAME* pFrame)
{
#ifdef MDBUS_DEBUG
    printf("Treat Frame:\n");
    printf("  - Address   : %d\n", pFrame->address);
    printf("  - Function  : %d\n", pFrame->function);
#endif

    if (pFrame->address == pCtx->address)
    {
        switch (pFrame->function)
        {
            case MDBUS_FUNCT_READ_COIL_STATUS:
            case MDBUS_FUNCT_READ_INPUT_STATUS:
            {
                _treatReadBits(pCtx, _convert_to_req_frame(pFrame));
                break;
            }
            case MDBUS_FUNCT_WRITE_MULTIPLE_COILS:
            {
                _treatWriteMultipleCoils(pCtx, _convert_to_req_data_frame(pFrame));
                break;
            }
            case MDBUS_FUNCT_READ_INPUT_REGISTERS:
            case MDBUS_FUNCT_READ_HOLDING_REGISTERS:
            {
                _treatReadRegisters(pCtx, _convert_to_req_frame(pFrame));
                break;
            }
            case MDBUS_FUNCT_WRITE_MULTIPLE_REGISTERS:
            {
                _treatWriteMultipleRegisters(pCtx, _convert_to_req_data_frame(pFrame));
                break;
            }
            default:
            {
                _send_error(pCtx, pFrame->function, MDBUS_EXCEPT_BAD_FUNCTION);
                break;
            }
        }
    }
}

void mdbus_fill_coil_data(unsigned char* o_buffer, unsigned char i_ucIndex, unsigned char i_ucValue)
{
    unsigned char ucByte = i_ucIndex / 8;
    unsigned char ucBit = i_ucIndex % 8;

    if (i_ucValue)
        o_buffer[ucByte] |= (1 << ucBit);
    else
        o_buffer[ucByte] &= ~(1 << ucBit);
}

void mdbus_fill_register_data(unsigned char* o_buffer, unsigned char i_ucIndex, unsigned short i_usValue)
{
    _swapShort(&i_usValue);

    unsigned char ucByte = i_ucIndex * 2;

    o_buffer[ucByte] = i_usValue & 0xFF;
    o_buffer[ucByte + 1] = (i_usValue >> 8) & 0xFF;
}

unsigned char mdbus_get_coil_data(unsigned char* i_buffer, unsigned char i_ucIndex)
{
    unsigned char ucVal;
    unsigned char ucByte = i_ucIndex / 8;
    unsigned char ucBit = i_ucIndex % 8;

    if ((i_buffer[ucByte] & (1 << ucBit)) == (1 << ucBit))
        ucVal = 1;
    else
        ucVal = 0;

    return ucVal;
}

unsigned short mdbus_get_register_data(unsigned char* i_buffer, unsigned char i_ucIndex)
{
    unsigned short usRet;

    unsigned char ucByte = i_ucIndex * 2;
    unsigned char a = i_buffer[ucByte];
    unsigned char b = i_buffer[ucByte + 1];

    usRet = a;
    usRet |= (b << 8);
    _swapShort(&usRet);

    return usRet;
}

void mdbus_init(T_MDBUS_CTX* pCtx, unsigned char* buffer, unsigned char i_ucSize, unsigned char address)
{
    if (pCtx == NULL)
        return;

    pCtx->buffer = buffer;
    pCtx->ucSzBuff = i_ucSize;
    pCtx->address = address;
    pCtx->back = NULL;

    _frame_reset(pCtx);
}

void mdbus_reset(T_MDBUS_CTX* pCtx)
{
    _frame_reset(pCtx);
}

int mdbus_receive(T_MDBUS_CTX* pCtx, unsigned char i_ucByte)
{
#ifdef MDBUS_DEBUG
    printf("%02x ", i_ucByte);
#endif

    if (pCtx->ucPos >= pCtx->ucSzBuff)
        return MDBUS_ERR;

    pCtx->buffer[pCtx->ucPos] = i_ucByte;
    pCtx->ucPos++;

    if (pCtx->ucPos < 7)
        return MDBUS_OK;

    unsigned char ucDataSize = 0;
    if (    (pCtx->buffer[1] == MDBUS_FUNCT_WRITE_MULTIPLE_COILS)
         || (pCtx->buffer[1] == MDBUS_FUNCT_WRITE_MULTIPLE_REGISTERS)
        )
    {
        ucDataSize = pCtx->buffer[6] + 1;
    }

    if (pCtx->ucPos >= 8 + ucDataSize)
    {
        unsigned short usCalc = _CRC16(pCtx->buffer, pCtx->ucPos - 2);
        unsigned short usFrm = pCtx->buffer[pCtx->ucPos - 2];
        usFrm |= (pCtx->buffer[pCtx->ucPos - 1] << 8);

#ifdef MDBUS_DEBUG
        printf("\n");
        printf("Calc CRC= %x\n", usCalc);
        printf("Frame CRC= %x\n\n", usFrm);
#endif

        if (usFrm == usCalc)
        {
            T_MDBUS_FRAME* pFrm = (T_MDBUS_FRAME*)pCtx->buffer;
            _treatFrame(pCtx, pFrm);
            _frame_reset(pCtx);
        }
        else
        {
            _frame_reset(pCtx);
        }
    }

    return MDBUS_OK;
}