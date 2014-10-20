#include <platform.h>
#include <string.h>
#include <printf.h>

static void cmd_help(char*);
static void cmd_helo(char*);
static void cmd_echo(char*);

typedef struct {
	char * command;
	void (*func)(char *args);
	char * help;
} command_t;

static command_t commands[] = {
	{ "help", cmd_help, "this help" },
	{ "helo", cmd_helo, "says helo" },
	{ "echo", cmd_echo, "echoes args" },
};

#define NB_COMMANDS(cmd) ((int)(sizeof(cmd)/sizeof(command_t)))

static void cmd_help(char *ignored)
{
	printf("available commands:\n");
	int i;
	for (i=0; i < NB_COMMANDS(commands); i++) {
		printf("  %10s: %s\n", commands[i].command, commands[i].help);
	}
}

static void cmd_helo(char *ignored)
{
	printf("HELO!\n");
}

static void cmd_echo(char *args)
{
	printf("echo:");
	char delim[] = " ";
	char *token;
	while ((token = strtok(NULL, delim)))
		printf(" %s", token);
	printf("\n");
}

static void interpret_line(char *line)
{
	char delim[] = " ";
	char *command = strtok(line, delim);
	int i;
	for (i=0; i < NB_COMMANDS(commands); i++) {
		if (! strcmp(command, commands[i].command)) {
			commands[i].func(line);
			return;
		}
	}
	printf("unknown command: %s\n", command);
}

static xQueueHandle char_queue;
static void char_rx(handler_arg_t arg, uint8_t c)
{
	xQueueSendFromISR(char_queue, &c, 0);
}

static unsigned int buff_index = 0;
static char buff[1024];
static void flush_buff()
{
	if (buff_index == sizeof(buff)) return; // please gcc

	buff[buff_index] = '\0';
	interpret_line(buff);
	buff_index = 0;
}

static void read_line()
{
	char value;
	while (xQueueReceive(char_queue, &value, 0) == pdTRUE ) {
		switch (value) {
		case '\r':
			break; // ignore and handle only \n
		case '\n':
			flush_buff();
			break;
		default:
			if (buff_index < sizeof(buff))
				buff[buff_index++] = value;
			else
				flush_buff();
		}
	}
}

static void app_task(void *param)
{
	printf("HELO serial server started.  Try: 'help'.\n");
	while (1) read_line();
}

int main()
{
	platform_init();
	char_queue = xQueueCreate(8, sizeof(char));
	uart_set_rx_handler(uart_print, char_rx, NULL);
	xTaskCreate(app_task, (const signed char * const) "app",
			configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	platform_run();
	return 0;
}
