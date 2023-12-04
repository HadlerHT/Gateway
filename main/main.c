#include <stdio.h>

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
int readModbusData(uart_port_t uart_num, uint8* buffer, size_t bufferSize, int interSymbolTimeoutMillis);


void app_main(void) {

    wifi_espSetup();
    wifi_initializeStation();
    
    uart_initiliaze();
    
    mqtt_clientStart();
    mqtt_setDataEventHandler(gatewayHandler);

    // uint8 pack[] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00};
    // uint16 crc = modbus_evaluateCRC(pack, sizeof(pack) - 2);
    // pack[sizeof(pack)-1] = high(crc);
    // pack[sizeof(pack)-2] = low(crc);
    
    while(true) {
    
        // uart_sendRequestPacket(pack, sizeof(pack));

        // for (unsigned k=0; k < sizeof(pack); k++)
        //     printf("%02x ", pack[k]);
        // printf("\n");

        vTaskDelay(100);

    };
};



// Function to read Modbus serial data with timeout detection
int readModbusData(uart_port_t uart_num, uint8* buffer, size_t bufferSize, int interSymbolTimeoutMicros) {
    int bytesRead = 0;
    uint8 currentChar;


    while (1) {
        int uart_read_len = uart_read_bytes(uart_num, (uint8_t*)&currentChar, 1, 7); //pdUS_TO_TICKS(interSymbolTimeoutMicros));

        if (uart_read_len == 0) {
            // Timeout reached
            break;
        } 
        else if (uart_read_len < 0) {
            // Handle error
            // uint8 TAG[] = "UART";
            // ESP_LOGE(TAG, "UART read error: %s", esp_err_to_name(uart_read_len));
            return -1;
        } 
        else {
            // Character successfully read
            buffer[bytesRead++] = currentChar;

            // Check for buffer overflow
            if (bytesRead >= bufferSize) {
                break;
            }
        }
    }

    return bytesRead;
}




// This function is called every time a message is published on this device mqtt topic
void gatewayHandler(void *handlerArgs, esp_event_base_t base, int32_t eventId, void *eventData) {   

    esp_mqtt_event_handle_t event = eventData;
    static uint16 cnt = 0;

    //-----------------------------------------------------
    
    printf("MQTT(%d)\n", cnt++);
    // for (unsigned k = 0; k < event->data_len; k++)
    //     printf("%02x ", event->data[k]);
    // printf("\n");

    int txlen = event->data_len + 2;
    uint8 request[txlen];
    for (unsigned k = 0; k < event->data_len; k++)
        request[k] = event->data[k];

    uint16 crc = modbus_evaluateCRC(request, txlen - 2);
    request[txlen - 1] = high(crc);
    request[txlen - 2] = low(crc);
    
    printf("TX: ");
    for (unsigned k = 0; k < txlen; k++)
        printf("%02x ", request[k]);
    printf("\n");

    uart_sendRequestPacket(request, txlen);
    
    //-----------------------------------------------------
    uint8 response[264];
    int rxlen = readModbusData(UART_ID, response, 264, 750);


    // int rxlen = uart_readResponsePacket(response, 264);
    
    printf("RX: ");    
    for (unsigned k=0; k < rxlen; k++)
        printf("%02x ", response[k]);
    printf("\n");

}