#include "../include/common.h"

void setup(void)
{
    
    Serial.begin(9600);

    pinMode(TRIGGER_PIN,OUTPUT);
    pinMode(ECHO_PIN,INPUT);

    ALARM_STATUS = ON;
    DOOR_STATUS  = CLOSE;

}

// --- Настройки таймера ---
const long SENSOR_READ_INTERVAL = 500; // Опрашивать датчик каждые 500 мс
unsigned long lastSensorReadTime = 0;

void loop(void)
{

    // --- Блок 1: Обработка входящих команд (выполняется на каждой итерации) ---
    if(Serial.available() > 0)
    {
        int command = Serial.read();

        if(command == DOOR_OPEN){
            DOOR_STATUS = OPEN;
            Serial.write((uint8_t)DOOR_OPEN);
        }
        else if(command == DOOR_CLOSE){
            DOOR_STATUS = CLOSE;
            Serial.write((uint8_t)DOOR_CLOSE);
        }
        else if(command == ALARM_ON){
            ALARM_STATUS = ON;
            Serial.write((uint8_t)ALARM_ON);
        }
        else if(command == ALARM_OFF){
            ALARM_STATUS = OFF;
            Serial.write((uint8_t)ALARM_OFF);
        }

    }

    // --- Блок 2: Опрос датчика по таймеру (выполняется только раз в 500 мс) ---
    if (ALARM_STATUS == ON) 
    {
        unsigned long currentTime = millis();
        if (currentTime - lastSensorReadTime >= SENSOR_READ_INTERVAL) {
            
            lastSensorReadTime = currentTime; // Сбрасываем таймер

            int distance = current_distance_cm(TRIGGER_PIN, ECHO_PIN);
            if (distance > 0 && distance <= 30)
                Serial.write((uint8_t)MOTION_DETECTED);
        
        }
    }

}