#include "config.h"

static const char *TAG = "WIFI_MQTT";

void dht11_task(void *PIN)
{
    esp_err_t res;
    dht_reading_t reading;
    int dht_pin = *(int*)PIN;

    for (;;) {

        res = dht_read_data(DHT_TYPE_DHT11, dht_pin, &reading.humidity, &reading.temperature);

        if (res == ESP_OK) {
            dht_reading_t reading_to_send = reading;
            // Критическая секция необходима для безопасного доступа к add_temp,
            // так как эта переменная изменяется в обработчике MQTT.
            portENTER_CRITICAL(&reading_mux);
            reading_to_send.temperature += (add_temp * 10);
            portEXIT_CRITICAL(&reading_mux);

            int msg_id = esp_mqtt_client_publish(mqtt_client, ESP32_TOPIC, (const char *)&reading_to_send, sizeof(reading_to_send), 2, 1);

            if (msg_id == -1)
                ESP_LOGE(TAG, "Ошибка отправки MQTT сообщения (например, клиент отключен)");
            else
                ESP_LOGI(TAG, "Сообщение с данными о температуре/влажности успешно отправлено, msg_id=%d", msg_id);
        } 
        else
            ESP_LOGE("DHT11", "Не удалось прочитать значения с DHT11");

        vTaskDelay(3000 / portTICK_PERIOD_MS);
        
    }
}
