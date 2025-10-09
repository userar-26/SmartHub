#ifndef SMARTHUB_PROTOCOL_H
#define SMARTHUB_PROTOCOL_H

// ================================================================
// ==================== MQTT ТОПИКИ (только для ESP32) ============
// ================================================================
#define TOPIC_ESP32_TO_HUB "smarthub/esp32/data"
#define TOPIC_HUB_TO_ESP32 "smarthub/esp32/command"


// ================================================================
// ==================== КЛЮЧИ JSON (Общие для всех) ===============
// ================================================================
// Используются во всех сообщениях
#define KEY_DEVICE      "device"
#define KEY_COMMAND     "command"
#define KEY_STATE       "state"
#define KEY_VALUES      "values"

// Идентификаторы устройств
#define DEVICE_DOOR             "door"
#define DEVICE_ALARM            "alarm"
#define DEVICE_MOTION_SENSOR    "motion_sensor"
#define DEVICE_DHT_SENSOR       "dht_sensor"
#define DEVICE_LIVING_ROOM_LIGHT "living_room_light"

// Команды
#define CMD_SET_STATE       "set_state"
#define CMD_GET_DATA        "get_data"
#define CMD_TEMP_INCREASE   "increase_temp"
#define CMD_TEMP_DECREASE   "decrease_temp"

// Состояния
#define STATE_OPEN      "open"
#define STATE_CLOSE     "close"
#define STATE_ON        "on"
#define STATE_OFF       "off"
#define STATE_DETECTED  "detected"
#define STATE_CLEAR     "clear"

#endif //SMARTHUB_PROTOCOL_H