#pragma once

#ifdef MODBUS_IP
#error "MULTIPLE VERSIONS OF MODBUS CORES INITILIZED"
#endif
#define MODBUS_SERIAL

// #define PRINT_PACKET_BYTESTREAMS

// Cpp Standart Lybraries
#include <iostream>
#include <vector>
#include <optional>

// EspIDF Packets
#include "freertos/FreeRTOS.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

// My Packets
#include "DataTypes.h"
#include "ModbusCore.h"

#define UART_ID 1
#define RX_PIN 2
#define TX_PIN 3
#define RTS_PIN 4 //Ready to Send
#define CTS_PIN 5 //Clear to Send

typedef struct : ModbusCore::txMbFormat {
    uint16 CRC = 0;
} txMbPacket;

typedef struct : ModbusCore::txMbFormat {
    uint16 CRC = 0;
} rxMbPacket;

enum Encoding {RTU, ASCII};

class ModbusSerial : public ModbusCore {

    public:
        ModbusSerial();
        void sendRequestPacket(txMbPacket&);
        std::optional<rxMbPacket> readRequestPacket();

    protected:
        uart_config_t uartParams;
        Encoding byteEncoding = RTU;
        uint16 waitForResponseMs = 1000 / portTICK_PERIOD_MS;
        uint8 t35ms = 1750 / portTICK_PERIOD_MS; //Min time between modbus frames
        uint8 t15ms = 750 / portTICK_PERIOD_MS; //Max time between modbus bytes

    private:
        const char* TAG = "Modbus Serial";

        void openUART();
        byteStream serializePacket(txMbPacket&);
        rxMbPacket structurizeStream(byteStream&);
        uint16 evaluateCRC(byteStream&, bool);
};
