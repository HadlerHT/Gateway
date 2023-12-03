#pragma once

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "sdkconfig.h"

#define WIFI_MAXIMUM_RETRY 5
#define WIFI_CONNECTED_BIT BIT0     //  Connected to the AP with an IP
#define WIFI_FAIL_BIT      BIT1     //  Failed to connect after the maximum amount of retries

// Variable declarations
extern EventGroupHandle_t wifi_eventGroup;
extern wifi_config_t wifi_configure;

// Functions
void wifi_espSetup();
void wifi_initializeStation();
void wifi_eventHandler(void*, esp_event_base_t, int32_t, void*);
void wifi_disconnect();