#include "platform.h"
#include "printf.h"
#include "time.h"

int tests_run = 0;
int got_error = 0;

char str[1024];

#define assert_eq(val1, val2, message) do {\
        if ((val1) != (val2)) {\
                got_error++;\
                printf("ERROR: "); \
                printf("%s, %s == %s: %u != %u\n", (message), #val1, #val2, (val1), (val2)); \
        } \
        } while (0)

static char *test_get_absolute_time(uint64_t timer_tick, uint32_t inc_seconds, char * message)
{
        struct timeval target_time;
        uint32_t timer_seconds = timer_tick / TIMER_FACTOR;
        uint32_t usecs = (((uint64_t) (timer_tick % TIMER_FACTOR)) * 1000000) / TIMER_FACTOR;
        tests_run++;

        get_absolute_time(&target_time, timer_tick, timer_seconds + inc_seconds);
        assert_eq(target_time.tv_sec, timer_seconds, message);
        assert_eq(target_time.tv_usec, usecs, message);
        return NULL;
}

static void all_tests()
{
        uint64_t timer_64;
        uint32_t increment;


        test_get_absolute_time((uint64_t) 0, 0, "get_time_32(0, 0)");
        test_get_absolute_time((uint64_t) 100, 0, "get_time_32(100, 0)");
        test_get_absolute_time((uint64_t) TIMER_FACTOR, 0, "get_time_32(TIMER_FACTOR, 0)");
        test_get_absolute_time((uint64_t) TIMER_FACTOR + 1, 0, "get_time_32(TIMER_FACTOR + 1, 0)");
        test_get_absolute_time((uint64_t) 1 << 31, 0, "get_time_32(1 << 31, 0)");
        test_get_absolute_time((uint64_t) 1 << 32, 0, "get_time_32(1 << 32, 0)");
        test_get_absolute_time(((uint64_t) 1 << 42 ) + 12345, 0, "get_time_32(1 << 42 + 12345, 0)");

        test_get_absolute_time(0, 1, "get_time_32(0, 1)");
        test_get_absolute_time(0, 0xF, "get_time_32(0, 0xF)");

        test_get_absolute_time((uint64_t) 100, 1, "get_time_32(100, 1)");
        test_get_absolute_time((uint64_t) TIMER_FACTOR, 1, "get_time_32(TIMER_FACTOR, 1)");
        test_get_absolute_time((uint64_t) TIMER_FACTOR + 1, 1, "get_time_32(TIMER_FACTOR + 1, 1)");
        test_get_absolute_time((uint64_t) 1 << 31, 1, "get_time_32(1 << 31, 1)");
        test_get_absolute_time((uint64_t) 1 << 32, 1, "get_time_32(1 << 32, 1)");
        test_get_absolute_time(((uint64_t) 1 << 42 ) + 12345, 1, "get_time_32(1 << 42 + 12345, 1)");

        test_get_absolute_time((uint64_t) 100, 5, "get_time_32(100, 5)");
        test_get_absolute_time((uint64_t) TIMER_FACTOR, 5, "get_time_32(TIMER_FACTOR, 5)");
        test_get_absolute_time((uint64_t) TIMER_FACTOR + 1, 5, "get_time_32(TIMER_FACTOR + 1, 5)");
        test_get_absolute_time((uint64_t) 1 << 31, 5, "get_time_32(1 << 31, 5)");
        test_get_absolute_time((uint64_t) 1 << 32, 5, "get_time_32(1 << 32, 5)");
        test_get_absolute_time(((uint64_t) 1 << 42 ) + 12345, 5, "get_time_32(1 << 42 + 12345, 5)");


        /* now u_seconds should be OK, Test more widly seconds */
        for (timer_64 = 0; timer_64 <= ((uint64_t) 1 << 63); timer_64 += TIMER_FACTOR) {

                if (0 == ((timer_64 / TIMER_FACTOR) % 1000))
                        printf("Timer_64 == TIMER_FACTOR * %u\n", (uint32_t) ((uint64_t) timer_64 / TIMER_FACTOR));

                for (increment = 0; increment <= 15; increment++) {
                        snprintf(str, 1024, "get_time(0x%08x%08x, %u)", (uint32_t) (timer_64 >> 32), (uint32_t) (timer_64 & 0xFFFFFFFF), increment);
                        test_get_absolute_time(timer_64, increment, str);
                }
        }

}

static void test_time_task(void *param)
{
        printf("\n\n");
        printf("Starting tests\n");

        all_tests();
        printf("Tests run: %d\n", tests_run);
        printf("Got Errors: %d\n\n", got_error);

        vTaskSuspend( NULL );
}

int main()
{
        platform_init();
        xTaskCreate(test_time_task, (const signed char * const) "test_time",
                    configMINIMAL_STACK_SIZE, NULL, 1, NULL);
        platform_run();
        return 0;
}
