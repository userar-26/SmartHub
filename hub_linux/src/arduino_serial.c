#include "common.h"

static void timer_handler(int signum)
{
    if (g_sync_pipe[1] != -1) {
        char byte = 'S';
        write(g_sync_pipe[1], &byte, 1);
    }
}

void setup_periodic_sync_timer(void)
{
    struct sigaction sa;
    struct itimerval timer;

    // 1. Настраиваем обработчик для сигнала SIGALRM
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &timer_handler;
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        err_quit("Не удалось установить обработчик сигнала для таймера.");
    }

    // 2. Настраиваем интервал таймера (здесь - 30 секунд)
    timer.it_value.tv_sec = 30;  // Первый запуск через 30 секунд
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 30; // Повторять каждые 30 секунд
    timer.it_interval.tv_usec = 0;
    
    // 3. Запускаем таймер
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        err_quit("Не удалось запустить таймер.\n");
    }

    HUB_LOG("SYNC", "Таймер для периодической синхронизации с Arduino запущен.");
}

void* arduinoThread(void* arg) 
{
    int arduino_fd = *(int*)arg;
    char buffer[BUF_SIZE];
    int pos = 0;
    char read_char;

    HUB_LOG("Arduino", "Поток для Arduino запущен.");

    for(;;)
    {
        ssize_t bytes_read = read(arduino_fd, &read_char, 1);

        if (bytes_read > 0) {
            if (read_char == '\n') {
                if (pos > 0) {
                    buffer[pos] = '\0';
                    process_arduino_json(buffer);
                }
                pos = 0; // Сбрасываем позицию для новой строки
            } else if (read_char != '\r') { // Игнорируем символ возврата каретки
                if (pos < BUF_SIZE - 1) {
                    buffer[pos++] = read_char;
                } else {
                    // Буфер переполнен, сбрасываем.
                    HUB_LOG("Arduino", "Предупреждение: буфер Serial переполнен, данные утеряны.");
                    pos = 0;
                }
            }
        }
    }
    return NULL;
}

void process_arduino_json(const char* json_string)
{
    cJSON *root = cJSON_Parse(json_string);
    if (root == NULL) {
        HUB_LOG("Arduino", "Ошибка парсинга JSON от Arduino: %s", json_string);
        return;
    }

    cJSON *type_item = cJSON_GetObjectItemCaseSensitive(root, KEY_TYPE);
    if (!cJSON_IsString(type_item)) {
        cJSON_Delete(root);
        return;
    }

    pthread_mutex_lock(&g_state_mutex);

    // --- Обработка сообщения TYPE_FULL_STATE ---
    if (strcmp(type_item->valuestring, TYPE_FULL_STATE) == 0) {
        cJSON *state_obj = cJSON_GetObjectItemCaseSensitive(root, KEY_STATE);
        if (cJSON_IsObject(state_obj)) {
            cJSON *door_item = cJSON_GetObjectItemCaseSensitive(state_obj, KEY_STATE_IS_DOOR_OPEN);
            cJSON *alarm_item = cJSON_GetObjectItemCaseSensitive(state_obj, KEY_STATE_IS_ALARM_ENABLED);

            if (cJSON_IsBool(door_item)) g_is_door_open = cJSON_IsTrue(door_item);
            if (cJSON_IsBool(alarm_item)) g_is_alarm_enabled = cJSON_IsTrue(alarm_item);
        }
    }
    // --- Обработка сообщения TYPE_EVENT ---
    else if (strcmp(type_item->valuestring, TYPE_EVENT) == 0) {
        cJSON *device_item = cJSON_GetObjectItemCaseSensitive(root, KEY_DEVICE);
        if (cJSON_IsString(device_item) && strcmp(device_item->valuestring, DEVICE_MOTION_SENSOR) == 0) {
            g_is_motion_detected = 1;
        }
    }
    // --- Обработка сообщения TYPE_RESPONSE ---
    else if (strcmp(type_item->valuestring, TYPE_RESPONSE) == 0) {
        cJSON *device_item = cJSON_GetObjectItemCaseSensitive(root, KEY_DEVICE);
        cJSON *state_item = cJSON_GetObjectItemCaseSensitive(root, KEY_STATE);

        if (cJSON_IsString(device_item) && cJSON_IsString(state_item)) {
            if (strcmp(device_item->valuestring, DEVICE_DOOR) == 0) {
                g_is_door_open = (strcmp(state_item->valuestring, STATE_OPEN) == 0);
            } else if (strcmp(device_item->valuestring, DEVICE_ALARM) == 0) {
                g_is_alarm_enabled = (strcmp(state_item->valuestring, STATE_ON) == 0);
            } else if (strcmp(device_item->valuestring, DEVICE_MOTION_SENSOR) == 0) {
                if (strcmp(state_item->valuestring, STATE_CLEAR) == 0) {
                    g_is_motion_detected = 0;
                }
            }
        }
    }

    pthread_mutex_unlock(&g_state_mutex);

    // Уведомляем главный поток, что данные обновились и нужно перерисовать меню
    write(g_update_arduino[1], "U", 1);

    cJSON_Delete(root);
}

int open_and_configure_arduino_port() 
{

    // Открываем порт для работы с Arduino
    int fd = open(DEV_ARDUINO, O_RDWR | O_NOCTTY);
    if(fd < 0)
        return -1;

    if(!isatty(fd)){
        close(fd);
        return -1;
    }

    struct termios tty;

    if(tcgetattr(fd,&tty) != 0){
        close(fd);
        return -1;
    }

    cfsetispeed(&tty,TTY_SPEED);
    cfsetospeed(&tty,TTY_SPEED);

    tty.c_cflag &= ~(PARENB);
    tty.c_cflag &= ~(CSTOPB);
    tty.c_cflag &= ~(CSIZE);
    tty.c_cflag |= (CS8);
    tty.c_cflag |= CREAD;
    tty.c_cflag |= (CLOCAL | CREAD);

    tty.c_lflag &= ~(ICANON | ECHO | ISIG | ECHOE);

    tty.c_oflag &= ~(OPOST);

    if(tcsetattr(fd,TCSANOW,&tty) != 0){
        close(fd);
        return -1;
    }
    return fd;
}