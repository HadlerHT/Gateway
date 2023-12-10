#include <stdio.h>
#include <stdlib.h>

#include "wifimanager.h"
#include "mqttclient.h"
#include "uartmanager.h"
#include "modbusserial.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/uart.h>

// Inter-character timeout (1.5 character times)
// #define INTER_CHARACTER_TIMEOUT (1000000 / BAUD_RATE * 15 / 10)
// Inter-frame timeout (3.5 character times)
// #define INTER_FRAME_TIMEOUT (1000000 / BAUD_RATE * 35 / 10)

void gatewayHandler(void*, esp_event_base_t, int32_t, void*);

void app_main(void) {

    wifi_espSetup();
    wifi_initializeStation();
    
    mqtt_clientStart();
    mqtt_setDataEventHandler(gatewayHandler);

    modbus_initialize();

    // uint8 pack[] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00};
    // uint16 crc = modbus_evaluateCRC(pack, sizeof(pack) - 2);
    // pack[sizeof(pack)-1] = high(crc);
    // pack[sizeof(pack)-2] = low(crc);

    // uint8 buffer[264];
    // modbus_readData(buffer, 264);

    while(true) {

        vTaskDelay(100);

    };
};


// This function is called every time a message is published on this device mqtt topic
void gatewayHandler(void *handlerArgs, esp_event_base_t base, int32_t eventId, void *eventData) {   

    esp_mqtt_event_handle_t event = eventData;

    // printf("Topic: %.*s\n", event->topic_len, event->topic);
    
    // printf("HEX: ");
    // for (uint8 k = 0; k < event->data_len; k++)
    //     printf("%02x ", event->data[k]);
    // printf("\n");

    // printf("ASCII: ");
    // for (uint8 k = 0; k < event->data_len; k++)
    //     printf("%c", event->data[k]);
    // printf("\n");


    // ======== PARSE PAYLOAD ==========================================================

    // Evaluates the payload size in fields
    uint16 fieldCounter = 1;
    for (uint16 k = 2; k < event->data_len; k++)
        if (event->data[k] == ',')
            fieldCounter++;

    // Creates a buffer of adequate size to acomodate the data, + 2 bytes for CRC
    uint8 payload[fieldCounter + 2];

    // Helps simplify border case
    event->data[0] = ',';
    event->data[event->data_len - 1] = ',';

    // Parses ASCII encoded packet to byte vector
    uint16 commaIndex = 0;
    for (uint16 field = 0; field < fieldCounter; field++) {

        while(event->data[++commaIndex] != ',');

        char digitAsStr[] = {'0', '0', '0', '\0'};
        uint8 digitCounter = 3;

        for (uint8 digitIndex = commaIndex - 1; event->data[digitIndex] != ','; digitIndex--)
                digitAsStr[--digitCounter] = event->data[digitIndex];

        payload[field] = (uint8)atoi(digitAsStr);
    }   
    // ======== END OF PARSE PAYLOAD ===================================================


    // ======== ADD CRC TO PAYLOAD =====================================================

    uint16 crc = modbus_evaluateCRC(payload, fieldCounter);

    payload[fieldCounter] = low(crc);
    payload[fieldCounter + 1] = high(crc);

    // ======== END OF ADD CRC TO PAYLOAD ==============================================


    // ======== SEND PARSED PAYLOAD ====================================================    




    // ======== END OF SEND PARSED PAYLOAD =============================================



    // , event->data_len, event->datae

    //-----------------------------------------------------
    
    // printf("MQTT(%d)\n", cnt++);
    // // for (unsigned k = 0; k < event->data_len; k++)
    // //     printf("%02x ", event->data[k]);
    // // printf("\n");

    // int txlen = event->data_len + 2;
    // uint8 request[txlen];
    // for (unsigned k = 0; k < event->data_len; k++)
    //     request[k] = event->data[k];

    // uint16 crc = modbus_evaluateCRC(request, txlen - 2);
    // request[txlen - 1] = high(crc);
    // request[txlen - 2] = low(crc);
    
    // printf("TX: ");
    // for (unsigned k = 0; k < txlen; k++)
    //     printf("%02x ", request[k]);
    // printf("\n");

    // uart_sendRequestPacket(request, txlen);
    
    // //-----------------------------------------------------
    // uint8 response[264];
    // int rxlen = readModbusData(UART_ID, response, 264, 750);


    // // int rxlen = uart_readResponsePacket(response, 264);
    
    // printf("RX: ");    
    // for (unsigned k=0; k < rxlen; k++)
    //     printf("%02x ", response[k]);
    // printf("\n");

}