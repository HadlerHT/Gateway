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
        tx.function, tx.slaveID,
        high(tx.targetOffset), low(tx.targetOffset),
        high(tx.targetSize), low(tx.targetSize)
    };

    if (tx.dataLength > 0) {
        serializedPacket.push_back(tx.dataLength);

        for (uint8 byte : tx.data)
            serializedPacket.push_back(byte);
    }

    return serializedPacket;
}

rxMbPacket ModbusTCPIP::getResponsePacket(bool printHex = false) {

    uint8 rxBuffer[50];
    int len = recv(mbSocket, rxBuffer, sizeof(rxBuffer) - 1, 0);
    if (len < 0)
        ESP_LOGE(TAG, "Failed to receive data. errno %d", errno);

    byteStream stream(rxBuffer, rxBuffer + len);

    if (printHex) {
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

// void ModbusTCPIP::run() {

//     // static const char payload[] = {
//     //     0x00, 0x00, // TI 
//     //     0x00, 0x00, // PI
//     //     0x00, 0x06, // Len
//     //     0x01,       // ADDR
//     //     0x01,       // FNC
//     //     0x00, 0x00, // OFFSET
//     //     0x00, 0x10  // DATA
//     // };

//     // static const char payload[] = {
//     //     0x00, 0x00, // TI 
//     //     0x00, 0x00, // PI
//     //     0x00, 0x09, // Len
//     //     0x01,       // ADDR
//     //     0x0F,       // FNC
//     //     0x00, 0x10, // OFFSET
//     //     0x00, 0x10,  // DATA
//     //     0x02,
//     //     0xAA, 0x99,
//     // };

//     std::vector<uint8> payload = {
//         0x00, 0x00, // TI 
//         0x00, 0x00, // PI
//         0x00, 0x09, // Len
//         0x01,       // ADDR
//         0x0F,       // FNC
//         0x00, 0x10, // OFFSET
//         0x00, 0x10,  // DATA
//         0x02,
//         0xAA, 0x99,
//     };


//     int PORT = 502;
//     char TAG[] = "Modbus Core";

//     char rx_buffer[128];
//     char host_ip[] = "192.168.0.9";
//     int addr_family = 0;
//     int ip_protocol = 0;

//     while (1) {

//         struct sockaddr_in dest_addr;
//         inet_pton(AF_INET, host_ip, &dest_addr.sin_addr);
//         dest_addr.sin_family = AF_INET;
//         dest_addr.sin_port = htons(PORT);
//         addr_family = AF_INET;
//         ip_protocol = IPPROTO_IP;

//         int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
//         if (sock < 0) {
//             ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
//             break;
//         }
//         ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, PORT);

//         int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
//         if (err != 0) {
//             ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
//             break;
//         }
//         ESP_LOGI(TAG, "Successfully connected");

//         vTaskDelay(pdMS_TO_TICKS(500));

//         while (1) {

//             vTaskDelay(pdMS_TO_TICKS(1000));

//             // int err = send(sock, payload, 15, 0);
//             int err = send(sock, payload.data(), payload.size(), 0);
//             if (err < 0) {
//                 ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
//                 break;
//             }

//             int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
//             std::cout << len << std::endl;
//             // Error occurred during receiving
//             if (len < 0) {
//                 ESP_LOGE(TAG, "recv failed: errno %d", errno);
//                 break;
//             }
//             // Data received
//             else {
//                 rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
//                 ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
                
//                 for (unsigned char byte=0; byte < len; byte++)
//                     printf("%02x ", rx_buffer[byte]);

//                 printf("\n"); 

//                 // ESP_LOGI(TAG, "%02x", rx_buffer);
//             }
//         }

//         if (sock != -1) {
//             ESP_LOGE(TAG, "Shutting down socket and restarting...");
//             shutdown(sock, 0);
//             close(sock);
//         }
//     }

// }
