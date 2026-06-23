//====================================================================================================100
//		UPDATE
//====================================================================================================100

// Lukasz G. Szafaryn 24 JAN 09

//====================================================================================================100
//		DESCRIPTION
//====================================================================================================100

// Myocyte application models cardiac myocyte (heart muscle cell) and simulates its behavior according to the work by Saucerman and Bers [8]. The model integrates
// cardiac myocyte electrical activity with the calcineurin pathway, which is a key aspect of the development of heart failure. The model spans large number of temporal
// scales to reflect how changes in heart rate as observed during exercise or stress contribute to calcineurin pathway activation, which ultimately leads to the expression
// of numerous genes that remodel the heart’s structure. It can be used to identify potential therapeutic targets that may be useful for the treatment of heart failure.
// Biochemical reactions, ion transport and electrical activity in the cell are modeled with 91 ordinary differential equations (ODEs) that are determined by more than 200
// experimentally validated parameters. The model is simulated by solving this group of ODEs for a specified time interval. The process of ODE solving is based on the
// causal relationship between values of ODEs at different time steps, thus it is mostly sequential. At every dynamically determined time step, the solver evaluates the
// model consisting of a set of 91 ODEs and 480 supporting equations to determine behavior of the system at that particular time instance. If evaluation results are not
// within the expected tolerance at a given time step (usually as a result of incorrect determination of the time step), another calculation attempt is made at a modified
// (usually reduced) time step. Since the ODEs are stiff (exhibit fast rate of change within short time intervals), they need to be simulated at small time scales with an
// adaptive step size solver.

//	1) The original version of the current solver code was obtained from: Mathematics Source Library (http://mymathlib.webtrellis.net/index.html). The solver has been
//      somewhat modified to tailor it to our needs. However, it can be reverted back to original form or modified to suit other simulations.
// 2) This solver and particular solving algorithm used with it (embedded_fehlberg_7_8) were adapted to work with a set of equations, not just one like in original version.
//	3) In order for solver to provide deterministic number of steps (needed for particular amount of memore previousely allocated for results), every next step is
//      incremented by 1 time unit (h_init).
//	4) Function assumes that simulation starts at some point of time (whatever time the initial values are provided for) and runs for the number of miliseconds (xmax)
//      specified by the uses as a parameter on command line.
// 5) The appropriate amount of memory is previousely allocated for that range (y).
//	6) This setup in 3) - 5) allows solver to adjust the step ony from current time instance to current time instance + 0.9. The next time instance is current time instance + 1;
//	7) The original solver cannot handle cases when equations return NAN and INF values due to discontinuities and /0. That is why equations provided by user need to
//      make sure that no NAN and INF are returned.
// 8) Application reads initial data and parameters from text files: y.txt and params.txt respectively that need to be located in the same folder as source files.
//     For simplicity and testing purposes only, when multiple number of simulation instances is specified, application still reads initial data from the same input files. That
//     can be modified in this source code.

//====================================================================================================100
//		IMPLEMENTATION-SPECIFIC DESCRIPTION (OPEN MP)
//====================================================================================================100

// This is the OpenMP version of Myocyte code.

// The original single-threaded code was written in MATLAB and used MATLAB ode45 ODE solver. In the process of accelerating this code, we arrived with the
// intermediate versions that used single-threaded Sundials CVODE solver which evaluated model parallelized with OpenMP at each time step. In order to convert entire
// solver to OpenMP code (to remove some of the operational overheads such as thread launches in OpenMP) we used a simpler solver, from Mathematics Source
// Library, and tailored it to our needs. The parallelism in the cardiac myocyte model is on a very fine-grained level, close to that of ILP, therefore it is very hard to exploit
// as DLP or TLB in OpenMP code. We were able to divide the model into 4 individual groups that run in parallel. However, even that is not enough work to compensate
// for some of the OpenMP thread launch overheads, which resulted in performance worse than that of single-threaded C code. Speedup in this code could
// be achieved only if a customizable accelerator such as FPGA was used for evaluation of the model itself. We also approached the application from another angle and
// allowed it to run several concurrent simulations, thus turning it into an embarrassingly parallel problem. This version of the code is also useful for scientists who want to
// run the same simulation with different sets of input parameters. OpenMP version of this code provides constant speedup of about 3.64x regardless of the number of
// concurrent simulations.

// Speedup numbers reported in the description of this application were obtained on the machine with: Intel Quad Core CPU, 4GB of RAM, Nvidia GTX280 GPU.

