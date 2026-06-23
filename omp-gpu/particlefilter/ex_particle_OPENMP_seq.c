/**
 * @file ex_particle_OPENMP_seq.c
 * @author Michael Trotter & Matt Goodrum
 * @brief Particle filter implementation in C/OpenMP
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include <omp.h>
#include <float.h>
#include <limits.h>
#include <stdint.h>
#define PI 3.1415926535897932
#define LCG_M INT_MAX
#define LCG_A 1103515245
#define LCG_C 12345
/*****************************
*GET_TIME
*returns a long int representing the time
*****************************/
long long get_time() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000000) + tv.tv_usec;
}
// Returns the number of seconds elapsed between the two specified times
float elapsed_time(long long start_time, long long end_time) {
        return (float) (end_time - start_time) / (1000 * 1000);
}

static int checked_mul_size(size_t a, size_t b, size_t *result) {
	if (a != 0 && b > SIZE_MAX / a) {
		return -1;
	}
	*result = a * b;
	return 0;
}

static void *checked_malloc_array(size_t count, size_t elem_size) {
	size_t bytes;
	if (checked_mul_size(count, elem_size, &bytes) != 0) {
		return NULL;
	}
	return malloc(bytes);
}
/**
* Takes in a double and returns the nearest integer value.
*/
#pragma omp declare target
static inline int lcg_next_seed(int current)
{
	uint32_t wrapped = (uint32_t)LCG_A * (uint32_t)current + (uint32_t)LCG_C;
	int64_t signed_wrapped = wrapped <= (uint32_t)INT32_MAX
		? (int64_t)wrapped
		: -((int64_t)(UINT32_MAX - wrapped) + 1);
	return (int)(signed_wrapped % LCG_M);
}

double roundDouble(double value){
	return round(value);
}
/**
* Set values of the 3D array to a newValue if that value is equal to the testValue
* @param testValue The value to be replaced
* @param newValue The value to replace testValue with
* @param array3D The image vector
* @param dimX The x dimension of the frame
* @param dimY The y dimension of the frame
* @param dimZ The number of frames
*/
void setIf(int testValue, int newValue, int * array3D, int * dimX, int * dimY, int * dimZ){
	int x, y, z;
	for(x = 0; x < *dimX; x++){
		for(y = 0; y < *dimY; y++){
			for(z = 0; z < *dimZ; z++){
				if(array3D[x * *dimY * *dimZ+y * *dimZ + z] == testValue)
				array3D[x * *dimY * *dimZ + y * *dimZ + z] = newValue;
			}
		}
	}
}
/**
* Generates a uniformly distributed random number using the provided seed and GCC's settings for the Linear Congruential Generator (LCG)
* @see http://en.wikipedia.org/wiki/Linear_congruential_generator
* @note This function is thread-safe
* @param seed The seed array
* @param index The specific index of the seed to be advanced
* @return a uniformly distributed number [0, 1)
*/
double randu(int * seed, int index)
{
	seed[index] = lcg_next_seed(seed[index]);
	double value = fabs(seed[index]/((double) LCG_M));
	return value == 0.0 ? 1.0 / (double)LCG_M : value;
}
/**
* Generates a normally distributed random number using the Box-Muller transformation
* @note This function is thread-safe
* @param seed The seed array
* @param index The specific index of the seed to be advanced
* @return a double representing random number generated using the Box-Muller algorithm
* @see http://en.wikipedia.org/wiki/Normal_distribution, section computing value for normal random distribution
*/
double randn(int * seed, int index){
	/*Box-Muller algorithm*/
	double u = randu(seed, index);
	double v = randu(seed, index);
	double cosine = cos(2*PI*v);
	double rt = -2*log(u);
	return sqrt(rt)*cosine;
}
#pragma omp end declare target
/**
* Sets values of 3D matrix using randomly generated numbers from a normal distribution
* @param array3D The video to be modified
* @param dimX The x dimension of the frame
* @param dimY The y dimension of the frame
* @param dimZ The number of frames
* @param seed The seed array
*/
void addNoise(int * array3D, int * dimX, int * dimY, int * dimZ, int * seed){
	int x, y, z;
	for(x = 0; x < *dimX; x++){
		for(y = 0; y < *dimY; y++){
			for(z = 0; z < *dimZ; z++){
				array3D[x * *dimY * *dimZ + y * *dimZ + z] = array3D[x * *dimY * *dimZ + y * *dimZ + z] + (int)(5*randn(seed, 0));
			}
		}
	}
}
/**
* Fills a radius x radius matrix representing the disk
* @param disk The pointer to the disk to be made
* @param radius  The radius of the disk to be made
*/
void strelDisk(int * disk, int radius)
{
	int diameter = radius*2 - 1;
	int x, y;
	for(x = 0; x < diameter; x++){
		for(y = 0; y < diameter; y++){
			double distance = sqrt(pow((double)(x-radius+1),2) + pow((double)(y-radius+1),2));
			disk[x*diameter + y] = (distance < radius) ? 1 : 0;
		}
	}
}
/**
* Dilates the provided video
* @param matrix The video to be dilated
* @param posX The x location of the pixel to be dilated
* @param posY The y location of the pixel to be dilated
* @param poxZ The z location of the pixel to be dilated
* @param dimX The x dimension of the frame
* @param dimY The y dimension of the frame
* @param dimZ The number of frames
* @param error The error radius
*/
void dilate_matrix(int * matrix, int posX, int posY, int posZ, int dimX, int dimY, int dimZ, int error)
{
	int startX = posX - error;
	while(startX < 0)
	startX++;
	int startY = posY - error;
	while(startY < 0)
	startY++;
	int endX = posX + error;
	while(endX > dimX)
	endX--;
	int endY = posY + error;
	while(endY > dimY)
	endY--;
	int x,y;
	for(x = startX; x < endX; x++){
		for(y = startY; y < endY; y++){
			double distance = sqrt( pow((double)(x-posX),2) + pow((double)(y-posY),2) );
			if(distance < error)
			matrix[x*dimY*dimZ + y*dimZ + posZ] = 1;
		}
	}
}

