#include "common.h"

// Инициализация глобальных параметров
pthread_mutex_t g_state_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int g_is_door_open          = 0;
volatile int g_is_alarm_enabled      = 0;
volatile float g_humidity_percent    = 0;
volatile float g_temperature_celsius = 0;
volatile int   g_is_motion_detected  = 0;
volatile int   g_is_light_on         = 0;
int g_update_arduino[2];
int g_update_esp32[2];

void err_quit(const char* format, ...) 
{
    char buf[BUF_SIZE];  // буфер для форматированного сообщения
    int len, err_fd;
    va_list args;

    va_start(args, format);
    len = vsnprintf(buf, BUF_SIZE, format, args);
    va_end(args);

    int bytes_to_write = len > BUF_SIZE ? BUF_SIZE - 1 : len;
    err_fd = open("err_log.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);

    if(err_fd < 0) {
        write(STDERR_FILENO, buf, bytes_to_write);
    } else {
        write(err_fd, buf, bytes_to_write);
        close(err_fd);
    }

    exit(1);
}

ssize_t writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;
			else
				return(-1);
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}

void HUB_LOG(const char* tag, const char *format, ...)
{
    char buf[BUF_SIZE];
    int len, log_fd;
    va_list args;

    // Формируем строку с тегом
    int tag_len = snprintf(buf, BUF_SIZE, "[%s] ", tag); // помещаем тег в buf
    if(tag_len < 0 || tag_len >= BUF_SIZE) {
        tag_len = BUF_SIZE - 1;
    }

    // Добавляем форматируемую часть
    va_start(args, format);
    int msg_len = vsnprintf(buf + tag_len, BUF_SIZE - tag_len, format, args);
    va_end(args);

    if(msg_len < 0) {
        // Ошибка форматирования
        return;
    }

    int total_len = tag_len + (msg_len > (BUF_SIZE - tag_len - 1) ? BUF_SIZE - tag_len - 1 : msg_len);

    // Открываем лог-файл
    log_fd = open("hub_log.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if(log_fd < 0) {
        // Если файл не открылся, пишем на STDOUT
        write(STDOUT_FILENO, buf, total_len);
        write(STDOUT_FILENO, "\n", 1);
    } else {
        write(log_fd, buf, total_len);
        write(log_fd, "\n", 1); // добавляем перенос строки
        close(log_fd);
    }
}

void displayMenu() 
{
    // --- Шаг 1: Безопасно читаем все данные ---
    pthread_mutex_lock(&g_state_mutex);
    float temp = g_temperature_celsius;
    float humidity = g_humidity_percent;
    bool is_door_open = g_is_door_open;
    bool is_alarm_active = g_is_alarm_enabled;
    bool is_light_on = g_is_light_on;
    pthread_mutex_unlock(&g_state_mutex);

    // --- Шаг 2: Рисуем интерфейс ---
    system("clear");

    // --- Заголовок ---
    printf("==================== SmartHub v1.0 ====================\n\n");

    // --- Блок статуса ---
    printf("--- СТАТУС СИСТЕМЫ ---\n");
    printf("Дверь:\t\t%s\n", is_door_open ? "Открыта" : "Закрыта");
    printf("Сигнализация:\t%s\n", is_alarm_active ? "Включена" : "Отключена");
    printf("Свет:\t%s\n", is_light_on ? "Включен" : "Отключен");
    printf("Температура:\t%.1f C\n", temp);
    printf("Влажность:\t%.1f %%\n", humidity);
    printf("Движение: \t%s\n",g_is_motion_detected ? "Обнаружено": "Не обнаружено");

    // --- Блок меню ---
    printf("\n--- МЕНЮ УПРАВЛЕНИЯ ---\n");
    printf("1) Открыть/Закрыть дверь\n");
    printf("2) Включить/Отключить сигнализацию\n");
    printf("3) Включить/Отключить свет\n");
    printf("4) Увеличить температуру на 1 градус\n");
    printf("5) Уменьшить температуру на 1 градус\n");
    printf("6) Сбросить уведомление о движение\n");
    printf("q) Выход\n");

    // --- Приглашение к вводу ---
    printf("\nВведите команду > ");

    // Принудительно выводим все из буфера на экран
    fflush(stdout);
}