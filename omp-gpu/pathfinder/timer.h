
#include <time.h>

// #define BENCH_PRINT

/*--------- using monotonic wall-clock time ------------*/

extern struct timespec rodinia_pathfinder_starttime;
extern struct timespec rodinia_pathfinder_endtime;

#define startTime() \
do { \
  clock_gettime(CLOCK_MONOTONIC, &rodinia_pathfinder_starttime); \
} while (0)
#define stopTime(valusecs) \
do { \
  clock_gettime(CLOCK_MONOTONIC, &rodinia_pathfinder_endtime); \
  valusecs = ((long long)(rodinia_pathfinder_endtime.tv_sec - rodinia_pathfinder_starttime.tv_sec) * 1000000000LL \
             + (rodinia_pathfinder_endtime.tv_nsec - rodinia_pathfinder_starttime.tv_nsec)) / 1000LL; \
} while (0)

#define startCycle() startTime()
#define stopCycle(cycles) stopTime(cycles)
