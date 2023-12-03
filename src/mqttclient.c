#include "mqttclient.h"

// Esp log
static const char *TAG = "MQTT_EXAMPLE";

// Variable definitions
esp_mqtt_client_handle_t mqtt_client;

const char* mqtt_deviceTopic = "/device1";

esp_mqtt_client_config_t mqtt_configure = {
    .broker.address.uri = "mqtt://192.168.0.6",
    .broker.address.port = 1883,
};


/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
void mqtt_controlEventsHandler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    switch ((esp_mqtt_event_id_t)event_id) {
    
    case MQTT_EVENT_CONNECTED:
        msg_id = esp_mqtt_client_subscribe(client, mqtt_deviceTopic, 0);
        ESP_LOGI(TAG, "Sent subscribe successful, msg_id=%d", msg_id);
        break;
    
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "Published by self");
        break;
    
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_dataEventHandler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {

    esp_mqtt_event_handle_t event = event_data;
    // esp_mqtt_client_handle_t client = event->client;
    // int msg_id;

    ESP_LOGI(TAG, "MQTT_EVENT_DATA");
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);
    
}


void mqtt_clientStart() {

    mqtt_client = esp_mqtt_client_init(&mqtt_configure);
    
    // Handler for control events in mqtt
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_controlEventsHandler, NULL);
    esp_mqtt_client_unregister_event(mqtt_client, MQTT_EVENT_DATA, mqtt_controlEventsHandler);

    // Handler for data event in mqtt
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_dataEventHandler, NULL);
    
    esp_mqtt_client_start(mqtt_client);
}

// extern "C" void app_main(void) {
    
//     ESP_LOGI(TAG, "[APP] Startup..");
//     ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
//     ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

//     esp_log_level_set("*", ESP_LOG_INFO);
//     esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
//     esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
//     esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
//     esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
//     esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
//     esp_log_level_set("outbox", ESP_LOG_VERBOSE);

//     ESP_ERROR_CHECK(nvs_flash_init());
//     ESP_ERROR_CHECK(esp_netif_init());
//     ESP_ERROR_CHECK(esp_event_loop_create_default());

//     /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
//      * Read "Establishing Wi-Fi or Ethernet Connection" section in
//      * examples/protocols/README.md for more information about this function.
//      */
//     ESP_ERROR_CHECK(example_connect());

//     mqtt_app_start();

//     const char jsonString[] = 
//         "{"
//         "    \"request\" : {"
//         "        \"id\": 1,"
//         "        \"function\": \"read\","
//         "        \"data-raw\": {"
//         "            \"offset\": 0,"
//         "            \"amount\": 16"
//         "        },"
//         "        \"data\": {"
//         "            \"from\": 0,"
//         "            \"to\": 32"
//         "        }"
//         "    }"
//         "}";

//     // int msg_id;
//     while (true) {
//         esp_mqtt_client_publish(client, "/device1", jsonString, 0, 0, 0);
//         vTaskDelay(500);
    
//         // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
//     }

// }


void log_error_if_nonzero(const char *message, int error_code) {
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}
