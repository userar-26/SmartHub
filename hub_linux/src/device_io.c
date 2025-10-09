#include "common.h"

void* arduinoThread(void* arg) {

    // Подключаемся к arduino
    int arduino_fd = *(int *)arg;

    
    // Буфер для сигнала от arduino
    unsigned char command_from_arduino; 

    for(;;)
    {
        ssize_t rbytes = read(arduino_fd,&command_from_arduino,1);

        pthread_mutex_lock(&g_state_mutex);
        // Обработка полученного сигнала
        if(command_from_arduino == OPEN_DOOR)
            g_is_door_open = 1;
        else if(command_from_arduino == CLOSE_DOOR)
            g_is_door_open = 0;
        else if(command_from_arduino == ON_ALARM)
            g_is_alarm_enabled = 1;
        else if(command_from_arduino == OFF_ALARM)
            g_is_alarm_enabled = 0;
        else if(command_from_arduino == MOTION_DETECTED)
            g_is_motion_detected = 1;
        pthread_mutex_unlock(&g_state_mutex);
        write(g_update_arduino[1], "U", 1);
    }

}

void mosq_connect_handler(struct mosquitto *mosq, void *obj, int rc)
{
    if(rc == 0)
    {
        HUB_LOG("MQTT","Успешно подключились к брокеру\n");

        // --- Подписываемся на топик от ESP32---
        int sub_rc = mosquitto_subscribe(mosq, NULL, TOPIC_ESP32_TO_HUB, 2);
        if(sub_rc != MOSQ_ERR_SUCCESS){
            HUB_LOG("MQTT", "Ошибка подписки на топик %s\n", TOPIC_ESP32_TO_HUB);
        }
    }
    else if(rc == 3){
        mosquitto_connect_async(mosq,MOSQ_NAME,MOSQ_PORT,MOSQ_TIME);
        HUB_LOG("MQTT","Ошибка: Сервер брокера недоступен, попытка переподключится\n");
    }
    else{
        err_quit("Критическая ошибка: Не удалось установить соединение с брокером\n");
    }
}

void mosq_disconnect_handler(struct mosquitto *mosq, void *obj, int rc)
{
    // Если отключились, не из-за вызова специальной функции отключения,
    // то пытаемся переподключится
    if(rc != MOSQ_ERR_SUCCESS) {
        mosquitto_reconnect_async(mosq);
    }
}

void mosq_message_hanler(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
    // Если пришли верные данные, то обрабатываем их
    if(strcmp(msg->topic, TOPIC_ESP32_TO_HUB) == 0 && msg->payloadlen == 4) 
    {
        int16_t *data = (int16_t *)msg->payload;
        pthread_mutex_lock(&g_state_mutex);
        g_temperature_celsius = data[0]/10.0;
        g_humidity_percent    = data[1]/10.0;
        pthread_mutex_unlock(&g_state_mutex);

        // Уведомляем главный поток, о том, что данные обновились
        char c = '1';
        write(*(int *)obj, &c, 1);
    }
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

    // read() будет ждать, пока не придет хотя бы 1 байт
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    if(tcsetattr(fd,TCSANOW,&tty) != 0){
        close(fd);
        return -1;
    }
    return fd;
}

struct mosquitto *esp32_mqtt_init(int *fd) 
{
    // Инициализация библиотеки Mosquitto
    if(mosquitto_lib_init() != MOSQ_ERR_SUCCESS)
        return NULL;
    
    // Создание нового клиента Mosquitto
    struct mosquitto *mosq = mosquitto_new("hub_linux", 0, fd);
    if(mosq == NULL){
        mosquitto_lib_cleanup();
        return NULL;
    }

    // Настройка callback-функций
    mosquitto_connect_callback_set(mosq, mosq_connect_handler);
    mosquitto_disconnect_callback_set(mosq, mosq_disconnect_handler);
    mosquitto_message_callback_set(mosq, mosq_message_hanler);

    // Асинхронное подключение к брокеру
    mosquitto_connect_async(mosq, MOSQ_NAME, MOSQ_PORT, MOSQ_TIME);

    // Запуск потока обработки сообщений
    mosquitto_loop_start(mosq);

    return mosq;
}

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

