#include "common.h"

void sendDeviceCommand(char com, int arduino_fd, struct mosquitto *mosq) 
{
    char c;

    switch (com) {
        case 'q':
            exit(0);
            break;

        case '1':  // Дверь
            c = g_is_door_open ? CLOSE_DOOR : OPEN_DOOR;
            write(arduino_fd, &c, 1);
            break;

        case '2':  // Сигнализация
            c = g_is_alarm_enabled ? OFF_ALARM : ON_ALARM;
            write(arduino_fd, &c, 1);
            break;

        case '3':  // Увеличить температуру
            c = INC_TEMPERATURE;
            if (mosq)
                mosquitto_publish(mosq, NULL, TOPIC_HUB_TO_ESP32, 1, &c, 0, 0);
            break;

        case '4':  // Уменьшить температуру
            c = DEC_TEMPERATURE;
            if (mosq)
                mosquitto_publish(mosq, NULL, TOPIC_HUB_TO_ESP32, 1, &c, 0, 0);
            break;

        case '5':  // Сброс уведомления о движении
            pthread_mutex_lock(&g_state_mutex);
            g_is_motion_detected = 0;
            pthread_mutex_unlock(&g_state_mutex);
            break;

        default:
            break;
    }
}