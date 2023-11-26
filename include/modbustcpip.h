#pragma once

#include <iostream>
#include <vector>

#include "sdkconfig.h"
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "esp_netif.h"
#include "esp_log.h"

#include "datatypes.h"
#include "modbuscore.h"

struct TxMbPacket : ModbusCore::txMbFormat {
    uint16 transmissionID = 0x0000;
    uint16 protocolID = 0x0000;
    uint16 packetLength;
};

struct RxMbPacket : ModbusCore::txMbFormat {
    uint16 transmissionID = 0x0000;
    uint16 protocolID = 0x0000;
    uint16 packetLength;
};

typedef struct TxMbPacket txMbPacket;
typedef struct RxMbPacket rxMbPacket;
typedef int webSocket;

class ModbusTCPIP : public ModbusCore {

    public:
        ModbusTCPIP();
        // void run();

        bool setTargetServerIP(std::string, bool);
        void sendRequestPacket(txMbPacket&);
        rxMbPacket getResponsePacket(bool);

    private:
        const char* TAG = "Modbus Core";
        webSocket mbSocket = -1;
        struct sockaddr_in targetServer;

        byteStream serializePacket(txMbPacket&);
        rxMbPacket structurizeStream(byteStream&);
        bool openSocket();
        void closeSocket();

};

