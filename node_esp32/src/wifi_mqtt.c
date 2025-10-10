#include "config.h"

static const char *TAG = "WIFI_MQTT";

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    
    switch (event->event_id) {

        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Подключено к MQTT. Подписка на команды...");
            esp_mqtt_client_subscribe(mqtt_client, TOPIC_HUB_TO_ESP32, 1);
            
            // Отправляем хабу запрос на получение полного состояния.
            // Это решает сценарий, когда ESP32 перезапустился, а хаб работал.
            cJSON *root = cJSON_CreateObject();
            cJSON_AddStringToObject(root, KEY_COMMAND, CMD_GET_STATE);
            char *msg = cJSON_PrintUnformatted(root);
            
            if (msg) {
                esp_mqtt_client_publish(mqtt_client, TOPIC_ESP32_TO_HUB, msg, strlen(msg), 1, 0);
                free(msg);
            }
            cJSON_Delete(root);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "Разорвано соединение с брокером MQTT");
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "Получено сообщение от %.*s: %.*s",
                     event->topic_len, event->topic,
                     event->data_len, event->data);
            process_json_command(event->data, event->data_len);
            break;
        default:
            break;
    }
}

static void wifi_event_handler_internal(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    // Wi-Fi станция стартовала, можно начинать подключение к точке доступа.
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_ERROR_CHECK(esp_wifi_connect());
    }
    // Потеряли соединение с точкой доступа, пытаемся переподключиться.
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Wi-Fi отключился, переподключение...");
        ESP_ERROR_CHECK(esp_wifi_connect());
    }
    // Устройство успешно получило IP-адрес от роутера.
    // Теперь можно безопасно инициализировать и запустить MQTT-клиент.
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "IP получен, запуск MQTT...");
        if (mqtt_client == NULL) {
            esp_mqtt_client_config_t mqtt_cfg = {
                .broker.address.uri = BROKER_URI,
                .credentials.client_id = "ESP32_01",
            };
            mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
            ESP_ERROR_CHECK(esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));
            ESP_ERROR_CHECK(esp_mqtt_client_start(mqtt_client));
        }
    }
}

esp_err_t esp_wifi_mqtt_conf(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler_internal, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler_internal, NULL));

    wifi_config_t wifi_cfg = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    return ESP_OK;
}
