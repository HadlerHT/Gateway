#include "modbusserial.h"

const uint16 interSymbolTimeout_ms = 1;

void modbus_initialize() {
    uart_initiliaze();
}

void modbus_sendRequestPacket(uint8* data, uint16 length) {
    uart_write_bytes(UART_ID, data, length);
};

// Function to read Modbus serial data with timeout detection
uint16 modbus_readResponsePacket(uint8* buffer, uint16 bufferSize, uint16 timeOut) {

    uint16 bytesRead = 0;
    uint8 currentChar;
    bool awaitingFirstByte = true;

    while (true) {
        
        // The line bellow is set up to await 'timeOut' for startuing receiving bytes,
        // and then ~750us (rounded to 1ms here) for each next byte, following modbus rtu specs
        int uart_readLen = uart_read_bytes(UART_ID, (uint8_t*)&currentChar, 1, 
                                           awaitingFirstByte ? timeOut : interSymbolTimeout_ms);

        // No byte arrived: assume end of packet
        if (uart_readLen == 0)
            break;
        
        // Unsuccessfull on reading byte
        else if (uart_readLen < 0)
            return -1;
        
        // Reading successfull
        else {
            buffer[bytesRead++] = currentChar;
            
            // Buffer overflow: shouldn't happen
            if (bytesRead >= bufferSize)
                break;
        }
    }
    return bytesRead;
}

uint16 modbus_evaluateCRC(uint8* data, uint16 length) {

    const uint16 polynomial = 0xA001;
    uint16 CRC = 0xFFFF;

    for (uint8 byte = 0; byte < length; byte++) {
        CRC ^= data[byte];
        for (uint8 i = 0; i < 8; i++)
            if (CRC & 0x01)
                CRC = (CRC >> 1) ^ polynomial;
            else
                CRC >>= 1;
    }

    return CRC;
}

