/*
 * =====================================================================================
 *
 *       Filename:  suite.c
 *
 *    Description:  The main wrapper for the suite
 *
 *        Version:  1.0
 *        Created:  10/22/2009 08:40:34 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Liang Wang (lw2aw), lw2aw@virginia.edu
 *        Company:  CS@UVa
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <assert.h>

#include "common.h"

static int do_verify = 0;

static struct option long_options[] = {
      /* name, has_arg, flag, val */
      {"input", 1, NULL, 'i'},
      {"threads", 1, NULL, 'n'},
      {"size", 1, NULL, 's'},
      {"verify", 0, NULL, 'v'},
      {0,0,0,0}
};

extern void
lud_openmp_gpu(float *m, int matrix_dim);

int
main ( int argc, char *argv[] )
{
  int matrix_dim = 32; /* default size */
  int opt, option_index=0;
  func_ret_t ret;
  const char *input_file = NULL;
  float *m, *mm;
  stopwatch sw;

  while ((opt = getopt_long(argc, argv, "::vs:n:i:",
                            long_options, &option_index)) != -1 ) {
      switch(opt){
        case 'i':
          input_file = optarg;
          break;
        case 'v':
          do_verify = 1;
          break;
        case 'n':
          /* Accepted for runner compatibility with openmp/lud_omp. */
          break;
        case 's':
          matrix_dim = atoi(optarg);
          break;
        case '?':
          fprintf(stderr, "invalid option\n");
          break;
        case ':':
          fprintf(stderr, "missing argument\n");
          break;
        default:
          fprintf(stderr, "Usage: %s [-v] [-n no. of threads] [-s matrix_size|-i input_file]\n",
                  argv[0]);
          exit(EXIT_FAILURE);
      }
  }

  if ( (optind < argc) || (optind == 1)) {
      fprintf(stderr, "Usage: %s [-v] [-n no. of threads] [-s matrix_size|-i input_file]\n", argv[0]);
      exit(EXIT_FAILURE);
  }

  if (input_file) {
      printf("Reading matrix from file %s\n", input_file);
      ret = create_matrix_from_file(&m, input_file, &matrix_dim);
      if (ret != RET_SUCCESS) {
          m = NULL;
          fprintf(stderr, "error create matrix from file %s\n", input_file);
          exit(EXIT_FAILURE);
      }
  } else if (matrix_dim) {
    printf("Creating matrix internally size=%d\n", matrix_dim);
    ret = create_matrix(&m, matrix_dim);
    if (ret != RET_SUCCESS) {
        m = NULL;
        fprintf(stderr, "error create matrix internally size=%d\n", matrix_dim);
        exit(EXIT_FAILURE);
    }
  } else {
    printf("No input file specified!\n");
    exit(EXIT_FAILURE);
  }

  if (do_verify){
    printf("Before LUD\n");
    matrix_duplicate(m, &mm, matrix_dim);
  }

      stopwatch_start(&sw);
      lud_openmp_gpu(m, matrix_dim);
      stopwatch_stop(&sw);
      printf("Time consumed(ms): %lf\n", 1000*get_interval_by_sec(&sw));
      double checksum = 0.0;
      long long matrix_elements = (long long)matrix_dim * (long long)matrix_dim;
      for (long long i = 0; i < matrix_elements; i++) {
        checksum += m[i];
      }
      printf("LUD checksum: %.8f\n", checksum);

  if (do_verify){
    printf("After LUD\n");
    printf(">>>Verify<<<<\n");
    if (lud_verify(mm, m, matrix_dim) != RET_SUCCESS) {
      free(mm);
      free(m);
      return EXIT_FAILURE;
    }
    free(mm);
  }

  free(m);

  return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
