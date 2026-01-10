/*
 * =====================================================================================
 *
 *       Filename:  lud.cu
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
#include <stdlib.h>
#include <assert.h>

#include "common.h"
#include <sys/time.h>
#include <CL/cl.h>

#include <string.h>
#include <string>
#ifdef RD_WG_SIZE_0_0
        #define BLOCK_SIZE RD_WG_SIZE_0_0
#elif defined(RD_WG_SIZE_0)
        #define BLOCK_SIZE RD_WG_SIZE_0
#elif defined(RD_WG_SIZE)
        #define BLOCK_SIZE RD_WG_SIZE
#else
        #define BLOCK_SIZE 16
#endif

 double gettime() {
  struct timeval t;
  gettimeofday(&t,NULL);
  return t.tv_sec+t.tv_usec*1e-6;
}

static cl_context	    context;
static cl_command_queue cmd_queue;
static cl_device_type   device_type;
static cl_device_id   * device_list;
static cl_uint           num_devices;

static int initialize(int use_gpu)
{
	cl_int result;
	size_t size;

	// create OpenCL context
	cl_platform_id platform_id;
	if (clGetPlatformIDs(1, &platform_id, NULL) != CL_SUCCESS) { printf("ERROR: clGetPlatformIDs(1,*,0) failed\n"); return -1; }
	device_type = use_gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU;

	result = clGetDeviceIDs(platform_id, device_type, 0, NULL, &num_devices);
	if (result != CL_SUCCESS || num_devices < 1) {
		printf("ERROR: clGetDeviceIDs() failed\n");
		return -1;
	}

	device_list = new cl_device_id[num_devices];
	if( !device_list ) { printf("ERROR: new cl_device_id[] failed\n"); return -1; }
	result = clGetDeviceIDs(platform_id, device_type, num_devices, device_list, NULL);
	if( result != CL_SUCCESS ) { printf("ERROR: clGetDeviceIDs() failed\n"); return -1; }

	context = clCreateContext( NULL, num_devices, device_list, NULL, NULL, &result );
	if( result != CL_SUCCESS || !context ) { printf("ERROR: clCreateContext() failed\n"); return -1; }
	printf("num_devices = %d\n", num_devices);

	// create command queue for the first device
	const cl_queue_properties queue_props[] = {CL_QUEUE_PROPERTIES, 0, 0};
	cmd_queue = clCreateCommandQueueWithProperties(context, device_list[0], queue_props, NULL);
	if( !cmd_queue ) { printf("ERROR: clCreateCommandQueueWithProperties() failed\n"); return -1; }
	return 0;
}

static int shutdown()
{
	// release resources
	if( cmd_queue ) clReleaseCommandQueue( cmd_queue );
	if( context ) clReleaseContext( context );
	if( device_list ) delete[] device_list;

	// reset all variables
	cmd_queue = 0;
	context = 0;
	device_list = 0;
	num_devices = 0;
	device_type = 0;

	return 0;
}

static int do_verify = 0;
void lud_cuda(float *d_m, int matrix_dim);

int
main ( int argc, char *argv[] )
{
  printf("WG size of kernel = %d X %d\n", BLOCK_SIZE, BLOCK_SIZE);
	int matrix_dim = 32; /* default matrix_dim */
	func_ret_t ret;
	const char *input_file = NULL;
	float *m, *mm;
	stopwatch sw;

	if (argc == 1) {
		fprintf(stderr, "Usage: %s [-v] [-s matrix_size|-i input_file]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	for (int i = 1; i < argc; ++i) {
		const char *arg = argv[i];
		if (strcmp(arg, "-v") == 0 || strcmp(arg, "--verify") == 0) {
			do_verify = 1;
			continue;
		}
		if (strcmp(arg, "-i") == 0 || strcmp(arg, "--input") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "missing argument\n");
				exit(EXIT_FAILURE);
			}
			input_file = argv[++i];
			continue;
		}
		if (strncmp(arg, "--input=", 8) == 0) {
			input_file = arg + 8;
			continue;
		}
		if (strncmp(arg, "-i", 2) == 0 && strlen(arg) > 2) {
			input_file = arg + 2;
			continue;
		}
		if (strcmp(arg, "-s") == 0 || strcmp(arg, "--size") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "missing argument\n");
				exit(EXIT_FAILURE);
			}
			matrix_dim = atoi(argv[++i]);
			printf("Generate input matrix internally, size =%d\n", matrix_dim);
			continue;
		}
		if (strncmp(arg, "--size=", 7) == 0) {
			matrix_dim = atoi(arg + 7);
			printf("Generate input matrix internally, size =%d\n", matrix_dim);
			continue;
		}
		if (strncmp(arg, "-s", 2) == 0 && strlen(arg) > 2) {
			matrix_dim = atoi(arg + 2);
			printf("Generate input matrix internally, size =%d\n", matrix_dim);
			continue;
		}

		fprintf(stderr, "Usage: %s [-v] [-s matrix_size|-i input_file]\n", argv[0]);
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
	} 
	
	else if (matrix_dim) {
	  printf("Creating matrix internally size=%d\n", matrix_dim);
	  ret = create_matrix(&m, matrix_dim);
	  if (ret != RET_SUCCESS) {
	    m = NULL;
	    fprintf(stderr, "error create matrix internally size=%d\n", matrix_dim);
	    exit(EXIT_FAILURE);
	  }
	}

	else {
	  printf("No input file specified!\n");
	  exit(EXIT_FAILURE);
	}

	if (do_verify){
		printf("Before LUD\n");
		// print_matrix(m, matrix_dim);
		matrix_duplicate(m, &mm, matrix_dim);
	}
	
	int sourcesize = 1024*1024;
	char * source = (char *)calloc(sourcesize, sizeof(char)); 
	if(!source) { printf("ERROR: calloc(%d) failed\n", sourcesize); return -1; }

	const char *kernel_lud_diag   = "lud_diagonal";
	const char *kernel_lud_peri   = "lud_perimeter";
	const char *kernel_lud_inter  = "lud_internal";
	const char *kernel_path = "./lud_kernel.cl";
	FILE * fp = fopen(kernel_path, "rb");
	if(!fp) { printf("ERROR: unable to open '%s'\n", kernel_path); return -1; }
	size_t read_size = fread(source, 1, (size_t)sourcesize - 1, fp);
	if (read_size == 0 && ferror(fp)) {
		printf("ERROR: unable to read '%s'\n", kernel_path);
		fclose(fp);
		free(source);
		return -1;
	}
	fclose(fp);

	// Use 1: GPU  0: CPU
	int use_gpu =
