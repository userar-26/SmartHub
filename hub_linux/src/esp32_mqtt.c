#include "common.h"

char *TAG = "MQTT";

void mosq_connect_handler(struct mosquitto *mosq, void *obj, int rc)
{
    if(rc == 0)
    {
        HUB_LOG("MQTT","Успешно подключились к брокеру\n");

        // --- Подписываемся на топик от ESP32---
        int sub_rc = mosquitto_subscribe(mosq, NULL, TOPIC_ESP32_TO_HUB, 2);
        if(sub_rc != MOSQ_ERR_SUCCESS){
            HUB_LOG("MQTT", "Ошибка подписки на топик %s\n", TOPIC_ESP32_TO_HUB);
        }
    }
    else if(rc == 3){
        mosquitto_connect_async(mosq,MOSQ_NAME,MOSQ_PORT,MOSQ_TIME);
        HUB_LOG("MQTT","Ошибка: Сервер брокера недоступен, попытка переподключится\n");
    }
    else{
        err_quit("Критическая ошибка: Не удалось установить соединение с брокером\n");
    }
}

void mosq_disconnect_handler(struct mosquitto *mosq, void *obj, int rc)
{
    // Если отключились, не из-за вызова специальной функции отключения,
    // то пытаемся переподключится
    if(rc != MOSQ_ERR_SUCCESS) {
        mosquitto_reconnect_async(mosq);
    }
}

