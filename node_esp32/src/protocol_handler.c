#include "config.h"

char *g_json_resp_light_on = NULL;
char *g_json_resp_light_off = NULL;
char *g_json_resp_command_error = NULL;

static const char *TAG = "PROTOCOL_HANDLER";

char* create_sensor_data_json(float temp, float humidity)
{
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        return NULL;
    }

    cJSON_AddStringToObject(root, KEY_TYPE, TYPE_EVENT);
    cJSON_AddStringToObject(root, KEY_DEVICE, DEVICE_DHT_SENSOR);

    cJSON *values_obj = cJSON_CreateObject();
    if (values_obj == NULL) {
        cJSON_Delete(root);
        return NULL;
    }
    cJSON_AddItemToObject(root, KEY_VALUES, values_obj);

    cJSON_AddNumberToObject(values_obj, KEY_VALUE_TEMPERATURE, temp);
    cJSON_AddNumberToObject(values_obj, KEY_VALUE_HUMIDITY, humidity);

    char *json_string = cJSON_PrintUnformatted(root);

    cJSON_Delete(root);

    return json_string;
}

esp_err_t init_esp32_json_messages(void)
{
    cJSON *root = NULL;
    esp_err_t ret = ESP_OK;

    // --- 1. Ответ: Свет успешно включен ---
    root = cJSON_CreateObject();
    if (root == NULL) { ret = ESP_ERR_NO_MEM; goto error; }
    
    if (cJSON_AddStringToObject(root, KEY_TYPE, TYPE_RESPONSE) == NULL ||
        cJSON_AddStringToObject(root, KEY_DEVICE, DEVICE_LIVING_ROOM_LIGHT) == NULL ||
        cJSON_AddBoolToObject(root, KEY_STATE_IS_LIGHT_ON, true) == NULL)
    {
        ret = ESP_ERR_NO_MEM;
        goto error;
    }
    g_json_resp_light_on = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    root = NULL;
    if (g_json_resp_light_on == NULL) { ret = ESP_ERR_NO_MEM; goto error; }


    // --- 2. Ответ: Свет успешно выключен ---
    root = cJSON_CreateObject();
    if (root == NULL) { ret = ESP_ERR_NO_MEM; goto error; }

    if (cJSON_AddStringToObject(root, KEY_TYPE, TYPE_RESPONSE) == NULL ||
        cJSON_AddStringToObject(root, KEY_DEVICE, DEVICE_LIVING_ROOM_LIGHT) == NULL ||
        cJSON_AddBoolToObject(root, KEY_STATE_IS_LIGHT_ON, false) == NULL)
    {
        ret = ESP_ERR_NO_MEM;
        goto error;
    }
    g_json_resp_light_off = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    root = NULL;
    if (g_json_resp_light_off == NULL) { ret = ESP_ERR_NO_MEM; goto error; }


    // --- 3. Ответ: Ошибка команды ---
    root = cJSON_CreateObject();
    if (root == NULL) { ret = ESP_ERR_NO_MEM; goto error; }
    
    if (cJSON_AddStringToObject(root, KEY_TYPE, TYPE_RESPONSE) == NULL ||
        cJSON_AddStringToObject(root, KEY_STATUS, STATUS_ERROR) == NULL ||
        cJSON_AddStringToObject(root, KEY_REASON, REASON_UNKNOWN_COMMAND) == NULL)
    {
        ret = ESP_ERR_NO_MEM;
        goto error;
    }
    g_json_resp_command_error = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    root = NULL;
    if (g_json_resp_command_error == NULL) { ret = ESP_ERR_NO_MEM; goto error; }

    ESP_LOGI(TAG, "Статические JSON-сообщения успешно инициализированы.");
    return ESP_OK;

error:
    ESP_LOGE(TAG, "Не удалось инициализировать JSON-сообщения, не хватило памяти!");
    cJSON_Delete(root);
    cleanup_json_messages();
    return ret;
}

void cleanup_json_messages(void) {
    free(g_json_resp_light_on);
    g_json_resp_light_on = NULL;

    free(g_json_resp_light_off);
    g_json_resp_light_off = NULL;

    free(g_json_resp_command_error);
    g_json_resp_command_error = NULL;
}

