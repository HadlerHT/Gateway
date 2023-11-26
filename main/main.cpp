#include <iostream>
#include "wifimanager.h"
#include "modbustcpip.h"

extern "C" void app_main(void) {

    WiFiManager wifiManager;
    wifiManager.connect();

    ModbusTCPIP modbus;
    modbus.setTargetServerIP("192.168.0.9", false);

    txMbPacket request;
        request.packetLength = 6;
        request.slaveID = 1;
        request.function = 1;
        request.targetOffset = 0;
        request.targetSize = 16;

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        modbus.sendRequestPacket(request);
        rxMbPacket response = modbus.getResponsePacket(true);
    }

    
}

