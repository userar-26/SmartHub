#include "config.h"

static const char *TAG = "WIFI_MQTT";

void dht11_task(void *PIN)
{
    esp_err_t res;
    float humidity, temperature;
    int dht_pin = *(int*)PIN;

    for (;;) {

        if(g_is_synced)
        {

            int temp_offset;
            portENTER_CRITICAL(&reading_mux);
            temp_offset = g_add_temp;
            portEXIT_CRITICAL(&reading_mux);

            res = dht_read_float_data(DHT_TYPE_DHT11, dht_pin, &humidity, &temperature);

            if (res == ESP_OK) {

                temperature += temp_offset;

                char *msg = create_sensor_data_json(temperature,humidity);
                if(msg == NULL){
                    ESP_LOGE(TAG,"Не удалось выделить память для сообщения с значением текущей температуры и влажности\n");
                    abort();
                }

                int msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_ESP32_TO_HUB, msg, strlen(msg), 2, 1);

                free(msg);

                if (msg_id == -1)
                    ESP_LOGE(TAG, "Ошибка отправки MQTT сообщения");
                else
                    ESP_LOGI(TAG, "Сообщение с данными о температуре/влажности успешно отправлено, msg_id=%d", msg_id);
            } 
            else
                ESP_LOGE("DHT11", "Не удалось прочитать значения с DHT11");

            vTaskDelay(3000 / portTICK_PERIOD_MS);
        }
        else
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        
    }
}
