#include "common.h"

volatile int g_is_door_open;
volatile int g_is_alarm_enabled;
volatile int g_is_motion_detected;

void setup(void)
{
    
    Serial.begin(9600);

    pinMode(TRIGGER_PIN,OUTPUT);
    pinMode(ECHO_PIN,INPUT);

    setup_json_responses();

    g_is_door_open        = 0;
    g_is_alarm_enabled    = 1;
    g_is_motion_detected  = 0;

}

// --- Настройки таймера ---
const long SENSOR_READ_INTERVAL = 500;  // Как часто проверять датчик
const long MOTION_SEND_INTERVAL = 1000; // Как часто отправлять уведомление, если есть тревога
unsigned long lastSensorReadTime = 0;
unsigned long lastMotionSendTime = 0;

void loop(void)
{
    // --- Блок 1: Обработка входящих команд (выполняется на каждой итерации) ---
    check_serial_for_command();

    unsigned long currentTime = millis();

    // --- Блок 2: Логика обнаружения и отправки тревоги ---

    if (g_is_alarm_enabled)
    {
        if (!g_is_motion_detected) {
            if (currentTime - lastSensorReadTime >= SENSOR_READ_INTERVAL) {
                lastSensorReadTime = currentTime; // Сбрасываем таймер проверки

                int distance = current_distance_cm(TRIGGER_PIN, ECHO_PIN);
                if (distance > 0 && distance <= 30)
                {
                    g_is_motion_detected = true;
                }
            }
        }

        if (g_is_motion_detected) {
            if (currentTime - lastMotionSendTime >= MOTION_SEND_INTERVAL) {
                lastMotionSendTime = currentTime; // Сбрасываем таймер отправки
                send_motion_detected_event();
            }
        }
    }
    else
        g_is_motion_detected = false;
}