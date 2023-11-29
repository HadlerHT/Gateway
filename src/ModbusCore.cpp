#include "ModbusCore.h"

uint8 ModbusCore::low(uint16 x) {
    return static_cast<uint8>(x);
}

uint8 ModbusCore::high(uint16 x) {
    return static_cast<uint8>(x >> 8);
}

uint16 ModbusCore::merge(uint8 high, uint8 low) {
    return static_cast<uint16>(high) << 8 | low;
}