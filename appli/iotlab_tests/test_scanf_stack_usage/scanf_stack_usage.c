#include <string.h>
#include "platform.h"
#include "printf.h"
#include "scanf.h"
#include "soft_timer_delay.h"
#include "event.h"

#include "task.h"


static unsigned int stack;
void print_queue_stack(void * arg)
{
    (void)arg;
    stack = uxTaskGetStackHighWaterMark( NULL );
    printf("Free stack event queue %u\n", stack);
}

static void test_task(void *param)
{
    static unsigned int stack1, stack2, stack3;

    static int a;
    static char str[128];
    static float pi;

    stack = uxTaskGetStackHighWaterMark( NULL );

    sscanf("", "");
    stack1 = uxTaskGetStackHighWaterMark( NULL );
    sscanf("1", "%d", &a);
    stack2 = uxTaskGetStackHighWaterMark( NULL );
    sscanf("1 string 3.14", "%d", &a, str, &pi);
    stack3 = uxTaskGetStackHighWaterMark( NULL );


    printf("\n");
    printf("Base stack %u\n", stack);
    printf("Post empty scanf   %u diff %u\n", stack1, stack - stack1);
    printf("Post simple scanf  %u diff %u\n", stack2, stack - stack2);
    printf("Post complex scanf %u diff %u\n", stack3, stack - stack3);
    printf("\n");

    event_post(EVENT_QUEUE_APPLI, print_queue_stack, NULL);

    while (1)
        asm("wfi");
}

int main()
{
    platform_init();
    xTaskCreate(test_task, (const signed char *const) "test_task",
            2 * configMINIMAL_STACK_SIZE, NULL, 1, NULL);
            //configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    platform_run();
    return 0;
}
