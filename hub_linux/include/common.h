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
#include "errno.h"
#include "signal.h"
#include <sys/time.h>

// ================================================================
// ========================= ENUMS & CONSTANTS ===================
// ================================================================

// Общие константы
#define TTY_SPEED B9600
#define BUF_SIZE  1024

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
extern int g_is_synced;

// Состояние устройств / сенсоров
extern volatile float g_temperature_celsius;
extern volatile float g_humidity_percent;
extern volatile int   g_is_door_open;
extern volatile int   g_is_alarm_enabled;
extern volatile int   g_is_motion_detected;
extern volatile int   g_is_light_on;

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

void request_full_state_from_esp32(struct mosquitto *mosq);

void request_full_state_from_arduino(int fd);

void send_full_state_to_esp32(struct mosquitto *mosq);

void send_full_state_to_arduino(int fd);

void process_arduino_json(const char* json_string);

void setup_periodic_sync_timer(int arduino_fd);

ssize_t writen(int fd, const void *vptr, size_t n);

int init_json_commands(void);

#endif // SMARTHUB_COMMON_H
