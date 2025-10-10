#ifndef SMARTHUB_PROTOCOL_H
#define SMARTHUB_PROTOCOL_H

// ================================================================
// ==================== MQTT ТОПИКИ ===============================
// ================================================================
#define TOPIC_ESP32_TO_HUB "smarthub/esp32/data"
#define TOPIC_HUB_TO_ESP32 "smarthub/esp32/command"


// ================================================================
// ==================== КЛЮЧИ JSON ================================
// ================================================================
#define KEY_TYPE        "type"
#define KEY_DEVICE      "device"
#define KEY_COMMAND     "command"
#define KEY_STATE       "state"
#define KEY_VALUES      "values"
#define KEY_REASON      "reason"
#define KEY_EVENT       "event"
#define KEY_STATUS      "status"

// --- Ключи для ответа с полным состоянием ---
#define KEY_STATE_IS_DOOR_OPEN      "is_door_open"
#define KEY_STATE_IS_ALARM_ENABLED  "is_alarm_enabled"
#define KEY_STATE_IS_LIGHT_ON       "is_light_on"

// --- Ключи для данных сенсоров ---
#define KEY_VALUE_TEMPERATURE "temperature"
#define KEY_VALUE_HUMIDITY    "humidity"


// ================================================================
// ==================== ЗНАЧЕНИЯ ==================================
// ================================================================

// --- Типы сообщений ---
#define TYPE_RESPONSE   "response" // Ответ на команду
#define TYPE_EVENT      "event"    // Асинхронное событие
#define TYPE_FULL_STATE "full_state" // Ответ на запрос полного состояния

// --- Статус для сообщений об ошибках ---
#define STATUS_ERROR    "error"

// --- Причины ошибок ---
#define REASON_UNKNOWN_COMMAND "unknown_command"

// --- Идентификаторы устройств ---
#define DEVICE_DOOR             "door"
#define DEVICE_ALARM            "alarm"
#define DEVICE_MOTION_SENSOR    "motion_sensor"
#define DEVICE_DHT_SENSOR       "dht_sensor"
#define DEVICE_LIVING_ROOM_LIGHT "living_room_light"

// --- Команды от Хаба к Устройствам ---
#define CMD_SET_STATE       "set_state"
#define CMD_GET_STATE       "get_state"
#define CMD_TEMP_INCREASE   "increase_temp"
#define CMD_TEMP_DECREASE   "decrease_temp"

// --- Состояния и События ---
#define STATE_OPEN      "open"
#define STATE_CLOSE     "close"
#define STATE_ON        "on"
#define STATE_OFF       "off"
#define STATE_CLEAR     "clear"
#define EVENT_DETECTED  "detected"

#endif //SMARTHUB_PROTOCOL_H