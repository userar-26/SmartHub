#include "common.h"

// Команды для arduino в формате json
static char *s_json_cmd_arduino_set_door_open  = NULL;
static char *s_json_cmd_arduino_set_door_close = NULL;
static char *s_json_cmd_arduino_set_alarm_on   = NULL;
static char *s_json_cmd_arduino_set_alarm_off  = NULL;
static char *s_json_cmd_arduino_clear_motion   = NULL;

// Команды для esp32 в формате json
static char *s_json_cmd_esp32_set_light_on     = NULL;
static char *s_json_cmd_esp32_set_light_off    = NULL;
static char *s_json_cmd_esp32_increase_temp    = NULL;
static char *s_json_cmd_esp32_decrease_temp    = NULL;

int init_json_commands(void)
{
    // --- 1. Создаем команду открыть дверь ---
    cJSON *root = NULL;
    root = cJSON_CreateObject();
    if(root == NULL)
        goto cleanup;
    
    if(cJSON_AddStringToObject(root,KEY_DEVICE,DEVICE_DOOR) == NULL)
        goto cleanup;
    if(cJSON_AddStringToObject(root,KEY_COMMAND,CMD_SET_STATE) == NULL)
        goto cleanup;
    if(cJSON_AddStringToObject(root,KEY_STATE,STATE_OPEN) == NULL)
        goto cleanup;
    
    s_json_cmd_arduino_set_door_open = cJSON_PrintUnformatted(root);
    if(s_json_cmd_arduino_set_door_open == NULL)
        goto cleanup;
    
    cJSON_Delete(root);

    // --- 2. Создаем команду закрыть дверь ---
    root = cJSON_CreateObject();
    if(root == NULL)
        goto cleanup;
    
    if(cJSON_AddStringToObject(root,KEY_DEVICE,DEVICE_DOOR) == NULL)
        goto cleanup;
    if(cJSON_AddStringToObject(root,KEY_COMMAND,CMD_SET_STATE) == NULL)
        goto cleanup;
    if(cJSON_AddStringToObject(root,KEY_STATE,STATE_CLOSE) == NULL)
        goto cleanup;
    
    s_json_cmd_arduino_set_door_close = cJSON_PrintUnformatted(root);
    if(s_json_cmd_arduino_set_door_close == NULL)
        goto cleanup;
    
    cJSON_Delete(root);

    // --- 3. Создаем команду включить систему сигнализации ---
    root = cJSON_CreateObject();
    if(root == NULL)
        goto cleanup;
    
    if(cJSON_AddStringToObject(root,KEY_DEVICE,DEVICE_ALARM) == NULL)
        goto cleanup;
    if(cJSON_AddStringToObject(root,KEY_COMMAND,CMD_SET_STATE) == NULL)
        goto cleanup;
    if(cJSON_AddStringToObject(root,KEY_STATE,STATE_ON) == NULL)
        goto cleanup;
    
    s_json_cmd_arduino_set_alarm_on = cJSON_PrintUnformatted(root);
    if(s_json_cmd_arduino_set_alarm_on == NULL)
        goto cleanup;
    
    cJSON_Delete(root);

    // --- 4. Создаем команду отключить систему сигнализации ---
    root = cJSON_CreateObject();
    if(root == NULL)
        goto cleanup;
    
    if(cJSON_AddStringToObject(root,KEY_DEVICE,DEVICE_ALARM) == NULL)
        goto cleanup;
    if(cJSON_AddStringToObject(root,KEY_COMMAND,CMD_SET_STATE) == NULL)
        goto cleanup;
    if(cJSON_AddStringToObject(root,KEY_STATE,STATE_OFF) == NULL)
        goto cleanup;
    
    s_json_cmd_arduino_set_alarm_off = cJSON_PrintUnformatted(root);
    if(s_json_cmd_arduino_set_alarm_off == NULL)
        goto cleanup;
    
    cJSON_Delete(root);

    // --- 5. Создаем команду отключить оповещение о движение ---
    root = cJSON_CreateObject();
    if(root == NULL)
        goto cleanup;
    
    if(cJSON_AddStringToObject(root,KEY_DEVICE,DEVICE_MOTION_SENSOR) == NULL)
        goto cleanup;
    if(cJSON_AddStringToObject(root,KEY_COMMAND,CMD_SET_STATE) == NULL)
        goto cleanup;
    if(cJSON_AddStringToObject(root,KEY_STATE,STATE_CLEAR) == NULL)
        goto cleanup;
    
    s_json_cmd_arduino_clear_motion = cJSON_PrintUnformatted(root);
    if(s_json_cmd_arduino_clear_motion == NULL)
        goto cleanup;

    // --- 6. Создаем команду увеличить темературу на 1 градус ---
    root = cJSON_CreateObject();
    if(root == NULL)
        goto cleanup;
    
    if(cJSON_AddStringToObject(root,KEY_DEVICE,DEVICE_DHT_SENSOR) == NULL)
        goto cleanup;
    if(cJSON_AddStringToObject(root,KEY_COMMAND,CMD_TEMP_INCREASE) == NULL)
        goto cleanup;
    
    s_json_cmd_esp32_increase_temp = cJSON_PrintUnformatted(root);
    if(s_json_cmd_esp32_increase_temp == NULL)
        goto cleanup;
    
    cJSON_Delete(root);

    // --- 7. Создаем команду уменьшить темературу на 1 градус ---
    root = cJSON_CreateObject();
    if(root == NULL)
        goto cleanup;
    
    if(cJSON_AddStringToObject(root,KEY_DEVICE,DEVICE_DHT_SENSOR) == NULL)
        goto cleanup;
    if(cJSON_AddStringToObject(root,KEY_COMMAND,CMD_TEMP_DECREASE) == NULL)
        goto cleanup;
    
    s_json_cmd_esp32_decrease_temp = cJSON_PrintUnformatted(root);
    if(s_json_cmd_esp32_decrease_temp == NULL)
        goto cleanup;
    
    cJSON_Delete(root);

    // --- 8. Создаем команду включить свет ---
    root = cJSON_CreateObject();
    if(root == NULL)
        goto cleanup;
    
    if(cJSON_AddStringToObject(root,KEY_DEVICE,DEVICE_LIVING_ROOM_LIGHT) == NULL)
        goto cleanup;
    if(cJSON_AddStringToObject(root,KEY_COMMAND, CMD_SET_STATE) == NULL)
        goto cleanup;
    if(cJSON_AddStringToObject(root,KEY_STATE, STATE_ON) == NULL)
        goto cleanup;
    
    s_json_cmd_esp32_set_light_on = cJSON_PrintUnformatted(root);
    if(s_json_cmd_esp32_set_light_on == NULL)
        goto cleanup;
    
    cJSON_Delete(root);

    // --- 9. Создаем команду отключить свет ---
    root = cJSON_CreateObject();
    if(root == NULL)
        goto cleanup;
    
    if(cJSON_AddStringToObject(root,KEY_DEVICE,DEVICE_LIVING_ROOM_LIGHT) == NULL)
        goto cleanup;
    if(cJSON_AddStringToObject(root,KEY_COMMAND, CMD_SET_STATE) == NULL)
        goto cleanup;
    if(cJSON_AddStringToObject(root,KEY_STATE, STATE_OFF) == NULL)
        goto cleanup;
    
    s_json_cmd_esp32_set_light_off = cJSON_PrintUnformatted(root);
    if(s_json_cmd_esp32_set_light_off == NULL)
        goto cleanup;
    
    cJSON_Delete(root);

    return 0;
    
    cleanup:
        cJSON_Delete(root);
        HUB_LOG("JSON","Не удалось выполнить инициализацию json команд\n");
        return -1;
}

