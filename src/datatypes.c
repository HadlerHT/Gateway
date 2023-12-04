#include "datatypes.h"

uint16 merge(uint8 high, uint8 low) {
    return ((uint16)high << 8) | low;
}

uint8 high(uint16 word) {
    return (uint8)(word >> 8);
}

uint8 low(uint16 word) {
    return (uint8)word;
}