#include "uartmanager.h"

// Macros
#define BAUDRATE 115200

// Variable initializations
// static uint16 = 

// UART Configure
uart_config_t uart_configure = {
    .baud_rate = BAUDRATE,
    .data_bits = UART_DATA_8_BITS,
    .parity    = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_DEFAULT,
};

// time out for modbus packet
// static const uint16 uart_timeOut = (double) (264 * 11 * 1000 / BAUDRATE) / portTICK_PERIOD_MS;

void uart_initiliaze() {

    uart_driver_install(UART_ID, 264, 0, 0, NULL, 0);
    uart_param_config(UART_ID, &uart_configure);
    uart_set_pin(UART_ID, TX_PIN, RX_PIN, RTS_PIN, CTS_PIN);
    uart_set_mode(UART_ID, UART_MODE_RS485_HALF_DUPLEX);

}


void uart_sendRequestPacket(uint8* data, uint16 length) {
    uart_write_bytes(UART_ID, data, length);
};


uint16 uart_readResponsePacket(uint8* data, uint16 length) {

    int len = uart_read_bytes(UART_ID, data, length, 400/portTICK_PERIOD_MS); 
    return len;
};
