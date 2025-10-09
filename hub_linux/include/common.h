#ifndef SMARTHUB_COMMON_H
#define SMARTHUB_COMMON_H

// ================================================================
// ========================= INCLUDES ============================
// ================================================================
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/select.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include <mosquitto.h>
#include "cJSON.h"
#include "smarthub_protocol.h"

// ================================================================
// ========================= ENUMS & CONSTANTS ===================
// ================================================================

// Команды для устройств
enum {
    OPEN_DOOR,
    CLOSE_DOOR,
    ON_ALARM,
    OFF_ALARM,
    MOTION_DETECTED,
    INC_TEMPERATURE = 'u',
    DEC_TEMPERATURE = 'd'
};

// Общие константы
#define TTY_SPEED B9600
#define BUF_SIZE  1024
#define ON  1
#define OFF 0

// Параметры подключения к устройствам
#define DEV_ARDUINO "/dev/ttyArduino"
#define MOSQ_PORT 1883
#define MOSQ_NAME "localhost"
#define MOSQ_TIME 100

// ================================================================
// ========================= GLOBAL VARIABLES ====================
// ================================================================
extern pthread_mutex_t g_state_mutex;
extern int g_update_arduino[2];
extern int g_update_esp32[2];

// Состояние устройств / сенсоров
extern volatile float g_temperature_celsius;
extern volatile float g_humidity_percent;
extern volatile int   g_is_door_open;
extern volatile int   g_is_alarm_enabled;
extern volatile int   g_is_motion_detected;

// ================================================================
// ========================= FUNCTION PROTOTYPES =================
// ================================================================

// ------------------------------------------------
// Завершение программы с выводом ошибки
// ------------------------------------------------
void err_quit(const char* format, ...);

// ------------------------------------------------
// Отображение меню и статуса системы
// ------------------------------------------------
void displayMenu();

// ------------------------------------------------
// Поток обработки сообщений от Arduino
// ------------------------------------------------
void* arduinoThread(void* arg);

// ------------------------------------------------
// Инициализация MQTT для ESP32 и запуск loop
// ------------------------------------------------
struct mosquitto *esp32_mqtt_init(int *fd);

// ------------------------------------------------
// Отправка команды на устройство (Arduino или ESP32)
// ------------------------------------------------
void sendDeviceCommand(char com, int arduino_fd, struct mosquitto *mosq);

// ------------------------------------------------
// Открытие и конфигурация последовательного порта Arduino
// ------------------------------------------------
int open_and_configure_arduino_port();

// ------------------------------------------------
// Функция логирования с тегом, не завершает программу.
// ------------------------------------------------
void HUB_LOG(const char* tag, const char *format, ...);

#endif // SMARTHUB_COMMON_H
