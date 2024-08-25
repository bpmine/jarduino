/**
* @file mdbus.h
* @brief Header of a very tiny modbus slave/server for Arduino
*/
#ifndef MDBUS_HEADER_INCLUDED
#define MDBUS_HEADER_INCLUDED

//#define MDBUS_DEBUG

#define MDBUS_OK       (0)
#define MDBUS_ERR      (-1)

/**
* @brief Callbacks to be implemented by the user
* @defgroup mdbus_callbacks
*
* All these functions must be defined and implemented into the user program
*/

/**
* @brief This function is called when the MODBUS slave must send a frame
*
* Call the backend send function like used into the caller context.
*
* @param back Context backend (NULL if not used)
* @param buffer Bytes to be sent
* @param size Size of the buffer
* @ingroup mdbus_callbacks
*/
void user_mdbus_send(void *back,unsigned char* buffer, int size);

/**
* @brief This function is called when the MODBUS slave must read some coils
*
* Fill in the o_pBuffer like specified into MODBUS standard (until count bits)
*
* @param addr Coil address where to start reading
* @param count Number of coils to read
* @param o_pBuffer Output buffer following MODBUS specifications
* @return MDBUS_OK if no error.
*
* @ingroup mdbus_callbacks
*/
int user_mdbus_read_coils(unsigned short addr, unsigned short count, unsigned char* o_pBuffer);

/**
* @brief This function is called when the MODBUS slave must read some inputs
*
* Fill in the o_pBuffer like specified into MODBUS standard (until count bits)
*
* @param addr Input address where to start reading
* @param count Number of inputs to read
* @param o_pBuffer Output buffer following MODBUS specifications
* @return MDBUS_OK if no error.
*
* @ingroup mdbus_callbacks
*/
int user_mdbus_read_inputs(unsigned short addr, unsigned short count, unsigned char* o_pBuffer);

/**
* @brief This function is called when the MODBUS slave must read some input registers
*
* Fill in the o_pBuffer like specified into MODBUS standard (until count registers)
*
* @param addr Input address where to start reading
* @param count Number of input registers to read
* @param o_pBuffer Output buffer following MODBUS specifications
* @return MDBUS_OK if no error.
*
* @ingroup mdbus_callbacks
*/
int user_mdbus_read_input_registers(unsigned short addr, unsigned short count, unsigned char* o_pBuffer);

/**
* @brief This function is called when the MODBUS slave must read some holding registers
*
* Fill in the o_pBuffer like specified into MODBUS standard (until count registers)
*
* @param addr Input address where to start reading
* @param count Number of holding registers to read
* @param o_pBuffer Output buffer following MODBUS specifications
* @return MDBUS_OK if no error.
*
* @ingroup mdbus_callbacks
*/
int user_mdbus_read_holding_registers(unsigned short addr, unsigned short count, unsigned char* o_pBuffer);

/**
* @brief This function is called when the MODBUS slave must write some holding registers
*
* Write the holding registers like decoded into i_pBuffer (See MODBUS standard)
*
* @param addr Holding register address where to start writing
* @param count Number of registers to write
* @param i_pBuffer Input buffer following MODBUS specifications
* @return MDBUS_OK if no error.
*
* @ingroup mdbus_callbacks
*/
int user_mdbus_write_holding_registers(unsigned short addr, unsigned short count, unsigned char* i_pBuffer);

/**
* @brief This function is called when the MODBUS slave must write some coils
*
* Write the coils like decoded into i_pBuffer (See MODBUS standard)
*
* @param addr Coil address where to start writing
* @param count Number of coils to write
* @param i_pBuffer Input buffer following MODBUS specifications
* @return MDBUS_OK if no error.
*
* @ingroup mdbus_callbacks
*/
int user_mdbus_write_coils(unsigned short addr, unsigned short count, unsigned char* i_pBuffer);

/**
* @struct T_MDBUS_CTX
* @brief MODBUS server context structure (to be used everywhere)
*/
typedef struct
{
    unsigned char* buffer;		///< Reference to buffer that contains send/receive data
    unsigned char ucSzBuff;		///< Maximum size of buffer
    unsigned char ucPos;		///< Current position into buffer (during fill in and/or receiving datas)

    unsigned char address;		///< MODBUS unit address
    void* back;                 ///< Backend context (useful inside send callback)
} T_MDBUS_CTX;

/**
* @brief Fill the MODBUS answer data buffer for coils
* @param o_buffer Data buffer
* @param i_ucIndex Coil index
* @param i_ucValue Coil value to write
*/
void mdbus_fill_coil_data(unsigned char* o_buffer, unsigned char i_ucIndex, unsigned char i_ucValue);

/**
* @brief Fill the MODBUS answer data buffer for registers
* @param o_buffer Data buffer
* @param i_ucIndex Register index
* @param i_usValue Register value to write
*/
void mdbus_fill_register_data(unsigned char* o_buffer, unsigned char i_ucIndex, unsigned short i_usValue);

/**
* @brief Get coil status from MODBUS data buffer
* @param i_buffer Data buffer
* @param i_ucIndex Coil index
* @return Coil status (1 or 0)
*/
unsigned char mdbus_get_coil_data(unsigned char* i_buffer, unsigned char i_ucIndex);

/**
* @brief Get register value from MODBUS data buffer
* @param i_buffer Data buffer
* @param i_ucIndex Register index
* @return Register value
*/
unsigned short mdbus_get_register_data(unsigned char* i_buffer, unsigned char i_ucIndex);

/**
* @brief Init the MODBUS server context
* @param pCtx Reference to MODBUS server context @see T_MDBUS_CTX
* @param buffer Reference to context buffer
* @param i_ucSize Size of the buffer (Maximum)
* @param address Unit Address of MODBUS slave
*/
void mdbus_init(T_MDBUS_CTX* pCtx, unsigned char* buffer, unsigned char i_ucSize, unsigned char address);

/**
* @brief Receive a new byte and add it to the buffer until frame completion. Then processes the frame.
* @param pCtx Reference to MODBUS server context @see T_MDBUS_CTX
* @param i_ucByte Received byte
* @return MDBUS_OK if no error
*/
int mdbus_receive(T_MDBUS_CTX* pCtx, unsigned char i_ucByte);

/**
* @brief Reset the modbus management. To be done when detecting an error.
* @param pCtx Reference to MODBUS server context @see T_MDBUS_CTX
*/
void mdbus_reset(T_MDBUS_CTX* pCtx);

#endif
