#include "wifimanager.h"

#define WIFI_MAXIMUM_RETRY 5
#define WIFI_CONNECTED_BIT BIT0     //  Connected to the AP with an IP
#define WIFI_FAIL_BIT      BIT1     //  Failed to connect after the maximum amount of retries

// Esp log
static const char* TAG = "Wifi Station";

// Variable definitions
EventGroupHandle_t wifi_eventGroup;
wifi_config_t wifi_configure = {
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_wifi.html
    .sta = {
        .ssid = CONFIG_WIFI_SSID,
        .password = CONFIG_WIFI_PASSWORD,
        .scan_method = WIFI_ALL_CHANNEL_SCAN,
        .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
        .threshold = {
            .rssi = -127, 
            .authmode = WIFI_AUTH_WPA2_PSK,
        },
    },
};


void wifi_eventHandler(void* arg, esp_event_base_t eventBase, int32_t eventID, void* eventData) {

    int retryNumber = 0;

    if (eventBase == WIFI_EVENT && eventID == WIFI_EVENT_STA_START)
        esp_wifi_connect();
    
    else if (eventBase == WIFI_EVENT && eventID == WIFI_EVENT_STA_DISCONNECTED) {
        if (retryNumber < WIFI_MAXIMUM_RETRY) {
            esp_wifi_connect();
            retryNumber++;
            ESP_LOGI(TAG, "Retrying to connect to the AP");
        } 
        else
            xEventGroupSetBits(wifi_eventGroup, WIFI_FAIL_BIT);

        ESP_LOGI(TAG,"Connect to the AP failed");
    } 
    
    else if (eventBase == IP_EVENT && eventID == IP_EVENT_STA_GOT_IP) {
        // ip_event_got_ip_t* event = (ip_event_got_ip_t*) eventData;
        retryNumber = 0;
        xEventGroupSetBits(wifi_eventGroup, WIFI_CONNECTED_BIT);
    }
}

void wifi_espSetup() {

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    
}

void wifi_initializeStation() {
    
    wifi_eventGroup = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instanceAnyId;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_eventHandler, NULL, &instanceAnyId));

    esp_event_handler_instance_t instanceGotIp;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_eventHandler, NULL, &instanceGotIp));
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_configure) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "Wifi Station Initialized.");

    EventBits_t bits = xEventGroupWaitBits(wifi_eventGroup,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to ap %s", CONFIG_WIFI_SSID);
    } 
    else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to ap %s", CONFIG_WIFI_SSID);
    } 
    else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instanceGotIp));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instanceAnyId));
    vEventGroupDelete(wifi_eventGroup);
}

void wifi_disconnect() {
    ESP_ERROR_CHECK(esp_wifi_disconnect());
}
