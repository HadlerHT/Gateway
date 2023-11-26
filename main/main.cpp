#include <iostream>
#include "wifimanager.h"
#include "modbustcpip.h"

extern "C" void app_main(void) {

    WiFiManager wifiManager;
    wifiManager.connect();

    ModbusTCPIP modbus;
    modbus.setTargetServerIP("192.168.0.9", false);

    txMbPacket request;
        request.packetLength = 11;
        request.slaveID = 1;
        request.function = 15;
        request.targetOffset = 0;
        request.targetSize = 32;
        request.dataLength = 4;
        request.data = {0xAA, 0xBB, 0xCC, 0xDD};

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        modbus.sendRequestPacket(request);
        rxMbPacket response = modbus.getResponsePacket(true);
    }

    
}

