#include "common.h"

// Формирует и отправляет JSON вручную, используя F-строки для экономии ОЗУ
void send_json(const char* type, const char* p1_key, const char* p1_val, const char* p2_key, const char* p2_val) {
    Serial.print(F("{\""));
    Serial.print(type);
    Serial.print(F("\":\""));
    Serial.print(p1_key);
    Serial.print(F("\",\""));
    Serial.print(p1_val);
    Serial.print(F("\":\""));
    Serial.print(p2_key);
    Serial.print(F("\",\""));
    Serial.print(p2_val);
    Serial.println(F("\"}"));
}

void send_full_state_response(void) {
    Serial.print(F("{\"type\":\"full_state\",\"state\":{\"is_door_open\":"));
    Serial.print(g_is_door_open ? "true" : "false");
    Serial.print(F(",\"is_alarm_enabled\":"));
    Serial.print(g_is_alarm_enabled ? "true" : "false");
    Serial.println(F("}}"));
}

void send_simple_response(const char* device, const char* state) {
    Serial.print(F("{\"type\":\"response\",\"device\":\""));
    Serial.print(device);
    Serial.print(F("\",\"state\":\""));
    Serial.print(state);
    Serial.println(F("\"}"));
}

void send_command_error_response() {
    Serial.println(F("{\"type\":\"response\",\"status\":\"error\",\"reason\":\"unknown_command\"}"));
}

void send_motion_detected_event(void) {
    Serial.println(F("{\"type\":\"event\",\"device\":\"motion_sensor\",\"event\":\"detected\"}"));
}

// Упрощенная обработка без cJSON
void process_json_command(char* command_str) {
    if (strstr(command_str, CMD_GET_STATE)) {
        send_full_state_response();
    } else if (strstr(command_str, DEVICE_DOOR) && strstr(command_str, STATE_OPEN)) {
        g_is_door_open = 1;
        send_simple_response(DEVICE_DOOR, STATE_OPEN);
    } else if (strstr(command_str, DEVICE_DOOR) && strstr(command_str, STATE_CLOSE)) {
        g_is_door_open = 0;
        send_simple_response(DEVICE_DOOR, STATE_CLOSE);
    } else if (strstr(command_str, DEVICE_ALARM) && strstr(command_str, STATE_ON)) {
        g_is_alarm_enabled = 1;
        send_simple_response(DEVICE_ALARM, STATE_ON);
    } else if (strstr(command_str, DEVICE_ALARM) && strstr(command_str, STATE_OFF)) {
        g_is_alarm_enabled = 0;
        send_simple_response(DEVICE_ALARM, STATE_OFF);
    } else if (strstr(command_str, DEVICE_MOTION_SENSOR) && strstr(command_str, STATE_CLEAR)) {
        g_is_motion_detected = 0;
        send_simple_response(DEVICE_MOTION_SENSOR, STATE_CLEAR);
    } else {
        send_command_error_response();
    }
}