/**
* Dilates the target matrix using the radius as a guide
* @param matrix The reference matrix
* @param dimX The x dimension of the video
* @param dimY The y dimension of the video
* @param dimZ The z dimension of the video
* @param error The error radius to be dilated
* @param newMatrix The target matrix
*/
void imdilate_disk(int * matrix, int dimX, int dimY, int dimZ, int error, int * newMatrix)
{
	int x, y, z;
	for(z = 0; z < dimZ; z++){
		for(x = 0; x < dimX; x++){
			for(y = 0; y < dimY; y++){
				if(matrix[x*dimY*dimZ + y*dimZ + z] == 1){
					dilate_matrix(newMatrix, x, y, z, dimX, dimY, dimZ, error);
				}
			}
		}
	}
}
/**
* Fills a 2D array describing the offsets of the disk object
* @param se The disk object
* @param numOnes The number of ones in the disk
* @param neighbors The array that will contain the offsets
* @param radius The radius used for dilation
*/
void getneighbors(int * se, int numOnes, double * neighbors, int radius){
	int x, y;
	int neighY = 0;
	int center = radius - 1;
	int diameter = radius*2 -1;
	for(x = 0; x < diameter; x++){
		for(y = 0; y < diameter; y++){
			if(se[x*diameter + y]){
				neighbors[neighY*2] = (int)(y - center);
				neighbors[neighY*2 + 1] = (int)(x - center);
				neighY++;
			}
		}
	}
}
/**
* The synthetic video sequence we will work with here is composed of a
* single moving object, circular in shape (fixed radius)
* The motion here is a linear motion
* the foreground intensity and the backgrounf intensity is known
* the image is corrupted with zero mean Gaussian noise
* @param I The video itself
* @param IszX The x dimension of the video
* @param IszY The y dimension of the video
* @param Nfr The number of frames of the video
* @param seed The seed array used for number generation
*/
void videoSequence(int * I, int IszX, int IszY, int Nfr, int * seed){
	int k;
	size_t total_size = (size_t)IszX * (size_t)IszY * (size_t)Nfr;
	if (total_size > INT_MAX) {
		fprintf(stderr, "Video sequence is too large for int indexing\n");
		exit(EXIT_FAILURE);
	}
	int max_size = (int)total_size;
	memset(I, 0, sizeof(int) * total_size);
	/*get object centers*/
	int x0 = (int)roundDouble(IszY/2.0);
	int y0 = (int)roundDouble(IszX/2.0);
	I[x0 *IszY *Nfr + y0 * Nfr  + 0] = 1;

	/*move point*/
	int xk, yk, pos;
	for(k = 1; k < Nfr; k++){
		xk = abs(x0 + (k-1));
		yk = abs(y0 - 2*(k-1));
		pos = yk * IszY * Nfr + xk *Nfr + k;
		if(pos >= max_size)
		pos = 0;
		I[pos] = 1;
	}

	/*dilate matrix*/
	int * newMatrix = (int *)calloc(total_size, sizeof(int));
	imdilate_disk(I, IszX, IszY, Nfr, 5, newMatrix);
	int x, y;
	for(x = 0; x < IszX; x++){
		for(y = 0; y < IszY; y++){
			for(k = 0; k < Nfr; k++){
				I[x*IszY*Nfr + y*Nfr + k] = newMatrix[x*IszY*Nfr + y*Nfr + k];
			}
		}
	}
	free(newMatrix);

	/*define background, add noise*/
	setIf(0, 100, I, &IszX, &IszY, &Nfr);
	setIf(1, 228, I, &IszX, &IszY, &Nfr);
	/*add noise*/
	addNoise(I, &IszX, &IszY, &Nfr, seed);
}
/**
* Finds the first element in the CDF that is greater than or equal to the provided value and returns that index
* @param CDF The CDF
* @param lengthCDF The length of CDF
* @param value The value to find
* @return The index of value in the CDF; if value is never found, returns the last index
*/
#pragma omp declare target
int findIndexBin(double * CDF, int lengthCDF, double value){
	if(lengthCDF <= 0)
		return -1;
	int low = 0;
	int high = lengthCDF - 1;
	int index = lengthCDF - 1;
	while(low <= high){
		int middleIndex = low + ((high - low) / 2);
		if(CDF[middleIndex] >= value){
			index = middleIndex;
			high = middleIndex - 1;
		}
		else{
			low = middleIndex + 1;
		}
	}
	return index;
}
#pragma omp end declare target
/**
* The implementation of the particle filter using OpenMP for many frames
* @see http://openmp.org/wp/
* @note This function is designed to work with a video of several frames. In addition, it references a provided MATLAB function which takes the video, the objxy matrix and the x and y arrays as arguments and returns the likelihoods
* @param I The video to be run
* @param IszX The x dimension of the video
* @param IszY The y dimension of the video
* @param Nfr The number of frames
* @param seed The seed array used for random number generation
* @param Nparticles The number of particles to be used
*/
void particleFilter(int * I, int IszX, int IszY, int Nfr, int * seed, int Nparticles){

	size_t frame_size;
	size_t total_size;
	if (checked_mul_size((size_t)IszX, (size_t)IszY, &frame_size) != 0 ||
	    checked_mul_size(frame_size, (size_t)Nfr, &total_size) != 0 ||
	    total_size > INT_MAX) {
		fprintf(stderr, "Video sequence is too large for int indexing\n");
		exit(EXIT_FAILURE);
	}
	int max_size = (int)total_size;
	long long start = get_time();
	//original particle centroid
	double xe = roundDouble(IszY/2.0);
	double ye = roundDouble(IszX/2.0);

	//expected object locations, compared to center
	int radius = 5;
	int diameter = radius*2 - 1;
	size_t disk_size;
	if (checked_mul_size((size_t)diameter, (size_t)diameter, &disk_size) != 0) {
		fprintf(stderr, "Error: Structuring element is too large\n");
		exit(EXIT_FAILURE);
	}
	int * disk = (int *)checked_malloc_array(disk_size, sizeof(int));
	if (disk == NULL) {
		fprintf(stderr, "Error: Memory allocation failed for structuring element\n");
		exit(EXIT_FAILURE);
	}
	strelDisk(disk, radius);
	int countOnes = 0;
	int x, y;
	for(x = 0; x < diameter; x++){
		for(y = 0; y < diameter; y++){
			if(disk[x*diameter + y] == 1)
				countOnes++;
		}
	}
	size_t objxy_size;
	if (checked_mul_size((size_t)countOnes, 2, &objxy_size) != 0) {
		fprintf(stderr, "Error: Object neighborhood is too large\n");
		free(disk);
		exit(EXIT_FAILURE);
	}
	double * objxy = (double *)checked_malloc_array(objxy_size, sizeof(double));
	if (objxy == NULL) {
		fprintf(stderr, "Error: Memory allocation failed for object neighborhood\n");
		free(disk);
		exit(EXIT_FAILURE);
	}
	getneighbors(disk, countOnes, objxy, radius);

	long long get_neighbors = get_time();
	printf("TIME TO GET NEIGHBORS TOOK: %f\n", elapsed_time(start, get_neighbors));
	//initial weights are all equal (1/Nparticles)
	size_t particle_count = (size_t)Nparticles;
	double * weights = (double *)checked_malloc_array(particle_count, sizeof(double));
	//initial likelihood to 0.0
	double * likelihood = (double *)checked_malloc_array(particle_count, sizeof(double));
	double * arrayX = (double *)checked_malloc_array(particle_count, sizeof(double));
	double * arrayY = (double *)checked_malloc_array(particle_count, sizeof(double));
	double * xj = (double *)checked_malloc_array(particle_count, sizeof(double));
	double * yj = (double *)checked_malloc_array(particle_count, sizeof(double));
	double * CDF = (double *)checked_malloc_array(particle_count, sizeof(double));
	double * u = (double *)checked_malloc_array(particle_count, sizeof(double));
	const int scan_block = 256;
	size_t block_count = (particle_count + (size_t)scan_block - 1) / (size_t)scan_block;
	if (block_count > INT_MAX) {
		fprintf(stderr, "Error: Too many scan blocks in particleFilter\n");
		free(disk);
		free(objxy);
		free(weights);
		free(likelihood);
		free(arrayX);
		free(arrayY);
		free(xj);
		free(yj);
		free(CDF);
		free(u);
		exit(EXIT_FAILURE);
	}
	int num_blocks = (int)block_count;
	double * block_sums = (double *)checked_malloc_array(block_count, sizeof(double));
	if (!weights || !likelihood || !arrayX || !arrayY || !xj || !yj || !CDF || !u || !block_sums) {
		fprintf(stderr, "Error: Memory allocation failed in particleFilter\n");
		free(disk);
		free(objxy);
		free(weights);
		free(likelihood);
		free(arrayX);
		free(arrayY);
		free(xj);
		free(yj);
		free(CDF);
		free(u);
		free(block_sums);
		exit(EXIT_FAILURE);
	}

		#pragma omp target data \
			map(to: I[0:total_size], objxy[0:countOnes*2]) \
			map(tofrom: seed[0:Nparticles], weights[0:Nparticles], likelihood[0:Nparticles], \
			            arrayX[0:Nparticles], arrayY[0:Nparticles], xj[0:Nparticles], \
			            yj[0:Nparticles], CDF[0:Nparticles], u[0:Nparticles], \
			            block_sums[0:num_blocks])
		{
		#pragma omp target teams distribute parallel for
		for(x = 0; x < Nparticles; x++){
			weights[x] = 1/((double)(Nparticles));
		}
		long long get_weights = get_time();
		printf("TIME TO GET WEIGHTSTOOK: %f\n", elapsed_time(get_neighbors, get_weights));

		#pragma omp target teams distribute parallel for firstprivate(xe, ye)
		for(x = 0; x < Nparticles; x++){
			arrayX[x] = xe;
			arrayY[x] = ye;
		}
		int k;

		printf("TIME TO SET ARRAYS TOOK: %f\n", elapsed_time(get_weights, get_time()));
		for(k = 1; k < Nfr; k++){
			long long set_arrays = get_time();
			//apply motion model
			//draws sample from motion model (random walk). The only prior information
			//is that the object moves 2x as fast as in the y direction
			#pragma omp target teams distribute parallel for
			for(x = 0; x < Nparticles; x++){
				arrayX[x] += 1 + 5*randn(seed, x);
				arrayY[x] += -2 + 2*randn(seed, x);
			}
			long long error = get_time();
			printf("TIME TO SET ERROR TOOK: %f\n", elapsed_time(set_arrays, error));
			//particle filter likelihood
			#pragma omp target teams distribute parallel for
			for(x = 0; x < Nparticles; x++){
				double likelihood_sum = 0.0;
				for(int y = 0; y < countOnes; y++){
					int indX = roundDouble(arrayX[x]) + objxy[y*2 + 1];
					int indY = roundDouble(arrayY[x]) + objxy[y*2];
					int index = abs(indX*IszY*Nfr + indY*Nfr + k);
					if(index >= max_size)
						index = 0;
					double foreground = (double)I[index] - 100.0;
					double background = (double)I[index] - 228.0;
					likelihood_sum += (foreground*foreground - background*background)/50.0;
				}
				likelihood[x] = likelihood_sum/((double) countOnes);
			}
			long long likelihood_time = get_time();
			printf("TIME TO GET LIKELIHOODS TOOK: %f\n", elapsed_time(error, likelihood_time));
			// update & normalize weights
			// using equation (63) of Arulampalam Tutorial
			#pragma omp target teams distribute parallel for
			for(x = 0; x < Nparticles; x++){
				weights[x] = weights[x] * exp(likelihood[x]);
			}
			long long exponential = get_time();
			printf("TIME TO GET EXP TOOK: %f\n", elapsed_time(likelihood_time, exponential));
			double sumWeights = 0;
			#pragma omp target teams distribute parallel for reduction(+:sumWeights)
			for(x = 0; x < Nparticles; x++){
				sumWeights += weights[x];
			}
			long long sum_time = get_time();
			printf("TIME TO SUM WEIGHTS TOOK: %f\n", elapsed_time(exponential, sum_time));
			int bad_sum = (sumWeights <= 0.0) || !isfinite(sumWeights) || (sumWeights > DBL_MAX);
			#pragma omp target teams distribute parallel for firstprivate(sumWeights, bad_sum)
			for(x = 0; x < Nparticles; x++){
				weights[x] = bad_sum ? 1/((double)Nparticles) : weights[x]/sumWeights;
			}
			long long normalize = get_time();
			printf("TIME TO NORMALIZE WEIGHTS TOOK: %f\n", elapsed_time(sum_time, normalize));
			xe = 0;
			ye = 0;
			// estimate the object location by expected values
			#pragma omp target teams distribute parallel for reduction(+:xe, ye)
			for(x = 0; x < Nparticles; x++){
				xe += arrayX[x] * weights[x];
				ye += arrayY[x] * weights[x];
			}
			long long move_time = get_time();
			printf("TIME TO MOVE OBJECT TOOK: %f\n", elapsed_time(normalize, move_time));
			printf("XE: %lf\n", xe);
			printf("YE: %lf\n", ye);
			double distance = sqrt( pow((double)(xe-(int)roundDouble(IszY/2.0)),2) + pow((double)(ye-(int)roundDouble(IszX/2.0)),2) );
			printf("%lf\n", distance);
			//display(hold off for now)

			//pause(hold off for now)

			//resampling
			#pragma omp target teams distribute parallel for
			for(int block = 0; block < num_blocks; block++){
				int begin = block * scan_block;
				int end = begin + scan_block;
				if(end > Nparticles)
					end = Nparticles;
				double running = 0.0;
				for(int i = begin; i < end; i++){
					running += weights[i];
					CDF[i] = running;
				}
				block_sums[block] = running;
			}
			#pragma omp target update from(block_sums[0:num_blocks])
			double offset = 0.0;
			for(int block = 0; block < num_blocks; block++){
				double block_total = block_sums[block];
				block_sums[block] = offset;
				offset += block_total;
			}
			#pragma omp target update to(block_sums[0:num_blocks])
			#pragma omp target teams distribute parallel for
			for(int block = 0; block < num_blocks; block++){
				int begin = block * scan_block;
				int end = begin + scan_block;
				if(end > Nparticles)
					end = Nparticles;
				double block_offset = block_sums[block];
				for(int i = begin; i < end; i++){
					CDF[i] += block_offset;
				}
			}
			long long cum_sum = get_time();
			printf("TIME TO CALC CUM SUM TOOK: %f\n", elapsed_time(move_time, cum_sum));
			#pragma omp target update from(seed[0:1])
			double u1 = (1/((double)(Nparticles)))*randu(seed, 0);
			#pragma omp target update to(seed[0:1])
			#pragma omp target teams distribute parallel for firstprivate(u1)
			for(x = 0; x < Nparticles; x++){
				u[x] = u1 + x/((double)(Nparticles));
			}
			long long u_time = get_time();
			printf("TIME TO CALC U TOOK: %f\n", elapsed_time(cum_sum, u_time));
			int j;

				#pragma omp target teams distribute parallel for
				for(j = 0; j < Nparticles; j++){
					int i = findIndexBin(CDF, Nparticles, u[j]);
					if(i == -1)
						i = Nparticles-1;
					xj[j] = arrayX[i];
				yj[j] = arrayY[i];

			}
			long long xyj_time = get_time();
			printf("TIME TO CALC NEW ARRAY X AND Y TOOK: %f\n", elapsed_time(u_time, xyj_time));

			#pragma omp target teams distribute parallel for
			for(x = 0; x < Nparticles; x++){
				//reassign arrayX and arrayY
				arrayX[x] = xj[x];
				arrayY[x] = yj[x];
				weights[x] = 1/((double)(Nparticles));
			}
			long long reset = get_time();
			printf("TIME TO RESET WEIGHTS TOOK: %f\n", elapsed_time(xyj_time, reset));
		}
		}
		free(disk);
		free(objxy);
		free(weights);
	free(likelihood);
	free(xj);
	free(yj);
	free(arrayX);
		free(arrayY);
		free(CDF);
		free(u);
		free(block_sums);
	}
