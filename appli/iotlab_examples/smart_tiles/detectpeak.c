#include <stdio.h>
#include <math.h>
#include "detectpeak.h"


void peak_setparam(count_peak_config_t *trace, short int window_size, short int peak_tempo, float threshold)
{
  /* Params verification before set */
  if (window_size < 0)
    trace->window_size  = 0;
  else if (window_size >  WINDOWS_MAX)
    trace->window_size =  WINDOWS_MAX;
  else
    trace->window_size = window_size;

  if (peak_tempo < 0)
    trace->peak_tempo = 0;
  else
    trace->peak_tempo  = peak_tempo;

  trace->threshold   = threshold;
}


void peak_detect(count_peak_config_t *trace, int k, float sig[3], float rpeak[2])
{
  float norm,  moy, dmoy, peak;
  short int sign; 

  /*
    0 - Choose the signal to detect
   */
  /*  norm = sig[2]; */
  norm = sig[0]*sig[0] + sig[1]*sig[1] + sig[2]*sig[2];
  norm = sqrt(norm);
  /* 
   1 - Moving Average 
  */
  if (k==0) {
    trace->sum = norm;
    moy = norm;
    trace->count = 0;
    trace->k = 0;
  } 
  else if (k < trace->window_size) {
    trace->sum = trace->sum + norm;
    moy = trace->sum / (k+1);
  }
  else {
    trace->sum = trace->sum + norm - trace->norm[k%trace->window_size];
    moy = trace->sum / trace->window_size;
  }
  /*
   2 - Search Peak 
  */
  /* Compute derivative signal */
  if (k==0) {
    trace->k = 0;
    dmoy = 0.0;
    if (dmoy >=0 )
      trace->sign = 1;
    else
      trace->sign = -1; 
    peak = 0;
  }
  else { 
    /* Compute derivative signal */
    dmoy = moy - trace->moy;
  }
  /* Compute the sign of the derivative signal */
  if (dmoy >=0 ) 
    sign = 1;
  else
    sign = -1; 
  /* Compute the switch of the sign with a signal threshold */
  if ( (trace->sign == 1) & (sign == -1) & (moy > trace->threshold) 
       & ( k > (trace->k + trace->peak_tempo) ) ) {
    //    printf("TEMPO;%d;%d;%d\n",k, trace->k, trace->peak_tempo);
    peak = moy; 
    trace->count ++;
    trace->k = k;
  }
  else
    peak = 0.0;

  /* return value */
  // printf("MOY %f %f %f\n",norm, trace->sum, moy);
  rpeak[0] = moy;
  rpeak[1] = peak;


 /* Store old values */
  trace->norm[k%trace->window_size] = norm; 
  trace->moy = moy;
  trace->sign = sign;
}


