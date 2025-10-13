#include "config.h"

// Инициализируем глобальные переменные
int g_is_synced = 0;
int g_add_temp  = 0;
int g_is_light_on = 0;
esp_mqtt_client_handle_t mqtt_client = NULL;
portMUX_TYPE reading_mux = portMUX_INITIALIZER_UNLOCKED;

void app_main(void) 
{
    // Настраиваем пин для света
    gpio_reset_pin(LIGHT_PIN);
    gpio_set_direction(LIGHT_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LIGHT_PIN,1);

    // Инициализируем статические JSON-сообщения
    if (init_esp32_json_messages() != ESP_OK)
        abort();

    // Инициализируем модуль wifi и настраиваем передачу данных через MQTT
    esp_wifi_mqtt_conf();

    // Создаем задачу, которая считывает данные с датчика, и отправляет их
    xTaskCreate(dht11_task, "DHT11", 2048, (void *const)&DHT_PIN, 5, NULL);
}
