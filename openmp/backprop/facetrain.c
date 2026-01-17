#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "backprop.h"
#include "omp.h"

int layer_size = 0;

static void backprop_face(void) {
  BPNN *net;
  float out_err, hid_err;

  net = bpnn_create(layer_size, 16, 1); // (16, 1 can not be changed)
  printf("Input layer size : %d\n", layer_size);
  load(net);
  // entering the training kernel, only one iteration
  printf("Starting training kernel\n");
  bpnn_train_kernel(net, &out_err, &hid_err);
  printf("Output error: %.8f\n", out_err);
  printf("Hidden error: %.8f\n", hid_err);
  bpnn_free(net);
  printf("Training done\n");
}

int setup(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: backprop <num of input elements>\n");
    return 1;
  }

  layer_size = atoi(argv[1]);

  int seed = 7;
  bpnn_initialize(seed);
  backprop_face();

  return 0;
}
