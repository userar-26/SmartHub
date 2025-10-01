#include <Arduino.h>

#define TRIGGER_PIN     9
#define ECHO_PIN        10

enum{
    ON,
    OFF
}ALARM_STATUS;

enum{
    OPEN,
    CLOSE
}DOOR_STATUS;

enum{
    DOOR_OPEN,
    DOOR_CLOSE,
    ALARM_ON,
    ALARM_OFF,
    MOTION_DETECTED,
};

int current_distance_cm(uint8_t trigger_pin,uint8_t echo_pin);