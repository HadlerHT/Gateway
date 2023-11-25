#include "wifimanager.h"

// Static member variable initialization
EventGroupHandle_t WiFiManager::wifiEventGroup = nullptr;
const char* WiFiManager::TAG = "Wifi Station";
int WiFiManager::retryNumber = 0;

void WiFiManager::eventRouter(void* arg, esp_event_base_t eventBase, int32_t eventID, void* eventData) {
    WiFiManager::eventHandler(eventBase, eventID, eventData);
}

void WiFiManager::eventHandler(esp_event_base_t eventBase, int32_t eventID, void* eventData) {
    if (eventBase == WIFI_EVENT && eventID == WIFI_EVENT_STA_START)
        esp_wifi_connect();
    
    else if (eventBase == WIFI_EVENT && eventID == WIFI_EVENT_STA_DISCONNECTED) {
        if (retryNumber < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            retryNumber++;
            ESP_LOGI(TAG, "Retrying to connect to the AP");
        } 
        else
            xEventGroupSetBits(wifiEventGroup, WIFI_FAIL_BIT);

        ESP_LOGI(TAG,"Connect to the AP failed");
    } 
    
    else if (eventBase == IP_EVENT && eventID == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) eventData;
        ESP_LOGI(TAG, "Got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        retryNumber = 0;
        xEventGroupSetBits(wifiEventGroup, WIFI_CONNECTED_BIT);
    }
}


void WiFiManager::initializeStation() {
    
    wifiEventGroup = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instanceAnyId;
    esp_event_handler_instance_t instanceGotIp;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &eventRouter,
                                                        NULL,
                                                        &instanceAnyId));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &eventRouter,
                                                        NULL,
                                                        &instanceGotIp));

    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_wifi.html#_CPPv413wifi_config_t
    wifi_config_t wifiConfig = {
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
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "Wifi Station Initialized.");

    EventBits_t bits = xEventGroupWaitBits(wifiEventGroup,
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
    vEventGroupDelete(wifiEventGroup);
}

void WiFiManager::connect() {
    initializeStation();
}

WiFiManager::WiFiManager() {

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
}