void process_json_command(const char *data, int len)
{
    cJSON *root = NULL;
    
    // 1. Создаем безопасную, C-строку из входящих данных
    char command_str[len + 1];
    memcpy(command_str, data, len);
    command_str[len] = '\0';

    // 2. Парсим строку
    root = cJSON_Parse(command_str);
    if (root == NULL) {
        ESP_LOGE(TAG, "Ошибка парсинга JSON");
        if (g_json_resp_command_error) {
            esp_mqtt_client_publish(mqtt_client, TOPIC_ESP32_TO_HUB, g_json_resp_command_error, strlen(g_json_resp_command_error), 1, 0);
        }
        return;
    }

    // 3. Извлекаем команду
    cJSON *command_item = cJSON_GetObjectItemCaseSensitive(root, KEY_COMMAND);
    if (!cJSON_IsString(command_item)) {
        if (g_json_resp_command_error) {
            esp_mqtt_client_publish(mqtt_client, TOPIC_ESP32_TO_HUB, g_json_resp_command_error, strlen(g_json_resp_command_error), 1, 0);
        }
        goto cleanup;
    }

    // --- 4. Основная логика обработки команд ---

    // Команда: Запросить полное состояние
    if (strcmp(command_item->valuestring, CMD_GET_STATE) == 0)
    {
        ESP_LOGI(TAG, "Получен запрос состояния от хаба.");
        send_full_state_response();
    }
    // Команда: Увеличить температуру
    else if (strcmp(command_item->valuestring, CMD_TEMP_INCREASE) == 0)
    {
        portENTER_CRITICAL(&reading_mux);
        g_add_temp++;
        portEXIT_CRITICAL(&reading_mux);
        ESP_LOGI(TAG, "Температура будет увеличена. Новое смещение: %d", g_add_temp);
    }
    // Команда: Уменьшить температуру
    else if (strcmp(command_item->valuestring, CMD_TEMP_DECREASE) == 0)
    {
        portENTER_CRITICAL(&reading_mux);
        g_add_temp--;
        portEXIT_CRITICAL(&reading_mux);
        ESP_LOGI(TAG, "Температура будет уменьшена. Новое смещение: %d", g_add_temp);
    }
    // Команда: Установить состояние (для света)
    else if (strcmp(command_item->valuestring, CMD_SET_STATE) == 0)
    {
        cJSON *device_item = cJSON_GetObjectItemCaseSensitive(root, KEY_DEVICE);
        cJSON *state_item = cJSON_GetObjectItemCaseSensitive(root, KEY_STATE);

        if (cJSON_IsString(device_item) && strcmp(device_item->valuestring, DEVICE_LIVING_ROOM_LIGHT) == 0)
        {
            if (cJSON_IsString(state_item) && strcmp(state_item->valuestring, STATE_ON) == 0) {
                gpio_set_level(LIGHT_PIN, 1);
                g_is_light_on = true;
                ESP_LOGI(TAG, "Свет включен");
                if (g_json_resp_light_on) {
                    esp_mqtt_client_publish(mqtt_client, TOPIC_ESP32_TO_HUB, g_json_resp_light_on, strlen(g_json_resp_light_on), 1, 0);
                }
            } else if (cJSON_IsString(state_item) && strcmp(state_item->valuestring, STATE_OFF) == 0) {
                gpio_set_level(LIGHT_PIN, 0);
                g_is_light_on = false;
                ESP_LOGI(TAG, "Свет выключен");
                if (g_json_resp_light_off) {
                    esp_mqtt_client_publish(mqtt_client, TOPIC_ESP32_TO_HUB, g_json_resp_light_off, strlen(g_json_resp_light_off), 1, 0);
                }
            }
        }
    }
    // Ни одна команда не подошла
    else
    {
        if (g_json_resp_command_error) {
            esp_mqtt_client_publish(mqtt_client, TOPIC_ESP32_TO_HUB, g_json_resp_command_error, strlen(g_json_resp_command_error), 1, 0);
        }
    }

    // --- 5. Логика синхронизации ---
    if (!g_is_synced) {
        ESP_LOGI(TAG, "Синхронизация с хабом установлена!");
        g_is_synced = true;
    }

cleanup:
    cJSON_Delete(root);
}

void send_full_state_response(void)
{
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) return;

    cJSON_AddStringToObject(root, KEY_TYPE, TYPE_FULL_STATE);

    cJSON *state_obj = cJSON_CreateObject();
    cJSON_AddItemToObject(root, KEY_STATE, state_obj);

    // Добавляем актуальные значения состояния
    cJSON_AddBoolToObject(state_obj, KEY_STATE_IS_LIGHT_ON, g_is_light_on);
    
    // Считываем текущее значения температуры и влажности
    float temperature,humidity;
    dht_read_float_data(DHT_TYPE_DHT11,DHT_PIN,&humidity,&temperature);
    portENTER_CRITICAL(&reading_mux);
    temperature += g_add_temp;
    portEXIT_CRITICAL(&reading_mux);

    // Добавляем значения температуры и влажности
    cJSON_AddNumberToObject(state_obj,KEY_VALUE_TEMPERATURE, temperature);
    cJSON_AddNumberToObject(state_obj,KEY_VALUE_HUMIDITY, humidity);

    char *json_string = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (json_string != NULL) {
        esp_mqtt_client_publish(mqtt_client, TOPIC_ESP32_TO_HUB, json_string, strlen(json_string), 1, 0);
        free(json_string);
    }
}