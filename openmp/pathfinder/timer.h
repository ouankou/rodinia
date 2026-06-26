
#include <chrono>

// #define BENCH_PRINT

/*--------- using monotonic wall-clock time ------------*/

extern std::chrono::steady_clock::time_point rodinia_pathfinder_starttime;
extern std::chrono::steady_clock::time_point rodinia_pathfinder_endtime;

#define startTime() \
do { \
  rodinia_pathfinder_starttime = std::chrono::steady_clock::now(); \
} while (0)
#define stopTime(valusecs) \
do { \
  rodinia_pathfinder_endtime = std::chrono::steady_clock::now(); \
  valusecs = std::chrono::duration_cast<std::chrono::microseconds>( \
               rodinia_pathfinder_endtime - rodinia_pathfinder_starttime).count(); \
} while (0)

#define startCycle() startTime()
#define stopCycle(cycles) stopTime(cycles)
