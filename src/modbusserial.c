#include "modbusserial.h"

const uint16 interSymbolTimeout_ms = 10;

void modbus_initialize() {
    uart_initiliaze();
}

void modbus_sendRequestPacket(uint8* data, uint16 length) {

    uart_flush(UART_ID);
    uart_write_bytes(UART_ID, data, length);

    // Cleans up wrong byte read by the transition on RS485 flow control
    vTaskDelay(5); 
    uart_flush_input(UART_ID); 
};

// Function to read Modbus serial data with timeout detection
uint16 modbus_readResponsePacket(uint8* buffer, uint16 bufferSize, uint16 timeOut) {
    return uart_read_bytes(UART_ID, buffer, bufferSize, timeOut);
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

