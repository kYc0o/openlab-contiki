#ifndef _DETECTPEAK_H
#define _DETECTPEAK_H

#define WINDOWS_MAX 100

typedef	struct count_peak_config {
  /* Parameters for peak_detect */
  short int window_size;
  short int peak_tempo;
  float threshold;
  /* Values stored during peak_detect */
  float norm[WINDOWS_MAX];
  float moy;
  float sum;
  short int sign;
  short int k;
  short int count;
} count_peak_config_t;

extern void peak_detect(count_peak_config_t *trace, int k, float sig[3], float peak[2]);

extern void peak_setparam(count_peak_config_t *trace, short int window_size, short int peak_tempo, float threshold);

#endif
