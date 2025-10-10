#include "common.h"

int current_distance_cm(uint8_t trigger_pin,uint8_t echo_pin)
{
    int distance,duration;

    digitalWrite(trigger_pin,LOW);
    delayMicroseconds(2);
    digitalWrite(trigger_pin,HIGH);
    delayMicroseconds(10);
    digitalWrite(trigger_pin,LOW);

    duration = pulseIn(ECHO_PIN,HIGH);

    distance = duration *0.0343 / 2;

    return distance;
}