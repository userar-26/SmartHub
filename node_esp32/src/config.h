#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "nvs_flash.h"
#include "dht.h"

// Конфигурация Wi-Fi и MQTT
#define WIFI_SSID       "Xiaomi_541D"
#define WIFI_PASS       "XTA210991926"
#define BROKER_URI      "mqtt://192.168.31.166:1883"
#define ESP32_TOPIC     "smarthub/esp32"
#define LINUX_TOPIC     "smarthub/linux"

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
void dht11_task(void *PIN);
esp_err_t esp_wifi_mqtt_conf(void);
void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
