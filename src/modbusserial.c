#include "modbusserial.h"

void modbus_serializePacket(mbPacket* payload, uint8** stream, uint16* streamSize) {

    *streamSize = payload->dataSize ? payload->dataSize + 9 : 8;

    // REMEMBER TO FREE THIS AFTER
    printf("%d\n", *streamSize);
    

    
    // *stream = (uint8*)malloc(*streamSize);
    // uint8* byteStream = (uint8*)malloc(*streamSize);
    if (*stream == NULL)
        return;

    // byteStream[0] = payload->slaveID;
    // byteStream[1] = payload->function;
    // byteStream[2] = high(payload->targetOffset);
    // byteStream[3] = low(payload->targetOffset);
    // byteStream[4] = high(payload->targetSize);
    // byteStream[5] = low(payload->targetSize);
    (*stream)[0] = payload->slaveID;
    (*stream)[1] = payload->function;
    (*stream)[2] = high(payload->targetOffset);
    (*stream)[3] = low(payload->targetOffset);
    (*stream)[4] = high(payload->targetSize);
    (*stream)[5] = low(payload->targetSize);

    for (int k = 0; k < 6; k++)
        printf ("%02x ", (*stream)[k]);
    printf("\n");

    if (payload->dataSize > 0) {
        (*stream)[6] = payload->dataSize;
        // memcpy(&byteStream[7], payload->data, payload->dataSize);

        for (uint16 byte = 0; byte < payload->dataSize; byte++)
            (*stream)[7 + byte] = 55;//payload->data[byte];
    }

    for (int k = 0; k < (*streamSize - 2); k++)
        printf ("%02x ", (*stream)[k]);
    printf("\n");

    uint16 crc = modbus_evaluateCRC(*stream, *streamSize - 2);
    (*stream)[*streamSize-2] = low(crc);
    (*stream)[*streamSize-1] = high(crc);

    printf("crc: ");
    for (int k = 0; k < (*streamSize); k++)
        printf ("%02x ", (*stream)[k]);
    printf("\n");

    return;
    // *stream = byteStream;
}

uint16 modbus_evaluateCRC(uint8* data, uint16 length) {

    const uint16 polynomial = 0xA001;
    uint16 CRC = 0xFFFF;

    for (uint8 byte = 0; byte < length; byte++) {
        CRC ^= data[byte];
        for (uint8 i = 0; i < 8; i++)
            if (CRC & 0x01)
                CRC = (CRC >> 1) ^ polynomial;
            else
                CRC >>= 1;
    }

    return CRC;
}