void request_full_state_from_esp32(struct mosquitto *mosq)
{
    cJSON *root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, KEY_COMMAND, CMD_GET_STATE);

    char *msg = cJSON_PrintUnformatted(root);

    cJSON_Delete(root);

    mosquitto_publish(mosq, NULL, TOPIC_HUB_TO_ESP32, strlen(msg), msg, 2, 0);

    free(msg);
}

void request_full_state_from_arduino(int fd)
{
    cJSON *root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, KEY_COMMAND, CMD_GET_STATE);

    char *msg = cJSON_PrintUnformatted(root);

    cJSON_Delete(root);

    writen(fd, msg, strlen(msg));
    writen(fd, "\n", 1);

    free(msg);
}

void send_full_state_to_esp32(struct mosquitto *mosq)
{
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) return;

    cJSON_AddStringToObject(root, KEY_TYPE, TYPE_FULL_STATE);

    cJSON *state_obj = cJSON_CreateObject();
    cJSON_AddItemToObject(root, KEY_STATE, state_obj);

    pthread_mutex_lock(&g_state_mutex);
    float temp_to_send = g_temperature_celsius;
    bool light_state_to_send = g_is_light_on;
    pthread_mutex_unlock(&g_state_mutex);

    cJSON_AddNumberToObject(state_obj, KEY_VALUE_TEMPERATURE, temp_to_send);
    cJSON_AddBoolToObject(state_obj, KEY_STATE_IS_LIGHT_ON, light_state_to_send);
    
    char *msg = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (msg != NULL && mosq != NULL) {
        mosquitto_publish(mosq, NULL, TOPIC_HUB_TO_ESP32, strlen(msg), msg, 1, 0);
        free(msg);
        HUB_LOG("SYNC", "Отправлено полное состояние на ESP32 для синхронизации.\n");
    }
}

