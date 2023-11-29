#pragma once

#include "DataTypes.h"

class ModbusCore {
    
    public:
        virtual ~ModbusCore() = default;

        uint8 high(uint16);
        uint8 low(uint16);
        uint16 merge(uint8, uint8);

        struct txMbFormat {
            uint8 function, slaveID;
            uint16 targetOffset, targetSize;
            uint8 dataLength;
            byteStream data;
        };

        struct rxMbFormat {
            uint8 function, slaveID;
            uint16 targetOffset, targetSize;
            uint8 dataLength;
            byteStream data;
        };


};