void mosq_message_hanler(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
    // Безопасно копируем payload в строку для парсинга
    char resp_str[msg->payloadlen + 1];
    memcpy(resp_str, msg->payload, msg->payloadlen);
    resp_str[msg->payloadlen] = '\0';

    cJSON *root = cJSON_Parse(resp_str);
    if (root == NULL){
        HUB_LOG("MQTT", "Ошибка парсинга JSON от ESP32: %s\n", resp_str);
        return;
    }

    // --- Логика диспетчеризации ---

    // Сначала проверяем, не является ли это запросом на синхронизацию от ESP32
    cJSON *command_item = cJSON_GetObjectItemCaseSensitive(root, KEY_COMMAND);
    if (cJSON_IsString(command_item) && strcmp(command_item->valuestring, CMD_GET_STATE) == 0 && g_is_synced) 
    {
        // ESP32 перезагрузился и запрашивает актуальное состояние. Отправляем ему.
        send_full_state_to_esp32(mosq);
        goto cleanup;
    }

    // Если это не команда, ищем ключ "type" для определения типа сообщения
    cJSON *type_item = cJSON_GetObjectItemCaseSensitive(root, KEY_TYPE);
    if (!cJSON_IsString(type_item)) {
        HUB_LOG("MQTT", "Получен JSON без ключа 'type' или 'command' от ESP32.\n");
        goto cleanup;
    }

    // --- Обработка сообщения TYPE_FULL_STATE ---
    if (strcmp(type_item->valuestring, TYPE_FULL_STATE) == 0) 
    {
        // Это ответ от ESP32 на наш запрос состояния (например, при старте хаба)
        cJSON *state_obj = cJSON_GetObjectItemCaseSensitive(root, KEY_STATE);
        if (cJSON_IsObject(state_obj)) {
            cJSON *light_item = cJSON_GetObjectItemCaseSensitive(state_obj, KEY_STATE_IS_LIGHT_ON);
            cJSON *temp_item = cJSON_GetObjectItemCaseSensitive(state_obj, KEY_VALUE_TEMPERATURE);
            cJSON *hum_item = cJSON_GetObjectItemCaseSensitive(state_obj, KEY_VALUE_HUMIDITY);

            if (cJSON_IsBool(light_item) && cJSON_IsNumber(temp_item) && cJSON_IsNumber(hum_item)) {
                pthread_mutex_lock(&g_state_mutex);
                g_is_light_on = cJSON_IsTrue(light_item);
                g_temperature_celsius = temp_item->valuedouble;
                g_humidity_percent = hum_item->valuedouble;
                pthread_mutex_unlock(&g_state_mutex);
                
                // Устанавливаем флаг, что хаб успешно синхронизировался с ESP32
                g_is_synced = 1;
                write(*(int *)obj, "U", 1); // Уведомляем главный поток об обновлении
            }
        }
    }
    // --- Обработка сообщения TYPE_RESPONSE ---
    else if (strcmp(type_item->valuestring, TYPE_RESPONSE) == 0) 
    {
        cJSON *status_item = cJSON_GetObjectItemCaseSensitive(root, KEY_STATUS);
        // Проверяем, не является ли это сообщением об ошибке
        if (cJSON_IsString(status_item) && strcmp(status_item->valuestring, STATUS_ERROR) == 0) 
        {
            cJSON *reas_item = cJSON_GetObjectItemCaseSensitive(root, KEY_REASON);
            if (cJSON_IsString(reas_item) && strcmp(reas_item->valuestring, REASON_UNKNOWN_COMMAND) == 0) {
                HUB_LOG("MQTT", "ESP32 сообщил об ошибке: неизвестная команда.\n");
            } else {
                HUB_LOG("MQTT", "ESP32 сообщил о неизвестной ошибке.\n");
            }
        }
        else // Если это не ошибка, значит это успешный ответ
        {
            cJSON *device_item = cJSON_GetObjectItemCaseSensitive(root, KEY_DEVICE);
            if (cJSON_IsString(device_item) && strcmp(device_item->valuestring, DEVICE_LIVING_ROOM_LIGHT) == 0)
            {
                cJSON *light_item = cJSON_GetObjectItemCaseSensitive(root, KEY_STATE_IS_LIGHT_ON);
                if(cJSON_IsBool(light_item)) {
                    pthread_mutex_lock(&g_state_mutex);
                    g_is_light_on = cJSON_IsTrue(light_item);
                    pthread_mutex_unlock(&g_state_mutex);
                    write(*(int *)obj, "U", 1); // Уведомляем главный поток об обновлении
                }
            }
        }
    }
    // --- Обработка сообщения TYPE_EVENT ---
    else if (strcmp(type_item->valuestring, TYPE_EVENT) == 0) 
    {
        cJSON *device_item = cJSON_GetObjectItemCaseSensitive(root, KEY_DEVICE);
        if (cJSON_IsString(device_item) && strcmp(device_item->valuestring, DEVICE_DHT_SENSOR) == 0) 
        {
            // Это периодическое обновление данных с датчика DHT11
            cJSON *values_obj = cJSON_GetObjectItemCaseSensitive(root, KEY_VALUES);
            if (cJSON_IsObject(values_obj)) {
                cJSON *temp_item = cJSON_GetObjectItemCaseSensitive(values_obj, KEY_VALUE_TEMPERATURE);
                cJSON *hum_item = cJSON_GetObjectItemCaseSensitive(values_obj, KEY_VALUE_HUMIDITY);

                if (cJSON_IsNumber(temp_item) && cJSON_IsNumber(hum_item)) {
                    pthread_mutex_lock(&g_state_mutex);
                    g_temperature_celsius = temp_item->valuedouble;
                    g_humidity_percent = hum_item->valuedouble;
                    pthread_mutex_unlock(&g_state_mutex);
                    write(*(int *)obj, "U", 1); // Уведомляем главный поток об обновлении
                } 
            }
        } 
    }

cleanup:
    cJSON_Delete(root);
}

struct mosquitto *esp32_mqtt_init(int *fd) 
{
    // Инициализация библиотеки Mosquitto
    if(mosquitto_lib_init() != MOSQ_ERR_SUCCESS)
        return NULL;
    
    // Создание нового клиента Mosquitto
    struct mosquitto *mosq = mosquitto_new("hub_linux", 0, fd);
    if(mosq == NULL){
        mosquitto_lib_cleanup();
        return NULL;
    }

    // Настройка callback-функций
    mosquitto_connect_callback_set(mosq, mosq_connect_handler);
    mosquitto_disconnect_callback_set(mosq, mosq_disconnect_handler);
    mosquitto_message_callback_set(mosq, mosq_message_hanler);

    // Асинхронное подключение к брокеру
    mosquitto_connect_async(mosq, MOSQ_NAME, MOSQ_PORT, MOSQ_TIME);

    // Запуск потока обработки сообщений
    mosquitto_loop_start(mosq);

    return mosq;
}