#pragma once

#include "datatypes.h"

#include "freertos/FreeRTOS.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#define UART_ID 1 // Which uart hardware to use
#define RX_PIN 8
#define TX_PIN 3
#define RTS_PIN 4 //Request to Send (Driver Enable in RS485)
#define CTS_PIN UART_PIN_NO_CHANGE //Clear to Send

// Variable declarations
extern uart_config_t uart_configure;

void uart_initiliaze();
void uart_sendRequestPacket(uint8*, uint16);
uint16 uart_readResponsePacket(uint8*, uint16);
