#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "nvs_flash.h"
#include "dht.h"
#include "smarthub_protocol.h"
#include "cJSON.h"

// Конфигурация Wi-Fi и MQTT
#define WIFI_SSID       "Xiaomi_541D"
#define WIFI_PASS       "XTA210991926"
#define BROKER_URI      "mqtt://192.168.31.166:1883"

// Параметры/переменные для датчика DHT11
#define INC_TEMPERATURE 'u'
#define DEC_TEMPERATURE 'd'
static const int DHT_PIN = GPIO_NUM_4;

// Глобальные переменные
extern esp_mqtt_client_handle_t mqtt_client;
extern int add_temp;
extern portMUX_TYPE reading_mux;

// Тип данных для DHT
typedef struct {
    int16_t temperature;
    int16_t humidity;
} dht_reading_t;

// Прототипы функций

// ------------------------------------------------
// Задача для периодического опроса датчика DHT11.
// Считывает температуру и влажность, после чего публикует
// данные в топик ESP32_TOPIC.
// ----------------------------------------------
void dht11_task(void *PIN);

// ------------------------------------------------
// Инициализирует и запускает Wi-Fi в режиме станции (STA) и
// регистрирует обработчики событий для подключения к сети и
// запуска MQTT.
//
// Возвращает ESP_OK в случае успеха.
// ------------------------------------------------
esp_err_t esp_wifi_mqtt_conf(void);

// ------------------------------------------------
// Обработчик событий MQTT-клиента.
// Вызывается библиотекой MQTT при подключении, отключении или получении
// данных. Обрабатывает команды изменения температуры от Linux-хаба.
// ------------------------------------------------
void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

// ------------------------------------------------
// Обработчик системных событий Wi-Fi и IP стека.
// Управляет логикой подключения к Wi-Fi и запускает MQTT-клиент
// только после успешного получения IP-адреса.
// ------------------------------------------------
void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
