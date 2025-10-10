#include "common.h"

static char *s_json_resp_door_is_open   = NULL;
static char *s_json_resp_door_is_close  = NULL;
static char *s_json_resp_alarm_is_on    = NULL;
static char *s_json_resp_alarm_is_off   = NULL;
static char *s_json_resp_motion_cleared = NULL;

static char *s_json_event_motion_detected = NULL;
static char *s_json_resp_command_error    = NULL;

void send_full_state_response(void)
{
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) return;

    cJSON_AddStringToObject(root, KEY_TYPE, TYPE_FULL_STATE);

    cJSON *state_obj = cJSON_CreateObject();
    cJSON_AddItemToObject(root, KEY_STATE, state_obj);

    cJSON_AddBoolToObject(state_obj, KEY_STATE_IS_DOOR_OPEN, g_is_door_open);
    cJSON_AddBoolToObject(state_obj, KEY_STATE_IS_ALARM_ENABLED, g_is_alarm_enabled);

    char *json_string = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (json_string != NULL) {
        Serial.println(json_string);
        free(json_string);
    }
}

void setup_json_responses(void)
{
    cJSON *root = NULL;

    // --- Ответ: Состояние двери - "открыто" ---
    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, KEY_TYPE, TYPE_RESPONSE);
    cJSON_AddStringToObject(root, KEY_DEVICE, DEVICE_DOOR);
    cJSON_AddStringToObject(root, KEY_STATE, STATE_OPEN);
    s_json_resp_door_is_open = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    // --- Ответ: Состояние двери - "закрыто" ---
    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, KEY_TYPE, TYPE_RESPONSE);
    cJSON_AddStringToObject(root, KEY_DEVICE, DEVICE_DOOR);
    cJSON_AddStringToObject(root, KEY_STATE, STATE_CLOSE);
    s_json_resp_door_is_close = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    // --- Ответ: Состояние сигнализации - "включена" ---
    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, KEY_TYPE, TYPE_RESPONSE);
    cJSON_AddStringToObject(root, KEY_DEVICE, DEVICE_ALARM);
    cJSON_AddStringToObject(root, KEY_STATE, STATE_ON);
    s_json_resp_alarm_is_on = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    // --- Ответ: Состояние сигнализации - "выключена" ---
    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, KEY_TYPE, TYPE_RESPONSE);
    cJSON_AddStringToObject(root, KEY_DEVICE, DEVICE_ALARM);
    cJSON_AddStringToObject(root, KEY_STATE, STATE_OFF);
    s_json_resp_alarm_is_off = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    // --- Ответ: Состояние тревоги по движению - "сброшено" ---
    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, KEY_TYPE, TYPE_RESPONSE);
    cJSON_AddStringToObject(root, KEY_DEVICE, DEVICE_MOTION_SENSOR);
    cJSON_AddStringToObject(root, KEY_STATE, STATE_CLEAR);
    s_json_resp_motion_cleared = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    // --- Событие: Обнаружено движение ---
    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, KEY_TYPE, TYPE_EVENT);
    cJSON_AddStringToObject(root, KEY_DEVICE, DEVICE_MOTION_SENSOR);
    cJSON_AddStringToObject(root, KEY_EVENT, EVENT_DETECTED);
    s_json_event_motion_detected = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    // --- Ответ: Ошибка команды ---
    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, KEY_TYPE, TYPE_RESPONSE);
    cJSON_AddStringToObject(root, KEY_STATUS, STATUS_ERROR);
    cJSON_AddStringToObject(root, KEY_REASON, REASON_UNKNOWN_COMMAND);
    s_json_resp_command_error = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
}

void process_json_command(char* command_str)
{
    cJSON *root = cJSON_Parse(command_str);
    if (root == NULL) {
        Serial.println(s_json_resp_command_error);
        return;
    }

    cJSON *command_item = cJSON_GetObjectItemCaseSensitive(root, KEY_COMMAND);
    if (!cJSON_IsString(command_item)) {
        Serial.println(s_json_resp_command_error);
        goto cleanup;
    }

    // --- Логика обработки команд ---

    // 1. Обрабатываем команду "set_state"
    if (strcmp(command_item->valuestring, CMD_SET_STATE) == 0)
    {
        cJSON *device_item = cJSON_GetObjectItemCaseSensitive(root, KEY_DEVICE);
        cJSON *state_item = cJSON_GetObjectItemCaseSensitive(root, KEY_STATE);

        if (!cJSON_IsString(device_item) || !cJSON_IsString(state_item)) {
            Serial.println(s_json_resp_command_error);
            goto cleanup;
        }

        if (strcmp(device_item->valuestring, DEVICE_DOOR) == 0) {
            if (strcmp(state_item->valuestring, STATE_OPEN) == 0) {
                g_is_door_open = 1;
                Serial.println(s_json_resp_door_is_open);
            } else if (strcmp(state_item->valuestring, STATE_CLOSE) == 0) {
                g_is_door_open = 0;
                Serial.println(s_json_resp_door_is_close);
            } else {
                Serial.println(s_json_resp_command_error);
            }
        }
        else if (strcmp(device_item->valuestring, DEVICE_ALARM) == 0) {
            if (strcmp(state_item->valuestring, STATE_ON) == 0) {
                g_is_alarm_enabled = 1;
                Serial.println(s_json_resp_alarm_is_on);
            } else if (strcmp(state_item->valuestring, STATE_OFF) == 0) {
                g_is_alarm_enabled = 0;
                Serial.println(s_json_resp_alarm_is_off);
            } else {
                Serial.println(s_json_resp_command_error);
            }
        }
        else if (strcmp(device_item->valuestring, DEVICE_MOTION_SENSOR) == 0) {
             if (strcmp(state_item->valuestring, STATE_CLEAR) == 0) {
                g_is_motion_detected = 0;
                Serial.println(s_json_resp_motion_cleared);
             }
        }
        else {
            Serial.println(s_json_resp_command_error);
        }
    }
    // 2. Обрабатываем команду "get_state"
    else if (strcmp(command_item->valuestring, CMD_GET_STATE) == 0)
    {
        send_full_state_response();
    }
    // 3. Если ни одна команда не подошла
    else
    {
        Serial.println(s_json_resp_command_error);
    }

cleanup:
    cJSON_Delete(root);
}

void send_motion_detected_event(void)
{
    Serial.println(s_json_event_motion_detected);
}