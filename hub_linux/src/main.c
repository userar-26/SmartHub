#include "common.h"

int g_is_synced = 0;

int main(void)
{
    pthread_t arduino_tid, esp32_tid;
    fd_set m_set;
    char input_buffer[BUF_SIZE];
    int maxfd;

    // Инициализируем строки json командами
    init_json_commands();

    if (pipe(g_update_arduino) == -1)
        err_quit("Не удалось создать pipe для Arduino\n");
    if (pipe(g_update_esp32) == -1)
        err_quit("Не удалось создать pipe для ESP32\n");

    int arduino_fd = open_and_configure_arduino_port();
    if (arduino_fd < 0)
        err_quit("Не удалось установить нормальное поделючение с Arduino\n");

    struct mosquitto * mosq = esp32_mqtt_init(&g_update_esp32[1]);
    if(mosq == NULL)
        err_quit("Не удалось установить нормальное поделючение с ESP32\n");
    
    // Создаем поток для приема сообщений от Arduino
    if(pthread_create(&arduino_tid, NULL,arduinoThread, (void *)&arduino_fd) != 0)
        err_quit("Не удалось создать поток для arduino");

    // Запуск периодической синхронизации с arduino
    setup_periodic_sync_timer(arduino_fd);

    // Запрашиваем текущее состояние у плат
    request_full_state_from_arduino(arduino_fd);
    request_full_state_from_esp32(mosq);

    for(;;)
    {
        displayMenu(); // Отображаем меню и текущий статус

        // Настраиваем pselect, для ожидания данных с клавиатуры,esp32,arduino
        FD_ZERO(&m_set);
        FD_SET(STDIN_FILENO, &m_set);
        FD_SET(g_update_arduino[0], &m_set);
        FD_SET(g_update_esp32[0], &m_set);

        maxfd = STDIN_FILENO;
        if (g_update_arduino[0] > maxfd) maxfd = g_update_arduino[0];
        if (g_update_esp32[0] > maxfd) maxfd = g_update_esp32[0];

        int result = pselect(maxfd+1,&m_set, NULL, NULL, NULL, NULL);

        if(result > 0)
        {
            if (FD_ISSET(STDIN_FILENO, &m_set)) {
                if (read(STDIN_FILENO, input_buffer, BUF_SIZE) == 2) {
                    char command = input_buffer[0];
                    sendDeviceCommand(command, arduino_fd, mosq);
                }
            }

            if(FD_ISSET(g_update_arduino[0],&m_set)){
                char temp_buf[1];
                read(g_update_arduino[0], temp_buf, 1);
            }

            if(FD_ISSET(g_update_esp32[0],&m_set)){
                char temp_buf[1];
                read(g_update_esp32[0], temp_buf, 1);
            }
        }
    }
}

