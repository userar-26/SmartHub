#include "config.h"

// Определение глобальных переменных
esp_mqtt_client_handle_t mqtt_client = NULL;
int add_temp = 0;
portMUX_TYPE reading_mux = portMUX_INITIALIZER_UNLOCKED;

static const char *TAG = "WIFI_MQTT";

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    
    switch (event->event_id) {

        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Установлено соединение с брокером MQTT");
            int msg_id = esp_mqtt_client_subscribe(mqtt_client, TOPIC_HUB_TO_ESP32, 2);
            if (msg_id == -1)
                ESP_LOGE(TAG, "Ошибка отправки команды на подписку на топик '%s'", TOPIC_HUB_TO_ESP32); 
            else
                ESP_LOGI(TAG, "Команда на подписку на топик '%s' успешно отправлена, msg_id=%d", TOPIC_HUB_TO_ESP32, msg_id);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "Разорвано соединение с брокером MQTT");
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "Получено сообщение от %.*s: %.*s",
                     event->topic_len, event->topic,
                     event->data_len, event->data);

            if (event->data_len == 1) {
                char command = event->data[0];
                if (command == INC_TEMPERATURE || command == DEC_TEMPERATURE) {
                    // Используем критическую секцию, так как переменная add_temp
                    // читается в задаче dht11_task, что может привести к гонке данных.
                    portENTER_CRITICAL(&reading_mux);
                    if (command == INC_TEMPERATURE) {
                        add_temp++;
                    } else {
                        add_temp--;
                    }
                    portEXIT_CRITICAL(&reading_mux);
                }
                else {
                    ESP_LOGE(TAG,"Получена неизвестная команда: %c\n",command);
                }
            }
            else {
                ESP_LOGE(TAG,"Получена команда [%.*s] с неверным размером: %d(ожидался 1)\n",event->data,event->data_len,event->data_len);
            }
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
