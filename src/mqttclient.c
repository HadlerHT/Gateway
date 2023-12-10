#include "mqttclient.h"

// Esp log
static const char *TAG = "MQTT";

// Variable definitions
esp_mqtt_client_handle_t mqtt_client;

// MQTT Configuration
const char mqtt_deviceID[] = "benchmark";
char mqtt_topic[sizeof(mqtt_deviceID) + 20];

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
void mqtt_controlEventsHandler(void *handlerArgs, esp_event_base_t base, int32_t eventId, void *eventData) {
    
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, eventId=%" PRIi32 "", base, eventId);
    
    esp_mqtt_event_handle_t event = eventData;
    
    switch ((esp_mqtt_event_id_t)eventId) {
    
    case MQTT_EVENT_CONNECTED:
        sprintf(mqtt_topic, "/mbnet/%.*s/mbbus", sizeof(mqtt_deviceID), mqtt_deviceID);
        printf("%.*s\n", sizeof(mqtt_deviceID) + 16, mqtt_topic);

        esp_mqtt_client_subscribe(event->client, mqtt_topic, 2);        
        break;
    
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msgId=%d", event->msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msgId=%d", event->msg_id);
        break;
    
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msgId=%d", event->msg_id);
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

void mqtt_clientStart() {

    mqtt_client = esp_mqtt_client_init(&mqtt_configure);
    
    // Handler for control events in mqtt - all except mqtt_event_data
    const esp_mqtt_event_id_t mqtt_eventsForInternalHandling[] = {
        MQTT_EVENT_CONNECTED,  MQTT_EVENT_DISCONNECTED,
        MQTT_EVENT_SUBSCRIBED, MQTT_EVENT_UNSUBSCRIBED,
        MQTT_EVENT_PUBLISHED,  MQTT_EVENT_ERROR,
    };
    for (uint8 k = 0; k < 6; k++)
        esp_mqtt_client_register_event(mqtt_client, mqtt_eventsForInternalHandling[k], mqtt_controlEventsHandler, NULL);

    esp_mqtt_client_start(mqtt_client);
}

void mqtt_publishMessage(char* payload, uint16 payloadLen) {
    esp_mqtt_client_publish(mqtt_client, mqtt_topic, payload, payloadLen, 2, false);
}

void mqtt_setDataEventHandler(esp_event_handler_t handler) {
    esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_DATA, handler, NULL);
}

void log_error_if_nonzero(const char *message, int error_code) {
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}
