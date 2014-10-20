#ifndef CN_CONSUMPTION_MEASURES_H
#define CN_CONSUMPTION_MEASURES_H


/** Start the consumption monitoring */
void cn_consumption_start();

/** Flush current measure packet. Should be used when updating timer */
void flush_current_consumption_measures();

#endif // CN_CONSUMPTION_MEASURES_H
