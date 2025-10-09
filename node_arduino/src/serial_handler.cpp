#include "common.h"

static char g_serial_buffer[SERIAL_BUFFER_SIZE];
static int g_buffer_pos = 0;

void check_serial_for_command(void)
{
    while (Serial.available() > 0)
    {
        char incoming_char = Serial.read();

        if (incoming_char == '\n')
        {
            g_serial_buffer[g_buffer_pos] = '\0';
            if (g_buffer_pos > 0)
            {
                process_json_command(g_serial_buffer);
            }
            g_buffer_pos = 0;
            break;
        }
        else if (incoming_char == '\r')
        {
            continue;
        }
        else
        {
            if (g_buffer_pos < SERIAL_BUFFER_SIZE - 1)
            {
                g_serial_buffer[g_buffer_pos] = incoming_char;
                g_buffer_pos++;
            }
            else
            {
                // Буфер переполнен, сбрасываем для приема новой команды
                g_buffer_pos = 0;
            }
        }
    }
}