#ifdef RODINIA_OPENCL_FORCE_CPU
		0;
#else
		1;
#endif
	// OpenCL initialization
	if (initialize(use_gpu)) {
		printf("ERROR: required OpenCL %s device not available\n", use_gpu ? "GPU" : "CPU");
		return -1;
	}
	// compile kernel
	cl_int err = 0;
	const char * slist[2] = { source, 0 };
	cl_program prog = clCreateProgramWithSource(context, 1, slist, NULL, &err);
	if(err != CL_SUCCESS) { printf("ERROR: clCreateProgramWithSource() => %d\n", err); return -1; }
	char clOptions[110];
	//  sprintf(clOptions,"-I../../src"); 
	sprintf(clOptions," ");
#ifdef BLOCK_SIZE
	sprintf(clOptions + strlen(clOptions), " -DBLOCK_SIZE=%d", BLOCK_SIZE);
#endif

	err = clBuildProgram(prog, 0, NULL, clOptions, NULL, NULL);
	{ // show warnings/errors
		//static char log[65536]; memset(log, 0, sizeof(log));
		//cl_device_id device_id = 0;
		//err = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(device_id), &device_id, NULL);
		//clGetProgramBuildInfo(prog, device_id, CL_PROGRAM_BUILD_LOG, sizeof(log)-1, log, NULL);
		//if(err || strstr(log,"warning:") || strstr(log, "error:")) printf("<<<<\n%s\n>>>>\n", log);
	}
	if(err != CL_SUCCESS) { printf("ERROR: clBuildProgram() => %d\n", err); return -1; }
    
	cl_kernel diagnal;
	cl_kernel perimeter;
	cl_kernel internal;
	diagnal   = clCreateKernel(prog, kernel_lud_diag, &err);  
	perimeter = clCreateKernel(prog, kernel_lud_peri, &err);  
	internal  = clCreateKernel(prog, kernel_lud_inter, &err);  
	if(err != CL_SUCCESS) { printf("ERROR: clCreateKernel() 0 => %d\n", err); return -1; }
	clReleaseProgram(prog);
  
	//size_t local_work[3] = { 1, 1, 1 };
	//size_t global_work[3] = {1, 1, 1 }; 
  
	cl_mem d_m;
	d_m = clCreateBuffer(context, CL_MEM_READ_WRITE, matrix_dim*matrix_dim * sizeof(float), NULL, &err );
	if(err != CL_SUCCESS) { printf("ERROR: clCreateBuffer d_m (size:%d) => %d\n", matrix_dim*matrix_dim, err); return -1;} 

	/* beginning of timing point */
	stopwatch_start(&sw);
	err = clEnqueueWriteBuffer(cmd_queue, d_m, 1, 0, matrix_dim*matrix_dim*sizeof(float), m, 0, 0, 0);
	if(err != CL_SUCCESS) { printf("ERROR: clEnqueueWriteBuffer d_m (size:%d) => %d\n", matrix_dim*matrix_dim, err); return -1; }
	
	int i=0;
	for (i=0; i < matrix_dim-BLOCK_SIZE; i += BLOCK_SIZE) {
	 
	  clSetKernelArg(diagnal, 0, sizeof(void *), (void*) &d_m);
	  clSetKernelArg(diagnal, 1, sizeof(float) * BLOCK_SIZE * BLOCK_SIZE, (void*)NULL );
	  clSetKernelArg(diagnal, 2, sizeof(cl_int), (void*) &matrix_dim);
	  clSetKernelArg(diagnal, 3, sizeof(cl_int), (void*) &i);
      
	  size_t global_work1[3]  = {BLOCK_SIZE, 1, 1};
	  size_t local_work1[3]  = {BLOCK_SIZE, 1, 1};
	   
	  err = clEnqueueNDRangeKernel(cmd_queue, diagnal, 2, NULL, global_work1, local_work1, 0, 0, 0);
	  if(err != CL_SUCCESS) { printf("ERROR:  diagnal clEnqueueNDRangeKernel()=>%d failed\n", err); return -1; }	
	  
	  clSetKernelArg(perimeter, 0, sizeof(void *), (void*) &d_m);
	  clSetKernelArg(perimeter, 1, sizeof(float) * BLOCK_SIZE * BLOCK_SIZE, (void*)NULL );
	  clSetKernelArg(perimeter, 2, sizeof(float) * BLOCK_SIZE * BLOCK_SIZE, (void*)NULL );
	  clSetKernelArg(perimeter, 3, sizeof(float) * BLOCK_SIZE * BLOCK_SIZE, (void*)NULL );
	  clSetKernelArg(perimeter, 4, sizeof(cl_int), (void*) &matrix_dim);
	  clSetKernelArg(perimeter, 5, sizeof(cl_int), (void*) &i);
	  
          size_t global_work2[3] = {(size_t)(BLOCK_SIZE * 2 * ((matrix_dim-i)/BLOCK_SIZE-1)), 1, 1};
	  size_t local_work2[3]  = {BLOCK_SIZE * 2, 1, 1};
	  
	  err = clEnqueueNDRangeKernel(cmd_queue, perimeter, 2, NULL, global_work2, local_work2, 0, 0, 0);
	  if(err != CL_SUCCESS) { printf("ERROR:  perimeter clEnqueueNDRangeKernel()=>%d failed\n", err); return -1; }	
	  
	  clSetKernelArg(internal, 0, sizeof(void *), (void*) &d_m);
	  clSetKernelArg(internal, 1, sizeof(float) * BLOCK_SIZE * BLOCK_SIZE, (void*)NULL );
	  clSetKernelArg(internal, 2, sizeof(float) * BLOCK_SIZE * BLOCK_SIZE, (void*)NULL );
	  clSetKernelArg(internal, 3, sizeof(cl_int), (void*) &matrix_dim);
	  clSetKernelArg(internal, 4, sizeof(cl_int), (void*) &i);
      
          size_t global_work3[3] = {(size_t)(BLOCK_SIZE * ((matrix_dim-i)/BLOCK_SIZE-1)), (size_t)(BLOCK_SIZE * ((matrix_dim-i)/BLOCK_SIZE-1)), 1};
	  size_t local_work3[3] = {BLOCK_SIZE, BLOCK_SIZE, 1};

	  err = clEnqueueNDRangeKernel(cmd_queue, internal, 2, NULL, global_work3, local_work3, 0, 0, 0);
	  if(err != CL_SUCCESS) { printf("ERROR:  internal clEnqueueNDRangeKernel()=>%d failed\n", err); return -1; }	
	}
	clSetKernelArg(diagnal, 0, sizeof(void *), (void*) &d_m);
	clSetKernelArg(diagnal, 1, sizeof(float) * BLOCK_SIZE * BLOCK_SIZE, (void*)NULL );
	clSetKernelArg(diagnal, 2, sizeof(cl_int), (void*) &matrix_dim);
	clSetKernelArg(diagnal, 3, sizeof(cl_int), (void*) &i);
      
	size_t global_work1[3]  = {BLOCK_SIZE, 1, 1};
	size_t local_work1[3]  = {BLOCK_SIZE, 1, 1};
	err = clEnqueueNDRangeKernel(cmd_queue, diagnal, 2, NULL, global_work1, local_work1, 0, 0, 0);
	if(err != CL_SUCCESS) { printf("ERROR:  diagnal clEnqueueNDRangeKernel()=>%d failed\n", err); return -1; }	
	
	err = clEnqueueReadBuffer(cmd_queue, d_m, 1, 0, matrix_dim*matrix_dim*sizeof(float), m, 0, 0, 0);
	if(err != CL_SUCCESS) { printf("ERROR: clEnqueueReadBuffer  d_m (size:%d) => %d\n", matrix_dim*matrix_dim, err); return -1; }
	clFinish(cmd_queue);
	/* end of timing point */
	stopwatch_stop(&sw);
	printf("Time consumed(ms): %lf\n", 1000*get_interval_by_sec(&sw));

	clReleaseMemObject(d_m);

	if (do_verify){
		printf("After LUD\n");
		// print_matrix(m, matrix_dim);
		printf(">>>Verify<<<<\n");
		lud_verify(mm, m, matrix_dim); 
		free(mm);
	}

	free(m);
	
	if(shutdown()) return -1;
	
}				

/* ----------  end of function main  ---------- */
