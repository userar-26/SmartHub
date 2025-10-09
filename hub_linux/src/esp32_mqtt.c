#include "common.h"

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
    // Если пришли верные данные, то обрабатываем их
    if(strcmp(msg->topic, TOPIC_ESP32_TO_HUB) == 0 && msg->payloadlen == 4) 
    {
        int16_t *data = (int16_t *)msg->payload;
        pthread_mutex_lock(&g_state_mutex);
        g_temperature_celsius = data[0]/10.0;
        g_humidity_percent    = data[1]/10.0;
        pthread_mutex_unlock(&g_state_mutex);

        // Уведомляем главный поток, о том, что данные обновились
        char c = '1';
        write(*(int *)obj, &c, 1);
    }
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