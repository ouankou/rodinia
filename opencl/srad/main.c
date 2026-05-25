// #ifdef __cplusplus
// extern "C" {
// #endif

//========================================================================================================================================================================================================200
//======================================================================================================================================================150
//====================================================================================================100
//==================================================50

//========================================================================================================================================================================================================200
//	INCLUDE/DEFINE
//========================================================================================================================================================================================================200

//======================================================================================================================================================150
//	LIBRARIES
//======================================================================================================================================================150

#include <stdio.h>									// (in path known to compiler)	needed by printf
#include <stdlib.h>									// (in path known to compiler)	needed by malloc, free
#include <sys/stat.h>								// (in path known to compiler)	needed by mkdir/stat
#include <errno.h>									// (in path known to compiler)	needed by errno
#include <limits.h>								// (in path known to compiler)	needed by PATH_MAX

//======================================================================================================================================================150
//	HEADER
//======================================================================================================================================================150

#include "./main.h"									// (in current path)

//======================================================================================================================================================150
//	UTILITIES
//======================================================================================================================================================150

#include "./util/graphics/graphics.h"				// (in specified path)
#include "./util/graphics/resize.h"					// (in specified path)
#include "./util/timer/timer.h"						// (in specified path)

//======================================================================================================================================================150
//	KERNEL
//======================================================================================================================================================150

#include "./kernel/kernel_gpu_opencl_wrapper.h"

//======================================================================================================================================================150
//	End
//======================================================================================================================================================150

//========================================================================================================================================================================================================200
//	MAIN FUNCTION
//========================================================================================================================================================================================================200