void send_full_state_to_arduino(int fd)
{
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        HUB_LOG("SYNC", "Ошибка создания JSON для отправки состояния на Arduino\n");
        return;
    }

    cJSON_AddStringToObject(root, KEY_TYPE, TYPE_FULL_STATE);

    cJSON *state_obj = cJSON_CreateObject();
    cJSON_AddItemToObject(root, KEY_STATE, state_obj);

    pthread_mutex_lock(&g_state_mutex);
    bool door_state = g_is_door_open;
    bool alarm_state = g_is_alarm_enabled;
    pthread_mutex_unlock(&g_state_mutex);

    cJSON_AddBoolToObject(state_obj, KEY_STATE_IS_DOOR_OPEN, door_state);
    cJSON_AddBoolToObject(state_obj, KEY_STATE_IS_ALARM_ENABLED, alarm_state);
    
    char *msg = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (msg != NULL) {
        writen(fd, msg, strlen(msg));
        writen(fd, "\n", 1);
        free(msg);
        HUB_LOG("SYNC", "Отправлено полное состояние на Arduino для синхронизации.\n");
    }
}

void sendDeviceCommand(char com, int arduino_fd, struct mosquitto *mosq) 
{
    switch (com) {
        case 'q':
            exit(0);
            break;

        case '1':  // Дверь
            if(g_is_door_open) {
                writen(arduino_fd,s_json_cmd_arduino_set_door_close,strlen(s_json_cmd_arduino_set_door_close));
                writen(arduino_fd,"\n",1);
            }
            else {
                writen(arduino_fd,s_json_cmd_arduino_set_door_open,strlen(s_json_cmd_arduino_set_door_open));
                writen(arduino_fd,"\n",1);
            }
            break;

        case '2':  // Сигнализация
            if(g_is_alarm_enabled) {
                writen(arduino_fd,s_json_cmd_arduino_set_alarm_off,strlen(s_json_cmd_arduino_set_alarm_off));
                writen(arduino_fd,"\n",1);
            }
            else {
                writen(arduino_fd,s_json_cmd_arduino_set_alarm_on,strlen(s_json_cmd_arduino_set_alarm_on));
                writen(arduino_fd,"\n",1);
            }
            break;

        case '3':  // Свет
            if(g_is_light_on)
                mosquitto_publish(mosq, NULL, TOPIC_HUB_TO_ESP32, strlen(s_json_cmd_esp32_set_light_off), s_json_cmd_esp32_set_light_off, 2, 0);
            else
                mosquitto_publish(mosq, NULL, TOPIC_HUB_TO_ESP32, strlen(s_json_cmd_esp32_set_light_on), s_json_cmd_esp32_set_light_on, 2, 0);
            break;

        case '4':  // Увеличить температуру
            if (mosq)
                mosquitto_publish(mosq, NULL, TOPIC_HUB_TO_ESP32, strlen(s_json_cmd_esp32_increase_temp), s_json_cmd_esp32_increase_temp, 2, 0);
            break;

        case '5':  // Уменьшить температуру
            if (mosq)
                mosquitto_publish(mosq, NULL, TOPIC_HUB_TO_ESP32, strlen(s_json_cmd_esp32_decrease_temp), s_json_cmd_esp32_decrease_temp, 2, 0);
            break;

        case '6':  // Сброс уведомления о движении
            if(g_is_motion_detected) {
                writen(arduino_fd, s_json_cmd_arduino_clear_motion, strlen(s_json_cmd_arduino_clear_motion));
                writen(arduino_fd,"\n",1);
            }
            break;

        default:
            HUB_LOG("SendCommand","Попытка отправить неизвестную команду\n");
            break;
    }
}