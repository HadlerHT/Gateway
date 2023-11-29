#include <iostream>
#include "WifiManager.h"
// #include "ModbusIP.h"
#include "ModbusSerial.h"

extern "C" void app_main(void) {

    WiFiManager wifiManager;
    wifiManager.connect();

    ModbusSerial modbus;

    // modbus.sendRequestPacket();
    // auto packet = modbus.readRequestPacket();
    // if (packet.has_value())
    //     int a = 0;

    // ModbusIP modbus;
    // modbus.setTargetServerIP("192.168.0.9", false);

    // txMbPacket request;
    //     request.packetLength = 11;
    //     request.slaveID = 1;
    //     request.function = 15;
    //     request.targetOffset = 0;
    //     request.targetSize = 32;
    //     request.dataLength = 4;
    //     request.data = {0xAA, 0xBB, 0xCC, 0xDD};

    // while (1) {
    //     vTaskDelay(pdMS_TO_TICKS(1000));
        
    //     modbus.sendRequestPacket(request);
    //     rxMbPacket response = modbus.getResponsePacket(true);
    // }

    
}