int 
main(	int argc, 
		char* argv []){
  printf("WG size of kernel = %d \n", NUMBER_THREADS);
	//======================================================================================================================================================150
	// 	VARIABLES
	//======================================================================================================================================================150

	// time
	long long time0;
	long long time1;
	long long time2;
	long long time3;
	long long time4;
	long long time5;
	long long time6;

	// inputs image, input paramenters
	fp* image_ori;																// originalinput image
	int image_ori_rows;
	int image_ori_cols;
	long image_ori_elem;

	// inputs image, input paramenters
	fp* image;															// input image
	int Nr,Nc;													// IMAGE nbr of rows/cols/elements
	long Ne;

	// output path
	const char *output_path = "./output/image_out.pgm";
	int output_is_default = 1;

	// algorithm parameters
	int niter;																// nbr of iterations
	fp lambda;															// update step size

	// size of IMAGE
	int r1,r2,c1,c2;												// row/col coordinates of uniform ROI
	long NeROI;														// ROI nbr of elements

	// surrounding pixel indicies
	int* iN;
	int* iS;
	int* jE;
	int* jW;    

	// counters
	int iter;   // primary loop
	long i;    // image row
	long j;    // image col

	// memory sizes
	int mem_size_i;
	int mem_size_j;

	time0 = get_time();

	//======================================================================================================================================================150
	//	INPUT ARGUMENTS
	//======================================================================================================================================================150

	if(argc != 5 && argc != 6){
		printf("ERROR: wrong number of arguments\n");
		printf("Usage: %s <niter> <lambda> <rows> <cols> [output_path]\n", argv[0]);
		return 0;
	}
	else{
		niter = atoi(argv[1]);
		lambda = atof(argv[2]);
		Nr = atoi(argv[3]);						// it is 502 in the original image
		Nc = atoi(argv[4]);						// it is 458 in the original image
		if (argc == 6) {
			output_path = argv[5];
			output_is_default = 0;
		}
	}
	if (output_is_default) {
		const char *output_dir = getenv("RODINIA_OUTPUT_DIR");
		char output_full[PATH_MAX];
		if (output_dir && output_dir[0] != '\0') {
			int written = snprintf(output_full, sizeof(output_full), "%s/%s", output_dir, "image_out.pgm");
			if (written > 0 && (size_t)written < sizeof(output_full)) {
				output_path = output_full;
			} else {
				fprintf(stderr, "Output path is too long for '%s'\n", output_path);
				return EXIT_FAILURE;
			}
		}
	}

	time1 = get_time();

	//======================================================================================================================================================150
	// 	READ INPUT FROM FILE
	//======================================================================================================================================================150

	//====================================================================================================100
	// 	READ IMAGE (SIZE OF IMAGE HAS TO BE KNOWN)
	//====================================================================================================100

	image_ori_rows = 502;
	image_ori_cols = 458;
	image_ori_elem = image_ori_rows * image_ori_cols;

	image_ori = (fp*)malloc(sizeof(fp) * image_ori_elem);

	read_graphics(	"../../data/srad/image.pgm",
					image_ori,
					image_ori_rows,
					image_ori_cols,
					1);

	//====================================================================================================100
	// 	RESIZE IMAGE (ASSUMING COLUMN MAJOR STORAGE OF image_orig)
	//====================================================================================================100

	Ne = Nr*Nc;

	image = (fp*)malloc(sizeof(fp) * Ne);

	resize(	image_ori,
				image_ori_rows,
				image_ori_cols,
				image,
				Nr,
				Nc,
				1);

	//====================================================================================================100
	// 	End
	//====================================================================================================100

	time2 = get_time();

	//======================================================================================================================================================150
	// 	SETUP
	//======================================================================================================================================================150

	// variables
	r1     = 0;											// top row index of ROI
	r2     = Nr - 1;									// bottom row index of ROI
	c1     = 0;											// left column index of ROI
	c2     = Nc - 1;									// right column index of ROI

	// ROI image size
	NeROI = (r2-r1+1)*(c2-c1+1);											// number of elements in ROI, ROI size

	// allocate variables for surrounding pixels
	mem_size_i = sizeof(int) * Nr;											//
	iN = (int *)malloc(mem_size_i) ;										// north surrounding element
	iS = (int *)malloc(mem_size_i) ;										// south surrounding element
	mem_size_j = sizeof(int) * Nc;											//
	jW = (int *)malloc(mem_size_j) ;										// west surrounding element
	jE = (int *)malloc(mem_size_j) ;										// east surrounding element

	// N/S/W/E indices of surrounding pixels (every element of IMAGE)
	for (i=0; i<Nr; i++) {
		iN[i] = i-1;														// holds index of IMAGE row above
		iS[i] = i+1;														// holds index of IMAGE row below
	}
	for (j=0; j<Nc; j++) {
		jW[j] = j-1;														// holds index of IMAGE column on the left
		jE[j] = j+1;														// holds index of IMAGE column on the right
	}

	// N/S/W/E boundary conditions, fix surrounding indices outside boundary of image
	iN[0]    = 0;															// changes IMAGE top row index from -1 to 0
	iS[Nr-1] = Nr-1;														// changes IMAGE bottom row index from Nr to Nr-1 
	jW[0]    = 0;															// changes IMAGE leftmost column index from -1 to 0
	jE[Nc-1] = Nc-1;														// changes IMAGE rightmost column index from Nc to Nc-1

	time3= get_time();

	//======================================================================================================================================================150
	// 	KERNEL
	//======================================================================================================================================================150

	kernel_gpu_opencl_wrapper(	image,											// input image
								Nr,												// IMAGE nbr of rows
								Nc,												// IMAGE nbr of cols
								Ne,												// IMAGE nbr of elem
								niter,											// nbr of iterations
								lambda,											// update step size
								NeROI,											// ROI nbr of elements
								iN,
								iS,
								jE,
								jW,
								iter,											// primary loop
								mem_size_i,
								mem_size_j);

	time4 = get_time();

	//======================================================================================================================================================150
	// 	WRITE OUTPUT IMAGE TO FILE
	//======================================================================================================================================================150

	if (output_is_default) {
		if (mkdir("./output", 0755) != 0 && errno != EEXIST) {
			printf("ERROR: unable to create ./output directory\n");
			return 0;
		}
	}

	write_graphics(	(char *)output_path,
					image,
					Nr,
					Nc,
					1,
					255);

	time5 = get_time();

	//======================================================================================================================================================150
	// 	FREE MEMORY
	//======================================================================================================================================================150

	free(image_ori);
	free(image);
	free(iN); 
	free(iS); 
	free(jW); 
	free(jE);

	time6 = get_time();

	//======================================================================================================================================================150
	//	DISPLAY TIMING
	//======================================================================================================================================================150

	printf("Time spent in different stages of the application:\n");
	printf("%.12f s, %.12f %% : READ COMMAND LINE PARAMETERS\n", 						(fp) (time1-time0) / 1000000, (fp) (time1-time0) / (fp) (time5-time0) * 100);
	printf("%.12f s, %.12f %% : READ AND RESIZE INPUT IMAGE FROM FILE\n", 				(fp) (time2-time1) / 1000000, (fp) (time2-time1) / (fp) (time5-time0) * 100);
	printf("%.12f s, %.12f %% : SETUP\n", 												(fp) (time3-time2) / 1000000, (fp) (time3-time2) / (fp) (time5-time0) * 100);
	printf("%.12f s, %.12f %% : KERNEL\n", 												(fp) (time4-time3) / 1000000, (fp) (time4-time3) / (fp) (time5-time0) * 100);
	printf("%.12f s, %.12f %% : WRITE OUTPUT IMAGE TO FILE\n", 							(fp) (time5-time4) / 1000000, (fp) (time5-time4) / (fp) (time5-time0) * 100);
	printf("%.12f s, %.12f %% : FREE MEMORY\n", 											(fp) (time6-time5) / 1000000, (fp) (time6-time5) / (fp) (time5-time0) * 100);
	printf("Total time:\n");
	printf("%.12f s\n", 																(fp) (time5-time0) / 1000000);

}

//========================================================================================================================================================================================================200
//	END
//========================================================================================================================================================================================================200
