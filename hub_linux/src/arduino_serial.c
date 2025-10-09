#include "common.h"

void* arduinoThread(void* arg) 
{

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