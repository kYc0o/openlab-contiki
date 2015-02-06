#ifndef COMPUTING_H
#define COMPUTING_H
#include <stdint.h>
#include <math.h>
#include "config.h"

extern uint32_t compute_number;
struct values {
    double v[NUM_VALUES];
};

struct received_values {
    uint32_t valid;
    uint32_t num_neighbours;
    struct values values;
};


extern struct values my_values;

extern struct received_values neighbours_values[MAX_NUM_NEIGHBOURS];


double init_value();
double compute_value_from_neighbours(double my_value, uint32_t n_neighbours,
        struct received_values *neighbors_values, uint8_t value_num);

double compute_value_from_gossip(double my_value, double neigh_value);
uint32_t compute_final_value();



// General functions managing the multiples values
void compute_all_values_from_gossip(struct received_values *neigh_values);


#endif//COMPUTING_H
