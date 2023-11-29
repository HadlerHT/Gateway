#pragma once

#ifdef MODBUS_SERIAL
#error "MULTIPLE VERSIONS OF MODBUS CORES INITILIZED"
#endif
#define MODBUS_IP

// #define PRINT_PACKET_BYTESTREAMS

#include <iostream>
#include <vector>
#include <optional>

#include "sdkconfig.h"
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "esp_netif.h"
#include "esp_log.h"

#include "DataTypes.h"
#include "ModbusCore.h"

typedef struct : ModbusCore::txMbFormat {
    uint16 transmissionID = 0x0000;
    uint16 protocolID = 0x0000;
    uint16 packetLength;
} txMbPacket;

typedef struct : ModbusCore::txMbFormat {
    uint16 transmissionID = 0x0000;
    uint16 protocolID = 0x0000;
    uint16 packetLength;
} rxMbPacket;

typedef int webSocket;

class ModbusIP : public ModbusCore {

    public:
        ModbusIP();

        bool setTargetServerIP(std::string, bool);
        
        void sendRequestPacket(txMbPacket&);
        std::optional<rxMbPacket> getResponsePacket();

    private:
        const char* TAG = "Modbus TCP/IP";
        webSocket mbSocket = -1;
        struct sockaddr_in targetServer;

        byteStream serializePacket(txMbPacket&);
        rxMbPacket structurizeStream(byteStream&);
        bool openSocket();
        void closeSocket();

};

