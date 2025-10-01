#include "config.h"

void app_main(void) 
{

    // Инициализируем модуль wifi и настраиваем передачу данных через MQTT
    ESP_ERROR_CHECK(esp_wifi_mqtt_conf());

    // Создаем задачу, которая считывает данные с датчика, и отправляет их
    xTaskCreate(dht11_task, "DHT11", 2048, &DHT_PIN, 5, NULL);
}