// 1) When running with parallelization inside each simulation instance (value of 3rd command line parameter equal to 0), performance is bad because:
// a) thread launch overhead
// b) small amount of work for each forked thread
// 2) When running with parallelization across simulation instances, code gets continues speedup with the increasing number of simulation insances which saturates
//     around 4 instances on Quad Core CPU (roughly corresponding to the number of multiprocessorsXprocessors in GTX280), with the speedup of around 3.5x compared
//     to serial C version of code, as expected.

// The following are the command parameters to the application:
// 1) Simulation time interval which is the number of miliseconds to simulate. Needs to be integer > 0
// 2) Number of instances of simulation to run. Needs to be integer > 0.
// 3) Method of parallelization. Need to be 0 for parallelization inside each simulation instance, or 1 for parallelization across instances.
// 4) Number of threads to use. Needs to be integer > 0.
// Example:
// a.out 100 100 1 4

//====================================================================================================100
//	DEFINE / INCLUDE
//====================================================================================================100

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>

#include <omp.h>

#include "define.c"
#pragma omp declare target
#include "ecc.c"
#include "cam.c"
#include "fin.c"
#include "master.c"
#include "embedded_fehlberg_7_8.c"
#include "solver.c"
#pragma omp end declare target

#include "file.c"
#include "timer.c"

static void make_data_path(char *path, size_t path_size, const char *filename)
{
	const char *data_dir = getenv("RODINIA_DATA_DIR");
	int written;
	if (data_dir && data_dir[0] != '\0') {
		written = snprintf(path, path_size, "%s/myocyte/%s", data_dir, filename);
	} else {
		written = snprintf(path, path_size, "../../data/myocyte/%s", filename);
	}
	if (written <= 0 || (size_t)written >= path_size) {
		fprintf(stderr, "Input path is too long for '%s'\n", filename);
		exit(EXIT_FAILURE);
	}
}

static void make_output_path(char *path, size_t path_size, const char *filename)
{
	const char *output_dir = getenv("RODINIA_OUTPUT_DIR");
	int written;
	if (output_dir && output_dir[0] != '\0') {
		written = snprintf(path, path_size, "%s/%s", output_dir, filename);
	} else {
		written = snprintf(path, path_size, "%s", filename);
	}
	if (written <= 0 || (size_t)written >= path_size) {
		fprintf(stderr, "Output path is too long for '%s'\n", filename);
		exit(EXIT_FAILURE);
	}
}

//====================================================================================================100
//	MAIN FUNCTION
//====================================================================================================100

static int checked_mul_size(size_t a, size_t b, size_t *result)
{
	if (a != 0 && b > ((size_t)-1) / a) {
		return -1;
	}
	*result = a * b;
	return 0;
}

static void *checked_malloc_array(size_t count, size_t elem_size)
{
	size_t bytes;
	if (checked_mul_size(count, elem_size, &bytes) != 0) {
		return NULL;
	}
	return malloc(bytes);
}

static void free_simulation_data(fp ***y, fp **params, fp *solver_workspace)
{
	if (y != NULL) {
		if (y[0] != NULL) {
			free(y[0][0]);
			free(y[0]);
		}
		free(y);
	}
	if (params != NULL) {
		free(params[0]);
		free(params);
	}
	free(solver_workspace);
}

