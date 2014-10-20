#include "printf.h"

#include "platform.h"
#include "stm32f1xx.h"
#include "debug.h"

#include "omlc.h"

static void test_main(void *param);

int main()
{
  platform_init();

  xTaskCreate(test_main, (const signed char * const) "oml",
          configMINIMAL_STACK_SIZE, NULL, 1, NULL);

  platform_run();
  return 0;
}

static void test_main(void *param)
{
  int8_t result = omlc_init ("Simple");
  if (result == -1) {
    printf ("Could not initialise OML\n");
    while(1);
  } else if (result == 1) {
    printf ("OML was disabled by the user, exiting\n");
    while(1);
  }
  
  OmlMPDef mp_def [] = {
    { "count", OML_UINT32_VALUE },
    { "count_str", OML_STRING_VALUE },
    { "count_real", OML_DOUBLE_VALUE },
    { NULL, (OmlValueT)0 }
  };
  
  OmlMP *mp = omlc_add_mp ("counter", mp_def);
  
  if (mp == NULL) {
    printf ("Error: could not register Measurement Point 'counter'");
    while(1);
  }
  
  omlc_start();
  
  uint32_t i = 0;
  for (i = 0; i < 100; i++) {
    uint32_t count = i;
    char count_str[16];
    double count_real = (double)i;
    OmlValueU values[3];
    
    omlc_zero_array(values, 3);
    
    snprintf(count_str, sizeof(count_str), "%d", i);
    
    omlc_set_uint32 (values[0], count);
    omlc_set_string (values[1], count_str);
    omlc_set_double (values[2], count_real);
    
    omlc_inject (mp, values);
    
    omlc_reset_string(values[1]);
  }
  omlc_close();
  while(1);
}