int main(int argc, char * argv[]){

	char* usage = "openmp.out -x <dimX> -y <dimY> -z <Nfr> -np <Nparticles>";
	//check number of arguments
	if(argc != 9)
	{
		printf("%s\n", usage);
		return EXIT_FAILURE;
	}
	//check args deliminators
	if( strcmp( argv[1], "-x" ) ||  strcmp( argv[3], "-y" ) || strcmp( argv[5], "-z" ) || strcmp( argv[7], "-np" ) ) {
		printf( "%s\n",usage );
		return EXIT_FAILURE;
	}

	int IszX, IszY, Nfr, Nparticles;

	//converting a string to a integer
	if( sscanf( argv[2], "%d", &IszX ) == EOF ) {
	   printf("ERROR: dimX input is incorrect");
	   return EXIT_FAILURE;
	}

	if( IszX <= 0 ) {
		printf("dimX must be > 0\n");
		return EXIT_FAILURE;
	}

	//converting a string to a integer
	if( sscanf( argv[4], "%d", &IszY ) == EOF ) {
	   printf("ERROR: dimY input is incorrect");
	   return EXIT_FAILURE;
	}

	if( IszY <= 0 ) {
		printf("dimY must be > 0\n");
		return EXIT_FAILURE;
	}

	//converting a string to a integer
	if( sscanf( argv[6], "%d", &Nfr ) == EOF ) {
	   printf("ERROR: Number of frames input is incorrect");
	   return EXIT_FAILURE;
	}

	if( Nfr <= 0 ) {
		printf("number of frames must be > 0\n");
		return EXIT_FAILURE;
	}

	//converting a string to a integer
	if( sscanf( argv[8], "%d", &Nparticles ) == EOF ) {
	   printf("ERROR: Number of particles input is incorrect");
	   return EXIT_FAILURE;
	}

	if( Nparticles <= 0 ) {
		printf("Number of particles must be > 0\n");
		return EXIT_FAILURE;
	}
	size_t particle_count = (size_t)Nparticles;
	if (particle_count > SIZE_MAX / sizeof(int)) {
		fprintf(stderr, "Error: Number of particles is too large\n");
		return EXIT_FAILURE;
	}
	//establish seed
	int * seed = (int *)malloc(sizeof(int)*particle_count);
	if (seed == NULL) {
		fprintf(stderr, "Error: Memory allocation failed for particle seeds\n");
		return EXIT_FAILURE;
	}
	int i;
	for(i = 0; i < Nparticles; i++)
		seed[i] = i + 1;
	//malloc matrix
	size_t total_size = (size_t)IszX;
	if ((size_t)IszY > SIZE_MAX / total_size) {
		fprintf(stderr, "Error: Video sequence is too large\n");
		free(seed);
		return EXIT_FAILURE;
	}
	total_size *= (size_t)IszY;
	if ((size_t)Nfr > SIZE_MAX / total_size) {
		fprintf(stderr, "Error: Video sequence is too large\n");
		free(seed);
		return EXIT_FAILURE;
	}
	total_size *= (size_t)Nfr;
	if (total_size > (size_t)INT_MAX || total_size > SIZE_MAX / sizeof(int)) {
		fprintf(stderr, "Error: Video sequence is too large\n");
		free(seed);
		return EXIT_FAILURE;
	}
	int * I = (int *)calloc(total_size, sizeof(int));
	if (I == NULL) {
		fprintf(stderr, "Error: Memory allocation failed for video matrix\n");
		free(seed);
		return EXIT_FAILURE;
	}
	long long start = get_time();
	//call video sequence
	videoSequence(I, IszX, IszY, Nfr, seed);
	long long endVideoSequence = get_time();
	printf("VIDEO SEQUENCE TOOK %f\n", elapsed_time(start, endVideoSequence));
	//call particle filter
	particleFilter(I, IszX, IszY, Nfr, seed, Nparticles);
	long long endParticleFilter = get_time();
	printf("PARTICLE FILTER TOOK %f\n", elapsed_time(endVideoSequence, endParticleFilter));
	printf("ENTIRE PROGRAM TOOK %f\n", elapsed_time(start, endParticleFilter));

	free(seed);
	free(I);
	return 0;
}
