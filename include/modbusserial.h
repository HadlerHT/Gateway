#pragma once

#include "datatypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uartmanager.h"

typedef struct {
    uint8 slaveID;
    uint8 function;
    uint16 targetOffset;
    uint16 targetSize;
    uint8 dataSize;
    uint8* data;
    // uint16 CRC;
} mbPacket;

extern uint16 interSymbolTimout_us;
extern uint16 interPacketTimeout_us;

void modbus_initialize();
uint16 modbus_readData(uint8*, uint16);

// void modbus_serializePacket(mbPacket*, uint8**, uint16*);
uint16 modbus_evaluateCRC(uint8*, uint16);

uint16 merge(uint8, uint8);
uint8 high(uint16);
uint8 low(uint16);