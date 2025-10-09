#ifndef SMARTHUB_PROTOCOL_H
#define SMARTHUB_PROTOCOL_H

// ================================================================
// ==================== MQTT TOPICS ===============================
// ================================================================
#define TOPIC_ESP32_TO_HUB "smarthub/esp32/data"
#define TOPIC_HUB_TO_ESP32 "smarthub/esp32/command"


// ================================================================
// ==================== JSON KEYS ================================
// ================================================================
// Используются во всех сообщениях
#define KEY_TYPE        "type"
#define KEY_DEVICE      "device"
#define KEY_COMMAND     "command"
#define KEY_STATE       "state"
#define KEY_VALUES      "values"

// --- Ключи для ответов ---
#define KEY_STATUS      "status"
#define KEY_NEW_STATE   "new_state"
#define KEY_REASON      "reason"

// --- Ключи для событий ---
#define KEY_EVENT       "event"

// ================================================================
// ==================== VALUES ==================================
// ================================================================

// --- Типы сообщений ---
#define TYPE_RESPONSE   "response"
#define TYPE_EVENT      "event"
#define TYPE_FULL_STATE "full_state"

// --- Статусы ответа ---
#define STATUS_SUCCESS  "success"
#define STATUS_ERROR    "error"

// --- Причины ошибок ---
#define REASON_UNKNOWN_COMMAND "unknown_command"

// --- Ключи для ответа с полным состоянием ---
#define KEY_STATE_IS_DOOR_OPEN      "is_door_open"
#define KEY_STATE_IS_ALARM_ENABLED  "is_alarm_enabled"

// --- Идентификаторы устройств ---
#define DEVICE_DOOR             "door"
#define DEVICE_ALARM            "alarm"
#define DEVICE_MOTION_SENSOR    "motion_sensor"
#define DEVICE_DHT_SENSOR       "dht_sensor"
#define DEVICE_LIVING_ROOM_LIGHT "living_room_light"

// --- Команды ---
#define CMD_SET_STATE       "set_state"
#define CMD_GET_DATA        "get_data"
#define CMD_GET_STATE       "get_state"
#define CMD_TEMP_INCREASE   "increase_temp"
#define CMD_TEMP_DECREASE   "decrease_temp"

// --- Состояния ---
#define STATE_OPEN      "open"
#define STATE_CLOSE     "close"
#define STATE_ON        "on"
#define STATE_OFF       "off"
#define STATE_CLEAR     "clear"
#define EVENT_DETECTED  "detected"

// --- Ответы-подтверждения ---
#define RESP_MOTION_CLEARED "motion_cleared"

#endif //SMARTHUB_PROTOCOL_H