#include "modbustcpip.h"

ModbusTCPIP::ModbusTCPIP() {

    targetServer = {
        .sin_family = AF_INET,
        .sin_port = htons(502),
        .sin_addr = 0,
    };

    openSocket();
}

void ModbusTCPIP::closeSocket() {

    ESP_LOGE(TAG, "Shutting down socket");
    shutdown(mbSocket, 0);
    close(mbSocket);
}

bool ModbusTCPIP::openSocket() {
    
    mbSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    
    if (mbSocket >= 0) {
        ESP_LOGI(TAG, "Socket created successfully");
        return true;
    }
    
    ESP_LOGE(TAG, "Unable to create socket. errno %d", errno);
    return false;
}

bool ModbusTCPIP::setTargetServerIP(std::string ipStr, bool closeFirst = false) {

    if (closeFirst)
        closeSocket();

    inet_pton(targetServer.sin_family, ipStr.c_str(), &targetServer.sin_addr);

    int err = connect(mbSocket, (struct sockaddr *)&targetServer, sizeof(targetServer));
    if (err != 0) {
        ESP_LOGE(TAG, "Unable to connect. errno %d", errno);
        return false;
    }

    ESP_LOGI(TAG, "Successfully connected to host %s", ipStr.c_str());
    return true;
}

void ModbusTCPIP::sendRequestPacket(txMbPacket& payload) {

    byteStream stream = serializePacket(payload);

    int err = send(mbSocket, stream.data(), stream.size(), 0);
    if (err < 0)
        ESP_LOGE(TAG, "Error occurred during sending. errno %d", errno);
}

byteStream ModbusTCPIP::serializePacket(txMbPacket& tx) {
    
    byteStream serializedPacket;

    serializedPacket = {
        high(tx.transmissionID), low(tx.transmissionID),
        high(tx.protocolID), low(tx.protocolID),
        high(tx.packetLength), low(tx.packetLength),
        tx.slaveID, tx.function, 
        high(tx.targetOffset), low(tx.targetOffset),
        high(tx.targetSize), low(tx.targetSize)
    };

    if (tx.dataLength > 0) {
        serializedPacket.push_back(tx.dataLength);

        for (uint8 byte : tx.data)
            serializedPacket.push_back(byte);
    }

    printf("TX: ");
    for (uint8 byte : serializedPacket)
        printf("%02x ", byte);
    printf("\n");


    return serializedPacket;
}

rxMbPacket ModbusTCPIP::getResponsePacket(bool printHex = false) {

    uint8 rxBuffer[50];
    int len = recv(mbSocket, rxBuffer, sizeof(rxBuffer) - 1, 0);
    if (len < 0)
        ESP_LOGE(TAG, "Failed to receive data. errno %d", errno);

    byteStream stream(rxBuffer, rxBuffer + len);

    if (printHex) {
        printf("RX: ");
        for (uint8 byte : stream)
            printf("%02x ", byte);
        printf("\n");
    }

    return structurizeStream(stream);

}

rxMbPacket ModbusTCPIP::structurizeStream(byteStream& rx) {

    // TI PI PL AD FN -- OF TS ---- MF5 MF6 MF15 MF16 (W)
    // TI PI PL AD FN -- DL DT ---- MF1 MF2 MF3 MF4 (R)

    rxMbPacket packet;

    packet.transmissionID = merge(rx[0], rx[1]);
    packet.protocolID = merge(rx[2], rx[3]);
    packet.packetLength = merge(rx[4], rx[5]);
    packet.slaveID = rx[6];
    packet.function = rx[7];

    // Reading Functions: MF1, MF2, MF3, MF4 
    if (packet.function > 0 && packet.function < 5) {
        packet.dataLength = rx[8];
        packet.data = byteStream(rx[9], rx[9] + packet.dataLength); 
    }

    // Writring Functions: MF5, MF6, MF15, MF16
    else {
        packet.targetOffset = merge(rx[8], rx[9]);
        packet.targetSize = merge(rx[10], rx[11]);
    }

    return packet;
}
