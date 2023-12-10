#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "datatypes.h"

// Variable declarations
extern const char mqtt_deviceID[];
extern char mqtt_topic[];

extern esp_mqtt_client_handle_t mqtt_client;
extern esp_mqtt_client_config_t mqtt_configure;

void mqtt_clientStart();
void mqtt_publishMessage(char*, uint16);
void mqtt_setDataEventHandler(esp_event_handler_t);
void mqtt_controlEventsHandler(void*, esp_event_base_t, int32_t, void*);

void log_error_if_nonzero(const char*, int);
