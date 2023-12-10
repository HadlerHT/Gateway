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

    esp_log_level_set("*", ESP_LOG_INFO);

    wifi_espSetup();
    wifi_initializeStation();
    
    mqtt_clientStart();
    mqtt_setDataEventHandler(gatewayHandler);

    modbus_initialize();
};


// This function is called every time a message is published on this device mqtt topic
// It handles the decoding of the message coming from mqtt
// Sends it via uart to the modbus network and awaits the response
// It then reencodes the message and sends it back to the mqtt broker
void gatewayHandler(void *handlerArgs, esp_event_base_t base, int32_t eventId, void *eventData) {   

    esp_mqtt_event_handle_t event = eventData;

    // Message sent by itself - IGNORE
    if (event->data[0] == 0xFF)
        return;

    ESP_LOGI("TIME_DEBUG", "1");

    // ======== PARSE PAYLOAD ==========================================================
    // Evaluates the payload size in fields
    uint16 fieldCounter = 1;
    for (uint16 chr = 1; chr < event->data_len; chr++)
        if (event->data[chr] == ',')
            fieldCounter++;

    // Creates a buffer of adequate size to acomodate the data, +2 bytes for CRC
    uint8 payload[fieldCounter + 2];

    // Helps simplify border case
    event->data[0] = ',';
    event->data[event->data_len - 1] = ',';

    // Parses ASCII encoded packet to byte vector
    uint16 commaIndex = 0;
    for (uint16 field = 0; field < fieldCounter; field++) {

        while(event->data[++commaIndex] != ',');

        char valueAsStr[] = {'0', '0', '0', '\0'};
        uint8 digitCounter = 3;

        for (uint8 digitIndex = commaIndex - 1; event->data[digitIndex] != ','; digitIndex--)
            valueAsStr[--digitCounter] = event->data[digitIndex];

        payload[field] = (uint8)atoi(valueAsStr);
    }   
    // ======== END OF PARSE PAYLOAD ===================================================
    
    ESP_LOGI("TIME_DEBUG", "2");

    // ======== ADD CRC TO PAYLOAD =====================================================
    uint16 crc = modbus_evaluateCRC(payload, fieldCounter);

    payload[fieldCounter] = low(crc);
    payload[fieldCounter + 1] = high(crc);
    // ======== END OF ADD CRC TO PAYLOAD ==============================================

    ESP_LOGI("TIME_DEBUG", "3");

    // ======== SEND PAYLOAD TO UART AND AWAIT RESPONSE ================================    
    uint8 response[265];
    int responseLen;

    for (uint8 attemps = 0; attemps < 3; attemps++) {
    
        modbus_sendRequestPacket(payload , sizeof(payload));
        responseLen = modbus_readResponsePacket((uint8*)(response + 1), 264, 500); //Timeout of 500ms

        printf("%d \n", responseLen);

        // If the response is non-null and the modbus checks out: success
        if (responseLen > 0 && !modbus_evaluateCRC(response, responseLen))
            break;
    }
    // ======== END OF SEND PAYLOAD TO UART AND AWAIT RESPONSE =========================

    ESP_LOGI("TIME_DEBUG", "4");

    // ======== ERROR HANDLING =========================================================
    if (responseLen < 1) {
        strcpy((char*)response, "_{\"status\":\"error\"}");
        responseLen = sizeof(response);
    }
    // ======== END OF ERROR HANDLING ==================================================

    ESP_LOGI("TIME_DEBUG", "5");

    // ======== PUBLISH MESSAGE TO MQTT TOPIC ==========================================
    // Indicating that this packet was created on the esp32, mqtt flow control
    // This cannot be confused with the slave id because 0xFF is out of bounds for that field
    response[0] = 0xFF; 

    // Publish: control + payload - crc
    mqtt_publishMessage((char*)response, responseLen-1);
    // ======== PUBLISH MESSAGE TO MQTT TOPIC ==========================================

    ESP_LOGI("TIME_DEBUG", "6");
}