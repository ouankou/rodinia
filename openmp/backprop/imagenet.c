
#include <stdio.h>
#include <stdlib.h>
#include "backprop.h"

extern int layer_size;

void load(BPNN *net) {
  float *units = net->input_units;
  int nr = layer_size;

  for (int i = 0, k = 1; i < nr; i++, k++) {
    units[k] = (float)rand() / (float)RAND_MAX;
  }
}
