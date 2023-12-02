#include "ModbusSerial.h"

ModbusSerial::ModbusSerial() {
    openUART();
}

void ModbusSerial::openUART() {

    uartParams = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_driver_install(UART_ID, 256, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_ID, &uartParams));
    ESP_ERROR_CHECK(uart_set_pin(UART_ID, TX_PIN, RX_PIN, RTS_PIN, CTS_PIN));

}

void ModbusSerial::sendRequestPacket(txMbPacket& payload) {

    byteStream stream = serializePacket(payload);

    uart_write_bytes(UART_ID, stream.data(), stream.size());

}

std::optional<rxMbPacket> ModbusSerial::readRequestPacket() {
    
    uint8 rxBuffer[256];
    int len = uart_read_bytes(UART_ID, rxBuffer, 256, waitForResponseMs); 
    if (len)
        // ESP_LOGE(TAG, "Failed to receive data. errno %d", errno);
        return std::nullopt;
    
        
    byteStream stream(rxBuffer, rxBuffer + len);

#ifdef PRINT_PACKET_BYTESTREAMS
    printf("RX: ");
    for (uint8 byte : stream)
        printf("%02x ", byte);
    printf("\n");
#endif

    // Transmission Error Detected
    if (evaluateCRC(stream))
        return std::nullopt;

    return structurizeStream(stream);
}

byteStream ModbusSerial::serializePacket(txMbPacket& tx) {

    byteStream serializedPacket;
    serializedPacket = {
        tx.slaveID, 
        tx.function, 
        high(tx.targetOffset), low(tx.targetOffset),
        high(tx.targetSize), low(tx.targetSize)
    };

    if (tx.dataLength > 0) {
        serializedPacket.push_back(tx.dataLength);

        for (uint8 byte : tx.data)
            serializedPacket.push_back(byte);
    }

    evaluateCRC(serializedPacket, true);

#ifdef PRINT_PACKET_BYTESTREAMS
    printf("TX: ");
    for (uint8 byte : serializedPacket)
        printf("%02x ", byte);
    printf("\n");
#endif

    return serializedPacket;
}

rxMbPacket ModbusSerial::structurizeStream(byteStream& rx) {

    // AD FN -- OF TS ---- MF5 MF6 MF15 MF16 (W)
    // AD FN -- DL DT ---- MF1 MF2 MF3 MF4 (R)
    rxMbPacket packet;

    packet.slaveID = rx[0];
    packet.function = rx[1];

    // Reading Functions: MF1, MF2, MF3, MF4 
    if (packet.function > 0 && packet.function < 5) {
        packet.dataLength = rx[2];
        packet.data = byteStream(rx[3], rx[3] + packet.dataLength - 2); 
        packet.CRC = merge(rx[2 + packet.dataLength - 1], rx[1 + packet.dataLength]);
    }

    // Writing Functions: MF5, MF6, MF15, MF16
    else {
        packet.targetOffset = merge(rx[2], rx[3]);
        packet.targetSize = merge(rx[4], rx[5]);
        packet.CRC = merge(rx[6], rx[7]);
    }
    
    return packet;
}

uint16 ModbusSerial::evaluateCRC(byteStream& stream, bool doAppend = false) {
    
    const uint16 polynomial = 0xA001; //Modbus Inverse Polynomial
    uint16 CRC = 0xFFFF;

    for (uint8 byte : stream) {
        CRC ^= byte;
        for (uint8 i = 0; i < 8; i++)
            if (CRC & 0x01)
                CRC = (CRC >> 1) ^ polynomial;
            else
                CRC >>= 1;
    }

    if (doAppend) {
        stream.push_back(low(CRC));
        stream.push_back(high(CRC));
    }
    
    return CRC;
}