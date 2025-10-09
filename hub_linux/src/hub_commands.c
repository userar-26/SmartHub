#include "common.h"

// Команды для arduino в формате json
static char *s_json_cmd_arduino_set_door_open  = NULL;
static char *s_json_cmd_arduino_set_door_close = NULL;
static char *s_json_cmd_arduino_set_alarm_on   = NULL;
static char *s_json_cmd_arduino_set_alarm_off  = NULL;
static char *s_json_cmd_arduino_clear_motion   = NULL;

// Команды для esp32 в формате json
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

    return 0;
    
    cleanup:
        cJSON_Delete(root);
        HUB_LOG("JSON","Не удалось выполнить инициализацию json команд\n");
        return -1;
}

void sendDeviceCommand(char com, int arduino_fd, struct mosquitto *mosq) 
{
    switch (com) {
        case 'q':
            exit(0);
            break;

        case '1':  // Дверь
            if(g_is_door_open)
                write(arduino_fd,s_json_cmd_arduino_set_door_close,strlen(s_json_cmd_arduino_set_door_close));
            else
                write(arduino_fd,s_json_cmd_arduino_set_door_open,strlen(s_json_cmd_arduino_set_door_open));
            break;

        case '2':  // Сигнализация
            if(g_is_alarm_enabled)
                write(arduino_fd,s_json_cmd_arduino_set_alarm_off,strlen(s_json_cmd_arduino_set_alarm_off));
            else
                write(arduino_fd,s_json_cmd_arduino_set_alarm_on,strlen(s_json_cmd_arduino_set_alarm_on));
            break;

        case '3':  // Увеличить температуру
            if (mosq)
                mosquitto_publish(mosq, NULL, TOPIC_HUB_TO_ESP32, strlen(s_json_cmd_esp32_increase_temp), s_json_cmd_esp32_increase_temp, 2, 0);
            break;

        case '4':  // Уменьшить температуру
            if (mosq)
                mosquitto_publish(mosq, NULL, TOPIC_HUB_TO_ESP32, strlen(s_json_cmd_esp32_decrease_temp), s_json_cmd_esp32_decrease_temp, 2, 0);
            break;

        case '5':  // Сброс уведомления о движении
            if(g_is_motion_detected)
                write(arduino_fd, s_json_cmd_arduino_clear_motion, strlen(s_json_cmd_arduino_clear_motion));
            break;

        default:
            HUB_LOG("SendCommand","Попытка отправить неизвестную команду\n");
            break;
    }
}