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

#define WIFI_CONNECTED_BIT BIT0     //  Connected to the AP with an IP
#define WIFI_FAIL_BIT      BIT1     //  Failed to connect after the maximum amount of retries
#define EXAMPLE_ESP_MAXIMUM_RETRY 5

class WiFiManager {
    public:
        WiFiManager();
        void connect();

    protected:
        static EventGroupHandle_t wifiEventGroup;
        static const char *TAG;
        static int retryNumber;

    private:
        static void initializeStation();
        static void eventRouter(void*, esp_event_base_t, int32_t, void*);
        static void eventHandler(esp_event_base_t, int32_t, void*);
};