int main(int argc, char *argv []){

	//================================================================================80
	//		VARIABLES
	//================================================================================80

	//============================================================60
	//		TIME
	//============================================================60

	long long time0;
	long long time1;
	long long time2;
	long long time3;
	long long time4;
	long long time5;

	time0 = get_time();

	//============================================================60
	//		COUNTERS
	//============================================================60

	long long memory;
	int i,j;
	int status;
	int mode;

	//============================================================60
	//		SOLVER PARAMETERS
	//============================================================60

	long workload;
	long xmin;
	long xmax;
	fp h;
	fp tolerance;

	//============================================================60
	//		DATA
	//============================================================60

	fp*** y;
	fp** params;
	fp *solver_workspace = NULL;

	//================================================================================80
	// 	GET INPUT PARAMETERS
	//================================================================================80

	//============================================================60
	//		CHECK NUMBER OF ARGUMENTS
	//============================================================60

	if(argc!=4){
		printf("ERROR: %d is the incorrect number of arguments, the number of arguments must be 4\n", argc-1);
		return EXIT_FAILURE;
	}

	//============================================================60
	//		GET AND CHECK PARTICULAR ARGUMENTS
	//============================================================60

	else{

		//========================================40
		//		SPAN
		//========================================40

		xmax = atoi(argv[1]);
		if(xmax<=0){
			printf("ERROR: %ld is the incorrect end of simulation interval, use numbers > 0\n", xmax);
			return EXIT_FAILURE;
		}

		//========================================40
		//		WORKLOAD
		//========================================40

		workload = atoi(argv[2]);
		if(workload<=0){
			printf("ERROR: %ld is the incorrect number of instances of simulation, use numbers > 0\n", workload);
			return EXIT_FAILURE;
		}

		//========================================40
		//		MODE
		//========================================40

		mode = 0;
		mode = atoi(argv[3]);
		if(mode != 0 && mode != 1){
			printf("ERROR: %d is the incorrect mode, it should be omitted or equal to 0 or 1\n", mode);
			return EXIT_FAILURE;
		}

	}

	time1 = get_time();

	//================================================================================80
	// 	ALLOCATE MEMORY
	//================================================================================80

	//============================================================60
	//		MEMORY CHECK
	//============================================================60

	size_t workload_count = (size_t)workload;
	size_t step_count = (size_t)(1 + xmax);
	size_t time_steps;
	size_t equation_values;
	size_t parameter_values;
	size_t workspace_values;
	if (checked_mul_size(workload_count, step_count, &time_steps) != 0 ||
		checked_mul_size(time_steps, (size_t)EQUATIONS, &equation_values) != 0 ||
		checked_mul_size(workload_count, (size_t)PARAMETERS, &parameter_values) != 0 ||
		checked_mul_size(workload_count, (size_t)FEHLBERG_WORKSPACE_ELEMENTS, &workspace_values) != 0) {
		fprintf(stderr, "ERROR: allocation size overflow\n");
		return EXIT_FAILURE;
	}
	if (equation_values > (size_t)LLONG_MAX / sizeof(fp)) {
		fprintf(stderr, "ERROR: allocation size overflow\n");
		return EXIT_FAILURE;
	}
	memory = (long long)(equation_values * sizeof(fp));
	if(memory>1000000000){
		printf("ERROR: trying to allocate more than 1.0GB of memory, decrease workload and span parameters or change memory parameter\n");
		return EXIT_FAILURE;
	}

	//============================================================60
	// 	ALLOCATE ARRAYS
	//============================================================60

	y = (fp ***) checked_malloc_array(workload_count, sizeof(fp **));
	if (y == NULL) {
		fprintf(stderr, "ERROR: Memory allocation failed for y\n");
		return EXIT_FAILURE;
	}
	y[0] = (fp **) checked_malloc_array(time_steps, sizeof(fp*));
	if (y[0] == NULL) {
		fprintf(stderr, "ERROR: Memory allocation failed for y[0]\n");
		free(y);
		return EXIT_FAILURE;
	}
	y[0][0] = (fp *) checked_malloc_array(equation_values, sizeof(fp));
	if (y[0][0] == NULL) {
		fprintf(stderr, "ERROR: Memory allocation failed for y[0][0]\n");
		free(y[0]);
		free(y);
		return EXIT_FAILURE;
	}
	for (i = 0; i < workload; i++){
		y[i] = y[0] + i * (1+xmax);
		for (j = 0; j < (1+xmax); j++){
			y[i][j] = y[0][0] + ((i * (1+xmax) + j) * EQUATIONS);
		}
	}

	params = (fp **) checked_malloc_array(workload_count, sizeof(fp *));
	if (params == NULL) {
		fprintf(stderr, "ERROR: Memory allocation failed for params\n");
		free(y[0][0]);
		free(y[0]);
		free(y);
		return EXIT_FAILURE;
	}
	params[0] = (fp *) checked_malloc_array(parameter_values, sizeof(fp));
	if (params[0] == NULL) {
		fprintf(stderr, "ERROR: Memory allocation failed for params[0]\n");
		free(params);
		free(y[0][0]);
		free(y[0]);
		free(y);
		return EXIT_FAILURE;
	}
	for (i = 0; i < workload; i++){
		params[i] = params[0] + i * PARAMETERS;
	}

	solver_workspace = (fp *) checked_malloc_array(workspace_values, sizeof(fp));
	if (solver_workspace == NULL) {
		fprintf(stderr, "ERROR: Memory allocation failed for solver workspace\n");
		free_simulation_data(y, params, NULL);
		return EXIT_FAILURE;
	}

	time2 = get_time();

	//================================================================================80
	// 	INITIAL VALUES
	//================================================================================80

	// y
	char input_path[PATH_MAX];
	make_data_path(input_path, sizeof(input_path), "y.txt");
	read_file(	input_path,
				y[0][0],
				EQUATIONS,
				1,
				0);
	for(i=1; i<workload; i++){
		memcpy(y[i][0], y[0][0], (size_t)EQUATIONS * sizeof(fp));
	}

	// params
	make_data_path(input_path, sizeof(input_path), "params.txt");
	read_file(	input_path,
				params[0],
				PARAMETER_INPUTS,
				1,
				0);
	for(i=1; i<workload; i++){
		memcpy(params[i], params[0], (size_t)PARAMETER_INPUTS * sizeof(fp));
	}

	time3 = get_time();

	//================================================================================80
	//	EXECUTION
	//================================================================================80

	fp *y_data = y[0][0];
	fp *params_data = params[0];

	status = 0;
	if (mode == 0) {
		for (i = 0; i < workload; i++) {
			fp *workspace = solver_workspace + (size_t)i * FEHLBERG_WORKSPACE_ELEMENTS;
			int local_status = solver(y[i][0], xmax, params[i], mode, workspace);
			if (local_status < status) {
				status = local_status;
			}
		}
	} else {
		#pragma omp target teams distribute parallel for \
			map(tofrom:y_data[0:equation_values]) \
			map(to:params_data[0:parameter_values]) \
			map(alloc:solver_workspace[0:workspace_values]) \
			reduction(min:status)
		for (i = 0; i < workload; i++) {
			fp *workspace = solver_workspace + (size_t)i * FEHLBERG_WORKSPACE_ELEMENTS;
			int local_status = solver(y_data + (size_t)i * (size_t)(xmax + 1) * EQUATIONS,
									  xmax,
									  params_data + (size_t)i * PARAMETERS,
									  mode,
									  workspace);
			if (local_status < status) {
				status = local_status;
			}
		}
	}

	if (status != 0) {
		fprintf(stderr, "STATUS: %d\n", status);
		free_simulation_data(y, params, solver_workspace);
		return EXIT_FAILURE;
	}


	  FILE * pFile;
	  char output_path[PATH_MAX];
	  make_output_path(output_path, sizeof(output_path), "output.txt");
	  pFile = fopen(output_path, "w");
	  if (pFile==NULL)
	    {
	  perror(output_path);
	  free_simulation_data(y, params, solver_workspace);
	  return EXIT_FAILURE;
	}
	  // print results
	  int k;
	  for(i=0; i<workload; i++){
	  fprintf(pFile, "WORKLOAD %d:\n", i);
	  for(j=0; j<(xmax+1); j++){
	  fprintf(pFile, "\tTIME %d:\n", j);
	  for(k=0; k<EQUATIONS; k++){
	  fprintf(pFile, "\t\ty[%d][%d][%d]=%10.7e\n", i, j, k, y[i][j][k]);
	}
	}
	}

	  fclose (pFile);


	time4 = get_time();

	//================================================================================80
	//	DEALLOCATION
	//================================================================================80

	free_simulation_data(y, params, solver_workspace);

	time5= get_time();

	//================================================================================80
	//		DISPLAY TIMING
	//================================================================================80

	printf("Time spent in different stages of the application:\n");
	printf("%.12f s, %.12f %% : SETUP VARIABLES, READ COMMAND LINE ARGUMENTS\n", 	(float) (time1-time0) / 1000000, (float) (time1-time0) / (float) (time5-time0) * 100);
	printf("%.12f s, %.12f %% : ALLOCATE MEMORY\n", 														(float) (time2-time1) / 1000000, (float) (time2-time1) / (float) (time5-time0) * 100);
	printf("%.12f s, %.12f %% : READ DATA FROM FILES\n", 												(float) (time3-time2) / 1000000, (float) (time3-time2) / (float) (time5-time0) * 100);
	printf("%.12f s, %.12f %% : RUN COMPUTATION\n", 														(float) (time4-time3) / 1000000, (float) (time4-time3) / (float) (time5-time0) * 100);
	printf("%.12f s, %.12f %% : FREE MEMORY\n", 																(float) (time5-time4) / 1000000, (float) (time5-time4) / (float) (time5-time0) * 100);
	printf("Total time:\n");
	printf("%.12f s\n", 																											(float) (time5-time0) / 1000000);

//====================================================================================================100
//	END OF FILE
//====================================================================================================100

}
